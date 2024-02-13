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

#include <qmimedata.h>

#include <qstringlist.h>
#include <qtextcodec.h>
#include <qurl.h>

struct QMimeDataStruct {
   QString format;
   QVariant data;
};

class QMimeDataPrivate
{
   Q_DECLARE_PUBLIC(QMimeData)

 public:
   virtual ~QMimeDataPrivate()
   { }

   void removeData(const QString &format);
   void setData(const QString &format, const QVariant &data);
   QVariant getData(const QString &format) const;
   QVariant retrieveTypedData(const QString &format, QVariant::Type type) const;

   QList<QMimeDataStruct> dataList;

 protected:
   QMimeData *q_ptr;

};

void QMimeDataPrivate::removeData(const QString &format)
{
   for (int i = 0; i < dataList.size(); i++) {
      if (dataList.at(i).format == format) {
         dataList.removeAt(i);
         return;
      }
   }
}

void QMimeDataPrivate::setData(const QString &format, const QVariant &data)
{
   // remove it first if the format is already here
   removeData(format);

   QMimeDataStruct mimeData;
   mimeData.format = format;
   mimeData.data   = data;

   dataList.append(mimeData);
}

QVariant QMimeDataPrivate::getData(const QString &format) const
{
   QVariant data;

   for (const auto &item : dataList) {

      if (item.format == format) {
         data = item.data;
         break;
      }
   }

   return data;
}

QVariant QMimeDataPrivate::retrieveTypedData(const QString &format, QVariant::Type type) const
{
   Q_Q(const QMimeData);

   QVariant data = q->retrieveData(format, type);

   if (data.type() == type || ! data.isValid()) {
      return data;
   }

   // provide more conversion possiblities than just what QVariant provides

   // URLs can be lists as well
   if ((type == QVariant::Url && data.type() == QVariant::List)
         || (type == QVariant::List && data.type() == QVariant::Url)) {
      return data;
   }

   // images and pixmaps are interchangeable
   if ((type == QVariant::Pixmap && data.type() == QVariant::Image)
         || (type == QVariant::Image && data.type() == QVariant::Pixmap)) {
      return data;
   }

   if (data.type() == QVariant::ByteArray) {
      // see if we can convert to the requested type

      switch (type) {

#ifndef QT_NO_TEXTCODEC

         case QVariant::String: {
            const QByteArray ba = data.toByteArray();
            QTextCodec *codec = QTextCodec::codecForName("utf-8");

            if (format == "text/html") {
               codec = QTextCodec::codecForHtml(ba, codec);
            }

            return codec->toUnicode(ba);
         }

#endif

         case QVariant::Color: {
            QVariant newData = data;
            newData.convert(QVariant::Color);
            return newData;
         }

         [[fallthrough]];

         case QVariant::List:
            if (format != "text/uri-list") {
               break;
            }

            [[fallthrough]];

         case QVariant::Url: {
            QByteArray ba = data.toByteArray();

            // legacy application will send text/uri-list with a trailing null-terminator
            // not sent for any other text mime-type, remove it

            if (ba.endsWith('\0')) {
               ba.chop(1);
            }

            QList<QByteArray> urls = ba.split('\n');
            QList<QVariant> list;

            for (int i = 0; i < urls.size(); ++i) {
               QByteArray ba = urls.at(i).trimmed();

               if (!ba.isEmpty()) {
                  list.append(QUrl::fromEncoded(ba));
               }
            }

            return list;
         }

         default:
            break;
      }

   } else if (type == QVariant::ByteArray) {

      // try to convert to bytearray
      switch (data.type()) {
         case QVariant::ByteArray:
         case QVariant::Color:
            return data.toByteArray();
            break;

         case QVariant::String:
            return data.toString().toUtf8();
            break;

         case QVariant::Url:
            return data.toUrl().toEncoded();
            break;

         case QVariant::List: {
            // has to be list of URLs
            QByteArray result;
            QList<QVariant> list = data.toList();

            for (int i = 0; i < list.size(); ++i) {
               if (list.at(i).type() == QVariant::Url) {
                  result += list.at(i).toUrl().toEncoded();
                  result += "\r\n";
               }
            }

            if (! result.isEmpty()) {
               return result;
            }

            break;
         }

         default:
            break;
      }
   }

   return data;
}

