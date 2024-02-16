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

#include <qcameraimagecapture.h>

#include <qcamera.h>
#include <qcameracapturebufferformatcontrol.h>
#include <qcameracapturedestinationcontrol.h>
#include <qcameracontrol.h>
#include <qcameraimagecapturecontrol.h>
#include <qdebug.h>
#include <qimageencodercontrol.h>
#include <qmediaencodersettings.h>
#include <qmediaservice.h>
#include <qmetaobject.h>
#include <qstringlist.h>
#include <qurl.h>

#include <qmediaobject_p.h>

class QCameraImageCapturePrivate
{
 public:
   QCameraImageCapturePrivate();

   QMediaObject *mediaObject;

   QCameraImageCaptureControl *control;
   QImageEncoderControl *encoderControl;
   QCameraCaptureDestinationControl *captureDestinationControl;
   QCameraCaptureBufferFormatControl *bufferFormatControl;

   QCameraImageCapture::Error error;
   QString errorString;

   void _q_error(int id, int error, const QString &errorString);
   void _q_readyChanged(bool ready);
   void _q_serviceDestroyed();

   void unsetError() {
      error = QCameraImageCapture::NoError;
      errorString.clear();
   }

   QCameraImageCapture *q_ptr;

 private:
   Q_DECLARE_NON_CONST_PUBLIC(QCameraImageCapture)
};

QCameraImageCapturePrivate::QCameraImageCapturePrivate()
   : mediaObject(nullptr), control(nullptr), encoderControl(nullptr), captureDestinationControl(nullptr),
     bufferFormatControl(nullptr), error(QCameraImageCapture::NoError)
{
}

void QCameraImageCapturePrivate::_q_error(int id, int error, const QString &errorString)
{
   Q_Q(QCameraImageCapture);

   this->error = QCameraImageCapture::Error(error);
   this->errorString = errorString;

   emit q->error(id, this->error, errorString);
}

void QCameraImageCapturePrivate::_q_readyChanged(bool ready)
{
   Q_Q(QCameraImageCapture);
   emit q->readyForCaptureChanged(ready);
}

void QCameraImageCapturePrivate::_q_serviceDestroyed()
{
   mediaObject    = nullptr;
   control        = nullptr;
   encoderControl = nullptr;

   captureDestinationControl = nullptr;
   bufferFormatControl       = nullptr;
}

QCameraImageCapture::QCameraImageCapture(QMediaObject *mediaObject, QObject *parent):
   QObject(parent), d_ptr(new QCameraImageCapturePrivate)
{
   Q_D(QCameraImageCapture);

   d->q_ptr = this;

   if (mediaObject) {
      mediaObject->bind(this);
   }
}

QCameraImageCapture::~QCameraImageCapture()
{
   Q_D(QCameraImageCapture);

   if (d->mediaObject) {
      d->mediaObject->unbind(this);
   }

   delete d_ptr;
}

QMediaObject *QCameraImageCapture::mediaObject() const
{
   return d_func()->mediaObject;
}

