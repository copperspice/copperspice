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

#ifndef AVFCAMERAMETADATACONTROL_H
#define AVFCAMERAMETADATACONTROL_H

#include <qmetadatawritercontrol.h>


class AVFCameraService;

class AVFCameraMetaDataControl : public QMetaDataWriterControl
{
   CS_OBJECT(AVFCameraMetaDataControl)

 public:
   AVFCameraMetaDataControl(AVFCameraService *service, QObject *parent = nullptr);
   virtual ~AVFCameraMetaDataControl();

   bool isMetaDataAvailable() const override;
   bool isWritable() const override;

   QVariant metaData(const QString &key) const override;
   void setMetaData(const QString &key, const QVariant &value) override;
   QStringList availableMetaData() const override;

 private:
   QMap<QString, QVariant> m_tags;
};

#endif
