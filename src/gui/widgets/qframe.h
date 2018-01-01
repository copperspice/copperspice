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

#ifndef QFRAME_H
#define QFRAME_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class QFramePrivate;

class Q_GUI_EXPORT QFrame : public QWidget
{
   GUI_CS_OBJECT(QFrame)

   GUI_CS_ENUM(Shape)
   GUI_CS_ENUM(Shadow)

   GUI_CS_PROPERTY_READ(frameShape, frameShape)
   GUI_CS_PROPERTY_WRITE(frameShape, setFrameShape)
   GUI_CS_PROPERTY_READ(frameShadow, frameShadow)
   GUI_CS_PROPERTY_WRITE(frameShadow, setFrameShadow)
   GUI_CS_PROPERTY_READ(lineWidth, lineWidth)
   GUI_CS_PROPERTY_WRITE(lineWidth, setLineWidth)
   GUI_CS_PROPERTY_READ(midLineWidth, midLineWidth)
   GUI_CS_PROPERTY_WRITE(midLineWidth, setMidLineWidth)
   GUI_CS_PROPERTY_READ(frameWidth, frameWidth)
   GUI_CS_PROPERTY_READ(frameRect, frameRect)
   GUI_CS_PROPERTY_WRITE(frameRect, setFrameRect)
   GUI_CS_PROPERTY_DESIGNABLE(frameRect, false)

 public:
   explicit QFrame(QWidget *parent = nullptr, Qt::WindowFlags f = 0);
   ~QFrame();

   int frameStyle() const;
   void setFrameStyle(int);

   int frameWidth() const;

   QSize sizeHint() const override;

   enum Shape {
      NoFrame  = 0, // no frame
      Box = 0x0001, // rectangular box
      Panel = 0x0002, // rectangular panel
      WinPanel = 0x0003, // rectangular panel (Windows)
      HLine = 0x0004, // horizontal line
      VLine = 0x0005, // vertical line
      StyledPanel = 0x0006 // rectangular panel depending on the GUI style
   };
   enum Shadow {
      Plain = 0x0010, // plain line
      Raised = 0x0020, // raised shadow effect
      Sunken = 0x0030 // sunken shadow effect
   };

   enum StyleMask {
      Shadow_Mask = 0x00f0, // mask for the shadow
      Shape_Mask = 0x000f // mask for the shape
   };

   Shape frameShape() const;
   void setFrameShape(Shape);
   Shadow frameShadow() const;
   void setFrameShadow(Shadow);

   int lineWidth() const;
   void setLineWidth(int);

   int midLineWidth() const;
   void setMidLineWidth(int);

   QRect frameRect() const;
   void setFrameRect(const QRect &);

 protected:
   bool event(QEvent *e) override;
   void paintEvent(QPaintEvent *) override;
   void changeEvent(QEvent *) override;
   void drawFrame(QPainter *);

   QFrame(QFramePrivate &dd, QWidget *parent = nullptr, Qt::WindowFlags f = 0);

 private:
   Q_DISABLE_COPY(QFrame)
   Q_DECLARE_PRIVATE(QFrame)
};

QT_END_NAMESPACE

#endif // QFRAME_H
