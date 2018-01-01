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

#ifndef QT7_MediaNode_H
#define QT7_MediaNode_H

#include <QtCore/QObject>
#include "backendheader.h"
#include "medianodeevent.h"
#include "audioconnection.h"
#include "videoframe.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class AudioNode;
    class AudioGraph;
    class MediaObject;
    class AudioConnection;

    class MediaNode : public QObject
    {
        QT7_CS_OBJECT(MediaNode)

        public:
            enum NodeDescriptionEnum {
                AudioSource     = 1,
                AudioSink       = 2,
                VideoSource     = 4,
                VideoSink       = 8,
                AudioGraphNode  = 16
            };
            using NodeDescription = QFlags<NodeDescriptionEnum>;

            MediaNode(NodeDescription description, QObject *parent);
            MediaNode(NodeDescription description, AudioNode *audioPart, QObject *parent);
            virtual ~MediaNode();

            void setAudioNode(AudioNode *audioPart);
            bool connectToSink(MediaNode *sink);
            bool disconnectToSink(MediaNode *sink);
            AudioConnection *getAudioConnectionToSink(MediaNode *sink);

            void notify(const MediaNodeEvent *event, bool propagate = true);
            void sendEventToSinks(const MediaNodeEvent *event);
            virtual void mediaNodeEvent(const MediaNodeEvent *event);

            virtual void updateVideo(VideoFrame &frame);
            AudioGraph *m_audioGraph;

            AudioNode *m_audioNode;
            QList<AudioConnection *> m_audioSinkList;
            QList<AudioConnection *> m_audioSourceList;
            QList<MediaNode *> m_videoSinkList;

            int availableAudioInputBus();
            int availableAudioOutputBus();

            NodeDescription m_description;
            MediaObject *m_owningMediaObject;
    };

    Q_DECLARE_OPERATORS_FOR_FLAGS(MediaNode::NodeDescription);

}} // namespace Phonon::QT7

QT_END_NAMESPACE
#endif // Phonon_QT7_MediaNode_H
