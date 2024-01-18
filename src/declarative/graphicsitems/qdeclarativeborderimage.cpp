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

#include <qdeclarativeborderimage_p.h>
#include <qdeclarativeborderimage_p_p.h>
#include <qdeclarativeinfo.h>
#include <qdeclarativeengine_p.h>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>

QT_BEGIN_NAMESPACE

/*!
    \qmlclass BorderImage QDeclarativeBorderImage
    \brief The BorderImage element provides an image that can be used as a border.
    \inherits Item
    \since 4.7
    \ingroup qml-basic-visual-elements

    The BorderImage element is used to create borders out of images by scaling or tiling
    parts of each image.

    A BorderImage element breaks a source image, specified using the \l url property,
    into 9 regions, as shown below:

    \image declarative-scalegrid.png

    When the image is scaled, regions of the source image are scaled or tiled to
    create the displayed border image in the following way:

    \list
    \i The corners (regions 1, 3, 7, and 9) are not scaled at all.
    \i Regions 2 and 8 are scaled according to
       \l{BorderImage::horizontalTileMode}{horizontalTileMode}.
    \i Regions 4 and 6 are scaled according to
       \l{BorderImage::verticalTileMode}{verticalTileMode}.
    \i The middle (region 5) is scaled according to both
       \l{BorderImage::horizontalTileMode}{horizontalTileMode} and
       \l{BorderImage::verticalTileMode}{verticalTileMode}.
    \endlist

    The regions of the image are defined using the \l border property group, which
    describes the distance from each edge of the source image to use as a border.

    \section1 Example Usage

    The following examples show the effects of the different modes on an image.
    Guide lines are overlaid onto the image to show the different regions of the
    image as described above.

    \beginfloatleft
    \image qml-borderimage-normal-image.png
    \endfloat

    An unscaled image is displayed using an Image element. The \l border property is
    used to determine the parts of the image that will lie inside the unscaled corner
    areas and the parts that will be stretched horizontally and vertically.

    \snippet doc/src/snippets/declarative/borderimage/normal-image.qml normal image

    \clearfloat
    \beginfloatleft
    \image qml-borderimage-scaled.png
    \endfloat

    A BorderImage element is used to display the image, and it is given a size that is
    larger than the original image. Since the \l horizontalTileMode property is set to
    \l{BorderImage::horizontalTileMode}{BorderImage.Stretch}, the parts of image in
    regions 2 and 8 are stretched horizontally. Since the \l verticalTileMode property
    is set to \l{BorderImage::verticalTileMode}{BorderImage.Stretch}, the parts of image
    in regions 4 and 6 are stretched vertically.

    \snippet doc/src/snippets/declarative/borderimage/borderimage-scaled.qml scaled border image

    \clearfloat
    \beginfloatleft
    \image qml-borderimage-tiled.png
    \endfloat

    Again, a large BorderImage element is used to display the image. With the
    \l horizontalTileMode property set to \l{BorderImage::horizontalTileMode}{BorderImage.Repeat},
    the parts of image in regions 2 and 8 are tiled so that they fill the space at the
    top and bottom of the element. Similarly, the \l verticalTileMode property is set to
    \l{BorderImage::verticalTileMode}{BorderImage.Repeat}, the parts of image in regions
    4 and 6 are tiled so that they fill the space at the left and right of the element.

    \snippet doc/src/snippets/declarative/borderimage/borderimage-tiled.qml tiled border image

    \clearfloat
    In some situations, the width of regions 2 and 8 may not be an exact multiple of the width
    of the corresponding regions in the source image. Similarly, the height of regions 4 and 6
    may not be an exact multiple of the height of the corresponding regions. It can be useful
    to use \l{BorderImage::horizontalTileMode}{BorderImage.Round} instead of
    \l{BorderImage::horizontalTileMode}{BorderImage.Repeat} in cases like these.

    The \l{declarative/imageelements/borderimage}{BorderImage example} shows how a BorderImage
    can be used to simulate a shadow effect on a rectangular item.

    \section1 Quality and Performance

    By default, any scaled regions of the image are rendered without smoothing to improve
    rendering speed. Setting the \l smooth property improves rendering quality of scaled
    regions, but may slow down rendering.

    The source image may not be loaded instantaneously, depending on its original location.
    Loading progress can be monitored with the \l progress property.

    \sa Image, AnimatedImage
 */

/*!
    \qmlproperty bool BorderImage::asynchronous

    Specifies that images on the local filesystem should be loaded
    asynchronously in a separate thread.  The default value is
    false, causing the user interface thread to block while the
    image is loaded.  Setting \a asynchronous to true is useful where
    maintaining a responsive user interface is more desirable
    than having images immediately visible.

    Note that this property is only valid for images read from the
    local filesystem.  Images loaded via a network resource (e.g. HTTP)
    are always loaded asynchonously.
*/
QDeclarativeBorderImage::QDeclarativeBorderImage(QDeclarativeItem *parent)
   : QDeclarativeImageBase(*(new QDeclarativeBorderImagePrivate), parent)
{
}

