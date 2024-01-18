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

#ifndef QMEDIAPLAYLISTSOURCECONTROL_P_H
#define QMEDIAPLAYLISTSOURCECONTROL_P_H

#include <qmediacontrol.h>

class QMediaPlaylist;

class Q_MULTIMEDIA_EXPORT QMediaPlaylistSourceControl : public QMediaControl
{
   MULTI_CS_OBJECT(QMediaPlaylistSourceControl)

 public:
   virtual ~QMediaPlaylistSourceControl();

   virtual QMediaPlaylist *playlist() const = 0;
   virtual void setPlaylist(QMediaPlaylist *) = 0;

   MULTI_CS_SIGNAL_1(Public, void playlistChanged(QMediaPlaylist *playlist))
   MULTI_CS_SIGNAL_2(playlistChanged, playlist)

 protected:
   QMediaPlaylistSourceControl(QObject *parent = nullptr);
};

#define QMediaPlaylistSourceControl_iid "com.copperspice.CS.mediaPlayListSourceControl/1.0"
CS_DECLARE_INTERFACE(QMediaPlaylistSourceControl, QMediaPlaylistSourceControl_iid)

#endif
