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

#include <qaccessible.h>
#include <qaccessiblecache_p.h>

#include <qapplication.h>
#include <qaccessibleplugin.h>
#include <qaccessibleobject.h>
#include <qaccessiblebridge.h>
#include <qclipboard.h>
#include <qdebug.h>
#include <qhash.h>
#include <qmetaobject.h>
#include <qtextcursor.h>
#include <qtextboundaryfinder.h>
#include <qplatform_accessibility.h>
#include <qplatform_integration.h>

#include <qapplication_p.h>
#include <qfactoryloader_p.h>

QAccessibleInterface::~QAccessibleInterface()
{
}

#ifndef QT_NO_ACCESSIBILITY

using QAccessiblePluginsHash = QHash<QString, QAccessiblePlugin *>;

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QAccessibleInterface_ID, "/accessible"))

Q_GLOBAL_STATIC(QAccessiblePluginsHash, qAccessiblePlugins)
Q_GLOBAL_STATIC(QList<QAccessible::InterfaceFactory>, qAccessibleFactories)
Q_GLOBAL_STATIC(QList<QAccessible::ActivationObserver *>, qAccessibleActivationObservers)

QAccessible::UpdateHandler QAccessible::updateHandler = 0;
QAccessible::RootObjectHandler QAccessible::rootObjectHandler = 0;

static bool cleanupAdded = false;

static QPlatformAccessibility *platformAccessibility()
{
   QPlatformIntegration *pfIntegration = QGuiApplicationPrivate::platformIntegration();
   return pfIntegration ? pfIntegration->accessibility() : 0;
}

void QAccessible::cleanup()
{
   if (QPlatformAccessibility *pfAccessibility = platformAccessibility()) {
      pfAccessibility->cleanup();
   }
}

static void qAccessibleCleanup()
{
   qAccessibleActivationObservers()->clear();
   qAccessibleFactories()->clear();
}

void QAccessible::installFactory(InterfaceFactory factory)
{
   if (!factory) {
      return;
   }

   if (!cleanupAdded) {
      qAddPostRoutine(qAccessibleCleanup);
      cleanupAdded = true;
   }
   if (qAccessibleFactories()->contains(factory)) {
      return;
   }
   qAccessibleFactories()->append(factory);
}

void QAccessible::removeFactory(InterfaceFactory factory)
{
   qAccessibleFactories()->removeAll(factory);
}

/*!
    \internal

    Installs the given \a handler as the function to be used by
    updateAccessibility(), and returns the previously installed
    handler.
*/
QAccessible::UpdateHandler QAccessible::installUpdateHandler(UpdateHandler handler)
{
   UpdateHandler old = updateHandler;
   updateHandler = handler;
   return old;
}

QAccessible::RootObjectHandler QAccessible::installRootObjectHandler(RootObjectHandler handler)
{
   RootObjectHandler old = rootObjectHandler;
   rootObjectHandler = handler;
   return old;
}

QAccessible::ActivationObserver::~ActivationObserver()
{
}
void QAccessible::installActivationObserver(QAccessible::ActivationObserver *observer)
{
   if (!observer) {
      return;
   }
   if (!cleanupAdded) {
      qAddPostRoutine(qAccessibleCleanup);
      cleanupAdded = true;
   }

   if (qAccessibleActivationObservers()->contains(observer)) {
      return;
   }

   qAccessibleActivationObservers()->append(observer);
}

void QAccessible::removeActivationObserver(ActivationObserver *observer)
{
   qAccessibleActivationObservers()->removeAll(observer);
}

