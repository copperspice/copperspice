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

#include <qobject.h>
#include <csmeta_callevent.h>
#include <qorderedmutexlocker_p.h>
#include <qdebug.h>
#include <qthread_p.h>

#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QRegExp>

QT_BEGIN_NAMESPACE

std::atomic<int> QObject::m_objectCount {0};

QObject::QObject(QObject *t_parent)
{
   m_parent                   = 0;        // no parent yet, set by setParent()

   m_currentSender            = 0;
   m_currentChildBeingDeleted = 0;
   m_declarativeData          = 0;
   m_postedEvents             = 0;
   m_raceCount                = 0;
   m_sharedRefCount           = 0;

   m_activateBusy             = false;
   m_pendTimer                = false;    // no timers yet
   m_blockSig                 = false;    // allow signal to be emitted
   m_wasDeleted               = false;    // double delete flag

   m_sentChildRemoved         = false;
   m_sendChildEvents          = true;     // should send ChildInsert and ChildRemove events to parent
   m_receiveChildEvents       = true;
   m_inThreadChangeEvent      = false;

   //
   if (t_parent && ! t_parent->thread()) {
      m_threadData.store(t_parent->m_threadData.load());
   } else {
      m_threadData.store(QThreadData::current());
   }

   m_threadData.load()->ref();

   //
   if (t_parent) {

      try {

         if (! this->check_parent_thread(t_parent, t_parent->m_threadData, m_threadData)) {
            t_parent = 0;
         }

         this->setParent(t_parent);

      } catch (...) {
         m_threadData.load()->deref();

         QT_RETHROW;
      }
   }

   this->addObject();
}

QObject::~QObject()
{
   this->m_blockSig   = false;    // unblock signals so we always emit destroyed()

   // this code comes before anything else since a signal can not be emited from a dead object
   try {
      emit destroyed(this);

   } catch (...) {
      // all signal/slots connections are still in place
      // quit now, as the app will crash soon

      qWarning("QObject:~QObject() Detected an unexpected exception while emitting destroyed()");
      throw;
   }

   // this line needs to be located after the emit destroyed
   this->m_wasDeleted = true;

   QtSharedPointer::ExternalRefCountData *sharedRefCount = m_sharedRefCount.exchange(0);

   if (sharedRefCount) {

      if (sharedRefCount->strongref.load() > 0) {
         // continue deleting, unclear what else to do
         qWarning("QObject:~QObject()  Shared QObject was deleted directly, application may crash.");
      }

      // indicate to all QWeakPointers QObject has now been deleted
      sharedRefCount->strongref.store(0);

      if (! sharedRefCount->weakref.deref()) {
         delete sharedRefCount;
      }
   }

   //
   if (this->m_declarativeData) {
      CSAbstractDeclarativeData::destroyed(this->m_declarativeData, this);
   }

   // set ref to zero, indicates this object has been deleted
   if (this->m_currentSender != 0) {
      this->m_currentSender->ref = 0;
   }

   this->m_currentSender = 0;

   std::unique_lock<std::mutex> senderLock {this->m_mutex_ToReceiver};
   std::unique_lock<std::mutex> receiverLock {this->m_mutex_FromSender};

   // disconnect all receivers from sender
   for (auto index = this->m_connectList_ToReceiver.begin(); index != this->m_connectList_ToReceiver.end(); ++index) {
      const ConnectStruct &temp = *index;

      if (temp.sender == 0) {
         // connection is marked for deletion
         continue;
      }

      if (! temp.receiver) {
         continue;
      }

      std::unique_lock<std::mutex> tempReceiverLock {temp.receiver->m_mutex_FromSender, std::defer_lock};
      if (this != temp.receiver) {
         tempReceiverLock.lock();
      }

      for (int x = 0; x < temp.receiver->m_connectList_FromSender.count(); ++x)  {

         if (this == temp.receiver->m_connectList_FromSender[x].sender) {
            temp.receiver->m_connectList_FromSender.removeAt(x);

            // yes, this is required
            x = x - 1;
         }
      }
   }

   // disconnect all senders from receiver
   bool isDone = false;

   for (auto index = this->m_connectList_FromSender.begin(); index != this->m_connectList_FromSender.end(); ++index) {
      ConnectStruct &temp = *index;

      std::unique_lock<std::mutex> tempSenderLock {temp.sender->m_mutex_ToReceiver, std::defer_lock};
      if (this != temp.sender) {
         tempSenderLock.lock();
      }

      for (int x = 0; x < temp.sender->m_connectList_ToReceiver.count(); ++x)  {
         // connection is in another object to one of our slots
         ConnectStruct &connection = temp.sender->m_connectList_ToReceiver[x];

         if (connection.sender == 0) {
            // connection is marked for deletion
            continue;
         }

         if (this == connection.receiver) {

            if (temp.sender->m_activateBusy) {

               if (! isDone) {
                  // warns activate the connectList has changed
                  temp.sender->m_raceCount++;
                  isDone = true;
               }

               // mark connection for deletion, activate() will finish
               connection.sender = 0;

            } else {
               temp.sender->m_connectList_ToReceiver.removeAt(x);

               // yes, this is required
               x = x - 1;
            }
         }
      }
   }

   if (! this->m_children.isEmpty()) {
      this->deleteChildren();
   }

   this->removeObject();

   if (this->m_parent)  {
      // remove 'this' from parent object
      this->setParent(0);
   }

   QThreadData *threadData = m_threadData.load();

   if (m_pendTimer) {
      // unregister pending timers

      if (threadData && threadData->eventDispatcher) {
         threadData->eventDispatcher->unregisterTimers(this);
      }
   }

   if (m_postedEvents) {
      QCoreApplication::removePostedEvents(this, 0);
   }

   if (threadData)  {
      threadData->deref();
   }
}

