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

#ifndef QACTION_H
#define QACTION_H

#include <qkeysequence.h>
#include <qstring.h>
#include <qwidget.h>
#include <qvariant.h>
#include <qicon.h>
#include <qscopedpointer.h>

#ifndef QT_NO_ACTION

class QMenu;
class QActionGroup;
class QActionPrivate;
class QGraphicsWidget;

class Q_GUI_EXPORT QAction : public QObject
{
   GUI_CS_OBJECT(QAction)

   GUI_CS_ENUM(MenuRole)
   GUI_CS_ENUM(Priority)

   GUI_CS_PROPERTY_READ(checkable, isCheckable)
   GUI_CS_PROPERTY_WRITE(checkable, setCheckable)
   GUI_CS_PROPERTY_NOTIFY(checkable, changed)

   GUI_CS_PROPERTY_READ(checked, isChecked)
   GUI_CS_PROPERTY_WRITE(checked, setChecked)
   GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(checked, isCheckable())
   GUI_CS_PROPERTY_NOTIFY(checked, toggled)

   GUI_CS_PROPERTY_READ(enabled, isEnabled)
   GUI_CS_PROPERTY_WRITE(enabled, setEnabled)
   GUI_CS_PROPERTY_NOTIFY(enabled, changed)

   GUI_CS_PROPERTY_READ(icon, icon)
   GUI_CS_PROPERTY_WRITE(icon, setIcon)
   GUI_CS_PROPERTY_NOTIFY(icon, changed)

   GUI_CS_PROPERTY_READ(text, text)
   GUI_CS_PROPERTY_WRITE(text, setText)
   GUI_CS_PROPERTY_NOTIFY(text, changed)

   GUI_CS_PROPERTY_READ(iconText, iconText)
   GUI_CS_PROPERTY_WRITE(iconText, setIconText)
   GUI_CS_PROPERTY_NOTIFY(iconText, changed)

   GUI_CS_PROPERTY_READ(toolTip, toolTip)
   GUI_CS_PROPERTY_WRITE(toolTip, setToolTip)
   GUI_CS_PROPERTY_NOTIFY(toolTip, changed)

   GUI_CS_PROPERTY_READ(statusTip, statusTip)
   GUI_CS_PROPERTY_WRITE(statusTip, setStatusTip)
   GUI_CS_PROPERTY_NOTIFY(statusTip, changed)

   GUI_CS_PROPERTY_READ(whatsThis, whatsThis)
   GUI_CS_PROPERTY_WRITE(whatsThis, setWhatsThis)
   GUI_CS_PROPERTY_NOTIFY(whatsThis, changed)

   GUI_CS_PROPERTY_READ(font, font)
   GUI_CS_PROPERTY_WRITE(font, setFont)
   GUI_CS_PROPERTY_NOTIFY(font, changed)

#ifndef QT_NO_SHORTCUT
   GUI_CS_PROPERTY_READ(shortcut, shortcut)
   GUI_CS_PROPERTY_WRITE(shortcut, setShortcut)
   GUI_CS_PROPERTY_NOTIFY(shortcut, changed)

   GUI_CS_PROPERTY_READ(shortcutContext, shortcutContext)
   GUI_CS_PROPERTY_WRITE(shortcutContext, setShortcutContext)
   GUI_CS_PROPERTY_NOTIFY(shortcutContext, changed)

   GUI_CS_PROPERTY_READ(autoRepeat, autoRepeat)
   GUI_CS_PROPERTY_WRITE(autoRepeat, setAutoRepeat)
   GUI_CS_PROPERTY_NOTIFY(autoRepeat, changed)
#endif

   GUI_CS_PROPERTY_READ(visible, isVisible)
   GUI_CS_PROPERTY_WRITE(visible, setVisible)
   GUI_CS_PROPERTY_NOTIFY(visible, changed)

   GUI_CS_PROPERTY_READ(menuRole, menuRole)
   GUI_CS_PROPERTY_WRITE(menuRole, setMenuRole)
   GUI_CS_PROPERTY_NOTIFY(menuRole, changed)

   GUI_CS_PROPERTY_READ(iconVisibleInMenu, isIconVisibleInMenu)
   GUI_CS_PROPERTY_WRITE(iconVisibleInMenu, setIconVisibleInMenu)
   GUI_CS_PROPERTY_NOTIFY(iconVisibleInMenu, changed)

   GUI_CS_PROPERTY_READ(priority, priority)
   GUI_CS_PROPERTY_WRITE(priority, setPriority)

 public:
   GUI_CS_REGISTER_ENUM(
      enum MenuRole {
         NoRole = 0,
         TextHeuristicRole,
         ApplicationSpecificRole,
         AboutCsRole,
         AboutRole,
         PreferencesRole,
         QuitRole
      };
   )

