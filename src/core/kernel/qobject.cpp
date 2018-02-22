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

#include <qobject.h>
#include <csmeta_callevent.h>
#include <qorderedmutexlocker_p.h>
#include <qdebug.h>
#include <qthread_p.h>

#include <QAbstractEventDispatcher>
#include <QCoreApplication>
#include <QRegExp>

QT_BEGIN_NAMESPACE

QObject::QObject(QObject *t_parent)
{
   m_parent                   = 0;        // no parent yet, set by setParent()

   m_currentChildBeingDeleted = 0;
   m_declarativeData          = 0;
   m_postedEvents             = 0;
   m_sharedRefCount           = 0;

   m_pendTimer                = false;    // no timers yet
   m_wasDeleted               = false;    // double delete flag
   m_sentChildRemoved         = false;
   m_sendChildEvents          = true;     // should send ChildInsert and ChildRemove events to parent
   m_receiveChildEvents       = true;

   // atomics
   m_inThreadChangeEvent      = false;
   m_blockSig                 = false;    // allow signal to be emitted

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
         throw;
      }
   }
}

QObject::~QObject()
{
   this->m_blockSig = false;    // unblock signals so we always emit destroyed()

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

bool QObject::blockSignals(bool block)
{
   bool oldValue = m_blockSig.exchange(block);
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

// signal & slot are QMetaMethod
bool QObject::connect(const QObject *sender, const QMetaMethod &signalMetaMethod, const QObject *receiver,
                      const QMetaMethod &slotMetaMethod, Qt::ConnectionType type)
{
   if (sender == nullptr) {
      qWarning("QObject::connect() Can not connect, sender is null");
      return false;
   }

   if (receiver == nullptr) {
      qWarning("QObject::connect() Can not connect, receiver is null");
      return false;
   }

   // get signal name
   const char *senderClass   = sender->metaObject()->className();
   const char *receiverClass = receiver->metaObject()->className();

   QByteArray signalTemp     = signalMetaMethod.methodSignature();
   QByteArray slotTemp       = slotMetaMethod.methodSignature();

   const char *signalName    = signalTemp.constData();
   const char *slotName      = slotTemp.constData();

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
   const CSBentoAbstract *signalMethod_Bento = signalMetaMethod.getBentoBox();
   const CSBentoAbstract *slotMethod_Bento   = slotMetaMethod.getBentoBox();

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

   CsSignal::ConnectionKind kind;
   bool uniqueConnection = false;

   if (type & Qt::UniqueConnection) {
      uniqueConnection = true;
   }

   // untangle the type
   kind = static_cast<CsSignal::ConnectionKind>(type & ~Qt::UniqueConnection);

   std::unique_ptr<CsSignal::Internal::BentoAbstract> signalMethod_Bento2 = signalMethod_Bento->clone();
   std::unique_ptr<CsSignal::Internal::BentoAbstract> slotMethod_Bento2   = slotMethod_Bento->clone();

   CsSignal::connect(*sender, std::move(signalMethod_Bento2), *receiver, std::move(slotMethod_Bento2), kind, uniqueConnection);
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

void QObject::connectNotify(const QMetaMethod &signalMethod) const
{
   // no code is suppose to appear here, remove during rewrite
   connectNotify(signalMethod.methodSignature().constData());
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

// signal & slot are strings
bool QObject::disconnect(const QObject *sender,   const char *signalMethod,
                         const QObject *receiver, const char *slotMethod)
{
   if (sender == nullptr) {
      qWarning("QObject::disconnect() Can not disconnect as sender is null");
      return false;

   } else if (receiver == nullptr && slotMethod != nullptr) {
      qWarning("QObject::disconnect() Can not disconnect as receiver is null and slot was specified");
      return false;

   }

   // normalize the signal and slot method names or signatures

   QByteArray signal_name;

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

   if (slotMethod) {

      try {
         slot_name  = QMetaObject::normalizedSignature(slotMethod);
         slotMethod = slot_name.constData();

      } catch (const std::bad_alloc &) {
         // if the method is already normalized, we can continue
         if (receiver->metaObject()->indexOfMethod(slotMethod) == -1) {
            throw;
         }
      }
   }

   const QMetaObject *senderMetaObject   = sender->metaObject();
   const QMetaObject *receiverMetaObject = nullptr;

   if (receiver) {
      receiverMetaObject = receiver->metaObject();
   }

   // retrieve signal bento object from QMetaMethod
   int signal_index = -1;

   if (signalMethod) {
      signal_index = senderMetaObject->indexOfSignal(signalMethod);
   }

   const CSBentoAbstract *signalMethod_Bento = nullptr;
   QMetaMethod signalMetaMethod;

   if (signal_index != -1) {
      signalMetaMethod = senderMetaObject->method(signal_index);
      signalMethod_Bento = signalMetaMethod.getBentoBox();
   }

   // retrieve slot bento object from QMetaMethod
   int slot_index = -1;

   if (slotMethod) {
      slot_index = receiverMetaObject->indexOfSlot(slotMethod);
   }

   const CSBentoAbstract * slotMethod_Bento = nullptr;

   if (slot_index != -1) {
      slotMethod_Bento = receiverMetaObject->method(slot_index).getBentoBox();
   }

   bool retval = CsSignal::internal_disconnect(*sender, signalMethod_Bento, receiver, slotMethod_Bento);

   if (retval) {
      const QMetaObject *senderMetaObject = sender->metaObject();

      if (senderMetaObject) {
         const_cast<QObject *>(sender)->disconnectNotify(signalMetaMethod);
      }
   }

   return retval;
}

// signal & slot QMetaMethods
bool QObject::disconnect(const QObject *sender,   const QMetaMethod &signalMetaMethod,
                         const QObject *receiver, const QMetaMethod &slotMetaMethod)
{
   const QMetaObject *signalMetaObject = signalMetaMethod.getMetaObject();
   const QMetaObject *slotMetaObject   = slotMetaMethod.getMetaObject();

   if (sender == nullptr) {
      qWarning("QObject::disconnect() Can not disconnect as sender is null");
      return false;

   } else if (receiver == nullptr && slotMetaObject != nullptr) {
      qWarning("QObject::disconnect() Can not disconnect as receiver is null and slot was specified");
      return false;
   }

   if (signalMetaObject) {

      if (signalMetaMethod.methodType() != QMetaMethod::Signal) {
         qWarning("QObject::disconnect() Can not disconnect %s::%s, is not a signal",
                  sender->metaObject()->className(), signalMetaMethod.methodSignature().constData());
         return false;
      }
   }

   if (slotMetaObject) {

      if (slotMetaMethod.methodType() == QMetaMethod::Constructor) {
         qWarning("QObject::disconnect() Can not use constructor as an argument %s::%s",
                  receiver->metaObject()->className(), slotMetaMethod.methodSignature().constData());
         return false;
      }
   }

   int signal_index = sender->metaObject()->indexOfSignal(signalMetaMethod.methodSignature().constData());

   // if signalMethod is not empty and signal_index is -1, then signal is not a member of sender
   if (signalMetaObject != nullptr && signal_index == -1) {
      qWarning("QObject::disconnect() Signal %s was not found in class %s",
               signalMetaMethod.methodSignature().constData(), sender->metaObject()->className());
      return false;
   }

   // slot method is not a member of receiver
   if (receiver) {
      int slot_index = receiver->metaObject()->indexOfMethod(slotMetaMethod.methodSignature().constData());

      if (slotMetaObject && slot_index == -1) {
         qWarning("QObject::disconnect() Method %s was not found in class %s",
                  slotMetaMethod.methodSignature().constData(), receiver->metaObject()->className());
         return false;
      }
   }

   const CSBentoAbstract *signalMethod_Bento = signalMetaMethod.getBentoBox();
   const CSBentoAbstract *slotMethod_Bento   = slotMetaMethod.getBentoBox();

   bool retval = CsSignal::internal_disconnect(*sender, signalMethod_Bento, receiver, slotMethod_Bento);

   if (retval) {
      const QMetaObject *senderMetaObject = sender->metaObject();

      if (senderMetaObject) {
         const_cast<QObject *>(sender)->disconnectNotify(signalMetaMethod);
      }
   }

   return true;
}

bool QObject::disconnect(const QObject *sender,   const char *signalMethod, const char *,
                         const QObject *receiver, const char *slotMethod)
{
   return QObject::disconnect(sender, signalMethod, receiver, slotMethod);
}

bool QObject::disconnect(const char *signalMethod, const QObject *receiver, const char *slotMethod) const
{
   return QObject::disconnect(this, signalMethod, receiver, slotMethod);
}

bool QObject::disconnect(const char *signalMethod, const char *, const QObject *receiver,
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

void QObject::disconnectNotify(const QMetaMethod &signalMethod) const
{
   // no code is suppose to appear here, remove during rewrite
   disconnectNotify(signalMethod.methodSignature().constData());
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
         metaCallEvent->placeMetaCall(this);

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

bool QObject::eventFilter(QObject *, QEvent *)
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
   m_eventFilters.removeAll( QPointer<QObject>(nullptr) );

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
   bool retval = false;

   if (signalMethod) {
      QByteArray signal_name = QMetaObject::normalizedSignature(signalMethod);
      const QMetaObject *metaObj = this->metaObject();

      int index = metaObj->indexOfSignal(signal_name.constData());

      if (index == -1)  {
         return false;
      }

      QMetaMethod metaMethod = metaObj->method(index);
      const CSBentoAbstract *signalMethod_Bento = metaMethod.getBentoBox();

      int count = CsSignal::SignalBase::internal_cntConnections(receiver, *signalMethod_Bento);

      if (count > 0) {
         retval = true;
      }
   }

   return retval;
}

bool QObject::isSignalConnected(const QMetaMethod &signalMetaMethod) const
{
   bool retval = false;

   const CSBentoAbstract *signalMethod_Bento = signalMetaMethod.getBentoBox();

   int count = CsSignal::SignalBase::internal_cntConnections(nullptr, *signalMethod_Bento);

   if (count > 0) {
      retval = true;
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

bool QObject::compareThreads() const
{
   Qt::HANDLE currentThreadId = QThread::currentThreadId();

   return (currentThreadId == this->m_threadData.load()->threadId);
}

void QObject::queueSlot(CsSignal::PendingSlot data, CsSignal::ConnectionKind kind)
{
   std::unique_ptr<CsSignal::Internal::BentoAbstract> slot_Bento = data.internal_moveSlotBento();
   std::unique_ptr<CsSignal::Internal::TeaCupAbstract> teaCup    = data.internal_moveTeaCup();

   QObject *sender  = dynamic_cast<QObject *>(data.sender());
   int signal_index = senderSignalIndex();

   if (kind == CsSignal::ConnectionKind::QueuedConnection)  {
      CSMetaCallEvent *event = new CSMetaCallEvent(slot_Bento.release(), teaCup.release(),
                  sender, signal_index);

      QCoreApplication::postEvent(this, event);

   } else if (kind == CsSignal::ConnectionKind::BlockingQueuedConnection) {
      if (compareThreads()) {

         qWarning("QObject::activate() Dead lock detected while activating a BlockingQueuedConnection: "
                  "Sender is %s(%p), receiver is %s(%p)", sender->metaObject()->className(), sender,
                  this->metaObject()->className(), this);
      }

      QSemaphore semaphore;

      // store the signal data, false indicates the data will not be copied
      CSMetaCallEvent *event = new CSMetaCallEvent(slot_Bento.release(), teaCup.release(),
                  sender, signal_index, &semaphore);

      QCoreApplication::postEvent(this, event);

      semaphore.acquire();
   }
}

QDebug operator<<(QDebug debug, const QObject *object)
{
   QByteArray msg;

   if (object) {

      msg =  QByteArray(object->metaObject()->className()) + "(";
      msg += QByteArray::number(static_cast<quint64>(object - static_cast<const QObject *>(0)), 16);

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

QVariant QObject::property(const QString &name) const
{
   const QMetaObject *metaObj = metaObject();

   if (name.isEmpty() || ! metaObj) {
      return QVariant();
   }

   int index = metaObj->indexOfProperty(name.toUtf8().constData());

   if (index < 0) {
      const int k = m_extra_propertyNames.indexOf(name.toUtf8().constData());

      if (k == -1) {
         return QVariant();
      }

      return m_extra_propertyValues.value(k);
   }

   QMetaProperty p = metaObj->property(index);

   if (! p.isReadable()) {
      qWarning("%s::property() Property \"%s\" is invalid or does not exist", metaObj->className(), csPrintable(name));
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

      int signal_index = metaObj->indexOfSignal(signal_name.constData());

      if (signal_index == -1)  {
         return retval;
      }

      const QMetaMethod signalMetaMethod = metaObj->method(signal_index);
      const CSBentoAbstract *signalMethod_Bento = signalMetaMethod.getBentoBox();

      std::set<SlotBase *> tempSet = CsSignal::SignalBase::internal_receiverList(*signalMethod_Bento);

      for (auto item : tempSet)  {
         QObject *obj = dynamic_cast<QObject *>(item);

         if (obj) {
            retval.append(obj);
         }
      }
   }

   return retval;
}

int QObject::receivers(const char *signalMethod) const
{
   int receivers    = 0;
   int signal_index = -1;

   if (signalMethod) {
      QByteArray signal_name = QMetaObject::normalizedSignature(signalMethod);
      const QMetaObject *metaObj = this->metaObject();

      signal_index = metaObj->indexOfSignal(signal_name.constData());

      if (signal_index != -1)  {
         QMetaMethod signalMetaMethod = metaObj->method(signal_index);
         const CSBentoAbstract *signalMethod_Bento = signalMetaMethod.getBentoBox();

         receivers = CsSignal::SignalBase::internal_cntConnections(nullptr, *signalMethod_Bento);
      }
   }

   if (m_declarativeData && CSAbstractDeclarativeData::receivers) {
      receivers += CSAbstractDeclarativeData::receivers(m_declarativeData, this, signal_index);
   }

   return receivers;
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
   return dynamic_cast<QObject *>(SlotBase::sender());
}

// used by QAccessibleWidget
QList<QObject *> QObject::senderList() const
{
   QList<QObject *> retval;

   std::set<SignalBase *> tempSet = CsSignal::SlotBase::internal_senderList();

   for (auto item : tempSet)  {
      QObject *obj = dynamic_cast<QObject *>(item);

      if (obj) {
         retval.append(obj);
      }
   }

   return retval;
}

// only vaild when called from the slot in a direct connection
int QObject::senderSignalIndex() const
{
   int retval = -1;

   QObject *tempSender = sender();

   if (tempSender == nullptr) {
      return retval;
   }

   const CsSignal::Internal::BentoAbstract *signalBase = CsSignal::SignalBase::get_threadLocal_currentSignal();

   if (signalBase != nullptr) {
      retval = tempSender->metaObject()->indexOfMethod(*signalBase);
   }

   return retval;
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

bool QObject::setProperty(const QString &name, const QVariant &value)
{
   const QMetaObject *metaObj = metaObject();

   if (name.isEmpty() || ! metaObj) {
      return false;
   }

   int index = metaObj->indexOfProperty(name.toUtf8().constData());

   if (index < 0) {
      const int k = m_extra_propertyNames.indexOf(name.toUtf8().constData());

      if (value.isValid()) {
         // add or update dynamic property

         if (k == -1) {
            m_extra_propertyNames.append(name.toUtf8());
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

      QDynamicPropertyChangeEvent event(name.toUtf8());
      QCoreApplication::sendEvent(this, &event);

      return false;
   }

   QMetaProperty p = metaObj->property(index);

   if (! p.isWritable()) {
      qWarning("%s::setProperty() Property \"%s\" is invalid, read only, or does not exist", metaObj->className(),
                  csPrintable(name));
   }

   //
   bool retval = p.write(this, value);

   if (! retval) {
      qWarning("%s::setProperty() Set property \"%s\" failed. Passed value is of type %s, property is of type %s",
               metaObj->className(), csPrintable(name), value.typeName(), p.typeName() );
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

bool QObject::cs_InstanceOf(const QString &iid)
{
   // check if the iid is part of this class

   if (iid.isEmpty()) {
      return false;
   }

   const QMetaObject *metaObject = this->metaObject();

   while (metaObject != nullptr)  {

      // 1
      const char *className = metaObject->className();

      // test 1
      if (iid == className) {
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

bool QObject::cs_interface_query(const QString &) const
{
   return false;
}

QMap<std::type_index, QMetaObject *> &QObject::m_metaObjectsAll()
{
   static QMap<std::type_index, QMetaObject *> metaObjects;
   return metaObjects;
}

std::recursive_mutex &QObject::m_metaObjectMutex()
{
   static std::recursive_mutex metaObjectMutex;
   return metaObjectMutex;
}

// **
void (*CSAbstractDeclarativeData::destroyed)(CSAbstractDeclarativeData *, QObject *) = 0;
void (*CSAbstractDeclarativeData::parentChanged)(CSAbstractDeclarativeData *, QObject *, QObject *) = 0;
void (*CSAbstractDeclarativeData::signalEmitted)(CSAbstractDeclarativeData *, QObject *, int, void **) = 0;
int  (*CSAbstractDeclarativeData::receivers)(CSAbstractDeclarativeData *, const QObject *, int) = 0;

// **
const QMetaObject &CSGadget_Fake_Parent::staticMetaObject()
{
   const QMetaObject *retval = nullptr;
   return *retval;
}

QMap<std::type_index, QMetaObject *> &CSGadget_Fake_Parent::m_metaObjectsAll()
{
   static QMap<std::type_index, QMetaObject *> metaObjects;
   return metaObjects;
}

std::recursive_mutex &CSGadget_Fake_Parent::m_metaObjectMutex()
{
   static std::recursive_mutex metaObjectMutex;
   return metaObjectMutex;
}

QT_END_NAMESPACE
