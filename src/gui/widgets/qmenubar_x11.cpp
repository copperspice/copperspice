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

#include <qmenubar_x11_p.h>

#ifndef QT_NO_MENUBAR

#include <qapplication.h>
#include <qdebug.h>
#include <qevent.h>
#include <qmenu.h>
#include <qmenubar.h>
#include <qfactoryloader_p.h>

QT_BEGIN_NAMESPACE

QX11MenuBar::~QX11MenuBar()
{
}

void QX11MenuBar::init(QMenuBar *_menuBar)
{
   nativeMenuBar = -1;
   menuBar = _menuBar;
}

void QX11MenuBar::setVisible(bool visible)
{
   menuBar->QWidget::setVisible(visible);
}

void QX11MenuBar::actionEvent(QActionEvent *e)
{
   Q_UNUSED(e);
}

void QX11MenuBar::handleReparent(QWidget *oldParent, QWidget *newParent, QWidget *oldWindow, QWidget *newWindow)
{
   Q_UNUSED(oldParent)
   Q_UNUSED(newParent)
   Q_UNUSED(oldWindow)
   Q_UNUSED(newWindow)
}

bool QX11MenuBar::allowCornerWidgets() const
{
   return true;
}

void QX11MenuBar::popupAction(QAction *)
{
}

void QX11MenuBar::setNativeMenuBar(bool value)
{
   if (nativeMenuBar == -1 || (value != bool(nativeMenuBar))) {
      nativeMenuBar = value;
   }
}

bool QX11MenuBar::isNativeMenuBar() const
{
   return false;
}

bool QX11MenuBar::shortcutsHandledByNativeMenuBar() const
{
   return false;
}

bool QX11MenuBar::menuBarEventFilter(QObject *, QEvent *)
{
   return false;
}

struct QX11MenuBarFactory : public QPlatformMenuBarFactoryInterface {
   QAbstractPlatformMenuBar *create() override {
      return new QX11MenuBar;
   }

   QStringList keys() const override {
      return QStringList();
   }
};

QPlatformMenuBarFactoryInterface *qt_guiPlatformMenuBarFactory()
{
   static QPlatformMenuBarFactoryInterface *factory = 0;
   if (!factory) {

      QFactoryLoader loader(QPlatformMenuBarFactoryInterface_iid, QLatin1String("/menubar"));
      factory = qobject_cast<QPlatformMenuBarFactoryInterface *>(loader.instance(QLatin1String("default")));

      if (!factory) {
         static QX11MenuBarFactory def;
         factory = &def;
      }
   }
   return factory;
}

QT_END_NAMESPACE

#endif // QT_NO_MENUBAR
