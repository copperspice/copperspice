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

#ifndef QDECLARATIVEDATA_P_H
#define QDECLARATIVEDATA_P_H

#include <QtScript/qscriptvalue.h>

QT_BEGIN_NAMESPACE

class QDeclarativeGuardImpl;
class QDeclarativeCompiledData;
class QDeclarativeAbstractBinding;
class QDeclarativeAbstractBoundSignal;
class QDeclarativeContext;
class QDeclarativePropertyCache;
class QDeclarativeContextData;
class QDeclarativeNotifier;
class QDeclarativeDataExtended;

// This class is structured in such a way, that simply zero'ing it is the
// default state for elemental object allocations.  This is crucial in the
// workings of the QDeclarativeInstruction::CreateSimpleObject instruction.
// Don't change anything here without first considering that case!

class QDeclarativeData : public CSAbstractDeclarativeData
{

 public:
   QDeclarativeData()
      : ownMemory(true), ownContext(false), indestructible(true), explicitIndestructibleSet(false),
        context(0), outerContext(0), bindings(0), nextContextObject(0), prevContextObject(0), bindingBitsSize(0),
        bindingBits(0), lineNumber(0), columnNumber(0), deferredComponent(0), deferredIdx(0),
        scriptValue(0), objectDataRefCount(0), propertyCache(0), guards(0), extendedData(0) {
      init();
   }

   static inline void init() {
      CSAbstractDeclarativeData::destroyed = destroyed;
      CSAbstractDeclarativeData::parentChanged = parentChanged;

      // BROOM (decalartive)
      // CSAbstractDeclarativeData::signalEmitted = 0;
      // CSAbstractDeclarativeData::receivers = 0;
   }

   static void destroyed(CSAbstractDeclarativeData *, QObject *);
   static void parentChanged(CSAbstractDeclarativeData *, QObject *, QObject *);

   // BROOM (decalartive)
   //static void signalEmitted(CSAbstractDeclarativeData *, QObject *, int, void **);
   //static int  receivers(CSAbstractDeclarativeData *, const QObject *, int);

   void destroyed(QObject *);
   void parentChanged(QObject *, QObject *);

   // BROOM (decalartive)
   //void signalEmitted(QObject *, int, void **);
   //void receivers(QObject *, int);

   void setImplicitDestructible() {
      if (!explicitIndestructibleSet) {
         indestructible = false;
      }
   }

   quint32 ownMemory: 1;
   quint32 ownContext: 1;
   quint32 indestructible: 1;
   quint32 explicitIndestructibleSet: 1;
   quint32 dummy: 28;

   // The context that created the C++ object
   QDeclarativeContextData *context;

   // The outermost context in which this object lives
   QDeclarativeContextData *outerContext;

   QDeclarativeAbstractBinding *bindings;

   // Linked list for QDeclarativeContext::contextObjects
   QDeclarativeData *nextContextObject;
   QDeclarativeData **prevContextObject;

   int bindingBitsSize;
   quint32 *bindingBits;
   bool hasBindingBit(int) const;
   void clearBindingBit(int);
   void setBindingBit(QObject *obj, int);

   ushort lineNumber;
   ushort columnNumber;

   QDeclarativeCompiledData *deferredComponent; // Can't this be found from the context?
   unsigned int deferredIdx;

   // ### Can we make this QScriptValuePrivate so we incur no additional allocation cost?
   QScriptValue *scriptValue;
   quint32 objectDataRefCount;
   QDeclarativePropertyCache *propertyCache;

   QDeclarativeGuardImpl *guards;

   // BROOM (decalartive)
   static QDeclarativeData *get(const QObject *object, bool create = false) {
      QObjectPrivate *priv = QObjectPrivate::get(const_cast<QObject *>(object));

      if (priv->wasDeleted) {
         Q_ASSERT(!create);
         return 0;

      } else if (priv->declarativeData) {
         return static_cast<QDeclarativeData *>(priv->declarativeData);

      } else if (create) {
         priv->declarativeData = new QDeclarativeData;
         return static_cast<QDeclarativeData *>(priv->declarativeData);

      } else {
         return 0;

      }
   }

   bool hasExtendedData() const {
      return extendedData != 0;
   }
   QDeclarativeNotifier *objectNameNotifier() const;
   QHash<int, QObject *> *attachedProperties() const;
   void addBoundSignal(QDeclarativeAbstractBoundSignal *signal);
   void removeBoundSignal(QDeclarativeAbstractBoundSignal *signal);
   void disconnectNotifiers();

 private:
   // For objectNameNotifier, attachedProperties and bound signal list
   mutable QDeclarativeDataExtended *extendedData;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEDATA_P_H
