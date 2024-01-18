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

#include <qvideowidget_p.h>

#include <qapplication.h>
#include <qboxlayout.h>
#include <qdialog.h>
#include <qevent.h>
#include <qmediaobject.h>
#include <qmediaservice.h>
#include <qnamespace.h>
#include <qpainter.h>
#include <qvideowindowcontrol.h>
#include <qvideowidgetcontrol.h>
#include <qvideorenderercontrol.h>
#include <qvideosurfaceformat.h>

#include <qpaintervideosurface_p.h>

QVideoWidgetControlBackend::QVideoWidgetControlBackend(QMediaService *service,
      QVideoWidgetControl *control, QVideoWidget *widget)
   : m_service(service), m_widgetControl(control)
{
   connect(control, &QVideoWidgetControl::brightnessChanged,  widget, &QVideoWidget::_q_brightnessChanged);
   connect(control, &QVideoWidgetControl::contrastChanged,    widget, &QVideoWidget::_q_contrastChanged);
   connect(control, &QVideoWidgetControl::hueChanged,         widget, &QVideoWidget::_q_hueChanged);
   connect(control, &QVideoWidgetControl::saturationChanged,  widget, &QVideoWidget::_q_saturationChanged);
   connect(control, &QVideoWidgetControl::fullScreenChanged,  widget, &QVideoWidget::_q_fullScreenChanged);

   QBoxLayout *layout = new QVBoxLayout;
   layout->setMargin(0);
   layout->setSpacing(0);

   layout->addWidget(control->videoWidget());

   widget->setLayout(layout);
}

void QVideoWidgetControlBackend::releaseControl()
{
   m_service->releaseControl(m_widgetControl);
}

void QVideoWidgetControlBackend::setBrightness(int brightness)
{
   m_widgetControl->setBrightness(brightness);
}

void QVideoWidgetControlBackend::setContrast(int contrast)
{
   m_widgetControl->setContrast(contrast);
}

void QVideoWidgetControlBackend::setHue(int hue)
{
   m_widgetControl->setHue(hue);
}

void QVideoWidgetControlBackend::setSaturation(int saturation)
{
   m_widgetControl->setSaturation(saturation);
}

void QVideoWidgetControlBackend::setFullScreen(bool fullScreen)
{
   m_widgetControl->setFullScreen(fullScreen);
}

Qt::AspectRatioMode QVideoWidgetControlBackend::aspectRatioMode() const
{
   return m_widgetControl->aspectRatioMode();
}

void QVideoWidgetControlBackend::setAspectRatioMode(Qt::AspectRatioMode mode)
{
   m_widgetControl->setAspectRatioMode(mode);
}

QRendererVideoWidgetBackend::QRendererVideoWidgetBackend(QMediaService *service,
      QVideoRendererControl *control, QVideoWidget *widget)
   : m_service(service), m_rendererControl(control), m_widget(widget), m_surface(new QPainterVideoSurface),
     m_aspectRatioMode(Qt::KeepAspectRatio), m_updatePaintDevice(true)
{
   connect(this, &QRendererVideoWidgetBackend::brightnessChanged, m_widget, &QVideoWidget::_q_brightnessChanged);
   connect(this, &QRendererVideoWidgetBackend::contrastChanged,   m_widget, &QVideoWidget::_q_contrastChanged);
   connect(this, &QRendererVideoWidgetBackend::hueChanged,        m_widget, &QVideoWidget::_q_hueChanged);
   connect(this, &QRendererVideoWidgetBackend::saturationChanged, m_widget, &QVideoWidget::_q_saturationChanged);

   connect(m_surface, &QPainterVideoSurface::frameChanged,         this, &QRendererVideoWidgetBackend::frameChanged);
   connect(m_surface, &QPainterVideoSurface::surfaceFormatChanged, this, &QRendererVideoWidgetBackend::formatChanged);

   m_rendererControl->setSurface(m_surface);
}

QRendererVideoWidgetBackend::~QRendererVideoWidgetBackend()
{
   delete m_surface;
}

void QRendererVideoWidgetBackend::releaseControl()
{
   m_service->releaseControl(m_rendererControl);
}

