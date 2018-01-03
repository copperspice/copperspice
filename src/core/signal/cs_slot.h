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

#ifndef LIB_CS_SLOT_H
#define LIB_CS_SLOT_H

#include <atomic>
#include <memory>
#include <set>
#include <thread>
#include <vector>

#include "cs_macro.h"
#include "rcu_guarded.hpp"
#include "rcu_list.hpp"

namespace CsSignal {

class SignalBase;
class SlotBase;
enum class ConnectionKind;

namespace Internal {
   class BentoAbstract;
   class TeaCupAbstract;
}

class LIB_SIG_EXPORT PendingSlot
{
   public:
      PendingSlot(const PendingSlot &) = delete;
      PendingSlot(PendingSlot &&) = default;

      PendingSlot(SignalBase *sender, std::unique_ptr<Internal::BentoAbstract> signal_Bento, SlotBase *receiver,
                     std::unique_ptr<Internal::BentoAbstract>  slot_Bento,
                     std::unique_ptr<Internal::TeaCupAbstract> teaCup_Data);

      SignalBase *sender() const {
         return m_sender;
      }

      SlotBase *receiver() const {
         return m_receiver;
      }

      std::unique_ptr<Internal::BentoAbstract> internal_moveSlotBento() {
         return std::move(m_slot_Bento);
      }

      std::unique_ptr<Internal::TeaCupAbstract> internal_moveTeaCup() {
         return std::move(m_teaCup_Data);
      }

      void operator()() const;

   private:
      SignalBase *m_sender;
      std::unique_ptr<Internal::BentoAbstract>  m_signal_Bento;
      SlotBase *m_receiver;
      std::unique_ptr<Internal::BentoAbstract>  m_slot_Bento;
      std::unique_ptr<Internal::TeaCupAbstract> m_teaCup_Data;
};

class LIB_SIG_EXPORT SlotBase
{
   public:
      SlotBase();
      SlotBase(const SlotBase &);
      virtual ~SlotBase();

      // SlotBase(SlotBase &&);
      // operator=(const SlotBase &);
      // operator=(SlotBase &&);

      SignalBase *sender() const;

   protected:
      std::set<SignalBase *> internal_senderList() const;

   private:
      static SignalBase *&get_threadLocal_currentSender();

      // list of possible Senders for this Receiver
      mutable LibG::SharedList<const SignalBase *> m_possibleSenders;

      virtual bool compareThreads() const;
      virtual void queueSlot(PendingSlot data, ConnectionKind type);

      friend class SignalBase;

      template<class Sender, class SignalClass, class ...SignalArgTypes, class ...Ts>
      friend void activate(Sender &sender, void (SignalClass::*signal)(SignalArgTypes...), Ts &&... Vs);

      template<class Sender, class Receiver>
      friend bool internal_disconnect(const Sender &sender, const Internal::BentoAbstract *signalBento,
                  const Receiver *receiver, const Internal::BentoAbstract *slotBento);
};


}

#endif