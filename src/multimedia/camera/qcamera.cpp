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

#include <qcamera_p.h>

#include <qcameracontrol.h>
#include <qcameraexposurecontrol.h>
#include <qcamerafocuscontrol.h>
#include <qcameraimagecapturecontrol.h>
#include <qcameraimageprocessingcontrol.h>
#include <qcamerainfo.h>
#include <qcamerainfocontrol.h>
#include <qcameralockscontrol.h>
#include <qcameraviewfindersettingscontrol.h>
#include <qdebug.h>
#include <qmediarecordercontrol.h>
#include <qvideodeviceselectorcontrol.h>

#include <qmediaserviceprovider_p.h>

static constexpr bool qt_sizeLessThan(const QSize &s1, const QSize &s2)
{
   return (s1.width() * s1.height()) < (s2.width() * s2.height());
}

static constexpr bool qt_frameRateRangeLessThan(const QCamera::FrameRateRange &s1, const QCamera::FrameRateRange &s2)
{
   return qFuzzyCompare(s1.maximumFrameRate, s2.maximumFrameRate) ? (s1.minimumFrameRate < s2.minimumFrameRate)
          : (s1.maximumFrameRate < s2.maximumFrameRate);
}

void QCameraPrivate::_q_error(int error, const QString &errorString)
{
   Q_Q(QCamera);

   this->error = QCamera::Error(error);
   this->errorString = errorString;

   emit q->error(this->error);
}

void QCameraPrivate::setState(QCamera::State newState)
{
   unsetError();

   if (!control) {
      _q_error(QCamera::ServiceMissingError, QCamera::tr("The camera service is missing"));
      return;
   }

   restartPending = false;
   control->setState(newState);
}

void QCameraPrivate::_q_updateState(QCamera::State newState)
{
   Q_Q(QCamera);

   //omit changins state to Loaded when the camera is temporarily
   //stopped to apply shanges
   if (restartPending) {
      return;
   }

   if (newState != state) {
      state = newState;
      emit q->stateChanged(state);
   }
}

void QCameraPrivate::_q_preparePropertyChange(int changeType)
{
   if (!control) {
      return;
   }

   QCamera::Status status = control->status();

   //all the changes are allowed until the camera is starting
   if (control->state() != QCamera::ActiveState) {
      return;
   }

   if (control->canChangeProperty(QCameraControl::PropertyChangeType(changeType), status)) {
      return;
   }

   restartPending = true;
   control->setState(QCamera::LoadedState);
   QMetaObject::invokeMethod(q_ptr, "_q_restartCamera", Qt::QueuedConnection);
}

void QCameraPrivate::_q_restartCamera()
{
   if (restartPending) {
      restartPending = false;
      control->setState(QCamera::ActiveState);
   }
}

void QCameraPrivate::init()
{
   Q_Q(QCamera);
   provider = QMediaServiceProvider::defaultServiceProvider();
   initControls();

   cameraExposure  = new QCameraExposure(q);
   cameraFocus     = new QCameraFocus(q);
   imageProcessing = new QCameraImageProcessing(q);
}

void QCameraPrivate::initControls()
{
   Q_Q(QCamera);

   if (service) {
      control       = dynamic_cast<QCameraControl *>(service->requestControl(QCameraControl_iid));
      locksControl  = dynamic_cast<QCameraLocksControl *>(service->requestControl(QCameraLocksControl_iid));
      deviceControl = dynamic_cast<QVideoDeviceSelectorControl *>(service->requestControl(QVideoDeviceSelectorControl_iid));
      infoControl   = dynamic_cast<QCameraInfoControl *>(service->requestControl(QCameraInfoControl_iid));

      viewfinderSettingsControl2 = dynamic_cast<QCameraViewfinderSettingsControl2 *>(service->requestControl(QCameraViewfinderSettingsControl2_iid));

      if (! viewfinderSettingsControl2) {
         viewfinderSettingsControl = dynamic_cast<QCameraViewfinderSettingsControl *>(service->requestControl(QCameraViewfinderSettingsControl_iid));
      }

      if (control) {
         q->connect(control, &QCameraControl::stateChanged,       q, &QCamera::_q_updateState);
         q->connect(control, &QCameraControl::statusChanged,      q, &QCamera::statusChanged);
         q->connect(control, &QCameraControl::captureModeChanged, q, &QCamera::captureModeChanged);
         q->connect(control, &QCameraControl::error,              q, &QCamera::_q_error);
      }

      if (locksControl) {
         q->connect(locksControl, &QCameraLocksControl::lockStatusChanged, q, &QCamera::_q_updateLockStatus);
      }

      error = QCamera::NoError;

   } else {
      control       = nullptr;
      locksControl  = nullptr;
      deviceControl = nullptr;
      infoControl   = nullptr;
      viewfinderSettingsControl  = nullptr;
      viewfinderSettingsControl2 = nullptr;

      error = QCamera::ServiceMissingError;
      errorString = QCamera::tr("The camera service is missing");
   }
}

