/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#include <qapplication.h>
#include <qevent.h>
#include <qbitmap.h>
#include <qsoftkeymanager_p.h>
#include <qaction_p.h>
#include <qsoftkeymanager_common_p.h>

#ifndef QT_NO_SOFTKEYMANAGER
QT_BEGIN_NAMESPACE

QScopedPointer<QSoftKeyManager> QSoftKeyManagerPrivate::self(0);

QString QSoftKeyManager::standardSoftKeyText(StandardSoftKey standardKey)
{
   QString softKeyText;
   switch (standardKey) {
      case OkSoftKey:
         softKeyText = QSoftKeyManager::tr("OK");
         break;
      case SelectSoftKey:
         softKeyText = QSoftKeyManager::tr("Select");
         break;
      case DoneSoftKey:
         softKeyText = QSoftKeyManager::tr("Done");
         break;
      case MenuSoftKey:
         softKeyText = QSoftKeyManager::tr("Options");
         break;
      case CancelSoftKey:
         softKeyText = QSoftKeyManager::tr("Cancel");
         break;
      default:
         break;
   };

   return softKeyText;
}

QSoftKeyManager *QSoftKeyManager::instance()
{
   if (!QSoftKeyManagerPrivate::self) {
      QSoftKeyManagerPrivate::self.reset(new QSoftKeyManager);
   }

   return QSoftKeyManagerPrivate::self.data();
}

QSoftKeyManager::QSoftKeyManager()
   : QObject(0), d_ptr(new QSoftKeyManagerPrivate)
{
   d_ptr->q_ptr = this;
}

QAction *QSoftKeyManager::createAction(StandardSoftKey standardKey, QWidget *actionWidget)
{
   QAction *action = new QAction(standardSoftKeyText(standardKey), actionWidget);

   QAction::SoftKeyRole softKeyRole = QAction::NoSoftKey;
   switch (standardKey) {
      case MenuSoftKey: // FALL-THROUGH
         QActionPrivate::get(action)->menuActionSoftkeys = true;
      case OkSoftKey:
      case SelectSoftKey:
      case DoneSoftKey:
         softKeyRole = QAction::PositiveSoftKey;
         break;
      case CancelSoftKey:
         softKeyRole = QAction::NegativeSoftKey;
         break;
   }
   action->setSoftKeyRole(softKeyRole);
   action->setVisible(false);
   setForceEnabledInSoftkeys(action);
   return action;
}

/*! \internal

  Creates a QAction and registers the 'triggered' signal to send the given key event to
  \a actionWidget as a convenience.

*/
QAction *QSoftKeyManager::createKeyedAction(StandardSoftKey standardKey, Qt::Key key, QWidget *actionWidget)
{
#ifndef QT_NO_ACTION
   QScopedPointer<QAction> action(createAction(standardKey, actionWidget));

   connect(action.data(), SIGNAL(triggered()), QSoftKeyManager::instance(), SLOT(sendKeyEvent()));
   connect(action.data(), SIGNAL(destroyed(QObject *)), QSoftKeyManager::instance(), SLOT(cleanupHash(QObject *)));

   QSoftKeyManager::instance()->d_func()->keyedActions.insert(action.data(), key);
   return action.take();
#endif //QT_NO_ACTION
}

void QSoftKeyManager::cleanupHash(QObject *obj)
{
   Q_D(QSoftKeyManager);
   // Can't use qobject_cast in destroyed() signal handler as that'll return NULL,
   // so use static_cast instead. Since the pointer is only used as a hash key, it is safe.
   QAction *action = static_cast<QAction *>(obj);
   d->keyedActions.remove(action);

}

void QSoftKeyManager::sendKeyEvent()
{
   Q_D(QSoftKeyManager);
   QAction *action = qobject_cast<QAction *>(sender());

   if (!action) {
      return;
   }

   Qt::Key keyToSend = d->keyedActions.value(action, Qt::Key_unknown);

   if (keyToSend != Qt::Key_unknown)
      QApplication::postEvent(action->parentWidget(),
                              new QKeyEvent(QEvent::KeyPress, keyToSend, Qt::NoModifier));
}

