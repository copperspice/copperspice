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

#ifndef QWINDOWCONTAINER_H
#define QWINDOWCONTAINER_H

#include <qwidget.h>

class QWindowContainerPrivate;

class Q_GUI_EXPORT QWindowContainer : public QWidget
{
   GUI_CS_OBJECT(QWindowContainer)
   Q_DECLARE_PRIVATE(QWindowContainer)

 public:
   explicit QWindowContainer(QWindow *embeddedWindow, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);
   ~QWindowContainer();
   QWindow *containedWindow() const;

   static void toplevelAboutToBeDestroyed(QWidget *parent);
   static void parentWasChanged(QWidget *parent);
   static void parentWasMoved(QWidget *parent);
   static void parentWasRaised(QWidget *parent);
   static void parentWasLowered(QWidget *parent);

 protected:
   bool event(QEvent *event) override;

 private :
   GUI_CS_SLOT_1(Private, void focusWindowChanged(QWindow *focusWindow))
   GUI_CS_SLOT_2(focusWindowChanged)
};

#endif
