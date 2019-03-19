/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qwsembedwidget.h>

#ifndef QT_NO_QWSEMBEDWIDGET

#include <qwsdisplay_qws.h>
#include <qwidget_p.h>
#include <qwsdisplay_qws_p.h>
#include <qwscommand_qws_p.h>

QT_BEGIN_NAMESPACE

// TODO:
// Must remove window decorations from the embedded window
// Focus In/Out, Keyboard/Mouse...
//
// BUG: what if my parent change parent?

class QWSEmbedWidgetPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QWSEmbedWidget);

 public:
   QWSEmbedWidgetPrivate(int winId);
   void updateWindow();
   void resize(const QSize &size);

   QWidget *window;
   WId windowId;
   WId embeddedId;
};

QWSEmbedWidgetPrivate::QWSEmbedWidgetPrivate(int winId)
   : window(0), windowId(0), embeddedId(winId)
{
}

void QWSEmbedWidgetPrivate::updateWindow()
{
   Q_Q(QWSEmbedWidget);

   QWidget *win = q->window();
   if (win == window) {
      return;
   }

   if (window) {
      window->removeEventFilter(q);
      QWSEmbedCommand command;
      command.setData(windowId, embeddedId, QWSEmbedEvent::StopEmbed);
      QWSDisplay::instance()->d->sendCommand(command);
   }

   window = win;
   if (!window) {
      return;
   }
   windowId = window->winId();

   QWSEmbedCommand command;
   command.setData(windowId, embeddedId, QWSEmbedEvent::StartEmbed);
   QWSDisplay::instance()->d->sendCommand(command);
   window->installEventFilter(q);
   q->installEventFilter(q);
}

void QWSEmbedWidgetPrivate::resize(const QSize &size)
{
   if (!window) {
      return;
   }

   Q_Q(QWSEmbedWidget);

   QWSEmbedCommand command;
   command.setData(windowId, embeddedId, QWSEmbedEvent::Region,
                   QRect(q->mapToGlobal(QPoint(0, 0)), size));
   QWSDisplay::instance()->d->sendCommand(command);
}

/*!
    \class QWSEmbedWidget
    \since 4.2
    \ingroup qws
    \ingroup advanced

    \brief The QWSEmbedWidget class enables embedded top-level widgets
    in Qt for Embedded Linux.

    Note that this class is only available in \l{Qt for Embedded Linux}.

    QWSEmbedWidget inherits QWidget and acts as any other widget, but
    in addition it is capable of embedding another top-level widget.

    An example of use is when painting directly onto the screen using
    the QDirectPainter class. Then the reserved region can be embedded
    into an instance of the QWSEmbedWidget class, providing for
    example event handling and size policies for the reserved region.

    All that is required to embed a top-level widget is its window ID.

    \sa {Qt for Embedded Linux Architecture}
*/

/*!
    Constructs a widget with the given \a parent, embedding the widget
    identified by the given window \a id.
*/
QWSEmbedWidget::QWSEmbedWidget(WId id, QWidget *parent)
   : QWidget(*new QWSEmbedWidgetPrivate(id), parent, 0)
{
   Q_D(QWSEmbedWidget);
   d->updateWindow();
}

/*!
    Destroys this widget.
*/
QWSEmbedWidget::~QWSEmbedWidget()
{
   Q_D(QWSEmbedWidget);
   if (!d->window) {
      return;
   }

   QWSEmbedCommand command;
   command.setData(d->windowId, d->embeddedId, QWSEmbedEvent::StopEmbed);
   QWSDisplay::instance()->d->sendCommand(command);
}

/*!
    \reimp
*/
bool QWSEmbedWidget::eventFilter(QObject *object, QEvent *event)
{
   Q_D(QWSEmbedWidget);
   if (object == d->window && event->type() == QEvent::Move) {
      resizeEvent(0);
   } else if (object == this && event->type() == QEvent::Hide) {
      d->resize(QSize());
   }
   return QWidget::eventFilter(object, event);
}

/*!
    \reimp
*/
void QWSEmbedWidget::changeEvent(QEvent *event)
{
   Q_D(QWSEmbedWidget);
   if (event->type() == QEvent::ParentChange) {
      d->updateWindow();
   }
}

/*!
    \reimp
*/
void QWSEmbedWidget::resizeEvent(QResizeEvent *)
{
   Q_D(QWSEmbedWidget);
   d->resize(rect().size());
}

/*!
    \reimp
*/
void QWSEmbedWidget::moveEvent(QMoveEvent *)
{
   resizeEvent(0);
}

/*!
    \reimp
*/
void QWSEmbedWidget::hideEvent(QHideEvent *)
{
   Q_D(QWSEmbedWidget);
   d->resize(QSize());
}

/*!
    \reimp
*/
void QWSEmbedWidget::showEvent(QShowEvent *)
{
   Q_D(QWSEmbedWidget);
   d->resize(rect().size());
}

QT_END_NAMESPACE

#endif // QT_NO_QWSEMBEDWIDGET