// **
void QObject::addConnection(const BentoAbstract *signalMethod, const QObject *receiver,
                            const BentoAbstract *slotMethod, Qt::ConnectionType type) const
{
   struct ConnectStruct tempStruct;

   tempStruct.sender       = this;
   tempStruct.signalMethod = signalMethod;
   tempStruct.receiver     = receiver;
   tempStruct.slotMethod   = slotMethod;
   tempStruct.type         = type;

   // list is in sender
   this->m_connectList_ToReceiver.append(tempStruct);

   if (m_activateBusy) {
      // warns activate the connectList has changed
      this->m_raceCount++;
   }

   // list is in receiver
   receiver->m_connectList_FromSender.append(tempStruct);
}

void QObject::addObject()
{
   m_objectCount++;
}

bool QObject::blockSignals(bool block)
{
   bool oldValue = m_blockSig;
   m_blockSig    = block;

   return oldValue;
}

// check the constructor's parent thread arguments
bool QObject::check_parent_thread(QObject *parent, QThreadData *parentThreadData, QThreadData *currentThreadData)
{
   if (parent && parentThreadData != currentThreadData) {

      QThread *parentThread  = parentThreadData->thread;
      QThread *currentThread = currentThreadData->thread;

      qWarning("QObject:check_parent_thread() Can not create children for a parent in a different thread.\n"
               "(Parent is %s(%p), parent's thread is %s(%p), current thread is %s(%p)",
               parent->metaObject()->className(), parent,
               parentThread ? parentThread->metaObject()->className() : "QThread",
               parentThread, currentThread ? currentThread->metaObject()->className() : "QThread", currentThread);

      return false;
   }

   return true;
}

const QList<QObject *> &QObject::children() const
{
   return m_children;
}

void QObject::childEvent(QChildEvent *)
{
   // no code is suppose to appear here
}

bool QObject::connect(const QObject *sender, const char *signalMethod, const char *location, const char *slotMethod,
                      Qt::ConnectionType type)
{
   // default third param (receiver)
   return connect(sender, signalMethod, this, slotMethod, type, location);
}

bool QObject::connect(const QObject *sender, const char *signalMethod, const char *location, const QObject *receiver,
                      const char *slotMethod, Qt::ConnectionType type)
{
   // location is generated by the SIGNAL macro, indicates where connect() was called
   return connect(sender, signalMethod, receiver, slotMethod, type, location);
}

bool QObject::connect(const QObject *sender, const char *signalMethod, const QObject *receiver, const char *slotMethod,
                      Qt::ConnectionType type, const char *location)
{
   const QMetaObject *senderMetaObject = sender->metaObject();
   int sIndex = senderMetaObject->indexOfSignal(signalMethod);

   const QMetaObject *receiverMetaObject = receiver->metaObject();
   int rIndex = receiverMetaObject->indexOfMethod(slotMethod);

   if (sIndex == -1 || rIndex == -1)  {
      const char *senderClass   = senderMetaObject->className();
      const char *receiverClass = receiverMetaObject->className();

      if (location) {
         qWarning("%s%s%s%s%s%s%s%s%s %s%d%s%d", "QObject::connect() ", senderClass, "::", signalMethod,
                  " Unable to connect to receiver in ", receiverClass, " (", location, ")",
                  " Signal Index: ", sIndex, " Slot Index: ", rIndex);

      } else {
         qWarning("%s%s%s%s%s%s %s%d%s%d", "QObject::connect() ", senderClass, "::", signalMethod,
                  " Unable to connect to receiver in ", receiverClass,
                  " Signal Index: ", sIndex, " Slot Index: ", rIndex);
      }

#ifdef CS_Debug
      qDebug("");
      for (int k = 0; k < senderMetaObject->methodCount(); ++k) {
         qDebug("QObject::connect()  Class %s has method %s", senderMetaObject->className(),
                senderMetaObject->method(k).methodSignature().constData() );
      }
      qDebug("");
#endif

      return false;
   }

   return QObject::connect(sender, senderMetaObject->method(sIndex), receiver, receiverMetaObject->method(rIndex), type);
}

