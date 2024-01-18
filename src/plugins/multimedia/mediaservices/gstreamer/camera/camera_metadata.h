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

   bool isMetaDataAvailable() const override {
      return true;
   }

   bool isWritable() const override {
      return true;
   }

   QVariant metaData(const QString &key) const override;
   void setMetaData(const QString &key, const QVariant &value) override;
   QStringList availableMetaData() const override;

   CS_SIGNAL_1(Public, void metaDataChanged(const QMap <QByteArray, QVariant> &metaData))
   CS_SIGNAL_2(metaDataChanged, metaData)

 private:
   QMap<QByteArray, QVariant> m_values;
};

#endif
