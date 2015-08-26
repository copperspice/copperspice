/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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
#include <qthread_p.h>
#include <QAbstractEventDispatcher>

void CSInternalChildren::deleteChildren(QObject *object)
{
   if (! object) {
      return;
   }

   object->deleteChildren();
}

void CSInternalChildren::moveChildren(QObject *object, int from, int to)
{
   if (! object) {
      return;
   }

   object->m_children.move(from, to);
}

void CSInternalChildren::removeOne(QObject *object, QObject *value)
{
   if (! object) {
      return;
   }

   object->m_children.removeOne(value);
}

void CSInternalChildren::set_mParent(QObject *object, QObject *value)
{
   if (! object) {
      return;
   }

   // hard setting this member!
   object->m_parent = value;
}

// **

CSAbstractDeclarativeData *CSInternalDeclarativeData::get_m_declarativeData(const QObject *object)
{
   if (! object) {
      return 0;
   }

   return object->m_declarativeData;
}

void CSInternalDeclarativeData::set_m_declarativeData(QObject *object, CSAbstractDeclarativeData *value)
{
   if (! object) {
      return;
   }

   object->m_declarativeData = value;
}


// **

bool CSInternalEvents::get_m_sendChildEvents(const QObject *object)
{
   if (! object) {
      return 0;
   }

   return object->m_sendChildEvents;
}

bool CSInternalEvents::get_m_receiveChildEvents(const QObject *object)
{
   if (! object) {
      return 0;
   }

   return object->m_receiveChildEvents;
}

int CSInternalEvents::get_m_PostedEvents(const QObject *object)
{
   if (! object) {
      return 0;
   }

   return object->m_postedEvents;
}

QList<QPointer<QObject> > &CSInternalEvents::get_m_EventFilters(QObject *object)
{
   if (! object) {
      static QList<QPointer<QObject> > emptyList;
      return emptyList;
   }

   return object->m_eventFilters;
}

std::atomic<bool> &CSInternalEvents::get_m_inThreadChangeEvent(QObject *object)
{
   if (! object) {
      static std::atomic<bool> emptyAtomic;
      return emptyAtomic;
   }

   return object->m_inThreadChangeEvent;
}

void CSInternalEvents::set_m_sendChildEvents(QObject *object, bool data)
{
   if (! object) {
      return;
   }

   object->m_sendChildEvents = data;
}

void CSInternalEvents::set_m_receiveChildEvents(QObject *object, bool data)
{
   if (! object) {
      return;
   }

   object->m_receiveChildEvents = data;
}


// **
bool CSInternalRefCount::get_m_wasDeleted(const QObject *object)
{
   if (! object) {
      return false;
   }

   return object->m_wasDeleted;
}

void CSInternalRefCount::set_m_wasDeleted(QObject *object, bool data)
{
   if (! object) {
      return;
   }

   object->m_wasDeleted = data;
}

std::atomic<QtSharedPointer::ExternalRefCountData *> &CSInternalRefCount::get_m_SharedRefCount(const QObject *object)
{
   if (! object) {
      static std::atomic<QtSharedPointer::ExternalRefCountData *> emptyAtomic;
      return emptyAtomic;
   }

   return object->m_sharedRefCount;
}


// **
bool CSInternalSender::isSender(const QObject *object, const QObject *receiver, const char *signal)
{
   if (! object) {
      return false;
   }

   return object->isSender(receiver, signal);
}


QList<QObject *> CSInternalSender::receiverList(const QObject *object, const char *signal)
{
   if (! object) {
      return QList<QObject *> {};
   }

   return object->receiverList(signal);
}

QList<QObject *> CSInternalSender::senderList(const QObject *object)
{
   if (! object) {
      return QList<QObject *> {};
   }

   return object->senderList();
}


// **
QThreadData *CSInternalThreadData::get_m_ThreadData(const QObject *object)
{
   if (! object) {
      return nullptr;
   }

   // returns a pointer to the threadData
   return object->m_threadData.load();
}

std::atomic<QThreadData *> &CSInternalThreadData::get_AtomicThreadData(QObject *object)
{
   if (! object) {
      static std::atomic<QThreadData *> emptyAtomic;
      return emptyAtomic;
   }

   // returns a reference to the atomic var whichs contains a pointer to the thread data
   return object->m_threadData;
}