QAccessibleInterface *QAccessible::queryAccessibleInterface(QObject *object)
{
   if (! object) {
      return 0;
   }

   if (Id id = QAccessibleCache::instance()->objectToId.value(object)) {
      return QAccessibleCache::instance()->interfaceForId(id);
   }

   const QMetaObject *mo = object->metaObject();

   while (mo) {
      const QString cn(mo->className());

      for (int i = qAccessibleFactories()->count(); i > 0; --i) {
         InterfaceFactory factory = qAccessibleFactories()->at(i - 1);

         if (QAccessibleInterface *iface = factory(cn, object)) {
            QAccessibleCache::instance()->insert(object, iface);
            Q_ASSERT(QAccessibleCache::instance()->objectToId.contains(object));
            return iface;
         }
      }

      // Find a QAccessiblePlugin (factory) for the class name. If there's
      // no entry in the cache try to create it using the plugin loader.

      if (! qAccessiblePlugins()->contains(cn)) {
         QAccessiblePlugin *factory = 0;                // 0 means "no plugin found". This is cached as well

         auto keySet = loader()->keySet();

         if (keySet.contains(cn)) {
            factory = qobject_cast<QAccessiblePlugin *>(loader()->instance(cn));
         }

         qAccessiblePlugins()->insert(cn, factory);
      }

      // At this point the cache should contain a valid factory pointer or 0:
      Q_ASSERT(qAccessiblePlugins()->contains(cn));

      QAccessiblePlugin *factory = qAccessiblePlugins()->value(cn);

      if (factory) {
         QAccessibleInterface *result = factory->create(cn, object);

         if (result) {
            // Need this condition because of QDesktopScreenWidget
            QAccessibleCache::instance()->insert(object, result);
            Q_ASSERT(QAccessibleCache::instance()->objectToId.contains(object));
         }
         return result;
      }

      mo = mo->superClass();
   }


   if (object == qApp) {
      QAccessibleInterface *appInterface = new QAccessibleApplication;
      QAccessibleCache::instance()->insert(object, appInterface);
      Q_ASSERT(QAccessibleCache::instance()->objectToId.contains(qApp));
      return appInterface;
   }

   return 0;
}

QAccessible::Id QAccessible::registerAccessibleInterface(QAccessibleInterface *iface)
{
   Q_ASSERT(iface);
   return QAccessibleCache::instance()->insert(iface->object(), iface);
}

void QAccessible::deleteAccessibleInterface(Id id)
{
   QAccessibleCache::instance()->deleteInterface(id);
}

QAccessible::Id QAccessible::uniqueId(QAccessibleInterface *iface)
{
   Id id = QAccessibleCache::instance()->idForInterface(iface);
   if (!id) {
      id = registerAccessibleInterface(iface);
   }
   return id;
}
QAccessibleInterface *QAccessible::accessibleInterface(Id id)
{
   return QAccessibleCache::instance()->interfaceForId(id);
}

bool QAccessible::isActive()
{
   if (QPlatformAccessibility *pfAccessibility = platformAccessibility()) {
      return pfAccessibility->isActive();
   }

   return false;
}

void QAccessible::setActive(bool active)
{
   for (int i = 0; i < qAccessibleActivationObservers()->count() ; ++i) {
      qAccessibleActivationObservers()->at(i)->accessibilityActiveChanged(active);
   }
}



