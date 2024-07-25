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

#ifndef QCAMERAIMAGECAPTURE_H
#define QCAMERAIMAGECAPTURE_H

#include <qmediabindableinterface.h>
#include <qmediaencodersettings.h>
#include <qmediaobject.h>
#include <qvideoframe.h>

class QCameraImageCapturePrivate;
class QImageEncoderSettings;
class QSize;

class Q_MULTIMEDIA_EXPORT QCameraImageCapture : public QObject, public QMediaBindableInterface
{
   MULTI_CS_OBJECT_MULTIPLE(QCameraImageCapture, QObject)

   CS_INTERFACES(QMediaBindableInterface)

   MULTI_CS_PROPERTY_READ(readyForCapture, isReadyForCapture)
   MULTI_CS_PROPERTY_NOTIFY(readyForCapture, readyForCaptureChanged)

   MULTI_CS_ENUM(Error)
   MULTI_CS_ENUM(CaptureDestination)

 public:
   enum Error {
      NoError,
      NotReadyError,
      ResourceError,
      OutOfSpaceError,
      NotSupportedFeatureError,
      FormatError
   };

   enum DriveMode {
      SingleImageCapture
   };

   enum CaptureDestination {
      CaptureToFile   = 0x01,
      CaptureToBuffer = 0x02
   };
   using CaptureDestinations = QFlags<CaptureDestination>;

   explicit QCameraImageCapture(QMediaObject *mediaObject, QObject *parent = nullptr);

   QCameraImageCapture(const QCameraImageCapture &) = delete;
   QCameraImageCapture &operator=(const QCameraImageCapture &) = delete;

   ~QCameraImageCapture();

   bool isAvailable() const;
   QMultimedia::AvailabilityStatus availability() const;

   QMediaObject *mediaObject() const override;

   Error error() const;
   QString errorString() const;

   bool isReadyForCapture() const;

   QStringList supportedImageCodecs() const;
   QString imageCodecDescription(const QString &codecName) const;

   QList<QSize> supportedResolutions(const QImageEncoderSettings &settings = QImageEncoderSettings(),
                                     bool *continuous = nullptr) const;

   QImageEncoderSettings encodingSettings() const;
   void setEncodingSettings(const QImageEncoderSettings &settings);

   QList<QVideoFrame::PixelFormat> supportedBufferFormats() const;
   QVideoFrame::PixelFormat bufferFormat() const;
   void setBufferFormat(const QVideoFrame::PixelFormat format);

   bool isCaptureDestinationSupported(CaptureDestinations destination) const;
   CaptureDestinations captureDestination() const;
   void setCaptureDestination(CaptureDestinations destination);

   MULTI_CS_SLOT_1(Public, int capture(const QString &fileName = QString()))
   MULTI_CS_SLOT_2(capture)

   MULTI_CS_SLOT_1(Public, void cancelCapture())
   MULTI_CS_SLOT_2(cancelCapture)

   MULTI_CS_SIGNAL_1(Public, void error(int id, QCameraImageCapture::Error error, const QString &errorString))
   MULTI_CS_SIGNAL_OVERLOAD(error, (int, QCameraImageCapture::Error, const QString &), id, error, errorString)

   MULTI_CS_SIGNAL_1(Public, void readyForCaptureChanged(bool ready))
   MULTI_CS_SIGNAL_2(readyForCaptureChanged, ready)

   MULTI_CS_SIGNAL_1(Public, void bufferFormatChanged(QVideoFrame::PixelFormat format))
   MULTI_CS_SIGNAL_2(bufferFormatChanged, format)

   MULTI_CS_SIGNAL_1(Public, void captureDestinationChanged(QCameraImageCapture::CaptureDestinations destination))
   MULTI_CS_SIGNAL_2(captureDestinationChanged, destination)

   MULTI_CS_SIGNAL_1(Public, void imageExposed(int id))
   MULTI_CS_SIGNAL_2(imageExposed, id)

   MULTI_CS_SIGNAL_1(Public, void imageCaptured(int id, const QImage &preview))
   MULTI_CS_SIGNAL_2(imageCaptured, id, preview)

   MULTI_CS_SIGNAL_1(Public, void imageMetadataAvailable(int id, const QString &key, const QVariant &value))
   MULTI_CS_SIGNAL_2(imageMetadataAvailable, id, key, value)

   MULTI_CS_SIGNAL_1(Public, void imageAvailable(int id, const QVideoFrame &buffer))
   MULTI_CS_SIGNAL_2(imageAvailable, id, buffer)

   MULTI_CS_SIGNAL_1(Public, void imageSaved(int id, const QString &fileName))
   MULTI_CS_SIGNAL_2(imageSaved, id, fileName)

 protected:
   bool setMediaObject(QMediaObject *mediaObject) override;
   QCameraImageCapturePrivate *d_ptr;

 private:
   Q_DECLARE_PRIVATE(QCameraImageCapture)

   MULTI_CS_SLOT_1(Private, void _q_error(int id, int error, const QString &errorString))
   MULTI_CS_SLOT_2(_q_error)

   MULTI_CS_SLOT_1(Private, void _q_readyChanged(bool ready))
   MULTI_CS_SLOT_2(_q_readyChanged)

   MULTI_CS_SLOT_1(Private, void _q_serviceDestroyed())
   MULTI_CS_SLOT_2(_q_serviceDestroyed)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCameraImageCapture::CaptureDestinations)

#endif

