/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef DS9_VOLUMEEFFECT_H
#define DS9_VOLUMEEFFECT_H

#include "effect.h"
#include <phonon/volumefaderinterface.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VOLUMEFADEREFFECT

namespace Phonon
{
    namespace DS9
    {
        class VolumeEffectFilter;
        class VolumeEffect : public Effect, public Phonon::VolumeFaderInterface
        {
            CS_OBJECT(VolumeEffect)
                CS_INTERFACES(Phonon::VolumeFaderInterface)
        public:
            VolumeEffect(QObject *parent);

            //reimplementation
            virtual float volume() const;
            virtual void setVolume(float);
            virtual Phonon::VolumeFaderEffect::FadeCurve fadeCurve() const;
            virtual void setFadeCurve(Phonon::VolumeFaderEffect::FadeCurve);
            virtual void fadeTo(float, int);

        private:
            float m_volume;

            //paramaters used to fade
            Phonon::VolumeFaderEffect::FadeCurve m_fadeCurve;

            bool m_fading; //determines if we should be fading.
            float m_initialVolume;  
            float m_targetVolume;
            int m_fadeDuration;
            int m_fadeSamplePosition;
            qreal (*m_fadeCurveFn)(const qreal, const qreal, const qreal);

            //allow the filter to get access to that
            friend class VolumeEffectFilter;

        };
    }
}

#endif //QT_NO_PHONON_VOLUMEFADEREFFECT

QT_END_NAMESPACE

#endif
