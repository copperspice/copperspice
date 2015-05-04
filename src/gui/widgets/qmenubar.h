/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QMENUBAR_H
#define QMENUBAR_H

#include <QtGui/qmenu.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_MENUBAR

class QMenuBarPrivate;
class QStyleOptionMenuItem;
class QWindowsStyle;

class Q_GUI_EXPORT QMenuBar : public QWidget
{
   CS_OBJECT(QMenuBar)

   GUI_CS_PROPERTY_READ(defaultUp, isDefaultUp)
   GUI_CS_PROPERTY_WRITE(defaultUp, setDefaultUp)
   GUI_CS_PROPERTY_READ(nativeMenuBar, isNativeMenuBar)
   GUI_CS_PROPERTY_WRITE(nativeMenuBar, setNativeMenuBar)

 public:
   explicit QMenuBar(QWidget *parent = 0);
   ~QMenuBar();

   using QWidget::addAction;

   QAction *addAction(const QString &text);
   QAction *addAction(const QString &text, const QObject *receiver, const char *member);

   QAction *addMenu(QMenu *menu);
   QMenu *addMenu(const QString &title);
   QMenu *addMenu(const QIcon &icon, const QString &title);


   QAction *addSeparator();
   QAction *insertSeparator(QAction *before);

   QAction *insertMenu(QAction *before, QMenu *menu);

   void clear();

   QAction *activeAction() const;
   void setActiveAction(QAction *action);

   void setDefaultUp(bool);
   bool isDefaultUp() const;

   QSize sizeHint() const;
   QSize minimumSizeHint() const;
   int heightForWidth(int) const;

   QRect actionGeometry(QAction *) const;
   QAction *actionAt(const QPoint &) const;

   void setCornerWidget(QWidget *w, Qt::Corner corner = Qt::TopRightCorner);
   QWidget *cornerWidget(Qt::Corner corner = Qt::TopRightCorner) const;

#ifdef Q_OS_MAC
   OSMenuRef macMenu();
   static bool macUpdateMenuBar();
#endif

   bool isNativeMenuBar() const;
   void setNativeMenuBar(bool nativeMenuBar);

   GUI_CS_SLOT_1(Public, virtual void setVisible(bool visible))
   GUI_CS_SLOT_2(setVisible)

   GUI_CS_SIGNAL_1(Public, void triggered(QAction *action))
   GUI_CS_SIGNAL_2(triggered, action)
   GUI_CS_SIGNAL_1(Public, void hovered(QAction *action))
   GUI_CS_SIGNAL_2(hovered, action)

 protected:
   void changeEvent(QEvent *);
   void keyPressEvent(QKeyEvent *);
   void mouseReleaseEvent(QMouseEvent *);
   void mousePressEvent(QMouseEvent *);
   void mouseMoveEvent(QMouseEvent *);
   void leaveEvent(QEvent *);
   void paintEvent(QPaintEvent *);
   void resizeEvent(QResizeEvent *);
   void actionEvent(QActionEvent *);
   void focusOutEvent(QFocusEvent *);
   void focusInEvent(QFocusEvent *);
   void timerEvent(QTimerEvent *);
   bool eventFilter(QObject *, QEvent *);
   bool event(QEvent *);
   void initStyleOption(QStyleOptionMenuItem *option, const QAction *action) const;

 private:
   Q_DECLARE_PRIVATE(QMenuBar)
   Q_DISABLE_COPY(QMenuBar)

   GUI_CS_SLOT_1(Private, void _q_actionTriggered())
   GUI_CS_SLOT_2(_q_actionTriggered)

   GUI_CS_SLOT_1(Private, void _q_actionHovered())
   GUI_CS_SLOT_2(_q_actionHovered)

   GUI_CS_SLOT_1(Private, void _q_internalShortcutActivated(int un_named_arg1))
   GUI_CS_SLOT_2(_q_internalShortcutActivated)

   GUI_CS_SLOT_1(Private, void _q_updateLayout())
   GUI_CS_SLOT_2(_q_updateLayout)

   friend class QMenu;
   friend class QMenuPrivate;
   friend class QWindowsStyle;

#ifdef Q_OS_MAC
   friend class QApplicationPrivate;
   friend class QWidgetPrivate;
   friend bool qt_mac_activate_action(MenuRef, uint, QAction::ActionEvent, bool);
#endif

};

#endif // QT_NO_MENUBAR

QT_END_NAMESPACE

#endif // QMENUBAR_H
