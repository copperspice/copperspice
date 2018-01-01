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

#ifndef GSTREAMER_AUDIOEFFECT_H
#define GSTREAMER_AUDIOEFFECT_H

#include "common.h"
#include "effect.h"
#include "medianode.h"

#include <phonon/effectparameter.h>
#include <phonon/effectinterface.h>

#include <QtCore/QObject>

#include <gst/gst.h>

#ifndef QT_NO_PHONON_EFFECT

QT_BEGIN_NAMESPACE
namespace Phonon
{
namespace Gstreamer
{
    class AudioOutput;
    class EffectInfo;

    class AudioEffect : public Effect
    {
        GSTRM_CS_OBJECT(AudioEffect)

        public:
            AudioEffect (Backend *backend, int effectId, QObject *parent);

        protected:
            GstElement* createEffectBin() override;

            GstElement* audioElement() override 
               { return m_effectBin; }

            QString m_effectName;
    };

}} //namespace Phonon::Gstreamer

QT_END_NAMESPACE
#endif //QT_NO_PHONON_EFFECT

#endif // Phonon_GSTREAMER_AUDIOEFFECT_H
