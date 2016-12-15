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

#include "cs_signal.h"
#include "cs_slot.h"

CsSignal::SlotBase::SlotBase()
{
}

CsSignal::SlotBase::SlotBase(const SlotBase &)   
{
}

CsSignal::SlotBase::~SlotBase()
{
   try {
      // clean up possible sender connections
      std::unique_lock<std::mutex> receiverLock {m_mutex_possibleSenders};

      std::vector<const SignalBase *> tmp_possibleSenders;   
      swap(tmp_possibleSenders, m_possibleSenders);
      receiverLock.unlock();
   
      for (auto sender : tmp_possibleSenders) {
         std::lock_guard<std::mutex> senderLock {sender->m_mutex_connectList}; 

         if (sender->m_activateBusy > 0)  {
   
            for (auto &item : sender->m_connectList)  {
               if (item.receiver == this) {
                  item.type = ConnectionKind::InternalDisconnected;
               }
            }
   
         } else {             
            sender->m_connectList.erase(std::remove_if(sender->m_connectList.begin(), sender->m_connectList.end(), 
                  [this](const SignalBase::ConnectStruct & tmp){ return tmp.receiver == this; } ));   
         }
      }

   } catch (...) {
      if (! std::uncaught_exception()) {
         throw;
      }                       
   }
}

CsSignal::SignalBase *&CsSignal::SlotBase::get_threadLocal_currentSender()
{
#ifdef __APPLE__ 
   static __thread CsSignal::SignalBase *threadLocal_currentSender = nullptr;
#else
   static thread_local CsSignal::SignalBase *threadLocal_currentSender = nullptr;
#endif

   return threadLocal_currentSender;
}

bool CsSignal::SlotBase::compareThreads() const
{
   return true;
}

void CsSignal::SlotBase::queueSlot(PendingSlot data, ConnectionKind)
{   
   // calls the slot immediately
   data();
} 

CsSignal::SignalBase *CsSignal::SlotBase::sender() const
{
   return get_threadLocal_currentSender();
}

std::set<CsSignal::SignalBase *> CsSignal::SlotBase::internal_senderList() const
{
   std::set<SignalBase *> retval;  

   std::lock_guard<std::mutex> receiverLock {m_mutex_possibleSenders};
   
   for (auto sender : m_possibleSenders) {
      retval.insert(const_cast<SignalBase *>(sender));
   }

   return retval;
}

CsSignal::PendingSlot::PendingSlot(SignalBase *sender, std::unique_ptr<Internal::BentoAbstract> signal_Bento, 
                  SlotBase *receiver, std::unique_ptr<Internal::BentoAbstract> slot_Bento, 
                  std::unique_ptr<Internal::TeaCupAbstract> teaCup_Data)
   : m_sender(sender), m_signal_Bento(std::move(signal_Bento)), m_receiver(receiver), 
     m_slot_Bento(std::move(slot_Bento)), m_teaCup_Data(std::move(teaCup_Data))
{
}

void CsSignal::PendingSlot::operator()() const
{
   // invoke the slot
   m_slot_Bento->invoke(m_receiver, m_teaCup_Data.get());
} 

