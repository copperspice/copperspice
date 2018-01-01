/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qsvgrenderer.h"

#ifndef QT_NO_SVGRENDERER

#include "qsvgtinydocument_p.h"

#include "qbytearray.h"
#include "qtimer.h"
#include "qdebug.h"

QT_BEGIN_NAMESPACE

class QSvgRendererPrivate
{
   Q_DECLARE_PUBLIC(QSvgRenderer)

 public:
   explicit QSvgRendererPrivate()
      : render(0), timer(0), fps(30) {
   }

   virtual ~QSvgRendererPrivate() {
      delete render;
   }

   static void callRepaintNeeded(QSvgRenderer *const q);

   QSvgTinyDocument *render;
   QTimer *timer;
   int fps;

 protected:
   QSvgRenderer *q_ptr;
};

/*!
    Constructs a new renderer with the given \a parent.
*/
QSvgRenderer::QSvgRenderer(QObject *parent)
   : QObject(parent), d_ptr(new QSvgRendererPrivate)
{
   d_ptr->q_ptr = this;
}

/*!
    Constructs a new renderer with the given \a parent and loads the contents of the
    SVG file with the specified \a filename.
*/
QSvgRenderer::QSvgRenderer(const QString &filename, QObject *parent)
   : QObject(parent), d_ptr(new QSvgRendererPrivate)
{
   d_ptr->q_ptr = this;
   load(filename);
}

/*!
    Constructs a new renderer with the given \a parent and loads the SVG data
    from the byte array specified by \a contents.
*/
QSvgRenderer::QSvgRenderer(const QByteArray &contents, QObject *parent)
   : QObject(parent), d_ptr(new QSvgRendererPrivate)
{
   d_ptr->q_ptr = this;
   load(contents);
}

/*!
    \since 4.5

    Constructs a new renderer with the given \a parent and loads the SVG data
    using the stream reader specified by \a contents.
*/
QSvgRenderer::QSvgRenderer(QXmlStreamReader *contents, QObject *parent)
   : QObject(parent), d_ptr(new QSvgRendererPrivate)
{
   d_ptr->q_ptr = this;
   load(contents);
}

/*!
    Destroys the renderer.
*/
QSvgRenderer::~QSvgRenderer()
{
}

/*!
    Returns true if there is a valid current document; otherwise returns false.
*/
bool QSvgRenderer::isValid() const
{
   Q_D(const QSvgRenderer);
   return d->render;
}

/*!
    Returns the default size of the document contents.
*/
QSize QSvgRenderer::defaultSize() const
{
   Q_D(const QSvgRenderer);
   if (d->render) {
      return d->render->size();
   } else {
      return QSize();
   }
}

/*!
    Returns viewBoxF().toRect().

    \sa viewBoxF()
*/
QRect QSvgRenderer::viewBox() const
{
   Q_D(const QSvgRenderer);
   if (d->render) {
      return d->render->viewBox().toRect();
   } else {
      return QRect();
   }
}

/*!
    \property QSvgRenderer::viewBox
    \brief the rectangle specifying the visible area of the document in logical coordinates
    \since 4.2
*/
void QSvgRenderer::setViewBox(const QRect &viewbox)
{
   Q_D(QSvgRenderer);
   if (d->render) {
      d->render->setViewBox(viewbox);
   }
}

/*!
    Returns true if the current document contains animated elements; otherwise
    returns false.

    \sa framesPerSecond()
*/
bool QSvgRenderer::animated() const
{
   Q_D(const QSvgRenderer);
   if (d->render) {
      return d->render->animated();
   } else {
      return false;
   }
}

/*!
    \property QSvgRenderer::framesPerSecond
    \brief the number of frames per second to be shown

    The number of frames per second is 0 if the current document is not animated.

    \sa animated()
*/
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

/*!
  \property QSvgRenderer::currentFrame
  \brief the current frame of the document's animation, or 0 if the document is not animated
  \internal

  \sa animationDuration(), framesPerSecond, animated()
*/

/*!
  \internal
*/
int QSvgRenderer::currentFrame() const
{
   Q_D(const QSvgRenderer);
   return d->render->currentFrame();
}

/*!
  \internal
*/
void QSvgRenderer::setCurrentFrame(int frame)
{
   Q_D(QSvgRenderer);
   d->render->setCurrentFrame(frame);
}

/*!
    \internal

    Returns the number of frames in the animation, or 0 if the current document is not
    animated.

    \sa animated(), framesPerSecond
*/
int QSvgRenderer::animationDuration() const
{
   Q_D(const QSvgRenderer);
   return d->render->animationDuration();
}