void QAccessible::setRootObject(QObject *object)
{
   if (rootObjectHandler) {
      rootObjectHandler(object);
      return;
   }

   if (QPlatformAccessibility *pfAccessibility = platformAccessibility()) {
      pfAccessibility->setRootObject(object);
   }
}
void QAccessible::updateAccessibility(QAccessibleEvent *event)
{
   // NOTE: Querying for the accessibleInterface below will result in
   // resolving and caching the interface, which in some cases will
   // cache the wrong information as updateAccessibility is called
   // during construction of widgets. If you see cases where the
   // cache seems wrong, this call is "to blame", but the code that
   // caches dynamic data should be updated to handle change events.
   QAccessibleInterface *iface = event->accessibleInterface();
   if (isActive() && iface) {
      if (event->type() == QAccessible::TableModelChanged) {
         if (iface->tableInterface()) {
            iface->tableInterface()->modelChange(static_cast<QAccessibleTableModelChangeEvent *>(event));
         }
      }

      if (updateHandler) {
         updateHandler(event);
         return;
      }
   }

   if (QPlatformAccessibility *pfAccessibility = platformAccessibility()) {
      pfAccessibility->notifyAccessibilityUpdate(event);
   }
}
/*!
    \internal
    \brief getBoundaries is a helper function to find the accessible text boundaries for QTextCursor based documents.
    \param documentCursor a valid cursor bound to the document (not null). It needs to ba at the position to look for the boundary
    \param boundaryType the type of boundary to find
    \return the boundaries as pair
*/
QPair< int, int > QAccessible::qAccessibleTextBoundaryHelper(const QTextCursor &offsetCursor, TextBoundaryType boundaryType)
{
   Q_ASSERT(!offsetCursor.isNull());

   QTextCursor endCursor = offsetCursor;
   endCursor.movePosition(QTextCursor::End);
   int characterCount = endCursor.position();

   QPair<int, int> result;
   QTextCursor cursor = offsetCursor;
   switch (boundaryType) {
      case CharBoundary:
         result.first = cursor.position();
         cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
         result.second = cursor.position();
         break;
      case WordBoundary:
         cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::MoveAnchor);
         result.first = cursor.position();
         cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
         result.second = cursor.position();
         break;
      case SentenceBoundary: {
         // QCursor does not provide functionality to move to next sentence.
         // We therefore find the current block, then go through the block using
         // QTextBoundaryFinder and find the sentence the \offset represents
         cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
         result.first = cursor.position();
         cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
         result.second = cursor.position();
         QString blockText = cursor.selectedText();
         const int offsetWithinBlockText = offsetCursor.position() - result.first;
         QTextBoundaryFinder sentenceFinder(QTextBoundaryFinder::Sentence, blockText);
         sentenceFinder.setPosition(offsetWithinBlockText);
         int prevBoundary = offsetWithinBlockText;
         int nextBoundary = offsetWithinBlockText;
         if (!(sentenceFinder.boundaryReasons() & QTextBoundaryFinder::StartOfItem)) {
            prevBoundary = sentenceFinder.toPreviousBoundary();
         }
         nextBoundary = sentenceFinder.toNextBoundary();
         if (nextBoundary != -1) {
            result.second = result.first + nextBoundary;
         }
         if (prevBoundary != -1) {
            result.first += prevBoundary;
         }
         break;
      }
      case LineBoundary:
         cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
         result.first = cursor.position();
         cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
         result.second = cursor.position();
         break;
      case ParagraphBoundary:
         cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
         result.first = cursor.position();
         cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
         result.second = cursor.position();
         break;
      case NoBoundary:
         result.first = 0;
         result.second = characterCount;
         break;
   }
   return result;
}
QVector<QPair<QAccessibleInterface *, QAccessible::Relation>> QAccessibleInterface::relations(
      QAccessible::Relation /*match = QAccessible::AllRelations*/) const
{
   return QVector<QPair<QAccessibleInterface *, QAccessible::Relation>>();
}

QAccessibleInterface *QAccessibleInterface::focusChild() const
{
   return 0;
}





QColor QAccessibleInterface::foregroundColor() const
{
   return QColor();
}
QColor QAccessibleInterface::backgroundColor() const
{
   return QColor();
}
QAccessibleEvent::~QAccessibleEvent()
{
}


QAccessible::Id QAccessibleEvent::uniqueId() const
{
   if (!m_object) {
      return m_uniqueId;
   }

   QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(m_object);

   if (!iface) {
      return 0;
   }

   if (m_child != -1) {
      iface = iface->child(m_child);
   }

   return QAccessible::uniqueId(iface);
}

QAccessibleValueChangeEvent::~QAccessibleValueChangeEvent()
{
}

QAccessibleStateChangeEvent::~QAccessibleStateChangeEvent()
{
}

QAccessibleTableModelChangeEvent::~QAccessibleTableModelChangeEvent()
{
}

QAccessibleTextCursorEvent::~QAccessibleTextCursorEvent()
{
}

