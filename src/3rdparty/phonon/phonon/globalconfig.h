/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#ifndef PHONON_GLOBALCONFIG_H
#define PHONON_GLOBALCONFIG_H

#include "phonon_export.h"
#include "phononnamespace.h"
#include "phonondefs.h"
#include "qlist.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
    class GlobalConfigPrivate;

    class PHONON_EXPORT GlobalConfig
    {
        K_DECLARE_PRIVATE(GlobalConfig)

    public:
        GlobalConfig();
        virtual ~GlobalConfig();

        enum DevicesToHideFlag {
            ShowUnavailableDevices = 0,
            ShowAdvancedDevices = 0,
            HideAdvancedDevices = 1,
            AdvancedDevicesFromSettings = 2,
            HideUnavailableDevices = 4
        };

        bool hideAdvancedDevices() const;
        void setHideAdvancedDevices(bool hide = true);
        void setAudioOutputDeviceListFor(Phonon::Category category, QList<int> order);
        QList<int> audioOutputDeviceListFor(Phonon::Category category, int override = AdvancedDevicesFromSettings) const;
        int audioOutputDeviceFor(Phonon::Category category, int override = AdvancedDevicesFromSettings) const;

#ifndef QT_NO_PHONON_AUDIOCAPTURE
        void setAudioCaptureDeviceListFor(Phonon::Category category, QList<int> order);
        QList<int> audioCaptureDeviceListFor(Phonon::Category category, int override = AdvancedDevicesFromSettings) const;
        int audioCaptureDeviceFor(Phonon::Category category, int override = AdvancedDevicesFromSettings) const;
#endif

    protected:
        GlobalConfigPrivate *const k_ptr;
    };
}

QT_END_NAMESPACE

#endif // PHONON_GLOBALCONFIG_H
