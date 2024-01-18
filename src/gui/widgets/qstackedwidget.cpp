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

#include <qstackedwidget.h>

#ifndef QT_NO_STACKEDWIDGET

#include <qstackedlayout.h>
#include <qevent.h>

#include <qframe_p.h>

class QStackedWidgetPrivate : public QFramePrivate
{
   Q_DECLARE_PUBLIC(QStackedWidget)

 public:
   QStackedWidgetPrivate()
      : layout(nullptr)
   {
   }

   QStackedLayout *layout;
   bool blockChildAdd;
};

QStackedWidget::QStackedWidget(QWidget *parent)
   : QFrame(*new QStackedWidgetPrivate, parent)
{
   Q_D(QStackedWidget);
   d->layout = new QStackedLayout(this);

   connect(d->layout, &QStackedLayout::widgetRemoved,  this, &QStackedWidget::widgetRemoved);
   connect(d->layout, &QStackedLayout::currentChanged, this, &QStackedWidget::currentChanged);
}

QStackedWidget::~QStackedWidget()
{
}

int QStackedWidget::addWidget(QWidget *widget)
{
   return d_func()->layout->addWidget(widget);
}

int QStackedWidget::insertWidget(int index, QWidget *widget)
{
   return d_func()->layout->insertWidget(index, widget);
}

void QStackedWidget::removeWidget(QWidget *widget)
{
   d_func()->layout->removeWidget(widget);
}

void QStackedWidget::setCurrentIndex(int index)
{
   d_func()->layout->setCurrentIndex(index);
}

int QStackedWidget::currentIndex() const
{
   return d_func()->layout->currentIndex();
}

QWidget *QStackedWidget::currentWidget() const
{
   return d_func()->layout->currentWidget();
}

void QStackedWidget::setCurrentWidget(QWidget *widget)
{
   Q_D(QStackedWidget);

   if (d->layout->indexOf(widget) == -1) {
      qWarning("QStackedWidget::setCurrentWidget() Current widget (%p) is not in this stack", static_cast<void *>(widget));
      return;
   }

   d->layout->setCurrentWidget(widget);
}

int QStackedWidget::indexOf(QWidget *widget) const
{
   return d_func()->layout->indexOf(widget);
}

QWidget *QStackedWidget::widget(int index) const
{
   return d_func()->layout->widget(index);
}

int QStackedWidget::count() const
{
   return d_func()->layout->count();
}

bool QStackedWidget::event(QEvent *e)
{
   return QFrame::event(e);
}

#endif // QT_NO_STACKEDWIDGET
