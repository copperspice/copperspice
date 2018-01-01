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

#ifndef QWORKSPACE_H
#define QWORKSPACE_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_WORKSPACE

class QAction;
class QWorkspaceChild;
class QShowEvent;
class QWorkspacePrivate;

class Q_GUI_EXPORT QWorkspace : public QWidget
{
   GUI_CS_OBJECT(QWorkspace)

   GUI_CS_PROPERTY_READ(scrollBarsEnabled, scrollBarsEnabled)
   GUI_CS_PROPERTY_WRITE(scrollBarsEnabled, setScrollBarsEnabled)
   GUI_CS_PROPERTY_READ(background, background)
   GUI_CS_PROPERTY_WRITE(background, setBackground)

 public:
   explicit QWorkspace(QWidget *parent = nullptr);
   ~QWorkspace();

   enum WindowOrder { CreationOrder, StackingOrder };

   QWidget *activeWindow() const;
   QWidgetList windowList(WindowOrder order = CreationOrder) const;

   QWidget *addWindow(QWidget *w, Qt::WindowFlags flags = 0);

   QSize sizeHint() const override;

   bool scrollBarsEnabled() const;
   void setScrollBarsEnabled(bool enable);

   void setBackground(const QBrush &background);
   QBrush background() const;

   GUI_CS_SIGNAL_1(Public, void windowActivated(QWidget *w))
   GUI_CS_SIGNAL_2(windowActivated, w)

   GUI_CS_SLOT_1(Public, void setActiveWindow(QWidget *w))
   GUI_CS_SLOT_2(setActiveWindow)
   GUI_CS_SLOT_1(Public, void cascade())
   GUI_CS_SLOT_2(cascade)
   GUI_CS_SLOT_1(Public, void tile())
   GUI_CS_SLOT_2(tile)
   GUI_CS_SLOT_1(Public, void arrangeIcons())
   GUI_CS_SLOT_2(arrangeIcons)
   GUI_CS_SLOT_1(Public, void closeActiveWindow())
   GUI_CS_SLOT_2(closeActiveWindow)
   GUI_CS_SLOT_1(Public, void closeAllWindows())
   GUI_CS_SLOT_2(closeAllWindows)
   GUI_CS_SLOT_1(Public, void activateNextWindow())
   GUI_CS_SLOT_2(activateNextWindow)
   GUI_CS_SLOT_1(Public, void activatePreviousWindow())
   GUI_CS_SLOT_2(activatePreviousWindow)

 protected:
   bool event(QEvent *e) override;
   void paintEvent(QPaintEvent *e) override;
   void changeEvent(QEvent *) override;
   void childEvent(QChildEvent *) override;
   void resizeEvent(QResizeEvent *) override;
   bool eventFilter(QObject *, QEvent *) override;
   void showEvent(QShowEvent *e) override;
   void hideEvent(QHideEvent *e) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *e) override;
#endif

 private:
   Q_DECLARE_PRIVATE(QWorkspace)
   Q_DISABLE_COPY(QWorkspace)

   GUI_CS_SLOT_1(Private, void _q_normalizeActiveWindow())
   GUI_CS_SLOT_2(_q_normalizeActiveWindow)

   GUI_CS_SLOT_1(Private, void _q_minimizeActiveWindow())
   GUI_CS_SLOT_2(_q_minimizeActiveWindow)

   GUI_CS_SLOT_1(Private, void _q_showOperationMenu())
   GUI_CS_SLOT_2(_q_showOperationMenu)

   GUI_CS_SLOT_1(Private, void _q_popupOperationMenu(const QPoint &un_named_arg1))
   GUI_CS_SLOT_2(_q_popupOperationMenu)

   GUI_CS_SLOT_1(Private, void _q_operationMenuActivated(QAction *un_named_arg1))
   GUI_CS_SLOT_2(_q_operationMenuActivated)

   GUI_CS_SLOT_1(Private, void _q_updateActions())
   GUI_CS_SLOT_2(_q_updateActions)

   GUI_CS_SLOT_1(Private, void _q_scrollBarChanged())
   GUI_CS_SLOT_2(_q_scrollBarChanged)

   friend class QWorkspaceChild;
};

#endif // QT_NO_WORKSPACE

QT_END_NAMESPACE

#endif // QWORKSPACE_H
