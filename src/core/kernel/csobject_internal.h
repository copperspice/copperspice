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

#ifndef CSOBJECT_INTERNAL_H
#define CSOBJECT_INTERNAL_H

#include <QCoreApplication>
#include <QThread>
#include <QSemaphore>

#include <csmeta_callevent.h>
#include <qthread.h>

// signal & slot method ptr
template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class SlotClass, class ...SlotArgs, class SlotReturn>
bool QObject::connect(const Sender *sender, void (SignalClass::*signalMethod)(SignalArgs...),
                      const Receiver *receiver, SlotReturn (SlotClass::*slotMethod)(SlotArgs...), Qt::ConnectionType type)
{
   if (sender == nullptr) {
      qWarning("QObject::connect() Can not connect, sender is null");
      return false;
   }

   if (receiver == nullptr) {
      qWarning("QObject::connect() Can not connect, receiver is null");
      return false;
   }

   if (signalMethod == nullptr) {
      qWarning("QObject::connect() Can not connect, signal is null");
      return false;
   }

   if (slotMethod == nullptr) {
      qWarning("QObject::connect() Can not connect, slot is null");
      return false;
   }

   // get signal MetaMethod
   const QMetaObject *senderMetaObject = sender->metaObject();
   QMetaMethod signalMetaMethod = senderMetaObject->method(signalMethod);

   const char *senderClass = senderMetaObject->className();

   QByteArray signature    = signalMetaMethod.methodSignature();
   const char *signalName  = signature.constData();

   if (signature.isEmpty())  {
      qWarning("%s%s%s", "QObject::connect() ", senderClass, "::<Invalid Signal> ");
      return false;
   }

   // is signalMethod a signal
   if (signalMetaMethod.methodType() != QMetaMethod::Signal ) {
      qWarning("%s%s%s%s%s", "QObject::connect() ", senderClass, "::", signalName, "was not a valid signal");
      return false;
   }

   CsSignal::ConnectionKind kind;
   bool uniqueConnection = false;

   if (type & Qt::UniqueConnection) {
      uniqueConnection = true;
   }

   // untangle the type
   kind = static_cast<CsSignal::ConnectionKind>(type & ~Qt::UniqueConnection);

   CsSignal::connect(*sender, signalMethod, *receiver, slotMethod, kind, uniqueConnection);
   sender->connectNotify(signalMetaMethod);

   return true;
}

// signal method ptr, slot lambda
template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class T>
bool QObject::connect(const Sender *sender, void (SignalClass::*signalMethod)(SignalArgs...),
                      const Receiver *receiver, T slotLambda, Qt::ConnectionType type)
{
   static_assert(std::is_base_of<QObject, Sender>::value, "Sender must inherit from QObject");

   if (sender == nullptr) {
      qWarning("QObject::connect() Can not connect, sender is null");
      return false;
   }

   if (receiver == nullptr) {
      qWarning("QObject::connect() Can not connect, receiver is null");
      return false;
   }

   if (signalMethod == nullptr) {
      qWarning("QObject::connect() Can not connect, signal is null");
      return false;
   }

   // get signal MetaMethod
   const QMetaObject *senderMetaObject = sender->metaObject();
   QMetaMethod signalMetaMethod = senderMetaObject->method(signalMethod);

   const char *senderClass = senderMetaObject->className();

   QByteArray signature    = signalMetaMethod.methodSignature();
   const char *signalName  = signature.constData();

   if (signature.isEmpty())  {
      qWarning("%s%s%s", "QObject::connect() ", senderClass, "::<Invalid Signal> ");
      return false;
   }

   // is signalMethod a signal
   if (signalMetaMethod.methodType() != QMetaMethod::Signal ) {
      qWarning("%s%s%s%s%s", "QObject::connect() ", senderClass, "::", signalName, "was not a valid signal");
      return false;
   }

   CsSignal::ConnectionKind kind;
   bool uniqueConnection = false;

   if (type & Qt::UniqueConnection) {
      uniqueConnection = true;
   }

   // untangle the type
   kind = static_cast<CsSignal::ConnectionKind>(type & ~Qt::UniqueConnection);

   CsSignal::connect(*sender, signalMethod, *receiver, slotLambda, kind, uniqueConnection);
   sender->connectNotify(signalMetaMethod);

   return true;
}

