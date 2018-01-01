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

#ifndef QMIME_H
#define QMIME_H

#include <QtCore/qmimedata.h>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QMimeSource
{

 public:
   virtual ~QMimeSource();
   virtual QString format(int n = 0) const = 0;
   virtual bool provides(const QString &mimeType) const;
   virtual QByteArray encodedData(const QString &format) const = 0;
};


#if defined(Q_OS_WIN)

QT_BEGIN_INCLUDE_NAMESPACE

typedef struct tagFORMATETC FORMATETC;
typedef struct tagSTGMEDIUM STGMEDIUM;
struct IDataObject;

#include <QtCore/qvariant.h>
QT_END_INCLUDE_NAMESPACE

/*
  Encapsulation of conversion between MIME and Windows CLIPFORMAT.
  Not need on X11, as the underlying protocol uses the MIME standard
  directly.
*/

class Q_GUI_EXPORT QWindowsMime
{

 public:
   QWindowsMime();
   virtual ~QWindowsMime();

   // for converting from Qt
   virtual bool canConvertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData) const = 0;
   virtual bool convertFromMime(const FORMATETC &formatetc, const QMimeData *mimeData, STGMEDIUM *pmedium) const = 0;
   virtual QVector<FORMATETC> formatsForMime(const QString &mimeType, const QMimeData *mimeData) const = 0;

   // for converting to Qt
   virtual bool canConvertToMime(const QString &mimeType, IDataObject *pDataObj) const = 0;
   virtual QVariant convertToMime(const QString &mimeType, IDataObject *pDataObj, QVariant::Type preferredType) const = 0;
   virtual QString mimeForFormat(const FORMATETC &formatetc) const = 0;

   static int registerMimeType(const QString &mime);

 private:
   friend class QClipboardWatcher;
   friend class QDragManager;
   friend class QDropData;
   friend class QOleDataObject;

   static QWindowsMime *converterToMime(const QString &mimeType, IDataObject *pDataObj);
   static QStringList allMimesForFormats(IDataObject *pDataObj);
   static QWindowsMime *converterFromMime(const FORMATETC &formatetc, const QMimeData *mimeData);
   static QVector<FORMATETC> allFormatsForMime(const QMimeData *mimeData);
};

#endif


#if defined(Q_OS_MAC)

/*
  Encapsulation of conversion between MIME and Mac flavor.
  Not needed on X11, as the underlying protocol uses the MIME standard
  directly.
*/

class Q_GUI_EXPORT QMacMime   //Obsolete
{
   char type;

 public:
   enum QMacMimeType { MIME_DND = 0x01, MIME_CLIP = 0x02, MIME_QT_CONVERTOR = 0x04, MIME_ALL = MIME_DND | MIME_CLIP };
   explicit QMacMime(char) {
      Q_UNUSED(type);
   }
   virtual ~QMacMime() { }

   static void initialize() { }

   static QList<QMacMime *> all(QMacMimeType) {
      return QList<QMacMime *>();
   }
   static QMacMime *convertor(QMacMimeType, const QString &, int) {
      return 0;
   }
   static QString flavorToMime(QMacMimeType, int) {
      return QString();
   }

   virtual QString convertorName() = 0;
   virtual int countFlavors() = 0;
   virtual int flavor(int index) = 0;
   virtual bool canConvert(const QString &mime, int flav) = 0;
   virtual QString mimeFor(int flav) = 0;
   virtual int flavorFor(const QString &mime) = 0;
   virtual QVariant convertToMime(const QString &mime, QList<QByteArray> data, int flav) = 0;
   virtual QList<QByteArray> convertFromMime(const QString &mime, QVariant data, int flav) = 0;
};

class Q_GUI_EXPORT QMacPasteboardMime
{
   char type;

 public:
   enum QMacPasteboardMimeType { MIME_DND = 0x01,
                                 MIME_CLIP = 0x02,
                                 MIME_QT_CONVERTOR = 0x04,
                                 MIME_ALL = MIME_DND | MIME_CLIP
                               };
   explicit QMacPasteboardMime(char);
   virtual ~QMacPasteboardMime();

   static void initialize();

   static QList<QMacPasteboardMime *> all(uchar);
   static QMacPasteboardMime *convertor(uchar, const QString &mime, QString flav);
   static QString flavorToMime(uchar, QString flav);

   virtual QString convertorName() = 0;

   virtual bool canConvert(const QString &mime, QString flav) = 0;
   virtual QString mimeFor(QString flav) = 0;
   virtual QString flavorFor(const QString &mime) = 0;
   virtual QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav) = 0;
   virtual QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav) = 0;
};

// ### Qt5/Add const QStringList& QMacPasteboardMime::supportedFlavours()
Q_GUI_EXPORT void qRegisterDraggedTypes(const QStringList &types);
#endif

QT_END_NAMESPACE

#endif // QMIME_H
