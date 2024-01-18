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

#include <camera_exposure.h>
#include <camera_session.h>
#include <qdebug.h>

#include <gst/interfaces/photography.h>

#if !GST_CHECK_VERSION(1,0,0)
typedef GstSceneMode GstPhotographySceneMode;
#endif

CameraBinExposure::CameraBinExposure(CameraBinSession *session)
   : QCameraExposureControl(session), m_session(session)
{
}

CameraBinExposure::~CameraBinExposure()
{
}

bool CameraBinExposure::isParameterSupported(ExposureParameter parameter) const
{
   switch (parameter) {
      case QCameraExposureControl::ExposureCompensation:
      case QCameraExposureControl::ISO:
      case QCameraExposureControl::Aperture:
      case QCameraExposureControl::ShutterSpeed:
         return true;

      default:
         return false;
   }
}

QVariantList CameraBinExposure::supportedParameterRange(ExposureParameter parameter,
      bool *continuous) const
{
   if (continuous) {
      *continuous = false;
   }

   QVariantList res;
   switch (parameter) {
      case QCameraExposureControl::ExposureCompensation:
         if (continuous) {
            *continuous = true;
         }
         res << -2.0 << 2.0;
         break;
      case QCameraExposureControl::ISO:
         res << 100 << 200 << 400;
         break;
      case QCameraExposureControl::Aperture:
         res << 2.8;
         break;
      default:
         break;
   }

   return res;
}

QVariant CameraBinExposure::requestedValue(ExposureParameter parameter) const
{
   return m_requestedValues.value(parameter);
}

QVariant CameraBinExposure::actualValue(ExposureParameter parameter) const
{
   switch (parameter) {
      case QCameraExposureControl::ExposureCompensation: {
         gfloat ev;
         gst_photography_get_ev_compensation(m_session->photography(), &ev);
         return QVariant(ev);
      }
      case QCameraExposureControl::ISO: {
         guint isoSpeed = 0;
         gst_photography_get_iso_speed(m_session->photography(), &isoSpeed);
         return QVariant(isoSpeed);
      }
      case QCameraExposureControl::Aperture:
         return QVariant(2.8);
      case QCameraExposureControl::ShutterSpeed: {
         guint32 shutterSpeed = 0;
         gst_photography_get_exposure(m_session->photography(), &shutterSpeed);

         return QVariant(shutterSpeed / 1000000.0);
      }
      case QCameraExposureControl::ExposureMode: {
         GstPhotographySceneMode sceneMode;
         gst_photography_get_scene_mode(m_session->photography(), &sceneMode);

         switch (sceneMode) {
            case GST_PHOTOGRAPHY_SCENE_MODE_PORTRAIT:
               return QVariant::fromValue(QCameraExposure::ExposurePortrait);
            case GST_PHOTOGRAPHY_SCENE_MODE_SPORT:
               return QVariant::fromValue(QCameraExposure::ExposureSports);
            case GST_PHOTOGRAPHY_SCENE_MODE_NIGHT:
               return QVariant::fromValue(QCameraExposure::ExposureNight);
            case GST_PHOTOGRAPHY_SCENE_MODE_MANUAL:
               return QVariant::fromValue(QCameraExposure::ExposureManual);
            case GST_PHOTOGRAPHY_SCENE_MODE_LANDSCAPE:
               return QVariant::fromValue(QCameraExposure::ExposureLandscape);
#if GST_CHECK_VERSION(1, 2, 0)
            case GST_PHOTOGRAPHY_SCENE_MODE_SNOW:
               return QVariant::fromValue(QCameraExposure::ExposureSnow);
            case GST_PHOTOGRAPHY_SCENE_MODE_BEACH:
               return QVariant::fromValue(QCameraExposure::ExposureBeach);
            case GST_PHOTOGRAPHY_SCENE_MODE_ACTION:
               return QVariant::fromValue(QCameraExposure::ExposureAction);
            case GST_PHOTOGRAPHY_SCENE_MODE_NIGHT_PORTRAIT:
               return QVariant::fromValue(QCameraExposure::ExposureNightPortrait);
            case GST_PHOTOGRAPHY_SCENE_MODE_THEATRE:
               return QVariant::fromValue(QCameraExposure::ExposureTheatre);
            case GST_PHOTOGRAPHY_SCENE_MODE_SUNSET:
               return QVariant::fromValue(QCameraExposure::ExposureSunset);
            case GST_PHOTOGRAPHY_SCENE_MODE_STEADY_PHOTO:
               return QVariant::fromValue(QCameraExposure::ExposureSteadyPhoto);
            case GST_PHOTOGRAPHY_SCENE_MODE_FIREWORKS:
               return QVariant::fromValue(QCameraExposure::ExposureFireworks);
            case GST_PHOTOGRAPHY_SCENE_MODE_PARTY:
               return QVariant::fromValue(QCameraExposure::ExposureParty);
            case GST_PHOTOGRAPHY_SCENE_MODE_CANDLELIGHT:
               return QVariant::fromValue(QCameraExposure::ExposureCandlelight);
            case GST_PHOTOGRAPHY_SCENE_MODE_BARCODE:
               return QVariant::fromValue(QCameraExposure::ExposureBarcode);
#endif
            //no direct mapping available so mapping to auto mode
            case GST_PHOTOGRAPHY_SCENE_MODE_CLOSEUP:
            case GST_PHOTOGRAPHY_SCENE_MODE_AUTO:
            default:
               return QVariant::fromValue(QCameraExposure::ExposureAuto);
         }
      }
      case QCameraExposureControl::MeteringMode:
         return QCameraExposure::MeteringMatrix;
      default:
         return QVariant();
   }
}

