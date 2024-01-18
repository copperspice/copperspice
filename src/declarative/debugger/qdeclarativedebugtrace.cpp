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

#include "qdeclarativedebugtrace_p.h"

#include <QtCore/qdatastream.h>
#include <QtCore/qurl.h>
#include <QtCore/qtimer.h>
#include <QDebug>

#ifdef CUSTOM_DECLARATIVE_DEBUG_TRACE_INSTANCE

namespace {

class GlobalInstanceDeleter
{
 private:
   QAtomicPointer<QDeclarativeDebugTrace> &m_pointer;

 public:
   GlobalInstanceDeleter(QAtomicPointer<QDeclarativeDebugTrace> &p)
      : m_pointer(p) {
   }

   ~GlobalInstanceDeleter() {
      delete m_pointer;
      m_pointer = 0;
   }
};

QAtomicPointer<QDeclarativeDebugTrace> s_globalInstance = 0;
}


static QDeclarativeDebugTrace *traceInstance()
{
   return QDeclarativeDebugTrace::globalInstance();
}

QDeclarativeDebugTrace *QDeclarativeDebugTrace::globalInstance()
{
   if (!s_globalInstance) {
      // create default QDeclarativeDebugTrace instance if it is not explicitly set by setGlobalInstance(QDeclarativeDebugTrace *instance)
      // thread safe implementation
      QDeclarativeDebugTrace *x = new QDeclarativeDebugTrace();
      if (!s_globalInstance.testAndSetOrdered(0, x)) {
         delete x;
      } else {
         static GlobalInstanceDeleter cleanup(s_globalInstance);
      }
   }
   return s_globalInstance;
}

/*!
 *  Set custom QDeclarativeDebugTrace instance \a custom_instance.
 *  Function fails if QDeclarativeDebugTrace::globalInstance() was called before.
 *  QDeclarativeDebugTrace framework takes ownership of the custom instance.
 */
void QDeclarativeDebugTrace::setGlobalInstance(QDeclarativeDebugTrace *custom_instance)
{
   if (!s_globalInstance.testAndSetOrdered(0, custom_instance)) {
      qWarning() << "QDeclarativeDebugTrace::setGlobalInstance() - instance already set.";
      delete custom_instance;
   } else {
      static GlobalInstanceDeleter cleanup(s_globalInstance);
   }
}

#else // CUSTOM_DECLARATIVE_DEBUG_TRACE_INSTANCE
Q_GLOBAL_STATIC(QDeclarativeDebugTrace, traceInstance);
#endif

// convert to a QByteArray that can be sent to the debug client
// use of QDataStream can skew results if m_deferredSend == false
//     (see tst_qdeclarativedebugtrace::trace() benchmark)
QByteArray QDeclarativeDebugData::toByteArray() const
{
   QByteArray data;
   //### using QDataStream is relatively expensive
   QDataStream ds(&data, QIODevice::WriteOnly);
   ds << time << messageType << detailType;
   if (messageType == (int)QDeclarativeDebugTrace::RangeData) {
      ds << detailData;
   }
   if (messageType == (int)QDeclarativeDebugTrace::RangeLocation) {
      ds << detailData << line;
   }
   return data;
}

QDeclarativeDebugTrace::QDeclarativeDebugTrace()
   : QDeclarativeDebugService(QLatin1String("CanvasFrameRate")),
     m_enabled(false), m_deferredSend(true), m_messageReceived(false)
{
   m_timer.start();
   if (status() == Enabled) {
      // wait for first message indicating whether to trace or not
      while (!m_messageReceived) {
         waitForMessage();
      }
   }
}

void QDeclarativeDebugTrace::addEvent(EventType t)
{
   if (QDeclarativeDebugService::isDebuggingEnabled()) {
      traceInstance()->addEventImpl(t);
   }
}

void QDeclarativeDebugTrace::startRange(RangeType t)
{
   if (QDeclarativeDebugService::isDebuggingEnabled()) {
      traceInstance()->startRangeImpl(t);
   }
}

void QDeclarativeDebugTrace::rangeData(RangeType t, const QString &data)
{
   if (QDeclarativeDebugService::isDebuggingEnabled()) {
      traceInstance()->rangeDataImpl(t, data);
   }
}

void QDeclarativeDebugTrace::rangeData(RangeType t, const QUrl &data)
{
   if (QDeclarativeDebugService::isDebuggingEnabled()) {
      traceInstance()->rangeDataImpl(t, data);
   }
}

