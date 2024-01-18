/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

#include <qdebug.h>
#include <qmediaplaylist.h>
#include <qsysinfo.h>
#include <qvariant.h>

#include "avfcameraservice.h"
#include "avfcameracontrol.h"
#include "avfcamerainfocontrol.h"
#include "avfcamerasession.h"
#include "avfcameradevicecontrol.h"
#include "avfaudioinputselectorcontrol.h"
#include "avfcamerametadatacontrol.h"
#include "avfmediarecordercontrol.h"
#include "avfimagecapturecontrol.h"
#include "avfcamerarenderercontrol.h"
#include "avfmediarecordercontrol.h"
#include "avfimagecapturecontrol.h"
#include "avfmediavideoprobecontrol.h"
#include "avfcamerafocuscontrol.h"
#include "avfcameraexposurecontrol.h"
#include "avfcameraviewfindersettingscontrol.h"
#include "avfimageencodercontrol.h"
#include "avfcameraflashcontrol.h"
#include "avfaudioencodersettingscontrol.h"
#include "avfvideoencodersettingscontrol.h"
#include "avfmediacontainercontrol.h"

#include <qmediaplaylistnavigator_p.h>

AVFCameraService::AVFCameraService(QObject *parent)
   : QMediaService(parent), m_videoOutput(nullptr)
{
    m_session            = new AVFCameraSession(this);
    m_cameraControl      = new AVFCameraControl(this);
    m_cameraInfoControl  = new AVFCameraInfoControl(this);
    m_videoDeviceControl = new AVFCameraDeviceControl(this);

    m_audioInputSelectorControl = new AVFAudioInputSelectorControl(this);

    m_metaDataControl = new AVFCameraMetaDataControl(this);

    // This will connect a slot to 'captureModeChanged' and will break viewfinder
    // by attaching AVCaptureMovieFileOutput in this slot.
    m_recorderControl = new AVFMediaRecorderControl(this);

    m_imageCaptureControl = new AVFImageCaptureControl(this);
    m_cameraFocusControl  = new AVFCameraFocusControl(this);

    m_cameraExposureControl = nullptr;
    m_cameraZoomControl     = nullptr;

    m_viewfinderSettingsControl2 = new AVFCameraViewfinderSettingsControl2(this);
    m_viewfinderSettingsControl = new AVFCameraViewfinderSettingsControl(this);
    m_imageEncoderControl = new AVFImageEncoderControl(this);
    m_flashControl = new AVFCameraFlashControl(this);
    m_audioEncoderSettingsControl = new AVFAudioEncoderSettingsControl(this);
    m_videoEncoderSettingsControl = new AVFVideoEncoderSettingsControl(this);
    m_mediaContainerControl = new AVFMediaContainerControl(this);
}

AVFCameraService::~AVFCameraService()
{
    m_cameraControl->setState(QCamera::UnloadedState);

    if (m_videoOutput) {
        m_session->setVideoOutput(nullptr);
        delete m_videoOutput;
        m_videoOutput = nullptr;
    }

    // delete controls before session,
    // so they have a chance to do deinitialization
    delete m_imageCaptureControl;

    // delete m_recorderControl;

    delete m_metaDataControl;
    delete m_cameraControl;
    delete m_cameraFocusControl;
    delete m_cameraExposureControl;
    delete m_viewfinderSettingsControl2;
    delete m_viewfinderSettingsControl;
    delete m_imageEncoderControl;
    delete m_flashControl;
    delete m_audioEncoderSettingsControl;
    delete m_videoEncoderSettingsControl;
    delete m_mediaContainerControl;

    delete m_session;
}

QMediaControl *AVFCameraService::requestControl(const QString &name)
{
    if (name == QCameraControl_iid)
        return m_cameraControl;

    if (name == QCameraInfoControl_iid)
        return m_cameraInfoControl;

    if (name == QVideoDeviceSelectorControl_iid)
        return m_videoDeviceControl;

    if (name == QAudioInputSelectorControl_iid)
        return m_audioInputSelectorControl;

    //metadata support is not implemented yet
    //if (name == QMetaDataWriterControl_iid)
    //    return m_metaDataControl;

    if (name == QMediaRecorderControl_iid)
        return m_recorderControl;

    if (name == QCameraImageCaptureControl_iid)
        return m_imageCaptureControl;

    if (name == QCameraExposureControl_iid)
        return m_cameraExposureControl;

    if (name == QCameraFocusControl_iid)
        return m_cameraFocusControl;

    if (name == QCameraViewfinderSettingsControl2_iid)
        return m_viewfinderSettingsControl2;

    if (name == QCameraViewfinderSettingsControl_iid)
        return m_viewfinderSettingsControl;

    if (name == QImageEncoderControl_iid)
        return m_imageEncoderControl;

    if (name == QCameraFlashControl_iid)
        return m_flashControl;

    if (name == QAudioEncoderSettingsControl_iid)
        return m_audioEncoderSettingsControl;

    if (name == QVideoEncoderSettingsControl_iid)
        return m_videoEncoderSettingsControl;

    if (name == QMediaContainerControl_iid)
        return m_mediaContainerControl;

    if (name == QMediaVideoProbeControl_iid)  {
        AVFMediaVideoProbeControl *videoProbe = nullptr;

        videoProbe = new AVFMediaVideoProbeControl(this);
        m_session->addProbe(videoProbe);

        return videoProbe;
    }

    if (! m_videoOutput) {
        if (name == QVideoRendererControl_iid)
            m_videoOutput = new AVFCameraRendererControl(this);

        if (m_videoOutput) {
            m_session->setVideoOutput(m_videoOutput);
            return m_videoOutput;
        }
    }

    return nullptr;
}

void AVFCameraService::releaseControl(QMediaControl *control)
{
    AVFMediaVideoProbeControl *videoProbe = dynamic_cast<AVFMediaVideoProbeControl *>(control);
    if (videoProbe) {
        m_session->removeProbe(videoProbe);
        delete videoProbe;

    } else if (m_videoOutput == control) {
        m_session->setVideoOutput(nullptr);
        delete m_videoOutput;
        m_videoOutput = nullptr;
    }
}
