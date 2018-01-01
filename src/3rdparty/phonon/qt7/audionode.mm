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

#include "audionode.h"
#include "audiograph.h"
#include "audioconnection.h"
#include "medianode.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{

AudioNode::AudioNode(int maxInputBusses, int maxOutputBusses)
{
    m_auNode = 0;
    m_audioUnit = 0;
    m_audioGraph = 0;
    m_maxInputBusses = maxInputBusses;
    m_maxOutputBusses = maxOutputBusses;
    m_lastConnectionIn = 0;
}

AudioNode::~AudioNode()
{
    setGraph(0);
}

void AudioNode::setGraph(AudioGraph *audioGraph)
{
    if (m_audioGraph == audioGraph)
        return;

    DEBUG_AUDIO_GRAPH("AudioNode" << int(this) << "is setting graph:" << int(audioGraph))    
    if (m_auNode){
        AUGraphRemoveNode(m_audioGraph->audioGraphRef(), m_auNode);
        m_auNode = 0;
    }
    
    m_audioUnit = 0;
    m_lastConnectionIn = 0;
    m_audioGraph = audioGraph;
}

void AudioNode::createAndConnectAUNodes()
{
    if (m_auNode)
        return;

    ComponentDescription description = getAudioNodeDescription();
    DEBUG_AUDIO_GRAPH("AudioNode" << int(this) << "creates AUNode" 
        << QString(!FindNextComponent(0, &description) ? "ERROR: COMPONENT NOT FOUND!" : "OK!"))

    OSStatus err = noErr;

    // The proper function to call here is AUGraphAddNode() but the type has
    // changed between 10.5 and 10.6. it's still OK to call this function, but
    // if we want to use the proper thing we need to move over to
    // AudioComponentDescription everywhere, which is very similar to the
    // ComponentDescription, but a different size.  however,
    // AudioComponentDescription only exists on 10.6+. More fun than we need to
    // deal with at the moment, so we'll take the "deprecated" warning instead.
    err = AUGraphNewNode(m_audioGraph->audioGraphRef(), &description, 0, 0, &m_auNode);
        
    BACKEND_ASSERT2(err != kAUGraphErr_OutputNodeErr, "A MediaObject can only be connected to one audio output device.", FATAL_ERROR)
    BACKEND_ASSERT2(err == noErr, "Could not create new AUNode.", FATAL_ERROR)
}

AUNode AudioNode::getInputAUNode()
{
    return m_auNode;
}

AUNode AudioNode::getOutputAUNode()
{
    return m_auNode;
}

void AudioNode::createAudioUnits()
{
    if (m_audioUnit)
        return;

    DEBUG_AUDIO_GRAPH("AudioNode" << int(this) << "creates AudioUnit")
    OSStatus err = AUGraphGetNodeInfo(m_audioGraph->audioGraphRef(), m_auNode, 0, 0, 0, &m_audioUnit);
    BACKEND_ASSERT2(err == noErr, "Could not get audio unit from audio node.", FATAL_ERROR)
    initializeAudioUnit();
}

ComponentDescription AudioNode::getAudioNodeDescription() const
{
    // Override if needed.
    ComponentDescription cd;
    Q_UNUSED(cd);
    return cd;
}

bool AudioNode::setStreamHelp(AudioConnection *c, int bus, OSType scope, bool fromSource)
{
    if (fromSource){
	    OSStatus err = AudioUnitSetProperty(m_audioUnit, kAudioUnitProperty_StreamFormat, scope,
	        bus, &c->m_sourceStreamDescription, sizeof(AudioStreamBasicDescription));
        if (err != noErr){
            DEBUG_AUDIO_STREAM("AudioNode" << int(this) << " - failed setting stream format")
            return false;
        }
	    AudioUnitSetProperty(m_audioUnit, kAudioUnitProperty_AudioChannelLayout, scope,
	    bus, c->m_sourceChannelLayout, c->m_sourceChannelLayoutSize);
    } else {
	    OSStatus err = AudioUnitSetProperty(m_audioUnit, kAudioUnitProperty_StreamFormat, scope,
	        bus, &c->m_sinkStreamDescription, sizeof(AudioStreamBasicDescription));
        if (err != noErr){
            DEBUG_AUDIO_STREAM("AudioNode" << int(this) << " - failed setting stream format")
            return false;
        }
	    AudioUnitSetProperty(m_audioUnit, kAudioUnitProperty_AudioChannelLayout, scope,
	    bus, c->m_sinkChannelLayout, c->m_sourceChannelLayoutSize);
    }
    return true;
}