void QCameraPrivate::clear()
{
   delete cameraExposure;
   delete cameraFocus;
   delete imageProcessing;

   if (service) {
      if (control) {
         service->releaseControl(control);
      }

      if (locksControl) {
         service->releaseControl(locksControl);
      }

      if (deviceControl) {
         service->releaseControl(deviceControl);
      }

      if (infoControl) {
         service->releaseControl(infoControl);
      }

      if (viewfinderSettingsControl) {
         service->releaseControl(viewfinderSettingsControl);
      }

      if (viewfinderSettingsControl2) {
         service->releaseControl(viewfinderSettingsControl2);
      }

      provider->releaseService(service);
   }

   cameraExposure  = nullptr;
   cameraFocus     = nullptr;
   imageProcessing = nullptr;
   control         = nullptr;
   locksControl    = nullptr;
   deviceControl   = nullptr;
   infoControl     = nullptr;
   viewfinderSettingsControl  = nullptr;
   viewfinderSettingsControl2 = nullptr;
   service         = nullptr;
}

void QCameraPrivate::updateLockStatus()
{
   Q_Q(QCamera);

   QCamera::LockStatus oldStatus = lockStatus;

   QMap<QCamera::LockStatus, int> lockStatusPriority;
   lockStatusPriority.insert(QCamera::Locked, 1);
   lockStatusPriority.insert(QCamera::Unlocked, 2);
   lockStatusPriority.insert(QCamera::Searching, 3);

   lockStatus = requestedLocks ? QCamera::Locked : QCamera::Unlocked;
   int priority = 0;

   QList<QCamera::LockStatus> lockStatuses;

   if (requestedLocks & QCamera::LockFocus) {
      lockStatuses << q->lockStatus(QCamera::LockFocus);
   }

   if (requestedLocks & QCamera::LockExposure) {
      lockStatuses << q->lockStatus(QCamera::LockExposure);
   }

   if (requestedLocks & QCamera::LockWhiteBalance) {
      lockStatuses << q->lockStatus(QCamera::LockWhiteBalance);
   }

   for (QCamera::LockStatus currentStatus : lockStatuses) {
      int currentPriority = lockStatusPriority.value(currentStatus, -1);

      if (currentPriority > priority) {
         priority = currentPriority;
         lockStatus = currentStatus;
      }
   }

   if (! supressLockChangedSignal && oldStatus != lockStatus) {
      emit q->cs_lockStatusChanged(lockStatus, lockChangeReason);
      emit q->lockStatusChanged(lockStatus, lockChangeReason);

      if (lockStatus == QCamera::Locked) {
         emit q->locked();
      }

      else if (lockStatus == QCamera::Unlocked && lockChangeReason == QCamera::LockFailed) {
         emit q->lockFailed();
      }
   }

#if defined(CS_SHOW_DEBUG_MULTIMEDIA)
   qDebug() << "Requested locks:" << (requestedLocks & QCamera::LockExposure ? 'e' : ' ')
         << (requestedLocks & QCamera::LockFocus ? 'f' : ' ')
         << (requestedLocks & QCamera::LockWhiteBalance ? 'w' : ' ');

   qDebug() << "Lock status: f:" << q->lockStatus(QCamera::LockFocus)
         << " e:" << q->lockStatus(QCamera::LockExposure)
         << " w:" << q->lockStatus(QCamera::LockWhiteBalance)
         << " composite:" << lockStatus;
#endif

}

