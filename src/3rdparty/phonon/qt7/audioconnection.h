/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QT7_AudioConnection_H
#define QT7_AudioConnection_H

#include <QtCore/QObject>
#include "backendheader.h"

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class MediaNode;
    class AudioNode;
    class AudioGraph;

    class AudioConnection {
    public:
        AudioConnection();
        AudioConnection(MediaNode *sink);
        AudioConnection(MediaNode *source, int output, MediaNode *sink, int input);

        AudioConnection(AudioNode *sink);
        AudioConnection(AudioNode *source, int output, AudioNode *sink, int input);

        ~AudioConnection();

        bool connect(AudioGraph *graph);
        bool disconnect(AudioGraph *graph);

        bool updateStreamSpecification();
        bool isBetween(MediaNode *source, MediaNode *sink);
        bool isValid();
        bool isSinkOnly();
        void freeMemoryAllocations();
        void invalidate();
        
        MediaNode *m_source;
        AudioNode *m_sourceAudioNode;
        int m_sourceOutputBus;

        MediaNode *m_sink;
        AudioNode *m_sinkAudioNode;
        int m_sinkInputBus;

        AudioChannelLayout *m_sourceChannelLayout;
    	UInt32 m_sourceChannelLayoutSize;

        AudioChannelLayout *m_sinkChannelLayout;
    	UInt32 m_sinkChannelLayoutSize;

        AudioStreamBasicDescription m_sourceStreamDescription;
        AudioStreamBasicDescription m_sinkStreamDescription;

        bool m_hasSourceSpecification;
        bool m_hasSinkSpecification;
        bool m_connected;
    };

}} // namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_AudioConnection_H
