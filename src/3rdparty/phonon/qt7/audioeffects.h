/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
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

#ifndef QT7_AUDIOEFFECTS_H
#define QT7_AUDIOEFFECTS_H

#include <QtCore/QVariant>
#include <QtCore/QHash>
#include <phonon/effectinterface.h>
#include <phonon/effectparameter.h>
#include "medianode.h"
#include "audionode.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class AudioEffectAudioNode : public AudioNode
    {
        public:
            AudioEffectAudioNode(int effectType);
            int m_effectType;

            ComponentDescription getAudioNodeDescription() const;
            void initializeAudioUnit();

            QVariant parameterValue(const Phonon::EffectParameter &value) const;
            void setParameterValue(const Phonon::EffectParameter &parameter, const QVariant &newValue);

        private:
            QHash<int, float> m_alteredParameters;
    };

///////////////////////////////////////////////////////////////////////

    class AudioEffect : public MediaNode, Phonon::EffectInterface
    {
        CS_OBJECT(AudioEffect)
        CS_INTERFACES(Phonon::EffectInterface)

        public:
            AudioEffect(int effectType, QObject *parent = 0);
            AudioEffectAudioNode *m_audioNode;

            QString name();
            QString description();
         
            // EffectInterface:
            virtual QList<Phonon::EffectParameter> parameters() const;
            virtual QVariant parameterValue(const Phonon::EffectParameter &parameter) const;
            virtual void setParameterValue(const Phonon::EffectParameter &parameter, const QVariant &newValue);

            static QList<int> effectList();

        private:
            Phonon::EffectParameter createParameter(const AudioUnit &audioUnit, const AudioUnitParameterID &id) const;
    };


}} //namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_AUDIOEFFECTS_H