bool QCameraImageCapture::setMediaObject(QMediaObject *mediaObject)
{
   Q_D(QCameraImageCapture);

   if (d->mediaObject) {

      if (d->control) {
         disconnect(d->control, &QCameraImageCaptureControl::imageExposed,
               this, &QCameraImageCapture::imageExposed);

         disconnect(d->control, &QCameraImageCaptureControl::imageCaptured,
               this, &QCameraImageCapture::imageCaptured);

         disconnect(d->control, &QCameraImageCaptureControl::imageMetadataAvailable,
               this, &QCameraImageCapture::imageMetadataAvailable);

         disconnect(d->control, &QCameraImageCaptureControl::imageAvailable,
               this, &QCameraImageCapture::imageAvailable);

         disconnect(d->control, &QCameraImageCaptureControl::imageSaved,
               this, &QCameraImageCapture::imageSaved);

         disconnect(d->control, &QCameraImageCaptureControl::readyForCaptureChanged,
               this, &QCameraImageCapture::_q_readyChanged);

         disconnect(d->control, &QCameraImageCaptureControl::error,
               this, &QCameraImageCapture::_q_error);

         if (d->captureDestinationControl) {
            disconnect(d->captureDestinationControl, &QCameraCaptureDestinationControl::captureDestinationChanged,
                  this, &QCameraImageCapture::captureDestinationChanged);
         }

         if (d->bufferFormatControl) {
            disconnect(d->bufferFormatControl, &QCameraCaptureBufferFormatControl::bufferFormatChanged,
                  this, &QCameraImageCapture::bufferFormatChanged);
         }

         QMediaService *service = d->mediaObject->service();
         service->releaseControl(d->control);

         if (d->encoderControl) {
            service->releaseControl(d->encoderControl);
         }

         if (d->captureDestinationControl) {
            service->releaseControl(d->captureDestinationControl);
         }

         if (d->bufferFormatControl) {
            service->releaseControl(d->bufferFormatControl);
         }

         disconnect(service, &QMediaService::destroyed, this, &QCameraImageCapture::_q_serviceDestroyed);
      }
   }

   d->mediaObject = mediaObject;

   if (d->mediaObject) {
      QMediaService *service = mediaObject->service();

      if (service) {
         d->control = dynamic_cast<QCameraImageCaptureControl *>(service->requestControl(QCameraImageCaptureControl_iid));

         if (d->control) {
            d->encoderControl            = dynamic_cast<QImageEncoderControl *>(
                  service->requestControl(QImageEncoderControl_iid));

            d->captureDestinationControl = dynamic_cast<QCameraCaptureDestinationControl *>(
                  service->requestControl(QCameraCaptureDestinationControl_iid));

            d->bufferFormatControl       = dynamic_cast<QCameraCaptureBufferFormatControl *>(
                  service->requestControl(QCameraCaptureBufferFormatControl_iid));

            connect(d->control, &QCameraImageCaptureControl::imageExposed,           this, &QCameraImageCapture::imageExposed);
            connect(d->control, &QCameraImageCaptureControl::imageCaptured,          this, &QCameraImageCapture::imageCaptured);
            connect(d->control, &QCameraImageCaptureControl::imageMetadataAvailable, this, &QCameraImageCapture::imageMetadataAvailable);
            connect(d->control, &QCameraImageCaptureControl::imageAvailable,         this, &QCameraImageCapture::imageAvailable);
            connect(d->control, &QCameraImageCaptureControl::imageSaved,             this, &QCameraImageCapture::imageSaved);
            connect(d->control, &QCameraImageCaptureControl::readyForCaptureChanged, this, &QCameraImageCapture::_q_readyChanged);
            connect(d->control, &QCameraImageCaptureControl::error,                  this, &QCameraImageCapture::_q_error);

            if (d->captureDestinationControl) {
               connect(d->captureDestinationControl, &QCameraCaptureDestinationControl::captureDestinationChanged,
                  this, &QCameraImageCapture::captureDestinationChanged);
            }

            if (d->bufferFormatControl) {
               connect(d->bufferFormatControl, &QCameraCaptureBufferFormatControl::bufferFormatChanged,
                  this, &QCameraImageCapture::bufferFormatChanged);
            }

            connect(service, &QMediaService::destroyed, this, &QCameraImageCapture::_q_serviceDestroyed);

            return true;
         }
      }
   }

   // without QCameraImageCaptureControl discard the media object
   d->mediaObject    = nullptr;
   d->control        = nullptr;
   d->encoderControl = nullptr;
   d->captureDestinationControl = nullptr;
   d->bufferFormatControl       = nullptr;

   return false;
}

bool QCameraImageCapture::isAvailable() const
{
   if (d_func()->control != nullptr) {
      return true;
   } else {
      return false;
   }
}

QMultimedia::AvailabilityStatus QCameraImageCapture::availability() const
{
   if (d_func()->control != nullptr) {
      return QMultimedia::Available;
   } else {
      return QMultimedia::ServiceMissing;
   }
}

QCameraImageCapture::Error QCameraImageCapture::error() const
{
   return d_func()->error;
}

QString QCameraImageCapture::errorString() const
{
   return d_func()->errorString;
}

QStringList QCameraImageCapture::supportedImageCodecs() const
{
   return d_func()->encoderControl ?
          d_func()->encoderControl->supportedImageCodecs() : QStringList();
}

QString QCameraImageCapture::imageCodecDescription(const QString &codec) const
{
   return d_func()->encoderControl ?
          d_func()->encoderControl->imageCodecDescription(codec) : QString();
}

