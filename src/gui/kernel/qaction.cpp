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

#include <qaction.h>
#include <qactiongroup.h>

#ifndef QT_NO_ACTION
#include <qaction_p.h>
#include <qapplication.h>
#include <qevent.h>
#include <qlist.h>

#include <qdebug_p.h>
#include <qshortcutmap_p.h>
#include <qapplication_p.h>
#include <qmenu_p.h>

#define QAPP_CHECK(functionName) \
    if (! qApp) { \
        qWarning("QAction::%s() QApplication must be created before calling this method", functionName ); \
        return; \
    }

/*
  internal: guesses a descriptive text from a text suited for a menu entry
 */
static QString qt_strippedText(QString s)
{
   s.remove("...");

   int i = 0;
   while (i < s.size()) {
      ++i;
      if (s.at(i - 1) != '&') {
         continue;
      }

      if (i < s.size() && s.at(i) == '&') {
         ++i;
      }

      s.remove(i - 1, 1);
   }

   return s.trimmed();
}

QActionPrivate::QActionPrivate()
   : group(nullptr), enabled(1), forceDisabled(0), visible(1), forceInvisible(0), checkable(0),
     checked(0), separator(0), fontSet(false), iconVisibleInMenu(-1),
     menuRole(QAction::TextHeuristicRole),  priority(QAction::NormalPriority)
{

#ifndef QT_NO_SHORTCUT
   shortcutId = 0;
   shortcutContext = Qt::WindowShortcut;
   autorepeat = true;
#endif
}

QActionPrivate::~QActionPrivate()
{
}

bool QActionPrivate::showStatusText(QWidget *widget, const QString &str)
{
   Q_Q(QAction);

#ifndef QT_NO_STATUSTIP
   if (QObject *object = widget ? widget : q->parent()) {
      QStatusTipEvent tip(str);
      QApplication::sendEvent(object, &tip);
      return true;
   }
#endif

   return false;
}

void QActionPrivate::sendDataChanged()
{
   Q_Q(QAction);

   QActionEvent e(QEvent::ActionChanged, q);
   for (int i = 0; i < widgets.size(); ++i) {
      QWidget *w = widgets.at(i);
      QApplication::sendEvent(w, &e);
   }

#ifndef QT_NO_GRAPHICSVIEW
   for (int i = 0; i < graphicsWidgets.size(); ++i) {
      QGraphicsWidget *w = graphicsWidgets.at(i);
      QApplication::sendEvent(w, &e);
   }
#endif

   QApplication::sendEvent(q, &e);

   emit q->changed();
}

#ifndef QT_NO_SHORTCUT
void QActionPrivate::redoGrab(QShortcutMap &map)
{
   Q_Q(QAction);
   if (shortcutId) {
      map.removeShortcut(shortcutId, q);
   }
   if (shortcut.isEmpty()) {
      return;
   }

   shortcutId = map.addShortcut(q, shortcut, shortcutContext, qWidgetShortcutContextMatcher);

   if (!enabled) {
      map.setShortcutEnabled(false, shortcutId, q);
   }
   if (!autorepeat) {
      map.setShortcutAutoRepeat(false, shortcutId, q);
   }
}

void QActionPrivate::redoGrabAlternate(QShortcutMap &map)
{
   Q_Q(QAction);

   for (int i = 0; i < alternateShortcutIds.count(); ++i) {
      if (const int id = alternateShortcutIds.at(i)) {
         map.removeShortcut(id, q);
      }
   }

   alternateShortcutIds.clear();
   if (alternateShortcuts.isEmpty()) {
      return;
   }

   for (int i = 0; i < alternateShortcuts.count(); ++i) {
      const QKeySequence &alternate = alternateShortcuts.at(i);

      if (! alternate.isEmpty()) {
         alternateShortcutIds.append(map.addShortcut(q, alternate, shortcutContext, qWidgetShortcutContextMatcher));
      } else {
         alternateShortcutIds.append(0);
      }
   }
   if (!enabled) {
      for (int i = 0; i < alternateShortcutIds.count(); ++i) {
         const int id = alternateShortcutIds.at(i);
         map.setShortcutEnabled(false, id, q);
      }
   }
   if (!autorepeat) {
      for (int i = 0; i < alternateShortcutIds.count(); ++i) {
         const int id = alternateShortcutIds.at(i);
         map.setShortcutAutoRepeat(false, id, q);
      }
   }
}

