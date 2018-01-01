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
**  This file is part of the KDE project.
********************************************************/

#include "audiopartoutput.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{

AudioPartOutput::AudioPartOutput()
    : AudioNode()
{
}

AudioPartOutput::~AudioPartOutput()
{
}

ComponentDescription AudioPartOutput::getAudioNodeDescription() const
{
	ComponentDescription description;
	description.componentType = kAudioUnitType_Output;
	description.componentSubType = kAudioUnitSubType_DefaultOutput;
	description.componentManufacturer = kAudioUnitManufacturer_Apple;
	description.componentFlags = 0;
	description.componentFlagsMask = 0;
    return description;
}

void AudioPartOutput::initializeAudioUnit(AudioNode *source)
{
    m_audioStreamDescription = source->outputStreamDescription();
    m_audioChannelLayout = source->outputChannelLayout();
    m_audioChannelLayoutSize = source->outputChannelLayoutSize();

    // Specify the stream format:
    OSStatus err;
	err = AudioUnitSetProperty(m_audioUnit,
	    kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input,
	    0, m_audioStreamDescription, sizeof(AudioStreamBasicDescription));
    BACKEND_ASSERT2(err == noErr, "Could not set stream format on audio output unit.", FATAL_ERROR)

    // Set the channel layout:
	err = AudioUnitSetProperty(m_audioUnit,
	    kAudioUnitProperty_AudioChannelLayout, kAudioUnitScope_Input,
	    0, m_audioChannelLayout, m_audioChannelLayoutSize);
    BACKEND_ASSERT2(err == noErr, "Could not set channel layout on audio output unit.", FATAL_ERROR)
}

}} // namespace Phonon::QT7

QT_END_NAMESPACE
