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

#ifndef QMEDIAPLAYLISTIOPLUGIN_P_H
#define QMEDIAPLAYLISTIOPLUGIN_P_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qplugin.h>
#include <qmediacontent.h>

class QUrl;
class QByteArray;
class QIODevice;

class Q_MULTIMEDIA_EXPORT QMediaPlaylistReader
{
 public:
   virtual ~QMediaPlaylistReader();

   virtual bool atEnd() const = 0;
   virtual QMediaContent readItem() = 0;
   virtual void close() = 0;
};

class Q_MULTIMEDIA_EXPORT QMediaPlaylistWriter
{
 public:
   virtual ~QMediaPlaylistWriter();

   virtual bool writeItem(const QMediaContent &content) = 0;
   virtual void close() = 0;
};

struct Q_MULTIMEDIA_EXPORT QMediaPlaylistIOInterface {
   virtual bool canRead(QIODevice *device, const QByteArray &format = QByteArray() ) const = 0;
   virtual bool canRead(const QUrl &location, const QByteArray &format = QByteArray()) const = 0;

   virtual bool canWrite(QIODevice *device, const QByteArray &format) const = 0;

   virtual QMediaPlaylistReader *createReader(QIODevice *device, const QByteArray &format = QByteArray()) = 0;
   virtual QMediaPlaylistReader *createReader(const QUrl &location, const QByteArray &format = QByteArray()) = 0;

   virtual QMediaPlaylistWriter *createWriter(QIODevice *device, const QByteArray &format) = 0;
};

#define QMediaPlaylistInterface_ID "com.copperspice.CS.mediaPlayList/1.0"
CS_DECLARE_INTERFACE(QMediaPlaylistIOInterface, QMediaPlaylistInterface_ID)


class Q_MULTIMEDIA_EXPORT QMediaPlaylistIOPlugin : public QObject, public QMediaPlaylistIOInterface
{
   MULTI_CS_OBJECT(QMediaPlaylistIOPlugin)
   CS_INTERFACES(QMediaPlaylistIOInterface)

 public:
   explicit QMediaPlaylistIOPlugin(QObject *parent = nullptr);
   virtual ~QMediaPlaylistIOPlugin();
};

#endif
