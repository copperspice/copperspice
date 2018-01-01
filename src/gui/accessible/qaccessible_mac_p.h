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

#ifndef QACCESSIBLE_MAC_P_H
#define QACCESSIBLE_MAC_P_H

#include <qglobal.h>
#include <qt_mac_p.h>
#include <qaccessible.h>
#include <qwidget.h>
#include <qdebug.h>

// #define Q_ACCESSIBLE_MAC_DEBUG

QT_BEGIN_NAMESPACE

//  QAccessibleInterfaceWrapper wraps QAccessibleInterface and adds a ref count.
//  QAccessibleInterfaceWrapper is a "by-value" class.
class QAccessibleInterfaceWrapper
{
 public:
   QAccessibleInterfaceWrapper()
      : interface(0), childrenIsRegistered(new bool(false)), refCount(new int(1)) { }

   QAccessibleInterfaceWrapper(QAccessibleInterface *interface)
      : interface(interface), childrenIsRegistered(new bool(false)), refCount(new int(1))  { }

   ~QAccessibleInterfaceWrapper() {
      if (--(*refCount) == 0) {
         delete interface;
         delete refCount;
         delete childrenIsRegistered;
      }
   }

   QAccessibleInterfaceWrapper(const QAccessibleInterfaceWrapper &other)
      : interface(other.interface), childrenIsRegistered(other.childrenIsRegistered), refCount(other.refCount) {
      ++(*refCount);
   }

   void operator=(const QAccessibleInterfaceWrapper &other) {
      if (other.interface == interface) {
         return;
      }

      if (--(*refCount) == 0) {
         delete interface;
         delete refCount;
         delete childrenIsRegistered;
      }

      interface = other.interface;
      childrenIsRegistered = other.childrenIsRegistered;
      refCount = other.refCount;
      ++(*refCount);
   }

   QAccessibleInterface *interface;
   bool *childrenIsRegistered;

 private:
   int *refCount;
};

/*
    QAInterface represents one accessiblity item. It hides the fact that
    one QAccessibleInterface may represent more than one item, and it also
    automates the memory management for QAccessibleInterfaces using the
    QAccessibleInterfaceWrapper wrapper class.

    It has the same API as QAccessibleInterface, minus the child parameter
    in the functions.
*/
class QAInterface : public QAccessible
{
 public:
   QAInterface()
      : base(QAccessibleInterfaceWrapper()) {
   }

   QAInterface(QAccessibleInterface *interface, int child = 0) {
      if (interface == 0 || child > interface->childCount()) {
         base = QAccessibleInterfaceWrapper();
      } else {
         base = QAccessibleInterfaceWrapper(interface);
         m_cachedObject = interface->object();
         this->child = child;
      }
   }

   QAInterface(QAccessibleInterfaceWrapper wrapper, int child = 0)
      : base(wrapper), m_cachedObject(wrapper.interface->object()), child(child) {
   }

   QAInterface(const QAInterface &other, int child) {
      if (other.isValid() == false || child > other.childCount()) {
         base = QAccessibleInterfaceWrapper();
      } else {
         base = other.base;
         m_cachedObject = other.m_cachedObject;
         this->child = child;
      }
   }

   bool operator==(const QAInterface &other) const;
   bool operator!=(const QAInterface &other) const;

   inline QString actionText (int action, Text text) const {
      return base.interface->actionText(action, text, child);
   }

   QAInterface childAt(int x, int y) const {
      if (!checkValid()) {
         return QAInterface();
      }

      const int foundChild = base.interface->childAt(x, y);

      if (foundChild == -1) {
         return QAInterface();
      }

      if (child == 0) {
         return navigate(QAccessible::Child, foundChild);
      }

      if (foundChild == child) {
         return *this;
      }
      return QAInterface();
   }

   int indexOfChild(const QAInterface &child) const {
      if (!checkValid()) {
         return -1;
      }

      if (*this != child.parent()) {
         return -1;
      }

      if (object() == child.object()) {
         return child.id();
      }

      return base.interface->indexOfChild(child.base.interface);
   }

   inline int childCount() const {
      if (!checkValid()) {
         return 0;
      }

      if (child != 0) {
         return 0;
      }
      return base.interface->childCount();
   }

   QList<QAInterface> children() const {
      if (!checkValid()) {
         return QList<QAInterface>();
      }

      QList<QAInterface> children;
      for (int i = 1; i <= childCount(); ++i) {
         children.append(navigate(QAccessible::Child, i));
      }
      return children;
   }

   QAInterface childAt(int index) const {
      return navigate(QAccessible::Child, index);
   }

   inline void doAction(int action, const QVariantList &params = QVariantList()) const {
      if (!checkValid()) {
         return;
      }

      base.interface->doAction(action, child, params);
   }

