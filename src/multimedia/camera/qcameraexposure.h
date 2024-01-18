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

#ifndef QCAMERAEXPOSURE_H
#define QCAMERAEXPOSURE_H

#include <qmediaobject.h>

class QCamera;
class QCameraExposurePrivate;

class Q_MULTIMEDIA_EXPORT QCameraExposure : public QObject
{
   MULTI_CS_OBJECT(QCameraExposure)

   MULTI_CS_PROPERTY_READ(aperture, aperture)
   MULTI_CS_PROPERTY_NOTIFY(aperture, apertureChanged)

   MULTI_CS_PROPERTY_READ(shutterSpeed, shutterSpeed)
   MULTI_CS_PROPERTY_NOTIFY(shutterSpeed, shutterSpeedChanged)

   MULTI_CS_PROPERTY_READ(isoSensitivity, isoSensitivity)
   MULTI_CS_PROPERTY_NOTIFY(isoSensitivity, isoSensitivityChanged)

   MULTI_CS_PROPERTY_READ(exposureCompensation, exposureCompensation)
   MULTI_CS_PROPERTY_WRITE(exposureCompensation, setExposureCompensation)
   MULTI_CS_PROPERTY_NOTIFY(exposureCompensation, exposureCompensationChanged)

   MULTI_CS_PROPERTY_READ(flashReady, isFlashReady)
   MULTI_CS_PROPERTY_NOTIFY(flashReady, flashReady)

   MULTI_CS_PROPERTY_READ(flashMode, flashMode)
   MULTI_CS_PROPERTY_WRITE(flashMode, setFlashMode)
   MULTI_CS_PROPERTY_READ(exposureMode, exposureMode)
   MULTI_CS_PROPERTY_WRITE(exposureMode, setExposureMode)
   MULTI_CS_PROPERTY_READ(meteringMode, meteringMode)
   MULTI_CS_PROPERTY_WRITE(meteringMode, setMeteringMode)

   MULTI_CS_ENUM(FlashMode)
   MULTI_CS_ENUM(ExposureMode)
   MULTI_CS_ENUM(MeteringMode)

 public:
   enum FlashMode {
      FlashAuto = 0x1,
      FlashOff = 0x2,
      FlashOn = 0x4,
      FlashRedEyeReduction  = 0x8,
      FlashFill = 0x10,
      FlashTorch = 0x20,
      FlashVideoLight = 0x40,
      FlashSlowSyncFrontCurtain = 0x80,
      FlashSlowSyncRearCurtain = 0x100,
      FlashManual = 0x200
   };
   using FlashModes = QFlags<FlashMode>;

   enum ExposureMode {
      ExposureAuto = 0,
      ExposureManual = 1,
      ExposurePortrait = 2,
      ExposureNight = 3,
      ExposureBacklight = 4,
      ExposureSpotlight = 5,
      ExposureSports = 6,
      ExposureSnow = 7,
      ExposureBeach = 8,
      ExposureLargeAperture = 9,
      ExposureSmallAperture = 10,
      ExposureAction = 11,
      ExposureLandscape = 12,
      ExposureNightPortrait = 13,
      ExposureTheatre = 14,
      ExposureSunset = 15,
      ExposureSteadyPhoto = 16,
      ExposureFireworks = 17,
      ExposureParty = 18,
      ExposureCandlelight = 19,
      ExposureBarcode = 20,
      ExposureModeVendor = 1000
   };

   enum MeteringMode {
      MeteringMatrix = 1,
      MeteringAverage = 2,
      MeteringSpot = 3
   };

   QCameraExposure(const QCameraExposure &) = delete;
   QCameraExposure &operator=(const QCameraExposure &) = delete;

   bool isAvailable() const;

   FlashModes flashMode() const;
   bool isFlashModeSupported(FlashModes mode) const;
   bool isFlashReady() const;

   ExposureMode exposureMode() const;
   bool isExposureModeSupported(ExposureMode mode) const;

   double exposureCompensation() const;