bool CameraBinExposure::setValue(ExposureParameter parameter, const QVariant &value)
{
   QVariant oldValue = actualValue(parameter);

   switch (parameter) {
      case QCameraExposureControl::ExposureCompensation:
         gst_photography_set_ev_compensation(m_session->photography(), value.toReal());
         break;
      case QCameraExposureControl::ISO:
         gst_photography_set_iso_speed(m_session->photography(), value.toInt());
         break;
      case QCameraExposureControl::Aperture:
         gst_photography_set_aperture(m_session->photography(), guint(value.toReal() * 1000000));
         break;
      case QCameraExposureControl::ShutterSpeed:
         gst_photography_set_exposure(m_session->photography(), guint(value.toReal() * 1000000));
         break;
      case QCameraExposureControl::ExposureMode: {
         QCameraExposure::ExposureMode mode = value.value<QCameraExposure::ExposureMode>();
         GstPhotographySceneMode sceneMode;

         gst_photography_get_scene_mode(m_session->photography(), &sceneMode);

         switch (mode) {
            case QCameraExposure::ExposureManual:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_MANUAL;
               break;
            case QCameraExposure::ExposurePortrait:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_PORTRAIT;
               break;
            case QCameraExposure::ExposureSports:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_SPORT;
               break;
            case QCameraExposure::ExposureNight:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_NIGHT;
               break;
            case QCameraExposure::ExposureAuto:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_AUTO;
               break;
            case QCameraExposure::ExposureLandscape:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_LANDSCAPE;
               break;
#if GST_CHECK_VERSION(1, 2, 0)
            case QCameraExposure::ExposureSnow:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_SNOW;
               break;
            case QCameraExposure::ExposureBeach:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_BEACH;
               break;
            case QCameraExposure::ExposureAction:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_ACTION;
               break;
            case QCameraExposure::ExposureNightPortrait:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_NIGHT_PORTRAIT;
               break;
            case QCameraExposure::ExposureTheatre:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_THEATRE;
               break;
            case QCameraExposure::ExposureSunset:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_SUNSET;
               break;
            case QCameraExposure::ExposureSteadyPhoto:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_STEADY_PHOTO;
               break;
            case QCameraExposure::ExposureFireworks:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_FIREWORKS;
               break;
            case QCameraExposure::ExposureParty:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_PARTY;
               break;
            case QCameraExposure::ExposureCandlelight:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_CANDLELIGHT;
               break;
            case QCameraExposure::ExposureBarcode:
               sceneMode = GST_PHOTOGRAPHY_SCENE_MODE_BARCODE;
               break;
#endif
            default:
               break;
         }

         gst_photography_set_scene_mode(m_session->photography(), sceneMode);
         break;
      }
      default:
         return false;
   }

   if (!qFuzzyCompare(m_requestedValues.value(parameter).toReal(), value.toReal())) {
      m_requestedValues[parameter] = value;
      emit requestedValueChanged(parameter);
   }

   QVariant newValue = actualValue(parameter);
   if (!qFuzzyCompare(oldValue.toReal(), newValue.toReal())) {
      emit actualValueChanged(parameter);
   }

   return true;
}