QList<QSize> QCameraImageCapture::supportedResolutions(const QImageEncoderSettings &settings, bool *continuous) const
{
   if (continuous) {
      *continuous = false;
   }

   return d_func()->encoderControl ?
          d_func()->encoderControl->supportedResolutions(settings, continuous) : QList<QSize>();
}

QImageEncoderSettings QCameraImageCapture::encodingSettings() const
{
   return d_func()->encoderControl ?
          d_func()->encoderControl->imageSettings() : QImageEncoderSettings();
}

void QCameraImageCapture::setEncodingSettings(const QImageEncoderSettings &settings)
{
   Q_D(QCameraImageCapture);

   if (d->encoderControl) {
      QCamera *camera = dynamic_cast<QCamera *>(d->mediaObject);

      if (camera && camera->captureMode() == QCamera::CaptureStillImage) {
         QMetaObject::invokeMethod(camera, "_q_preparePropertyChange",
                  Qt::DirectConnection, Q_ARG(int, QCameraControl::ImageEncodingSettings));
      }

      d->encoderControl->setImageSettings(settings);
   }
}

QList<QVideoFrame::PixelFormat> QCameraImageCapture::supportedBufferFormats() const
{
   if (d_func()->bufferFormatControl) {
      return d_func()->bufferFormatControl->supportedBufferFormats();
   } else {
      return QList<QVideoFrame::PixelFormat>();
   }
}

/*!
    Returns the buffer image capture format being used.

    \sa supportedBufferFormats(), setBufferFormat()
*/
QVideoFrame::PixelFormat QCameraImageCapture::bufferFormat() const
{
   if (d_func()->bufferFormatControl) {
      return d_func()->bufferFormatControl->bufferFormat();
   } else {
      return QVideoFrame::Format_Invalid;
   }
}

/*!
    Sets the buffer image capture \a format to be used.

    \sa bufferFormat(), supportedBufferFormats(), captureDestination()
*/
void QCameraImageCapture::setBufferFormat(const QVideoFrame::PixelFormat format)
{
   if (d_func()->bufferFormatControl) {
      d_func()->bufferFormatControl->setBufferFormat(format);
   }
}

bool QCameraImageCapture::isCaptureDestinationSupported(QCameraImageCapture::CaptureDestinations destination) const
{
   if (d_func()->captureDestinationControl) {
      return d_func()->captureDestinationControl->isCaptureDestinationSupported(destination);
   } else {
      return destination == CaptureToFile;
   }
}

QCameraImageCapture::CaptureDestinations QCameraImageCapture::captureDestination() const
{
   if (d_func()->captureDestinationControl) {
      return d_func()->captureDestinationControl->captureDestination();
   } else {
      return CaptureToFile;
   }
}

void QCameraImageCapture::setCaptureDestination(QCameraImageCapture::CaptureDestinations destination)
{
   Q_D(QCameraImageCapture);

   if (d->captureDestinationControl) {
      d->captureDestinationControl->setCaptureDestination(destination);
   }
}

bool QCameraImageCapture::isReadyForCapture() const
{
   if (d_func()->control) {
      return d_func()->control->isReadyForCapture();
   } else {
      return false;
   }
}

int QCameraImageCapture::capture(const QString &file)
{
   Q_D(QCameraImageCapture);

   d->unsetError();

   if (d->control) {
      return d->control->capture(file);

   } else {
      d->error = NotSupportedFeatureError;
      d->errorString = tr("Device does not support images capture.");

      emit error(-1, d->error, d->errorString);
   }

   return -1;
}

void QCameraImageCapture::cancelCapture()
{
   Q_D(QCameraImageCapture);

   d->unsetError();

   if (d->control) {
      d->control->cancelCapture();
   } else {
      d->error = NotSupportedFeatureError;
      d->errorString = tr("Device does not support images capture.");

      emit error(-1, d->error, d->errorString);
   }
}

void QCameraImageCapture::_q_error(int id, int error, const QString &errorString)
{
   Q_D(QCameraImageCapture);
   d->_q_error(id, error, errorString);
}

void QCameraImageCapture::_q_readyChanged(bool ready)
{
   Q_D(QCameraImageCapture);
   d->_q_readyChanged(ready);
}

void QCameraImageCapture::_q_serviceDestroyed()
{
   Q_D(QCameraImageCapture);
   d->_q_serviceDestroyed();
}