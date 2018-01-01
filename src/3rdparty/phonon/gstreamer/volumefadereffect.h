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

#ifndef GSTREAMER_VOLUMEFADEREFFECT_H
#define GSTREAMER_VOLUMEFADEREFFECT_H

#include "medianode.h"
#include "effect.h"
#include <phonon/effectinterface.h>
#include <phonon/effectparameter.h>
#include <phonon/volumefaderinterface.h>
#include <QtCore>
#include <gst/gst.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VOLUMEFADEREFFECT
namespace Phonon
{
namespace Gstreamer
{
    class VolumeFaderEffect : public Effect, public VolumeFaderInterface
    {
        GSTRM_CS_OBJECT(VolumeFaderEffect)
        CS_INTERFACES(Phonon::VolumeFaderInterface)

        public:
            VolumeFaderEffect(Backend *backend, QObject *parent = nullptr);
            ~VolumeFaderEffect();

            GstElement* createEffectBin() override;
            GstElement *audioElement() override { return m_effectBin; }
            bool event(QEvent *) override;
            void updateFade();

            // VolumeFaderInterface:
            float volume() const override;
            void setVolume(float volume) override;
            Phonon::VolumeFaderEffect::FadeCurve fadeCurve() const override;
            void setFadeCurve(Phonon::VolumeFaderEffect::FadeCurve fadeCurve) override;
            void fadeTo(float volume, int fadeTime) override;

            Phonon::VolumeFaderEffect::FadeCurve m_fadeCurve;
            int m_fadeTimer;
            int m_fadeDuration;
            float m_fadeFromVolume;
            float m_fadeToVolume;
            QTime m_fadeStartTime;
    };
}} //namespace Phonon::Gstreamer
#endif //QT_NO_PHONON_VOLUMEFADEREFFECT
QT_END_NAMESPACE

#endif // Phonon_GSTREAMER_VOLUMEFADEREFFECT_H
