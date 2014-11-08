/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

static void dumpRecursive(int level, QObject *object)
{

#if defined(QT_DEBUG)

   if (object) {
      QByteArray buffer;

      buffer.fill(' ', level / 2 * 8);
      if (level % 2) {
         buffer += "    ";
      }

      QString name   = object->objectName();
      QString flags  = QLatin1String("");

      qDebug("%s%s::%s %s", (const char *)buffer, object->metaObject()->className(),
             name.toLocal8Bit().data(), flags.toLatin1().data());

      QList<QObject *> children = object->children();

      if (! children.isEmpty()) {
         for (int i = 0; i < children.size(); ++i) {
            dumpRecursive(level + 1, children.at(i));
         }
      }
   }

#endif

}

void QObject::dumpObjectTree()
{
   qDebug("\n--  dumpObjectTree  --\n");
   dumpRecursive(0, this);
   qDebug("--\n");
}

void QObject::dumpObjectInfo()
{

#if defined(QT_DEBUG)

   qDebug("\n--  dumpObjectInfo  --\n");

   qDebug("  OBJECT %s::%s", this->metaObject()->className(),
          objectName().isEmpty() ? "unnamed" : objectName().toLocal8Bit().data());

   std::unique_lock<std::mutex> senderLock {this->m_mutex_ToReceiver};
   std::unique_lock<std::mutex> receiverLock {this->m_mutex_FromSender};

   // look for connections where this object is the sender
   qDebug("  SIGNALS OUT");

   for (auto k = this->m_connectList_ToReceiver.begin(); k != this->m_connectList_ToReceiver.end(); ++k) {
      const ConnectStruct &temp = *k;

      if (temp.sender == 0) {
         // connection is marked for deletion
         continue;
      }

      const QMetaMethod signal = metaObject()->method(*temp.signalMethod);

      qDebug("        signal: %s", signal.methodSignature().constData());

      if (! temp.receiver) {
         qDebug("          <Null receiver>");
         continue;
      }

      const QMetaObject *receiverMetaObject = temp.receiver->metaObject();
      const QMetaMethod method = receiverMetaObject->method(*temp.slotMethod);

      qDebug("          --> %s::%s %s",
             receiverMetaObject->className(),
             temp.receiver->objectName().isEmpty() ? "unnamed" : qPrintable(temp.receiver->objectName()),
             method.methodSignature().constData());
   }

   // look for connections where this object is the receiver
   qDebug("\n  SIGNALS IN");

   for (auto k = this->m_connectList_FromSender.begin(); k != this->m_connectList_FromSender.end(); ++k) {
      const ConnectStruct &temp = *k;

      const QMetaMethod slot = metaObject()->method(*temp.slotMethod);

      qDebug("          <-- %s::%s  %s",
             temp.sender->metaObject()->className(),
             temp.sender->objectName().isEmpty() ? "unnamed" : qPrintable(temp.sender->objectName()),
             slot.methodSignature().constData());
   }

   qDebug("--\n");

#endif

}
