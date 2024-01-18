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

#ifndef QDECLARATIVEBORDERIMAGE_P_P_H
#define QDECLARATIVEBORDERIMAGE_P_P_H

#include <qdeclarativeimagebase_p_p.h>
#include <qdeclarativescalegrid_p_p.h>

QT_BEGIN_NAMESPACE

class QNetworkReply;
class QDeclarativeBorderImagePrivate : public QDeclarativeImageBasePrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeBorderImage)

 public:
   QDeclarativeBorderImagePrivate()
      : border(0), sciReply(0),
        horizontalTileMode(QDeclarativeBorderImage::Stretch),
        verticalTileMode(QDeclarativeBorderImage::Stretch),
        redirectCount(0) {
   }

   ~QDeclarativeBorderImagePrivate() {
   }


   QDeclarativeScaleGrid *getScaleGrid() {
      Q_Q(QDeclarativeBorderImage);
      if (!border) {
         border = new QDeclarativeScaleGrid(q);
         static int borderChangedSignalIdx = -1;
         static int doUpdateSlotIdx = -1;
         if (borderChangedSignalIdx < 0) {
            borderChangedSignalIdx = QDeclarativeScaleGrid::staticMetaObject.indexOfSignal("borderChanged()");
         }
         if (doUpdateSlotIdx < 0) {
            doUpdateSlotIdx = QDeclarativeBorderImage::staticMetaObject.indexOfSlot("doUpdate()");
         }
         QMetaObject::connect(border, borderChangedSignalIdx, q, doUpdateSlotIdx);
      }
      return border;
   }

   QDeclarativeScaleGrid *border;
   QUrl sciurl;
   QNetworkReply *sciReply;
   QDeclarativeBorderImage::TileMode horizontalTileMode;
   QDeclarativeBorderImage::TileMode verticalTileMode;
   int redirectCount;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEBORDERIMAGE_P_H
