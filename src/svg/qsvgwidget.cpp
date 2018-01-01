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

#include "qsvgwidget.h"

#ifndef QT_NO_SVGWIDGET

#include "qsvgrenderer.h"
#include "qpainter.h"
#include "qwidget_p.h"

QT_BEGIN_NAMESPACE

class QSvgWidgetPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QSvgWidget)
 public:
   QSvgRenderer *renderer;
};

/*!
    Constructs a new SVG display widget with the given \a parent.
*/
QSvgWidget::QSvgWidget(QWidget *parent)
   : QWidget(*new QSvgWidgetPrivate, parent, 0)
{
   d_func()->renderer = new QSvgRenderer(this);
   QObject::connect(d_func()->renderer, SIGNAL(repaintNeeded()),
                    this, SLOT(update()));
}

/*!
    Constructs a new SVG display widget with the given \a parent and loads the contents
    of the specified \a file.
*/
QSvgWidget::QSvgWidget(const QString &file, QWidget *parent)
   : QWidget(*new QSvgWidgetPrivate, parent, 0)
{
   d_func()->renderer = new QSvgRenderer(file, this);
   QObject::connect(d_func()->renderer, SIGNAL(repaintNeeded()),
                    this, SLOT(update()));
}

/*!
    Destroys the widget.
*/
QSvgWidget::~QSvgWidget()
{

}

/*!
    Returns the renderer used to display the contents of the widget.
*/
QSvgRenderer *QSvgWidget::renderer() const
{
   Q_D(const QSvgWidget);
   return d->renderer;
}


/*!
    \reimp
*/
QSize QSvgWidget::sizeHint() const
{
   Q_D(const QSvgWidget);
   if (d->renderer->isValid()) {
      return d->renderer->defaultSize();
   } else {
      return QSize(128, 64);
   }
}


/*!
    \reimp
*/
void QSvgWidget::paintEvent(QPaintEvent *)
{
   Q_D(QSvgWidget);
   QPainter p(this);
   d->renderer->render(&p);
}

/*!
    Loads the contents of the specified SVG \a file and updates the widget.
*/
void QSvgWidget::load(const QString &file)
{
   Q_D(const QSvgWidget);
   d->renderer->load(file);
}

/*!
    Loads the specified SVG format \a contents and updates the widget.
*/
void QSvgWidget::load(const QByteArray &contents)
{
   Q_D(const QSvgWidget);
   d->renderer->load(contents);
}

QT_END_NAMESPACE

#endif // QT_NO_SVGWIDGET
