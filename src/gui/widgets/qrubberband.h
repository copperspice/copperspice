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

#ifndef QRUBBERBAND_H
#define QRUBBERBAND_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_RUBBERBAND

class QRubberBandPrivate;
class QStyleOptionRubberBand;

class Q_GUI_EXPORT QRubberBand : public QWidget
{
   GUI_CS_OBJECT(QRubberBand)

 public:
   enum Shape { Line, Rectangle };
   explicit QRubberBand(Shape, QWidget * = 0);
   ~QRubberBand();

   Shape shape() const;

   void setGeometry(const QRect &r);

   inline void setGeometry(int x, int y, int w, int h);
   inline void move(int x, int y);

   inline void move(const QPoint &p) {
      move(p.x(), p.y());
   }

   inline void resize(int w, int h) {
      setGeometry(geometry().x(), geometry().y(), w, h);
   }

   inline void resize(const QSize &s) {
      resize(s.width(), s.height());
   }

 protected:
   bool event(QEvent *e) override;
   void paintEvent(QPaintEvent *) override;
   void changeEvent(QEvent *) override;
   void showEvent(QShowEvent *) override;
   void resizeEvent(QResizeEvent *) override;
   void moveEvent(QMoveEvent *) override;
   void initStyleOption(QStyleOptionRubberBand *option) const;

 private:
   Q_DECLARE_PRIVATE(QRubberBand)
};

void QRubberBand::setGeometry(int ax, int ay, int aw, int ah)
{
   setGeometry(QRect(ax, ay, aw, ah));
}

void QRubberBand::move(int ax, int ay)
{
   setGeometry(ax, ay, width(), height());
}

#endif // QT_NO_RUBBERBAND

QT_END_NAMESPACE

#endif // QRUBBERBAND_H