// signal & slot method ptr
template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class SlotClass, class ...SlotArgs, class SlotReturn>
bool QObject::disconnect(const Sender *sender, void (SignalClass::*signalMethod)(SignalArgs...),
                         const Receiver *receiver, SlotReturn (SlotClass::*slotMethod)(SlotArgs...))
{
   static_assert(std::is_base_of<QObject, Sender>::value, "Sender must inherit from QObject");

   if (sender == nullptr || (receiver == nullptr && slotMethod != nullptr)) {
      qWarning("QObject::disconnect() Unexpected null parameter");
      return false;
   }

   bool retval = CsSignal::disconnect(*sender, signalMethod, *receiver, slotMethod);

   if (retval) {
      const QMetaObject *senderMetaObject = sender->metaObject();

      if (senderMetaObject) {
         QMetaMethod signalMetaMethod = senderMetaObject->method(signalMethod);
         const_cast<Sender *>(sender)->disconnectNotify(signalMetaMethod);
      }
   }

   return retval;
}

// signal method ptr, slot lambda or function ptr
template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class T>
bool QObject::disconnect(const Sender *sender, void (SignalClass::*signalMethod)(SignalArgs...),
                         const Receiver *receiver, T slotMethod)
{
   bool retval = CsSignal::disconnect(*sender, signalMethod, *receiver, slotMethod);

   if (retval) {
      const QMetaObject *senderMetaObject = sender->metaObject();

      if (senderMetaObject) {
         QMetaMethod signalMetaMethod = senderMetaObject->method(signalMethod);
         const_cast<Sender *>(sender)->disconnectNotify(signalMetaMethod);
      }
   }

   return retval;
}

template<class ...Ts>
bool cs_factory_interface_query(const QString &data)
{
   std::vector<const char *> list = { qobject_interface_iid<Ts *>()... };

   for (unsigned int k = 0; k < list.size(); ++k) {

      if (data == list[k]) {
         return true;
      }
   }

   return false;
}

/*
   if (strcmp(data, qobject_interface_iid<QSqlDriverFactoryInterface>()) == 0) {
      return true;
   }

   if (strcmp(data, qobject_interface_iid<QFactoryInterface>()) == 0) {
      return true;
   }
*/

template<class R, class ...Ts>
bool QMetaObject::invokeMethod(QObject *object, const char *member, Qt::ConnectionType type,
               CSReturnArgument<R> retval, CSArgument<Ts>... Vs)
{
   if (! object) {
      return false;
   }

   // signature of the method being invoked
   QByteArray sig = member;
   sig += "(";
   sig += cs_argName(Vs...);
   sig += ")";

   //
   const QMetaObject *metaObject = object->metaObject();
   int index = metaObject->indexOfMethod(sig.constData());

   if (index == -1)  {
      QList<QString> msgList;

      // find registerd methods which match the name
      for (int k = 0; k < metaObject->methodCount(); ++k) {

         int numOfChars = sig.indexOf('(') + 1;

         QMetaMethod testMethod   = metaObject->method(k);
         QByteArray testSignature = testMethod.methodSignature();

         if (strncmp(testSignature.constData(), sig.constData(), numOfChars) == 0)  {
            msgList.append(testSignature);

            // test if the related method matches
            if (testMethod.invoke(object, type, retval, std::forward<Ts>(Vs.getData())...)) {
               return true;
            }
         }
      }

      qWarning("QMetaObject::invokeMethod() No such method %s::%s", metaObject->className(), sig.constData());

      for (int k = 0; k < msgList.size(); ++k ) {
         qWarning(" Related methods: %s", qPrintable(msgList[k]) );
      }

      return false;
   }

   QMetaMethod metaMethod = metaObject->method(index);

   // about to call QMetaMethod::invoke()
   return metaMethod.invoke(object, type, retval, std::forward<Ts>(Vs.getData())...);
}

