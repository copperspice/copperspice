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

#ifndef QT7_AUDIOMIXER_H
#define QT7_AUDIOMIXER_H

#include <QtCore/QObject>
#include <QtCore/QTime>
#include <qcoreevent.h>
#include <phonon/effectinterface.h>
#include <phonon/effectparameter.h>
#include <phonon/volumefaderinterface.h>
#include "medianode.h"
#include "audionode.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class AudioMixerAudioNode : public AudioNode
    {
        public:
            AudioMixerAudioNode();
            void setVolume(float volume, int bus = 0);
            float volume(int bus = 0);

        protected:
            ComponentDescription getAudioNodeDescription() const;
            void initializeAudioUnit();

        private:
            friend class AudioMixer;
            int m_numberOfBusses;
            float m_volume;
    };

    class AudioMixer : public MediaNode, Phonon::EffectInterface, Phonon::VolumeFaderInterface
    {
        QT7_CS_OBJECT(AudioMixer)

        CS_INTERFACES(Phonon::EffectInterface, Phonon::VolumeFaderInterface)
  
        public:
            AudioMixer(QObject *parent = nullptr);
            ~AudioMixer();
            AudioMixerAudioNode *m_audioNode;
            Phonon::VolumeFaderEffect::FadeCurve m_fadeCurve;

            int m_fadeTimer;
            int m_fadeDuration;
            float m_fadeToVolume;
            float m_fadeFromVolume;
            QTime m_fadeStartTime;

            // EffectInterface:
            QList<Phonon::EffectParameter> parameters() const override;
            QVariant parameterValue(const Phonon::EffectParameter &parameter) const override;
            void setParameterValue(const Phonon::EffectParameter &parameter, const QVariant &newValue) override;

            // VolumeFaderInterface:
            float volume() const override;
            void setVolume(float volume) override;
            Phonon::VolumeFaderEffect::FadeCurve fadeCurve() const override;
            void setFadeCurve(Phonon::VolumeFaderEffect::FadeCurve fadeCurve) override;
            void fadeTo(float volume, int fadeTime) override;
            void updateFade();

        protected:
            bool event(QEvent *event) override;
    };

}} //namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_AUDIOMIXER_H
