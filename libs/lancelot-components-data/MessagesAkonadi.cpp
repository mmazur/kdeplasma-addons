/*
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "MessagesAkonadi.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>

#include <KIcon>
#include <KRun>
#include <KToolInvocation>
#include <KStandardDirs>

#include "config-lancelot-components-data.h"

#ifndef LANCELOT_DATA_HAS_PIMLIBS

// just in case messages:
I18N_NOOP("Unread messages");
I18N_NOOP("Unable to find Kontact");
I18N_NOOP("Start Akonadi server");
I18N_NOOP("Akonadi server is not running");

#warning "Pimlibs are not present"

    #define DummyModelClassName MessagesAkonadi
    #define DummyModelInit \
        setSelfTitle(i18n("Unread messages"));  \
        setSelfIcon(KIcon("kmail"));            \
        if (!addService("kontact") && !addService("kmail")) {   \
            add(i18n("Unable to find Kontact"), "",             \
                    KIcon("application-x-executable"),          \
                    QVariant("http://kontact.kde.org"));        \
        }

    #include "DummyModel_p.cpp"

    #undef DummyModelClassName
    #undef DummyModelInit

    void MessagesAkonadi::Private::fetchEmailCollectionsDone(KJob * job)
    {}
#else

// We have kdepimlibs

#include <KJob>
#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionStatistics>
#include <Akonadi/CollectionStatisticsJob>
#include <Akonadi/EntityDisplayAttribute>
#include "MessagesAkonadi_p.h"

MessagesAkonadi::Private::Private(MessagesAkonadi * parent)
  : monitor(NULL), unread(0), q(parent)
{
}

void MessagesAkonadi::Private::fetchEmailCollectionsDone(KJob * job)
{
    kDebug();

    q->clear();
    unread = 0;

    q->beginResetModel();
    // TODO: q->setEmitInhibited(true);

    if ( job->error() ) {
        kDebug() << "Job Error:" << job->errorString();

    } else {
        Akonadi::CollectionFetchJob * cjob =
            static_cast < Akonadi::CollectionFetchJob * > ( job );

        foreach (const Akonadi::Collection & collection, cjob->collections()) {
            if (collection.contentMimeTypes().contains("message/rfc822")) {
                int unreadCount = collection.statistics().unreadCount();

                if (unreadCount) {
                    kDebug()
                        << collection.name()
                        << unreadCount
                        << collection.url()
                        ;
                    q->add(
                            i18nc("Directory name (number of unread messages)",
                                "%1 (%2)",
                                collection.name(),
                                unreadCount),
                            QString::null,
                            entityIcon(collection),
                            collection.url()
                        );
                    unread += unreadCount;
                }
            }
        }
    }

    if (q->count() == 0) {
        q->add(i18n("No unread mail"), "", KIcon("mail-folder-inbox"), QVariant());
    }

    if (unread) {
        q->setSelfTitle(i18nc("Unread messages (number of unread messages)",
                              "Unread messages (%1)", QString::number(unread)));
    } else {
        q->setSelfTitle(i18n("Unread messages"));
    }

    // TODO: q->setEmitInhibited(false);
    // TODO: q->updated();
    q->endResetModel();
}

KIcon MessagesAkonadi::Private::entityIcon(const Akonadi::Collection & collection) const
{
    Akonadi::EntityDisplayAttribute * displayAttribute =
        static_cast < Akonadi::EntityDisplayAttribute * > (
            collection.attribute < Akonadi::EntityDisplayAttribute > ()
        );

    if (displayAttribute) {
        return displayAttribute->icon();
    }

    return KIcon("mail-folder-inbox");
}

MessagesAkonadi::MessagesAkonadi()
    : d(new Private(this))
{
    setSelfTitle(i18n("Unread messages"));
    setSelfIcon(KIcon("kmail"));

    d->monitor = new Akonadi::Monitor();
    d->monitor->setCollectionMonitored(Akonadi::Collection::root());
    d->monitor->fetchCollection(true);

    connect(d->monitor, SIGNAL(collectionAdded(Akonadi::Collection,Akonadi::Collection)),
            this, SLOT(updateLater()));
    connect(d->monitor, SIGNAL(collectionRemoved(Akonadi::Collection)),
            this, SLOT(updateLater()));
    connect(d->monitor, SIGNAL(collectionChanged(Akonadi::Collection)),
            this, SLOT(updateLater()));
    connect(d->monitor, SIGNAL(collectionStatisticsChanged(Akonadi::Collection::Id,Akonadi::CollectionStatistics)),
            this, SLOT(updateLater()));

    connect(Akonadi::ServerManager::self(), SIGNAL(stateChanged(Akonadi::ServerManager::State)),
            this, SLOT(updateLater()));

    load();
}

MessagesAkonadi::~MessagesAkonadi()
{
    delete d->monitor;
    delete d;
}

void MessagesAkonadi::updateLater()
{
    QTimer::singleShot(200, this, SLOT(update()));
}

void MessagesAkonadi::update()
{
    load();
}

void MessagesAkonadi::activate(int index)
{
    Q_UNUSED(index);

    // TODO: This could be a bit prettier
    if (index == 0 && d->unread == 0 && !Akonadi::ServerManager::isRunning()) {
        Akonadi::ServerManager::start();
        return;
    }

    if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kmail")) {
        QDBusInterface kmailInterface("org.kde.kmail", "/KMail", "org.kde.kmail.kmail");
        kmailInterface.call("openReader");
    } else {
        KToolInvocation::startServiceByDesktopName("kmail");
    }
}

// QString MessagesAkonadi::selfShortTitle() const
// {
//     if (d->unread) {
//         return QString::number(d->unread);
//     } else {
//         return QString();
//     }
// }

void MessagesAkonadi::load()
{
    kDebug();

    if (!Akonadi::ServerManager::isRunning()) {
        clear();
        d->unread = 0;
        add(i18n("Start Akonadi server"), i18n("Akonadi server is not running"), KIcon("akonadi"), QVariant("start-akonadi"));
        return;

    }

    Akonadi::Collection emailCollection(Akonadi::Collection::root());
    emailCollection.setContentMimeTypes(QStringList() << "message/rfc822");

    Akonadi::CollectionFetchScope scope;
    scope.setIncludeStatistics(true);
    scope.setContentMimeTypes(QStringList() << "message/rfc822");
    scope.setAncestorRetrieval(Akonadi::CollectionFetchScope::All);

    Akonadi::CollectionFetchJob * fetch = new Akonadi::CollectionFetchJob(
            emailCollection, Akonadi::CollectionFetchJob::Recursive);
    fetch->setFetchScope(scope);

    connect(fetch, SIGNAL(result(KJob*)),
            d, SLOT(fetchEmailCollectionsDone(KJob*)));
}

#include "MessagesAkonadi_p.moc"
#include "MessagesAkonadi.moc"

#endif