void QRendererVideoWidgetBackend::clearSurface()
{
   m_rendererControl->setSurface(nullptr);
}

void QRendererVideoWidgetBackend::setBrightness(int brightness)
{
   m_surface->setBrightness(brightness);

   emit brightnessChanged(brightness);
}

void QRendererVideoWidgetBackend::setContrast(int contrast)
{
   m_surface->setContrast(contrast);

   emit contrastChanged(contrast);
}

void QRendererVideoWidgetBackend::setHue(int hue)
{
   m_surface->setHue(hue);

   emit hueChanged(hue);
}

void QRendererVideoWidgetBackend::setSaturation(int saturation)
{
   m_surface->setSaturation(saturation);

   emit saturationChanged(saturation);
}

Qt::AspectRatioMode QRendererVideoWidgetBackend::aspectRatioMode() const
{
   return m_aspectRatioMode;
}

void QRendererVideoWidgetBackend::setAspectRatioMode(Qt::AspectRatioMode mode)
{
   m_aspectRatioMode = mode;

   m_widget->updateGeometry();
}

void QRendererVideoWidgetBackend::setFullScreen(bool)
{
}

QSize QRendererVideoWidgetBackend::sizeHint() const
{
   return m_surface->surfaceFormat().sizeHint();
}

void QRendererVideoWidgetBackend::showEvent()
{
}

void QRendererVideoWidgetBackend::hideEvent(QHideEvent *)
{
#if ! defined(QT_NO_OPENGL) && ! defined(QT_OPENGL_ES_1_CL) && ! defined(QT_OPENGL_ES_1)
   m_updatePaintDevice = true;
   m_surface->setGLContext(nullptr);
#endif
}

void QRendererVideoWidgetBackend::resizeEvent(QResizeEvent *)
{
   updateRects();
}

void QRendererVideoWidgetBackend::moveEvent(QMoveEvent *)
{
}

void QRendererVideoWidgetBackend::paintEvent(QPaintEvent *event)
{
   QPainter painter(m_widget);

   if (m_widget->testAttribute(Qt::WA_OpaquePaintEvent)) {
      QRegion borderRegion = event->region();
      borderRegion = borderRegion.subtracted(m_boundingRect);

      QBrush brush = m_widget->palette().window();

      QVector<QRect> rects = borderRegion.rects();
      for (QVector<QRect>::iterator it = rects.begin(), end = rects.end(); it != end; ++it) {
         painter.fillRect(*it, brush);
      }
   }

   if (m_surface->isActive() && m_boundingRect.intersects(event->rect())) {
      m_surface->paint(&painter, m_boundingRect, m_sourceRect);

      m_surface->setReady(true);

   } else {

#if ! defined(QT_NO_OPENGL) && ! defined(QT_OPENGL_ES_1_CL) && ! defined(QT_OPENGL_ES_1)
      if (m_updatePaintDevice && (painter.paintEngine()->type() == QPaintEngine::OpenGL
            || painter.paintEngine()->type() == QPaintEngine::OpenGL2)) {
         m_updatePaintDevice = false;

         m_surface->setGLContext(const_cast<QGLContext *>(QGLContext::currentContext()));

         if (m_surface->supportedShaderTypes() & QPainterVideoSurface::GlslShader) {
            m_surface->setShaderType(QPainterVideoSurface::GlslShader);
         } else {
            m_surface->setShaderType(QPainterVideoSurface::FragmentProgramShader);
         }
      }
#endif
   }
}

void QRendererVideoWidgetBackend::formatChanged(const QVideoSurfaceFormat &format)
{
   m_nativeSize = format.sizeHint();

   updateRects();

   m_widget->updateGeometry();
   m_widget->update();
}

void QRendererVideoWidgetBackend::frameChanged()
{
   m_widget->update(m_boundingRect);
}

