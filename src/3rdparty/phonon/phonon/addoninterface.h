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

#ifndef PHONON_ADDONINTERFACE_H
#define PHONON_ADDONINTERFACE_H

#include "phononnamespace.h"

#include <QtCore/QList>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_MEDIACONTROLLER

namespace Phonon
{
class AddonInterface
{
    public:
        virtual ~AddonInterface() {}

        enum Interface {
            NavigationInterface   = 1,
            ChapterInterface      = 2,
            AngleInterface        = 3,
            TitleInterface        = 4,
            SubtitleInterface     = 5,
            AudioChannelInterface = 6
        };

        enum NavigationCommand {
            Menu1Button
        };
        enum ChapterCommand {
            availableChapters,
            chapter,
            setChapter
        };
        enum AngleCommand {
            availableAngles,
            angle,
            setAngle
        };
        enum TitleCommand {
            availableTitles,
            title,
            setTitle,
            autoplayTitles,
            setAutoplayTitles
        };
        enum SubtitleCommand {
            availableSubtitles,
            currentSubtitle,
            setCurrentSubtitle
        };
        enum AudioChannelCommand {
            availableAudioChannels,
            currentAudioChannel,
            setCurrentAudioChannel
        };

        virtual bool hasInterface(Interface iface) const = 0;

        virtual QVariant interfaceCall(Interface iface, int command,
                const QList<QVariant> &arguments = QList<QVariant>()) = 0;
};

} // namespace Phonon

CS_DECLARE_INTERFACE(Phonon::AddonInterface, "AddonInterface0.2.phonon.kde.org")

#endif //QT_NO_PHONON_MEDIACONTROLLER

QT_END_NAMESPACE

#endif // PHONON_ADDONINTERFACE_H