bool AudioNode::setStreamSpecification(AudioConnection *connection, ConnectionSide side)
{
    if (side == Source){
        // This object am source of connection:
        if (connection->m_hasSourceSpecification){
            DEBUG_AUDIO_STREAM("AudioNode" << int(this) << "sets stream specification out"
                << connection->m_sourceOutputBus << "from connection source")
            return setStreamHelp(connection, connection->m_sourceOutputBus, kAudioUnitScope_Output, true);
        } else {
            DEBUG_AUDIO_STREAM("AudioNode" << int(this) << "did not set stream specification out")
        }
    } else {
        if (connection->m_hasSinkSpecification){
            DEBUG_AUDIO_STREAM("AudioNode" << int(this) << "sets stream specification"
                << connection->m_sinkInputBus << "from connection sink")
            return setStreamHelp(connection, connection->m_sinkInputBus, kAudioUnitScope_Input, false);
        } else if (connection->m_hasSourceSpecification){
            DEBUG_AUDIO_STREAM("AudioNode" << int(this) << "sets stream specification"
            << connection->m_sinkInputBus << "from connection source")
            return setStreamHelp(connection, connection->m_sinkInputBus, kAudioUnitScope_Input, true);
        } else {
            DEBUG_AUDIO_STREAM("AudioNode" << int(this) << "did not set stream specification in")
        }
    }
    return true;
}

bool AudioNode::fillInStreamSpecification(AudioConnection *connection, ConnectionSide side)
{
    if (side == Source){
        // As default, use the last description to describe the source:
        if (m_lastConnectionIn->m_hasSinkSpecification){
            DEBUG_AUDIO_STREAM("AudioNode" << int(this) << "is source, and fills in stream spec using last connection sink.")
            connection->m_sourceStreamDescription = m_lastConnectionIn->m_sinkStreamDescription;
            connection->m_sourceChannelLayout = (AudioChannelLayout *) malloc(m_lastConnectionIn->m_sinkChannelLayoutSize);
            memcpy(connection->m_sourceChannelLayout, m_lastConnectionIn->m_sinkChannelLayout, m_lastConnectionIn->m_sinkChannelLayoutSize);
            connection->m_sourceChannelLayoutSize = m_lastConnectionIn->m_sinkChannelLayoutSize;
            connection->m_hasSourceSpecification = true;
        } else if (m_lastConnectionIn->m_hasSourceSpecification){
            DEBUG_AUDIO_STREAM("AudioNode" << int(this) << "is source, and fills in stream spec using last connection source.")
            connection->m_sourceStreamDescription = m_lastConnectionIn->m_sourceStreamDescription;
            connection->m_sourceChannelLayout = (AudioChannelLayout *) malloc(m_lastConnectionIn->m_sourceChannelLayoutSize);
            memcpy(connection->m_sourceChannelLayout, m_lastConnectionIn->m_sourceChannelLayout, m_lastConnectionIn->m_sourceChannelLayoutSize);
            connection->m_sourceChannelLayoutSize = m_lastConnectionIn->m_sourceChannelLayoutSize;
            connection->m_hasSourceSpecification = true;
        } else {
            DEBUG_AUDIO_STREAM("AudioNode" << int(this) << " __WARNING__: could not get stream specification...")
        }
    } else {
        DEBUG_AUDIO_STREAM("AudioNode" << int(this) << "is sink, skips filling in stream.")
        if (!connection->isSinkOnly())
            m_lastConnectionIn = connection;
    }
    return true;
}

/**
    Let timeProperty be one of e.g
    {kAudioUnitProperty_Latency, kAudioUnitProperty_TailTime,
    kAudioOutputUnitProperty_StartTime, kAudioUnitProperty_CurrentPlayTime}
*/
Float64 AudioNode::getTimeInSamples(int timeProperty)
{
    if (!m_audioUnit)
        return 0;

    AudioTimeStamp timeStamp;
    UInt32 size = sizeof(timeStamp);
    memset(&timeStamp, 0, sizeof(timeStamp));
	OSStatus err = AudioUnitGetProperty(m_audioUnit,
        timeProperty, kAudioUnitScope_Global,
        0, &timeStamp, &size);
    if (err != noErr)
        return 0;
    return timeStamp.mSampleTime;
}

void AudioNode::notify(const MediaNodeEvent *event)
{
    switch(event->type()){
    case MediaNodeEvent::AudioGraphAboutToBeDeleted:
        setGraph(0);
        break;
    case MediaNodeEvent::NewAudioGraph:
        setGraph(static_cast<AudioGraph *>(event->data()));
        break;
    default:
        break;
    }

    mediaNodeEvent(event);
}

void AudioNode::mediaNodeEvent(const MediaNodeEvent */*event*/)
{
    // Override if needed
}

void AudioNode::initializeAudioUnit()
{
    // Override if needed.
}

}} //namespace Phonon::QT7

QT_END_NAMESPACE