void QRendererVideoWidgetBackend::updateRects()
{
   QRect rect = m_widget->rect();

   if (m_nativeSize.isEmpty()) {
      m_boundingRect = QRect();

   } else if (m_aspectRatioMode == Qt::IgnoreAspectRatio) {
      m_boundingRect = rect;
      m_sourceRect = QRectF(0, 0, 1, 1);

   } else if (m_aspectRatioMode == Qt::KeepAspectRatio) {
      QSize size = m_nativeSize;
      size.scale(rect.size(), Qt::KeepAspectRatio);

      m_boundingRect = QRect(0, 0, size.width(), size.height());
      m_boundingRect.moveCenter(rect.center());

      m_sourceRect = QRectF(0, 0, 1, 1);

   } else if (m_aspectRatioMode == Qt::KeepAspectRatioByExpanding) {
      m_boundingRect = rect;

      QSizeF size = rect.size();
      size.scale(m_nativeSize, Qt::KeepAspectRatio);

      m_sourceRect = QRectF(0, 0, size.width() / m_nativeSize.width(), size.height() / m_nativeSize.height());
      m_sourceRect.moveCenter(QPointF(0.5, 0.5));
   }
}

QWindowVideoWidgetBackend::QWindowVideoWidgetBackend(QMediaService *service,
      QVideoWindowControl *control, QVideoWidget *widget)
   : m_service(service), m_windowControl(control), m_widget(widget)
{
   connect(control, &QVideoWindowControl::brightnessChanged,  m_widget, &QVideoWidget::_q_brightnessChanged);
   connect(control, &QVideoWindowControl::contrastChanged,    m_widget, &QVideoWidget::_q_contrastChanged);
   connect(control, &QVideoWindowControl::hueChanged,         m_widget, &QVideoWidget::_q_hueChanged);
   connect(control, &QVideoWindowControl::saturationChanged,  m_widget, &QVideoWidget::_q_saturationChanged);
   connect(control, &QVideoWindowControl::fullScreenChanged,  m_widget, &QVideoWidget::_q_fullScreenChanged);
   connect(control, &QVideoWindowControl::nativeSizeChanged,  m_widget, &QVideoWidget::_q_dimensionsChanged);

   control->setWinId(widget->winId());

#ifdef Q_OS_WIN
   m_widget->setUpdatesEnabled(false);
#endif

}

QWindowVideoWidgetBackend::~QWindowVideoWidgetBackend()
{
}

void QWindowVideoWidgetBackend::releaseControl()
{
   m_service->releaseControl(m_windowControl);
}

void QWindowVideoWidgetBackend::setBrightness(int brightness)
{
   m_windowControl->setBrightness(brightness);
}

void QWindowVideoWidgetBackend::setContrast(int contrast)
{
   m_windowControl->setContrast(contrast);
}

void QWindowVideoWidgetBackend::setHue(int hue)
{
   m_windowControl->setHue(hue);
}

void QWindowVideoWidgetBackend::setSaturation(int saturation)
{
   m_windowControl->setSaturation(saturation);
}

void QWindowVideoWidgetBackend::setFullScreen(bool fullScreen)
{
   m_windowControl->setFullScreen(fullScreen);
}

Qt::AspectRatioMode QWindowVideoWidgetBackend::aspectRatioMode() const
{
   return m_windowControl->aspectRatioMode();
}

void QWindowVideoWidgetBackend::setAspectRatioMode(Qt::AspectRatioMode mode)
{
   m_windowControl->setAspectRatioMode(mode);
}

QSize QWindowVideoWidgetBackend::sizeHint() const
{
   return m_windowControl->nativeSize();
}

void QWindowVideoWidgetBackend::showEvent()
{
   m_windowControl->setWinId(m_widget->winId());
   m_windowControl->setDisplayRect(m_widget->rect());

#if defined(Q_OS_WIN)
   m_windowControl->repaint();
#endif
}

void QWindowVideoWidgetBackend::hideEvent(QHideEvent *)
{
}

void QWindowVideoWidgetBackend::moveEvent(QMoveEvent *)
{
   m_windowControl->setDisplayRect(m_widget->rect());
}

void QWindowVideoWidgetBackend::resizeEvent(QResizeEvent *)
{
   m_windowControl->setDisplayRect(m_widget->rect());
}

void QWindowVideoWidgetBackend::paintEvent(QPaintEvent *event)
{
   if (m_widget->testAttribute(Qt::WA_OpaquePaintEvent)) {
      QPainter painter(m_widget);

      painter.fillRect(event->rect(), m_widget->palette().window());
   }

   m_windowControl->repaint();

   event->accept();
}

