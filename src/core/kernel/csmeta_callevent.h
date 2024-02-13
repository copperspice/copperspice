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

#ifndef CSMETA_CALLEVENT_H
#define CSMETA_CALLEVENT_H

class QObject;
class QSemaphore;

class Q_CORE_EXPORT CSMetaCallEvent : public QEvent
{
 public:
   CSMetaCallEvent(const CsSignal::Internal::BentoAbstract *bento, const CsSignal::Internal::TeaCupAbstract *dataPack,
         const QObject *sender, int signal_index, QSemaphore *semaphore = nullptr);

   ~CSMetaCallEvent();

   void placeMetaCall(QObject *object);
   const QObject *sender() const;
   int signal_index() const;

 private:
   const CsSignal::Internal::BentoAbstract *m_bento;
   const CsSignal::Internal::TeaCupAbstract *m_dataPack;

   const QObject *m_sender;
   QSemaphore *m_semaphore;
   int m_signal_index;
};

#endif
