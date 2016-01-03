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

#ifndef CSOBJECT_INTERNAL_H
#define CSOBJECT_INTERNAL_H

#include <QCoreApplication>
#include <QThread>
#include <QSemaphore>

#include <csmeta_callevent.h>
#include <qthread.h>


/**   \cond INTERNAL (notation so DoxyPress will not parse this class  */

// 1
template<class T, class U, class = void>
class cs_check_connect_args
   : public cs_check_connect_args<T, decltype(&U::operator())>
{
};

/**   \endcond   */


// 2 slot is a func ptr, first signal and first slot parameters match
template<class T, class ...ArgsX, class ...ArgsY>
class cs_check_connect_args<void (*)(T, ArgsX...), void (*)(T, ArgsY...)>
   : public cs_check_connect_args<void (*) (ArgsX...), void (*) (ArgsY...)>
{
};

// slot is a func ptr, slot has no parameters
template<class ...ArgsX>
class cs_check_connect_args<void (*)(ArgsX...), void (*)()>
   : public std::integral_constant<bool, true>
{
};

//  slot is a func ptr, signal has the same number of parms as the slot, types mismatch
template<class ...ArgsX, class ...ArgsY>
class cs_check_connect_args < void (*)(ArgsX...), void (*)(ArgsY...),
   typename std::enable_if < sizeof...(ArgsX) == sizeof...(ArgsY) &&
   ! std::is_same<std::tuple<ArgsX...>, std::tuple<ArgsY...>>::value >::type >
         : public std::integral_constant<bool, false>
{
};

//  slot is a func ptr, signal has fewer number of parms than the slot
template<class ...ArgsX, class ...ArgsY>
class cs_check_connect_args < void (*)(ArgsX...), void (*)(ArgsY...),
   typename std::enable_if<sizeof...(ArgsX) < sizeof...(ArgsY)>::type >
   : public std::integral_constant<bool, false>
{
};


// 3 slot is a method ptr
template<class T, class...ArgsX, class...ArgsY>
class cs_check_connect_args<void (*)(ArgsX...), void (T::*)(ArgsY...) >
   : public cs_check_connect_args<void (*)(ArgsX...), void (*) (ArgsY...)>
{
};

// slot is a const method ptr
template<class T, class...ArgsX, class...ArgsY>
class cs_check_connect_args<void (*)(ArgsX...), void (T::*)(ArgsY...) const>
   : public cs_check_connect_args<void (*)(ArgsX...), void (*) (ArgsY...)>
{
};


