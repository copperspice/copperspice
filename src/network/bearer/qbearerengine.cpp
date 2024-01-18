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

#include <qbearerengine_p.h>

#ifndef QT_NO_BEARERMANAGEMENT

QBearerEngine::QBearerEngine(QObject *parent)
   : QObject(parent)
{
}

QBearerEngine::~QBearerEngine()
{
   QHash<QString, QNetworkConfigurationPrivatePointer>::iterator it;
   QHash<QString, QNetworkConfigurationPrivatePointer>::iterator end;

   for (it = snapConfigurations.begin(), end = snapConfigurations.end(); it != end; ++it) {
      it.value()->isValid = false;
      it.value()->id.clear();
   }
   snapConfigurations.clear();

   for (it = accessPointConfigurations.begin(), end = accessPointConfigurations.end();
         it != end; ++it) {
      it.value()->isValid = false;
      it.value()->id.clear();
   }
   accessPointConfigurations.clear();

   for (it = userChoiceConfigurations.begin(), end = userChoiceConfigurations.end();
         it != end; ++it) {
      it.value()->isValid = false;
      it.value()->id.clear();
   }
   userChoiceConfigurations.clear();
}

bool QBearerEngine::requiresPolling() const
{
   return false;
}

bool QBearerEngine::configurationsInUse() const
{
   QHash<QString, QNetworkConfigurationPrivatePointer>::const_iterator it;
   QHash<QString, QNetworkConfigurationPrivatePointer>::const_iterator end;

   QRecursiveMutexLocker locker(&mutex);

   for (it = accessPointConfigurations.constBegin(),
         end = accessPointConfigurations.constEnd(); it != end; ++it) {
      if (it.value()->ref.load() > 1) {
         return true;
      }
   }

   for (it = snapConfigurations.constBegin(),
         end = snapConfigurations.constEnd(); it != end; ++it) {
      if (it.value()->ref.load() > 1) {
         return true;
      }
   }

   for (it = userChoiceConfigurations.constBegin(),
         end = userChoiceConfigurations.constEnd(); it != end; ++it) {
      if (it.value()->ref.load() > 1) {
         return true;
      }
   }

   return false;
}

#endif // QT_NO_BEARERMANAGEMENT
