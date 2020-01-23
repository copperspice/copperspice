/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QMENU_H
#define QMENU_H

#include <qwidget.h>
#include <qstring.h>
#include <qicon.h>
#include <qaction.h>

#ifdef Q_OS_DARWIN

#if defined(__OBJC__)
@class NSMenu;
#else
using NSMenu = objc_object;
#endif

#endif

#ifndef QT_NO_MENU

class QMenuPrivate;
class QStyleOptionMenuItem;
class QPlatformMenu;

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
   QAction *addAction(const QIcon &icon,   const QString &text);
   QAction *addAction(const QString &text, const QObject *receiver, const QString &member, const QKeySequence &shortcut = 0);
   QAction *addAction(const QIcon &icon,   const QString &text, const QObject *receiver, const QString &member,
      const QKeySequence &shortcut = 0);

   // connect to a slot or function pointer (with context)
   template<class Obj, typename Func1>
   typename std::enable_if < ! std::is_convertible<Func1, QString>::value &&
         ! std::is_convertible<Func1, const char *>::value &&
         std::is_base_of<QObject, Obj>::value, QAction * >::type
      addAction(const QString &text, const Obj *object, Func1 slot, const QKeySequence &shortcut = 0) {
      QAction *result = addAction(text);

#ifndef QT_NO_SHORTCUT
      result->setShortcut(shortcut);
#endif
      connect(result, &QAction::triggered, object, slot);
      return result;
   }

   // connect to a slot or function pointer (without context)
   template <typename Func1>
   typename std::enable_if < ! std::is_convertible<Func1, QObject *>::value, QAction * >::type
      addAction(const QString &text, Func1 slot, const QKeySequence &shortcut = 0) {
      QAction *result = addAction(text);

#ifndef QT_NO_SHORTCUT
      result->setShortcut(shortcut);
#endif
      connect(result, &QAction::triggered, slot);
      return result;
   }

   // addAction(QIcon, QString): Connect to a QObject slot / functor or function pointer (with context)
   template<class Obj, typename Func1>
   typename std::enable_if < ! std::is_convertible<Func1, QString>::value
   &&std::is_base_of<QObject, Obj>::value, QAction * >::type addAction(const QIcon &actionIcon, const QString &text,
      const Obj *object, Func1 slot, const QKeySequence &shortcut = 0) {
      QAction *result = addAction(actionIcon, text);

#ifndef QT_NO_SHORTCUT
      result->setShortcut(shortcut);
#endif
      connect(result, &QAction::triggered, object, slot);
      return result;
   }

   // addAction(QIcon, QString): Connect to a functor or function pointer (without context)
   template <typename Func1>
   QAction *addAction(const QIcon &actionIcon, const QString &text, Func1 slot, const QKeySequence &shortcut = 0) {
      QAction *result = addAction(actionIcon, text);

#ifndef QT_NO_SHORTCUT

      result->setShortcut(shortcut);
#endif
      connect(result, &QAction::triggered, slot);
      return result;
   }

   QAction *addMenu(QMenu *menu);
   QMenu *addMenu(const QString &title);
   QMenu *addMenu(const QIcon &icon, const QString &title);

   QAction *addSeparator();

   QAction *addSection(const QString &text);
   QAction *addSection(const QIcon &icon, const QString &text);
   QAction *insertMenu(QAction *before, QMenu *menu);
   QAction *insertSeparator(QAction *before);

   QAction *insertSection(QAction *before, const QString &text);
   QAction *insertSection(QAction *before, const QIcon &icon, const QString &text);

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
   QPlatformMenu *platformMenu();
   void setPlatformMenu(QPlatformMenu *platformMenu);

#ifdef Q_OS_DARWIN
   NSMenu *toNSMenu();
   void setAsDockMenu();
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

 private:
   Q_DISABLE_COPY(QMenu)

   GUI_CS_SLOT_1(Private, void internalDelayedPopup())
   GUI_CS_SLOT_2(internalDelayedPopup)

   GUI_CS_SLOT_1(Private, void _q_actionTriggered())
   GUI_CS_SLOT_2(_q_actionTriggered)

   GUI_CS_SLOT_1(Private, void _q_actionHovered())
   GUI_CS_SLOT_2(_q_actionHovered)

   GUI_CS_SLOT_1(Private, void _q_overrideMenuActionDestroyed())
   GUI_CS_SLOT_2(_q_overrideMenuActionDestroyed)

   GUI_CS_SLOT_1(Private, void _q_platformMenuAboutToShow())
   GUI_CS_SLOT_2(_q_platformMenuAboutToShow)

   friend class QMenuBar;
   friend class QMenuBarPrivate;
   friend class QTornOffMenu;
   friend class QComboBox;
   friend class QAction;
   friend class QToolButtonPrivate;

   friend void qt_mac_emit_menuSignals(QMenu *menu, bool show);
   friend void qt_mac_menu_emit_hovered(QMenu *menu, QAction *action);
};

#endif // QT_NO_MENU

QT_END_NAMESPACE

#endif // QMENU_H
