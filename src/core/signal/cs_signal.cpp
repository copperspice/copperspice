/***********************************************************************
*
* Copyright (c) 2015-2018 Barbara Geller
* Copyright (c) 2015-2018 Ansel Sermersheim
* All rights reserved.
*
* This file is part of libCsSignal
*
* libCsSignal is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
***********************************************************************/

#include "cs_signal.h"

CsSignal::SignalBase::~SignalBase()
{
   try {
      auto senderListHandle = m_connectList.lock_read();

      if (m_activateBusy > 0)  {
         // activate() called a slot which then destroys this sender
         std::lock_guard<std::mutex> lock(get_mutex_beingDestroyed());
         get_beingDestroyed().insert(this);
      }

      for (auto &item : *senderListHandle) {
         const SlotBase *receiver = item.receiver;

         if (receiver != nullptr) {
            auto receiverListHandle = receiver->m_possibleSenders.lock_write();

            auto iter = receiverListHandle->begin();

            while (iter != receiverListHandle->end())   {

               if (*iter == this) {
                  iter = receiverListHandle->erase(iter);
               } else {
                  ++iter;
               }

            }
         }
      }

   } catch (...) {
      if (! std::uncaught_exception()) {
         throw;
      }
   }
}

CsSignal::Internal::BentoAbstract *&CsSignal::SignalBase::get_threadLocal_currentSignal()
{

#ifdef __APPLE__
   static __thread CsSignal::Internal::BentoAbstract *threadLocal_currentSignal = nullptr;
#else
   static thread_local CsSignal::Internal::BentoAbstract *threadLocal_currentSignal = nullptr;
#endif

   return threadLocal_currentSignal;
}

std::mutex &CsSignal::SignalBase::get_mutex_beingDestroyed()
{
   static std::mutex mutex_beingDestroyed;

   return mutex_beingDestroyed;
}

std::unordered_set<const CsSignal::SignalBase *> &CsSignal::SignalBase::get_beingDestroyed()
{
   static std::unordered_set<const CsSignal::SignalBase *> beingDestroyed;

   return beingDestroyed;
}

void CsSignal::SignalBase::addConnection(std::unique_ptr<const Internal::BentoAbstract> signalMethod, const SlotBase *receiver,
                  std::unique_ptr<const Internal::BentoAbstract> slotMethod, ConnectionKind type,
                  LibG::SharedList<ConnectStruct>::write_handle senderListHandle) const
{
   struct ConnectStruct tempStruct;

   tempStruct.signalMethod = std::move(signalMethod);
   tempStruct.receiver     = receiver;
   tempStruct.slotMethod   = std::move(slotMethod);
   tempStruct.type         = type;

   senderListHandle->push_back(std::move(tempStruct));

   // broom - senderListHandle->unlock()

   if (receiver != nullptr)  {
      auto receiverListHandle = receiver->m_possibleSenders.lock_write();
      receiverListHandle->push_back(this);
   }
}

void CsSignal::SignalBase::handleException(std::exception_ptr)
{
}

int CsSignal::SignalBase::internal_cntConnections(const SlotBase *receiver,
                  const Internal::BentoAbstract &signalMethod_Bento) const
{
   int retval = 0;

   auto senderListHandle = m_connectList.lock_read();

   for (auto &item : *senderListHandle) {

      if (receiver && item.receiver != receiver) {
         continue;
      }

      if (*(item.signalMethod) != signalMethod_Bento)  {
         continue;
      }

      retval++;
   }

   return retval;
}

std::set<CsSignal::SlotBase *> CsSignal::SignalBase::internal_receiverList(
                  const Internal::BentoAbstract &signalMethod_Bento) const
{
   std::set<SlotBase *> retval;

   auto senderListHandle = m_connectList.lock_read();

   for (auto &item : *senderListHandle) {

      if (*(item.signalMethod) != signalMethod_Bento)  {
         continue;
      }

      retval.insert(const_cast<SlotBase *>(item.receiver));
   }

   return retval;
}

