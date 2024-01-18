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

#ifndef QSCROLLBAR_H
#define QSCROLLBAR_H

#include <qwidget.h>
#include <qabstractslider.h>

#ifndef QT_NO_SCROLLBAR

class QScrollBarPrivate;
class QStyleOptionSlider;

class Q_GUI_EXPORT QScrollBar : public QAbstractSlider
{
   GUI_CS_OBJECT(QScrollBar)

 public:
   explicit QScrollBar(QWidget *parent = nullptr);
   explicit QScrollBar(Qt::Orientation orientation, QWidget *parent = nullptr);

   QScrollBar(const QScrollBar &other) = delete;
   QScrollBar &operator=(const QScrollBar &other) = delete;

   ~QScrollBar();

   QSize sizeHint() const override;
   bool event(QEvent *event) override;

 protected:
   void paintEvent(QPaintEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void hideEvent(QHideEvent *event) override;
   void sliderChange(SliderChange change) override;

#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QContextMenuEvent *event) override;
#endif

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *event) override;
#endif

   void initStyleOption(QStyleOptionSlider *option) const;

 private:
   Q_DECLARE_PRIVATE(QScrollBar)

   friend class QAbstractScrollAreaPrivate;
   friend Q_GUI_EXPORT QStyleOptionSlider qt_qscrollbarStyleOption(QScrollBar *scrollBar);
};

#endif // QT_NO_SCROLLBAR

#endif
