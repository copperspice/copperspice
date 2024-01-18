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

#ifndef QDECLARATIVEBINDING_P_H
#define QDECLARATIVEBINDING_P_H

#include <qdeclarative.h>
#include <qdeclarativepropertyvaluesource.h>
#include <qdeclarativeexpression.h>
#include <qdeclarativeproperty.h>
#include <qdeclarativeproperty_p.h>

#include <QtCore/QObject>
#include <QtCore/QMetaProperty>

QT_BEGIN_NAMESPACE

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeAbstractBinding
{
 public:
   typedef QWeakPointer<QDeclarativeAbstractBinding> Pointer;

   QDeclarativeAbstractBinding();

   enum DestroyMode {
      // The binding should disconnect itself upon destroy
      DisconnectBinding,

      // The binding doesn't need to disconnect itself, but it can if it wants to.
      //
      // This is used in QDeclarativeData::destroyed() - at the point at which the bindings are
      // destroyed, the notifiers are already disconnected, so no need to disconnect each
      // binding again.
      //
      // Bindings can use this flag to speed up destruction, especially for compiled bindings
      // disconnecting a single binding might be slow.
      KeepBindingConnected
   };

   virtual void destroy(DestroyMode mode = DisconnectBinding);

   virtual QString expression() const;

   enum DisconnectMode {

      // Just this single binding is getting disconnected, other bindings remain connected and
      // should not be changed.
      DisconnectOne,

      // All bindings of the same object are getting disconnected. As an optimization, it is
      // therefore valid to disconnect all bindings in one go.
      DisconnectAll
   };

   // disconnectMode can be ignored, it is just a hint for potential optimization
   virtual void disconnect(DisconnectMode disconnectMode) = 0;

   enum Type { PropertyBinding, ValueTypeProxy };
   virtual Type bindingType() const {
      return PropertyBinding;
   }

   QObject *object() const;
   int propertyIndex() const;

   void setEnabled(bool e) {
      setEnabled(e, QDeclarativePropertyPrivate::DontRemoveBinding);
   }
   virtual void setEnabled(bool, QDeclarativePropertyPrivate::WriteFlags) = 0;

   void update() {
      update(QDeclarativePropertyPrivate::DontRemoveBinding);
   }
   virtual void update(QDeclarativePropertyPrivate::WriteFlags) = 0;

   void addToObject(QObject *, int);
   void removeFromObject();

   static Pointer getPointer(QDeclarativeAbstractBinding *p) {
      return p ? p->weakPointer() : Pointer();
   }

 protected:
   virtual ~QDeclarativeAbstractBinding();
   void clear();

 private:
   Pointer weakPointer();

   friend class QDeclarativeData;
   friend class QDeclarativeComponentPrivate;
   friend class QDeclarativeValueTypeProxyBinding;
   friend class QDeclarativePropertyPrivate;
   friend class QDeclarativeVME;
   friend class QtSharedPointer::ExternalRefCount<QDeclarativeAbstractBinding>;

   QObject *m_object;
   int m_propertyIndex;
   QDeclarativeAbstractBinding **m_mePtr;
   QDeclarativeAbstractBinding **m_prevBinding;
   QDeclarativeAbstractBinding  *m_nextBinding;
   QSharedPointer<QDeclarativeAbstractBinding> m_selfPointer;
};

class QDeclarativeValueTypeProxyBinding : public QDeclarativeAbstractBinding
{
 public:
   QDeclarativeValueTypeProxyBinding(QObject *o, int coreIndex);

   virtual Type bindingType() const {
      return ValueTypeProxy;
   }

   virtual void setEnabled(bool, QDeclarativePropertyPrivate::WriteFlags);
   virtual void update(QDeclarativePropertyPrivate::WriteFlags);
   virtual void disconnect(DisconnectMode disconnectMode);

   QDeclarativeAbstractBinding *binding(int propertyIndex);

   void removeBindings(quint32 mask);

 protected:
   ~QDeclarativeValueTypeProxyBinding();

 private:
   void recursiveEnable(QDeclarativeAbstractBinding *, QDeclarativePropertyPrivate::WriteFlags);
   void recursiveDisable(QDeclarativeAbstractBinding *);

   friend class QDeclarativeAbstractBinding;
   QObject *m_object;
   int m_index;
   QDeclarativeAbstractBinding *m_bindings;
};

class QDeclarativeContext;
class QDeclarativeBindingPrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeBinding : public QDeclarativeExpression,
   public QDeclarativeAbstractBinding
{
   DECL_CS_OBJECT(QDeclarativeBinding)

 public:
   enum EvaluateFlag { RequiresThisObject = 0x01 };
   using EvaluateFlags = QFlags<EvaluateFlag>;

   QDeclarativeBinding(const QString &, QObject *, QDeclarativeContext *, QObject *parent = nullptr);
   QDeclarativeBinding(const QString &, QObject *, QDeclarativeContextData *, QObject *parent = nullptr);
   QDeclarativeBinding(void *, QDeclarativeRefCount *, QObject *, QDeclarativeContextData *,
                       const QString &, int, QObject *parent);
   QDeclarativeBinding(const QScriptValue &, QObject *, QDeclarativeContextData *, QObject *parent = nullptr);

   void setTarget(const QDeclarativeProperty &);
   QDeclarativeProperty property() const;

   void setEvaluateFlags(EvaluateFlags flags);
   EvaluateFlags evaluateFlags() const;

   bool enabled() const;

   // Inherited from  QDeclarativeAbstractBinding
   virtual void setEnabled(bool, QDeclarativePropertyPrivate::WriteFlags flags);
   virtual void update(QDeclarativePropertyPrivate::WriteFlags flags);
   virtual QString expression() const;
   virtual void disconnect(DisconnectMode disconnectMode);

   typedef int Identifier;
   static Identifier Invalid;
   static QDeclarativeBinding *createBinding(Identifier, QObject *, QDeclarativeContext *, const QString &, int,
         QObject *parent = nullptr);

 public :
   DECL_CS_SLOT_1(Public, void update())
   DECL_CS_SLOT_OVERLOAD(update)

 protected:
   ~QDeclarativeBinding();
   void emitValueChanged();

 private:
   Q_DECLARE_PRIVATE(QDeclarativeBinding)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativeBinding::EvaluateFlags)

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDeclarativeBinding *)

#endif // QDECLARATIVEBINDING_P_H
