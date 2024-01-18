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

#ifndef QCAMERAIMAGEPROCESSING_H
#define QCAMERAIMAGEPROCESSING_H

#include <qstringlist.h>
#include <qpair.h>
#include <qsize.h>
#include <qpoint.h>
#include <qrect.h>

#include <qmediacontrol.h>
#include <qmediaobject.h>
#include <qmediaservice.h>

class QCamera;
class QCameraImageProcessingPrivate;

class Q_MULTIMEDIA_EXPORT QCameraImageProcessing : public QObject
{
   MULTI_CS_OBJECT(QCameraImageProcessing)

   MULTI_CS_ENUM(WhiteBalanceMode)
   MULTI_CS_ENUM(ColorFilter)

 public:
   enum WhiteBalanceMode {
      WhiteBalanceAuto = 0,
      WhiteBalanceManual = 1,
      WhiteBalanceSunlight = 2,
      WhiteBalanceCloudy = 3,
      WhiteBalanceShade = 4,
      WhiteBalanceTungsten = 5,
      WhiteBalanceFluorescent = 6,
      WhiteBalanceFlash = 7,
      WhiteBalanceSunset = 8,
      WhiteBalanceVendor = 1000
   };

   enum ColorFilter {
      ColorFilterNone,
      ColorFilterGrayscale,
      ColorFilterNegative,
      ColorFilterSolarize,
      ColorFilterSepia,
      ColorFilterPosterize,
      ColorFilterWhiteboard,
      ColorFilterBlackboard,
      ColorFilterAqua,
      ColorFilterVendor = 1000
   };

   QCameraImageProcessing(const QCameraImageProcessing &) = delete;
   QCameraImageProcessing &operator=(const QCameraImageProcessing &) = delete;

   bool isAvailable() const;

   WhiteBalanceMode whiteBalanceMode() const;
   void setWhiteBalanceMode(WhiteBalanceMode mode);
   bool isWhiteBalanceModeSupported(WhiteBalanceMode mode) const;

   qreal manualWhiteBalance() const;
   void setManualWhiteBalance(qreal colorTemperature);

   qreal contrast() const;
   void setContrast(qreal value);

   qreal saturation() const;
   void setSaturation(qreal value);

   qreal sharpeningLevel() const;
   void setSharpeningLevel(qreal level);

   qreal denoisingLevel() const;
   void setDenoisingLevel(qreal level);

   ColorFilter colorFilter() const;
   void setColorFilter(ColorFilter filter);
   bool isColorFilterSupported(ColorFilter filter) const;

 private:
   QCameraImageProcessing(QCamera *camera);
   ~QCameraImageProcessing();

   Q_DECLARE_PRIVATE(QCameraImageProcessing)

   friend class QCamera;
   friend class QCameraPrivate;

   QCameraImageProcessingPrivate *d_ptr;
};

#endif
