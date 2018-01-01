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

#include "audiosplitter.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{

AudioNodeSplitter::AudioNodeSplitter() : AudioNode(1, 2)
{
}

ComponentDescription AudioNodeSplitter::getAudioNodeDescription() const
{
	ComponentDescription description;
	description.componentType = kAudioUnitType_FormatConverter;
	description.componentSubType = kAudioUnitSubType_Splitter;
	description.componentManufacturer = kAudioUnitManufacturer_Apple;
	description.componentFlags = 0;
	description.componentFlagsMask = 0;
    return description;
}

AudioSplitter::AudioSplitter(QObject *parent) : MediaNode(AudioSink | AudioSource, new AudioNodeSplitter(), parent)
{
}

AudioSplitter::~AudioSplitter()
{
}

}} //namespace Phonon::QT7

QT_END_NAMESPACE