void QCameraPrivate::_q_updateLockStatus(QCamera::LockType type, QCamera::LockStatus status, QCamera::LockChangeReason reason)
{
   Q_Q(QCamera);
   lockChangeReason = reason;
   updateLockStatus();

   emit q->lockStatusChanged(type, status, reason);
}

QCamera::QCamera(QObject *parent)
   : QMediaObject(*new QCameraPrivate, parent,
        QMediaServiceProvider::defaultServiceProvider()->requestService(Q_MEDIASERVICE_CAMERA))
{
   Q_D(QCamera);
   d->init();

   // Select the default camera
   if (d->service != nullptr && d->deviceControl) {
      d->deviceControl->setSelectedDevice(d->deviceControl->defaultDevice());
   }
}


QCamera::QCamera(const QByteArray &deviceName, QObject *parent)
   : QMediaObject(*new QCameraPrivate, parent,
        QMediaServiceProvider::defaultServiceProvider()->requestService(Q_MEDIASERVICE_CAMERA, QMediaServiceProviderHint(deviceName)))
{
   Q_D(QCamera);
   d->init();

   if (d->service != nullptr) {
      //pass device name to service
      if (d->deviceControl) {
         const QString name = QString::fromLatin1(deviceName);

         for (int i = 0; i < d->deviceControl->deviceCount(); i++) {
            if (d->deviceControl->deviceName(i) == name) {
               d->deviceControl->setSelectedDevice(i);
               break;
            }
         }
      }
   }
}

QCamera::QCamera(const QCameraInfo &cameraInfo, QObject *parent)
   : QMediaObject(*new QCameraPrivate, parent,
        QMediaServiceProvider::defaultServiceProvider()->requestService(Q_MEDIASERVICE_CAMERA,
        QMediaServiceProviderHint(cameraInfo.deviceName().toLatin1())))
{
   Q_D(QCamera);
   d->init();

   if (d->service != nullptr && d->deviceControl) {
      for (int i = 0; i < d->deviceControl->deviceCount(); i++) {
         if (d->deviceControl->deviceName(i) == cameraInfo.deviceName()) {
            d->deviceControl->setSelectedDevice(i);
            break;
         }
      }
   }
}


QCamera::QCamera(QCamera::Position position, QObject *parent)
   : QMediaObject(*new QCameraPrivate, parent,
        QMediaServiceProvider::defaultServiceProvider()->requestService(Q_MEDIASERVICE_CAMERA, QMediaServiceProviderHint(position)))
{
   Q_D(QCamera);
   d->init();

   if (d->service != nullptr && d->deviceControl) {
      bool selectDefault = true;

      if (d->infoControl && position != UnspecifiedPosition) {
         for (int i = 0; i < d->deviceControl->deviceCount(); i++) {
            if (d->infoControl->cameraPosition(d->deviceControl->deviceName(i)) == position) {
               d->deviceControl->setSelectedDevice(i);
               selectDefault = false;
               break;
            }
         }
      }

      if (selectDefault) {
         d->deviceControl->setSelectedDevice(d->deviceControl->defaultDevice());
      }
   }
}

QCamera::~QCamera()
{
   Q_D(QCamera);
   d->clear();
}

QMultimedia::AvailabilityStatus QCamera::availability() const
{
   Q_D(const QCamera);

   if (d->control == nullptr) {
      return QMultimedia::ServiceMissing;
   }

   if (d->deviceControl && d->deviceControl->deviceCount() == 0) {
      return QMultimedia::ResourceError;
   }

   if (d->error != QCamera::NoError) {
      return QMultimedia::ResourceError;
   }

   return QMediaObject::availability();
}


QCameraExposure *QCamera::exposure() const
{
   return d_func()->cameraExposure;
}

QCameraFocus *QCamera::focus() const
{
   return d_func()->cameraFocus;
}


QCameraImageProcessing *QCamera::imageProcessing() const
{
   return d_func()->imageProcessing;
}

void QCamera::setViewfinder(QVideoWidget *viewfinder)
{
   Q_D(QCamera);
   d->_q_preparePropertyChange(QCameraControl::Viewfinder);

   if (d->viewfinder) {
      unbind(d->viewfinder);
   }

   // We don't know (in this library) that QVideoWidget inherits QObject
   QObject *viewFinderObject = reinterpret_cast<QObject *>(viewfinder);

   d->viewfinder = viewFinderObject && bind(viewFinderObject) ? viewFinderObject : nullptr;
}

