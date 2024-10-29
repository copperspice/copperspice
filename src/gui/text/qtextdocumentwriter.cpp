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

#include <qtextdocumentwriter.h>

#include <qfile.h>
#include <qbytearray.h>
#include <qfileinfo.h>
#include <qtextcodec.h>
#include <qtextstream.h>
#include <qdebug.h>
#include <qtextdocument.h>
#include <qtextdocumentfragment.h>

#include <qtextdocumentfragment_p.h>
#include <qtextodfwriter_p.h>

#include <algorithm>

class QTextDocumentWriterPrivate
{
 public:
   QTextDocumentWriterPrivate(QTextDocumentWriter *qq);

   // device
   QByteArray format;
   QIODevice *device;
   bool deleteDevice;

#ifndef QT_NO_TEXTCODEC
   QTextCodec *codec;
#endif

   QTextDocumentWriter *q;
};

QTextDocumentWriterPrivate::QTextDocumentWriterPrivate(QTextDocumentWriter *qq)
   : device(nullptr), deleteDevice(false),
#ifndef QT_NO_TEXTCODEC
     codec(QTextCodec::codecForName("utf-8")),
#endif
     q(qq)
{
}

QTextDocumentWriter::QTextDocumentWriter()
   : d(new QTextDocumentWriterPrivate(this))
{
}

QTextDocumentWriter::QTextDocumentWriter(QIODevice *device, const QByteArray &format)
   : d(new QTextDocumentWriterPrivate(this))
{
   d->device = device;
   d->format = format;
}

QTextDocumentWriter::QTextDocumentWriter(const QString &fileName, const QByteArray &format)
   : d(new QTextDocumentWriterPrivate(this))
{
   QFile *file = new QFile(fileName);
   d->device = file;
   d->deleteDevice = true;
   d->format = format;
}

QTextDocumentWriter::~QTextDocumentWriter()
{
   if (d->deleteDevice) {
      delete d->device;
   }
   delete d;
}

void QTextDocumentWriter::setFormat (const QByteArray &format)
{
   d->format = format;
}

QByteArray QTextDocumentWriter::format () const
{
   return d->format;
}

void QTextDocumentWriter::setDevice (QIODevice *device)
{
   if (d->device && d->deleteDevice) {
      delete d->device;
   }

   d->device = device;
   d->deleteDevice = false;
}

QIODevice *QTextDocumentWriter::device () const
{
   return d->device;
}

void QTextDocumentWriter::setFileName (const QString &fileName)
{
   setDevice(new QFile(fileName));
   d->deleteDevice = true;
}

QString QTextDocumentWriter::fileName () const
{
   QFile *file = qobject_cast<QFile *>(d->device);
   return file ? file->fileName() : QString();
}

bool QTextDocumentWriter::write(const QTextDocument *document)
{
   QByteArray suffix;

   if (d->device && d->format.isEmpty()) {
      // if there's no format, see if device is a file, and if so, find
      // the file suffix
      if (QFile *file = qobject_cast<QFile *>(d->device)) {
         suffix = QFileInfo(file->fileName()).suffix().toLower().toLatin1();
      }
   }

   QByteArray format = !d->format.isEmpty() ? d->format.toLower() : suffix;

#ifndef QT_NO_TEXTODFWRITER
   if (format == "odf" || format == "opendocumentformat" || format == "odt") {
      QTextOdfWriter writer(*document, d->device);
#ifndef QT_NO_TEXTCODEC
      writer.setCodec(d->codec);
#endif
      return writer.writeAll();
   }
#endif

#ifndef QT_NO_TEXTHTMLPARSER
   if (format == "html" || format == "htm") {
      if (!d->device->isWritable() && ! d->device->open(QIODevice::WriteOnly)) {
         qWarning() << "QTextDocumentWriter::write() Device can not be opened for writing";
         return false;
      }

      QTextStream ts(d->device);

#ifndef QT_NO_TEXTCODEC
      ts.setCodec(d->codec);
      ts << document->toHtml(d->codec->name());
#endif
      d->device->close();
      return true;
   }
#endif
   if (format == "txt" || format == "plaintext") {
      if (!d->device->isWritable() && ! d->device->open(QIODevice::WriteOnly)) {
         qWarning() << "QTextDocumentWriter::write() Device can not be opened for writing";
         return false;
      }

      QTextStream ts(d->device);

#ifndef QT_NO_TEXTCODEC
      ts.setCodec(d->codec);
#endif

      ts << document->toPlainText();
      d->device->close();

      return true;
   }

   return false;
}

bool QTextDocumentWriter::write(const QTextDocumentFragment &fragment)
{
   if (fragment.d == nullptr) {
      return false;   // invalid fragment.
   }

   QTextDocument *doc = fragment.d->doc;
   if (doc) {
      return write(doc);
   }
   return false;
}

#ifndef QT_NO_TEXTCODEC
void QTextDocumentWriter::setCodec(QTextCodec *codec)
{
   if (codec == nullptr) {
      codec = QTextCodec::codecForName("UTF-8");
   }

   Q_ASSERT(codec);
   d->codec = codec;
}
#endif

#ifndef QT_NO_TEXTCODEC
QTextCodec *QTextDocumentWriter::codec() const
{
   return d->codec;
}
#endif

QList<QByteArray> QTextDocumentWriter::supportedDocumentFormats()
{
   QList<QByteArray> answer;
   answer << "plaintext";

#ifndef QT_NO_TEXTHTMLPARSER
   answer << "HTML";
#endif

#ifndef QT_NO_TEXTODFWRITER
   answer << "ODF";
#endif

   std::sort(answer.begin(), answer.end());

   return answer;
}
