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

#ifndef QAUDIORECORDER_H
#define QAUDIORECORDER_H

#include <qstring.h>
#include <qpair.h>
#include <qmediarecorder.h>
#include <qmediaobject.h>
#include <qmediaencodersettings.h>

class QSize;
class QAudioFormat;
class QAudioRecorderPrivate;

class Q_MULTIMEDIA_EXPORT QAudioRecorder : public QMediaRecorder
{
   MULTI_CS_OBJECT(QAudioRecorder)

   MULTI_CS_PROPERTY_READ(audioInput, audioInput)
   MULTI_CS_PROPERTY_WRITE(audioInput, setAudioInput)
   MULTI_CS_PROPERTY_NOTIFY(audioInput, audioInputChanged)

 public:
   explicit QAudioRecorder(QObject *parent = nullptr);

   QAudioRecorder(const QAudioRecorder &) = delete;
   QAudioRecorder &operator=(const QAudioRecorder &) = delete;

   ~QAudioRecorder();

   QStringList audioInputs() const;
   QString defaultAudioInput() const;
   QString audioInputDescription(const QString &name) const;

   QString audioInput() const;

   MULTI_CS_SLOT_1(Public, void setAudioInput(const QString &name))
   MULTI_CS_SLOT_2(setAudioInput)

   MULTI_CS_SIGNAL_1(Public, void audioInputChanged(const QString &name))
   MULTI_CS_SIGNAL_2(audioInputChanged, name)

   MULTI_CS_SIGNAL_1(Public, void availableAudioInputsChanged())
   MULTI_CS_SIGNAL_2(availableAudioInputsChanged)

 private:
   Q_DECLARE_PRIVATE(QAudioRecorder)
};

#endif
