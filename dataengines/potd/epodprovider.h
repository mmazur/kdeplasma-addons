/*
 *   Copyright (C) 2007 Tobias Koenig <tokoe@kde.org>
 *   Copyright  2008 by Anne-Marie Mahfouf <annma@kde.org>                 
 *
 *   This program is free software; you can redistribute it and/or modify  
 *   it under the terms of the GNU General Public License as published by  
 *   the Free Software Foundation; either version 2 of the License, or     
 *   (at your option) any later version.   
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

#ifndef EPODPROVIDER_H
#define EPODPROVIDER_H

#include "potdprovider.h"
// Qt
#include <QImage>

class KJob;

/**
 * This class provides the image for EPOD 
 * "Earth Science Picture Of the Day"
 * located at http://epod.usra.edu/.
 */
class EpodProvider : public PotdProvider
{
    Q_OBJECT

    public:
        /**
         * Creates a new EPOD provider.
         *
         * @param date The date for which the image shall be fetched.
         * @param parent The parent object.
         */
        EpodProvider( QObject *parent, const QVariantList &args );

        /**
         * Destroys the EPOD provider.
         */
        ~EpodProvider() override;

        /**
         * Returns the requested image.
         *
         * Note: This method returns only a valid image after the
         *       finished() signal has been emitted.
         */
        QImage image() const override;

    private:
        void pageRequestFinished(KJob *job);
        void imageRequestFinished(KJob *job);

    private:
        QImage mImage;
};

#endif
