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

#include "fakesource.h"
#include "qpin.h"

#include <dshow.h>
#include <initguid.h>
#include <dvdmedia.h> // VIDEOINFOHEADER2

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {
        static WAVEFORMATEX g_defaultWaveFormat = {WAVE_FORMAT_PCM, 2, 44100, 176400, 4, 16, 0};
        static VIDEOINFOHEADER2 g_defaultVideoInfo = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, 0, 0, 0, 0, 0, {0}, 0, {sizeof(BITMAPINFOHEADER), 1, 1, 1, 0, 0, 0, 0, 0, 0, 0} };

        static const AM_MEDIA_TYPE g_fakeAudioType = {MEDIATYPE_Audio, MEDIASUBTYPE_PCM, 0, 0, 2, FORMAT_WaveFormatEx, 0, sizeof(WAVEFORMATEX), reinterpret_cast<BYTE*>(&g_defaultWaveFormat)};
        static const AM_MEDIA_TYPE g_fakeVideoType = {MEDIATYPE_Video, MEDIASUBTYPE_RGB32, TRUE, FALSE, 0, FORMAT_VideoInfo2, 0, sizeof(VIDEOINFOHEADER2), reinterpret_cast<BYTE*>(&g_defaultVideoInfo)};

        class FakePin : public QPin
        {
        public:
            FakePin(FakeSource *source, const AM_MEDIA_TYPE &mt) :
              QPin(source, PINDIR_OUTPUT, QVector<AM_MEDIA_TYPE>() << mt), m_source(source)
              {
                  setAvailable(true);
              }

              ~FakePin()
              {
              }


              STDMETHODIMP Disconnect() override
              {
                  HRESULT hr = QPin::Disconnect();
                  if (SUCCEEDED(hr)) {
                      setAvailable(true);
                  }
                  return hr;
              }


              STDMETHODIMP Connect(IPin *pin, const AM_MEDIA_TYPE *type) override
              {
                  HRESULT hr = QPin::Connect(pin, type);
                  if (SUCCEEDED(hr)) {
                      setAvailable(false);
                  }
                  return hr;
              }

        private:
            void setAvailable(bool avail)
            {
                if (mediaTypes().first().majortype == MEDIATYPE_Audio) {
                    if (avail) {
                        m_source->addAvailableAudioPin(this);
                    } else {
                        m_source->removeAvailableAudioPin(this);
                    }
                } else {
                    if (avail) {
                        m_source->addAvailableVideoPin(this);
                    } else {
                        m_source->removeAvailableVideoPin(this);
                    }
                }
            }

            FakeSource *m_source;


        };

        FakeSource::FakeSource() : QBaseFilter(CLSID_NULL)
        {
            createFakeAudioPin();
            createFakeVideoPin();
        }

        FakeSource::~FakeSource()
        {
        }

        void FakeSource::addAvailableAudioPin(FakePin *pin)
        {
            availableAudioPins += pin;
        }

        void FakeSource::addAvailableVideoPin(FakePin *pin)
        {
            availableVideoPins += pin;
        }

        void FakeSource::removeAvailableAudioPin(FakePin *pin)
        {
            availableAudioPins -= pin;

            if (availableAudioPins.isEmpty()) {
                createFakeAudioPin();
            }
        }

        void FakeSource::removeAvailableVideoPin(FakePin *pin)
        {
            availableVideoPins -= pin;

            if (availableVideoPins.isEmpty()) {
                createFakeVideoPin();
            }
        }

        void FakeSource::createFakeAudioPin()
        {
            new FakePin(this, g_fakeAudioType);
        }

        void FakeSource::createFakeVideoPin()
        {
            new FakePin(this, g_fakeVideoType);
        }

    }
}

QT_END_NAMESPACE
