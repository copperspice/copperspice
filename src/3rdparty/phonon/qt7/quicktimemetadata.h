/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QT7_QUICKTIMEMETADATA_H
#define QT7_QUICKTIMEMETADATA_H

#include "backendheader.h"
#include <phonon/mediasource.h>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class QuickTimeVideoPlayer;
    class QuickTimeMetaData
    {
        public:
            QuickTimeMetaData();
            virtual ~QuickTimeMetaData();

            void setVideo(QuickTimeVideoPlayer *videoPlayer);
            QMultiMap<QString, QString> metaData();

        private:
            QMultiMap<QString, QString> m_metaData;
            bool m_movieChanged;
            QuickTimeVideoPlayer *m_videoPlayer;
            void readMetaData();
    };

}} // namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_QUICKTIMEMETADATA_H