QAccessibleTextInsertEvent::~QAccessibleTextInsertEvent()
{
}

QAccessibleTextRemoveEvent::~QAccessibleTextRemoveEvent()
{
}

QAccessibleTextUpdateEvent::~QAccessibleTextUpdateEvent()
{
}
QAccessibleTextSelectionEvent::~QAccessibleTextSelectionEvent()
{
}

QAccessibleInterface *QAccessibleEvent::accessibleInterface() const
{
   if (m_object == 0) {
      return QAccessible::accessibleInterface(m_uniqueId);
   }

   QAccessibleInterface *iface = QAccessible::queryAccessibleInterface(m_object);
   if (!iface || !iface->isValid()) {
      return 0;
   }

   if (m_child >= 0) {
      QAccessibleInterface *child = iface->child(m_child);
      if (child) {
         iface = child;
      } else {
         qWarning() << "Cannot creat accessible child interface for object: " << m_object << " index: " << m_child;
      }
   }
   return iface;
}

QWindow *QAccessibleInterface::window() const
{
   return 0;
}

void QAccessibleInterface::virtual_hook(int /*id*/, void * /*data*/)
{
}

QString qAccessibleRoleString(QAccessible::Role role)
{
   if (role >= QAccessible::UserRole) {
      role = QAccessible::UserRole;
   }

   static int roleEnum = QAccessible::staticMetaObject().indexOfEnumerator("Role");

   return QAccessible::staticMetaObject().enumerator(roleEnum).valueToKey(role);
}

QString qAccessibleEventString(QAccessible::Event event)
{
   static int eventEnum = QAccessible::staticMetaObject().indexOfEnumerator("Event");
   return QAccessible::staticMetaObject().enumerator(eventEnum).valueToKey(event);
}

bool operator==(const QAccessible::State &first, const QAccessible::State &second)
{
   return memcmp(&first, &second, sizeof(QAccessible::State)) == 0;
}

Q_GUI_EXPORT QDebug operator<<(QDebug d, const QAccessibleInterface *iface)
{
   QDebugStateSaver saver(d);

   if (! iface) {
      d << "QAccessibleInterface(null)";
      return d;
   }

   d.nospace();
   d << "QAccessibleInterface(" << hex << (const void *) iface << dec;

   if (iface->isValid()) {
      d << " name=" << iface->text(QAccessible::Name) << ' ';
      d << "role=" << qAccessibleRoleString(iface->role()) << ' ';
      if (iface->childCount()) {
         d << "childc=" << iface->childCount() << ' ';
      }
      if (iface->object()) {
         d << "obj=" << iface->object();
      }
      QStringList stateStrings;
      QAccessible::State st = iface->state();
      if (st.focusable) {
         stateStrings << QLatin1String("focusable");
      }
      if (st.focused) {
         stateStrings << QLatin1String("focused");
      }
      if (st.selected) {
         stateStrings << QLatin1String("selected");
      }
      if (st.invisible) {
         stateStrings << QLatin1String("invisible");
      }

      if (!stateStrings.isEmpty()) {
         d << stateStrings.join(QLatin1Char('|'));
      }

      if (!st.invisible) {
         d << "rect=" << iface->rect();
      }

   } else {
      d << " invalid";
   }
   d << ')';
   return d;
}

