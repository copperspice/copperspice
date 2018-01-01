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

/**************************************************************
** Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
** Copyright (C) 2009 Martin Sandsmark <sandsmark@samfundet.no>
**************************************************************/

#ifndef GSTREAMER_AUDIODATAOUTPUT_H
#define GSTREAMER_AUDIODATAOUTPUT_H

#include "abstractaudiooutput.h"
#include "backend.h"
#include "medianode.h"
#include <phonon/audiodataoutput.h>
#include <phonon/audiodataoutputinterface.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{
    /**
     * \author Martin Sandsmark <sandsmark@samfundet.no>
     */
    class AudioDataOutput : public QObject,
                            public AudioDataOutputInterface,
                            public MediaNode
    {
        GSTRM_CS_OBJECT(AudioDataOutput)

        CS_INTERFACES(Phonon::AudioDataOutputInterface, Phonon::Gstreamer::MediaNode)       

        public:
            AudioDataOutput(Backend *, QObject *);
            ~AudioDataOutput();
      
            GSTRM_CS_SLOT_1(Public, int dataSize() const)
            GSTRM_CS_SLOT_2(dataSize) 

            GSTRM_CS_SLOT_1(Public, int sampleRate() const)
            GSTRM_CS_SLOT_2(sampleRate) 

            GSTRM_CS_SLOT_1(Public, void setDataSize(int size))
            GSTRM_CS_SLOT_2(setDataSize) 
        
            /// callback function for handling new audio data
            static void processBuffer(GstPad*, GstBuffer*, gpointer);

            Phonon::AudioDataOutput* frontendObject() const override 
                  {  return m_frontend; }

            void setFrontendObject(Phonon::AudioDataOutput *frontend) override
                  { m_frontend = frontend; }

            GstElement *audioElement() override
                  { return m_queue; }

            void mediaNodeEvent(const MediaNodeEvent *event) override;
        
            GSTRM_CS_SIGNAL_1(Public, void dataReady(const QMap <Phonon::AudioDataOutput::Channel,QVector <qint16>> & data))
            GSTRM_CS_SIGNAL_OVERLOAD(dataReady,(const QMap <Phonon::AudioDataOutput::Channel,QVector <qint16>> &), data) 

            GSTRM_CS_SIGNAL_1(Public, void dataReady(const QMap <Phonon::AudioDataOutput::Channel,QVector <float>> & data))
            GSTRM_CS_SIGNAL_OVERLOAD(dataReady,(const QMap <Phonon::AudioDataOutput::Channel,QVector <float>> &), data) 

            GSTRM_CS_SIGNAL_1(Public, void endOfMedia(int remainingSamples))
            GSTRM_CS_SIGNAL_2(endOfMedia,remainingSamples) 

        private:
            void convertAndEmit(const QVector<qint16>&, const QVector<qint16>&);

            GstElement *m_queue;
            int m_dataSize;
            QVector<qint16> m_pendingData;
            Phonon::AudioDataOutput *m_frontend;
            int m_channels;
    };
}} //namespace Phonon::Gstreamer

QT_END_NAMESPACE

#endif