void QDeclarativeDebugTrace::rangeLocation(RangeType t, const QString &fileName, int line)
{
   if (QDeclarativeDebugService::isDebuggingEnabled()) {
      traceInstance()->rangeLocationImpl(t, fileName, line);
   }
}

void QDeclarativeDebugTrace::rangeLocation(RangeType t, const QUrl &fileName, int line)
{
   if (QDeclarativeDebugService::isDebuggingEnabled()) {
      traceInstance()->rangeLocationImpl(t, fileName, line);
   }
}

void QDeclarativeDebugTrace::endRange(RangeType t)
{
   if (QDeclarativeDebugService::isDebuggingEnabled()) {
      traceInstance()->endRangeImpl(t);
   }
}

void QDeclarativeDebugTrace::addEventImpl(EventType event)
{
   if (status() != Enabled || !m_enabled) {
      return;
   }

   QDeclarativeDebugData ed = {m_timer.nsecsElapsed(), (int)Event, (int)event, QString(), -1};
   processMessage(ed);
}

void QDeclarativeDebugTrace::startRangeImpl(RangeType range)
{
   if (status() != Enabled || !m_enabled) {
      return;
   }

   QDeclarativeDebugData rd = {m_timer.nsecsElapsed(), (int)RangeStart, (int)range, QString(), -1};
   processMessage(rd);
}

void QDeclarativeDebugTrace::rangeDataImpl(RangeType range, const QString &rData)
{
   if (status() != Enabled || !m_enabled) {
      return;
   }

   QDeclarativeDebugData rd = {m_timer.nsecsElapsed(), (int)RangeData, (int)range, rData, -1};
   processMessage(rd);
}

void QDeclarativeDebugTrace::rangeDataImpl(RangeType range, const QUrl &rData)
{
   if (status() != Enabled || !m_enabled) {
      return;
   }

   QDeclarativeDebugData rd = {m_timer.nsecsElapsed(), (int)RangeData, (int)range, rData.toString(QUrl::FormattingOption(0x100)), -1};
   processMessage(rd);
}

void QDeclarativeDebugTrace::rangeLocationImpl(RangeType range, const QString &fileName, int line)
{
   if (status() != Enabled || !m_enabled) {
      return;
   }

   QDeclarativeDebugData rd = {m_timer.nsecsElapsed(), (int)RangeLocation, (int)range, fileName, line};
   processMessage(rd);
}

void QDeclarativeDebugTrace::rangeLocationImpl(RangeType range, const QUrl &fileName, int line)
{
   if (status() != Enabled || !m_enabled) {
      return;
   }

   QDeclarativeDebugData rd = {m_timer.nsecsElapsed(), (int)RangeLocation, (int)range, fileName.toString(QUrl::FormattingOption(0x100)), line};
   processMessage(rd);
}

void QDeclarativeDebugTrace::endRangeImpl(RangeType range)
{
   if (status() != Enabled || !m_enabled) {
      return;
   }

   QDeclarativeDebugData rd = {m_timer.nsecsElapsed(), (int)RangeEnd, (int)range, QString(), -1};
   processMessage(rd);
}

/*
    Either send the message directly, or queue up
    a list of messages to send later (via sendMessages)
*/
void QDeclarativeDebugTrace::processMessage(const QDeclarativeDebugData &message)
{
   if (m_deferredSend) {
      m_data.append(message);
   } else {
      sendMessage(message.toByteArray());
   }
}

/*
    Send the messages queued up by processMessage
*/
void QDeclarativeDebugTrace::sendMessages()
{
   if (m_deferredSend) {
      //### this is a suboptimal way to send batched messages
      for (int i = 0; i < m_data.count(); ++i) {
         sendMessage(m_data.at(i).toByteArray());
      }
      m_data.clear();

      //indicate completion
      QByteArray data;
      QDataStream ds(&data, QIODevice::WriteOnly);
      ds << (qint64) - 1 << (int)Complete;
      sendMessage(data);
   }
}

void QDeclarativeDebugTrace::messageReceived(const QByteArray &message)
{
   QByteArray rwData = message;
   QDataStream stream(&rwData, QIODevice::ReadOnly);

   stream >> m_enabled;

   m_messageReceived = true;

   if (!m_enabled) {
      sendMessages();
   }
}
