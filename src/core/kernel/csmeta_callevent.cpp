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

// internal class
CSMetaCallEvent::CSMetaCallEvent(const CsSignal::Internal::BentoAbstract *bento, 
                  const CsSignal::Internal::TeaCupAbstract *dataPack, 
                  const QObject *sender, int signal_index, QSemaphore *semaphore)
   : QEvent(MetaCall), m_bento(bento), m_dataPack(dataPack), m_sender(sender),
     m_semaphore(semaphore), m_signal_index(signal_index)
{
}

CSMetaCallEvent::~CSMetaCallEvent()
{
   delete m_dataPack;

   if (m_semaphore) {
      m_semaphore->release();
   }
}

void CSMetaCallEvent::placeMetaCall(QObject *object)
{
   m_bento->invoke(object, m_dataPack);
}

const QObject *CSMetaCallEvent::sender() const
{
   return m_sender;
}

int CSMetaCallEvent::signal_index() const
{
   return m_signal_index;
}
