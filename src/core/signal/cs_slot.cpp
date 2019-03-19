/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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
      auto receiverListHandle = m_possibleSenders.lock_read();

      for (auto &sender : *receiverListHandle) {
         auto senderListHandle = sender->m_connectList.lock_write();

         auto iter = senderListHandle->begin();

         while (iter != senderListHandle->end())   {

            if (iter->receiver == this) {
               iter = senderListHandle->erase(iter);
            } else {
               ++iter;
            }
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

   auto receiverListHandle = m_possibleSenders.lock_read();

   for (auto &sender : *receiverListHandle) {
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