QDebug operator<<(QDebug d, const QAccessibleEvent &ev)
{
   QDebugStateSaver saver(d);
   d.nospace() << "QAccessibleEvent(";
   if (ev.object()) {
      d.nospace() << "object=" << hex << ev.object() << dec;
      d.nospace() << "child=" << ev.child();
   } else {
      d.nospace() << "no object, uniqueId=" << ev.uniqueId();
   }
   d << " event=" << qAccessibleEventString(ev.type());
   if (ev.type() == QAccessible::StateChanged) {
      QAccessible::State changed = static_cast<const QAccessibleStateChangeEvent *>(&ev)->changedStates();
      d << "State changed:";
      if (changed.disabled) {
         d << "disabled";
      }
      if (changed.selected) {
         d << "selected";
      }
      if (changed.focusable) {
         d << "focusable";
      }
      if (changed.focused) {
         d << "focused";
      }
      if (changed.pressed) {
         d << "pressed";
      }
      if (changed.checkable) {
         d << "checkable";
      }
      if (changed.checked) {
         d << "checked";
      }
      if (changed.checkStateMixed) {
         d << "checkStateMixed";
      }
      if (changed.readOnly) {
         d << "readOnly";
      }
      if (changed.hotTracked) {
         d << "hotTracked";
      }
      if (changed.defaultButton) {
         d << "defaultButton";
      }
      if (changed.expanded) {
         d << "expanded";
      }
      if (changed.collapsed) {
         d << "collapsed";
      }
      if (changed.busy) {
         d << "busy";
      }
      if (changed.expandable) {
         d << "expandable";
      }
      if (changed.marqueed) {
         d << "marqueed";
      }
      if (changed.animated) {
         d << "animated";
      }
      if (changed.invisible) {
         d << "invisible";
      }
      if (changed.offscreen) {
         d << "offscreen";
      }
      if (changed.sizeable) {
         d << "sizeable";
      }
      if (changed.movable) {
         d << "movable";
      }
      if (changed.selfVoicing) {
         d << "selfVoicing";
      }
      if (changed.selectable) {
         d << "selectable";
      }
      if (changed.linked) {
         d << "linked";
      }
      if (changed.traversed) {
         d << "traversed";
      }
      if (changed.multiSelectable) {
         d << "multiSelectable";
      }
      if (changed.extSelectable) {
         d << "extSelectable";
      }
      if (changed.passwordEdit) {
         d << "passwordEdit";   // used to be Protected
      }
      if (changed.hasPopup) {
         d << "hasPopup";
      }
      if (changed.modal) {
         d << "modal";
      }

      // IA2 - we chose to not add some IA2 states for now
      // Below the ones that seem helpful
      if (changed.active) {
         d << "active";
      }
      if (changed.invalid) {
         d << "invalid";   // = defuncts
      }
      if (changed.editable) {
         d << "editable";
      }
      if (changed.multiLine) {
         d << "multiLine";
      }
      if (changed.selectableText) {
         d << "selectableText";
      }
      if (changed.supportsAutoCompletion) {
         d << "supportsAutoCompletion";
      }

   }

   d << ')';
   return d;
}

QAccessibleTextInterface::~QAccessibleTextInterface()
{
}