template<class ...Ts>
bool QMetaObject::invokeMethod(QObject *object, const char *member, Qt::ConnectionType type, CSArgument<Ts>... Vs)
{
   if (! object) {
      return false;
   }

   // signature of the method being invoked
   QByteArray sig = member;
   sig += "(";
   sig += cs_argName(Vs...);
   sig += ")";

   //
   const QMetaObject *metaObject = object->metaObject();
   int index = metaObject->indexOfMethod(sig.constData());

   if (index == -1)  {
      QList<QString> msgList;

      // find registerd methods which match the name
      for (int k = 0; k < metaObject->methodCount(); ++index) {

         int numOfChars = sig.indexOf('(') + 1;

         QMetaMethod testMethod   = metaObject->method(k);
         QByteArray testSignature = testMethod.methodSignature();

         if (strncmp(testSignature.constData(), sig.constData(), numOfChars) == 0)  {
            msgList.append(testSignature);

            // test if the related method matches
            if (testMethod.invoke(object, type, std::forward<Ts>(Vs.getData())...)) {
               return true;
            }
         }
      }

      qWarning("QMetaObject::invokeMethod() No such method %s::%s", metaObject->className(), sig.constData());

      for (int k = 0; k < msgList.size(); ++k ) {
         qWarning(" Related methods: %s", qPrintable(msgList[k]) );
      }

      return false;
   }

   QMetaMethod metaMethod = metaObject->method(index);

   // about to call QMetaMethod::invoke()
   return metaMethod.invoke(object, type, std::forward<Ts>(Vs.getData())...);
}

template<class R, class ...Ts>
bool QMetaObject::invokeMethod(QObject *object, const char *member, CSReturnArgument<R> retval, CSArgument<Ts>... Vs)
{
   // calls the first overload
   return invokeMethod(object, member, Qt::AutoConnection, retval, Vs...);
}

template<class ...Ts>
bool QMetaObject::invokeMethod(QObject *object, const char *member, CSArgument<Ts>... Vs)
{
   // calls the second overload
   return invokeMethod(object, member, Qt::AutoConnection, Vs...);
}


// ***
// QMetaMethod::invoke moved from csmeta.h because this method calls methods in QObject