void QCamera::setViewfinder(QGraphicsVideoItem *viewfinder)
{
   Q_D(QCamera);
   d->_q_preparePropertyChange(QCameraControl::Viewfinder);

   if (d->viewfinder) {
      unbind(d->viewfinder);
   }

   // We don't know (in this library) that QGraphicsVideoItem (multiply) inherits QObject
   // but QObject inheritance depends on QObject coming first, so try this out.
   QObject *viewFinderObject = reinterpret_cast<QObject *>(viewfinder);

   d->viewfinder = viewFinderObject && bind(viewFinderObject) ? viewFinderObject : nullptr;
}

void QCamera::setViewfinder(QAbstractVideoSurface *surface)
{
   Q_D(QCamera);

   d->surfaceViewfinder.setVideoSurface(surface);

   if (d->viewfinder != &d->surfaceViewfinder) {
      if (d->viewfinder) {
         unbind(d->viewfinder);
      }

      d->viewfinder = nullptr;

      if (surface && bind(&d->surfaceViewfinder)) {
         d->viewfinder = &d->surfaceViewfinder;
      }
   } else if (!surface) {
      //unbind the surfaceViewfinder if null surface is set
      unbind(&d->surfaceViewfinder);
      d->viewfinder = nullptr;
   }
}

QCameraViewfinderSettings QCamera::viewfinderSettings() const
{
   Q_D(const QCamera);

   if (d->viewfinderSettingsControl2) {
      return d->viewfinderSettingsControl2->viewfinderSettings();
   }

   QCameraViewfinderSettings settings;

   if (d->viewfinderSettingsControl) {
      if (d->viewfinderSettingsControl->isViewfinderParameterSupported(QCameraViewfinderSettingsControl::Resolution)) {
         settings.setResolution(d->viewfinderSettingsControl->viewfinderParameter(QCameraViewfinderSettingsControl::Resolution).toSize());
      }

      if (d->viewfinderSettingsControl->isViewfinderParameterSupported(QCameraViewfinderSettingsControl::MinimumFrameRate)) {
         settings.setMinimumFrameRate(d->viewfinderSettingsControl->viewfinderParameter(QCameraViewfinderSettingsControl::MinimumFrameRate).toReal());
      }

      if (d->viewfinderSettingsControl->isViewfinderParameterSupported(QCameraViewfinderSettingsControl::MaximumFrameRate)) {
         settings.setMaximumFrameRate(d->viewfinderSettingsControl->viewfinderParameter(QCameraViewfinderSettingsControl::MaximumFrameRate).toReal());
      }

      if (d->viewfinderSettingsControl->isViewfinderParameterSupported(QCameraViewfinderSettingsControl::PixelAspectRatio)) {
         settings.setPixelAspectRatio(d->viewfinderSettingsControl->viewfinderParameter(QCameraViewfinderSettingsControl::PixelAspectRatio).toSize());
      }

      if (d->viewfinderSettingsControl->isViewfinderParameterSupported(QCameraViewfinderSettingsControl::PixelFormat)) {
         QVariant data = d->viewfinderSettingsControl->viewfinderParameter(QCameraViewfinderSettingsControl::PixelFormat);
         settings.setPixelFormat(data.value<QVideoFrame::PixelFormat>());
      }
   }

   return settings;
}