void QActionPrivate::setShortcutEnabled(bool enable, QShortcutMap &map)
{
   Q_Q(QAction);
   if (shortcutId) {
      map.setShortcutEnabled(enable, shortcutId, q);
   }
   for (int i = 0; i < alternateShortcutIds.count(); ++i) {
      if (const int id = alternateShortcutIds.at(i)) {
         map.setShortcutEnabled(enable, id, q);
      }
   }
}
#endif // QT_NO_SHORTCUT


QAction::QAction(QObject *parent)
   : QObject(parent), d_ptr(new QActionPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QAction);

   d->group = qobject_cast<QActionGroup *>(parent);
   if (d->group) {
      d->group->addAction(this);
   }
}

QAction::QAction(const QString &text, QObject *parent)
   : QObject(parent), d_ptr(new QActionPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QAction);

   d->text = text;
   d->group = qobject_cast<QActionGroup *>(parent);
   if (d->group) {
      d->group->addAction(this);
   }
}

QAction::QAction(const QIcon &icon, const QString &text, QObject *parent)
   : QObject(parent), d_ptr(new QActionPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QAction);

   d->icon = icon;
   d->text = text;
   d->group = qobject_cast<QActionGroup *>(parent);
   if (d->group) {
      d->group->addAction(this);
   }
}

// internal
QAction::QAction(QActionPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   Q_D(QAction);

   d->group = qobject_cast<QActionGroup *>(parent);
   if (d->group) {
      d->group->addAction(this);
   }
}

QWidget *QAction::parentWidget() const
{
   QObject *ret = parent();
   while (ret && !ret->isWidgetType()) {
      ret = ret->parent();
   }
   return (QWidget *)ret;
}

QList<QWidget *> QAction::associatedWidgets() const
{
   Q_D(const QAction);
   return d->widgets;
}

#ifndef QT_NO_GRAPHICSVIEW
QList<QGraphicsWidget *> QAction::associatedGraphicsWidgets() const
{
   Q_D(const QAction);
   return d->graphicsWidgets;
}
#endif

#ifndef QT_NO_SHORTCUT

void QAction::setShortcut(const QKeySequence &shortcut)
{
   QAPP_CHECK("setShortcut");

   Q_D(QAction);
   if (d->shortcut == shortcut) {
      return;
   }

   d->shortcut = shortcut;
   d->redoGrab(qApp->d_func()->shortcutMap);
   d->sendDataChanged();
}

void QAction::setShortcuts(const QList<QKeySequence> &shortcuts)
{
   Q_D(QAction);

   QList <QKeySequence> listCopy = shortcuts;

   QKeySequence primary;
   if (!listCopy.isEmpty()) {
      primary = listCopy.takeFirst();
   }

   if (d->shortcut == primary && d->alternateShortcuts == listCopy) {
      return;
   }

   QAPP_CHECK("setShortcuts");

   d->shortcut = primary;
   d->alternateShortcuts = listCopy;
   d->redoGrab(qApp->d_func()->shortcutMap);
   d->redoGrabAlternate(qApp->d_func()->shortcutMap);
   d->sendDataChanged();
}

void QAction::setShortcuts(QKeySequence::StandardKey key)
{
   QList <QKeySequence> list = QKeySequence::keyBindings(key);
   setShortcuts(list);
}

QKeySequence QAction::shortcut() const
{
   Q_D(const QAction);
   return d->shortcut;
}

