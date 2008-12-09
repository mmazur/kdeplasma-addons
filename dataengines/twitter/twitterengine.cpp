/*
 *   Copyright (C) 2007 Trever Fischer <wm161@wm161.net>
 *   Copyright (C) 2007 André Duffeck <duffeck@kde.org>
 *   Copyright (C) 2007 Chani Armitage <chanika@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "twitterengine.h"

#include <QDateTime>
#include <QDomDocument>
#include <QDomNode>
#include <QDomNodeList>
#include <QImage>
#include <QPixmap>

#include <KDebug>
#include <KUrl>
#include <ksocketfactory.h>
#include <KUrl>

#include "timelinesource.h"
#include "imagesource.h"

TwitterEngine::TwitterEngine(QObject* parent, const QVariantList& args)
    : Plasma::DataEngine(parent, args)
{
    addSource(ImageSource::self());
    setMinimumPollingInterval(2 * 60 * 1000); // 2 minutes minimum
}

TwitterEngine::~TwitterEngine()
{
}

bool TwitterEngine::sourceRequestEvent(const QString &name)
{
    //kDebug() << name;
    if (name == "UserImages") {
        // these are updated by the engine itself, not consumers
        return true;
    }

    if (!name.startsWith("Timeline:") && !name.startsWith("TimelineWithFriends:")  && !name.startsWith("Profile:")) {
        return false;
    }

    updateSourceEvent(name); //start a download
    return true;
}

Plasma::Service* TwitterEngine::serviceForSource(const QString &name)
{
    TimelineSource *source = dynamic_cast<TimelineSource*>(containerForSource(name));

    if (!source) {
        return Plasma::DataEngine::serviceForSource(name);
    }

    Plasma::Service *service = source->createService();
    service->setParent(this);
    return service;
}

//called when it's time to update a source
//also called by twitter from sourceRequested
//and when it thinks an update would be useful
//always returns false becaues everything is async
bool TwitterEngine::updateSourceEvent(const QString &name)
{
    //kDebug() << name;
    //right now it only makes sense to do an update on timelines
    if (!name.startsWith("Timeline:") && !name.startsWith("TimelineWithFriends:") && !name.startsWith("Profile:")) {
        return false;
    }

    QStringList tokens = name.split(':');
    if (tokens.count() != 2) {
        return false;
    }

    TimelineSource::RequestType requestType = TimelineSource::Timeline;

    QString who = tokens.at(1);
    if (tokens.at(0) == "TimelineWithFriends") {
        requestType = TimelineSource::TimelineWithFriends;
    } else if (tokens.at(0) == "Profile") {
        requestType = TimelineSource::Profile;
    }

    TimelineSource *source = dynamic_cast<TimelineSource*>(containerForSource(name));

    if (!source) {
        source = new TimelineSource(who, requestType, this);
        source->setObjectName(name);
        addSource(source);
    }

    source->update();
    return false;
}

#include "twitterengine.moc"