void CSInternalEvents::incr_PostedEvents(QObject *object)
{
   if (! object) {
      return;
   }

   ++(object->m_postedEvents);
}

void CSInternalEvents::decr_PostedEvents(QObject *object)
{
   if (! object) {
      return;
   }

   --(object->m_postedEvents);
}


// **
bool QObject::internal_disconnect(const QObject *sender, const BentoAbstract *signalBento,
                                  const QObject *receiver, const BentoAbstract *slotBento)
{
   bool retval = false;
   bool isDone = false;

   if (! sender) {
      return false;
   }

   std::unique_lock<std::mutex> senderLock {sender->m_mutex_ToReceiver};
   std::unique_lock<std::mutex> receiverLock;

   if (receiver) {
      receiverLock = std::unique_lock<std::mutex> {receiver->m_mutex_FromSender};
   }

   bool isMatch;

   for (int k = 0; k < sender->m_connectList_ToReceiver.count(); ++k) {
      ConnectStruct &temp = sender->m_connectList_ToReceiver[k];

      if (temp.sender == 0) {
         // connection is marked for deletion
         continue;
      }

      isMatch = false;

      if (signalBento == 0 && receiver == 0) {
         // delete all connections in Sender
         isMatch = true;

      } else if (receiver != 0)  {

         if (receiver == temp.receiver) {

            if (signalBento == 0 && (slotBento == 0 || *slotBento == *temp.slotMethod)) {
               isMatch = true;

            } else if (signalBento != 0 && *signalBento == *temp.signalMethod && (slotBento == 0 ||
                       *slotBento == *temp.slotMethod)) {
               isMatch = true;

            }
         }

      } else if (signalBento != 0) {
         // receiver must be zero therefore slot is zero

         if (*signalBento == *temp.signalMethod) {
            isMatch = true;
         }
      }

      if (isMatch)  {
         // delete connection in Receiver
         retval = true;

         if (! receiver)  {
            // no receiver was locked, lock temp.receiver now
            receiverLock = std::unique_lock<std::mutex> {temp.receiver->m_mutex_FromSender};
         }

         for (int x = 0; x < temp.receiver->m_connectList_FromSender.count(); ++x)  {
            const ConnectStruct &listR = temp.receiver->m_connectList_FromSender[x];

            if (temp.sender == listR.sender && *temp.signalMethod == *listR.signalMethod &&
                  *temp.slotMethod == *listR.slotMethod) {

               temp.receiver->m_connectList_FromSender.removeAt(x);

               // yes, this is required
               x = x - 1;
            }
         }

         if (! receiver)  {
            receiverLock.unlock();
         }

         if (sender->m_activateBusy) {

            if (! isDone) {
               // warns activate the connectList has changed
               sender->m_raceCount++;
               isDone = true;
            }

            // mark connection for deletion, activate() will finish
            temp.sender = 0;

         } else {
            // delete conneciton in Sender
            sender->m_connectList_ToReceiver.removeAt(k);

            // yes, this is required
            k = k - 1;
         }

      }
   }

   return retval;
}

// private slot
void QObject::internal_reregisterTimers(QList< std::pair<int, int> > timerList)
{
   QAbstractEventDispatcher *eventDispatcher = m_threadData.load()->eventDispatcher;

   for (int k = 0; k < timerList.size(); ++k) {
      const std::pair<int, int> &pair = timerList.at(k);
      eventDispatcher->registerTimer(pair.first, pair.second, this);
   }
}

// not currently using this method
int *QObject::queuedConnectionTypes(const QList<QByteArray> &typeNames)
{
   int *types = new int [typeNames.count() + 1];
   Q_CHECK_PTR(types);

   for (int i = 0; i < typeNames.count(); ++i) {
      const QByteArray typeName = typeNames.at(i);

      if (typeName.endsWith('*')) {
         types[i] = QMetaType::VoidStar;

      }  else {
         types[i] = QMetaType::type(typeName);

      }

      if (! types[i]) {
         qWarning("QObject::connect() Can not queue arguments of type '%s'\n"
                  "(Make sure '%s' is registered using qRegisterMetaType())",
                  typeName.constData(), typeName.constData());
         delete [] types;
         return 0;
      }
   }

   types[typeNames.count()] = 0;

   return types;
}