static QString textLineBoundary(int beforeAtAfter, const QString &text, int offset, int *startOffset, int *endOffset)
{
   Q_ASSERT(beforeAtAfter >= -1 && beforeAtAfter <= 1);
   Q_ASSERT(*startOffset == -1 && *endOffset == -1);

   int length = text.length();
   Q_ASSERT(offset >= 0 && offset <= length);

   if (beforeAtAfter == 1) {
      offset = text.indexOf(QChar(QChar::LineFeed), qMin(offset, length - 1));

      if (offset < 0) {
         return QString();   // after the last line comes nothing
      }

      ++offset; // move after the newline

   } else if (beforeAtAfter == -1) {
      offset = text.lastIndexOf(QChar(QChar::LineFeed), qMax(offset - 1, 0));

      if (offset < 0) {
         return QString();   // before first line comes nothing
      }
   }

   if (offset > 0) {
      *startOffset = text.lastIndexOf(QChar(QChar::LineFeed), offset - 1);
   }

   ++*startOffset; // move to the char after the newline (0 if lastIndexOf returned -1)

   *endOffset = text.indexOf(QChar(QChar::LineFeed), qMin(offset, length - 1)) + 1; // include newline char

   if (*endOffset <= 0 || *endOffset > length) {
      *endOffset = length;   // if the text doesn't end with a newline it ends at length
   }

   return text.mid(*startOffset, *endOffset - *startOffset);
}
QString QAccessibleTextInterface::textBeforeOffset(int offset, QAccessible::TextBoundaryType boundaryType,
   int *startOffset, int *endOffset) const
{
   const QString txt = text(0, characterCount());

   if (offset == -1) {
      offset = txt.length();
   }

   *startOffset = *endOffset = -1;

   if (txt.isEmpty() || offset <= 0 || offset > txt.length()) {
      return QString();
   }

   QTextBoundaryFinder::BoundaryType type = QTextBoundaryFinder::Grapheme;
   switch (boundaryType) {
      case QAccessible::CharBoundary:
         type = QTextBoundaryFinder::Grapheme;
         break;
      case QAccessible::WordBoundary:
         type = QTextBoundaryFinder::Word;
         break;
      case QAccessible::SentenceBoundary:
         type = QTextBoundaryFinder::Sentence;
         break;
      case QAccessible::LineBoundary:
      case QAccessible::ParagraphBoundary:
         return textLineBoundary(-1, txt, offset, startOffset, endOffset);
      case QAccessible::NoBoundary:
         return QString();
      default:
         // error, may want to throw
         break;
   }

   // keep behavior in sync with QTextCursor::movePosition()!

   QTextBoundaryFinder boundary(type, txt);
   boundary.setPosition(offset);

   do {
      if ((boundary.boundaryReasons() & (QTextBoundaryFinder::StartOfItem | QTextBoundaryFinder::EndOfItem))) {
         break;
      }
   } while (boundary.toPreviousBoundary() > 0);
   Q_ASSERT(boundary.position() >= 0);
   *endOffset = boundary.position();

   while (boundary.toPreviousBoundary() > 0) {
      if ((boundary.boundaryReasons() & (QTextBoundaryFinder::StartOfItem | QTextBoundaryFinder::EndOfItem))) {
         break;
      }
   }
   Q_ASSERT(boundary.position() >= 0);
   *startOffset = boundary.position();

   return txt.mid(*startOffset, *endOffset - *startOffset);
}
QString QAccessibleTextInterface::textAfterOffset(int offset, QAccessible::TextBoundaryType boundaryType,
   int *startOffset, int *endOffset) const
{
   const QString txt = text(0, characterCount());

   if (offset == -1) {
      offset = txt.length();
   }

   *startOffset = *endOffset = -1;
   if (txt.isEmpty() || offset < 0 || offset >= txt.length()) {
      return QString();
   }

   // type initialized just to silence a compiler warning [-Werror=maybe-uninitialized]
   QTextBoundaryFinder::BoundaryType type = QTextBoundaryFinder::Grapheme;
   switch (boundaryType) {
      case QAccessible::CharBoundary:
         type = QTextBoundaryFinder::Grapheme;
         break;
      case QAccessible::WordBoundary:
         type = QTextBoundaryFinder::Word;
         break;
      case QAccessible::SentenceBoundary:
         type = QTextBoundaryFinder::Sentence;
         break;
      case QAccessible::LineBoundary:
      case QAccessible::ParagraphBoundary:
         // Lines can not use QTextBoundaryFinder since Line there means any potential line-break.
         return textLineBoundary(1, txt, offset, startOffset, endOffset);
      case QAccessible::NoBoundary:
         // return empty, this function currently only supports single lines, so there can be no line after
         return QString();
      default:
         // error, may want to throw
         break;
   }

   // keep behavior in sync with QTextCursor::movePosition()!

   QTextBoundaryFinder boundary(type, txt);
   boundary.setPosition(offset);

   while (true) {
      int toNext = boundary.toNextBoundary();
      if ((boundary.boundaryReasons() & (QTextBoundaryFinder::StartOfItem | QTextBoundaryFinder::EndOfItem))) {
         break;
      }
      if (toNext < 0 || toNext >= txt.length()) {
         break;   // not found, the boundary might not exist
      }
   }
   Q_ASSERT(boundary.position() <= txt.length());
   *startOffset = boundary.position();

   while (true) {
      int toNext = boundary.toNextBoundary();
      if ((boundary.boundaryReasons() & (QTextBoundaryFinder::StartOfItem | QTextBoundaryFinder::EndOfItem))) {
         break;
      }
      if (toNext < 0 || toNext >= txt.length()) {
         break;   // not found, the boundary might not exist
      }
   }
   Q_ASSERT(boundary.position() <= txt.length());
   *endOffset = boundary.position();

   if ((*startOffset == -1) || (*endOffset == -1) || (*startOffset == *endOffset)) {
      *endOffset = -1;
      *startOffset = -1;
   }

   return txt.mid(*startOffset, *endOffset - *startOffset);
}
QString QAccessibleTextInterface::textAtOffset(int offset, QAccessible::TextBoundaryType boundaryType,
   int *startOffset, int *endOffset) const
{
   const QString txt = text(0, characterCount());
   if (offset == -1) {
      offset = txt.length();
   }
   *startOffset = *endOffset = -1;
   if (txt.isEmpty() || offset < 0 || offset > txt.length()) {
      return QString();
   }
   if (offset == txt.length() && boundaryType == QAccessible::CharBoundary) {
      return QString();
   }
   QTextBoundaryFinder::BoundaryType type = QTextBoundaryFinder::Grapheme;
   switch (boundaryType) {
      case QAccessible::CharBoundary:
         type = QTextBoundaryFinder::Grapheme;
         break;
      case QAccessible::WordBoundary:
         type = QTextBoundaryFinder::Word;
         break;
      case QAccessible::SentenceBoundary:
         type = QTextBoundaryFinder::Sentence;
         break;
      case QAccessible::LineBoundary:
      case QAccessible::ParagraphBoundary:
         return textLineBoundary(0, txt, offset, startOffset, endOffset);
      case QAccessible::NoBoundary:
         *startOffset = 0;
         *endOffset = txt.length();
         return txt;
      default:
         // error, may want to throw
         break;
   }
   QTextBoundaryFinder boundary(type, txt);
   boundary.setPosition(offset);
   do {
      if ((boundary.boundaryReasons() & (QTextBoundaryFinder::StartOfItem | QTextBoundaryFinder::EndOfItem))) {
         break;
      }
   } while (boundary.toPreviousBoundary() > 0);
   Q_ASSERT(boundary.position() >= 0);
   *startOffset = boundary.position();
   while (boundary.toNextBoundary() < txt.length()) {
      if ((boundary.boundaryReasons() & (QTextBoundaryFinder::StartOfItem | QTextBoundaryFinder::EndOfItem))) {
         break;
      }
   }
   Q_ASSERT(boundary.position() <= txt.length());
   *endOffset = boundary.position();
   return txt.mid(*startOffset, *endOffset - *startOffset);
}
QAccessibleEditableTextInterface::~QAccessibleEditableTextInterface()
{
}
QAccessibleValueInterface::~QAccessibleValueInterface()
{
}
QAccessibleImageInterface::~QAccessibleImageInterface()
{
}

