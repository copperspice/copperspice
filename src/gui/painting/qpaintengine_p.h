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

#ifndef QPAINTENGINE_P_H
#define QPAINTENGINE_P_H

#include <qpainter.h>
#include <qpaintengine.h>
#include <qregion.h>

class QPaintDevice;

class QPaintEnginePrivate
{
   Q_DECLARE_PUBLIC(QPaintEngine)

 public:
   QPaintEnginePrivate()
      : pdev(nullptr), q_ptr(nullptr), currentClipDevice(nullptr), hasSystemTransform(0), hasSystemViewport(0)
   {
   }

   virtual ~QPaintEnginePrivate()
   {
   }

   QPaintDevice *pdev;
   QPaintEngine *q_ptr;
   QRegion systemClip;
   QRect systemRect;
   QRegion systemViewport;
   QTransform systemTransform;
   QPaintDevice *currentClipDevice;

   uint hasSystemTransform : 1;
   uint hasSystemViewport : 1;

   inline void transformSystemClip() {
      if (systemClip.isEmpty()) {
         return;
      }

      if (hasSystemTransform) {
         if (systemTransform.type() <= QTransform::TxTranslate) {
            systemClip.translate(qRound(systemTransform.dx()), qRound(systemTransform.dy()));
         } else {
            systemClip = systemTransform.map(systemClip);
         }
      }

      // Make sure we're inside the viewport.
      if (hasSystemViewport) {
         systemClip &= systemViewport;
         if (systemClip.isEmpty()) {
            // We don't want to paint without system clip, so set it to 1 pixel :)
            systemClip = QRect(systemViewport.boundingRect().topLeft(), QSize(1, 1));
         }
      }
   }

   inline void setSystemTransform(const QTransform &xform) {
      systemTransform = xform;
      if ((hasSystemTransform = !xform.isIdentity()) || hasSystemViewport) {
         transformSystemClip();
      }
      systemStateChanged();
   }

   inline void setSystemViewport(const QRegion &region) {
      systemViewport = region;
      hasSystemViewport = !systemViewport.isEmpty();
   }

   virtual void systemStateChanged() { }

   void drawBoxTextItem(const QPointF &p, const QTextItemInt &ti);
};


#endif