template<class R, class ...Ts>
bool QMetaMethod::invoke(QObject *object, Qt::ConnectionType type, CSReturnArgument<R> retval, Ts &&...Vs) const
{
   bool isConstructor = false;

   if (this->methodType() == QMetaMethod::Constructor) {
      isConstructor = true;

   } else if (! object || ! m_metaObject) {
      return false;

   }

   if (m_bento == 0)  {
      qWarning("QMetaMethod::invoke() MetaMethod registration issue, Receiver is %s", m_metaObject->className());
      return false;
   }

   // check return type, only place this method is called
   bool ok = m_bento->checkReturnType(retval);

   if (! ok) {
      qWarning("QMetaMethod::invoke() Return type mismatch");
      return false;
   }

   //
   int passedArgCount = sizeof...(Ts);
   int methodArgCount = this->parameterTypes().count();

   if (passedArgCount != methodArgCount) {
      qWarning("QMetaMethod::invoke() Passed argument count does not equal the method argument count, Receiver is %s",
               m_metaObject->className());

      return false;
   }

   QThread *currentThread = QThread::currentThread();
   QThread *objectThread  = 0;

   if (isConstructor) {
      // only allowed to create a new object in your own thread
      type = Qt::DirectConnection;

   } else {
      // check connection type
      objectThread = object->thread();

      if (type == Qt::AutoConnection) {

         if (currentThread == objectThread) {
            type = Qt::DirectConnection;

         } else   {
            type = Qt::QueuedConnection;

         }
      }
   }

   // store the signal data, false indicates the data will not be copied
   CsSignal::Internal::TeaCup_Data<typename std::remove_reference<Ts>::type...> dataPack(false, std::forward<Ts>(Vs)...);

   if (type == Qt::DirectConnection) {
      // retval is passed by pointer
      m_bento->invoke(object, &dataPack, &retval);

   } else if (type == Qt::QueuedConnection) {

      if (! dynamic_cast<CSReturnArgument<void> *>(&retval)) {
         qWarning("QMetaMethod::invoke() Queued connections can not have a return value");
         return false;
      }

      // store the signal data, true indicates the data will be copied into a TeaCup Object (stored on the heap)
      CSMetaCallEvent *event = new CSMetaCallEvent(m_bento,
                  new CsSignal::Internal::TeaCup_Data<Ts...>(true, std::forward<Ts>(Vs)...), 0, -1);

      QCoreApplication::postEvent(object, event);

   } else {
      // blocking queued connection

      if (currentThread == objectThread) {
         qWarning("QMetaMethod::invoke() Dead lock detected in BlockingQueuedConnection, Receiver is %s(%p)",
                  m_metaObject->className(), object);
      }

      QSemaphore semaphore;

      // broom (on hold, ok)
      // add &retval to QMetaCallEvent so we can return a value

      // store the signal data, false indicates the data will not be copied
      CSMetaCallEvent *event = new CSMetaCallEvent(m_bento,
                  new CsSignal::Internal::TeaCup_Data<Ts...>(false, std::forward<Ts>(Vs)...), 0, -1, &semaphore);

      QCoreApplication::postEvent(object, event);

      semaphore.acquire();
   }

   return true;
}

template<class ...Ts>
bool QMetaMethod::invoke(QObject *object, Qt::ConnectionType type, Ts &&...Vs) const
{
   if (! object || ! m_metaObject) {
      return false;
   }

   // check return type, omitted since it retuns void

   int passedArgCount = sizeof...(Ts);
   int methodArgCount = this->parameterTypes().count();

   if (passedArgCount != methodArgCount) {
      qWarning("QMetaMethod::invoke() Passed argument count does not equal the method argument count");
      return false;
   }

   // check connection type
   QThread *currentThread = QThread::currentThread();
   QThread *objectThread  = object->thread();

   if (type == Qt::AutoConnection) {

      if (currentThread == objectThread) {
         type = Qt::DirectConnection;

      } else   {
         type = Qt::QueuedConnection;

      }
   }

   // store the signal data, false indicates the data will not be copied
   CsSignal::Internal::TeaCup_Data<Ts...> dataPack(false, std::forward<Ts>(Vs)...);

   if (type == Qt::DirectConnection) {
      // invoke calls the method

      m_bento->invoke(object, &dataPack);

   } else if (type == Qt::QueuedConnection) {

      // store the signal data, false indicates the data will not be copied
      CSMetaCallEvent *event = new CSMetaCallEvent(m_bento, new CsSignal::Internal::TeaCup_Data<Ts...>(true, std::forward<Ts>(Vs)...), 0, -1);
      QCoreApplication::postEvent(object, event);

   } else {
      // blocking queued connection

      if (currentThread == objectThread) {
         qWarning("QMetaMethod::invoke() Dead lock detected in BlockingQueuedConnection, Receiver is %s(%p)",
                  m_metaObject->className(), object);
      }

      QSemaphore semaphore;

      // store the signal data, false indicates the data will not be copied
      CSMetaCallEvent *event = new CSMetaCallEvent(m_bento, new CsSignal::Internal::TeaCup_Data<Ts...>(false, std::forward<Ts>(Vs)...), 0, -1,
            &semaphore);
      QCoreApplication::postEvent(object, event);

      semaphore.acquire();
   }

   return true;
}

#endif