void QVideoWidgetPrivate::setCurrentControl(QVideoWidgetControlInterface *control)
{
   if (currentControl != control) {
      currentControl = control;

      currentControl->setBrightness(brightness);
      currentControl->setContrast(contrast);
      currentControl->setHue(hue);
      currentControl->setSaturation(saturation);
      currentControl->setAspectRatioMode(aspectRatioMode);
   }
}

void QVideoWidgetPrivate::clearService()
{
   if (service) {
      QObject::disconnect(service, &QMediaService::destroyed, q_func(), &QVideoWidget::_q_serviceDestroyed);

      if (widgetBackend) {
         QLayout *layout = q_func()->layout();

         for (QLayoutItem *item = layout->takeAt(0); item; item = layout->takeAt(0)) {
            item->widget()->setParent(nullptr);
            delete item;
         }
         delete layout;

         widgetBackend->releaseControl();

         delete widgetBackend;
         widgetBackend = nullptr;

      } else if (rendererBackend) {
         rendererBackend->clearSurface();
         rendererBackend->releaseControl();

         delete rendererBackend;
         rendererBackend = nullptr;

      } else {
         windowBackend->releaseControl();

         delete windowBackend;
         windowBackend = nullptr;
      }

      currentBackend = nullptr;
      currentControl = nullptr;
      service        = nullptr;
   }
}

bool QVideoWidgetPrivate::createWidgetBackend()
{
   if (QMediaControl *control = service->requestControl(QVideoWidgetControl_iid)) {
      if (QVideoWidgetControl *widgetControl = dynamic_cast<QVideoWidgetControl *>(control)) {
         widgetBackend = new QVideoWidgetControlBackend(service, widgetControl, q_func());

         setCurrentControl(widgetBackend);

         return true;
      }

      service->releaseControl(control);
   }

   return false;
}

bool QVideoWidgetPrivate::createWindowBackend()
{
   if (QMediaControl *control = service->requestControl(QVideoWindowControl_iid)) {

      if (QVideoWindowControl *windowControl = dynamic_cast<QVideoWindowControl *>(control)) {
         windowBackend  = new QWindowVideoWidgetBackend(service, windowControl, q_func());
         currentBackend = windowBackend;

         setCurrentControl(windowBackend);

         return true;
      }

      service->releaseControl(control);
   }

   return false;
}

bool QVideoWidgetPrivate::createRendererBackend()
{
   if (QMediaControl *control = service->requestControl(QVideoRendererControl_iid)) {
      if (QVideoRendererControl *rendererControl = dynamic_cast<QVideoRendererControl *>(control)) {
         rendererBackend = new QRendererVideoWidgetBackend(service, rendererControl, q_func());
         currentBackend  = rendererBackend;

         setCurrentControl(rendererBackend);

         return true;
      }

      service->releaseControl(control);
   }

   return false;
}

void QVideoWidgetPrivate::_q_serviceDestroyed()
{
   if (widgetBackend) {
      delete q_func()->layout();
   }

   delete widgetBackend;
   delete windowBackend;
   delete rendererBackend;

   widgetBackend   = nullptr;
   windowBackend   = nullptr;
   rendererBackend = nullptr;
   currentControl  = nullptr;
   currentBackend  = nullptr;
   service         = nullptr;
}

void QVideoWidgetPrivate::_q_brightnessChanged(int b)
{
   if (b != brightness) {
      emit q_func()->brightnessChanged(brightness = b);
   }
}

void QVideoWidgetPrivate::_q_contrastChanged(int c)
{
   if (c != contrast) {
      emit q_func()->contrastChanged(contrast = c);
   }
}

void QVideoWidgetPrivate::_q_hueChanged(int h)
{
   if (h != hue) {
      emit q_func()->hueChanged(hue = h);
   }
}

void QVideoWidgetPrivate::_q_saturationChanged(int s)
{
   if (s != saturation) {
      emit q_func()->saturationChanged(saturation = s);
   }
}


void QVideoWidgetPrivate::_q_fullScreenChanged(bool fullScreen)
{
   if (! fullScreen && q_func()->isFullScreen()) {
      q_func()->showNormal();
   }
}

