/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QDECLARATIVEANIMATEDIMAGE_P_P_H
#define QDECLARATIVEANIMATEDIMAGE_P_P_H

#include <qdeclarativeimage_p_p.h>

#ifndef QT_NO_MOVIE

QT_BEGIN_NAMESPACE

class QMovie;
class QNetworkReply;

class QDeclarativeAnimatedImagePrivate : public QDeclarativeImagePrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeAnimatedImage)

 public:
   QDeclarativeAnimatedImagePrivate()
      : playing(true), paused(false), preset_currentframe(0), _movie(0), reply(0), redirectCount(0) {
   }

   bool playing;
   bool paused;
   int preset_currentframe;
   QMovie *_movie;
   QNetworkReply *reply;
   int redirectCount;
};

QT_END_NAMESPACE

#endif // QT_NO_MOVIE

#endif // QDECLARATIVEANIMATEDIMAGE_P_H