QAccessibleTableCellInterface::~QAccessibleTableCellInterface()
{
}
QAccessibleTableInterface::~QAccessibleTableInterface()
{
}
QAccessibleActionInterface::~QAccessibleActionInterface()
{
}
struct QAccessibleActionStrings {
   QAccessibleActionStrings() :
      pressAction(        QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Press")),
      increaseAction(     QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Increase")),
      decreaseAction(     QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Decrease")),
      showMenuAction(     QT_TRANSLATE_NOOP("QAccessibleActionInterface", "ShowMenu")),
      setFocusAction(     QT_TRANSLATE_NOOP("QAccessibleActionInterface", "SetFocus")),
      toggleAction(       QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Toggle")),
      scrollLeftAction(   QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Scroll Left")),
      scrollRightAction(  QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Scroll Right")),
      scrollUpAction(     QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Scroll Up")),
      scrollDownAction(   QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Scroll Down")),
      previousPageAction( QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Previous Page")),
      nextPageAction(     QT_TRANSLATE_NOOP("QAccessibleActionInterface", "Next Page"))
   {}

   const QString pressAction;
   const QString increaseAction;
   const QString decreaseAction;
   const QString showMenuAction;
   const QString setFocusAction;
   const QString toggleAction;
   const QString scrollLeftAction;
   const QString scrollRightAction;
   const QString scrollUpAction;
   const QString scrollDownAction;
   const QString previousPageAction;
   const QString nextPageAction;
   QString localizedDescription(const QString &actionName) {
      if (actionName == pressAction) {
         return QAccessibleActionInterface::tr("Triggers the action");
      } else if (actionName == increaseAction) {
         return QAccessibleActionInterface::tr("Increase the value");
      } else if (actionName == decreaseAction) {
         return QAccessibleActionInterface::tr("Decrease the value");
      } else if (actionName == showMenuAction) {
         return QAccessibleActionInterface::tr("Shows the menu");
      } else if (actionName == setFocusAction) {
         return QAccessibleActionInterface::tr("Sets the focus");
      } else if (actionName == toggleAction) {
         return QAccessibleActionInterface::tr("Toggles the state");
      } else if (actionName == scrollLeftAction) {
         return QAccessibleActionInterface::tr("Scrolls to the left");
      } else if (actionName == scrollRightAction) {
         return QAccessibleActionInterface::tr("Scrolls to the right");
      } else if (actionName == scrollUpAction) {
         return QAccessibleActionInterface::tr("Scrolls up");
      } else if (actionName == scrollDownAction) {
         return QAccessibleActionInterface::tr("Scrolls down");
      } else if (actionName == previousPageAction) {
         return QAccessibleActionInterface::tr("Goes back a page");
      } else if (actionName == nextPageAction) {
         return QAccessibleActionInterface::tr("Goes to the next page");
      }
      return QString();
   }
};
Q_GLOBAL_STATIC(QAccessibleActionStrings, accessibleActionStrings)

QString QAccessibleActionInterface::localizedActionName(const QString &actionName) const
{
   return QAccessibleActionInterface::tr(qPrintable(actionName));
}
QString QAccessibleActionInterface::localizedActionDescription(const QString &actionName) const
{
   return accessibleActionStrings()->localizedDescription(actionName);
}
const QString &QAccessibleActionInterface::pressAction()
{
   return accessibleActionStrings()->pressAction;
}
const QString &QAccessibleActionInterface::increaseAction()
{
   return accessibleActionStrings()->increaseAction;
}
const QString &QAccessibleActionInterface::decreaseAction()
{
   return accessibleActionStrings()->decreaseAction;
}
const QString &QAccessibleActionInterface::showMenuAction()
{
   return accessibleActionStrings()->showMenuAction;
}
const QString &QAccessibleActionInterface::setFocusAction()
{
   return accessibleActionStrings()->setFocusAction;
}
const QString &QAccessibleActionInterface::toggleAction()
{
   return accessibleActionStrings()->toggleAction;
}
QString QAccessibleActionInterface::scrollLeftAction()
{
   return accessibleActionStrings()->scrollLeftAction;
}
QString QAccessibleActionInterface::scrollRightAction()
{
   return accessibleActionStrings()->scrollRightAction;
}
QString QAccessibleActionInterface::scrollUpAction()
{
   return accessibleActionStrings()->scrollUpAction;
}
QString QAccessibleActionInterface::scrollDownAction()
{
   return accessibleActionStrings()->scrollDownAction;
}
QString QAccessibleActionInterface::previousPageAction()
{
   return accessibleActionStrings()->previousPageAction;
}
QString QAccessibleActionInterface::nextPageAction()
{
   return accessibleActionStrings()->nextPageAction;
}
QString qAccessibleLocalizedActionDescription(const QString &actionName)
{
   return accessibleActionStrings()->localizedDescription(actionName);
}

#endif // QT_NO_ACCESSIBILITY

