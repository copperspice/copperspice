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

#ifndef QAUDIOINPUTSELECTORCONTROL_H
#define QAUDIOINPUTSELECTORCONTROL_H

#include <qstring.h>
#include <qaudio.h>
#include <qmediacontrol.h>

class Q_MULTIMEDIA_EXPORT QAudioInputSelectorControl : public QMediaControl
{
   MULTI_CS_OBJECT(QAudioInputSelectorControl)

 public:
   virtual ~QAudioInputSelectorControl();

   virtual QList<QString> availableInputs() const = 0;
   virtual QString inputDescription(const QString &name) const = 0;
   virtual QString defaultInput() const = 0;
   virtual QString activeInput() const = 0;

   MULTI_CS_SLOT_1(Public, virtual void setActiveInput(const QString &name) = 0)
   MULTI_CS_SLOT_2(setActiveInput)

   MULTI_CS_SIGNAL_1(Public, void activeInputChanged(const QString &name))
   MULTI_CS_SIGNAL_2(activeInputChanged, name)

   MULTI_CS_SIGNAL_1(Public, void availableInputsChanged())
   MULTI_CS_SIGNAL_2(availableInputsChanged)

 protected:
   explicit QAudioInputSelectorControl(QObject *parent = nullptr);
};

#define QAudioInputSelectorControl_iid "com.copperspice.CS.audioInputSelectorControl/1.0"
CS_DECLARE_INTERFACE(QAudioInputSelectorControl, QAudioInputSelectorControl_iid)

#endif
