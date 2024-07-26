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

#include <qsvgrenderer.h>

#ifndef QT_NO_SVGRENDERER

#include <qbytearray.h>
#include <qdebug.h>
#include <qtimer.h>

#include <qsvgtinydocument_p.h>

class QSvgRendererPrivate
{
   Q_DECLARE_PUBLIC(QSvgRenderer)

 public:
   explicit QSvgRendererPrivate()
      : render(nullptr), timer(nullptr), fps(30)
   {
   }

   virtual ~QSvgRendererPrivate()
   {
      delete render;
   }

   static void callRepaintNeeded(QSvgRenderer *const q);

   QSvgTinyDocument *render;
   QTimer *timer;
   int fps;

 protected:
   QSvgRenderer *q_ptr;
};

QSvgRenderer::QSvgRenderer(QObject *parent)
   : QObject(parent), d_ptr(new QSvgRendererPrivate)
{
   d_ptr->q_ptr = this;
}

QSvgRenderer::QSvgRenderer(const QString &filename, QObject *parent)
   : QObject(parent), d_ptr(new QSvgRendererPrivate)
{
   d_ptr->q_ptr = this;
   load(filename);
}

QSvgRenderer::QSvgRenderer(const QByteArray &contents, QObject *parent)
   : QObject(parent), d_ptr(new QSvgRendererPrivate)
{
   d_ptr->q_ptr = this;
   load(contents);
}

QSvgRenderer::QSvgRenderer(QXmlStreamReader *contents, QObject *parent)
   : QObject(parent), d_ptr(new QSvgRendererPrivate)
{
   d_ptr->q_ptr = this;
   load(contents);
}

QSvgRenderer::~QSvgRenderer()
{
}

bool QSvgRenderer::isValid() const
{
   Q_D(const QSvgRenderer);
   return d->render;
}

QSize QSvgRenderer::defaultSize() const
{
   Q_D(const QSvgRenderer);
   if (d->render) {
      return d->render->size();
   } else {
      return QSize();
   }
}

QRect QSvgRenderer::viewBox() const
{
   Q_D(const QSvgRenderer);
   if (d->render) {
      return d->render->viewBox().toRect();
   } else {
      return QRect();
   }
}

void QSvgRenderer::setViewBox(const QRect &viewbox)
{
   Q_D(QSvgRenderer);
   if (d->render) {
      d->render->setViewBox(viewbox);
   }
}

bool QSvgRenderer::animated() const
{
   Q_D(const QSvgRenderer);
   if (d->render) {
      return d->render->animated();
   } else {
      return false;
   }
}

int QSvgRenderer::framesPerSecond() const
{
   Q_D(const QSvgRenderer);
   return d->fps;
}

void QSvgRenderer::setFramesPerSecond(int num)
{
   Q_D(QSvgRenderer);
   if (num < 0) {
      qWarning("QSvgRenderer::setFramesPerSecond: Cannot set negative value %d", num);
      return;
   }
   d->fps = num;
}

int QSvgRenderer::currentFrame() const
{
   Q_D(const QSvgRenderer);
   return d->render->currentFrame();
}

void QSvgRenderer::setCurrentFrame(int frame)
{
   Q_D(QSvgRenderer);
   d->render->setCurrentFrame(frame);
}

int QSvgRenderer::animationDuration() const
{
   Q_D(const QSvgRenderer);
   return d->render->animationDuration();
}

void QSvgRendererPrivate::callRepaintNeeded(QSvgRenderer *const q)
{
   q->repaintNeeded();
}

template <typename TInputType>
static bool loadDocument(QSvgRenderer *const q, QSvgRendererPrivate *const d, const TInputType &in)
{
   delete d->render;
   d->render = QSvgTinyDocument::load(in);

   if (d->render && d->render->animated() && d->fps > 0) {
      if (! d->timer) {
         d->timer = new QTimer(q);
      } else {
         d->timer->stop();
      }

      q->connect(d->timer, SIGNAL(timeout()), q, SLOT(repaintNeeded()));
      d->timer->start(1000 / d->fps);

   } else if (d->timer) {
      d->timer->stop();
   }

   // force first update
   QSvgRendererPrivate::callRepaintNeeded(q);

   return d->render;
}

bool QSvgRenderer::load(const QString &filename)
{
   Q_D(QSvgRenderer);
   return loadDocument(this, d, filename);
}

bool QSvgRenderer::load(const QByteArray &contents)
{
   Q_D(QSvgRenderer);
   return loadDocument(this, d, contents);
}

bool QSvgRenderer::load(QXmlStreamReader *contents)
{
   Q_D(QSvgRenderer);
   return loadDocument(this, d, contents);
}

void QSvgRenderer::render(QPainter *painter)
{
   Q_D(QSvgRenderer);
   if (d->render) {
      d->render->draw(painter);
   }
}

void QSvgRenderer::render(QPainter *painter, const QString &elementId, const QRectF &bounds)
{
   Q_D(QSvgRenderer);
   if (d->render) {
      d->render->draw(painter, elementId, bounds);
   }
}

void QSvgRenderer::render(QPainter *painter, const QRectF &bounds)
{
   Q_D(QSvgRenderer);
   if (d->render) {
      d->render->draw(painter, bounds);
   }
}

QRectF QSvgRenderer::viewBoxF() const
{
   Q_D(const QSvgRenderer);
   if (d->render) {
      return d->render->viewBox();
   } else {
      return QRect();
   }
}

void QSvgRenderer::setViewBox(const QRectF &viewbox)
{
   Q_D(QSvgRenderer);
   if (d->render) {
      d->render->setViewBox(viewbox);
   }
}

QRectF QSvgRenderer::boundsOnElement(const QString &id) const
{
   Q_D(const QSvgRenderer);
   QRectF bounds;
   if (d->render) {
      bounds = d->render->boundsOnElement(id);
   }
   return bounds;
}

bool QSvgRenderer::elementExists(const QString &id) const
{
   Q_D(const QSvgRenderer);
   bool exists = false;
   if (d->render) {
      exists = d->render->elementExists(id);
   }
   return exists;
}

QMatrix QSvgRenderer::matrixForElement(const QString &id) const
{
   Q_D(const QSvgRenderer);
   QMatrix mat;
   if (d->render) {
      mat = d->render->matrixForElement(id);
   }
   return mat;
}

#endif // QT_NO_SVGRENDERER
