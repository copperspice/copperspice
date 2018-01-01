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

#ifndef QABSTRACTPLATFORMMENUBAR_P_H
#define QABSTRACTPLATFORMMENUBAR_P_H

#include <qfactoryinterface.h>
#include <qglobal.h>
#include <qplugin.h>

#ifndef QT_NO_MENUBAR

QT_BEGIN_NAMESPACE

class QAction;
class QActionEvent;
class QEvent;
class QMenuBar;
class QObject;
class QWidget;

class QAbstractPlatformMenuBar;

struct QPlatformMenuBarFactoryInterface : public QFactoryInterface {
   virtual QAbstractPlatformMenuBar *create() = 0;
};

#define QPlatformMenuBarFactoryInterface_iid "com.copperspice.QPlatformMenuBarFactoryInterface"
CS_DECLARE_INTERFACE(QPlatformMenuBarFactoryInterface, QPlatformMenuBarFactoryInterface_iid)

class QAbstractPlatformMenuBar
{
 public:
   virtual ~QAbstractPlatformMenuBar() {}

   virtual void init(QMenuBar *) = 0;

   virtual void setVisible(bool visible) = 0;

   virtual void actionEvent(QActionEvent *) = 0;

   virtual void handleReparent(QWidget *oldParent, QWidget *newParent, QWidget *oldWindow, QWidget *newWindow) = 0;

   virtual bool allowCornerWidgets() const = 0;

   virtual void popupAction(QAction *) = 0;

   virtual void setNativeMenuBar(bool) = 0;

   virtual bool isNativeMenuBar() const = 0;

   // Return true if the native menubar is capable of listening to the
   // shortcut keys. If false is returned, QMenuBar will trigger actions on shortcut itself.
   virtual bool shortcutsHandledByNativeMenuBar() const = 0;

   virtual bool menuBarEventFilter(QObject *, QEvent *event) = 0;
};

QT_END_NAMESPACE

#endif // QT_NO_MENUBAR

#endif // QABSTRACTPLATFORMMENUBAR_P_H
