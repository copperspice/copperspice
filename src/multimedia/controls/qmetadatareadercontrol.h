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

#ifndef QMETADATAREADERCONTROL_H
#define QMETADATAREADERCONTROL_H

#include <qstring.h>
#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qmediaresource.h>
#include <qmultimedia.h>

class Q_MULTIMEDIA_EXPORT QMetaDataReaderControl : public QMediaControl
{
   MULTI_CS_OBJECT(QMetaDataReaderControl)

 public:
   ~QMetaDataReaderControl();

   virtual bool isMetaDataAvailable() const = 0;

   virtual QVariant metaData(const QString &key) const = 0;
   virtual QStringList availableMetaData() const = 0;

   MULTI_CS_SIGNAL_1(Public, void metaDataChanged())
   MULTI_CS_SIGNAL_OVERLOAD(metaDataChanged, ())

   MULTI_CS_SIGNAL_1(Public, void metaDataChanged(const QString &key, const QVariant &value))
   MULTI_CS_SIGNAL_OVERLOAD(metaDataChanged, (const QString &, const QVariant &), key, value)

   MULTI_CS_SIGNAL_1(Public, void metaDataAvailableChanged(bool available))
   MULTI_CS_SIGNAL_2(metaDataAvailableChanged, available)

 protected:
   explicit QMetaDataReaderControl(QObject *parent = nullptr);
};

#define QMetaDataReaderControl_iid "com.copperspice.CS.metaDataReaderControl/1.0"
CS_DECLARE_INTERFACE(QMetaDataReaderControl, QMetaDataReaderControl_iid)

#endif