QMimeData::QMimeData()
   : QObject(nullptr), d_ptr(new QMimeDataPrivate)
{
   d_ptr->q_ptr = this;
}

QMimeData::~QMimeData()
{
}

QList<QUrl> QMimeData::urls() const
{
   Q_D(const QMimeData);

   QVariant data = d->retrieveTypedData(QLatin1String("text/uri-list"), QVariant::List);
   QList<QUrl> urls;

   if (data.type() == QVariant::Url) {
      urls.append(data.toUrl());

   } else if (data.type() == QVariant::List) {
      QList<QVariant> list = data.toList();

      for (int i = 0; i < list.size(); ++i) {
         if (list.at(i).type() == QVariant::Url) {
            urls.append(list.at(i).toUrl());
         }
      }
   }

   return urls;
}

void QMimeData::setUrls(const QList<QUrl> &urls)
{
   Q_D(QMimeData);
   QList<QVariant> list;

   for (int i = 0; i < urls.size(); ++i) {
      list.append(urls.at(i));
   }

   d->setData("text/uri-list", list);
}

bool QMimeData::hasUrls() const
{
   return hasFormat("text/uri-list");
}

QString QMimeData::text() const
{
   Q_D(const QMimeData);
   QVariant data = d->retrieveTypedData("text/plain", QVariant::String);

   return data.toString();
}

void QMimeData::setText(const QString &text)
{
   Q_D(QMimeData);
   d->setData("text/plain", text);
}

bool QMimeData::hasText() const
{
   return hasFormat("text/plain");
}

QString QMimeData::html() const
{
   Q_D(const QMimeData);
   QVariant data = d->retrieveTypedData("text/html", QVariant::String);

   return data.toString();
}

void QMimeData::setHtml(const QString &html)
{
   Q_D(QMimeData);
   d->setData("text/html", html);
}

bool QMimeData::hasHtml() const
{
   return hasFormat("text/html");
}

QVariant QMimeData::imageData() const
{
   Q_D(const QMimeData);
   return d->retrieveTypedData("application/x-qt-image", QVariant::Image);
}

void QMimeData::setImageData(const QVariant &image)
{
   Q_D(QMimeData);
   d->setData("application/x-qt-image", image);
}

bool QMimeData::hasImage() const
{
   return hasFormat("application/x-qt-image");
}

QVariant QMimeData::colorData() const
{
   Q_D(const QMimeData);
   return d->retrieveTypedData("application/x-color", QVariant::Color);
}

void QMimeData::setColorData(const QVariant &color)
{
   Q_D(QMimeData);
   d->setData("application/x-color", color);
}

bool QMimeData::hasColor() const
{
   return hasFormat("application/x-color");
}

QByteArray QMimeData::data(const QString &mimeType) const
{
   Q_D(const QMimeData);
   QVariant data = d->retrieveTypedData(mimeType, QVariant::ByteArray);

   return data.toByteArray();
}

void QMimeData::setData(const QString &mimeType, const QByteArray &data)
{
   Q_D(QMimeData);
   d->setData(mimeType, QVariant(data));
}

bool QMimeData::hasFormat(const QString &mimeType) const
{
   return formats().contains(mimeType);
}

QStringList QMimeData::formats() const
{
   Q_D(const QMimeData);
   QStringList list;

   for (int i = 0; i < d->dataList.size(); i++) {
      list += d->dataList.at(i).format;
   }

   return list;
}

QVariant QMimeData::retrieveData(const QString &mimeType, QVariant::Type type) const
{
   (void) type;
   Q_D(const QMimeData);

   return d->getData(mimeType);
}

void QMimeData::clear()
{
   Q_D(QMimeData);
   d->dataList.clear();
}

void QMimeData::removeFormat(const QString &mimeType)
{
   Q_D(QMimeData);
   d->removeData(mimeType);
}
