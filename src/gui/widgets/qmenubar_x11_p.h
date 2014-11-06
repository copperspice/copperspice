/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QMENUBAR_X11_P_H
#define QMENUBAR_X11_P_H

#ifndef QT_NO_MENUBAR

#include <qabstractplatformmenubar_p.h>

QT_BEGIN_NAMESPACE

class QMenuBar;

class QX11MenuBar : public QAbstractPlatformMenuBar
{
 public:
   ~QX11MenuBar();

   virtual void init(QMenuBar *);

   virtual void setVisible(bool visible);

   virtual void actionEvent(QActionEvent *e);

   virtual void handleReparent(QWidget *oldParent, QWidget *newParent, QWidget *oldWindow, QWidget *newWindow);

   virtual bool allowCornerWidgets() const;

   virtual void popupAction(QAction *);

   virtual void setNativeMenuBar(bool);
   virtual bool isNativeMenuBar() const;

   virtual bool shortcutsHandledByNativeMenuBar() const;
   virtual bool menuBarEventFilter(QObject *, QEvent *event);

 private:
   QMenuBar *menuBar;
   int nativeMenuBar : 3;  // Only has values -1, 0, and 1
};

QPlatformMenuBarFactoryInterface *qt_guiPlatformMenuBarFactory();

QT_END_NAMESPACE

#endif // QT_NO_MENUBAR

#endif /* QX11MENUBAR_P_H */
