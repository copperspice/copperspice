/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#ifndef CAMERABINCAPTUREMETADATACONTROL_H
#define CAMERABINCAPTUREMETADATACONTROL_H

#include <qmetadatawritercontrol.h>

class CameraBinMetaData : public QMetaDataWriterControl
{
   CS_OBJECT(CameraBinMetaData)

 public:
   CameraBinMetaData(QObject *parent);
   virtual ~CameraBinMetaData() {
   }

   bool isMetaDataAvailable() const {
      return true;
   }
   bool isWritable() const {
      return true;
   }

   QVariant metaData(const QString &key) const;
   void setMetaData(const QString &key, const QVariant &value);
   QStringList availableMetaData() const;

 public:
   CS_SIGNAL_1(Public, void metaDataChanged(const QMap <QByteArray, QVariant> &un_named_arg1))
   CS_SIGNAL_2(metaDataChanged, un_named_arg1)

 private:
   QMap<QByteArray, QVariant> m_values;
};

#endif
