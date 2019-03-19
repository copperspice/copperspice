/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QAUDIOENGINEPLUGIN_H
#define QAUDIOENGINEPLUGIN_H

#include <QtCore/qstring.h>
#include <QtCore/qplugin.h>
#include <QtCore/qfactoryinterface.h>
#include <QtMultimedia/qaudioformat.h>
#include <QtMultimedia/qaudiodeviceinfo.h>
#include <QtMultimedia/qaudioengine.h>

QT_BEGIN_NAMESPACE

struct Q_MULTIMEDIA_EXPORT QAudioEngineFactoryInterface : public QFactoryInterface {
   virtual QList<QString > availableDevices(QAudio::Mode) const = 0;
   virtual QAbstractAudioInput *createInput(const QString  &device, const QAudioFormat &format = QAudioFormat()) = 0;
   virtual QAbstractAudioOutput *createOutput(const QString  &device, const QAudioFormat &format = QAudioFormat()) = 0;
   virtual QAbstractAudioDeviceInfo *createDeviceInfo(const QString  &device, QAudio::Mode mode) = 0;
};

#define QAudioEngineFactoryInterface_iid  "com.copperspice.QAudioEngineFactoryInterface"
CS_DECLARE_INTERFACE(QAudioEngineFactoryInterface, QAudioEngineFactoryInterface_iid)

class Q_MULTIMEDIA_EXPORT QAudioEnginePlugin : public QObject, public QAudioEngineFactoryInterface
{
   MULTI_CS_OBJECT(QAudioEnginePlugin)
   CS_INTERFACES(QAudioEngineFactoryInterface, QFactoryInterface)

 public:
   QAudioEnginePlugin(QObject *parent = nullptr);
   ~QAudioEnginePlugin();
};

QT_END_NAMESPACE

#endif
