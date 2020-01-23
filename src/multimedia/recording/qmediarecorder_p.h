/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QMEDIARECORDER_P_H
#define QMEDIARECORDER_P_H

#include <qmediarecorder.h>
#include <qmediaobject_p.h>
#include <qurl.h>

class QMediaRecorderControl;
class QMediaContainerControl;
class QAudioEncoderSettingsControl;
class QVideoEncoderSettingsControl;
class QMetaDataWriterControl;
class QMediaAvailabilityControl;
class QTimer;

class QMediaRecorderPrivate
{
   Q_DECLARE_NON_CONST_PUBLIC(QMediaRecorder)

 public:
   QMediaRecorderPrivate();
   virtual ~QMediaRecorderPrivate() {}

   void applySettingsLater();
   void restartCamera();

   QMediaObject *mediaObject;

   QMediaRecorderControl *control;
   QMediaContainerControl *formatControl;
   QAudioEncoderSettingsControl *audioControl;
   QVideoEncoderSettingsControl *videoControl;
   QMetaDataWriterControl *metaDataControl;
   QMediaAvailabilityControl *availabilityControl;

   bool settingsChanged;

   QTimer *notifyTimer;

   QMediaRecorder::State state;
   QMediaRecorder::Error error;
   QString errorString;
   QUrl actualLocation;

   void _q_stateChanged(QMediaRecorder::State state);
   void _q_error(int error, const QString &errorString);
   void _q_serviceDestroyed();
   void _q_updateActualLocation(const QUrl &);
   void _q_notify();
   void _q_updateNotifyInterval(int ms);
   void _q_applySettings();
   void _q_availabilityChanged(QMultimedia::AvailabilityStatus availability);

   QMediaRecorder *q_ptr;
};

#endif

