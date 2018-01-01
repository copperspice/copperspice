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


#ifndef PHONON_VOLUMEFADEREFFECT_P_H
#define PHONON_VOLUMEFADEREFFECT_P_H

#include "volumefadereffect.h"
#include "effect_p.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VOLUMEFADEREFFECT

namespace Phonon
{
class VolumeFaderEffectPrivate : public EffectPrivate
{
    Q_DECLARE_PUBLIC(VolumeFaderEffect)
    PHONON_PRIVATECLASS
    protected:
        VolumeFaderEffectPrivate()
            : currentVolume(1.0)
            , fadeCurve(VolumeFaderEffect::Fade3Decibel)
        {
            // invalid EffectDescription
            // ############# parameter functions are incorrect
        }

        float currentVolume;
        VolumeFaderEffect::FadeCurve fadeCurve;
};
}

#endif //QT_NO_PHONON_VOLUMEFADEREFFECT

QT_END_NAMESPACE

#endif // PHONON_VOLUMEFADEREFFECT_P_H

