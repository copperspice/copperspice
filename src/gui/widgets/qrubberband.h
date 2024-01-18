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

#ifndef QRUBBERBAND_H
#define QRUBBERBAND_H

#include <qwidget.h>

#ifndef QT_NO_RUBBERBAND

class QRubberBandPrivate;
class QStyleOptionRubberBand;

class Q_GUI_EXPORT QRubberBand : public QWidget
{
   GUI_CS_OBJECT(QRubberBand)

 public:
   enum Shape { Line, Rectangle };

   explicit QRubberBand(Shape shape, QWidget *parent = nullptr);

   ~QRubberBand();

   Shape shape() const;

   void setGeometry(const QRect &rect);

   inline void setGeometry(int x, int y, int width, int height);
   inline void move(int x, int y);

   inline void move(const QPoint &point) {
      move(point.x(), point.y());
   }

   inline void resize(int width, int height) {
      setGeometry(geometry().x(), geometry().y(), width, height);
   }

   inline void resize(const QSize &size) {
      resize(size.width(), size.height());
   }

 protected:
   bool event(QEvent *event) override;
   void paintEvent(QPaintEvent *event) override;
   void changeEvent(QEvent *event) override;
   void showEvent(QShowEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void moveEvent(QMoveEvent *event) override;
   void initStyleOption(QStyleOptionRubberBand *option) const;

 private:
   Q_DECLARE_PRIVATE(QRubberBand)
};

void QRubberBand::setGeometry(int x, int y, int width, int height)
{
   setGeometry(QRect(x, y, width, height));
}

void QRubberBand::move(int x, int y)
{
   setGeometry(x, y, width(), height());
}

#endif // QT_NO_RUBBERBAND

#endif // QRUBBERBAND_H
