/***********************************************************************
*
* Copyright (c) 2015-2016 Barbara Geller
* Copyright (c) 2015-2016 Ansel Sermersheim
* All rights reserved.
*
* This file is part of libCsSignal
*
* libCsSignal is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
***********************************************************************/

#ifndef LIB_CS_SIGNAL_H
#define LIB_CS_SIGNAL_H

#include <algorithm>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "cs_internal.h"
#include "cs_macro.h"
#include "cs_slot.h"

namespace CsSignal {

enum class ConnectionKind {
   AutoConnection,
   DirectConnection,
   QueuedConnection,
   BlockingQueuedConnection,

   // internal, do not use as this can change
   InternalDisconnected
};

enum class DisconnectKind {
   DisconnectAll,
   DisconnectOne
};

template<class Sender, class SignalClass, class ...SignalArgs, class Receiver,
                  class SlotClass, class ...SlotArgs, class SlotReturn>
bool connect(const Sender &sender, void (SignalClass::*signalMethod)(SignalArgs...),
                  const Receiver &receiver, SlotReturn (SlotClass::*slotMethod)(SlotArgs...),
                  ConnectionKind type = ConnectionKind::AutoConnection, bool uniqueConnection = false);

template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class T>
bool connect(const Sender &sender, void (SignalClass::*signalMethod)(SignalArgs...),
                  const Receiver &receiver, T slotLambda,
                  ConnectionKind type = ConnectionKind::AutoConnection, bool uniqueConnection = false);

template<class Sender, class Receiver>
bool connect(const Sender &sender, std::unique_ptr<Internal::BentoAbstract> signalMethod_Bento,
                  const Receiver &receiver, std::unique_ptr<Internal::BentoAbstract> slotMethod_Bento,
                  ConnectionKind type = ConnectionKind::AutoConnection, bool uniqueConnection = false);

// base class
class LIB_SIG_EXPORT SignalBase
{
   public:
      virtual ~SignalBase();

   protected:
      static Internal::BentoAbstract *&get_threadLocal_currentSignal();

      int internal_cntConnections(const SlotBase *receiver,
                  const Internal::BentoAbstract &signalMethod_Bento) const;

      std::set<SlotBase *> internal_receiverList(
                  const Internal::BentoAbstract &signalMethod_Bento) const;

   private:
      mutable std::mutex m_mutex_connectList;

      // part of destructor
      static std::mutex &get_mutex_beingDestroyed();
      static std::unordered_set<const SignalBase *> &get_beingDestroyed();

      // part of disconnect
      mutable int m_activateBusy = 0;
      mutable int m_raceCount    = 0;

      struct ConnectStruct {
         std::unique_ptr<const Internal::BentoAbstract> signalMethod;
         const SlotBase *receiver;
         std::unique_ptr<const Internal::BentoAbstract> slotMethod;
         ConnectionKind type;
      };

      // list of connections from my Signal to some Receiver
      mutable std::vector<ConnectStruct> m_connectList;

      void addConnection(std::unique_ptr<const Internal::BentoAbstract> signalMethod, const SlotBase *,
                  std::unique_ptr<const Internal::BentoAbstract> slotMethod, ConnectionKind type) const;

      virtual void handleException(std::exception_ptr data);

      template<class Sender, class SignalClass, class ...SignalArgTypes, class ...Ts>
      friend void activate(Sender &sender, void (SignalClass::*signal)(SignalArgTypes...), Ts &&... Vs);

      template<class Sender, class SignalClass, class ...SignalArgs, class Receiver,
                  class SlotClass, class ...SlotArgs, class SlotReturn>
      friend bool connect(const Sender &sender, void (SignalClass::*signalMethod)(SignalArgs...),
                  const Receiver &receiver, SlotReturn (SlotClass::*slotMethod)(SlotArgs...),
                  ConnectionKind type, bool uniqueConnection);

      template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class T>
      friend bool connect(const Sender &sender, void (SignalClass::*signalMethod)(SignalArgs...),
                  const Receiver &receiver, T slotLambda,
                  ConnectionKind type, bool uniqueConnection);

