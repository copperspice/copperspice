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

#ifndef QMDISUBWINDOW_H
#define QMDISUBWINDOW_H

#include <qwidget.h>

#ifndef QT_NO_MDIAREA

class QMenu;
class QMdiArea;

namespace QMdi {
class ControlContainer;
}

class QMdiSubWindowPrivate;

class Q_GUI_EXPORT QMdiSubWindow : public QWidget
{
   GUI_CS_OBJECT(QMdiSubWindow)

   GUI_CS_PROPERTY_READ(keyboardSingleStep, keyboardSingleStep)
   GUI_CS_PROPERTY_WRITE(keyboardSingleStep, setKeyboardSingleStep)

   GUI_CS_PROPERTY_READ(keyboardPageStep, keyboardPageStep)
   GUI_CS_PROPERTY_WRITE(keyboardPageStep, setKeyboardPageStep)

 public:
   enum SubWindowOption {
      AllowOutsideAreaHorizontally = 0x1,    // internal
      AllowOutsideAreaVertically = 0x2,      // internal
      RubberBandResize = 0x4,
      RubberBandMove = 0x8
   };
   using SubWindowOptions = QFlags<SubWindowOption>;

   QMdiSubWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::EmptyFlag);

   QMdiSubWindow(const  QMdiSubWindow &) = delete;
   QMdiSubWindow &operator=(const  QMdiSubWindow &) = delete;

   ~QMdiSubWindow();

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

   void setWidget(QWidget *widget);
   QWidget *widget() const;

   QWidget *maximizedButtonsWidget() const; // internal
   QWidget *maximizedSystemMenuIconWidget() const; // internal

   bool isShaded() const;

   void setOption(SubWindowOption option, bool on = true);
   bool testOption(SubWindowOption option) const;

   void setKeyboardSingleStep(int step);
   int keyboardSingleStep() const;

   void setKeyboardPageStep(int step);
   int keyboardPageStep() const;

#ifndef QT_NO_MENU
   void setSystemMenu(QMenu *systemMenu);
   QMenu *systemMenu() const;
#endif

   QMdiArea *mdiArea() const;

   GUI_CS_SIGNAL_1(Public, void windowStateChanged(Qt::WindowStates oldState, Qt::WindowStates newState))
   GUI_CS_SIGNAL_2(windowStateChanged, oldState, newState)
   GUI_CS_SIGNAL_1(Public, void aboutToActivate())
   GUI_CS_SIGNAL_2(aboutToActivate)

#ifndef QT_NO_MENU
   GUI_CS_SLOT_1(Public, void showSystemMenu())
   GUI_CS_SLOT_2(showSystemMenu)
#endif

   GUI_CS_SLOT_1(Public, void showShaded())
   GUI_CS_SLOT_2(showShaded)

 protected:
   bool eventFilter(QObject *object, QEvent *event) override;
   bool event(QEvent *event) override;
   void showEvent(QShowEvent *showEvent) override;
   void hideEvent(QHideEvent *hideEvent) override;
   void changeEvent(QEvent *changeEvent) override;
   void closeEvent(QCloseEvent *closeEvent) override;
   void leaveEvent(QEvent *leaveEvent) override;
   void resizeEvent(QResizeEvent *resizeEvent) override;
   void timerEvent(QTimerEvent *timerEvent) override;
   void moveEvent(QMoveEvent *moveEvent) override;
   void paintEvent(QPaintEvent *paintEvent) override;
   void mousePressEvent(QMouseEvent *mouseEvent) override;
   void mouseDoubleClickEvent(QMouseEvent *mouseEvent) override;
   void mouseReleaseEvent(QMouseEvent *mouseEvent) override;
   void mouseMoveEvent(QMouseEvent *mouseEvent) override;
   void keyPressEvent(QKeyEvent *keyEvent) override;

#ifndef QT_NO_CONTEXTMENU
   void contextMenuEvent(QContextMenuEvent *contextMenuEvent) override;
#endif

   void focusInEvent(QFocusEvent *focusInEvent) override;
   void focusOutEvent(QFocusEvent *focusOutEvent) override;
   void childEvent(QChildEvent *childEvent) override;

 private:
   Q_DECLARE_PRIVATE(QMdiSubWindow)

   GUI_CS_SLOT_1(Private, void _q_updateStaysOnTopHint())
   GUI_CS_SLOT_2(_q_updateStaysOnTopHint)

   GUI_CS_SLOT_1(Private, void _q_enterInteractiveMode())
   GUI_CS_SLOT_2(_q_enterInteractiveMode)

   GUI_CS_SLOT_1(Private, void _q_processFocusChanged(QWidget *oldWidget, QWidget *newWidget))
   GUI_CS_SLOT_2(_q_processFocusChanged)

   friend class QMdiAreaPrivate;
   friend class QMdi::ControlContainer;

#ifndef QT_NO_TABBAR
   friend class QMdiAreaTabBar;
#endif

};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMdiSubWindow::SubWindowOptions)

#endif // QT_NO_MDIAREA

#endif
