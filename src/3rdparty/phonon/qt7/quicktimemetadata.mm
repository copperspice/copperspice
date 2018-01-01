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

/********************************************************
**  This file is part of the KDE project.
********************************************************/

#import <QTKit/QTMovie.h>

#include "quicktimemetadata.h"
#include "quicktimevideoplayer.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{

QuickTimeMetaData::QuickTimeMetaData()
{
    m_videoPlayer = 0;
    m_movieChanged = false;
}

QuickTimeMetaData::~QuickTimeMetaData()
{
}

void QuickTimeMetaData::setVideo(QuickTimeVideoPlayer *videoPlayer)
{
    m_videoPlayer = videoPlayer;
    m_movieChanged = true;
    m_metaData.clear();
}

void QuickTimeMetaData::readMetaData()
{
	if (!m_videoPlayer)
        return;
    QMultiMap<QString, QString> metaMap;
   
	 NSString *name = [m_videoPlayer->qtMovie() attributeForKey:@"QTMovieDisplayNameAttribute"];
	 metaMap.insert(QLatin1String("nam"), QString::fromUtf8([name UTF8String]));

    m_metaData.insert(QLatin1String("ARTIST"), metaMap.value(QLatin1String("ART")));
    m_metaData.insert(QLatin1String("ALBUM"), metaMap.value(QLatin1String("alb")));
    m_metaData.insert(QLatin1String("TITLE"), metaMap.value(QLatin1String("nam")));
    m_metaData.insert(QLatin1String("DATE"), metaMap.value(QLatin1String("day")));
    m_metaData.insert(QLatin1String("GENRE"), metaMap.value(QLatin1String("gnre")));
    m_metaData.insert(QLatin1String("TRACKNUMBER"), metaMap.value(QLatin1String("trk")));
    m_metaData.insert(QLatin1String("DESCRIPTION"), metaMap.value(QLatin1String("des")));
}

QMultiMap<QString, QString> QuickTimeMetaData::metaData()
{
    if (m_videoPlayer && m_videoPlayer->hasMovie() && m_movieChanged)
        readMetaData();
    return m_metaData;
}

}}

QT_END_NAMESPACE
