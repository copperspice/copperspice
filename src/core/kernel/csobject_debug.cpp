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

#include <qobject.h>

static void dumpRecursive(int level, QObject *object)
{

#if defined(CS_SHOW_DEBUG_CORE)
   if (object) {
      QByteArray buffer;

      buffer.fill(' ', level / 2 * 8);

      if (level % 2) {
         buffer += "    ";
      }

      qDebug("%s%s::%s", buffer.constData(), csPrintable(object->metaObject()->className()),
            csPrintable(object->objectName()) );

      QList<QObject *> children = object->children();

      if (! children.isEmpty()) {
         for (int i = 0; i < children.size(); ++i) {
            dumpRecursive(level + 1, children.at(i));
         }
      }
   }
#else
   (void) level;
   (void) object;
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

#if defined(CS_SHOW_DEBUG_CORE)
   qDebug("\n--  dumpObjectInfo  --\n");
   qDebug("  OBJECT %s::%s", csPrintable(this->metaObject()->className()), objectName().isEmpty() ? "unnamed" :
         csPrintable(objectName()) );

   qDebug("  SIGNAL LIST - CONNECTED TO WHICH RECEIVERS");

   const QMetaObject *metaObject = this->metaObject();

   for (int index = 0; index < metaObject->methodCount(); ++index)  {
      // iterate over the signals in "this"

      QMetaMethod signalMetaMethod = metaObject->method(index);

      if (signalMetaMethod.methodType() != QMetaMethod::Signal)  {
         continue;
      }

      const CSBentoAbstract *signalMethod_Bento = signalMetaMethod.getBentoBox();
      std::set<SlotBase *> receiverList = internal_receiverList(*signalMethod_Bento);

      // look for connections where "this" object is the sender
      for (auto receiver : receiverList) {
         qDebug("        signal name: %s", csPrintable(signalMetaMethod.methodSignature()) );

         QObject *obj = dynamic_cast<QObject *>(receiver);

         if (obj) {
            const QMetaObject *receiverMetaObject = obj->metaObject();

            // emerald - hold, ok
            // const QMetaMethod slotMetaMethod = receiverMetaObject->method(*temp.slotMethod);

            qDebug("          --> %s::%s", csPrintable(receiverMetaObject->className()),
                  obj->objectName().isEmpty() ? "unnamed" : csPrintable(obj->objectName()) );

         } else {
            // receiver does not inherit from QObject
            qDebug("          --> %s", typeid (*receiver).name() );
         }
      }
   }

   // look for connections where this object is the receiver
   qDebug("\n  SIGNAL LIST - RECEIVER HAS CONNECTION TO WHICH SENDERS");

   std::set<SignalBase *> senderList = internal_senderList();

   for (auto sender : senderList) {

      // review again (can wait)
      // const QMetaMethod slot = metaObject()->method(*temp.slotMethod);

      QObject *obj = dynamic_cast<QObject *>(sender);

      if (obj) {
         const QMetaObject *senderMetaObject = obj->metaObject();

         qDebug("          <-- %s::%s", csPrintable(senderMetaObject->className()),
               obj->objectName().isEmpty() ? "unnamed" : csPrintable(obj->objectName()));

      } else {
         // sender does not inherit from QObject
         qDebug("          --> %s", typeid (*sender).name() );

      }
   }

   qDebug("--\n");
#endif

}
