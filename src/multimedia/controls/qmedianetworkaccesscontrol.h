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

#ifndef QMEDIANETWORKACCESSCONTROL_H
#define QMEDIANETWORKACCESSCONTROL_H

#include <qmediacontrol.h>

#include <qlist.h>
#include <qstring.h>
#include <qnetworkconfiguration.h>

class Q_MULTIMEDIA_EXPORT QMediaNetworkAccessControl : public QMediaControl
{
   MULTI_CS_OBJECT(QMediaNetworkAccessControl)

 public:
   virtual ~QMediaNetworkAccessControl();

   virtual void setConfigurations(const QList<QNetworkConfiguration> &configurations) = 0;
   virtual QNetworkConfiguration currentConfiguration() const = 0;

   MULTI_CS_SIGNAL_1(Public, void configurationChanged(const QNetworkConfiguration &configuration))
   MULTI_CS_SIGNAL_2(configurationChanged, configuration)

 protected:
   explicit QMediaNetworkAccessControl(QObject *parent = nullptr);
};

#define QMediaNetworkAccessControl_iid "com.copperspice.CS.mediaNetworkAccessControl/1.0"
CS_DECLARE_INTERFACE(QMediaNetworkAccessControl, QMediaNetworkAccessControl_iid)

#endif
