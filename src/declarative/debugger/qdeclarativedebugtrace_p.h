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

#ifndef QDECLARATIVEDEBUGTRACE_P_H
#define QDECLARATIVEDEBUGTRACE_P_H

#include <qdeclarativedebugservice_p.h>
#include <qelapsedtimer.h>

QT_BEGIN_NAMESPACE

struct QDeclarativeDebugData {
   qint64 time;
   int messageType;
   int detailType;

   //###
   QString detailData; //used by RangeData and RangeLocation
   int line;           //used by RangeLocation

   QByteArray toByteArray() const;
};

class QUrl;
class Q_DECLARATIVE_EXPORT QDeclarativeDebugTrace : public QDeclarativeDebugService
{
 public:
   enum Message {
      Event,
      RangeStart,
      RangeData,
      RangeLocation,
      RangeEnd,
      Complete,

      MaximumMessage
   };

   enum EventType {
      FramePaint,
      Mouse,
      Key,

      MaximumEventType
   };

   enum RangeType {
      Painting,
      Compiling,
      Creating,
      Binding,            //running a binding
      HandlingSignal,     //running a signal handler

      MaximumRangeType
   };

   static void addEvent(EventType);

   static void startRange(RangeType);
   static void rangeData(RangeType, const QString &);
   static void rangeData(RangeType, const QUrl &);
   static void rangeLocation(RangeType, const QString &, int);
   static void rangeLocation(RangeType, const QUrl &, int);
   static void endRange(RangeType);

   QDeclarativeDebugTrace();

#ifdef CUSTOM_DECLARATIVE_DEBUG_TRACE_INSTANCE

 public:
   static QDeclarativeDebugTrace *globalInstance();
   static void setGlobalInstance(QDeclarativeDebugTrace *custom_instance);

 protected:
   virtual void messageReceived(const QByteArray &);
   virtual void addEventImpl(EventType);
   virtual void startRangeImpl(RangeType);
   virtual void rangeDataImpl(RangeType, const QString &);
   virtual void rangeDataImpl(RangeType, const QUrl &);
   virtual void rangeLocationImpl(RangeType, const QString &, int);
   virtual void rangeLocationImpl(RangeType, const QUrl &, int);
   virtual void endRangeImpl(RangeType);
#else

 protected:
   virtual void messageReceived(const QByteArray &);
 private:
   void addEventImpl(EventType);
   void startRangeImpl(RangeType);
   void rangeDataImpl(RangeType, const QString &);
   void rangeDataImpl(RangeType, const QUrl &);
   void rangeLocationImpl(RangeType, const QString &, int);
   void rangeLocationImpl(RangeType, const QUrl &, int);
   void endRangeImpl(RangeType);
#endif
   void processMessage(const QDeclarativeDebugData &);
   void sendMessages();
   QElapsedTimer m_timer;
   bool m_enabled;
   bool m_deferredSend;
   bool m_messageReceived;
   QList<QDeclarativeDebugData> m_data;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEDEBUGTRACE_P_H

