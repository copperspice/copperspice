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

#ifndef QT7_AUDIOGRAPH_H
#define QT7_AUDIOGRAPH_H

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include <QtCore/qnamespace.h>
#include "audioconnection.h"
#include "medianode.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class AudioGraph : public MediaNode
    {
        public:
            AudioGraph(MediaNode *root);
            virtual ~AudioGraph();
            AUGraph audioGraphRef();
            void startAllOverFromScratch();

            bool openAndInit();
            void start();
            void tryStartGraph();
            void stop();
            void prepare();
            void rebuildGraph();
            void update();
            bool isRunning();
            void setPaused(bool pause);
            bool graphCannotPlay();
            void rebuildGraphIfNeeded();
            void updateStreamSpecifications();
            void setStatusCannotPlay();
            MediaNode *root();
            void notify(const MediaNodeEvent *event, bool propagate = true);

        private:
            friend class MediaNode;
            friend class AudioNode;

            void deleteGraph();
            bool updateStreamSpecificationRecursive(AudioConnection *connection);
            void createAndConnectAuNodesRecursive(AudioConnection *connection);
            bool createAudioUnitsRecursive(AudioConnection *connection);

            void connectLate(AudioConnection *connection);
            void disconnectLate(AudioConnection *connection);

            int nodeCount();

            bool m_initialized;
            bool m_startedLogically;
            bool m_rebuildLater;
            bool m_graphCannotPlay;
            bool m_paused;

            AUGraph m_audioGraphRef;
            MediaNode *m_root;
    };

}} // namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_AUDIOGRAPH_H
