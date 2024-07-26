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

#include <qxcb_mime.h>

#include <qtextcodec.h>
#include <qimagewriter.h>
#include <qbuffer.h>

#include <X11/Xutil.h>

#undef XCB_ATOM_STRING
#undef XCB_ATOM_PIXMAP
#undef XCB_ATOM_BITMAP

#if ! (defined(QT_NO_DRAGANDDROP) && defined(QT_NO_CLIPBOARD))

QXcbMime::QXcbMime()
   : QInternalMimeData()
{ }

QXcbMime::~QXcbMime()
{}

QString QXcbMime::mimeAtomToString(QXcbConnection *connection, xcb_atom_t a)
{
   if (a == XCB_NONE) {
      return QString();
   }

   // special cases for string type
   if (a == XCB_ATOM_STRING
      || a == connection->atom(QXcbAtom::UTF8_STRING)
      || a == connection->atom(QXcbAtom::TEXT)) {
      return QLatin1String("text/plain");
   }

   // special case for images
   if (a == XCB_ATOM_PIXMAP) {
      return QLatin1String("image/ppm");
   }

   QByteArray atomName = connection->atomName(a);

   // special cases for uris
   if (atomName == "text/x-moz-url") {
      atomName = "text/uri-list";
   }

   return QString::fromLatin1(atomName.constData());
}

bool QXcbMime::mimeDataForAtom(QXcbConnection *connection, xcb_atom_t a, QMimeData *mimeData, QByteArray *data,
   xcb_atom_t *atomFormat, int *dataFormat)
{
   if (! data) {
      return false;
   }

   bool ret = false;
   *atomFormat = a;
   *dataFormat = 8;

   if ((a == connection->atom(QXcbAtom::UTF8_STRING) || a == XCB_ATOM_STRING || a == connection->atom(QXcbAtom::TEXT))
      && QInternalMimeData::hasFormatHelper("text/plain", mimeData)) {

      if (a == connection->atom(QXcbAtom::UTF8_STRING)) {
         *data = QInternalMimeData::renderDataHelper("text/plain", mimeData);
         ret = true;

      } else if (a == XCB_ATOM_STRING || a == connection->atom(QXcbAtom::TEXT)) {
         // ICCCM says STRING is latin1
         *data = QString::fromUtf8(QInternalMimeData::renderDataHelper("text/plain", mimeData)).toLatin1();
         ret = true;
      }
      return ret;
   }

   QString atomName = mimeAtomToString(connection, a);

   if (QInternalMimeData::hasFormatHelper(atomName, mimeData)) {
      *data = QInternalMimeData::renderDataHelper(atomName, mimeData);

      // mimeAtomToString() converts "text/x-moz-url" to "text/uri-list",
      // so QXcbConnection::atomName() has to be used.

      if (atomName == "text/uri-list" && connection->atomName(a) == "text/x-moz-url") {

         const QByteArray uri = data->split('\n').first();

         QString mozUri = QString::fromLatin1(uri);
         mozUri += '\n';

         QTextCodec *codec = QTextCodec::codecForName("UTF-16");
         *data = codec->fromUnicode(mozUri);

      } else if (atomName == "application/x-color") {
         *dataFormat = 16;
      }

      ret = true;

   } else if ((a == XCB_ATOM_PIXMAP || a == XCB_ATOM_BITMAP) && mimeData->hasImage()) {
      ret = true;

   } else if (atomName == "text/plain" && mimeData->hasFormat("text/uri-list")) {
      // return URLs also as plain text.
      *data = QInternalMimeData::renderDataHelper(atomName, mimeData);
      ret = true;
   }

   return ret;
}

QVector<xcb_atom_t> QXcbMime::mimeAtomsForFormat(QXcbConnection *connection, const QString &format)
{
   QVector<xcb_atom_t> atoms;
   atoms.reserve(7);
   atoms.append(connection->internAtom(format.toLatin1().constData()));

   // special cases for strings
   if (format == "text/plain") {
      atoms.append(connection->atom(QXcbAtom::UTF8_STRING));
      atoms.append(XCB_ATOM_STRING);
      atoms.append(connection->atom(QXcbAtom::TEXT));
   }

   // special cases for uris
   if (format == "text/uri-list") {
      atoms.append(connection->internAtom("text/x-moz-url"));
      atoms.append(connection->internAtom("text/plain"));
   }

   //special cases for images
   if (format == "image/ppm") {
      atoms.append(XCB_ATOM_PIXMAP);
   }

   if (format == "image/pbm") {
      atoms.append(XCB_ATOM_BITMAP);
   }

   return atoms;
}