bool QObject::connect(const QObject *sender, const QMetaMethod &signalMetaMethod, const QObject *receiver,
                      const QMetaMethod &slotMetaMethod, Qt::ConnectionType type)
{
   if (sender == 0) {
      qWarning("QObject::connect() Can not connect as sender is null");
      return false;
   }

   if (receiver == 0) {
      qWarning("QObject::connect() Can not connect as receiver is null");
      return false;
   }
   // get signal name
   const char *senderClass   = sender->metaObject()->className();
   const char *receiverClass = receiver->metaObject()->className();

   QByteArray signalTemp = signalMetaMethod.methodSignature();
   QByteArray slotTemp   = slotMetaMethod.methodSignature();

   const char *signalName = signalTemp.constData();
   const char *slotName   = slotTemp.constData();

   if (! signalName || signalName[0] == 0)  {
      qWarning("%s%s%s%s%s", "QObject::connect() ", senderClass, "::<Invalid Signal> ",
               " Unable to connect to receiver in ", receiverClass);

      return false;
   }

   if (! slotName || slotName[0] == 0 )  {
      qWarning("%s%s%s%s%s%s%s", "QObject::connect() ", senderClass, "::", signalName,
               " Unable to connect to receiver in ", receiverClass, "::<Invalid Slot>");
      return false;
   }

   // is signalMethod a signal
   if (signalMetaMethod.methodType() != QMetaMethod::Signal) {
      qWarning("%s%s%s%s%s", "QObject::connect() ", senderClass, "::", signalName,
               ": Is not a valid signal");
      return false;
   }

   // is slotMethod a clone, then there is a defualt parameter
   if (slotMetaMethod.attributes() & QMetaMethod::Cloned) {
      qWarning("%s%s%s%s%s", "QObject::connect() ", receiverClass, "::", slotName,
               ": Unable to connect to a slot with a default parameter");
      return false;
   }

   // test arguments
   bool err = false;
   QList<QByteArray> typesSignal = signalMetaMethod.parameterTypes();
   QList<QByteArray> typesSlot   = slotMetaMethod.parameterTypes();

   if (typesSignal.count() < typesSlot.count() )  {
      err = true;

   } else {

      for (int index = 0; index != typesSlot.count(); ++index)   {

         if (typesSignal.at(index) != typesSlot.at(index)) {
            // unable to test if typeDefs are used
            err = true;
            break;
         }

      }
   }

   if (err) {
      qWarning("%s%s%s%s%s%s%s%s", "QObject::connect() ", senderClass, "::", signalName,
               ": Incompatible signal/slot arguments ", receiverClass, "::", slotName);
      return false;
   }

   //
   if (type == Qt::AutoCompatConnection) {
      type = Qt::AutoConnection;
   }

   //
   const BentoAbstract *signalMethod_Bento = signalMetaMethod.getBentoBox();
   const BentoAbstract *slotMethod_Bento   = slotMetaMethod.getBentoBox();

   if (! signalMethod_Bento) {
      qWarning("%s%s%s%s%s", "QObject::connect() ", senderClass, "::", signalName,
               " Unable to locate the requested signal, verify connect arguments and signal declaration");
      return false;
   }

   if (! slotMethod_Bento) {
      qWarning("%s%s%s%s%s", "QObject::connect() ", receiverClass, "::", slotName,
               " Unable to locate the requested slot, verify connect arguments and slot declaration");
      return false;
   }

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

bool QObject::connect(const QObject *sender, const char *signalMethod, const char *slotMethod, Qt::ConnectionType type)
{
   return QObject::connect(sender, signalMethod, this, slotMethod, type);
}

void QObject::connectNotify(const char *signal) const
{
   // no code is suppose to appear here
}

void QObject::connectNotify(const QMetaMethod &signal) const
{
   // no code is suppose to appear here
}

void QObject::customEvent(QEvent *)
{
   // no code is suppose to appear here
}

void QObject::deleteChildren()
{
   const bool reallyWasDeleted = m_wasDeleted;
   m_wasDeleted = true;

   // do not use qDeleteAll as the destructor of the child might delete siblings

   for (int i = 0; i < m_children.count(); ++i) {
      m_currentChildBeingDeleted = m_children.at(i);
      m_children[i] = 0;
      delete m_currentChildBeingDeleted;
   }

   m_children.clear();
   m_currentChildBeingDeleted = 0;

   m_wasDeleted = reallyWasDeleted;
}

void QObject::deleteLater()
{
   QCoreApplication::postEvent(this, new QEvent(QEvent::DeferredDelete));
}

bool QObject::disconnect(const QObject *sender,   const char *signalMethod, const QObject *receiver,
                         const char *slotMethod)
{
   bool retval = false;

   if (sender == 0) {
      qWarning("QObject::disconnect() Can not disconnect as sender is null");
      return false;

   } else if (receiver == 0 && slotMethod != 0) {
      qWarning("QObject::disconnect() Can not disconnect as receiver is null and slot was specified");
      return false;

   }

   QByteArray signal_name;
   bool signal_found = false;

   if (signalMethod) {

      try {
         signal_name  = QMetaObject::normalizedSignature(signalMethod);
         signalMethod = signal_name.constData();

      } catch (const std::bad_alloc &) {
         // if the signal is already normalized, we can continue

         if (sender->metaObject()->indexOfSignal(signalMethod) == -1) {
            throw;
         }
      }
   }

   QByteArray slot_name;
   bool slot_found = false;

   if (slotMethod) {

      try {
         slot_name  = QMetaObject::normalizedSignature(slotMethod);
         slotMethod = slot_name.constData();

      } catch (const std::bad_alloc &) {
         // if the method is already normalized, we can continue.
         if (receiver->metaObject()->indexOfMethod(slotMethod) == -1) {
            throw;
         }
      }
   }

   // iterate through all the sender's and receiver's meta objects to disconnect
   // signals and slots with the same signature

   const QMetaObject *senderMetaObject = sender->metaObject();

   while (true) {
      int signal_index = -1;

      if (signalMethod) {
         signal_index = senderMetaObject->indexOfSignal(signalMethod);

         if (signal_index < 0) {
            break;
         }

         signal_found = true;
      }

      if (slotMethod) {
         const QMetaObject *receiverMetaObject = receiver->metaObject();

         while (true)  {
            int slot_index = receiverMetaObject->indexOfMethod(slotMethod);

            if (slot_index < 0) {
               break;
            }

            slot_found = true;

            const BentoAbstract *signal_Bento = 0;
            if (signal_index != -1) {
               signal_Bento = senderMetaObject->method(signal_index).getBentoBox();
            }

            const BentoAbstract *slot_Bento = receiverMetaObject->method(slot_index).getBentoBox();

            retval |= QObject::internal_disconnect(sender, signal_Bento, receiver, slot_Bento);

            // parent of receiver
            receiverMetaObject = receiverMetaObject->superClass();

            if (! receiverMetaObject) {
               break;
            }
         }

      } else   {
         const BentoAbstract *signal_Bento = 0;
         if (signal_index != -1) {
            signal_Bento = senderMetaObject->method(signal_index).getBentoBox();
         }

         retval |= QObject::internal_disconnect(sender, signal_Bento, receiver, 0);
      }

      //
      if (! signalMethod) {
         break;
      }

      // parent of sender
      senderMetaObject = senderMetaObject->superClass();

      if (! senderMetaObject) {
         break;
      }
   }

   if (signalMethod && ! signal_found) {
      qWarning("QObject::disconnect() No registered signal, %s::%s", sender->metaObject()->className(), signalMethod);

      QString ss;
      if (sender) {
         ss = sender->objectName();
      }

      QString rr;
      if (receiver) {
         ss = receiver->objectName();
      }

      qWarning("QObject::disconnect() No registered signal, Sender name: '%s', Receiver name: '%s'",
               ss.toLocal8Bit().data(), rr.toLocal8Bit().data() );

   } else if (slotMethod && ! slot_found) {
      qWarning("QObject::disconnect() No registered slot, %s::%s", receiver->metaObject()->className(), slotMethod);

      QString ss;
      if (sender) {
         ss = sender->objectName();
      }

      QString rr;
      if (receiver) {
         ss = receiver->objectName();
      }

      qWarning("QObject::disconnect() No registered slot, Sender name: '%s', Receiver name: '%s'",
               ss.toLocal8Bit().data(), rr.toLocal8Bit().data() );
   }

   if (retval) {
      // calling the const char * (Qt4 API version)

      if (signalMethod) {
         const_cast<QObject *>(sender)->disconnectNotify(signalMethod);

      } else {
         const_cast<QObject *>(sender)->disconnectNotify(0);

      }
   }

   return retval;
}

bool QObject::disconnect(const QObject *sender,   const QMetaMethod &signalMethod,
                         const QObject *receiver, const QMetaMethod &slotMethod)
{
   const QMetaObject *signalMetaObject = signalMethod.getMetaObject();
   const QMetaObject *slotMetaObject   = slotMethod.getMetaObject();

   // run time checks
   if (sender == 0) {
      qWarning("QObject::disconnect() Can not disconnect as sender is null");
      return false;

   } else if (receiver == 0 && slotMetaObject != 0) {
      qWarning("QObject::disconnect() Can not disconnect as receiver is null and slot was specified");
      return false;
   }

   if (signalMetaObject) {

      if (signalMethod.methodType() != QMetaMethod::Signal) {
         qWarning("QObject::disconnect() Can not disconnect %s::%s, is not a signal",
                  sender->metaObject()->className(), signalMethod.methodSignature().constData());
         return false;
      }
   }

   if (slotMetaObject) {

      if (slotMethod.methodType() == QMetaMethod::Constructor) {
         qWarning("QObject::disconnect() Can not use constructor as an argument %s::%s",
                  receiver->metaObject()->className(), slotMethod.methodSignature().constData());
         return false;
      }
   }

   int signal_index = sender->metaObject()->indexOfSignal(signalMethod.methodSignature().constData());

   // if signalMethod is not empty and signal_index is -1, then signal is not a member of sender
   if (signalMetaObject != 0 && signal_index == -1) {
      qWarning("QObject::disconnect() Signal %s was not found in class %s",
               signalMethod.methodSignature().constData(), sender->metaObject()->className());
      return false;
   }

   // method is not a member of receiver
   if (receiver) {
      int slot_index = receiver->metaObject()->indexOfMethod(slotMethod.methodSignature().constData());

      if (slotMetaObject && slot_index == -1) {
         qWarning("QObject::disconnect() Method %s was not found in class %s",
                  slotMethod.methodSignature().constData(), receiver->metaObject()->className());
         return false;
      }
   }

   const BentoAbstract *signal_Bento = signalMethod.getBentoBox();
   const BentoAbstract *slot_Bento   = slotMethod.getBentoBox();

   if (! QObject::internal_disconnect(sender, signal_Bento, receiver, slot_Bento)) {
      return false;
   }

   // calling the const char * (Qt4 API version)
   if (signalMetaObject) {
      const_cast<QObject *>(sender)->disconnectNotify(signalMethod.methodSignature().constData());

   } else {
      const_cast<QObject *>(sender)->disconnectNotify(0);

   }

   return true;
}

bool QObject::disconnect(const QObject *sender,   const char *signalMethod, const char *location,
                         const QObject *receiver, const char *slotMethod)
{
   return QObject::disconnect(sender, signalMethod, receiver, slotMethod);
}

bool QObject::disconnect(const char *signalMethod, const QObject *receiver, const char *slotMethod) const
{
   return QObject::disconnect(this, signalMethod, receiver, slotMethod);
}

bool QObject::disconnect(const char *signalMethod, const char *lineNumber, const QObject *receiver,
                         const char *slotMethod) const
{
   return QObject::disconnect(this, signalMethod, receiver, slotMethod);
}

bool QObject::disconnect(const QObject *receiver, const char *slotMethod) const
{
   return QObject::disconnect(this, 0, receiver, slotMethod);
}

void QObject::disconnectNotify(const char *signal) const
{
   // no code is suppose to appear here
}

void QObject::disconnectNotify(const QMetaMethod &signal) const
{
   // no code is suppose to appear here
}

QList<QByteArray> QObject::dynamicPropertyNames() const
{
   return m_extra_propertyNames;
}

bool QObject::event(QEvent *e)
{
   switch (e->type()) {

      case QEvent::Timer:
         timerEvent((QTimerEvent *) e);
         break;

      case QEvent::ChildAdded:
      case QEvent::ChildPolished:
      case QEvent::ChildRemoved:
         childEvent((QChildEvent *)e);
         break;

      case QEvent::DeferredDelete:
         delete this;
         break;

      case QEvent::MetaCall: {
         CSMetaCallEvent *metaCallEvent = dynamic_cast<CSMetaCallEvent *>(e);

         QObject::SenderStruct currentSender;
         currentSender.sender       = const_cast<QObject *>(metaCallEvent->sender());
         currentSender.signal_index = metaCallEvent->signal_index();
         currentSender.ref          = 1;

         QObject::SenderStruct *previousSender = QObject::setCurrentSender(this, &currentSender);

         try {
            metaCallEvent->placeMetaCall(this);

         } catch (...) {
            QObject::resetCurrentSender(this, &currentSender, previousSender);
            throw;
         }

         QObject::resetCurrentSender(this, &currentSender, previousSender);
         break;
      }

      case QEvent::ThreadChange: {
         QThreadData *threadData = m_threadData;
         QAbstractEventDispatcher *eventDispatcher = threadData->eventDispatcher;

         if (eventDispatcher) {
            QList< std::pair<int, int> > timerList = eventDispatcher->registeredTimers(this);

            if (! timerList.isEmpty()) {
               // set m_inThreadChangeEvent to true tells the dispatcher not to release timer
               // ids back to the pool (since the timer ids are moving to a new thread)

               m_inThreadChangeEvent = true;
               eventDispatcher->unregisterTimers(this);
               m_inThreadChangeEvent = false;

               // using csArgument directly since the datatype contains a comma
               QMetaObject::invokeMethod(this, "internal_reregisterTimers", Qt::QueuedConnection,
                                         CSArgument<QList< std::pair<int, int> > > (timerList) );
            }
         }

         break;
      }

      default:
         if (e->type() >= QEvent::User) {
            customEvent(e);
            break;
         }

         return false;
   }

   return true;
}

bool QObject::eventFilter(QObject *watched, QEvent *event)
{
   // no code is suppose to appear here
   return false;
}

void QObject::installEventFilter(QObject *obj)
{
   if (! obj)  {
      return;
   }

   if (m_threadData.load() != obj->m_threadData.load()) {
      qWarning("QObject::installEventFilter() Can not filter events for objects in a different thread");
      return;
   }

   // clean up unused items in the list
   m_eventFilters.removeAll((QObject *) 0);
   m_eventFilters.removeAll(obj);
   m_eventFilters.prepend(obj);
}

bool QObject::inherits(const char *classname) const
{
   bool retval = false;

   const QMetaObject *metaObj = this->metaObject();

   while (metaObj)  {

      if (qstrcmp(metaObj->className(), classname) == 0)  {
         retval = true;
         break;
      }

      // get parent
      metaObj = metaObj->superClass();
   }

   return retval;
}

// used by QAccessibleWidget
bool QObject::isSender(const QObject *receiver, const char *signalMethod) const
{
   if (signalMethod) {
      QByteArray signal_name = QMetaObject::normalizedSignature(signalMethod);
      const QMetaObject *metaObj = this->metaObject();

      int index = metaObj->indexOfSignal(signal_name);

      if (index == -1)  {
         return false;
      }

      QMetaMethod metaMethod = metaObj->method(index);
      const BentoAbstract *signalMethod_Bento = metaMethod.getBentoBox();

      std::unique_lock<std::mutex> senderLock {this->m_mutex_ToReceiver};

      for (auto k = this->m_connectList_ToReceiver.begin(); k != this->m_connectList_ToReceiver.end(); ++k) {
         const ConnectStruct &temp = *k;

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

         // found a match
         return true;
      }
   }

   return false;
}

bool QObject::isSignalConnected(const BentoAbstract &signalMethod_Bento) const
{
   bool retval = false;

   std::unique_lock<std::mutex> senderLock {this->m_mutex_ToReceiver};

   for (auto index = this->m_connectList_ToReceiver.begin(); index != this->m_connectList_ToReceiver.end(); ++index) {
      const ConnectStruct &temp = *index;

      if (*(temp.signalMethod) != signalMethod_Bento)  {
         continue;
      }

      if (temp.sender == 0) {
         // connection is marked for deletion
         continue;
      }

      retval = true;
      break;
   }

   return retval;
}

bool QObject::isSignalConnected(const QMetaMethod &signalMetaMethod) const
{
   bool retval = false;
   const BentoAbstract *signalMethod_Bento = signalMetaMethod.getBentoBox();

   std::unique_lock<std::mutex> senderLock {this->m_mutex_ToReceiver};

   for (auto index = this->m_connectList_ToReceiver.begin(); index != this->m_connectList_ToReceiver.end(); ++index) {
      const ConnectStruct &temp = *index;

      if (*(temp.signalMethod) != *(signalMethod_Bento))  {
         continue;
      }

      if (temp.sender == 0) {
         // connection is marked for deletion
         continue;
      }

      retval = true;
      break;
   }

   return retval;
}

bool QObject::isWidgetType() const
{
   return cs_isWidgetType();
}

bool QObject::cs_isWidgetType() const
{
   return false;
}

void QObject::killTimer(int id)
{
   QThreadData *threadData = m_threadData.load();

   if (threadData->eventDispatcher) {
      threadData->eventDispatcher->unregisterTimer(id);
   }
}

QString QObject::objectName() const
{
   return m_objectName;
}

void QObject::moveToThread(QThread *targetThread)
{
   if (m_threadData.load()->thread == targetThread) {
      // object is already in this thread
      return;
   }

   if (m_parent != 0) {
      qWarning("QObject::moveToThread() Can not move an object with a parent");
      return;
   }

   if (isWidgetType()) {
      qWarning("QObject::moveToThread() Widgets can not be moved to a new thread");
      return;
   }

   QThreadData *currentData = QThreadData::current();
   QThreadData *targetData = targetThread ? QThreadData::get2(targetThread) : new QThreadData(0);

   QThreadData *threadData = m_threadData.load();

   if (threadData->thread == 0 && currentData == targetData) {
      // exception: allow moving objects with no thread affinity to the current thread
      currentData = threadData;

   } else if (threadData != currentData) {

      qWarning("QObject::moveToThread() Current thread (%p) is not the current object's thread (%p).\n"
               "Can not move to target thread (%p)\n", currentData->thread, threadData->thread, targetData->thread);

#ifdef Q_OS_MAC

      qWarning("Multiple libraries might be loaded in the same process. Verify all plugins are "
               "linked with the correct binaries. Export DYLD_PRINT_LIBRARIES=1 and verify only one set of "
               "binaries are being loaded.");
#endif

      return;
   }

   // prepare to move
   this->moveToThread_helper();

   QOrderedMutexLocker locker(&currentData->postEventList.mutex, &targetData->postEventList.mutex);

   // keep currentData alive
   currentData->ref();

   // move the object
   this->setThreadData_helper(currentData, targetData);

   locker.unlock();
   currentData->deref();
}

void QObject::moveToThread_helper()
{
   QEvent event(QEvent::ThreadChange);
   QCoreApplication::sendEvent(this, &event);

   for (int k = 0; k < m_children.size(); ++k) {
      QObject *child = m_children.at(k);
      child->moveToThread_helper();
   }
}

QDebug operator<<(QDebug debug, const QObject *object)
{
   QByteArray msg;

   if (object) {

      msg =  QByteArray(object->metaObject()->className()) + "(";
      msg += QByteArray::number(static_cast<qulonglong>(object - static_cast<const QObject *>(0)), 16);

      if (! object->objectName().isEmpty())  {
         msg += ", name = ";
         msg += object->objectName();
      }

      msg += ")";


   } else {
      msg = "QObject(nullptr) ";

   }

   return debug << msg;
}


QObject *QObject::parent() const
{
   return m_parent;
}

QVariant QObject::property(const char *name) const
{
   const QMetaObject *metaObj = metaObject();

   if (! name || ! metaObj) {
      return QVariant();
   }

   int index = metaObj->indexOfProperty(name);

   if (index < 0) {
      const int k = m_extra_propertyNames.indexOf(name);

      if (k == -1) {
         return QVariant();
      }

      return m_extra_propertyValues.value(k);
   }

   QMetaProperty p = metaObj->property(index);

   if (! p.isReadable()) {
      qWarning("%s::property() Property \"%s\" is invalid or does not exist", metaObj->className(), name);
   }

   return p.read(this);
}

// used by QAccessibleWidget
QList<QObject *> QObject::receiverList(const char *signalMethod) const
{
   QList<QObject *> retval;

   if (signalMethod) {
      QByteArray signal_name = QMetaObject::normalizedSignature(signalMethod);
      const QMetaObject *metaObj = this->metaObject();

      int index = metaObj->indexOfSignal(signal_name);

      if (index == -1)  {
         return retval;
      }

      const QMetaMethod signalMetaMethod = metaObj->method(index);
      const BentoAbstract *signalMethod_Bento = signalMetaMethod.getBentoBox();

      std::unique_lock<std::mutex> senderLock {this->m_mutex_ToReceiver};

      for (auto k = this->m_connectList_ToReceiver.begin(); k != this->m_connectList_ToReceiver.end(); ++k) {
         const ConnectStruct &temp = *k;

         if (! temp.receiver) {
            continue;
         }

         if (*(temp.signalMethod) != *(signalMethod_Bento))  {
            continue;
         }

         if (temp.sender == 0) {
            // connection is marked for deletion
            continue;
         }

         retval.append(const_cast<QObject *>(temp.receiver));
      }
   }

   return retval;
}

int QObject::receivers(const char *signalMethod) const
{
   int receivers = 0;
   int index = 0;

   if (signalMethod) {
      QByteArray signal_name = QMetaObject::normalizedSignature(signalMethod);
      const QMetaObject *metaObj = this->metaObject();

      index = metaObj->indexOfSignal(signal_name);

      if (index != -1)  {
         QMetaMethod metaMethod = metaObj->method(index);
         const BentoAbstract *signalMethod_Bento = metaMethod.getBentoBox();

         std::unique_lock<std::mutex> senderLock {this->m_mutex_ToReceiver};

         for (auto index = this->m_connectList_ToReceiver.begin(); index != this->m_connectList_ToReceiver.end(); ++index) {
            const ConnectStruct &temp = *index;

            if (*(temp.signalMethod) != *(signalMethod_Bento))  {
               continue;
            }

            if (temp.sender == 0) {
               // connection is marked for deletion
               continue;
            }

            receivers++;
         }
      }

   }

   // Qt5
   if (m_declarativeData && CSAbstractDeclarativeData::receivers) {
      receivers += CSAbstractDeclarativeData::receivers(m_declarativeData, this, index);
   }

   return receivers;
}

void QObject::resetCurrentSender(QObject *receiver, SenderStruct *newSender, SenderStruct *oldSender)
{
   // ref is set to zero when this object is deleted during the metacall
   if (newSender->ref == 1)  {
      receiver->m_currentSender = oldSender;
   }

   if (oldSender)  {
      oldSender->ref = newSender->ref;
   }
}

void QObject::removeEventFilter(QObject *obj)
{
   for (int i = 0; i < m_eventFilters.count(); ++i) {

      if (m_eventFilters.at(i) == obj) {
         m_eventFilters[i] = 0;
      }
   }
}

void QObject::removeObject()
{
}

QObject *QObject::sender() const
{
   std::unique_lock<std::mutex> senderLock {this->m_mutex_ToReceiver};

   if (! m_currentSender)  {
      return 0;
   }

   return m_currentSender->sender;
}

// used by QAccessibleWidget
QList<QObject *> QObject::senderList() const
{
   QList<QObject *> retval;

   std::unique_lock<std::mutex> receiverLock {this->m_mutex_FromSender};

   for (auto k = this->m_connectList_FromSender.begin(); k != this->m_connectList_FromSender.end(); ++k) {
      const ConnectStruct &temp = *k;
      retval.append(const_cast<QObject *>(temp.sender));
   }

   return retval;
}

int QObject::senderSignalIndex() const
{
   std::unique_lock<std::mutex> senderLock {this->m_mutex_ToReceiver};

   if (! m_currentSender) {
      return -1;
   }

   return m_currentSender->signal_index;
}

QObject::SenderStruct *QObject::setCurrentSender(QObject *receiver, SenderStruct *sender)
{
   SenderStruct *oldSender   = receiver->m_currentSender;
   receiver->m_currentSender = sender;

   return oldSender;
}

void QObject::setObjectName(const QString &name)
{
   if (m_objectName != name)   {
      m_objectName = name;
      emit objectNameChanged(m_objectName);
   }
}

void QObject::cs_forceRemoveChild()
{
   if (m_parent && m_parent->m_receiveChildEvents) {
      QChildEvent e(QEvent::ChildRemoved, this);
      QCoreApplication::sendEvent(m_parent, &e);

      m_sentChildRemoved = true;
   }
}

void QObject::setParent(QObject *newParent)
{
   // QWidget overrides this methods

   if (newParent == m_parent) {
      return;
   }

   if (m_parent) {
      // remove the current object from the old m_parent m_children list

      if (m_parent->m_wasDeleted && m_wasDeleted && m_parent->m_currentChildBeingDeleted == this) {
         // QObject::deleteChildren() cleared our entry for m_parent->m_children

      } else {
         const int index = m_parent->m_children.indexOf(this);

         if (m_parent->m_wasDeleted) {
            m_parent->m_children[index] = 0;

         } else {
            m_parent->m_children.removeAt(index);

            if (m_sendChildEvents && m_parent->m_receiveChildEvents && ! m_sentChildRemoved) {
               QChildEvent e(QEvent::ChildRemoved, this);
               QCoreApplication::sendEvent(m_parent, &e);
            }
         }
      }
   }

   m_parent = newParent;

   if (m_parent) {
      // object hierarchies are constrained to a single thread

      if (m_threadData.load() != m_parent->m_threadData.load()) {
         qWarning("QObject::setParent() Can not set parent, new parent is in a different thread");
         m_parent = 0;
         return;
      }

      m_parent->m_children.append(this);

      if (m_sendChildEvents && m_parent->m_receiveChildEvents) {
         if (! isWidgetType()) {
            QChildEvent e(QEvent::ChildAdded, this);
            QCoreApplication::sendEvent(m_parent, &e);
         }
      }
   }

   if (! m_wasDeleted && m_declarativeData) {
      CSAbstractDeclarativeData::parentChanged(m_declarativeData, this, newParent);
   }
}

bool QObject::setProperty(const char *name, const QVariant &value)
{
   const QMetaObject *metaObj = metaObject();

   if (! name || ! metaObj) {
      return false;
   }

   int index = metaObj->indexOfProperty(name);

   if (index < 0) {
      const int k = m_extra_propertyNames.indexOf(name);

      if (value.isValid()) {
         // add or update dynamic property

         if (k == -1) {
            m_extra_propertyNames.append(name);
            m_extra_propertyValues.append(value);

         } else {
            m_extra_propertyValues[k] = value;

         }

      } else {
         // remove dynamic property

         if (k == -1) {
            return false;
         }

         m_extra_propertyNames.removeAt(k);
         m_extra_propertyValues.removeAt(k);
      }

      QDynamicPropertyChangeEvent event(name);
      QCoreApplication::sendEvent(this, &event);

      return false;
   }

   QMetaProperty p = metaObj->property(index);

   if (! p.isWritable()) {
      qWarning("%s::setProperty() Property \"%s\" is invalid, read only, or does not exist", metaObj->className(), name);
   }

   //
   bool retval = p.write(this, value);

   if (! retval) {
      qWarning("%s::setProperty() Set property \"%s\" failed. Passed value is of type %s, property is of type %s",
               metaObj->className(), name, value.typeName(), p.typeName() );
   }

   return retval;
}

void QObject::setThreadData_helper(QThreadData *currentData, QThreadData *targetData)
{
   // move posted events
   int eventsMoved = 0;

   for (int i = 0; i < currentData->postEventList.size(); ++i) {
      const QPostEvent &postedEvent = currentData->postEventList.at(i);

      if (! postedEvent.event) {
         continue;
      }

      if (postedEvent.receiver == this) {
         // move this post event to the targetList
         targetData->postEventList.addEvent(postedEvent);
         const_cast<QPostEvent &>(postedEvent).event = 0;
         ++eventsMoved;
      }
   }

   if (eventsMoved > 0 && targetData->eventDispatcher) {
      targetData->canWait = false;
      targetData->eventDispatcher->wakeUp();
   }

   // the current emitting thread shouldn't restore currentSender after calling moveToThread()
   if (m_currentSender)  {
      m_currentSender->ref = 0;
   }

   m_currentSender = 0;

   // set new thread data
   targetData->ref();

   QThreadData *threadData = m_threadData.exchange(targetData);
   threadData->deref();

   for (int k = 0; k < m_children.size(); ++k) {
      QObject *child = m_children.at(k);
      child->setThreadData_helper(currentData, targetData);
   }
}

bool QObject::signalsBlocked() const
{
   return m_blockSig;
}

int QObject::startTimer(int interval)
{
   // timer flag hasTimer set when startTimer is called.
   // not reset when killing the timer because more than one timer might be active

   if (interval < 0) {
      qWarning("QObject::startTimer() QTimer can not have a negative interval");
      return 0;
   }

   // set timer flag
   m_pendTimer = true;

   QThreadData *threadData = m_threadData.load();

   if (! threadData->eventDispatcher) {
      qWarning("QObject::startTimer() QTimer can only be used with threads started with QThread");
      return 0;
   }
   return threadData->eventDispatcher->registerTimer(interval, this);
}

QThread *QObject::thread() const
{
   return m_threadData.load()->thread;
}

void QObject::timerEvent(QTimerEvent *)
{
   // no code is suppose to appear here
}

bool QObject::cs_InstanceOf(const char *iid)
{
   // check if the iid is part of this class

   if (! iid) {
      return false;
   }

   const QMetaObject *metaObject = this->metaObject();

   while (metaObject != nullptr)  {

      // 1
      const char *className = metaObject->className();

      // test 1
      if (strcmp(iid, className) == 0) {
         return true;
      }

      metaObject = metaObject->superClass();
   }

   // test 2
   if (this->cs_interface_query(iid)) {
      return true;
   }

   return false;
}

bool QObject::cs_interface_query(const char *data) const
{
   return false;
}

QMap<std::type_index, QMetaObject *> &QObject::m_metaObjectsAll()
{
   static QMap<std::type_index, QMetaObject *> metaObjects;
   return metaObjects;
}


// **
void (*CSAbstractDeclarativeData::destroyed)(CSAbstractDeclarativeData *, QObject *) = 0;
void (*CSAbstractDeclarativeData::parentChanged)(CSAbstractDeclarativeData *, QObject *, QObject *) = 0;
void (*CSAbstractDeclarativeData::signalEmitted)(CSAbstractDeclarativeData *, QObject *, int, void **) = 0;
int  (*CSAbstractDeclarativeData::receivers)(CSAbstractDeclarativeData *, const QObject *, int) = 0;



// **
const QMetaObject &CSGadget_Fake_Parent::staticMetaObject()
{
   const QMetaObject *retval = 0;
   return *retval;
}

QMap<std::type_index, QMetaObject *> &CSGadget_Fake_Parent::m_metaObjectsAll()
{
   static QMap<std::type_index, QMetaObject *> metaObjects;
   return metaObjects;
}


QT_END_NAMESPACE