QList<QKeySequence> QAction::shortcuts() const
{
   Q_D(const QAction);
   QList <QKeySequence> shortcuts;

   if (!d->shortcut.isEmpty()) {
      shortcuts << d->shortcut;
   }

   if (!d->alternateShortcuts.isEmpty()) {
      shortcuts << d->alternateShortcuts;
   }

   return shortcuts;
}

void QAction::setShortcutContext(Qt::ShortcutContext context)
{
   Q_D(QAction);

   if (d->shortcutContext == context) {
      return;
   }

   QAPP_CHECK("setShortcutContext");

   d->shortcutContext = context;
   d->redoGrab(qApp->d_func()->shortcutMap);
   d->redoGrabAlternate(qApp->d_func()->shortcutMap);
   d->sendDataChanged();
}

Qt::ShortcutContext QAction::shortcutContext() const
{
   Q_D(const QAction);
   return d->shortcutContext;
}

void QAction::setAutoRepeat(bool on)
{
   Q_D(QAction);

   if (d->autorepeat == on) {
      return;
   }

   QAPP_CHECK("setAutoRepeat");

   d->autorepeat = on;
   d->redoGrab(qApp->d_func()->shortcutMap);
   d->redoGrabAlternate(qApp->d_func()->shortcutMap);
   d->sendDataChanged();
}

bool QAction::autoRepeat() const
{
   Q_D(const QAction);
   return d->autorepeat;
}
#endif // QT_NO_SHORTCUT

void QAction::setFont(const QFont &font)
{
   Q_D(QAction);

   if (d->font == font) {
      return;
   }

   d->fontSet = true;
   d->font = font;
   d->sendDataChanged();
}

QFont QAction::font() const
{
   Q_D(const QAction);
   return d->font;
}

QAction::~QAction()
{
   Q_D(QAction);
   for (int i = d->widgets.size() - 1; i >= 0; --i) {
      QWidget *w = d->widgets.at(i);
      w->removeAction(this);
   }

#ifndef QT_NO_GRAPHICSVIEW
   for (int i = d->graphicsWidgets.size() - 1; i >= 0; --i) {
      QGraphicsWidget *w = d->graphicsWidgets.at(i);
      w->removeAction(this);
   }
#endif

   if (d->group) {
      d->group->removeAction(this);
   }

#ifndef QT_NO_SHORTCUT
   if (d->shortcutId && qApp) {
      qApp->d_func()->shortcutMap.removeShortcut(d->shortcutId, this);
      for (int i = 0; i < d->alternateShortcutIds.count(); ++i) {
         const int id = d->alternateShortcutIds.at(i);
         qApp->d_func()->shortcutMap.removeShortcut(id, this);
      }
   }
#endif
}

void QAction::setActionGroup(QActionGroup *group)
{
   Q_D(QAction);

   if (group == d->group) {
      return;
   }

   if (d->group) {
      d->group->removeAction(this);
   }

   d->group = group;

   if (group) {
      group->addAction(this);
   }
}

QActionGroup *QAction::actionGroup() const
{
   Q_D(const QAction);
   return d->group;
}

void QAction::setIcon(const QIcon &icon)
{
   Q_D(QAction);
   d->icon = icon;
   d->sendDataChanged();
}

QIcon QAction::icon() const
{
   Q_D(const QAction);
   return d->icon;
}

#ifndef QT_NO_MENU
QMenu *QAction::menu() const
{
   Q_D(const QAction);
   return d->menu;
}

void QAction::setMenu(QMenu *menu)
{
   Q_D(QAction);
   if (d->menu) {
      d->menu->d_func()->setOverrideMenuAction(nullptr);   //we reset the default action of any previous menu
   }
   d->menu = menu;
   if (menu) {
      menu->d_func()->setOverrideMenuAction(this);
   }
   d->sendDataChanged();
}
#endif

void QAction::setSeparator(bool b)
{
   Q_D(QAction);

   if (d->separator == b) {
      return;
   }

   d->separator = b;
   d->sendDataChanged();
}