   QAInterface navigate(RelationFlag relation, int entry) const;

   inline QObject *object() const {
      if (! checkValid()) {
         return 0;
      }

      return base.interface->object();
   }

   QAInterface objectInterface() const {
      if (!checkValid()) {
         return QAInterface();
      }

      QObject *obj = object();
      QAInterface current = *this;
      while (obj == 0) {
         QAInterface parent = current.parent();
         if (parent.isValid() == false) {
            break;
         }

         obj = parent.object();
         current = parent;
      }
      return current;
   }

   inline QObject *cachedObject() const {
      if (!checkValid()) {
         return 0;
      }

      return m_cachedObject;
   }

   inline QRect rect() const {
      if (!checkValid()) {
         return QRect();
      }

      return base.interface->rect(child);
   }

   inline Role role() const {
      if (!checkValid()) {
         return QAccessible::NoRole;
      }

      return base.interface->role(child);
   }

   inline void setText(Text t, const QString &text) const {
      if (!checkValid()) {
         return;
      }

      base.interface->setText(t, child, text);
   }

   inline State state() const {
      if (!checkValid()) {
         return 0;
      }

      return base.interface->state(child);
   }

   inline QString text (Text text) const {
      if (!checkValid()) {
         return QString();
      }

      return base.interface->text(text, child);
   }

   inline QString value() const {
      return text(QAccessible::Value);
   }

   inline QString name() const {
      return text(QAccessible::Name);
   }

   inline int userActionCount() const {
      if (!checkValid()) {
         return 0;
      }

      return base.interface->userActionCount(child);
   }

   inline QString className() const {
      if (!checkValid()) {
         return QString();
      }

      return QLatin1String(base.interface->object()->metaObject()->className());
   }

   inline int id() const {
      return child;
   }

   inline bool isValid() const {
      return (base.interface != 0 && base.interface->isValid());
   }

   QAInterface parent() const {
      return navigate(QAccessible::Ancestor, 1);
   }

   QAccessibleInterfaceWrapper interfaceWrapper() const {
      return base;
   }

 protected:
   bool checkValid() const {
      const bool valid = isValid();

#ifdef Q_ACCESSIBLE_MAC_DEBUG
      if (! valid) {
         qFatal("QAInterface::checkValid() Attempted to use an invalid interface.");
      }
#endif
      return valid;
   }

   QAccessibleInterfaceWrapper base;
   QObject *m_cachedObject;
   int child;
};

// QAElement is a thin wrapper around an AXUIElementRef that automates the ref-counting
class QAElement
{
 public:
   QAElement();
   explicit QAElement(AXUIElementRef elementRef);
   QAElement(const QAElement &element);

   ~QAElement();

   inline int id() const {
      UInt64 theId;
      theId = 0;

      return theId;
   }

   inline AXUIElementRef element() const {
      return elementRef;
   }

   inline bool isValid() const {
      return (elementRef != 0);
   }

   void operator=(const QAElement &other);
   bool operator==(const QAElement &other) const;

 private:
   AXUIElementRef elementRef;
};

class QInterfaceFactory
{
 public:
   virtual QAInterface interface(UInt64 identifier) = 0;
   virtual QAElement element(int id) = 0;

   virtual QAElement element(const QAInterface &interface) {
      return element(interface.id());
   }

   virtual void registerChildren() = 0;
   virtual ~QInterfaceFactory() {}
};

/*
    QAccessibleHierarchyManager bridges the Mac and Qt accessibility hierarchies.
    There is a one-to-one relationship between QAElements on the Mac side
    and QAInterfaces on the Qt side, and this class provides lookup functions
    that translates between these to items.

    The identity of a QAInterface is determined by its QAccessibleInterface and
    child identifier, and the identity of a QAElement is determined by its identifier.

    QAccessibleHierarchyManager receives QObject::destroyed() signals and deletes
    the accessibility objects for destroyed objects.
*/
class QAccessibleHierarchyManager : public QObject
{
   GUI_CS_OBJECT(QAccessibleHierarchyManager)

 public:
   ~QAccessibleHierarchyManager() {
      reset();
   }

   static QAccessibleHierarchyManager *instance();
   void reset();

   void registerChildren(const QAInterface &interface);

   QAElement lookup(const QAInterface &interface);
   QAElement lookup(QObject *const object, int id);

 private :
   GUI_CS_SLOT_1(Private, void objectDestroyed(QObject *un_named_arg1))
   GUI_CS_SLOT_2(objectDestroyed)

   typedef QHash<QObject *, QInterfaceFactory *> QObjectElementHash;
   QObjectElementHash qobjectElementHash;
};

QDebug operator<<(QDebug debug, const QAInterface &interface);
bool isItInteresting(const QAInterface &interface);

QT_END_NAMESPACE

#endif
