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

#ifndef QGSTREAMERSTREAMSCONTROL_H
#define QGSTREAMERSTREAMSCONTROL_H

#include <qmediastreamscontrol.h>

class QGstreamerPlayerSession;

class QGstreamerStreamsControl : public QMediaStreamsControl
{
   CS_OBJECT(QGstreamerStreamsControl)

 public:
   QGstreamerStreamsControl(QGstreamerPlayerSession *session, QObject *parent);
   virtual ~QGstreamerStreamsControl();

   int streamCount() override;
   StreamType streamType(int streamNumber) override;

   QVariant metaData(int streamNumber, const QString &key) override;

   bool isActive(int streamNumber) override;
   void setActive(int streamNumber, bool state) override;

 private:
   QGstreamerPlayerSession *m_session;
};

#endif