QDeclarativeBorderImage::~QDeclarativeBorderImage()
{
   Q_D(QDeclarativeBorderImage);
   if (d->sciReply) {
      d->sciReply->deleteLater();
   }
}
/*!
    \qmlproperty enumeration BorderImage::status

    This property describes the status of image loading.  It can be one of:

    \list
    \o BorderImage.Null - no image has been set
    \o BorderImage.Ready - the image has been loaded
    \o BorderImage.Loading - the image is currently being loaded
    \o BorderImage.Error - an error occurred while loading the image
    \endlist

    \sa progress
*/

/*!
    \qmlproperty real BorderImage::progress

    This property holds the progress of image loading, from 0.0 (nothing loaded)
    to 1.0 (finished).

    \sa status
*/

/*!
    \qmlproperty bool BorderImage::smooth

    Set this property if you want the image to be smoothly filtered when scaled or
    transformed.  Smooth filtering gives better visual quality, but is slower.  If
    the image is displayed at its natural size, this property has no visual or
    performance effect.

    By default, this property is set to false.

    \note Generally scaling artifacts are only visible if the image is stationary on
    the screen.  A common pattern when animating an image is to disable smooth
    filtering at the beginning of the animation and enable it at the conclusion.
*/

/*!
    \qmlproperty bool BorderImage::cache
    \since QtQuick 1.1

    Specifies whether the image should be cached. The default value is
    true. Setting \a cache to false is useful when dealing with large images,
    to make sure that they aren't cached at the expense of small 'ui element' images.
*/

/*!
    \qmlproperty bool BorderImage::mirror
    \since QtQuick 1.1

    This property holds whether the image should be horizontally inverted
    (effectively displaying a mirrored image).

    The default value is false.
*/

/*!
    \qmlproperty url BorderImage::source

    This property holds the URL that refers to the source image.

    BorderImage can handle any image format supported by Qt, loaded from any
    URL scheme supported by Qt.

    This property can also be used to refer to .sci files, which are
    written in a QML-specific, text-based format that specifies the
    borders, the image file and the tile rules for a given border image.

    The following .sci file sets the borders to 10 on each side for the
    image \c picture.png:

    \code
    border.left: 10
    border.top: 10
    border.bottom: 10
    border.right: 10
    source: "picture.png"
    \endcode

    The URL may be absolute, or relative to the URL of the component.

    \sa QDeclarativeImageProvider
*/

/*!
    \qmlproperty QSize BorderImage::sourceSize

    This property holds the actual width and height of the loaded image.

    In BorderImage, this property is read-only.

    \sa Image::sourceSize
*/
void QDeclarativeBorderImage::setSource(const QUrl &url)
{
   Q_D(QDeclarativeBorderImage);
   //equality is fairly expensive, so we bypass for simple, common case
   if ((d->url.isEmpty() == url.isEmpty()) && url == d->url) {
      return;
   }

   if (d->sciReply) {
      d->sciReply->deleteLater();
      d->sciReply = 0;
   }

   d->url = url;
   d->sciurl = QUrl();
   emit sourceChanged(d->url);

   if (isComponentComplete()) {
      load();
   }
}

void QDeclarativeBorderImage::setSourceSize(const QSize &size)
{
   Q_UNUSED(size);
   qmlInfo(this) << "Setting sourceSize for borderImage not supported";
}