void QCamera::setViewfinderSettings(const QCameraViewfinderSettings &settings)
{
   Q_D(QCamera);

   if (d->viewfinderSettingsControl || d->viewfinderSettingsControl2) {
      d->_q_preparePropertyChange(QCameraControl::ViewfinderSettings);
   }

   if (d->viewfinderSettingsControl2) {
      d->viewfinderSettingsControl2->setViewfinderSettings(settings);

   } else if (d->viewfinderSettingsControl) {
      if (d->viewfinderSettingsControl->isViewfinderParameterSupported(QCameraViewfinderSettingsControl::Resolution)) {
         d->viewfinderSettingsControl->setViewfinderParameter(QCameraViewfinderSettingsControl::Resolution, settings.resolution());
      }

      if (d->viewfinderSettingsControl->isViewfinderParameterSupported(QCameraViewfinderSettingsControl::MinimumFrameRate)) {
         d->viewfinderSettingsControl->setViewfinderParameter(QCameraViewfinderSettingsControl::MinimumFrameRate, settings.minimumFrameRate());
      }

      if (d->viewfinderSettingsControl->isViewfinderParameterSupported(QCameraViewfinderSettingsControl::MaximumFrameRate)) {
         d->viewfinderSettingsControl->setViewfinderParameter(QCameraViewfinderSettingsControl::MaximumFrameRate, settings.maximumFrameRate());
      }

      if (d->viewfinderSettingsControl->isViewfinderParameterSupported(QCameraViewfinderSettingsControl::PixelAspectRatio)) {
         d->viewfinderSettingsControl->setViewfinderParameter(QCameraViewfinderSettingsControl::PixelAspectRatio, settings.pixelAspectRatio());
      }

      if (d->viewfinderSettingsControl->isViewfinderParameterSupported(QCameraViewfinderSettingsControl::PixelFormat)) {
         d->viewfinderSettingsControl->setViewfinderParameter(QCameraViewfinderSettingsControl::PixelFormat, settings.pixelFormat());
      }
   }
}

QList<QCameraViewfinderSettings> QCamera::supportedViewfinderSettings(const QCameraViewfinderSettings &settings) const
{
   Q_D(const QCamera);

   if (!d->viewfinderSettingsControl2) {
      return QList<QCameraViewfinderSettings>();
   }

   if (settings.isNull()) {
      return d->viewfinderSettingsControl2->supportedViewfinderSettings();
   }

   QList<QCameraViewfinderSettings> results;
   QList<QCameraViewfinderSettings> supported = d->viewfinderSettingsControl2->supportedViewfinderSettings();

   for (const QCameraViewfinderSettings &s : supported) {
      if ((settings.resolution().isEmpty() || settings.resolution() == s.resolution())
            && (qFuzzyIsNull(settings.minimumFrameRate()) || qFuzzyCompare((float)settings.minimumFrameRate(), (float)s.minimumFrameRate()))
            && (qFuzzyIsNull(settings.maximumFrameRate()) || qFuzzyCompare((float)settings.maximumFrameRate(), (float)s.maximumFrameRate()))
            && (settings.pixelFormat() == QVideoFrame::Format_Invalid || settings.pixelFormat() == s.pixelFormat())
            && (settings.pixelAspectRatio().isEmpty() || settings.pixelAspectRatio() == s.pixelAspectRatio())) {
         results.append(s);
      }
   }

   return results;
}

QList<QSize> QCamera::supportedViewfinderResolutions(const QCameraViewfinderSettings &settings) const
{
   QList<QSize> resolutions;
   QList<QCameraViewfinderSettings> capabilities = supportedViewfinderSettings(settings);

   for (const QCameraViewfinderSettings &s : capabilities) {
      if (!resolutions.contains(s.resolution())) {
         resolutions.append(s.resolution());
      }
   }

   std::sort(resolutions.begin(), resolutions.end(), qt_sizeLessThan);

   return resolutions;
}

QList<QCamera::FrameRateRange> QCamera::supportedViewfinderFrameRateRanges(const QCameraViewfinderSettings &settings) const
{
   QList<QCamera::FrameRateRange> frameRateRanges;
   QList<QCameraViewfinderSettings> capabilities = supportedViewfinderSettings(settings);

   for (const QCameraViewfinderSettings &s : capabilities) {
      QCamera::FrameRateRange range(s.minimumFrameRate(), s.maximumFrameRate());

      if (!frameRateRanges.contains(range)) {
         frameRateRanges.append(range);
      }
   }
   std::sort(frameRateRanges.begin(), frameRateRanges.end(), qt_frameRateRangeLessThan);

   return frameRateRanges;
}

QList<QVideoFrame::PixelFormat> QCamera::supportedViewfinderPixelFormats(const QCameraViewfinderSettings &settings) const
{
   QList<QVideoFrame::PixelFormat> pixelFormats;
   QList<QCameraViewfinderSettings> capabilities = supportedViewfinderSettings(settings);

   for (const QCameraViewfinderSettings &s : capabilities) {
      if (!pixelFormats.contains(s.pixelFormat())) {
         pixelFormats.append(s.pixelFormat());
      }
   }

   return pixelFormats;
}

