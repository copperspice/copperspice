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

#include <qcamerafocus.h>

#include <qcamera.h>
#include <qcameracontrol.h>
#include <qcameraexposurecontrol.h>
#include <qcamerafocuscontrol.h>
#include <qcamerazoomcontrol.h>
#include <qdebug.h>
#include <qmediarecordercontrol.h>
#include <qcameraimagecapturecontrol.h>
#include <qvideodeviceselectorcontrol.h>

#include <qmediaobject_p.h>

class QCameraFocusFakeZoomControl : public QCameraZoomControl
{
 public:
   QCameraFocusFakeZoomControl(QObject *parent)
      : QCameraZoomControl(parent) {}

   qreal maximumOpticalZoom() const override {
      return 1.0;
   }

   qreal maximumDigitalZoom() const override {
      return 1.0;
   }

   qreal requestedOpticalZoom() const override {
      return 1.0;
   }

   qreal requestedDigitalZoom() const override {
      return 1.0;
   }

   qreal currentOpticalZoom() const override {
      return 1.0;
   }

   qreal currentDigitalZoom() const override {
      return 1.0;
   }

   void zoomTo(qreal, qreal) override {
      qWarning("This camera does not support zooming");
   }
};

class QCameraFocusFakeFocusControl : public QCameraFocusControl
{
 public:
   QCameraFocusFakeFocusControl(QObject *parent)
      : QCameraFocusControl(parent)
   { }

   QCameraFocus::FocusModes focusMode() const override {
      return QCameraFocus::AutoFocus;
   }

   void setFocusMode(QCameraFocus::FocusModes) override {
      qWarning("Focus mode selection is not supported");
   }

   bool isFocusModeSupported(QCameraFocus::FocusModes) const override {
      return false;
   }

   QCameraFocus::FocusPointMode focusPointMode() const override {
      return QCameraFocus::FocusPointAuto;
   }

   void setFocusPointMode(QCameraFocus::FocusPointMode) override {
      qWarning("Focus points mode selection is not supported");
   }

   bool isFocusPointModeSupported(QCameraFocus::FocusPointMode) const override {
      return false;
   }

   QPointF customFocusPoint() const override {
      return QPointF(0.5, 0.5);
   }

   void setCustomFocusPoint(const QPointF &) override {
      qWarning("Focus points selection is not supported");
   }

   QCameraFocusZoneList focusZones() const override {
      return QCameraFocusZoneList();
   }
};

class QCameraFocusZoneData : public QSharedData
{
 public:
   QCameraFocusZoneData()
      : status(QCameraFocusZone::Invalid)
   {
   }

   QCameraFocusZoneData(const QRectF &_area, QCameraFocusZone::FocusZoneStatus _status)
      : area(_area), status(_status)
   {
   }

   QCameraFocusZoneData(const QCameraFocusZoneData &other)
      : QSharedData(other), area(other.area), status(other.status) {
   }

   QCameraFocusZoneData &operator=(const QCameraFocusZoneData &other) {
      area = other.area;
      status = other.status;
      return *this;
   }

   QRectF area;
   QCameraFocusZone::FocusZoneStatus status;
};

QCameraFocusZone::QCameraFocusZone()
   : d(new QCameraFocusZoneData)
{
}

/*!
 * \internal
 * Creates a new QCameraFocusZone with the supplied \a area and \a status.
 */
QCameraFocusZone::QCameraFocusZone(const QRectF &area, QCameraFocusZone::FocusZoneStatus status)
   : d(new QCameraFocusZoneData(area, status))
{
}

QCameraFocusZone::QCameraFocusZone(const QCameraFocusZone &other)
   : d(other.d)
{
}

QCameraFocusZone::~QCameraFocusZone()
{
}

QCameraFocusZone &QCameraFocusZone::operator=(const QCameraFocusZone &other)
{
   d = other.d;
   return *this;
}

/*!
 * Returns true if this focus zone is the same as \a other.
 */
bool QCameraFocusZone::operator==(const QCameraFocusZone &other) const
{
   return d == other.d ||
          (d->area == other.d->area && d->status == other.d->status);
}

/*!
 * Returns true if this focus zone is not the same as \a other.
 */
bool QCameraFocusZone::operator!=(const QCameraFocusZone &other) const
{
   return !(*this == other);
}

/*!
 * Returns true if this focus zone has a valid area and status.
 */
bool QCameraFocusZone::isValid() const
{
   return d->status != Invalid && !d->area.isValid();
}

/*!
 * Returns the area of the camera frame that this focus zone encompasses.
 *
 * Coordinates are in frame relative coordinates - \c QPointF(0,0) is the top
 * left of the frame, and \c QPointF(1,1) is the bottom right.
 */