void QVideoWidgetPrivate::_q_dimensionsChanged()
{
   q_func()->updateGeometry();
   q_func()->update();
}

QVideoWidget::QVideoWidget(QWidget *parent)
   : QWidget(parent, Qt::EmptyFlag), d_ptr(new QVideoWidgetPrivate)
{
   d_ptr->q_ptr = this;
}

QVideoWidget::QVideoWidget(QVideoWidgetPrivate &dd, QWidget *parent)
   : QWidget(parent, Qt::EmptyFlag), d_ptr(&dd)
{
   d_ptr->q_ptr = this;

   QPalette palette = QWidget::palette();
   palette.setColor(QPalette::Background, Qt::black);
   setPalette(palette);
}

QVideoWidget::~QVideoWidget()
{
   d_ptr->clearService();

   delete d_ptr;
}

QMediaObject *QVideoWidget::mediaObject() const
{
   return d_func()->mediaObject;
}

bool QVideoWidget::setMediaObject(QMediaObject *object)
{
   Q_D(QVideoWidget);

   if (object == d->mediaObject) {
      return true;
   }

   d->clearService();
   d->mediaObject = object;

   if (d->mediaObject) {
      d->service = d->mediaObject->service();
   }

   if (d->service) {
      if (d->createWidgetBackend()) {
         // Nothing to do

      } else if ((!window() || !window()->testAttribute(Qt::WA_DontShowOnScreen))
            && d->createWindowBackend()) {

         if (isVisible()) {
            d->windowBackend->showEvent();
         }

      } else if (d->createRendererBackend()) {
         if (isVisible()) {
            d->rendererBackend->showEvent();
         }

      } else {
         d->service     = nullptr;
         d->mediaObject = nullptr;

         return false;
      }

      connect(d->service, &QMediaService::destroyed, this, &QVideoWidget::_q_serviceDestroyed);

   } else {
      d->mediaObject = nullptr;

      return false;
   }

   return true;
}

Qt::AspectRatioMode QVideoWidget::aspectRatioMode() const
{
   return d_func()->aspectRatioMode;
}

void QVideoWidget::setAspectRatioMode(Qt::AspectRatioMode mode)
{
   Q_D(QVideoWidget);

   if (d->currentControl) {
      d->currentControl->setAspectRatioMode(mode);
      d->aspectRatioMode = d->currentControl->aspectRatioMode();
   } else {
      d->aspectRatioMode = mode;
   }
}

void QVideoWidget::setFullScreen(bool fullScreen)
{
   Q_D(QVideoWidget);

   Qt::WindowFlags flags = windowFlags();

   if (fullScreen) {
      d->nonFullScreenFlags = flags & (Qt::Window | Qt::SubWindow);
      flags |= Qt::Window;
      flags &= ~Qt::SubWindow;
      setWindowFlags(flags);

      showFullScreen();
   } else {
      flags &= ~(Qt::Window | Qt::SubWindow);    // clear the flags
      flags |= d->nonFullScreenFlags;            // then we reset the flags (window and subwindow)
      setWindowFlags(flags);

      showNormal();
   }
}

int QVideoWidget::brightness() const
{
   return d_func()->brightness;
}

void QVideoWidget::setBrightness(int brightness)
{
   Q_D(QVideoWidget);

   int boundedBrightness = qBound(-100, brightness, 100);

   if (d->currentControl) {
      d->currentControl->setBrightness(boundedBrightness);
   } else if (d->brightness != boundedBrightness) {
      emit brightnessChanged(d->brightness = boundedBrightness);
   }
}

int QVideoWidget::contrast() const
{
   return d_func()->contrast;
}

void QVideoWidget::setContrast(int contrast)
{
   Q_D(QVideoWidget);

   int boundedContrast = qBound(-100, contrast, 100);

   if (d->currentControl) {
      d->currentControl->setContrast(boundedContrast);
   } else if (d->contrast != boundedContrast) {
      emit contrastChanged(d->contrast = boundedContrast);
   }
}

int QVideoWidget::hue() const
{
   return d_func()->hue;
}

