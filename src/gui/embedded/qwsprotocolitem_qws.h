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

#ifndef QWSPROTOCOLITEM_QWS_H
#define QWSPROTOCOLITEM_QWS_H

/*********************************************************************
 *
 * QWSCommand base class - only use derived classes from that
 *
 *********************************************************************/

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QIODevice;

struct QWSProtocolItem {
   // ctor - dtor
   QWSProtocolItem(int t, int len, char *ptr) : type(t),
      simpleLen(len), rawLen(-1), deleteRaw(false), simpleDataPtr(ptr),
      rawDataPtr(0), bytesRead(0) { }
   virtual ~QWSProtocolItem();

   // data
   int type;
   int simpleLen;
   int rawLen;
   bool deleteRaw;

   // functions
#ifndef QT_NO_QWS_MULTIPROCESS
   void write(QIODevice *s);
   bool read(QIODevice *s);
#endif
   void copyFrom(const QWSProtocolItem *item);

   virtual void setData(const char *data, int len, bool allocateMem = true);

   char *simpleDataPtr;
   char *rawDataPtr;
   // temp variables
   int bytesRead;
};

// This should probably be a method on QWSProtocolItem, but this way avoids
// changing the API of this apparently public header
// size = (int)type + (int)rawLenSize + simpleLen + rawLen
#define QWS_PROTOCOL_ITEM_SIZE( item ) \
    (2 * sizeof(int)) + ((item).simpleDataPtr ? (item).simpleLen : 0) + ((item).rawDataPtr ? (item).rawLen : 0)

QT_END_NAMESPACE

#endif // QWSPROTOCOLITEM_QWS_H