void QDeclarativeBorderImage::load()
{
   Q_D(QDeclarativeBorderImage);
   if (d->progress != 0.0) {
      d->progress = 0.0;
      emit progressChanged(d->progress);
   }

   if (d->url.isEmpty()) {
      d->pix.clear(this);
      d->status = Null;
      setImplicitWidth(0);
      setImplicitHeight(0);
      emit statusChanged(d->status);
      update();
   } else {
      d->status = Loading;
      if (d->url.path().endsWith(QLatin1String("sci"))) {
#ifndef QT_NO_LOCALFILE_OPTIMIZED_QML
         QString lf = QDeclarativeEnginePrivate::urlToLocalFileOrQrc(d->url);
         if (!lf.isEmpty()) {
            QFile file(lf);
            file.open(QIODevice::ReadOnly);
            setGridScaledImage(QDeclarativeGridScaledImage(&file));
         } else
#endif
         {
            QNetworkRequest req(d->url);
            d->sciReply = qmlEngine(this)->networkAccessManager()->get(req);

            static int sciReplyFinished = -1;
            static int thisSciRequestFinished = -1;
            if (sciReplyFinished == -1) {
               sciReplyFinished =
                  QNetworkReply::staticMetaObject.indexOfSignal("finished()");
               thisSciRequestFinished =
                  QDeclarativeBorderImage::staticMetaObject.indexOfSlot("sciRequestFinished()");
            }

            QMetaObject::connect(d->sciReply, sciReplyFinished, this,
                                 thisSciRequestFinished, Qt::DirectConnection);
         }
      } else {

         QDeclarativePixmap::Options options;
         if (d->async) {
            options |= QDeclarativePixmap::Asynchronous;
         }
         if (d->cache) {
            options |= QDeclarativePixmap::Cache;
         }
         d->pix.clear(this);
         d->pix.load(qmlEngine(this), d->url, options);

         if (d->pix.isLoading()) {
            d->pix.connectFinished(this, SLOT(requestFinished()));
            d->pix.connectDownloadProgress(this, SLOT(requestProgress(qint64, qint64)));
         } else {
            QSize impsize = d->pix.implicitSize();
            setImplicitWidth(impsize.width());
            setImplicitHeight(impsize.height());

            if (d->pix.isReady()) {
               d->status = Ready;
            } else {
               d->status = Error;
               qmlInfo(this) << d->pix.error();
            }

            d->progress = 1.0;
            emit statusChanged(d->status);
            emit progressChanged(d->progress);
            requestFinished();
            update();
         }
      }
   }

   emit statusChanged(d->status);
}

/*!
    \qmlproperty int BorderImage::border.left
    \qmlproperty int BorderImage::border.right
    \qmlproperty int BorderImage::border.top
    \qmlproperty int BorderImage::border.bottom

    The 4 border lines (2 horizontal and 2 vertical) break the image into 9 sections,
    as shown below:

    \image declarative-scalegrid.png

    Each border line (left, right, top, and bottom) specifies an offset in pixels
    from the respective edge of the source image. By default, each border line has
    a value of 0.

    For example, the following definition sets the bottom line 10 pixels up from
    the bottom of the image:

    \qml
    BorderImage {
        border.bottom: 10
        // ...
    }
    \endqml

    The border lines can also be specified using a
    \l {BorderImage::source}{.sci file}.
*/

QDeclarativeScaleGrid *QDeclarativeBorderImage::border()
{
   Q_D(QDeclarativeBorderImage);
   return d->getScaleGrid();
}

/*!
    \qmlproperty enumeration BorderImage::horizontalTileMode
    \qmlproperty enumeration BorderImage::verticalTileMode

    This property describes how to repeat or stretch the middle parts of the border image.

    \list
    \o BorderImage.Stretch - Scales the image to fit to the available area.
    \o BorderImage.Repeat - Tile the image until there is no more space. May crop the last image.
    \o BorderImage.Round - Like Repeat, but scales the images down to ensure that the last image is not cropped.
    \endlist

    The default tile mode for each property is BorderImage.Stretch.
*/
QDeclarativeBorderImage::TileMode QDeclarativeBorderImage::horizontalTileMode() const
{
   Q_D(const QDeclarativeBorderImage);
   return d->horizontalTileMode;
}

void QDeclarativeBorderImage::setHorizontalTileMode(TileMode t)
{
   Q_D(QDeclarativeBorderImage);
   if (t != d->horizontalTileMode) {
      d->horizontalTileMode = t;
      emit horizontalTileModeChanged();
      update();
   }
}

QDeclarativeBorderImage::TileMode QDeclarativeBorderImage::verticalTileMode() const
{
   Q_D(const QDeclarativeBorderImage);
   return d->verticalTileMode;
}

void QDeclarativeBorderImage::setVerticalTileMode(TileMode t)
{
   Q_D(QDeclarativeBorderImage);
   if (t != d->verticalTileMode) {
      d->verticalTileMode = t;
      emit verticalTileModeChanged();
      update();
   }
}