QCamera::Error QCamera::error() const
{
   return d_func()->error;
}

QString QCamera::errorString() const
{
   return d_func()->errorString;
}

bool QCamera::isCaptureModeSupported(QCamera::CaptureModes mode) const
{
   return d_func()->control ? d_func()->control->isCaptureModeSupported(mode) : false;
}

QCamera::CaptureModes QCamera::captureMode() const
{
   return d_func()->control ? d_func()->control->captureMode() : QCamera::CaptureStillImage;
}

void QCamera::setCaptureMode(QCamera::CaptureModes mode)
{
   Q_D(QCamera);

   if (mode != captureMode()) {
      if (d->control) {
         d->_q_preparePropertyChange(QCameraControl::CaptureMode);
         d->control->setCaptureMode(mode);
      }
   }
}

void QCamera::start()
{
   Q_D(QCamera);
   d->setState(QCamera::ActiveState);
}

void QCamera::stop()
{
   Q_D(QCamera);
   d->setState(QCamera::LoadedState);
}


void QCamera::load()
{
   Q_D(QCamera);
   d->setState(QCamera::LoadedState);
}

void QCamera::unload()
{
   Q_D(QCamera);
   d->setState(QCamera::UnloadedState);
}

QCamera::State QCamera::state() const
{
   return d_func()->state;
}

QCamera::Status QCamera::status() const
{
   if (d_func()->control) {
      return (QCamera::Status)d_func()->control->status();
   }

   return QCamera::UnavailableStatus;
}

QCamera::LockTypes QCamera::supportedLocks() const
{
   Q_D(const QCamera);

   return d->locksControl
          ? d->locksControl->supportedLocks()
          : QCamera::LockTypes();
}

QCamera::LockTypes QCamera::requestedLocks() const
{
   return d_func()->requestedLocks;
}

QCamera::LockStatus QCamera::lockStatus() const
{
   return d_func()->lockStatus;
}

QCamera::LockStatus QCamera::lockStatus(QCamera::LockType lockType) const
{
   const QCameraPrivate *d = d_func();

   if (! (lockType & d->requestedLocks)) {
      return QCamera::Unlocked;
   }

   if (d->locksControl) {
      return d->locksControl->lockStatus(lockType);
   }

   return QCamera::Locked;
}

void QCamera::searchAndLock(QCamera::LockTypes locks)
{
   Q_D(QCamera);

   QCamera::LockStatus oldStatus = d->lockStatus;
   d->supressLockChangedSignal = true;

   if (d->locksControl) {
      locks &= d->locksControl->supportedLocks();
      d->requestedLocks |= locks;
      d->locksControl->searchAndLock(locks);
   }

   d->supressLockChangedSignal = false;

   d->lockStatus = oldStatus;
   d->updateLockStatus();
}

void QCamera::searchAndLock()
{
   searchAndLock(LockExposure | LockWhiteBalance | LockFocus);
}

void QCamera::unlock(QCamera::LockTypes locks)
{
   Q_D(QCamera);

   QCamera::LockStatus oldStatus = d->lockStatus;
   d->supressLockChangedSignal = true;

   d->requestedLocks &= ~locks;

   if (d->locksControl) {
      locks &= d->locksControl->supportedLocks();
      d->locksControl->unlock(locks);
   }

   d->supressLockChangedSignal = false;

   d->lockStatus = oldStatus;
   d->updateLockStatus();
}

void QCamera::unlock()
{
   unlock(d_func()->requestedLocks);
}

void QCamera::_q_preparePropertyChange(int changeType)
{
   Q_D(QCamera);
   d->_q_preparePropertyChange(changeType);
}

void QCamera::_q_restartCamera()
{
   Q_D(QCamera);
   d->_q_restartCamera();
}

void QCamera::_q_error(int error, const QString &errorString)
{
   Q_D(QCamera);
   d->_q_error(error, errorString);
}

void QCamera::_q_updateLockStatus(QCamera::LockType type, QCamera::LockStatus status,
      QCamera::LockChangeReason reason)
{
   Q_D(QCamera);
   d->_q_updateLockStatus(type, status, reason);
}

void QCamera::_q_updateState(QCamera::State state)
{
   Q_D(QCamera);
   d->_q_updateState(state);
}