bool QAction::isSeparator() const
{
   Q_D(const QAction);
   return d->separator;
}

void QAction::setText(const QString &text)
{
   Q_D(QAction);
   if (d->text == text) {
      return;
   }

   d->text = text;
   d->sendDataChanged();
}

QString QAction::text() const
{
   Q_D(const QAction);

   QString s = d->text;
   if (s.isEmpty()) {
      s = d->iconText;
      s.replace(QLatin1Char('&'), QString("&&"));
   }
   return s;
}

void QAction::setIconText(const QString &text)
{
   Q_D(QAction);
   if (d->iconText == text) {
      return;
   }

   d->iconText = text;
   d->sendDataChanged();
}

QString QAction::iconText() const
{
   Q_D(const QAction);
   if (d->iconText.isEmpty()) {
      return qt_strippedText(d->text);
   }
   return d->iconText;
}

void QAction::setToolTip(const QString &tooltip)
{
   Q_D(QAction);
   if (d->tooltip == tooltip) {
      return;
   }

   d->tooltip = tooltip;
   d->sendDataChanged();
}

QString QAction::toolTip() const
{
   Q_D(const QAction);
   if (d->tooltip.isEmpty()) {
      if (!d->text.isEmpty()) {
         return qt_strippedText(d->text);
      }
      return qt_strippedText(d->iconText);
   }
   return d->tooltip;
}

void QAction::setStatusTip(const QString &statustip)
{
   Q_D(QAction);
   if (d->statustip == statustip) {
      return;
   }

   d->statustip = statustip;
   d->sendDataChanged();
}

QString QAction::statusTip() const
{
   Q_D(const QAction);
   return d->statustip;
}

void QAction::setWhatsThis(const QString &whatsthis)
{
   Q_D(QAction);
   if (d->whatsthis == whatsthis) {
      return;
   }

   d->whatsthis = whatsthis;
   d->sendDataChanged();
}

QString QAction::whatsThis() const
{
   Q_D(const QAction);
   return d->whatsthis;
}

void QAction::setPriority(Priority priority)
{
   Q_D(QAction);
   if (d->priority == priority) {
      return;
   }

   d->priority = priority;
   d->sendDataChanged();
}

QAction::Priority QAction::priority() const
{
   Q_D(const QAction);
   return d->priority;
}

void QAction::setCheckable(bool b)
{
   Q_D(QAction);

   if (d->checkable == b) {
      return;
   }

   d->checkable = b;
   d->checked = false;
   d->sendDataChanged();
}

bool QAction::isCheckable() const
{
   Q_D(const QAction);
   return d->checkable;
}

void QAction::toggle()
{
   Q_D(QAction);
   setChecked(!d->checked);
}

void QAction::setChecked(bool b)
{
   Q_D(QAction);

   if (!d->checkable || d->checked == b) {
      return;
   }

   QPointer<QAction> guard(this);
   d->checked = b;
   d->sendDataChanged();

   if (guard) {
      emit toggled(b);
   }
}

bool QAction::isChecked() const
{
   Q_D(const QAction);
   return d->checked;
}

void QAction::setEnabled(bool b)
{
   Q_D(QAction);
   if (b == d->enabled && b != d->forceDisabled) {
      return;
   }
   d->forceDisabled = !b;
   if (b && (!d->visible || (d->group && !d->group->isEnabled()))) {
      return;
   }
   QAPP_CHECK("setEnabled");
   d->enabled = b;

#ifndef QT_NO_SHORTCUT
   d->setShortcutEnabled(b, qApp->d_func()->shortcutMap);
#endif

   d->sendDataChanged();
}

bool QAction::isEnabled() const
{
   Q_D(const QAction);
   return d->enabled;
}

void QAction::setVisible(bool b)
{
   Q_D(QAction);
   if (b == d->visible && b != d->forceInvisible) {
      return;
   }
   QAPP_CHECK("setVisible");
   d->forceInvisible = !b;
   d->visible = b;
   d->enabled = b && !d->forceDisabled && (!d->group || d->group->isEnabled());

#ifndef QT_NO_SHORTCUT
   d->setShortcutEnabled(d->enabled, qApp->d_func()->shortcutMap);
#endif

   d->sendDataChanged();
}

