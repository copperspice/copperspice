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

#ifndef QAUDIOOUTPUTSELECTORCONTROL_H
#define QAUDIOOUTPUTSELECTORCONTROL_H

#include <qstring.h>
#include <qaudio.h>
#include <qmediacontrol.h>

class Q_MULTIMEDIA_EXPORT QAudioOutputSelectorControl : public QMediaControl
{
   MULTI_CS_OBJECT(QAudioOutputSelectorControl)

 public:
   virtual ~QAudioOutputSelectorControl();

   virtual QList<QString> availableOutputs() const = 0;
   virtual QString outputDescription(const QString &name) const = 0;
   virtual QString defaultOutput() const = 0;
   virtual QString activeOutput() const = 0;

   MULTI_CS_SLOT_1(Public, virtual void setActiveOutput(const QString &name) = 0)
   MULTI_CS_SLOT_2(setActiveOutput)

   MULTI_CS_SIGNAL_1(Public, void activeOutputChanged(const QString &name))
   MULTI_CS_SIGNAL_2(activeOutputChanged, name)

   MULTI_CS_SIGNAL_1(Public, void availableOutputsChanged())
   MULTI_CS_SIGNAL_2(availableOutputsChanged)

 protected:
   explicit QAudioOutputSelectorControl(QObject *parent = nullptr);
};

#define QAudioOutputSelectorControl_iid "com.copperspice.CS.audioOutputSelectorControl/1.0"
CS_DECLARE_INTERFACE(QAudioOutputSelectorControl, QAudioOutputSelectorControl_iid)

#endif