void QSoftKeyManager::updateSoftKeys()
{
   QSoftKeyManager::instance()->d_func()->pendingUpdate = true;
   QEvent *event = new QEvent(QEvent::UpdateSoftKeys);
   QApplication::postEvent(QSoftKeyManager::instance(), event);
}

bool QSoftKeyManager::appendSoftkeys(const QWidget &source, int level)
{
   Q_D(QSoftKeyManager);
   bool ret = false;

   for (QAction * action : source.actions()) {
      if (action->softKeyRole() != QAction::NoSoftKey && (action->isVisible() || isForceEnabledInSofkeys(action))) {
         d->requestedSoftKeyActions.insert(level, action);
         ret = true;
      }
   }
   return ret;
}


static bool isChildOf(const QWidget *c, const QWidget *p)
{
   while (c) {
      if (c == p) {
         return true;
      }
      c = c->parentWidget();
   }
   return false;
}

QWidget *QSoftKeyManager::softkeySource(QWidget *previousSource, bool &recursiveMerging)
{
   Q_D(QSoftKeyManager);
   QWidget *source = NULL;
   if (!previousSource) {
      // Initial source is primarily focuswidget and secondarily activeWindow
      QWidget *focus = QApplication::focusWidget();
      QWidget *popup = QApplication::activePopupWidget();
      if (popup) {
         if (isChildOf(focus, popup)) {
            source = focus;
         } else {
            source = popup;
         }
      }
      if (!source) {
         QWidget *modal = QApplication::activeModalWidget();
         if (modal) {
            if (isChildOf(focus, modal)) {
               source = focus;
            } else {
               source = modal;
            }
         }
      }
      if (!source) {
         source = focus;
         if (!source) {
            source = QApplication::activeWindow();
         }
      }
   } else {
      // Softkey merging is based on four criterias
      // 1. Implicit merging is used whenever focus widget does not specify any softkeys
      bool implicitMerging = d->requestedSoftKeyActions.isEmpty();
      // 2. Explicit merging with parent is used whenever WA_MergeSoftkeys widget attribute is set
      bool explicitMerging = previousSource->testAttribute(Qt::WA_MergeSoftkeys);
      // 3. Explicit merging with all parents
      recursiveMerging |= previousSource->testAttribute(Qt::WA_MergeSoftkeysRecursively);
      // 4. Implicit and explicit merging always stops at window boundary
      bool merging = (implicitMerging || explicitMerging || recursiveMerging) && !previousSource->isWindow();

      source = merging ? previousSource->parentWidget() : NULL;
   }
   return source;
}

bool QSoftKeyManager::handleUpdateSoftKeys()
{
   Q_D(QSoftKeyManager);
   int level = 0;
   d->requestedSoftKeyActions.clear();
   bool recursiveMerging = false;
   QWidget *source = softkeySource(NULL, recursiveMerging);
   d->initialSoftKeySource = source;
   while (source) {
      if (appendSoftkeys(*source, level)) {
         ++level;
      }
      source = softkeySource(source, recursiveMerging);
   }

   d->updateSoftKeys_sys();
   d->pendingUpdate = false;
   return true;
}

void QSoftKeyManager::setForceEnabledInSoftkeys(QAction *action)
{
   QActionPrivate::get(action)->forceEnabledInSoftkeys = true;
}

bool QSoftKeyManager::isForceEnabledInSofkeys(QAction *action)
{
   return QActionPrivate::get(action)->forceEnabledInSoftkeys;
}

bool QSoftKeyManager::event(QEvent *e)
{
#ifndef QT_NO_ACTION
   if (e->type() == QEvent::UpdateSoftKeys) {
      return handleUpdateSoftKeys();
   }
#endif //QT_NO_ACTION
   return false;
}

QT_END_NAMESPACE
#endif //QT_NO_SOFTKEYMANAGER
