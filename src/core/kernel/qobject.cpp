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

#include <qobject.h>

#include <csmeta_callevent.h>
#include <qabstracteventdispatcher.h>
#include <qcoreapplication.h>
#include <qdebug.h>
#include <qregularexpression.h>

#include <qorderedmutexlocker_p.h>
#include <qthread_p.h>

QObject::QObject(QObject *t_parent)
{

   m_parent                   = nullptr;     // no parent yet, set by setParent()
   m_currentChildBeingDeleted = nullptr;
   m_declarativeData          = nullptr;
   m_postedEvents             = 0;
   m_sharedRefCount           = nullptr;

   m_pendTimer                = false;       // no timers yet
   m_wasDeleted               = false;       // double delete flag
   m_sentChildRemoved         = false;
   m_sendChildEvents          = true;        // should send ChildInsert and ChildRemove events to parent
   m_receiveChildEvents       = true;

   // atomics
   m_inThreadChangeEvent      = false;
   m_blockSig                 = false;       // allow signal to be emitted

   //
   if (t_parent && ! t_parent->thread()) {
      m_threadData.store(t_parent->m_threadData.load());
   } else {
      m_threadData.store(QThreadData::current());
   }

   m_threadData.load()->ref();

   if (t_parent) {
      try {

         if (! this->check_parent_thread(t_parent, t_parent->m_threadData, m_threadData)) {
            t_parent = nullptr;
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
      // program will crash soon

      qWarning("QObject:~QObject() Detected an unexpected exception while emitting destroyed()");
   }

   // this line needs to be located after the emit destroyed
   this->m_wasDeleted = true;

   QtSharedPointer::ExternalRefCountData *sharedRefCount = m_sharedRefCount.exchange(nullptr);

   if (sharedRefCount) {

      if (sharedRefCount->strongref.load() > 0) {
         // continue deleting, unclear what else to do
         qWarning("QObject:~QObject() Shared QObject was deleted directly, application may crash.");
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
      this->setParent(nullptr);
   }

   QThreadData *threadData = m_threadData.load();

   if (m_pendTimer) {
      if (threadData->thread == QThread::currentThread()) {

         // unregister pending timers
         auto tmp = threadData->eventDispatcher.load();

         if (tmp) {
            tmp->unregisterTimers(this);
         }
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

      qWarning("QObject:check_parent_thread() Unable to create children for a parent in a different thread.\n"
            "(Parent is %s(%p), parent's thread is %s(%p), current thread is %s(%p)",
            csPrintable(parent->metaObject()->className()), static_cast<void *>(parent),
            parentThread ? csPrintable(parentThread->metaObject()->className()) : "QThread",
            static_cast<void *>(parentThread),
            currentThread ? csPrintable(currentThread->metaObject()->className()) : "QThread",
            static_cast<void *>(currentThread));

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
   // no code should appear here
}

bool QObject::connect(const QObject *sender, const QString8 &signalMethod, const QString8 &location,
      const QString8 &slotMethod, Qt::ConnectionType type)
{
   // default third param (receiver)
   return connect(sender, signalMethod, this, slotMethod, type, location);
}

bool QObject::connect(const QObject *sender, const QString8 &signalMethod, const QString8 &location,
      const QObject *receiver, const QString8 &slotMethod, Qt::ConnectionType type)
{
   // location is generated by the SIGNAL macro, indicates where connect() was called
   return connect(sender, signalMethod, receiver, slotMethod, type, location);
}

bool QObject::connect(const QObject *sender, const QString8 &signalMethod, const QObject *receiver,
      const QString8 &slotMethod, Qt::ConnectionType type, const QString8 &location)
{
   const QMetaObject *senderMetaObject = sender->metaObject();
   int sIndex = senderMetaObject->indexOfSignal(signalMethod);

   const QMetaObject *receiverMetaObject = receiver->metaObject();
   int rIndex = receiverMetaObject->indexOfMethod(slotMethod);

   if (sIndex == -1 || rIndex == -1)  {
      const QString8 &senderClass   = senderMetaObject->className();
      const QString8 &receiverClass = receiverMetaObject->className();

      if (location.isEmpty()) {
         qWarning("%s%s%s%s%s%s%s%s%s %s%d%s%d", "QObject::connect() ", csPrintable(senderClass), "::", csPrintable(signalMethod),
               " Unable to connect to receiver in ", csPrintable(receiverClass), " (", csPrintable(location), ")",
               " Signal Index: ", sIndex, " Slot Index: ", rIndex);

      } else {
         qWarning("%s%s%s%s%s%s %s%d%s%d", "QObject::connect() ", csPrintable(senderClass), "::", csPrintable(signalMethod),
               " Unable to connect to receiver in ", csPrintable(receiverClass),
               " Signal Index: ", sIndex, " Slot Index: ", rIndex);
      }

#if defined(CS_INTERNAL_DEBUG)
      qDebug("");

      for (int k = 0; k < senderMetaObject->methodCount(); ++k) {
         qDebug("QObject::connect()  Class %s has method %s", csPrintable(senderMetaObject->className()),
               csPrintable(senderMetaObject->method(k).methodSignature()) );
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
   const QString8 &senderClass   = sender->metaObject()->className();
   const QString8 &receiverClass = receiver->metaObject()->className();

   const QString8 &signalName    = signalMetaMethod.methodSignature();
   const QString8 &slotName      = slotMetaMethod.methodSignature();

   if (signalName.isEmpty())  {
      qWarning("%s%s%s%s%s", "QObject::connect() ", csPrintable(senderClass), "::<Invalid Signal> ",
            " Unable to connect to receiver in ", csPrintable(receiverClass));

      return false;
   }

   if (slotName.isEmpty())  {
      qWarning("%s%s%s%s%s%s%s", "QObject::connect() ", csPrintable(senderClass), "::", csPrintable(signalName),
            " Unable to connect to receiver in ", csPrintable(receiverClass), "::<Invalid Slot>");
      return false;
   }

   // is signalMethod a signal
   if (signalMetaMethod.methodType() != QMetaMethod::Signal) {
      qWarning("%s%s%s%s%s", "QObject::connect() ", csPrintable(senderClass), "::", csPrintable(signalName),
            ": Is not a valid signal");
      return false;
   }

   // is slotMethod a clone, then there is a defualt parameter
   if (slotMetaMethod.attributes() & QMetaMethod::Cloned) {
      qWarning("%s%s%s%s%s", "QObject::connect() ", csPrintable(receiverClass), "::", csPrintable(slotName),
            ": Unable to connect to a slot with a default parameter");
      return false;
   }

   // test arguments
   bool err = false;
   QList<QString8> typesSignal = signalMetaMethod.parameterTypes();
   QList<QString8> typesSlot   = slotMetaMethod.parameterTypes();

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
      qWarning("%s%s%s%s%s%s%s%s", "QObject::connect() ", csPrintable(senderClass), "::", csPrintable(signalName),
            ": Incompatible signal/slot arguments ", csPrintable(receiverClass), "::", csPrintable(slotName));
      return false;
   }

   //
   const CSBentoAbstract *signalMethod_Bento = signalMetaMethod.getBentoBox();
   const CSBentoAbstract *slotMethod_Bento   = slotMetaMethod.getBentoBox();

   if (! signalMethod_Bento) {
      qWarning("%s%s%s%s%s", "QObject::connect() ", csPrintable(senderClass), "::", csPrintable(signalName),
            " Unable to locate the requested signal, verify connect arguments and signal declaration");
      return false;
   }

   if (! slotMethod_Bento) {
      qWarning("%s%s%s%s%s", "QObject::connect() ", csPrintable(receiverClass), "::", csPrintable(slotName),
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

bool QObject::connect(const QObject *sender, const QString8 &signalMethod, const QString8 &slotMethod, Qt::ConnectionType type)
{
   return QObject::connect(sender, signalMethod, this, slotMethod, type);
}

void QObject::connectNotify(const QMetaMethod &) const
{
   // no code should appear here
}

void QObject::customEvent(QEvent *)
{
   // no code should appear here
}

void QObject::deleteChildren()
{
   const bool reallyWasDeleted = m_wasDeleted;
   m_wasDeleted = true;

   // do not use qDeleteAll as the destructor of the child might delete siblings

   for (int i = 0; i < m_children.count(); ++i) {
      m_currentChildBeingDeleted = m_children.at(i);
      m_children[i] = nullptr;
      delete m_currentChildBeingDeleted;
   }

   m_children.clear();
   m_currentChildBeingDeleted = nullptr;

   m_wasDeleted = reallyWasDeleted;
}

void QObject::deleteLater()
{
   QCoreApplication::postEvent(this, new QDeferredDeleteEvent());
}

// signal & slot are strings
bool QObject::disconnect(const QObject *sender,   const QString8 &signalMethod,
      const QObject *receiver, const QString8 &slotMethod)
{
   if (sender == nullptr) {
      qWarning("QObject::disconnect() Can not disconnect as sender is null");
      return false;

   } else if (receiver == nullptr && ! slotMethod.isEmpty()) {
      qWarning("QObject::disconnect() Can not disconnect as the receiver is null and a slot method was specified");
      return false;
   }

   // normalize the signal and slot method names or signatures
   QString8 signalNormalized;
   QString8 slotNormalized;

   if (! signalMethod.isEmpty()) {

      try {
         signalNormalized = QMetaObject::normalizedSignature(signalMethod);

      } catch (const std::bad_alloc &) {
         // if the signal is already normalized, we can continue
         if (sender->metaObject()->indexOfSignal(signalMethod) == -1) {
            throw;
         }

         signalNormalized = signalMethod;
      }
   }

   if (! slotMethod.isEmpty()) {

      try {
         slotNormalized = QMetaObject::normalizedSignature(slotMethod);

      } catch (const std::bad_alloc &) {
         // if the method is already normalized, we can continue
         if (receiver->metaObject()->indexOfMethod(slotMethod) == -1) {
            throw;
         }

         slotNormalized = slotMethod;
      }
   }

   const QMetaObject *senderMetaObject   = sender->metaObject();
   const QMetaObject *receiverMetaObject = nullptr;

   if (receiver) {
      receiverMetaObject = receiver->metaObject();
   }

   // retrieve signal bento object from QMetaMethod
   int signal_index = -1;

   if (! signalNormalized.isEmpty()) {
      signal_index = senderMetaObject->indexOfSignal(signalNormalized);
   }

   const CSBentoAbstract *signalMethod_Bento = nullptr;
   QMetaMethod signalMetaMethod;

   if (signal_index != -1) {
      signalMetaMethod   = senderMetaObject->method(signal_index);
      signalMethod_Bento = signalMetaMethod.getBentoBox();
   }

   // retrieve slot bento object from QMetaMethod
   int slot_index = -1;

   if (! slotNormalized.isEmpty()) {
      slot_index = receiverMetaObject->indexOfSlot(slotNormalized);
   }

   const CSBentoAbstract *slotMethod_Bento = nullptr;

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

// signal & slot are both a nullptr
bool QObject::disconnect(const QObject *sender, std::nullptr_t, const QObject *receiver, std::nullptr_t)
{
   if (sender == nullptr) {
      qWarning("QObject::disconnect() Can not disconnect as sender is null");
      return false;
   }

   const CSBentoAbstract *signalMethod_Bento = nullptr;
   const CSBentoAbstract *slotMethod_Bento   = nullptr;

   bool retval = CsSignal::internal_disconnect(*sender, signalMethod_Bento, receiver, slotMethod_Bento);

   if (retval) {
      QMetaMethod signalMetaMethod;

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
      qWarning("QObject::disconnect() Unable to disconnect as sender is null");
      return false;

   } else if (receiver == nullptr && slotMetaObject != nullptr) {
      qWarning("QObject::disconnect() Unable to disconnect as receiver is null and slot was specified");
      return false;
   }

   if (signalMetaObject) {

      if (signalMetaMethod.methodType() != QMetaMethod::Signal) {
         qWarning("QObject::disconnect() Unable to disconnect %s::%s, is not a signal",
               csPrintable(sender->metaObject()->className()), csPrintable(signalMetaMethod.methodSignature()));
         return false;
      }
   }

   if (slotMetaObject) {

      if (slotMetaMethod.methodType() == QMetaMethod::Constructor) {
         qWarning("QObject::disconnect() Unable to use constructor as an argument %s::%s",
               csPrintable(receiver->metaObject()->className()), csPrintable(slotMetaMethod.methodSignature()));
         return false;
      }
   }

   int signal_index = sender->metaObject()->indexOfSignal(signalMetaMethod.methodSignature());

   // if signalMethod is not empty and signal_index is -1, then signal is not a member of sender
   if (signalMetaObject != nullptr && signal_index == -1) {
      qWarning("QObject::disconnect() Signal %s was not found in class %s",
            csPrintable(signalMetaMethod.methodSignature()), csPrintable(sender->metaObject()->className()));
      return false;
   }

   // slot method is not a member of receiver
   if (receiver) {
      int slot_index = receiver->metaObject()->indexOfMethod(slotMetaMethod.methodSignature());

      if (slotMetaObject && slot_index == -1) {
         qWarning("QObject::disconnect() Method %s was not found in class %s",
               csPrintable(slotMetaMethod.methodSignature()), csPrintable(receiver->metaObject()->className()));
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

bool QObject::disconnect(const QObject *sender,   const QString8 &signalMethod, const QString8 &,
      const QObject *receiver, const QString8 &slotMethod)
{
   return QObject::disconnect(sender, signalMethod, receiver, slotMethod);
}

bool QObject::disconnect(const QString8 &signalMethod, const QObject *receiver, const QString8 &slotMethod) const
{
   return QObject::disconnect(this, signalMethod, receiver, slotMethod);
}

bool QObject::disconnect(const QString8 &signalMethod, const QString8 &, const QObject *receiver,
      const QString8 &slotMethod) const
{
   return QObject::disconnect(this, signalMethod, receiver, slotMethod);
}

bool QObject::disconnect(const QObject *receiver, const QString8 &slotMethod) const
{
   return QObject::disconnect(this, QString8(), receiver, slotMethod);
}

void QObject::disconnectNotify(const QMetaMethod &) const
{
   // no code should appear here
}

QList<QString8> QObject::dynamicPropertyNames() const
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
         QAbstractEventDispatcher *eventDispatcher = threadData->eventDispatcher.load();

         if (eventDispatcher) {
            QList<QTimerInfo> timers = eventDispatcher->registeredTimers(this);

            if (! timers.isEmpty()) {
               // set m_inThreadChangeEvent to true tells the dispatcher not to release timer
               // ids back to the pool (since the timer ids are moving to a new thread)

               m_inThreadChangeEvent = true;
               eventDispatcher->unregisterTimers(this);
               m_inThreadChangeEvent = false;

               // using csArgument directly since the datatype contains a comma
               QMetaObject::invokeMethod(this, "internal_reregisterTimers", Qt::QueuedConnection,
                     CSArgument<QList<QTimerInfo>> (timers) );
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
   // no code should appear here
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

bool QObject::inherits(const QString8 &classname) const
{
   bool retval = false;
   const QMetaObject *metaObj = this->metaObject();

   while (metaObj)  {

      if (metaObj->className() == classname)  {
         retval = true;
         break;
      }

      // get parent
      metaObj = metaObj->superClass();
   }

   return retval;
}

// used by QAccessibleWidget
bool QObject::isSender(const QObject *receiver, const QString8 &signalMethod) const
{
   bool retval = false;

   if (! signalMethod.isEmpty()) {
      QString8 signal_name = QMetaObject::normalizedSignature(signalMethod);
      const QMetaObject *metaObj = this->metaObject();

      int index = metaObj->indexOfSignal(signal_name);

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

bool QObject::isWindowType() const
{
   return cs_isWindowType();
}

bool QObject::cs_isWindowType() const
{
   return false;
}

void QObject::killTimer(int id)
{
   QThreadData *threadData = m_threadData.load();
   auto tmp = threadData->eventDispatcher.load();

   if (tmp) {
      tmp->unregisterTimer(id);
   }
}

QString8 QObject::objectName() const
{
   return m_objectName;
}

void QObject::moveToThread(QThread *targetThread)
{
   if (m_threadData.load()->thread == targetThread) {
      // object is already in this thread
      return;
   }

   if (m_parent != nullptr) {
      qWarning("QObject::moveToThread() Unble to not move an object with a parent");
      return;
   }

   if (isWidgetType()) {
      qWarning("QObject::moveToThread() Widgets can not be moved to a new thread");
      return;
   }

   QThreadData *currentData = QThreadData::current();
   QThreadData *targetData = targetThread ? QThreadData::get2(targetThread) : new QThreadData(0);

   QThreadData *threadData = m_threadData.load();

   if (threadData->thread == nullptr && currentData == targetData) {
      // exception: allow moving objects with no thread affinity to the current thread
      currentData = threadData;

   } else if (threadData != currentData) {

      qWarning("QObject::moveToThread() Current thread (%p) does not match thread for the current object (%p),\n"
            "unable to move to target thread (%p)\n",
            static_cast<void *>(currentData->thread.load()), static_cast<void *>(threadData->thread.load()),
            targetData ? static_cast<void *>(targetData->thread.load()) : nullptr);

#ifdef Q_OS_DARWIN

      qWarning("QObject::moveToThread() Multiple libraries might be loaded in the same process. Verify all plugins "
            "are linked with the correct binaries. Set DYLD_PRINT_LIBRARIES=1 and verify only one set of "
            "binaries are loaded.");
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
               "Sender is %s(%p), receiver is %s(%p)", csPrintable(sender->metaObject()->className()),
               static_cast<void *>(sender),
               csPrintable(this->metaObject()->className()), static_cast<void *>(this) );
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
   QString8 msg;

   if (object == nullptr) {
      msg = "QObject(nullptr) ";

   } else {
      msg =  object->metaObject()->className() + "(";
      msg += QString8::number(bit_cast<quintptr>(object), 16);

      if (! object->objectName().isEmpty())  {
         msg += ", name = " + object->objectName();
      }

      msg += ")";
   }

   return debug << msg;
}

QObject *QObject::parent() const
{
   return m_parent;
}

// used by QAccessibleWidget
QList<QObject *> QObject::receiverList(const QMetaMethod &signalMetaMethod) const
{
   QList<QObject *> retval;

   if (signalMetaMethod.isValid()) {
      const CSBentoAbstract *signalMethod_Bento = signalMetaMethod.getBentoBox();
      std::set<SlotBase *> tmpSet = CsSignal::SignalBase::internal_receiverList(*signalMethod_Bento);

      for (auto item : tmpSet)  {
         QObject *obj = dynamic_cast<QObject *>(item);

         if (obj != nullptr) {
            retval.append(obj);
         }
      }
   }

   return retval;
}

int QObject::receivers(const QString8 &signalMethod) const
{
   int receivers    = 0;
   int signal_index = -1;

   if (! signalMethod.isEmpty()) {
      QString8 signal_name = QMetaObject::normalizedSignature(signalMethod);
      const QMetaObject *metaObj = this->metaObject();

      signal_index = metaObj->indexOfSignal(signal_name);

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
         m_eventFilters[i] = nullptr;
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

void QObject::setObjectName(const QString8 &name)
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
            m_parent->m_children[index] = nullptr;

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
         qWarning("QObject::setParent() Unable to set parent, new parent is in a different thread");
         m_parent = nullptr;
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

bool QObject::setProperty(const QString8 &name, const QVariant &value)
{
   const QMetaObject *metaObj = metaObject();

   if (name.isEmpty() || ! metaObj) {
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

      QDynamicPropertyChangeEvent event(name.toUtf8());
      QCoreApplication::sendEvent(this, &event);

      return false;
   }

   QMetaProperty p = metaObj->property(index);

   if (! p.isWritable()) {
      qWarning("%s::setProperty() Property \"%s\" is invalid, read only, or does not exist",
            csPrintable(metaObj->className()), csPrintable(name));
   }

   bool retval = p.write(this, value);

   if (! retval) {
      qWarning("%s::setProperty() Set property \"%s\" failed. Passed value is of type %s, property is of type %s",
            csPrintable(metaObj->className()), csPrintable(name), csPrintable(value.typeName()),
            csPrintable(p.typeName()));
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
         const_cast<QPostEvent &>(postedEvent).event = nullptr;
         ++eventsMoved;
      }
   }

   if (eventsMoved > 0 && targetData->eventDispatcher) {
      targetData->canWait = false;
      targetData->eventDispatcher.load()->wakeUp();
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

int QObject::startTimer(int interval, Qt::TimerType timerType)
{
   if (interval < 0) {
      qWarning("QObject::startTimer() QTimer can not have a negative interval");
      return 0;
   }

   // set timer flag
   m_pendTimer = true;

   QThreadData *threadData = m_threadData.load();
   auto tmp = threadData->eventDispatcher.load();

   if (! tmp ) {
      qWarning("QObject::startTimer() QTimer can only be used with threads started with QThread");
      return 0;
   }

   if (thread() != QThread::currentThread()) {
      qWarning("QObject::startTimer() Timers can not be started from another thread");
      return 0;
   }

   return tmp->registerTimer(interval, timerType, this);
}

QThread *QObject::thread() const
{
   return m_threadData.load()->thread;
}

void QObject::timerEvent(QTimerEvent *)
{
   // no code should appear here
}

bool QObject::cs_InstanceOf(const QString8 &iid)
{
   // check if the iid is part of this class

   if (iid.isEmpty()) {
      return false;
   }

   const QMetaObject *metaObject = this->metaObject();

   while (metaObject != nullptr)  {
      // test 1
      if (iid == metaObject->className()) {
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

bool QObject::cs_interface_query(const QString8 &) const
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
void (*CSAbstractDeclarativeData::destroyed)(CSAbstractDeclarativeData *, QObject *) = nullptr;
void (*CSAbstractDeclarativeData::parentChanged)(CSAbstractDeclarativeData *, QObject *, QObject *) = nullptr;
void (*CSAbstractDeclarativeData::signalEmitted)(CSAbstractDeclarativeData *, QObject *, int, void **) = nullptr;
int  (*CSAbstractDeclarativeData::receivers)(CSAbstractDeclarativeData *, const QObject *, int) = nullptr;

// **
const QMetaObject &CSGadget_Fake_Parent::staticMetaObject()
{
   const QMetaObject *retval = nullptr;
   return *retval;
}
