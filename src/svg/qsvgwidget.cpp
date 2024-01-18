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

#include <qsvgwidget.h>

#ifndef QT_NO_SVGWIDGET

#include <qsvgrenderer.h>
#include <qpainter.h>

#include <qwidget_p.h>

class QSvgWidgetPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QSvgWidget)

 public:
   QSvgRenderer *renderer;
};

QSvgWidget::QSvgWidget(QWidget *parent)
   : QWidget(*new QSvgWidgetPrivate, parent, Qt::EmptyFlag)
{
   d_func()->renderer = new QSvgRenderer(this);
   QObject::connect(d_func()->renderer, SIGNAL(repaintNeeded()), this, SLOT(update()));
}

QSvgWidget::QSvgWidget(const QString &file, QWidget *parent)
   : QWidget(*new QSvgWidgetPrivate, parent, Qt::EmptyFlag)
{
   d_func()->renderer = new QSvgRenderer(file, this);
   QObject::connect(d_func()->renderer, SIGNAL(repaintNeeded()), this, SLOT(update()));
}

QSvgWidget::~QSvgWidget()
{
}

QSvgRenderer *QSvgWidget::renderer() const
{
   Q_D(const QSvgWidget);
   return d->renderer;
}

QSize QSvgWidget::sizeHint() const
{
   Q_D(const QSvgWidget);

   if (d->renderer->isValid()) {
      return d->renderer->defaultSize();
   } else {
      return QSize(128, 64);
   }
}

void QSvgWidget::paintEvent(QPaintEvent *)
{
   Q_D(QSvgWidget);
   QPainter p(this);
   d->renderer->render(&p);
}

void QSvgWidget::load(const QString &file)
{
   Q_D(const QSvgWidget);
   d->renderer->load(file);
}

void QSvgWidget::load(const QByteArray &contents)
{
   Q_D(const QSvgWidget);
   d->renderer->load(contents);
}

#endif // QT_NO_SVGWIDGET