bool QAction::isVisible() const
{
   Q_D(const QAction);
   return d->visible;
}

bool QAction::event(QEvent *e)
{
#ifndef QT_NO_SHORTCUT
   if (e->type() == QEvent::Shortcut) {
      QShortcutEvent *se = static_cast<QShortcutEvent *>(e);

      Q_ASSERT_X(se->key() == d_func()->shortcut || d_func()->alternateShortcuts.contains(se->key()),
         "QAction::event", "Received shortcut event from incorrect shortcut");

      if (se->isAmbiguous()) {
         qWarning("QAction::event() Current shortcut is an ambiguous overload: %s",
            se->key().toString(QKeySequence::NativeText).toLatin1().constData());

      } else {
         activate(Trigger);
      }

      return true;
   }
#endif

   return QObject::event(e);
}

QVariant QAction::data() const
{
   Q_D(const QAction);
   return d->userData;
}

void QAction::setData(const QVariant &data)
{
   Q_D(QAction);
   d->userData = data;
   d->sendDataChanged();
}

bool QAction::showStatusText(QWidget *widget)
{
   return d_func()->showStatusText(widget, statusTip());
}

void QAction::activate(ActionEvent event)
{
   Q_D(QAction);

   if (event == Trigger) {
      QPointer<QObject> guard = this;

      if (d->checkable) {
         // the checked action of an exclusive group cannot be  unchecked
         if (d->checked && (d->group && d->group->isExclusive()
               && d->group->checkedAction() == this)) {
            if (!guard.isNull()) {
               emit triggered(true);
            }
            return;
         }
         setChecked(!d->checked);
      }

      if (!guard.isNull()) {
         emit triggered(d->checked);
      }
   } else if (event == Hover) {
      emit hovered();
   }
}

void QAction::setMenuRole(MenuRole menuRole)
{
   Q_D(QAction);
   if (d->menuRole == menuRole) {
      return;
   }

   d->menuRole = menuRole;
   d->sendDataChanged();
}

QAction::MenuRole QAction::menuRole() const
{
   Q_D(const QAction);
   return d->menuRole;
}

void QAction::setIconVisibleInMenu(bool visible)
{
   Q_D(QAction);
   if (d->iconVisibleInMenu == -1 || visible != bool(d->iconVisibleInMenu)) {
      int oldValue = d->iconVisibleInMenu;
      d->iconVisibleInMenu = visible;
      // Only send data changed if we really need to.
      if (oldValue != -1
         || (oldValue == -1
            && visible == !QApplication::instance()->testAttribute(Qt::AA_DontShowIconsInMenus))) {
         d->sendDataChanged();
      }
   }
}

bool QAction::isIconVisibleInMenu() const
{
   Q_D(const QAction);
   if (d->iconVisibleInMenu == -1) {
      return !QApplication::instance()->testAttribute(Qt::AA_DontShowIconsInMenus);
   }
   return d->iconVisibleInMenu;
}

Q_GUI_EXPORT QDebug operator<<(QDebug d, const QAction *action)
{
   QDebugStateSaver saver(d);
   d.nospace();
   d << "QAction(" << static_cast<const void *>(action);

   if (action) {
      d << " text=" << action->text();
      if (!action->toolTip().isEmpty()) {
         d << " toolTip=" << action->toolTip();
      }

      if (action->isCheckable()) {
         d << " checked=" << action->isChecked();
      }
      if (!action->shortcut().isEmpty()) {
         d << " shortcut=" << action->shortcut();
      }

      d << " menuRole=";
      QtDebugUtils::formatQEnum(d, action->menuRole());
      d << " visible=" << action->isVisible();
   } else {

      d << '0';
   }

   d << ')';

   return d;
}

#endif
