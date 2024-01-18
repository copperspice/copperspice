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

#ifndef QMACMIME_H
#define QMACMIME_H

#include <CoreFoundation/CoreFoundation.h>

#include <qlist.h>
#include <qmimedata.h>
#include <qstring.h>
#include <qurl.h>

// Duplicate of QMacPasteboardMime in QtMacExtras. Keep in sync!
class QMacInternalPasteboardMime
{
   char type;

 public:
   enum QMacPasteboardMimeType {
      MIME_DND             = 0x01,
      MIME_CLIP            = 0x02,
      MIME_QT_CONVERTOR    = 0x04,
      MIME_QT3_CONVERTOR   = 0x08,
      MIME_ALL             = MIME_DND | MIME_CLIP
   };

   explicit QMacInternalPasteboardMime(char);
   virtual ~QMacInternalPasteboardMime();

   static void initializeMimeTypes();
   static void destroyMimeTypes();

   static QList<QMacInternalPasteboardMime *> all(uchar);
   static QMacInternalPasteboardMime *convertor(uchar, const QString &mime, QString flav);
   static QString flavorToMime(uchar, QString flav);

   virtual QString convertorName() = 0;

   virtual bool canConvert(const QString &mime, QString flav) = 0;
   virtual QString mimeFor(QString flav) = 0;
   virtual QString flavorFor(const QString &mime) = 0;
   virtual QVariant convertToMime(const QString &mime, QList<QByteArray> data, QString flav) = 0;
   virtual QList<QByteArray> convertFromMime(const QString &mime, QVariant data, QString flav) = 0;
   virtual int count(QMimeData *mimeData);
};

void qt_mac_addToGlobalMimeList(QMacInternalPasteboardMime *macMime);
void qt_mac_removeFromGlobalMimeList(QMacInternalPasteboardMime *macMime);
void qt_mac_registerDraggedTypes(const QStringList &types);
const QStringList &qt_mac_enabledDraggedTypes();

#endif

