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

#ifndef QWINDOWSMIME_H
#define QWINDOWSMIME_H

#include <qwin_additional.h>
#include <qvector.h>
#include <qlist.h>
#include <qvariant.h>

class QDebug;
class QMimeData;

class QWindowsMime
{
 public:
   QWindowsMime();

   QWindowsMime(const QWindowsMime &) = delete;
   QWindowsMime &operator=(const QWindowsMime &) = delete;

   virtual ~QWindowsMime();

   // for converting from CS
   virtual bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const = 0;
   virtual bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const = 0;
   virtual QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const = 0;

   virtual bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const = 0;
   virtual QVariant convertToMime(const QString &mimeType, IDataObject *pDataObj, QVariant::Type preferredType) const = 0;
   virtual QString mimeForFormat(const FORMATETC &formatetc) const = 0;

   static int registerMimeType(const QString &mime);
};

class QWindowsMimeConverter
{
 public:
   QWindowsMimeConverter();

   QWindowsMimeConverter(const QWindowsMimeConverter &) = delete;
   QWindowsMimeConverter &operator=(const QWindowsMimeConverter &) = delete;

   ~QWindowsMimeConverter();

   QWindowsMime *converterToMime(const QString &mimeType, IDataObject *pDataObj) const;
   QStringList allMimesForFormats(IDataObject *pDataObj) const;
   QWindowsMime *converterFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const;
   QVector<FORMATETC> allFormatsForMime(const QMimeData *mimeData) const;

   QVariant convertToMime(const QStringList &mimeTypes, IDataObject *pDataObj, QVariant::Type preferredType,
      QString *format = nullptr) const;

   void registerMime(QWindowsMime *mime);
   void unregisterMime(QWindowsMime *mime) {
      m_mimes.removeOne(mime);
   }

   static QString clipboardFormatName(int cf);

 private:
   void ensureInitialized() const;

   mutable QList<QWindowsMime *> m_mimes;
   mutable int m_internalMimeCount;
};

QDebug operator<<(QDebug debug, const FORMATETC &);
QDebug operator<<(QDebug debug, IDataObject *);

#endif // QWINDOWSMIME_H
