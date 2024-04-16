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

#include <qm3u_plugin.h>

#include <qmediaresource.h>
#include <qiodevice.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qurl.h>

CS_PLUGIN_REGISTER(QM3uPlaylistPlugin)

class QM3uPlaylistReader : public QMediaPlaylistReader
{
 public:
   QM3uPlaylistReader(QIODevice *device)
      : m_ownDevice(false), m_device(device), m_textStream(new QTextStream(m_device)) {
      readItem();
   }

   QM3uPlaylistReader(const QUrl &location)
      : m_location(location), m_ownDevice(true) {

      QFile *f = new QFile(location.toLocalFile());

      if (f->open(QIODevice::ReadOnly | QIODevice::Text)) {
         m_device = f;
         m_textStream = new QTextStream(m_device);
         readItem();

      } else {
         delete f;
         m_device     = nullptr;
         m_textStream = nullptr;
      }
   }

   virtual ~QM3uPlaylistReader() {
      if (m_ownDevice) {
         delete m_device;
      }
      delete m_textStream;
   }

   bool atEnd() const override {
      //we can't just use m_textStream->atEnd(),
      //for files with empty lines/comments at end
      return nextResource.isNull();
   }

   QMediaContent readItem() override {
      QMediaContent item;
      if (!nextResource.isNull()) {
         item = QMediaContent(nextResource);
      }

      nextResource = QMediaContent();

      while (m_textStream && !m_textStream->atEnd()) {
         QString line = m_textStream->readLine().trimmed();
         if (line.isEmpty() || line[0] == '#' || line.size() > 4096) {
            continue;
         }

         QUrl fileUrl = QUrl::fromLocalFile(line);
         QUrl url(line);

         //m3u may contain url encoded entries or absolute/relative file names
         //prefer existing file if any
         QList<QUrl> candidates;
         if (!m_location.isEmpty()) {
            candidates << m_location.resolved(fileUrl);
            candidates << m_location.resolved(url);
         }
         candidates << fileUrl;
         candidates << url;

         for (const QUrl &candidate : candidates) {
            if (QFile::exists(candidate.toLocalFile())) {
               nextResource = candidate;
               break;
            }
         }

         if (nextResource.isNull()) {
            //assume the relative urls are file names, not encoded urls if m3u is local file
            if (!m_location.isEmpty() && url.isRelative()) {
               if (m_location.scheme() == QLatin1String("file")) {
                  nextResource = m_location.resolved(fileUrl);
               } else {
                  nextResource = m_location.resolved(url);
               }
            } else {
               nextResource = QMediaContent(QUrl::fromUserInput(line));
            }
         }

         break;
      }

      return item;
   }

   void close() override {
   }

 private:
   QUrl m_location;
   bool m_ownDevice;
   QIODevice *m_device;
   QTextStream *m_textStream;
   QMediaContent nextResource;
};

class QM3uPlaylistWriter : public QMediaPlaylistWriter
{
 public:
   QM3uPlaylistWriter(QIODevice *device)
      : m_device(device), m_textStream(new QTextStream(m_device)) {
   }

   virtual ~QM3uPlaylistWriter() {
      delete m_textStream;
   }

   bool writeItem(const QMediaContent &item) override {
      *m_textStream << item.canonicalUrl().toString() << endl;
      return true;
   }

   void close() override {
   }

 private:
   QIODevice *m_device;
   QTextStream *m_textStream;
};

QM3uPlaylistPlugin::QM3uPlaylistPlugin(QObject *parent)
   : QMediaPlaylistIOPlugin(parent)
{
}

bool QM3uPlaylistPlugin::canRead(QIODevice *device, const QByteArray &format) const
{
   return device->isReadable() && (format == "m3u" || format == "m3u8" || format.isEmpty());
}

bool QM3uPlaylistPlugin::canRead(const QUrl &location, const QByteArray &format) const
{
   if (! QFileInfo(location.toLocalFile()).isReadable()) {
      return false;
   }

   if (format == "m3u" || format == "m3u8") {
      return true;
   }

   if (! format.isEmpty()) {
      return false;
   }

   QString localFile = location.toLocalFile().toLower();

   return localFile.endsWith(QLatin1String("m3u")) || localFile.endsWith(QLatin1String("m3u8"));
}

bool QM3uPlaylistPlugin::canWrite(QIODevice *device, const QByteArray &format) const
{
   return device->isOpen() && device->isWritable() && (format == "m3u" || format == "m3u8");
}

QMediaPlaylistReader *QM3uPlaylistPlugin::createReader(QIODevice *device, const QByteArray &format)
{
   (void) format;
   return new QM3uPlaylistReader(device);
}

QMediaPlaylistReader *QM3uPlaylistPlugin::createReader(const QUrl &location, const QByteArray &format)
{
   (void) format;
   return new QM3uPlaylistReader(location);
}

QMediaPlaylistWriter *QM3uPlaylistPlugin::createWriter(QIODevice *device, const QByteArray &format)
{
   (void) format;
   return new QM3uPlaylistWriter(device);
}

