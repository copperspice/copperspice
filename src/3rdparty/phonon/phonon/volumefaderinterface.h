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

#ifndef PHONON_VOLUMEFADERINTERFACE_H
#define PHONON_VOLUMEFADERINTERFACE_H

#include "volumefadereffect.h"
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VOLUMEFADEREFFECT

namespace Phonon
{
class VolumeFaderInterface
{
    public:
        virtual ~VolumeFaderInterface() {}

        virtual float volume() const { return 1.0; }
        virtual void setVolume(float) {}
        virtual Phonon::VolumeFaderEffect::FadeCurve fadeCurve() const {
            return VolumeFaderEffect::Fade3Decibel;
        }
        virtual void setFadeCurve(Phonon::VolumeFaderEffect::FadeCurve) {}
        virtual void fadeTo(float, int) {}
};
}

CS_DECLARE_INTERFACE(Phonon::VolumeFaderInterface, "VolumeFaderInterface4.phonon.kde.org")

#endif //QT_NO_PHONON_VOLUMEFADEREFFECT

QT_END_NAMESPACE

#endif // PHONON_VOLUMEFADERINTERFACE_H