      template<class Sender, class Receiver>
      friend bool connect(const Sender &sender, std::unique_ptr<Internal::BentoAbstract> signalMethod_Bento,
                  const Receiver &receiver, std::unique_ptr<Internal::BentoAbstract> slotMethod_Bento,
                  ConnectionKind type, bool uniqueConnection);

      template<class Sender, class Receiver>
      friend bool internal_disconnect(const Sender &sender, const Internal::BentoAbstract *signalBento,
                  const Receiver *receiver, const Internal::BentoAbstract *slotBento);

      friend class SlotBase;
};

template<class Sender, class SignalClass, class ...SignalArgTypes, class ...Ts>
void activate(Sender &sender, void (SignalClass::*signal)(SignalArgTypes...), Ts &&... Vs)
{
   // ensure signal args are passed
   static_assert( std::is_convertible<std::tuple<Ts...>, std::tuple<SignalArgTypes...>>::value,
                  "activate():  Signal parameter mismatch.");

   Internal::Bento<void (SignalClass::*)(SignalArgTypes...)> signal_Bento(signal);

   // save the addresss of sender
   const SignalBase *senderPtr = &sender;

   // threading and queuedConnections
   std::unique_lock<std::mutex> senderLock {sender.m_mutex_connectList};

   // store the signal data, false indicates the data will not be copied
   CsSignal::Internal::TeaCup_Data<SignalArgTypes...> dataPack(false, std::forward<Ts>(Vs)...);

   bool raceHappened = false;
   int maxCount = sender.m_connectList.size();

   SignalBase *priorSender = SlotBase::get_threadLocal_currentSender();
   SlotBase::get_threadLocal_currentSender() = &sender;

   Internal::BentoAbstract *priorSignal = SignalBase::get_threadLocal_currentSignal();
   SignalBase::get_threadLocal_currentSignal() = &signal_Bento;

   for (int k = 0; k < maxCount; ++k) {
      const SignalBase::ConnectStruct &connection = sender.m_connectList[k];

      if (*(connection.signalMethod) != signal_Bento)  {
         // no match in connectionList for this signal
         continue;
      }

      if (connection.type == ConnectionKind::InternalDisconnected) {
         // connection is marked for deletion
         continue;
      }

      SlotBase *receiver = const_cast<SlotBase *>(connection.receiver);

      // const reference to a unique ptr
      const std::unique_ptr<const CsSignal::Internal::BentoAbstract> &slot_Bento = connection.slotMethod;

      bool receiverInSameThread = receiver->compareThreads();

      int old_activateBusy = sender.m_activateBusy;
      int old_raceCount    = sender.m_raceCount;

      sender.m_activateBusy++;
      senderLock.unlock();

      try {

         if ( (connection.type == ConnectionKind::AutoConnection && ! receiverInSameThread) ||
              (connection.type == ConnectionKind::QueuedConnection)) {

            // passing true indicates the data will be copied (stored on the heap)
            PendingSlot tempObj(&sender, signal_Bento.clone(), receiver, slot_Bento->clone(),
                  Internal::make_unique<CsSignal::Internal::TeaCup_Data<SignalArgTypes...>>(true, std::forward<Ts>(Vs)... ));

            receiver->queueSlot(std::move(tempObj), ConnectionKind::QueuedConnection);

         } else if (connection.type == ConnectionKind::BlockingQueuedConnection) {

            // passing false indicates the data will not be copied
            PendingSlot tempObj(&sender, signal_Bento.clone(), receiver, slot_Bento->clone(),
                     Internal::make_unique<CsSignal::Internal::TeaCup_Data<SignalArgTypes...>>(false, std::forward<Ts>(Vs)... ));

            receiver->queueSlot(std::move(tempObj), ConnectionKind::BlockingQueuedConnection);

         } else if (connection.type == ConnectionKind::DirectConnection || connection.type == ConnectionKind::AutoConnection) {
            // direct connection

            // invoke calls the actual method
            slot_Bento->invoke(receiver, &dataPack);
         }

         std::lock_guard<std::mutex> lock(SignalBase::get_mutex_beingDestroyed());   // should be a read lock

         if (SignalBase::get_beingDestroyed().count(senderPtr)) {
            // sender has been destroyed
            SlotBase::get_threadLocal_currentSender()   = priorSender;
            SignalBase::get_threadLocal_currentSignal() = priorSignal;

            if (old_activateBusy == 0)  {
               SignalBase::get_beingDestroyed().erase(senderPtr);
            }

            return;
         }

      } catch (...) {
         SlotBase::get_threadLocal_currentSender()   = priorSender;
         SignalBase::get_threadLocal_currentSignal() = priorSignal;

         std::lock_guard<std::mutex> lock(SignalBase::get_mutex_beingDestroyed());   // should be a read lock

         if (SignalBase::get_beingDestroyed().count(senderPtr)) {
            // sender has been destroyed, all done

            if (old_activateBusy == 0)  {
               SignalBase::get_beingDestroyed().erase(senderPtr);
            }

            return;

         } else {
            sender.handleException(std::current_exception());
            SlotBase::get_threadLocal_currentSender() = &sender;

         }
      }

      try {
         senderLock.lock();
         sender.m_activateBusy--;

      } catch (std::exception &) {
         SlotBase::get_threadLocal_currentSender()   = priorSender;
         SignalBase::get_threadLocal_currentSignal() = priorSignal;

         std::throw_with_nested(std::invalid_argument("activate(): Failed to obtain sender lock"));
      }

      if (old_raceCount != sender.m_raceCount) {
         // connectionList modified
         raceHappened = true;

         maxCount = sender.m_connectList.size();

         // connect() can add an entry to the end of the list
         // disconnect() can mark a connection as pending deletion
      }
   }

   SlotBase::get_threadLocal_currentSender()   = priorSender;
   SignalBase::get_threadLocal_currentSignal() = priorSignal;

   if (raceHappened && sender.m_activateBusy == 0)  {
      // finish clean up for disconnect
      sender.m_connectList.erase(std::remove_if(sender.m_connectList.begin(), sender.m_connectList.end(),
                  [](const SignalBase::ConnectStruct & tmp){ return tmp.type == ConnectionKind::InternalDisconnected; } ),
                  sender.m_connectList.end() );
   }
}

// signal & slot method ptr
template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class SlotClass,
                  class ...SlotArgs, class SlotReturn>
bool connect(const Sender &sender, void (SignalClass::*signalMethod)(SignalArgs...),
                  const Receiver &receiver, SlotReturn (SlotClass::*slotMethod)(SlotArgs...),
                  ConnectionKind type, bool uniqueConnection)
{

/*
   // is the sender an rvalue reference
   static_assert(! std::is_rvalue_reference<Sender &&>::value,
                  "connect():  Sender can not be an rvalue");

   // is the receiver an rvalue reference
   static_assert(! std::is_rvalue_reference<Receiver &&>::value,
                  "connect():  Receiver can not be an rvalue");
*/

   // (1) Sender must be the same class as SignalClass OR (2) Sender is a child of SignalClass
   static_assert( std::is_base_of<SignalClass, Sender>::value,
                  "connect():  Signal was not a child class of Sender");

   // (1) Receiver must be the same class as SlotClass OR (2) Receiver is a child of SlotClass
   static_assert( std::is_base_of<SlotClass, Receiver>::value,
                  "connect():  Slot was not a child class of Receiver");

   // compare signal and slot paramerter list
   static_assert( Internal::cs_check_connect_args<void (*)(SignalArgs...), void (*)(SlotArgs...) >::value,
                  "connect():  Incompatible signal/slot arguments");

   if (signalMethod == nullptr) {
      throw std::invalid_argument("connect() Can not connect, signal is null");
   }

   if (slotMethod == nullptr) {
      throw std::invalid_argument("connect(): Can not connect, slot is null");
   }

   std::unique_ptr<Internal::Bento<void (SignalClass::*)(SignalArgs...)>>
                  signalMethod_Bento(new Internal::Bento<void (SignalClass::*)(SignalArgs...)>(signalMethod));

   std::unique_ptr<Internal::Bento<void (SlotClass::*)(SlotArgs...)>>
                  slotMethod_Bento(new Internal::Bento<void (SlotClass::*)(SlotArgs...)>(slotMethod));

   std::unique_lock<std::mutex> senderLock {sender.m_mutex_connectList};

   if (uniqueConnection) {
      // ensure the connection is not added twice

      for (auto &item : sender.m_connectList) {

         if (item.receiver != &receiver) {
            continue;
         }

         if (*(item.signalMethod) != *(signalMethod_Bento))  {
            continue;
         }

         if (*(item.slotMethod) != *(slotMethod_Bento))  {
            continue;
         }

         // connection already exists
         return false;
      }
   }

   sender.addConnection(std::move(signalMethod_Bento), &receiver, std::move(slotMethod_Bento), type);

   return true;
}

// signal method ptr, slot lambda
template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class T>
bool connect(const Sender &sender, void (SignalClass::*signalMethod)(SignalArgs...), const Receiver &receiver,
                  T slotLambda, ConnectionKind type, bool uniqueConnection)
{
   // Sender must be the same class as SignalClass and Sender is a child of SignalClass
   Internal::cs_testConnect_SenderSignal<Sender, SignalClass>();

   // compare signal and slot paramerter list
   Internal::cs_testConnect_SignalSlotArgs_1<T, SignalArgs...>();

   if (signalMethod == nullptr) {
      throw std::invalid_argument("connect(): Can not connect, signal is null");
   }

   std::unique_ptr<Internal::Bento<void (SignalClass::*)(SignalArgs...)>>
                  signalMethod_Bento(new Internal::Bento<void (SignalClass::*)(SignalArgs...)>(signalMethod));

   std::unique_ptr<Internal::Bento<T>> slotLambda_Bento(new Internal::Bento<T>(slotLambda));

   std::unique_lock<std::mutex> senderLock {sender.m_mutex_connectList};

   if (uniqueConnection) {
      // ensure the connection is not added twice

      for (auto &item : sender.m_connectList) {

         if (item.receiver != &receiver) {
            continue;
         }

         if (*(item.signalMethod) != *(signalMethod_Bento))  {
            continue;
         }

         // unable to test if the passed slotLambda = slotLambda_Bento

         // connection already exists
         return false;
      }
   }

   sender.addConnection(std::move(signalMethod_Bento), &receiver, std::move(slotLambda_Bento), type);

   return true;
}


// signal & slot bento
template<class Sender, class Receiver>
bool connect(const Sender &sender, std::unique_ptr<Internal::BentoAbstract> signalMethod_Bento, const Receiver &receiver,
                  std::unique_ptr<Internal::BentoAbstract> slotMethod_Bento, ConnectionKind type, bool uniqueConnection)
{
   std::unique_lock<std::mutex> senderLock{sender.m_mutex_connectList};

   if (uniqueConnection) {
      // ensure the connection is not added twice

      for (auto &item : sender.m_connectList) {

         if (item.receiver != &receiver) {
            continue;
         }

         if (*(item.signalMethod) != *(signalMethod_Bento))  {
            continue;
         }

         if (*(item.slotMethod) != *(slotMethod_Bento))  {
            continue;
         }

         // connection already exists
         return false;
      }
   }

   sender.addConnection(std::move(signalMethod_Bento), &receiver, std::move(slotMethod_Bento), type);

   return true;
}

// signal & slot method ptr
template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class SlotClass, class ...SlotArgs, class SlotReturn>
bool disconnect(const Sender &sender, void (SignalClass::*signalMethod)(SignalArgs...), const Receiver &receiver,
                  SlotReturn (SlotClass::*slotMethod)(SlotArgs...))
{
   // Sender must be the same class as SignalClass and Sender is a child of SignalClass
   Internal::cs_testConnect_SenderSignal<Sender, SignalClass>();

   // Receiver must be the same class as SlotClass and Receiver is a child of SlotClass
   Internal::cs_testConnect_ReceiverSlot<SlotClass, Receiver>();

   // signal & slot arguments do not agree
   Internal::cs_testConnect_SignalSlotArgs_2< void (*)(SignalArgs...), void (*)(SlotArgs...) >();

   Internal::Bento<void (SignalClass::*)(SignalArgs...)> signalMethod_Bento(signalMethod);
   Internal::Bento<void (SlotClass::*)(SlotArgs...)> slotMethod_Bento(slotMethod);

   if (! internal_disconnect(sender, &signalMethod_Bento, &receiver, &slotMethod_Bento)) {
      return false;
   }

   return true;
}

// signal method ptr, slot lambda or function ptr
template<class Sender, class SignalClass, class ...SignalArgs, class Receiver, class T>
bool disconnect(const Sender &sender, void (SignalClass::*signalMethod)(SignalArgs...), const Receiver &receiver, T slotMethod)
{
   // lambda, compile error
   static_assert(std::is_convertible<decltype(slotMethod == slotMethod), bool>::value,
                 "disconnect():  Slot argument invalid or calling disconnect using a lambda" );

   // function ptr
   Internal::Bento<void (SignalClass::*)(SignalArgs...)> signalMethod_Bento(signalMethod);
   Internal::Bento<T> slotMethod_Bento(slotMethod);

   if (! internal_disconnect(sender, &signalMethod_Bento, &receiver, &slotMethod_Bento)) {
      return false;
   }

   return true;
}

// signal & slot bento objects
template<class Sender, class Receiver>
bool internal_disconnect(const Sender &sender, const Internal::BentoAbstract *signalBento,
                  const Receiver *receiver, const Internal::BentoAbstract *slotBento)
{
   bool retval = false;
   bool isDone = false;

   std::unique_lock<std::mutex> senderLock {sender.m_mutex_connectList};
   std::unique_lock<std::mutex> receiverLock;

   if (receiver != nullptr) {
      receiverLock = std::unique_lock<std::mutex> {receiver->m_mutex_possibleSenders};
   }

   for (int k = 0; k < sender.m_connectList.size(); ++k) {
      SignalBase::ConnectStruct &temp = sender.m_connectList[k];

      if (temp.type == ConnectionKind::InternalDisconnected) {
         // already marked as disconnected
         continue;
      }
 
      bool isMatch = false;

      if (signalBento == nullptr && receiver == nullptr) {
         // delete all connections in Sender
         isMatch = true;

      } else if (receiver != nullptr)  {

         if (receiver == temp.receiver) {

            if (signalBento == nullptr && (slotBento == nullptr || *slotBento == *temp.slotMethod)) {
               isMatch = true;

            } else if (signalBento != nullptr && *signalBento == *temp.signalMethod && (slotBento == nullptr ||
                       *slotBento == *temp.slotMethod)) {
               isMatch = true;

            }
         }

      } else if (signalBento != nullptr) {
         // receiver must be null therefore slot is null

         if (*signalBento == *temp.signalMethod) {
            isMatch = true;
         }
      }

      if (isMatch)  {
         // delete possible sender in the receiver
         retval = true;

         if (receiver == nullptr)  {
            // no receiver, lock temp.receiver now
            receiverLock = std::unique_lock<std::mutex> {temp.receiver->m_mutex_possibleSenders};

            auto &senderList = temp.receiver->m_possibleSenders;
            senderList.erase(std::find(senderList.rbegin(), senderList.rend(), &sender).base() - 1);

            receiverLock.unlock();

         } else {
            auto &senderList = temp.receiver->m_possibleSenders;
            senderList.erase(std::find(senderList.rbegin(), senderList.rend(), &sender).base() - 1);

         }

         if (sender.m_activateBusy != 0) {

            if (! isDone) {
               // warn activate() the connectList has changed
               sender.m_raceCount++;
               isDone = true;
            }

            // mark connection for deletion, activate() will finish the clean up
            temp.type = ConnectionKind::InternalDisconnected;

         } else {
            // delete conneciton in sender
            sender.m_connectList.erase(sender.m_connectList.begin() + k);

            // yes, this is required
            k = k - 1;
         }

      }
   }

   return retval;
}


}

#endif

