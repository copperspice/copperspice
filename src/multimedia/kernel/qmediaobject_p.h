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

#ifndef QMEDIAOBJECT_P_H
#define QMEDIAOBJECT_P_H

#include <qbytearray.h>
#include <qset.h>
#include <qtimer.h>
#include <qmediaobject.h>

class QMetaDataReaderControl;
class QMediaAvailabilityControl;

#define Q_DECLARE_NON_CONST_PUBLIC(T) \
    inline T *q_func() { return static_cast<T *>(q_ptr); } \
    friend class Ts;

class QMediaObjectPrivate
{
   Q_DECLARE_PUBLIC(QMediaObject)

 public:
   QMediaObjectPrivate()
      : service(nullptr), metaDataControl(nullptr), availabilityControl(nullptr), notifyTimer(nullptr), q_ptr(nullptr)
   {}

   virtual ~QMediaObjectPrivate()
   {}

   void _q_notify();
   void _q_availabilityChanged();

   QMediaService *service;
   QMetaDataReaderControl *metaDataControl;
   QMediaAvailabilityControl *availabilityControl;

   QTimer *notifyTimer;
   QMap<QString, std::function<void ()>  > notifyProperties;

   QMediaObject *q_ptr;
};

#endif
