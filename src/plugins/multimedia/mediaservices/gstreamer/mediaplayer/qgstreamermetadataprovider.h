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

#ifndef QGSTREAMERMETADATAPROVIDER_H
#define QGSTREAMERMETADATAPROVIDER_H

#include <qmetadatareadercontrol.h>

class QGstreamerPlayerSession;

class QGstreamerMetaDataProvider : public QMetaDataReaderControl
{
   CS_OBJECT(QGstreamerMetaDataProvider)

 public:
   QGstreamerMetaDataProvider(QGstreamerPlayerSession *session, QObject *parent);
   virtual ~QGstreamerMetaDataProvider();

   bool isMetaDataAvailable() const override;
   bool isWritable() const;

   QVariant metaData(const QString &key) const override;
   QStringList availableMetaData() const override;

 private:
   QGstreamerPlayerSession *m_session;
   QVariantMap m_tags;

   CS_SLOT_1(Private, void updateTags())
   CS_SLOT_2(updateTags)
};

#endif
