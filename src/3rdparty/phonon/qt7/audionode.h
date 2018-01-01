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

#ifndef QT7_AudioNode_H
#define QT7_AudioNode_H

#include <QtCore/QObject>
#include "backendheader.h"
#include "audioconnection.h"
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class AudioGraph;
    class MediaNodeEvent;
    class MediaNode;

    class MediaNodeConnection{
        MediaNode *source;
        MediaNode *sink;
        int inputPort;
        int outputPort;
    };

    class AudioNode
    {
        public:
            enum ConnectionSide {Source, Sink};

            AudioNode(int maxInput, int maxOutput);
            virtual ~AudioNode();

            virtual void createAndConnectAUNodes();
            virtual void createAudioUnits();
            virtual void setGraph(AudioGraph *audioGraph);
            virtual AUNode getInputAUNode();
            virtual AUNode getOutputAUNode();
            virtual bool fillInStreamSpecification(AudioConnection *connection, ConnectionSide side);
            virtual bool setStreamSpecification(AudioConnection *connection, ConnectionSide side);
            void notify(const MediaNodeEvent *event);

            virtual void mediaNodeEvent(const MediaNodeEvent *event);
            Float64 getTimeInSamples(int timeProperty);
            
            AudioGraph *m_audioGraph;    
            AudioConnection *m_lastConnectionIn;

            int m_maxInputBusses;
            int m_maxOutputBusses;

        protected:
	        AUNode m_auNode;
            AudioUnit m_audioUnit;

            // Only the following methods needs to
            // be overridden by only_one-audio-unit nodes:
            virtual ComponentDescription getAudioNodeDescription() const;
            virtual void initializeAudioUnit();

        private:
            bool setStreamHelp(AudioConnection *c, int bus, OSType scope, bool fromSource);
    };
}} // namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_AudioNode_H