QRectF QCameraFocusZone::area() const
{
   return d->area;
}

/*!
 * Returns the current status of this focus zone.
 */
QCameraFocusZone::FocusZoneStatus QCameraFocusZone::status() const
{
   return d->status;
}

/*!
 * \internal
 * Sets the current status of this focus zone to \a status.
 */
void QCameraFocusZone::setStatus(QCameraFocusZone::FocusZoneStatus status)
{
   d->status = status;
}

class QCameraFocusPrivate : public QMediaObjectPrivate
{
   Q_DECLARE_NON_CONST_PUBLIC(QCameraFocus)

 public:
   void initControls();

   QCameraFocus *q_ptr;

   QCamera *camera;

   QCameraFocusControl *focusControl;
   QCameraZoomControl *zoomControl;
   bool available;
};

void QCameraFocusPrivate::initControls()
{
   Q_Q(QCameraFocus);

   focusControl = nullptr;
   zoomControl  = nullptr;

   QMediaService *service = camera->service();

   if (service) {
      focusControl = dynamic_cast<QCameraFocusControl *>(service->requestControl(QCameraFocusControl_iid));
      zoomControl  = dynamic_cast<QCameraZoomControl *>(service->requestControl(QCameraZoomControl_iid));
   }

   available = (focusControl != nullptr);

   if (! focusControl) {
      focusControl = new QCameraFocusFakeFocusControl(q);
   }

   if (! zoomControl) {
      zoomControl = new QCameraFocusFakeZoomControl(q);
   }

   q->connect(focusControl, &QCameraFocusControl::focusZonesChanged,        q, &QCameraFocus::focusZonesChanged);

   q->connect(zoomControl,  &QCameraZoomControl::currentOpticalZoomChanged, q, &QCameraFocus::opticalZoomChanged);
   q->connect(zoomControl,  &QCameraZoomControl::currentDigitalZoomChanged, q, &QCameraFocus::digitalZoomChanged);
   q->connect(zoomControl,  &QCameraZoomControl::maximumOpticalZoomChanged, q, &QCameraFocus::maximumOpticalZoomChanged);
   q->connect(zoomControl,  &QCameraZoomControl::maximumDigitalZoomChanged, q, &QCameraFocus::maximumDigitalZoomChanged);
}

// internal
QCameraFocus::QCameraFocus(QCamera *camera)
   : QObject(camera), d_ptr(new QCameraFocusPrivate)
{
   Q_D(QCameraFocus);
   d->camera = camera;
   d->q_ptr = this;
   d->initControls();
}

QCameraFocus::~QCameraFocus()
{
   delete d_ptr;
}

bool QCameraFocus::isAvailable() const
{
   return d_func()->available;
}

QCameraFocus::FocusModes QCameraFocus::focusMode() const
{
   return d_func()->focusControl->focusMode();
}

void QCameraFocus::setFocusMode(QCameraFocus::FocusModes mode)
{
   d_func()->focusControl->setFocusMode(mode);
}

bool QCameraFocus::isFocusModeSupported(FocusModes mode) const
{
   return d_func()->focusControl->isFocusModeSupported(mode);
}

QCameraFocus::FocusPointMode QCameraFocus::focusPointMode() const
{
   return d_func()->focusControl->focusPointMode();
}

void QCameraFocus::setFocusPointMode(QCameraFocus::FocusPointMode mode)
{
   d_func()->focusControl->setFocusPointMode(mode);
}

bool QCameraFocus::isFocusPointModeSupported(QCameraFocus::FocusPointMode mode) const
{
   return d_func()->focusControl->isFocusPointModeSupported(mode);
}

QPointF QCameraFocus::customFocusPoint() const
{
   return d_func()->focusControl->customFocusPoint();
}

void QCameraFocus::setCustomFocusPoint(const QPointF &point)
{
   d_func()->focusControl->setCustomFocusPoint(point);
}

QCameraFocusZoneList QCameraFocus::focusZones() const
{
   return d_func()->focusControl->focusZones();
}

qreal QCameraFocus::maximumOpticalZoom() const
{
   return d_func()->zoomControl->maximumOpticalZoom();
}

qreal QCameraFocus::maximumDigitalZoom() const
{
   return d_func()->zoomControl->maximumDigitalZoom();
}

qreal QCameraFocus::opticalZoom() const
{
   return d_func()->zoomControl->currentOpticalZoom();
}

qreal QCameraFocus::digitalZoom() const
{
   return d_func()->zoomControl->currentDigitalZoom();
}

void QCameraFocus::zoomTo(qreal optical, qreal digital)
{
   d_func()->zoomControl->zoomTo(optical, digital);
}