// **
template<class SignalClass, class ...SignalArgTypes, class ...Ts>
void QMetaObject::activate(QObject *sender, void (SignalClass::*signal)(SignalArgTypes...), Ts &&... Vs)
{
   // ensure signal args are passed
   static_assert( std::is_convertible<std::tuple<Ts...>, std::tuple<SignalArgTypes...>>::value,
                  "QObject::Activate()  Signal parameter or mismatch. Verify CS_SIGNAL_1 and CS_SIGNAL_2");

   Bento<void (SignalClass::*)(SignalArgTypes...)> signal_Bento(signal);

   if (sender->m_declarativeData && CSAbstractDeclarativeData::signalEmitted) {
      // broom (on hold, declarative)
      // CSAbstractDeclarativeData::signalEmitted(sender->m_declarativeData, sender, methodOffset, Vs...);
   }

   bool isConnected = sender->isSignalConnected(signal_Bento);

   if (! isConnected)  {
      // nothing is connected to this signal
      return;
   }

   if (false) {
      // not supporting QtTest, isSpyTest() was removed
      return;
   }

   if (sender->signalsBlocked()) {
      // signals from this object are blocked, do not process
      return;
   }

   // threading and queuedConnections
   Qt::HANDLE currentThreadId = QThread::currentThreadId();
   std::unique_lock<std::mutex> senderLock {sender->m_mutex_ToReceiver};

   // store the signal data, false indicates the data will not be copied
   TeaCup_Data<SignalArgTypes...> dataPack(false, std::forward<Ts>(Vs)...);

   QPointer<QObject> senderTest = sender;

   bool raceHappened = false;
   int maxCount = sender->m_connectList_ToReceiver.count();

   for (int k = 0; k < maxCount; ++k) {
      const QObject::ConnectStruct &connection = sender->m_connectList_ToReceiver[k];

      if (*(connection.signalMethod) != signal_Bento)  {
         // no match in connectionList for this signal
         continue;
      }

      if (connection.sender == 0) {
         // connection is marked for deletion
         continue;
      }

      const BentoAbstract *slot_Bento = connection.slotMethod;

      QObject *receiver = const_cast<QObject *>(connection.receiver);

      bool receiverInSameThread;
      receiverInSameThread = compareThreads(currentThreadId, receiver);

      int signal_index = sender->metaObject()->indexOfMethod(signal_Bento);

      if ( (connection.type == Qt::AutoConnection && ! receiverInSameThread) ||
            (connection.type == Qt::QueuedConnection)) {

         // store the signal data, true indicates the data will be copied into a TeaCup Object (stored on the heap)
         CSMetaCallEvent *event = new CSMetaCallEvent(slot_Bento, new TeaCup_Data<SignalArgTypes...>(true,
               std::forward<Ts>(Vs)... ), sender, signal_index);

         QCoreApplication::postEvent(receiver, event);

      } else if (connection.type == Qt::BlockingQueuedConnection) {

         senderLock.unlock();

         if (receiverInSameThread) {

            qWarning("QObject::activate() Dead lock detected while activating a BlockingQueuedConnection: "
                     "Sender is %s(%p), receiver is %s(%p)", sender->metaObject()->className(), sender,
                     receiver->metaObject()->className(), receiver);
         }

         QSemaphore semaphore;

         // store the signal data, false indicates the data will not be copied
         CSMetaCallEvent *event = new CSMetaCallEvent(slot_Bento, new TeaCup_Data<SignalArgTypes...>(false,
               std::forward<Ts>(Vs)... ), sender, signal_index, &semaphore);

         QCoreApplication::postEvent(receiver, event);

         semaphore.acquire();
         senderLock.lock();

      } else {
         // direct connection

         QObject::SenderStruct currentSender;
         QObject::SenderStruct *previousSender = 0;

         if (receiverInSameThread) {

            currentSender.sender       = sender;
            currentSender.signal_index = signal_index;
            currentSender.ref          = 1;

            previousSender = QObject::setCurrentSender(receiver, &currentSender);
         }

         sender->m_activateBusy++;
         int old_raceCount = sender->m_raceCount;

         senderLock.unlock();

         try {
            // invoke calls the actual method
            slot_Bento->invoke(receiver, &dataPack);

            if (! senderTest) {
               // sender object was destroyed during the invoke of the slot
               return;
            }

         } catch (...) {
            senderLock.lock();

            if (receiverInSameThread) {
               QObject::resetCurrentSender(receiver, &currentSender, previousSender);
            }

            throw;
         }

         try {
            senderLock.lock();

         } catch (std::exception &) {

            if (receiverInSameThread) {
               QObject::resetCurrentSender(receiver, &currentSender, previousSender);
            }

            // bail out
            qWarning("QObject::activate() Failed to obtain sender lock");
            return;
         }


         if (old_raceCount != sender->m_raceCount) {
            // connectionList modified
            raceHappened = true;

            maxCount = sender->m_connectList_ToReceiver.count();

            // connect() can add an entry to the end of the list
            // disconnect() can mark a connection as pending deletion
         }

         sender->m_activateBusy--;

         if (receiverInSameThread) {
            QObject::resetCurrentSender(receiver, &currentSender, previousSender);
         }
      }

   }

   if (raceHappened)  {
      // finish clean up for disconnect
      maxCount = sender->m_connectList_ToReceiver.count();

      for (int k = 0; k < maxCount; ++k) {
         const QObject::ConnectStruct &connection = sender->m_connectList_ToReceiver[k];

         if (connection.sender == 0) {
            sender->m_connectList_ToReceiver.removeAt(k);

            // yes, this is required
            k = k - 1;

            // reduce the count by one
            maxCount = maxCount - 1;

         }
      }
   }

}

