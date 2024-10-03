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

#include <qhighdpiscaling_p.h>

#include <qapplication.h>
#include <qdebug.h>
#include <qscreen.h>
#include <qplatform_integration.h>

#include <qscreen_p.h>

static const char legacyDevicePixelEnvVar[] = "QT_DEVICE_PIXEL_RATIO";
static const char scaleFactorEnvVar[]       = "QT_SCALE_FACTOR";
static const char autoScreenEnvVar[]        = "QT_AUTO_SCREEN_SCALE_FACTOR";
static const char screenFactorsEnvVar[]     = "QT_SCREEN_SCALE_FACTORS";

static inline qreal initialGlobalScaleFactor()
{
   qreal result     = 1;
   QByteArray env_1 = qgetenv(scaleFactorEnvVar);

   if (! env_1.isEmpty()) {
      bool ok;

      const qreal f = env_1.toDouble(&ok);

      if (ok && f > 0) {

#if defined(CS_SHOW_DEBUG_GUI_DPI)
         qDebug() << "QHighDpiScaling initialGlobalScaleFactor() Factor =" << scaleFactorEnvVar << f;
#endif

         result = f;
      }

   } else {
      QByteArray env_2 = qgetenv(legacyDevicePixelEnvVar);

      if (! env_2.isEmpty()) {
         qWarning() << "Warning:" << legacyDevicePixelEnvVar << "is deprecated. Instead use: " << endl
            << "   " << autoScreenEnvVar << "to enable platform plugin controlled per-screen factors." << endl
            << "   " << screenFactorsEnvVar << "to set per-screen factors." << endl
            << "   " << scaleFactorEnvVar << "to set the application global scale factor.";

         int dpr = env_2.toInt();

         if (dpr > 0) {
            result = dpr;
         }
      }
   }

   return result;
}

qreal QHighDpiScaling::m_factor                   = 1.0;
bool QHighDpiScaling::m_active                    = false;   // "overall active" - is there any scale factor set.
bool QHighDpiScaling::m_usePixelDensity           = false;   // use scale factor from platform plugin
bool QHighDpiScaling::m_pixelDensityScalingActive = false;   // pixel density scale factor > 1
bool QHighDpiScaling::m_globalScalingActive       = false;   // global scale factor is active
bool QHighDpiScaling::m_screenFactorSet           = false;   // QHighDpiScaling::setScreenFactor has been used

QDpi QHighDpiScaling::m_logicalDpi = QDpi(-1, -1);           // scaled logical DPI of the primary screen

static inline bool usePixelDensity()
{
   // call before the platform plugin is created.

   // Determine if we should set a scale factor based on the pixel density
   // reported by the platform plugin. There are several enablers and several
   // disablers. A single disable may veto all other enablers.

   if (QCoreApplication::testAttribute(Qt::AA_DisableHighDpiScaling)) {
      return false;
   }

   bool screenEnvValueOk;
   const int screenEnvValue = qgetenv(autoScreenEnvVar).toInt(&screenEnvValueOk);

   if (screenEnvValueOk && screenEnvValue < 1) {
      return false;
   }

   return QCoreApplication::testAttribute(Qt::AA_EnableHighDpiScaling)
      || (screenEnvValueOk && screenEnvValue > 0)
      || (qgetenv(legacyDevicePixelEnvVar).toLower() == "auto");
}

void QHighDpiScaling::initHighDpiScaling()
{
   // Determine if there is a global scale factor set.
   m_factor = initialGlobalScaleFactor();
   m_globalScalingActive = !qFuzzyCompare(m_factor, qreal(1));

   m_usePixelDensity = usePixelDensity();

   m_pixelDensityScalingActive = false; //set in updateHighDpiScaling below

   // we update m_active in updateHighDpiScaling, but while we create the
   // screens, we have to assume that m_usePixelDensity implies scaling
   m_active = m_globalScalingActive || m_usePixelDensity;
}

