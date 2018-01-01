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

#ifndef QT7_AUDIOPARTOUTPUT_H
#define QT7_AUDIOPARTOUTPUT_H

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include "audionode.h"
#include <QtCore/qnamespace.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class AudioPartOutput : public AudioNode
    {
        public:
            AudioPartOutput();
            virtual ~AudioPartOutput();

        private:
            ComponentDescription getAudioNodeDescription() const;
            void initializeAudioUnit(AudioNode *source);
    };

}} // namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_AUDIOPARTOUTPUT_H