// signal & slot method ptr
template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class SlotClass, class ...SlotArgs, class SlotReturn>
bool QObject::connect(const Sender *sender, void (SignalClass::*signalMethod)(SignalArgs...),
                      const Receiver *receiver, SlotReturn (SlotClass::*slotMethod)(SlotArgs...), Qt::ConnectionType type)
{
   // Sender must inhert from QObject
   static_assert( std::is_base_of<QObject, Sender>::value,
                  "QObject::connect()  Sender must inherit from QObject");

   // Receiver must inhert from QObject
   static_assert( std::is_base_of<QObject, Receiver>::value,
                  "QObject::connect()  Receiver must inherit from QObject");

   // (1) Sender must be the same class as SignalClass OR (2) Sender is a child of SignalClass
   static_assert( std::is_base_of<SignalClass, Sender>::value,
                  "QObject::connect()  Signal was not a child class of Sender");

   // (1) Receiver must be the same class as SlotClass OR (2) Receiver is a child of SlotClass
   static_assert( std::is_base_of<SlotClass, Receiver>::value,
                  "QObject::connect()  Slot was not a child class of Receiver");

   // compare signal and slot paramerter list
   static_assert( cs_check_connect_args<void (*)(SignalArgs...), void (*)(SlotArgs...) >::value,
                  "QObject::connect()  Incompatible signal/slot arguments");

   //
   if (sender == 0) {
      qWarning("QObject::connect() Can not connect, sender is null");
      return false;
   }

   if (receiver == 0) {
      qWarning("QObject::connect() Can not connect, receiver is null");
      return false;
   }

   if (signalMethod == 0) {
      qWarning("QObject::connect() Can not connect, signal is null");
      return false;
   }

   if (slotMethod == 0) {
      qWarning("QObject::connect() Can not connect, slot is null");
      return false;
   }

   // get signal name
   const QMetaObject *senderMetaObject = sender->metaObject();

   const char *senderClass   = senderMetaObject->className();
   const char *receiverClass = receiver->metaObject()->className();

   QMetaMethod signalMetaMethod = senderMetaObject->method(signalMethod);

   QByteArray signature   = signalMetaMethod.methodSignature();
   const char *signalName = signature.constData();

   if (signature.isEmpty())  {
      qWarning("%s%s%s%s%s", "QObject::connect() ", senderClass, "::<Invalid Signal>",
               " Unable to connect to receiver in ", receiverClass);
      return false;
   }

   // is signalMethod a signal
   if (signalMetaMethod.methodType() != QMetaMethod::Signal ) {
      qWarning("%s%s%s%s%s", "QObject::connect() ", senderClass, "::", signalName, " was not a valid signal" );
      return false;
   }

   if (type == Qt::AutoCompatConnection) {
      type = Qt::AutoConnection;
   }

   //
   Bento<void (SignalClass::*)(SignalArgs...)> *signalMethod_Bento =
      new Bento<void (SignalClass::*)(SignalArgs...)>(signalMethod);

   Bento<void (SlotClass::*)(SlotArgs...)> *slotMethod_Bento = new Bento<void (SlotClass::*)(SlotArgs...)>(slotMethod);

   std::unique_lock<std::mutex> senderLock {sender->m_mutex_ToReceiver};
   std::unique_lock<std::mutex> receiverLock {receiver->m_mutex_FromSender};

   if (type & Qt::UniqueConnection) {
      // user passed enum to ensure the connection is not added twice

      for (auto index = sender->m_connectList_ToReceiver.begin(); index != sender->m_connectList_ToReceiver.end(); ++index) {
         const ConnectStruct &temp = *index;

         if (temp.sender == 0) {
            // connection is marked for deletion
            continue;
         }

         if (temp.receiver != receiver) {
            continue;
         }

         if (*(temp.signalMethod) != *(signalMethod_Bento))  {
            continue;
         }

         if (*(temp.slotMethod) != *(slotMethod_Bento))  {
            continue;
         }

         // connection already exists
         return false;
      }

      // change type
      type = static_cast<Qt::ConnectionType>(type & ~Qt::UniqueConnection);
   }

   sender->addConnection(signalMethod_Bento, receiver, slotMethod_Bento, type);
   sender->connectNotify(signalMetaMethod);

   return true;
}