// ** macros defined in csmeta.h

// classes
CS_REGISTER_CLASS(QAbstractState)
CS_REGISTER_CLASS(QColor)
CS_REGISTER_CLASS(QChar)
CS_REGISTER_CLASS(QCursor)
CS_REGISTER_CLASS(QBitmap)
CS_REGISTER_CLASS(QBrush)
CS_REGISTER_CLASS(QBitArray)
CS_REGISTER_CLASS(QByteArray)
CS_REGISTER_CLASS(QDate)
CS_REGISTER_CLASS(QDateTime)
CS_REGISTER_CLASS(QEasingCurve)
CS_REGISTER_CLASS(QFont)
CS_REGISTER_CLASS(QGraphicsEffect)
CS_REGISTER_CLASS(QGraphicsLayout)
CS_REGISTER_CLASS(QIcon)
CS_REGISTER_CLASS(QImage)
CS_REGISTER_CLASS(QJsonValue)
CS_REGISTER_CLASS(QJsonObject)
CS_REGISTER_CLASS(QJsonArray)
CS_REGISTER_CLASS(QJsonDocument)
CS_REGISTER_CLASS(QKeySequence)
CS_REGISTER_CLASS(QLine)
CS_REGISTER_CLASS(QLineF)
CS_REGISTER_CLASS(QLocale)  
CS_REGISTER_CLASS(QMatrix)
CS_REGISTER_CLASS(QMatrix4x4)
CS_REGISTER_CLASS(QModelIndex)
CS_REGISTER_CLASS(QPalette)
CS_REGISTER_CLASS(QPen)
CS_REGISTER_CLASS(QPoint)
CS_REGISTER_CLASS(QPointF)
CS_REGISTER_CLASS(QPolygon)
CS_REGISTER_CLASS(QPolygonF)
CS_REGISTER_CLASS(QPixmap)
CS_REGISTER_CLASS(QRect)
CS_REGISTER_CLASS(QRectF)
CS_REGISTER_CLASS(QRegion)
CS_REGISTER_CLASS(QRegExp)
CS_REGISTER_CLASS(QRegularExpression)
CS_REGISTER_CLASS(QSize)
CS_REGISTER_CLASS(QSizeF)
CS_REGISTER_CLASS(QSizePolicy)
CS_REGISTER_CLASS(QState)
CS_REGISTER_CLASS(QString)
CS_REGISTER_CLASS(QStringList)
CS_REGISTER_CLASS(QStyleOption)
CS_REGISTER_CLASS(QStyleOptionViewItem)
CS_REGISTER_CLASS(QTextCursor)
CS_REGISTER_CLASS(QTextFormat)
CS_REGISTER_CLASS(QTextLength)
CS_REGISTER_CLASS(QTextOption)
CS_REGISTER_CLASS(QTime)
CS_REGISTER_CLASS(QTransform)
CS_REGISTER_CLASS(QQuaternion)
CS_REGISTER_CLASS(QUrl)
CS_REGISTER_CLASS(QUuid)
CS_REGISTER_CLASS(QVariant)
CS_REGISTER_CLASS(QVector2D)
CS_REGISTER_CLASS(QVector3D)
CS_REGISTER_CLASS(QVector4D)

// primitive
CS_REGISTER_TYPE(bool)
CS_REGISTER_TYPE(char)
CS_REGISTER_TYPE(signed char)
CS_REGISTER_TYPE(unsigned char)
CS_REGISTER_TYPE(double)
CS_REGISTER_TYPE(long double)
CS_REGISTER_TYPE(float)
CS_REGISTER_TYPE(int)
CS_REGISTER_TYPE(unsigned int)
CS_REGISTER_TYPE(long)
CS_REGISTER_TYPE(unsigned long)
CS_REGISTER_TYPE(long long)
CS_REGISTER_TYPE(unsigned long long)
CS_REGISTER_TYPE(short)
CS_REGISTER_TYPE(unsigned short)
CS_REGISTER_TYPE(void)