void QDeclarativeBorderImage::setGridScaledImage(const QDeclarativeGridScaledImage &sci)
{
   Q_D(QDeclarativeBorderImage);
   if (!sci.isValid()) {
      d->status = Error;
      emit statusChanged(d->status);
   } else {
      QDeclarativeScaleGrid *sg = border();
      sg->setTop(sci.gridTop());
      sg->setBottom(sci.gridBottom());
      sg->setLeft(sci.gridLeft());
      sg->setRight(sci.gridRight());
      d->horizontalTileMode = sci.horizontalTileRule();
      d->verticalTileMode = sci.verticalTileRule();

      d->sciurl = d->url.resolved(QUrl(sci.pixmapUrl()));

      QDeclarativePixmap::Options options;
      if (d->async) {
         options |= QDeclarativePixmap::Asynchronous;
      }
      if (d->cache) {
         options |= QDeclarativePixmap::Cache;
      }
      d->pix.clear(this);
      d->pix.load(qmlEngine(this), d->sciurl, options);

      if (d->pix.isLoading()) {
         static int thisRequestProgress = -1;
         static int thisRequestFinished = -1;
         if (thisRequestProgress == -1) {
            thisRequestProgress =
               QDeclarativeBorderImage::staticMetaObject.indexOfSlot("requestProgress(qint64,qint64)");
            thisRequestFinished =
               QDeclarativeBorderImage::staticMetaObject.indexOfSlot("requestFinished()");
         }

         d->pix.connectFinished(this, thisRequestFinished);
         d->pix.connectDownloadProgress(this, thisRequestProgress);

      } else {

         QSize impsize = d->pix.implicitSize();
         setImplicitWidth(impsize.width());
         setImplicitHeight(impsize.height());

         if (d->pix.isReady()) {
            d->status = Ready;
         } else {
            d->status = Error;
            qmlInfo(this) << d->pix.error();
         }

         d->progress = 1.0;
         emit statusChanged(d->status);
         emit progressChanged(1.0);
         update();

      }
   }
}

void QDeclarativeBorderImage::requestFinished()
{
   Q_D(QDeclarativeBorderImage);

   QSize impsize = d->pix.implicitSize();
   if (d->pix.isError()) {
      d->status = Error;
      qmlInfo(this) << d->pix.error();
   } else {
      d->status = Ready;
   }

   setImplicitWidth(impsize.width());
   setImplicitHeight(impsize.height());

   if (d->sourcesize.width() != d->pix.width() || d->sourcesize.height() != d->pix.height()) {
      emit sourceSizeChanged();
   }

   d->progress = 1.0;
   emit statusChanged(d->status);
   emit progressChanged(1.0);
   update();
}

#define BORDERIMAGE_MAX_REDIRECT 16

void QDeclarativeBorderImage::sciRequestFinished()
{
   Q_D(QDeclarativeBorderImage);

   d->redirectCount++;
   if (d->redirectCount < BORDERIMAGE_MAX_REDIRECT) {
      QVariant redirect = d->sciReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
      if (redirect.isValid()) {
         QUrl url = d->sciReply->url().resolved(redirect.toUrl());
         setSource(url);
         return;
      }
   }
   d->redirectCount = 0;

   if (d->sciReply->error() != QNetworkReply::NoError) {
      d->status = Error;
      d->sciReply->deleteLater();
      d->sciReply = 0;
      emit statusChanged(d->status);
   } else {
      QDeclarativeGridScaledImage sci(d->sciReply);
      d->sciReply->deleteLater();
      d->sciReply = 0;
      setGridScaledImage(sci);
   }
}

void QDeclarativeBorderImage::doUpdate()
{
   update();
}

void QDeclarativeBorderImage::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
   Q_D(QDeclarativeBorderImage);
   if (d->pix.isNull() || d->width() <= 0.0 || d->height() <= 0.0) {
      return;
   }

   bool oldAA = p->testRenderHint(QPainter::Antialiasing);
   bool oldSmooth = p->testRenderHint(QPainter::SmoothPixmapTransform);
   QTransform oldTransform;
   if (d->smooth) {
      p->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, d->smooth);
   }
   if (d->mirror) {
      oldTransform = p->transform();
      QTransform mirror;
      mirror.translate(d->width(), 0).scale(-1, 1.0);
      p->setWorldTransform(mirror * oldTransform);
   }

   const QDeclarativeScaleGrid *border = d->getScaleGrid();
   int left = border->left();
   int right = border->right();
   qreal borderWidth = left + right;
   if (borderWidth > 0.0 && d->width() < borderWidth) {
      qreal diff = borderWidth - d->width() - 1;
      left -= qRound(diff * qreal(left) / borderWidth);
      right -= qRound(diff * qreal(right) / borderWidth);
   }
   int top = border->top();
   int bottom = border->bottom();
   qreal borderHeight = top + bottom;
   if (borderHeight > 0.0 && d->height() < borderHeight) {
      qreal diff = borderHeight - d->height() - 1;
      top -= qRound(diff * qreal(top) / borderHeight);
      bottom -= qRound(diff * qreal(bottom) / borderHeight);
   }
   QMargins margins(left, top, right, bottom);
   QTileRules rules((Qt::TileRule)d->horizontalTileMode, (Qt::TileRule)d->verticalTileMode);
   qDrawBorderPixmap(p, QRect(0, 0, (int)d->width(), (int)d->height()), margins, d->pix, d->pix.rect(), margins, rules);
   if (d->smooth) {
      p->setRenderHint(QPainter::Antialiasing, oldAA);
      p->setRenderHint(QPainter::SmoothPixmapTransform, oldSmooth);
   }
   if (d->mirror) {
      p->setWorldTransform(oldTransform);
   }
}

QT_END_NAMESPACE