// signal bento, slot method ptr
template<class Receiver, class SlotClass, class ...SlotArgs, class SlotReturn>
bool QObject::connect(const QObject *sender, BentoAbstract *signalMethod_Bento,
                      const Receiver *receiver, SlotReturn (SlotClass::*slotMethod)(SlotArgs...), Qt::ConnectionType type)
{
   // Receiver must inhert from QObject
   static_assert( std::is_base_of<QObject, Receiver>::value,
                  "QObject::connect()  Receiver must inherit from QObject");

   // (1) Receiver must be the same class as SlotClass OR (2) Receiver is a child of SlotClass
   static_assert( std::is_base_of<SlotClass, Receiver>::value,
                  "QObject::connect()  Slot was not a child class of Receiver");

   //
   if (sender == 0) {
      qWarning("QObject::connect() Can not connect, sender is null");
      return false;
   }

   if (receiver == 0) {
      qWarning("QObject::connect() Can not connect, receiver is null");
      return false;
   }

   if (signalMethod_Bento == 0) {
      qWarning("QObject::connect() Can not connect, signal is null");
      return false;
   }

   if (slotMethod == 0) {
      qWarning("QObject::connect() Can not connect, slot is null");
      return false;
   }

   if (type == Qt::AutoCompatConnection) {
      type = Qt::AutoConnection;
   }

   //
   Bento<void (SlotClass::*)(SlotArgs...)> *slotMethod_Bento = new Bento<void (SlotClass::*)(SlotArgs...)>(slotMethod);

   std::unique_lock<std::mutex> senderLock {sender->m_mutex_ToReceiver};
   std::unique_lock<std::mutex> receiverLokc {receiver->m_mutex_FromSender};

   if (type & Qt::UniqueConnection) {
      // user passed enum to ensure the connection is not added twice

      for (auto index = sender->m_connectList_ToReceiver.begin(); index != sender->m_connectList_ToReceiver.end(); ++index) {
         const ConnectStruct &temp = *index;

         if (temp.sender == 0) {
            // connection is marked for deletion
            continue;
         }

         if (temp.receiver != receiver) {
            continue;
         }

         if (*(temp.signalMethod) != *(signalMethod_Bento))  {
            continue;
         }

         if (*(temp.slotMethod) != *(slotMethod_Bento))  {
            continue;
         }

         // connection already exists
         return false;
      }

      // change type
      type = static_cast<Qt::ConnectionType>(type & ~Qt::UniqueConnection);
   }

   sender->addConnection(signalMethod_Bento, receiver, slotMethod_Bento, type);

   return true;
}

template<class Sender>
void cs_testConnect_Sender()
{
   static_assert( std::is_base_of<QObject, Sender>::value,
                  "QObject Sender must inherit from QObject");
}

template<class Receiver>
void cs_testConnect_Receiver()
{
   static_assert( std::is_base_of<QObject, Receiver>::value,
                  "QObject Receiver must inherit from QObject");
}

template<class Sender, class SignalClass>
void cs_testConnect_SenderSignal()
{
   static_assert( std::is_base_of<SignalClass, Sender>::value,
                  "QObject Signal is not defined in the sender class");
}

template<class Slot_LambdaType, class ...SignalArgs>
void cs_testConnect_SignalSlotArgs_1()
{
   static_assert( cs_check_connect_args<void (*)(SignalArgs...), Slot_LambdaType>::value,
                  "QObject Incompatible signal/slot arguments");
}

template<class SlotClass, class Receiver>
void cs_testConnect_ReceiverSlot()
{
   static_assert( std::is_base_of<SlotClass, Receiver>::value,
                  "QObject Slot is not defined in the receiver class");
}

template<class Signal_ArgType, class Slot_ArgType>
void cs_testConnect_SignalSlotArgs_2()
{
   static_assert( cs_check_connect_args<Signal_ArgType, Slot_ArgType>::value,
                  "QObject Incompatible signal/slot arguments");
}

