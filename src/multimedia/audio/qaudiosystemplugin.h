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

#ifndef QAUDIOSYSTEMPLUGIN_H
#define QAUDIOSYSTEMPLUGIN_H

#include <qstring.h>
#include <qplugin.h>

#include <qmultimedia.h>
#include <qaudioformat.h>
#include <qaudiodeviceinfo.h>
#include <qaudiosystem.h>

struct Q_MULTIMEDIA_EXPORT QAudioSystemFactoryInterface {
   virtual QList<QString> availableDevices(QAudio::Mode) const = 0;

   virtual QAbstractAudioInput  *createInput(const QString  &device) = 0;
   virtual QAbstractAudioOutput *createOutput(const QString &device) = 0;
   virtual QAbstractAudioDeviceInfo *createDeviceInfo(const QString &device, QAudio::Mode mode) = 0;

   virtual ~QAudioSystemFactoryInterface();
};

#define QAudioSystemFactoryInterface_iid "com.copperspice.CS.audioSystemFactory/1.0"
CS_DECLARE_INTERFACE(QAudioSystemFactoryInterface, QAudioSystemFactoryInterface_iid)

class Q_MULTIMEDIA_EXPORT QAudioSystemPlugin : public QObject, public QAudioSystemFactoryInterface
{
   MULTI_CS_OBJECT_MULTIPLE(QAudioSystemPlugin, QObject)
   CS_INTERFACES(QAudioSystemFactoryInterface)

 public:
   explicit QAudioSystemPlugin(QObject *parent = nullptr);
   ~QAudioSystemPlugin();

   QList<QString> availableDevices(QAudio::Mode) const override = 0;

   QAbstractAudioInput *createInput(const QString  &device) override = 0;
   QAbstractAudioOutput *createOutput(const QString &device) override = 0;
   QAbstractAudioDeviceInfo *createDeviceInfo(const QString &device, QAudio::Mode mode) override = 0;

};

#endif