   MeteringMode meteringMode() const;
   bool isMeteringModeSupported(MeteringMode mode) const;

   QPointF spotMeteringPoint() const;
   void setSpotMeteringPoint(const QPointF &point);

   int isoSensitivity() const;
   double aperture() const;
   double shutterSpeed() const;

   int requestedIsoSensitivity() const;
   double requestedAperture() const;
   double requestedShutterSpeed() const;

   QList<int> supportedIsoSensitivities(bool *continuous = nullptr) const;
   QList<double> supportedApertures(bool *continuous = nullptr) const;
   QList<double> supportedShutterSpeeds(bool *continuous = nullptr) const;

   MULTI_CS_SLOT_1(Public, void setFlashMode(FlashModes mode))
   MULTI_CS_SLOT_2(setFlashMode)

   MULTI_CS_SLOT_1(Public, void setExposureMode(ExposureMode mode))
   MULTI_CS_SLOT_2(setExposureMode)

   MULTI_CS_SLOT_1(Public, void setMeteringMode(MeteringMode mode))
   MULTI_CS_SLOT_2(setMeteringMode)

   MULTI_CS_SLOT_1(Public, void setExposureCompensation(double value))
   MULTI_CS_SLOT_2(setExposureCompensation)

   MULTI_CS_SLOT_1(Public, void setManualIsoSensitivity(int iso))
   MULTI_CS_SLOT_2(setManualIsoSensitivity)

   MULTI_CS_SLOT_1(Public, void setAutoIsoSensitivity())
   MULTI_CS_SLOT_2(setAutoIsoSensitivity)

   MULTI_CS_SLOT_1(Public, void setManualAperture(double aperture))
   MULTI_CS_SLOT_2(setManualAperture)

   MULTI_CS_SLOT_1(Public, void setAutoAperture())
   MULTI_CS_SLOT_2(setAutoAperture)

   MULTI_CS_SLOT_1(Public, void setManualShutterSpeed(double seconds))
   MULTI_CS_SLOT_2(setManualShutterSpeed)

   MULTI_CS_SLOT_1(Public, void setAutoShutterSpeed())
   MULTI_CS_SLOT_2(setAutoShutterSpeed)

   MULTI_CS_SIGNAL_1(Public, void flashReady(bool ready))
   MULTI_CS_SIGNAL_2(flashReady, ready)

   MULTI_CS_SIGNAL_1(Public, void apertureChanged(double value))
   MULTI_CS_SIGNAL_2(apertureChanged, value)

   MULTI_CS_SIGNAL_1(Public, void apertureRangeChanged())
   MULTI_CS_SIGNAL_2(apertureRangeChanged)

   MULTI_CS_SIGNAL_1(Public, void shutterSpeedChanged(double speed))
   MULTI_CS_SIGNAL_2(shutterSpeedChanged, speed)

   MULTI_CS_SIGNAL_1(Public, void shutterSpeedRangeChanged())
   MULTI_CS_SIGNAL_2(shutterSpeedRangeChanged)

   MULTI_CS_SIGNAL_1(Public, void isoSensitivityChanged(int value))
   MULTI_CS_SIGNAL_2(isoSensitivityChanged, value)

   MULTI_CS_SIGNAL_1(Public, void exposureCompensationChanged(double value))
   MULTI_CS_SIGNAL_2(exposureCompensationChanged, value)

 private:
   explicit QCameraExposure(QCamera *parent = nullptr);
   virtual ~QCameraExposure();

   Q_DECLARE_PRIVATE(QCameraExposure)

   MULTI_CS_SLOT_1(Private, void _q_exposureParameterChanged(int value))
   MULTI_CS_SLOT_2(_q_exposureParameterChanged)

   MULTI_CS_SLOT_1(Private, void _q_exposureParameterRangeChanged(int value))
   MULTI_CS_SLOT_2(_q_exposureParameterRangeChanged)

   QCameraExposurePrivate *d_ptr;

   friend class QCamera;
   friend class QCameraPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCameraExposure::FlashModes)

#endif