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

#ifndef QM3U_PLUGIN_H
#define QM3U_PLUGIN_H

#include <qmediaplaylistioplugin_p.h>
#include <qobject.h>

class QM3uPlaylistPlugin : public QMediaPlaylistIOPlugin
{
   CS_OBJECT(QM3uPlaylistPlugin)

   CS_PLUGIN_IID(QMediaPlaylistInterface_ID)
   CS_PLUGIN_KEY("m3u")

 public:
   explicit QM3uPlaylistPlugin(QObject *parent = nullptr);

   bool canRead(QIODevice *device, const QByteArray &format = QByteArray() ) const override;
   bool canRead(const QUrl &location, const QByteArray &format = QByteArray()) const override;

   bool canWrite(QIODevice *device, const QByteArray &format) const override;

   QMediaPlaylistReader *createReader(QIODevice *device, const QByteArray &format = QByteArray()) override;
   QMediaPlaylistReader *createReader(const QUrl &location, const QByteArray &format = QByteArray()) override;

   QMediaPlaylistWriter *createWriter(QIODevice *device, const QByteArray &format) override;
};

#endif