/*!
 \internal
 \since 4.5

 We can't have template functions, that's loadDocument(), as friends, for this
 code, so we let this function be a friend of QSvgRenderer instead.
 */
void QSvgRendererPrivate::callRepaintNeeded(QSvgRenderer *const q)
{
   q->repaintNeeded();
}

template<typename TInputType>
static bool loadDocument(QSvgRenderer *const q,
                         QSvgRendererPrivate *const d,
                         const TInputType &in)
{
   delete d->render;
   d->render = QSvgTinyDocument::load(in);

   if (d->render && d->render->animated() && d->fps > 0) {
      if (!d->timer) {
         d->timer = new QTimer(q);
      } else {
         d->timer->stop();
      }

      q->connect(d->timer, SIGNAL(timeout()), q, SLOT(repaintNeeded()));
      d->timer->start(1000 / d->fps);

   } else if (d->timer) {
      d->timer->stop();
   }

   //force first update
   QSvgRendererPrivate::callRepaintNeeded(q);

   return d->render;
}

/*!
    Loads the SVG file specified by \a filename, returning true if the content
    was successfully parsed; otherwise returns false.
*/
bool QSvgRenderer::load(const QString &filename)
{
   Q_D(QSvgRenderer);
   return loadDocument(this, d, filename);
}

/*!
    Loads the specified SVG format \a contents, returning true if the content
    was successfully parsed; otherwise returns false.
*/
bool QSvgRenderer::load(const QByteArray &contents)
{
   Q_D(QSvgRenderer);
   return loadDocument(this, d, contents);
}

/*!
  Loads the specified SVG in \a contents, returning true if the content
  was successfully parsed; otherwise returns false.

  The reader will be used from where it currently is positioned. If \a contents
  is \c null, behavior is undefined.

  \since 4.5
*/
bool QSvgRenderer::load(QXmlStreamReader *contents)
{
   Q_D(QSvgRenderer);
   return loadDocument(this, d, contents);
}

/*!
    Renders the current document, or the current frame of an animated
    document, using the given \a painter.
*/
void QSvgRenderer::render(QPainter *painter)
{
   Q_D(QSvgRenderer);
   if (d->render) {
      d->render->draw(painter);
   }
}

/*!
    \fn void QSvgRenderer::repaintNeeded()

    This signal is emitted whenever the rendering of the document
    needs to be updated, usually for the purposes of animation.
*/

/*!
    Renders the given element with \a elementId using the given \a painter
    on the specified \a bounds. If the bounding rectangle is not specified
    the SVG element is mapped to the whole paint device.
*/
void QSvgRenderer::render(QPainter *painter, const QString &elementId,
                          const QRectF &bounds)
{
   Q_D(QSvgRenderer);
   if (d->render) {
      d->render->draw(painter, elementId, bounds);
   }
}

/*!
    Renders the current document, or the current frame of an animated
    document, using the given \a painter on the specified \a bounds within
    the painter.  If the bounding rectangle is not specified
    the SVG file is mapped to the whole paint device.
*/
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

/*!
    \since 4.2

    Returns bounding rectangle of the item with the given \a id.
    The transformation matrix of parent elements is not affecting
    the bounds of the element.

    \sa matrixForElement()
*/
QRectF QSvgRenderer::boundsOnElement(const QString &id) const
{
   Q_D(const QSvgRenderer);
   QRectF bounds;
   if (d->render) {
      bounds = d->render->boundsOnElement(id);
   }
   return bounds;
}


/*!
    \since 4.2

    Returns true if the element with the given \a id exists
    in the currently parsed SVG file and is a renderable
    element.

    Note: this method returns true only for elements that
    can be rendered. Which implies that elements that are considered
    part of the fill/stroke style properties, e.g. radialGradients
    even tough marked with "id" attributes will not be found by this
    method.
*/
bool QSvgRenderer::elementExists(const QString &id) const
{
   Q_D(const QSvgRenderer);
   bool exists = false;
   if (d->render) {
      exists = d->render->elementExists(id);
   }
   return exists;
}

/*!
    \since 4.2

    Returns the transformation matrix for the element
    with the given \a id. The matrix is a product of
    the transformation of the element's parents. The transformation of
    the element itself is not included.

    To find the bounding rectangle of the element in logical coordinates,
    you can apply the matrix on the rectangle returned from boundsOnElement().

    \sa boundsOnElement()
*/
QMatrix QSvgRenderer::matrixForElement(const QString &id) const
{
   Q_D(const QSvgRenderer);
   QMatrix mat;
   if (d->render) {
      mat = d->render->matrixForElement(id);
   }
   return mat;
}

QT_END_NAMESPACE

#endif // QT_NO_SVGRENDERER
