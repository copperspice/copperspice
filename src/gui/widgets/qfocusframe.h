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

#ifndef QFOCUSFRAME_H
#define QFOCUSFRAME_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

class QFocusFramePrivate;
class QStyleOption;

class Q_GUI_EXPORT QFocusFrame : public QWidget
{
   GUI_CS_OBJECT(QFocusFrame)

 public:
   QFocusFrame(QWidget *parent = nullptr);
   ~QFocusFrame();

   void setWidget(QWidget *widget);
   QWidget *widget() const;

 protected:
   bool event(QEvent *e) override;

   bool eventFilter(QObject *, QEvent *) override;
   void paintEvent(QPaintEvent *) override;
   void initStyleOption(QStyleOption *option) const;

 private:
   Q_DECLARE_PRIVATE(QFocusFrame)
   Q_DISABLE_COPY(QFocusFrame)
};

QT_END_NAMESPACE

#endif // QFOCUSFRAME_H
