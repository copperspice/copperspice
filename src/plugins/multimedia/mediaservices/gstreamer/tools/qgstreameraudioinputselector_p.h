/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#ifndef QGSTREAMERAUDIOINPUTSELECTOR_H
#define QGSTREAMERAUDIOINPUTSELECTOR_H

#include <qaudioinputselectorcontrol.h>
#include <qstringlist.h>

class QGstreamerAudioInputSelector : public QAudioInputSelectorControl
{
   CS_OBJECT(QGstreamerAudioInputSelector)

 public:
    QGstreamerAudioInputSelector(QObject *parent);
    ~QGstreamerAudioInputSelector();

    QList<QString> availableInputs() const;
    QString inputDescription(const QString& name) const;
    QString defaultInput() const;
    QString activeInput() const;

    CS_SLOT_1(Public, void setActiveInput(const QString& name))
    CS_SLOT_2(setActiveInput)

 private:
    void update();
    void updateAlsaDevices();
    void updateOssDevices();
    void updatePulseDevices();

    QString m_audioInput;
    QList<QString> m_names;
    QList<QString> m_descriptions;
};

#endif