void QHighDpiScaling::updateHighDpiScaling()
{
   if (QCoreApplication::testAttribute(Qt::AA_DisableHighDpiScaling)) {
      return;
   }

   if (m_usePixelDensity && !m_pixelDensityScalingActive) {
      for (QScreen *screen : QApplication::screens()) {
         if (!qFuzzyCompare(screenSubfactor(screen->handle()), qreal(1))) {
            m_pixelDensityScalingActive = true;
            break;
         }
      }
   }

   QByteArray env = qgetenv(screenFactorsEnvVar);

   if (! env.isEmpty()) {
      int i = 0;

      for (const QByteArray &spec : env.split(';')) {
         QScreen *screen = nullptr;
         int equalsPos = spec.lastIndexOf('=');
         double factor = 0;

         if (equalsPos > 0) {
            // support "name=factor"
            QByteArray name = spec.mid(0, equalsPos);
            QByteArray f = spec.mid(equalsPos + 1);
            bool ok;
            factor = f.toDouble(&ok);

            if (ok) {
               for (QScreen *s : QApplication::screens()) {
                  if (s->name() == QString::fromUtf8(name)) {
                     screen = s;
                     break;
                  }
               }
            }
         } else {
            // listing screens in order
            bool ok;
            factor = spec.toDouble(&ok);
            if (ok && i < QApplication::screens().count()) {
               screen = QApplication::screens().at(i);
            }
         }
         if (screen) {
            setScreenFactor(screen, factor);
         }
         ++i;
      }
   }
   m_active = m_globalScalingActive || m_screenFactorSet || m_pixelDensityScalingActive;

   QPlatformScreen *primaryScreen = QApplication::primaryScreen()->handle();
   qreal sf = screenSubfactor(primaryScreen);

   QDpi primaryDpi = primaryScreen->logicalDpi();
   m_logicalDpi = QDpi(primaryDpi.first / sf, primaryDpi.second / sf);
}

/*
    Sets the global scale factor which is applied to all windows.
*/
void QHighDpiScaling::setGlobalFactor(qreal factor)
{
   if (qFuzzyCompare(factor, m_factor)) {
      return;
   }

   if (! QApplication::allWindows().isEmpty()) {
      qWarning("QHighDpiScaling::setFactor() Only call this methdod when no windows exist");
   }

   m_globalScalingActive = ! qFuzzyCompare(factor, qreal(1));
   m_factor = m_globalScalingActive ? factor : qreal(1);
   m_active = m_globalScalingActive || m_screenFactorSet || m_pixelDensityScalingActive;

   for (QScreen *screen : QApplication::screens()) {
      screen->d_func()->updateHighDpi();
   }
}

static const char scaleFactorProperty[] = "_q_scaleFactor";

/*
    Sets a per-screen scale factor.
*/
void QHighDpiScaling::setScreenFactor(QScreen *screen, qreal factor)
{
   if (!qFuzzyCompare(factor, qreal(1))) {
      m_screenFactorSet = true;
      m_active = true;
   }
   screen->setProperty(scaleFactorProperty, QVariant(factor));

   // hack to force re-evaluation of screen geometry
   if (screen->handle()) {
      screen->d_func()->setPlatformScreen(screen->handle());   // updates geometries based on scale factor
   }
}

QPoint QHighDpiScaling::mapPositionToNative(const QPoint &pos, const QPlatformScreen *platformScreen)
{
   if (!platformScreen) {
      return pos;
   }

   const qreal scaleFactor = factor(platformScreen);
   const QPoint topLeft    = platformScreen->geometry().topLeft();
   return (pos - topLeft) * scaleFactor + topLeft;
}

QPoint QHighDpiScaling::mapPositionFromNative(const QPoint &pos, const QPlatformScreen *platformScreen)
{
   if (!platformScreen) {
      return pos;
   }

   const qreal scaleFactor = factor(platformScreen);
   const QPoint topLeft    = platformScreen->geometry().topLeft();
   return (pos - topLeft) / scaleFactor + topLeft;
}

qreal QHighDpiScaling::screenSubfactor(const QPlatformScreen *screen)
{
   qreal factor = qreal(1.0);

   if (screen) {
      if (m_usePixelDensity) {
         factor *= screen->pixelDensity();
      }
      if (m_screenFactorSet) {
         QVariant screenFactor = screen->screen()->property(scaleFactorProperty);
         if (screenFactor.isValid()) {
            factor *= screenFactor.toReal();
         }
      }
   }
   return factor;
}

QDpi QHighDpiScaling::logicalDpi()
{
   return m_logicalDpi;
}

qreal QHighDpiScaling::factor(const QScreen *screen)
{
   // Fast path for when scaling is not used at all
   if (! m_active) {
      return qreal(1.0);
   }

   // The effective factor for a given screen is the product of the
   // screen and global sub-factors
   qreal factor = m_factor;

   if (screen) {
      factor *= screenSubfactor(screen->handle());
   }
   return factor;
}

qreal QHighDpiScaling::factor(const QPlatformScreen *platformScreen)
{
   if (! m_active) {
      return qreal(1.0);
   }

   return m_factor * screenSubfactor(platformScreen);
}

qreal QHighDpiScaling::factor(const QWindow *window)
{
   if (!m_active) {
      return qreal(1.0);
   }

   return factor(window ? window->screen() : QApplication::primaryScreen());
}

QPoint QHighDpiScaling::origin(const QScreen *screen)
{
   return screen->geometry().topLeft();
}

QPoint QHighDpiScaling::origin(const QPlatformScreen *platformScreen)
{
   return platformScreen->geometry().topLeft();
}