void QVideoWidget::setHue(int hue)
{
   Q_D(QVideoWidget);

   int boundedHue = qBound(-100, hue, 100);

   if (d->currentControl) {
      d->currentControl->setHue(boundedHue);
   } else if (d->hue != boundedHue) {
      emit hueChanged(d->hue = boundedHue);
   }
}

int QVideoWidget::saturation() const
{
   return d_func()->saturation;
}

void QVideoWidget::setSaturation(int saturation)
{
   Q_D(QVideoWidget);

   int boundedSaturation = qBound(-100, saturation, 100);

   if (d->currentControl) {
      d->currentControl->setSaturation(boundedSaturation);
   } else if (d->saturation != boundedSaturation) {
      emit saturationChanged(d->saturation = boundedSaturation);
   }

}

QSize QVideoWidget::sizeHint() const
{
   Q_D(const QVideoWidget);

   if (d->currentBackend) {
      return d->currentBackend->sizeHint();
   } else {
      return QWidget::sizeHint();
   }


}

bool QVideoWidget::event(QEvent *event)
{
   Q_D(QVideoWidget);

   if (event->type() == QEvent::WindowStateChange) {
      if (windowState() & Qt::WindowFullScreen) {
         if (d->currentControl) {
            d->currentControl->setFullScreen(true);
         }

         if (! d->wasFullScreen) {
            emit fullScreenChanged(d->wasFullScreen = true);
         }

      } else {
         if (d->currentControl) {
            d->currentControl->setFullScreen(false);
         }

         if (d->wasFullScreen) {
            emit fullScreenChanged(d->wasFullScreen = false);
         }
      }
   }
   return QWidget::event(event);
}

void QVideoWidget::showEvent(QShowEvent *event)
{
   Q_D(QVideoWidget);

   QWidget::showEvent(event);

   // The window backend won't work for re-directed windows so use the renderer backend instead.
   if (d->windowBackend && window()->testAttribute(Qt::WA_DontShowOnScreen)) {
      d->windowBackend->releaseControl();

      delete d->windowBackend;
      d->windowBackend = nullptr;

      d->createRendererBackend();
   }

   if (d->currentBackend) {
      d->currentBackend->showEvent();
   }
}

void QVideoWidget::hideEvent(QHideEvent *event)
{
   Q_D(QVideoWidget);

   if (d->currentBackend) {
      d->currentBackend->hideEvent(event);
   }

   QWidget::hideEvent(event);
}

void QVideoWidget::resizeEvent(QResizeEvent *event)
{
   Q_D(QVideoWidget);

   QWidget::resizeEvent(event);

   if (d->currentBackend) {
      d->currentBackend->resizeEvent(event);
   }
}

void QVideoWidget::moveEvent(QMoveEvent *event)
{
   Q_D(QVideoWidget);

   if (d->currentBackend) {
      d->currentBackend->moveEvent(event);
   }
}

void QVideoWidget::paintEvent(QPaintEvent *event)
{
   Q_D(QVideoWidget);

   if (d->currentBackend) {
      d->currentBackend->paintEvent(event);
   } else if (testAttribute(Qt::WA_OpaquePaintEvent)) {
      QPainter painter(this);

      painter.fillRect(event->rect(), palette().window());
   }
}

void QVideoWidget::_q_serviceDestroyed()
{
   Q_D(QVideoWidget);
   d->_q_serviceDestroyed();
}

void QVideoWidget::_q_brightnessChanged(int brightness)
{
   Q_D(QVideoWidget);
   d->_q_brightnessChanged(brightness);
}

void QVideoWidget::_q_contrastChanged(int contrast)
{
   Q_D(QVideoWidget);
   d->_q_contrastChanged(contrast);
}

void QVideoWidget::_q_hueChanged(int hue)
{
   Q_D(QVideoWidget);
   d->_q_hueChanged(hue);
}

void QVideoWidget::_q_saturationChanged(int saturation)
{
   Q_D(QVideoWidget);
   d->_q_saturationChanged(saturation);
}

void QVideoWidget::_q_fullScreenChanged(bool fullScreen)
{
   Q_D(QVideoWidget);
   d->_q_fullScreenChanged(fullScreen);
}

void QVideoWidget::_q_dimensionsChanged()
{
   Q_D(QVideoWidget);
   d->_q_dimensionsChanged();
}