// signal method ptr, slot lambda
template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class T>
bool QObject::connect(const Sender *sender, void (SignalClass::*signalMethod)(SignalArgs...),
                      const Receiver *receiver, T slotLambda, Qt::ConnectionType type)
{
   // Sender must inhert from QObject
   cs_testConnect_Sender<Sender>();

   // Receiver must inhert from QObject
   cs_testConnect_Receiver<Receiver>();

   // Sender must be the same class as SignalClass and Sender is a child of SignalClass
   cs_testConnect_SenderSignal<Sender, SignalClass>();

   // compare signal and slot paramerter list
   cs_testConnect_SignalSlotArgs_1<T, SignalArgs...>();

   //
   if (sender == 0) {
      qWarning("QObject::connect() Can not connect, sender is null");
      return false;
   }

   if (receiver == 0) {
      qWarning("QObject::connect() Can not connect, receiver is null");
      return false;
   }

   if (signalMethod == 0) {
      qWarning("QObject::connect() Can not connect, signal is null");
      return false;
   }

   // get signal name
   const QMetaObject *senderMetaObject = sender->metaObject();

   const char *senderClass   = senderMetaObject->className();
   const char *receiverClass = receiver->metaObject()->className();

   QMetaMethod signalMetaMethod = senderMetaObject->method(signalMethod);

   QByteArray signature   = signalMetaMethod.methodSignature();
   const char *signalName = signature.constData();

   if (signature.isEmpty())  {
      qWarning("%s%s%s%s%s", "QObject::connect() ", senderClass, "::<Invalid Signal> ",
               " Unable to connect to receiver in ", receiverClass);
      return false;
   }

   // is signalMethod a signal
   if (signalMetaMethod.methodType() != QMetaMethod::Signal ) {
      qWarning("%s%s%s%s%s", "QObject::connect() ", senderClass, "::", signalName,
               " was not a valid signal");
      return false;
   }

   if (type == Qt::AutoCompatConnection) {
      type = Qt::AutoConnection;
   }

   //
   Bento<void (SignalClass::*)(SignalArgs...)> *signalMethod_Bento = new Bento<void (SignalClass::*)(SignalArgs...)>
   (signalMethod);
   Bento<T> *slotLambda_Bento = new Bento<T>(slotLambda);

   std::unique_lock<std::mutex> senderLock {sender->m_mutex_ToReceiver};
   std::unique_lock<std::mutex> receiverLokc {receiver->m_mutex_FromSender};

   if (type & Qt::UniqueConnection) {
      // user passed enum to ensure the connection is not added twice

      for (auto index = sender->m_connectList_ToReceiver.begin(); index != sender->m_connectList_ToReceiver.end(); ++index) {
         const ConnectStruct &temp = *index;

         if (temp.sender == 0) {
            // connection is marked for deletion
            continue;
         }

         if (temp.receiver != receiver) {
            continue;
         }

         if (*(temp.signalMethod) != *(signalMethod_Bento))  {
            continue;
         }

         // unable to test if the passed slotLambda = slotLambda_Bento

         // connection already exists
         return false;
      }

      // change type
      type = static_cast<Qt::ConnectionType>(type & ~Qt::UniqueConnection);
   }

   sender->addConnection(signalMethod_Bento, receiver, slotLambda_Bento, type);
   sender->connectNotify(signalMetaMethod);

   return true;
}

// signal & slot method ptr
template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class SlotClass, class ...SlotArgs, class SlotReturn>
bool QObject::disconnect(const Sender *sender, void (SignalClass::*signalMethod)(SignalArgs...),
                         const Receiver *receiver, SlotReturn (SlotClass::*slotMethod)(SlotArgs...))
{
   // Sender must inhert from QObject
   cs_testConnect_Sender<Sender>();

   // Receiver must inhert from QObject
   cs_testConnect_Receiver<Receiver>();

   // Sender must be the same class as SignalClass and Sender is a child of SignalClass
   cs_testConnect_SenderSignal<Sender, SignalClass>();

   // Receiver must be the same class as SlotClass and Receiver is a child of SlotClass
   cs_testConnect_ReceiverSlot<SlotClass, Receiver>();

   // signal & slot arguments do not agree
   cs_testConnect_SignalSlotArgs_2< void (*)(SignalArgs...), void (*)(SlotArgs...) >();


   // run time checks
   if (sender == 0 || (receiver == 0 && slotMethod != 0)) {
      qWarning("QObject::disconnect() Unexpected null parameter");
      return false;
   }

   //
   Bento<void (SignalClass::*)(SignalArgs...)> signalMethod_Bento(signalMethod);
   Bento<void (SlotClass::*)(SlotArgs...)> slotMethod_Bento(slotMethod);

   if (! QObject::internal_disconnect(sender, &signalMethod_Bento, receiver, &slotMethod_Bento)) {
      return false;
   }

   // calling the const char * (Qt4 API version)
   const QMetaObject *senderMetaObject = sender->metaObject();
   QByteArray signalName;

   if (senderMetaObject) {
      QMetaMethod signalMetaMethod = senderMetaObject->method(signalMethod);
      signalName = signalMetaMethod.methodSignature();

      // call the Qt5 API version
      const_cast<Sender *>(sender)->disconnectNotify(signalMetaMethod);
   }

   if (signalName.isEmpty())  {
      const_cast<Sender *>(sender)->disconnectNotify(0);

   } else {
      const_cast<Sender *>(sender)->disconnectNotify(signalName.constData());

   }

   return true;
}

