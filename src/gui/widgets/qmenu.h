/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QMENU_H
#define QMENU_H

#include <QtGui/qwidget.h>
#include <QtCore/qstring.h>
#include <QtGui/qicon.h>
#include <QtGui/qaction.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_MENU

class QMenuPrivate;
class QStyleOptionMenuItem;

class Q_GUI_EXPORT QMenu : public QWidget
{
   GUI_CS_OBJECT(QMenu)
   Q_DECLARE_PRIVATE(QMenu)

   GUI_CS_PROPERTY_READ(tearOffEnabled, isTearOffEnabled)
   GUI_CS_PROPERTY_WRITE(tearOffEnabled, setTearOffEnabled)
   GUI_CS_PROPERTY_READ(title, title)
   GUI_CS_PROPERTY_WRITE(title, setTitle)
   GUI_CS_PROPERTY_READ(icon, icon)
   GUI_CS_PROPERTY_WRITE(icon, setIcon)
   GUI_CS_PROPERTY_READ(separatorsCollapsible, separatorsCollapsible)
   GUI_CS_PROPERTY_WRITE(separatorsCollapsible, setSeparatorsCollapsible)
   GUI_CS_PROPERTY_READ(toolTipsVisible, toolTipsVisible)
   GUI_CS_PROPERTY_WRITE(toolTipsVisible, setToolTipsVisible)

 public:
   explicit QMenu(QWidget *parent = nullptr);
   explicit QMenu(const QString &title, QWidget *parent = nullptr);
   ~QMenu();

   using QWidget::addAction;

   QAction *addAction(const QString &text);
   QAction *addAction(const QIcon &icon, const QString &text);
   QAction *addAction(const QString &text, const QObject *receiver,
                      const char *member, const QKeySequence &shortcut = 0);
   QAction *addAction(const QIcon &icon, const QString &text,
                      const QObject *receiver, const char *member, const QKeySequence &shortcut = 0);

   QAction *addMenu(QMenu *menu);
   QMenu *addMenu(const QString &title);
   QMenu *addMenu(const QIcon &icon, const QString &title);

   QAction *addSeparator();

   QAction *insertMenu(QAction *before, QMenu *menu);
   QAction *insertSeparator(QAction *before);

   bool isEmpty() const;
   void clear();

   void setTearOffEnabled(bool);
   bool isTearOffEnabled() const;

   bool isTearOffMenuVisible() const;
   void hideTearOffMenu();

   void setDefaultAction(QAction *);
   QAction *defaultAction() const;

   void setActiveAction(QAction *act);
   QAction *activeAction() const;

   void popup(const QPoint &pos, QAction *at = nullptr);

   QAction *exec();
   QAction *exec(const QPoint &pos, QAction *at = nullptr);
   static QAction *exec(const QList<QAction *> &actions, const QPoint &pos, QAction *at = nullptr, QWidget *parent = nullptr);

   QSize sizeHint() const override;

   QRect actionGeometry(QAction *) const;
   QAction *actionAt(const QPoint &) const;

   QAction *menuAction() const;

   QString title() const;
   void setTitle(const QString &title);

   QIcon icon() const;
   void setIcon(const QIcon &icon);

   void setNoReplayFor(QWidget *widget);

#ifdef Q_OS_MAC
   OSMenuRef macMenu(OSMenuRef merge = 0);
#endif

   bool separatorsCollapsible() const;
   void setSeparatorsCollapsible(bool collapse);

   bool toolTipsVisible() const;
   void setToolTipsVisible(bool visible);

   GUI_CS_SIGNAL_1(Public, void aboutToShow())
   GUI_CS_SIGNAL_2(aboutToShow)
   GUI_CS_SIGNAL_1(Public, void aboutToHide())
   GUI_CS_SIGNAL_2(aboutToHide)
   GUI_CS_SIGNAL_1(Public, void triggered(QAction *action))
   GUI_CS_SIGNAL_2(triggered, action)
   GUI_CS_SIGNAL_1(Public, void hovered(QAction *action))
   GUI_CS_SIGNAL_2(hovered, action)

 protected:
   int columnCount() const;

   void changeEvent(QEvent *) override;
   void keyPressEvent(QKeyEvent *) override;
   void mouseReleaseEvent(QMouseEvent *) override;
   void mousePressEvent(QMouseEvent *) override;
   void mouseMoveEvent(QMouseEvent *) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *) override;
#endif

   void enterEvent(QEvent *) override;
   void leaveEvent(QEvent *) override;
   void hideEvent(QHideEvent *) override;
   void paintEvent(QPaintEvent *) override;
   void actionEvent(QActionEvent *) override;
   void timerEvent(QTimerEvent *) override;
   bool event(QEvent *) override;
   bool focusNextPrevChild(bool next) override;
   void initStyleOption(QStyleOptionMenuItem *option, const QAction *action) const;

   QMenu(QMenuPrivate &dd, QWidget *parent = nullptr);

 private :
   GUI_CS_SLOT_1(Private, void internalSetSloppyAction())
   GUI_CS_SLOT_2(internalSetSloppyAction)
   GUI_CS_SLOT_1(Private, void internalDelayedPopup())
   GUI_CS_SLOT_2(internalDelayedPopup)

   GUI_CS_SLOT_1(Private, void _q_actionTriggered())
   GUI_CS_SLOT_2(_q_actionTriggered)

   GUI_CS_SLOT_1(Private, void _q_actionHovered())
   GUI_CS_SLOT_2(_q_actionHovered)

   GUI_CS_SLOT_1(Private, void _q_overrideMenuActionDestroyed())
   GUI_CS_SLOT_2(_q_overrideMenuActionDestroyed)

   Q_DISABLE_COPY(QMenu)

   friend class QMenuBar;
   friend class QMenuBarPrivate;
   friend class QTornOffMenu;
   friend class QComboBox;
   friend class QAction;
   friend class QToolButtonPrivate;

#ifdef Q_OS_MAC
   friend void qt_mac_trayicon_activate_action(QMenu *, QAction *action);
   friend bool qt_mac_watchingAboutToShow(QMenu *);
   friend OSStatus qt_mac_menu_event(EventHandlerCallRef, EventRef, void *);
   friend bool qt_mac_activate_action(OSMenuRef, uint, QAction::ActionEvent, bool);
   friend void qt_mac_emit_menuSignals(QMenu *, bool);
   friend void qt_mac_menu_emit_hovered(QMenu *menu, QAction *action);
#endif
};

#endif // QT_NO_MENU

QT_END_NAMESPACE

#endif // QMENU_H
