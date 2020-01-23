/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include "qwindowsinternalmimedata.h"
#include "qwindowscontext.h"
#include "qwindowsmime.h"
#include <QDebug>

bool QWindowsInternalMimeData::hasFormat_sys(const QString &mime) const
{
   IDataObject *pDataObj = retrieveDataObject();
   if (!pDataObj) {
      return false;
   }

   const QWindowsMimeConverter &mc = QWindowsContext::instance()->mimeConverter();
   const bool has = mc.converterToMime(mime, pDataObj) != 0;
   releaseDataObject(pDataObj);

#if defined(CS_SHOW_DEBUG)
   qDebug() << "QWindowsInternalMimeData::hasFormat_sys:" <<  mime << has;
#endif

   return has;
}

QStringList QWindowsInternalMimeData::formats_sys() const
{
   IDataObject *pDataObj = retrieveDataObject();
   if (! pDataObj) {
      return QStringList();
   }

   const QWindowsMimeConverter &mc = QWindowsContext::instance()->mimeConverter();
   const QStringList fmts = mc.allMimesForFormats(pDataObj);
   releaseDataObject(pDataObj);

#if defined(CS_SHOW_DEBUG)
   qDebug() << "QWindowsInternalMimeData::formats_sys:" <<  fmts;
#endif

   return fmts;
}

QVariant QWindowsInternalMimeData::retrieveData_sys(const QString &mimeType, QVariant::Type type) const
{
   IDataObject *pDataObj = retrieveDataObject();
   if (!pDataObj) {
      return QVariant();
   }

   QVariant result;
   const QWindowsMimeConverter &mc = QWindowsContext::instance()->mimeConverter();

   if (const QWindowsMime *converter = mc.converterToMime(mimeType, pDataObj)) {
      result = converter->convertToMime(mimeType, pDataObj, type);
   }
   releaseDataObject(pDataObj);

   if (QWindowsContext::verbose) {
      qDebug() << __FUNCTION__ << ' '  << mimeType << ' ' << type
               << " returns " << result.type()
               << (result.type() != QVariant::ByteArray ? result.toString() : QString("<data>"));
   }

   return result;
}
