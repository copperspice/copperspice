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

#ifndef QCAMERAFOCUS_H
#define QCAMERAFOCUS_H

#include <qstringlist.h>
#include <qpair.h>
#include <qsize.h>
#include <qpoint.h>
#include <qrect.h>
#include <qshareddata.h>
#include <qmediaobject.h>

class QCamera;
class QCameraFocusZoneData;
class QCameraFocusPrivate;

class Q_MULTIMEDIA_EXPORT QCameraFocusZone
{
 public:
   enum FocusZoneStatus {
      Invalid,
      Unused,
      Selected,
      Focused
   };

   QCameraFocusZone();
   QCameraFocusZone(const QRectF &area, FocusZoneStatus status = Selected);
   QCameraFocusZone(const QCameraFocusZone &other);

   QCameraFocusZone &operator=(const QCameraFocusZone &other);
   bool operator==(const QCameraFocusZone &other) const;
   bool operator!=(const QCameraFocusZone &other) const;

   ~QCameraFocusZone();

   bool isValid() const;

   QRectF area() const;

   FocusZoneStatus status() const;
   void setStatus(FocusZoneStatus status);

 private:
   QSharedDataPointer<QCameraFocusZoneData> d;
};

using QCameraFocusZoneList = QList<QCameraFocusZone>;

class Q_MULTIMEDIA_EXPORT QCameraFocus : public QObject
{
   MULTI_CS_OBJECT(QCameraFocus)

   MULTI_CS_PROPERTY_READ(focusMode, focusMode)
   MULTI_CS_PROPERTY_WRITE(focusMode, setFocusMode)

   MULTI_CS_PROPERTY_READ(focusPointMode, focusPointMode)
   MULTI_CS_PROPERTY_WRITE(focusPointMode, setFocusPointMode)

   MULTI_CS_PROPERTY_READ(customFocusPoint, customFocusPoint)
   MULTI_CS_PROPERTY_WRITE(customFocusPoint, setCustomFocusPoint)

   MULTI_CS_PROPERTY_READ(focusZones, focusZones)
   MULTI_CS_PROPERTY_NOTIFY(focusZones, focusZonesChanged)

   MULTI_CS_PROPERTY_READ(opticalZoom, opticalZoom)
   MULTI_CS_PROPERTY_NOTIFY(opticalZoom, opticalZoomChanged)

   MULTI_CS_PROPERTY_READ(digitalZoom, digitalZoom)
   MULTI_CS_PROPERTY_NOTIFY(digitalZoom, digitalZoomChanged)

   MULTI_CS_ENUM(FocusMode)
   MULTI_CS_ENUM(FocusPointMode)

 public:
   enum FocusMode {
      ManualFocus     = 0x1,
      HyperfocalFocus = 0x02,
      InfinityFocus   = 0x04,
      AutoFocus       = 0x8,
      ContinuousFocus = 0x10,
      MacroFocus      = 0x20
   };
   using FocusModes = QFlags<FocusMode>;

   enum FocusPointMode {
      FocusPointAuto,
      FocusPointCenter,
      FocusPointFaceDetection,
      FocusPointCustom
   };

   QCameraFocus(const QCameraFocus &) = delete;
   QCameraFocus &operator=(const QCameraFocus &) = delete;

   bool isAvailable() const;

   FocusModes focusMode() const;
   void setFocusMode(FocusModes mode);
   bool isFocusModeSupported(FocusModes mode) const;

   QCameraFocus::FocusPointMode focusPointMode() const;
   void setFocusPointMode(QCameraFocus::FocusPointMode mode);
   bool isFocusPointModeSupported(QCameraFocus::FocusPointMode mode) const;
   QPointF customFocusPoint() const;
   void setCustomFocusPoint(const QPointF &point);

   QCameraFocusZoneList focusZones() const;

   double maximumOpticalZoom() const;
   double maximumDigitalZoom() const;
   double opticalZoom() const;
   double digitalZoom() const;

   void zoomTo(double opticalZoom, double digitalZoom);

   MULTI_CS_SIGNAL_1(Public, void opticalZoomChanged(double value))
   MULTI_CS_SIGNAL_2(opticalZoomChanged, value)
   MULTI_CS_SIGNAL_1(Public, void digitalZoomChanged(double value))
   MULTI_CS_SIGNAL_2(digitalZoomChanged, value)

   MULTI_CS_SIGNAL_1(Public, void focusZonesChanged())
   MULTI_CS_SIGNAL_2(focusZonesChanged)

   MULTI_CS_SIGNAL_1(Public, void maximumOpticalZoomChanged(double value))
   MULTI_CS_SIGNAL_2(maximumOpticalZoomChanged, value)
   MULTI_CS_SIGNAL_1(Public, void maximumDigitalZoomChanged(double value))
   MULTI_CS_SIGNAL_2(maximumDigitalZoomChanged, value)

 private:
   QCameraFocus(QCamera *camera);
   ~QCameraFocus();

   Q_DECLARE_PRIVATE(QCameraFocus)

   QCameraFocusPrivate *d_ptr;

   friend class QCamera;
   friend class QCameraPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCameraFocus::FocusModes)

CS_DECLARE_METATYPE(QCameraFocusZone)

#endif
