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

#ifndef QWSEMBEDWIDGET_H
#define QWSEMBEDWIDGET_H

#include <QtGui/qwidget.h>

#ifndef QT_NO_QWSEMBEDWIDGET

QT_BEGIN_NAMESPACE

class QWSEmbedWidgetPrivate;

class Q_GUI_EXPORT QWSEmbedWidget : public QWidget
{
   GUI_CS_OBJECT(QWSEmbedWidget)

 public:
   QWSEmbedWidget(WId winId, QWidget *parent = nullptr);
   ~QWSEmbedWidget();

 protected:
   bool eventFilter(QObject *object, QEvent *event);
   void changeEvent(QEvent *event);
   void resizeEvent(QResizeEvent *event);
   void moveEvent(QMoveEvent *event);
   void hideEvent(QHideEvent *event);
   void showEvent(QShowEvent *event);

 private:
   Q_DECLARE_PRIVATE(QWSEmbedWidget)
};

QT_END_NAMESPACE


#endif // QT_NO_QWSEMBEDWIDGET

#endif // QWSEMBEDWIDGET_H