QVariant QXcbMime::mimeConvertToFormat(QXcbConnection *connection, xcb_atom_t a, const QByteArray &data, const QString &format,
   QVariant::Type requestedType, const QByteArray &encoding)
{
   QString atomName = mimeAtomToString(connection, a);

   if (!encoding.isEmpty() && atomName == format + ";charset=" + QString::fromLatin1(encoding)) {

#ifndef QT_NO_TEXTCODEC
      if (requestedType == QVariant::String) {
         QTextCodec *codec = QTextCodec::codecForName(encoding);

         if (codec) {
            return codec->toUnicode(data);
         }
      }
#endif

      return data;
   }

   // special cases for string types
   if (format == "text/plain") {
      if (a == connection->atom(QXcbAtom::UTF8_STRING)) {
         return QString::fromUtf8(data);
      }

      if (a == XCB_ATOM_STRING || a == connection->atom(QXcbAtom::TEXT)) {
         return QString::fromLatin1(data);
      }
   }

   // If data contains UTF16 text, convert it to a string.
   // Firefox uses UTF16 without BOM for text/x-moz-url, "text/html",
   // Google Chrome uses UTF16 without BOM for "text/x-moz-url", UTF16 with BOM for "text/html".

   if ((format == "text/html" || format == "text/uri-list") && data.size() > 1) {
      const quint8 byte0 = data.at(0);
      const quint8 byte1 = data.at(1);

      if ((byte0 == 0xff && byte1 == 0xfe) || (byte0 == 0xfe && byte1 == 0xff)
         || (byte0 != 0 && byte1 == 0) || (byte0 == 0 && byte1 != 0)) {

         QTextCodec *codec = QTextCodec::codecForName("UTF-16");
         const QString str = codec->toUnicode(data);

         if (! str.isEmpty()) {

            if (format == "text/uri-list") {
               const QStringList urls = str.split('\n');
               QList<QVariant> list;

               for (const QString &s : urls) {
                  const QUrl url(s.trimmed());

                  if (url.isValid()) {
                     list.append(url);
                  }
               }

               // We expect "text/x-moz-url" as <url><space><title>.
               // The atomName variable is not used because mimeAtomToString()
               // converts "text/x-moz-url" to "text/uri-list".

               if (! list.isEmpty() && connection->atomName(a) == "text/x-moz-url") {
                  return list.first();
               }

               return list;

            } else {
               return str;
            }
         }
      }
   }

   if (atomName == format) {
      return data;
   }

#if 0 // ###
   // special case for images
   if (format == "image/ppm") {
      if (a == XCB_ATOM_PIXMAP && data.size() == sizeof(Pixmap)) {
         Pixmap xpm = *((Pixmap *)data.data());

         if (!xpm) {
            return QByteArray();
         }

         Window root;
         int x;
         int y;
         uint width;
         uint height;
         uint border_width;
         uint depth;

         XGetGeometry(display, xpm, &root, &x, &y, &width, &height, &border_width, &depth);
         XImage *ximg = XGetImage(display, xpm, x, y, width, height, AllPlanes, depth == 1 ? XYPixmap : ZPixmap);
         QImage qimg = QXlibStatic::qimageFromXImage(ximg);
         XDestroyImage(ximg);

         QImageWriter imageWriter;
         imageWriter.setFormat("PPMRAW");
         QBuffer buf;
         buf.open(QIODevice::WriteOnly);
         imageWriter.setDevice(&buf);
         imageWriter.write(qimg);
         return buf.buffer();
      }
   }
#endif
   return QVariant();
}

xcb_atom_t QXcbMime::mimeAtomForFormat(QXcbConnection *connection, const QString &format, QVariant::Type requestedType,
   const QVector<xcb_atom_t> &atoms, QByteArray *requestedEncoding)
{
   requestedEncoding->clear();

   // find matches for string types
   if (format == "text/plain") {
      if (atoms.contains(connection->atom(QXcbAtom::UTF8_STRING))) {
         return connection->atom(QXcbAtom::UTF8_STRING);
      }

      if (atoms.contains(XCB_ATOM_STRING)) {
         return XCB_ATOM_STRING;
      }

      if (atoms.contains(connection->atom(QXcbAtom::TEXT))) {
         return connection->atom(QXcbAtom::TEXT);
      }
   }

   // find matches for uri types
   if (format == "text/uri-list") {
      xcb_atom_t a = connection->internAtom(format.toLatin1().constData());
      if (a && atoms.contains(a)) {
         return a;
      }

      a = connection->internAtom("text/x-moz-url");
      if (a && atoms.contains(a)) {
         return a;
      }
   }

   // find match for image
   if (format == "image/ppm") {
      if (atoms.contains(XCB_ATOM_PIXMAP)) {
         return XCB_ATOM_PIXMAP;
      }
   }

   // for string/text requests try to use a format with a well-defined charset
   // first to avoid encoding problems
   if (requestedType == QVariant::String && format.startsWith("text/") && ! format.contains("charset=")) {

      QString formatWithCharset = format;
      formatWithCharset.append(";charset=utf-8");

      xcb_atom_t a = connection->internAtom(formatWithCharset.toLatin1().constData());

      if (a && atoms.contains(a)) {
         *requestedEncoding = "utf-8";
         return a;
      }
   }

   xcb_atom_t a = connection->internAtom(format.toLatin1().constData());
   if (a && atoms.contains(a)) {
      return a;
   }

   return 0;
}

#endif // !(defined(QT_NO_DRAGANDDROP) && defined(QT_NO_CLIPBOARD))


