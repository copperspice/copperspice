/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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
#include <qpaintervideosurface_p.h>

#include <qmediaobject.h>
#include <qmediaservice.h>
#include <qvideowindowcontrol.h>
#include <qvideowidgetcontrol.h>

#include <qvideorenderercontrol.h>
#include <qvideosurfaceformat.h>
#include <qpainter.h>

#include <qapplication.h>
#include <qevent.h>
#include <qdialog.h>
#include <qboxlayout.h>
#include <qnamespace.h>

QVideoWidgetControlBackend::QVideoWidgetControlBackend(
   QMediaService *service, QVideoWidgetControl *control, QWidget *widget)
   : m_service(service), m_widgetControl(control)
{
   connect(control, SIGNAL(brightnessChanged(int)),  widget, SLOT(_q_brightnessChanged(int)));
   connect(control, SIGNAL(contrastChanged(int)),    widget, SLOT(_q_contrastChanged(int)));
   connect(control, SIGNAL(hueChanged(int)),         widget, SLOT(_q_hueChanged(int)));
   connect(control, SIGNAL(saturationChanged(int)),  widget, SLOT(_q_saturationChanged(int)));
   connect(control, SIGNAL(fullScreenChanged(bool)), widget, SLOT(_q_fullScreenChanged(bool)));

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

QRendererVideoWidgetBackend::QRendererVideoWidgetBackend(
   QMediaService *service, QVideoRendererControl *control, QWidget *widget)
   : m_service(service)
   , m_rendererControl(control)
   , m_widget(widget)
   , m_surface(new QPainterVideoSurface)
   , m_aspectRatioMode(Qt::KeepAspectRatio)
   , m_updatePaintDevice(true)
{
   connect(this, SIGNAL(brightnessChanged(int)), m_widget, SLOT(_q_brightnessChanged(int)));
   connect(this, SIGNAL(contrastChanged(int)), m_widget, SLOT(_q_contrastChanged(int)));
   connect(this, SIGNAL(hueChanged(int)), m_widget, SLOT(_q_hueChanged(int)));
   connect(this, SIGNAL(saturationChanged(int)), m_widget, SLOT(_q_saturationChanged(int)));
   connect(m_surface, SIGNAL(frameChanged()), this, SLOT(frameChanged()));
   connect(m_surface, SIGNAL(surfaceFormatChanged(QVideoSurfaceFormat)),
      this, SLOT(formatChanged(QVideoSurfaceFormat)));

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
   m_rendererControl->setSurface(0);
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
#if ! defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_1_CL) && !defined(QT_OPENGL_ES_1)
   m_updatePaintDevice = true;
   m_surface->setGLContext(0);
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
#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_1_CL) && !defined(QT_OPENGL_ES_1)
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

      m_sourceRect = QRectF(
            0, 0, size.width() / m_nativeSize.width(), size.height() / m_nativeSize.height());
      m_sourceRect.moveCenter(QPointF(0.5, 0.5));
   }
}

QWindowVideoWidgetBackend::QWindowVideoWidgetBackend(
   QMediaService *service, QVideoWindowControl *control, QWidget *widget)
   : m_service(service)
   , m_windowControl(control)
   , m_widget(widget)
{
   connect(control, SIGNAL(brightnessChanged(int)), m_widget, SLOT(_q_brightnessChanged(int)));
   connect(control, SIGNAL(contrastChanged(int)), m_widget, SLOT(_q_contrastChanged(int)));
   connect(control, SIGNAL(hueChanged(int)), m_widget, SLOT(_q_hueChanged(int)));
   connect(control, SIGNAL(saturationChanged(int)), m_widget, SLOT(_q_saturationChanged(int)));
   connect(control, SIGNAL(fullScreenChanged(bool)), m_widget, SLOT(_q_fullScreenChanged(bool)));
   connect(control, SIGNAL(nativeSizeChanged()), m_widget, SLOT(_q_dimensionsChanged()));

   control->setWinId(widget->winId());
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

#if defined(Q_WS_WIN)
   m_widget->setUpdatesEnabled(false);
#endif
}