   enum Priority {
      LowPriority    = 0,
      NormalPriority = 128,
      HighPriority   = 256
   };

   enum ActionEvent {
      Trigger,
      Hover
   };

   explicit QAction(QObject *parent);
   QAction(const QString &text, QObject *parent);
   QAction(const QIcon &icon, const QString &text, QObject *parent);

   QAction(const QAction &) = delete;
   QAction &operator=(const QAction &) = delete;

   ~QAction();

   void setActionGroup(QActionGroup *group);
   QActionGroup *actionGroup() const;
   void setIcon(const QIcon &icon);
   QIcon icon() const;

   void setText(const QString &text);
   QString text() const;

   void setIconText(const QString &text);
   QString iconText() const;

   void setToolTip(const QString &tooltip);
   QString toolTip() const;

   void setStatusTip(const QString &statusTip);
   QString statusTip() const;

   void setWhatsThis(const QString &what);
   QString whatsThis() const;

   void setPriority(Priority priority);
   Priority priority() const;

#ifndef QT_NO_MENU
   QMenu *menu() const;
   void setMenu(QMenu *menu);
#endif

   void setSeparator(bool separator);
   bool isSeparator() const;

#ifndef QT_NO_SHORTCUT
   void setShortcut(const QKeySequence &shortcut);
   QKeySequence shortcut() const;

   void setShortcuts(const QList<QKeySequence> &shortcuts);
   void setShortcuts(QKeySequence::StandardKey key);
   QList<QKeySequence> shortcuts() const;

   void setShortcutContext(Qt::ShortcutContext context);
   Qt::ShortcutContext shortcutContext() const;

   void setAutoRepeat(bool repeat);
   bool autoRepeat() const;
#endif

   void setFont(const QFont &font);
   QFont font() const;

   void setCheckable(bool isChecked);
   bool isCheckable() const;

   QVariant data() const;
   void setData(const QVariant &data);

   bool isChecked() const;
   bool isEnabled() const;
   bool isVisible() const;

   void activate(ActionEvent event);
   bool showStatusText(QWidget *widget = nullptr);

   void setMenuRole(MenuRole menuRole);
   MenuRole menuRole() const;

   void setIconVisibleInMenu(bool visible);
   bool isIconVisibleInMenu() const;

   QWidget *parentWidget() const;

   QList<QWidget *> associatedWidgets() const;

#ifndef QT_NO_GRAPHICSVIEW
   QList<QGraphicsWidget *> associatedGraphicsWidgets() const; // ### suboptimal
#endif

   GUI_CS_SLOT_1(Public, void trigger())
   GUI_CS_SLOT_2(trigger)

   GUI_CS_SLOT_1(Public, void hover())
   GUI_CS_SLOT_2(hover)

   GUI_CS_SLOT_1(Public, void setChecked(bool isChecked))
   GUI_CS_SLOT_2(setChecked)

   GUI_CS_SLOT_1(Public, void toggle())
   GUI_CS_SLOT_2(toggle)

   GUI_CS_SLOT_1(Public, void setEnabled(bool enabled))
   GUI_CS_SLOT_2(setEnabled)

   GUI_CS_SLOT_1(Public, void setDisabled(bool disabled))
   GUI_CS_SLOT_2(setDisabled)

   GUI_CS_SLOT_1(Public, void setVisible(bool visible))
   GUI_CS_SLOT_2(setVisible)

   GUI_CS_SIGNAL_1(Public, void changed())
   GUI_CS_SIGNAL_2(changed)

   GUI_CS_SIGNAL_1(Public, void triggered(bool isChecked = false))
   GUI_CS_SIGNAL_2(triggered, isChecked)

   GUI_CS_SIGNAL_1(Public, void hovered())
   GUI_CS_SIGNAL_2(hovered)

   GUI_CS_SIGNAL_1(Public, void toggled(bool isChecked))
   GUI_CS_SIGNAL_2(toggled, isChecked)

 protected:
   bool event(QEvent *event) override;
   QAction(QActionPrivate &dd, QObject *parent);

   QScopedPointer<QActionPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QAction)

   friend class QGraphicsWidget;
   friend class QWidget;
   friend class QActionGroup;
   friend class QMenu;
   friend class QMenuPrivate;
   friend class QMenuBar;
   friend class QToolButton;

#ifdef Q_OS_DARWIN
   friend void qt_mac_clear_status_text(QAction *action);
#endif

};

Q_GUI_EXPORT QDebug operator<<(QDebug, const QAction *);

inline void QAction::trigger() {
   activate(ActionEvent::Trigger);
}

inline void QAction::hover() {
   activate(ActionEvent::Hover);
}

inline void QAction::setDisabled(bool disabled) {
   setEnabled(! disabled);
}

// recursive, leave here
#include <qactiongroup.h>

#endif // QT_NO_ACTION

#endif
