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

#ifndef QRADIODATACONTROL_H
#define QRADIODATACONTROL_H

#include <qstring.h>
#include <qmediacontrol.h>
#include <qradiodata.h>

class Q_MULTIMEDIA_EXPORT QRadioDataControl : public QMediaControl
{
    MULTI_CS_OBJECT(QRadioDataControl)

public:
    ~QRadioDataControl();

    virtual QString stationId() const = 0;
    virtual QRadioData::ProgramType programType() const = 0;
    virtual QString programTypeName() const = 0;
    virtual QString stationName() const = 0;
    virtual QString radioText() const = 0;
    virtual void setAlternativeFrequenciesEnabled(bool enabled) = 0;
    virtual bool isAlternativeFrequenciesEnabled() const = 0;

    virtual QRadioData::Error error() const = 0;
    virtual QString errorString() const = 0;

    MULTI_CS_SIGNAL_1(Public, void stationIdChanged(QString stationId))
    MULTI_CS_SIGNAL_2(stationIdChanged,stationId)

    MULTI_CS_SIGNAL_1(Public, void programTypeChanged(QRadioData::ProgramType programType))
    MULTI_CS_SIGNAL_2(programTypeChanged,programType)

    MULTI_CS_SIGNAL_1(Public, void programTypeNameChanged(QString programTypeName))
    MULTI_CS_SIGNAL_2(programTypeNameChanged,programTypeName)

    MULTI_CS_SIGNAL_1(Public, void stationNameChanged(QString stationName))
    MULTI_CS_SIGNAL_2(stationNameChanged,stationName)

    MULTI_CS_SIGNAL_1(Public, void radioTextChanged(QString radioText))
    MULTI_CS_SIGNAL_2(radioTextChanged,radioText)

    MULTI_CS_SIGNAL_1(Public, void alternativeFrequenciesEnabledChanged(bool enabled))
    MULTI_CS_SIGNAL_2(alternativeFrequenciesEnabledChanged,enabled)

    MULTI_CS_SIGNAL_1(Public, void error(QRadioData::Error error))
    MULTI_CS_SIGNAL_OVERLOAD(error, (QRadioData::Error), error)

protected:
    explicit QRadioDataControl(QObject *parent = nullptr);
};

#define QRadioDataControl_iid "com.copperspice.CS.radioDataControl/1.0"
CS_DECLARE_INTERFACE(QRadioDataControl, QRadioDataControl_iid)

#endif