void QWindowVideoWidgetBackend::hideEvent(QHideEvent *)
{
#if defined(Q_WS_WIN)
   m_widget->setUpdatesEnabled(true);
#endif
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

#if defined(Q_WS_WIN)
bool QWindowVideoWidgetBackend::winEvent(MSG *message, long *)
{
   if (message->message == WM_PAINT) {
      m_windowControl->repaint();
   }

   return false;
}
#endif

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
      QObject::disconnect(service, SIGNAL(destroyed()), q_func(), SLOT(_q_serviceDestroyed()));

      if (widgetBackend) {
         QLayout *layout = q_func()->layout();

         for (QLayoutItem *item = layout->takeAt(0); item; item = layout->takeAt(0)) {
            item->widget()->setParent(0);
            delete item;
         }
         delete layout;

         widgetBackend->releaseControl();

         delete widgetBackend;
         widgetBackend = 0;
      } else if (rendererBackend) {
         rendererBackend->clearSurface();
         rendererBackend->releaseControl();

         delete rendererBackend;
         rendererBackend = 0;
      } else {
         windowBackend->releaseControl();

         delete windowBackend;
         windowBackend = 0;
      }

      currentBackend = 0;
      currentControl = 0;
      service = 0;
   }
}

bool QVideoWidgetPrivate::createWidgetBackend()
{
   if (QMediaControl *control = service->requestControl(QVideoWidgetControl_iid)) {
      if (QVideoWidgetControl *widgetControl = qobject_cast<QVideoWidgetControl *>(control)) {
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
      if (QVideoWindowControl *windowControl = qobject_cast<QVideoWindowControl *>(control)) {
         windowBackend = new QWindowVideoWidgetBackend(service, windowControl, q_func());
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
      if (QVideoRendererControl *rendererControl = qobject_cast<QVideoRendererControl *>(control)) {
         rendererBackend = new QRendererVideoWidgetBackend(service, rendererControl, q_func());
         currentBackend = rendererBackend;

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

   widgetBackend = 0;
   windowBackend = 0;
   rendererBackend = 0;
   currentControl = 0;
   currentBackend = 0;
   service = 0;
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
   if (!fullScreen && q_func()->isFullScreen()) {
      q_func()->showNormal();
   }
}

void QVideoWidgetPrivate::_q_dimensionsChanged()
{
   q_func()->updateGeometry();
   q_func()->update();
}

QVideoWidget::QVideoWidget(QWidget *parent)
   : QWidget(parent, 0)
   , d_ptr(new QVideoWidgetPrivate)
{
   d_ptr->q_ptr = this;
}

/*!
  \internal
*/
QVideoWidget::QVideoWidget(QVideoWidgetPrivate &dd, QWidget *parent)
   : QWidget(parent, 0)
   , d_ptr(&dd)
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

/*!
    \internal
*/
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
         // Nothing to do here

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
         d->service     = 0;
         d->mediaObject = 0;

         return false;
      }

      connect(d->service, SIGNAL(destroyed()), this, SLOT(_q_serviceDestroyed()));

   } else {
      d->mediaObject = 0;

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

/*!
    \property QVideoWidget::fullScreen
    \brief whether video display is confined to a window or is fullScreen.
*/

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
      flags &= ~(Qt::Window | Qt::SubWindow); //clear the flags...
      flags |= d->nonFullScreenFlags; //then we reset the flags (window and subwindow)
      setWindowFlags(flags);

      showNormal();
   }
}

/*!
    \fn QVideoWidget::fullScreenChanged(bool fullScreen)

    Signals that the \a fullScreen mode of a video widget has changed.

    \sa isFullScreen()
*/

/*!
    \property QVideoWidget::brightness
    \brief an adjustment to the brightness of displayed video.

    Valid brightness values range between -100 and 100, the default is 0.
*/

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

/*!
    \fn QVideoWidget::brightnessChanged(int brightness)

    Signals that a video widgets's \a brightness adjustment has changed.

    \sa brightness()
*/

/*!
    \property QVideoWidget::contrast
    \brief an adjustment to the contrast of displayed video.

    Valid contrast values range between -100 and 100, the default is 0.

*/

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

/*!
    \fn QVideoWidget::contrastChanged(int contrast)

    Signals that a video widgets's \a contrast adjustment has changed.

    \sa contrast()
*/

/*!
    \property QVideoWidget::hue
    \brief an adjustment to the hue of displayed video.

    Valid hue values range between -100 and 100, the default is 0.
*/

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

/*!
    \fn QVideoWidget::hueChanged(int hue)

    Signals that a video widgets's \a hue has changed.

    \sa hue()
*/

/*!
    \property QVideoWidget::saturation
    \brief an adjustment to the saturation of displayed video.

    Valid saturation values range between -100 and 100, the default is 0.
*/

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

/*!
    \fn QVideoWidget::saturationChanged(int saturation)

    Signals that a video widgets's \a saturation has changed.

    \sa saturation()
*/

/*!
  Returns the size hint for the current back end,
  if there is one, or else the size hint from QWidget.
 */
QSize QVideoWidget::sizeHint() const
{
   Q_D(const QVideoWidget);

   if (d->currentBackend) {
      return d->currentBackend->sizeHint();
   } else {
      return QWidget::sizeHint();
   }


}

/*!
  \reimp
  Current event \a event.
  Returns the value of the baseclass QWidget::event(QEvent *event) function.
*/
bool QVideoWidget::event(QEvent *event)
{
   Q_D(QVideoWidget);

   if (event->type() == QEvent::WindowStateChange) {
      if (windowState() & Qt::WindowFullScreen) {
         if (d->currentControl) {
            d->currentControl->setFullScreen(true);
         }

         if (!d->wasFullScreen) {
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

/*!
  \reimp
  Handles the show \a event.
 */
void QVideoWidget::showEvent(QShowEvent *event)
{
   Q_D(QVideoWidget);

   QWidget::showEvent(event);

   // The window backend won't work for re-directed windows so use the renderer backend instead.
   if (d->windowBackend && window()->testAttribute(Qt::WA_DontShowOnScreen)) {
      d->windowBackend->releaseControl();

      delete d->windowBackend;
      d->windowBackend = 0;

      d->createRendererBackend();
   }

   if (d->currentBackend) {
      d->currentBackend->showEvent();
   }
}

/*!
  \reimp
  Handles the hide \a event.
*/
void QVideoWidget::hideEvent(QHideEvent *event)
{
   Q_D(QVideoWidget);

   if (d->currentBackend) {
      d->currentBackend->hideEvent(event);
   }

   QWidget::hideEvent(event);
}

/*!
  \reimp
  Handles the resize \a event.
 */
void QVideoWidget::resizeEvent(QResizeEvent *event)
{
   Q_D(QVideoWidget);

   QWidget::resizeEvent(event);

   if (d->currentBackend) {
      d->currentBackend->resizeEvent(event);
   }
}

/*!
  \reimp
  Handles the move \a event.
 */
void QVideoWidget::moveEvent(QMoveEvent *event)
{
   Q_D(QVideoWidget);

   if (d->currentBackend) {
      d->currentBackend->moveEvent(event);
   }
}

/*!
  \reimp
  Handles the paint \a event.
 */
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


#if defined(Q_WS_WIN)
/*!
    \internal
*/
bool QVideoWidget::winEvent(MSG *message, long *result)
{
   return d_func()->windowBackend && d_func()->windowBackend->winEvent(message, result)
      ? true : QWidget::winEvent(message, result);
}
#endif

void QVideoWidget::_q_serviceDestroyed()
{
   Q_D(QVideoWidget);
   d->_q_serviceDestroyed();
}

void QVideoWidget::_q_brightnessChanged(int un_named_arg1)
{
   Q_D(QVideoWidget);
   d->_q_brightnessChanged(un_named_arg1);
}

void QVideoWidget::_q_contrastChanged(int un_named_arg1)
{
   Q_D(QVideoWidget);
   d->_q_contrastChanged(un_named_arg1);
}

void QVideoWidget::_q_hueChanged(int un_named_arg1)
{
   Q_D(QVideoWidget);
   d->_q_hueChanged(un_named_arg1);
}

void QVideoWidget::_q_saturationChanged(int un_named_arg1)
{
   Q_D(QVideoWidget);
   d->_q_saturationChanged(un_named_arg1);
}

void QVideoWidget::_q_fullScreenChanged(bool un_named_arg1)
{
   Q_D(QVideoWidget);
   d->_q_fullScreenChanged(un_named_arg1);
}

void QVideoWidget::_q_dimensionsChanged()
{
   Q_D(QVideoWidget);
   d->_q_dimensionsChanged();

}