// signal bento, slot method ptr
template<class Receiver, class SlotClass, class ...SlotArgs, class SlotReturn>
bool QObject::disconnect(const QObject *sender, BentoAbstract *signalMethod_Bento,
                         const Receiver *receiver, SlotReturn (SlotClass::*slotMethod)(SlotArgs...))
{
   // Receiver must inhert from QObject
   static_assert( std::is_base_of<QObject, Receiver>::value,
                  "QObject::disconnect()  Receiver must inherit from QObject");

   // (1) Receiver must be the same class as SlotClass OR (2) Receiver is a child of SlotClass
   static_assert( std::is_base_of<SlotClass, Receiver>::value,
                  "QObject::disconnect()  Slot was not a child class of Receiver");

   // run time checks
   if (sender == 0 || (receiver == 0 && slotMethod != 0)) {
      qWarning("QObject::disconnect()  Unexpected null parameter");
      return false;
   }

   //
   Bento<void (SlotClass::*)(SlotArgs...)> slotMethod_Bento(slotMethod);

   if (! QObject::internal_disconnect(sender, signalMethod_Bento, receiver, &slotMethod_Bento)) {
      return false;
   }

   const_cast<QObject *>(sender)->disconnectNotify(0);

   return true;
}

// lambda and function ptr
template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class T>
bool QObject::disconnect(const Sender *sender, void (SignalClass::*signalMethod)(SignalArgs...),
                         const Receiver *receiver, T slotMethod)
{
   // lambda, compile error
   static_assert(std::is_convertible<decltype(slotMethod == slotMethod), bool>::value,
                 "QObject::disconnect()  Slot argument invalid or calling disconnect using a lambda" );

   // function ptr
   Bento<void (SignalClass::*)(SignalArgs...)> signalMethod_Bento(signalMethod);
   Bento<T> slotMethod_Bento(slotMethod);

   if (! QObject::internal_disconnect(sender, &signalMethod_Bento, receiver, &slotMethod_Bento)) {
      return false;
   }

   return true;
}

template<class ...Ts>
bool cs_factory_interface_query(const char *data)
{
   std::vector<const char *> list = { qobject_interface_iid<Ts *>()... };

   for (unsigned int k = 0; k < list.size(); ++k) {

      if (strcmp(data, list[k]) == 0) {
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


// ***************************
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
   TeaCup_Data<typename std::remove_reference<Ts>::type...> dataPack(false, std::forward<Ts>(Vs)...);    

   if (type == Qt::DirectConnection) {
      // retval is passed by pointer
      m_bento->invoke(object, &dataPack, &retval);

   } else if (type == Qt::QueuedConnection) {

      if (! dynamic_cast<CSReturnArgument<void> *>(&retval)) {
         qWarning("QMetaMethod::invoke() Queued connections can not have a return value");
         return false;
      }

      // store the signal data, true indicates the data will be copied into a TeaCup Object (stored on the heap)
      CSMetaCallEvent *event = new CSMetaCallEvent(m_bento, new TeaCup_Data<Ts...>(true, std::forward<Ts>(Vs)...), 0, -1);
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
      CSMetaCallEvent *event = new CSMetaCallEvent(m_bento, new TeaCup_Data<Ts...>(false, std::forward<Ts>(Vs)...), 
            0, -1, &semaphore);
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
   TeaCup_Data<Ts...> dataPack(false, std::forward<Ts>(Vs)...);

   if (type == Qt::DirectConnection) {
      // invoke calls the method

      m_bento->invoke(object, &dataPack);

   } else if (type == Qt::QueuedConnection) {

      // store the signal data, false indicates the data will not be copied
      CSMetaCallEvent *event = new CSMetaCallEvent(m_bento, new TeaCup_Data<Ts...>(true, std::forward<Ts>(Vs)...), 0, -1);
      QCoreApplication::postEvent(object, event);

   } else {
      // blocking queued connection

      if (currentThread == objectThread) {
         qWarning("QMetaMethod::invoke() Dead lock detected in BlockingQueuedConnection, Receiver is %s(%p)",
                  m_metaObject->className(), object);
      }

      QSemaphore semaphore;

      // store the signal data, false indicates the data will not be copied
      CSMetaCallEvent *event = new CSMetaCallEvent(m_bento, new TeaCup_Data<Ts...>(false, std::forward<Ts>(Vs)...), 0, -1,
            &semaphore);
      QCoreApplication::postEvent(object, event);

      semaphore.acquire();
   }

   return true;
}

#endif
