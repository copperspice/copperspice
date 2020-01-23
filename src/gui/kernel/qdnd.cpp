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

#include <qplatformdefs.h>
#include <qbitmap.h>
#include <qdrag.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qfile.h>
#include <qtextcodec.h>
#include <qapplication.h>
#include <qpoint.h>

#include <qbuffer.h>
#include <qimage.h>
#include <qpainter.h>
#include <qregularexpression.h>
#include <qdir.h>
#include <qdnd_p.h>
#include <qimagereader.h>
#include <qimagewriter.h>
#include <qplatform_integration.h>
#include <qplatform_drag.h>
#include <qguiapplication_p.h>

#include <ctype.h>

#ifndef QT_NO_DRAGANDDROP

// the universe's only drag manager
QDragManager *QDragManager::m_instance = 0;

QDragManager::QDragManager()
   : QObject(qApp), m_platformDropData(0), m_currentDropTarget(0),
     m_platformDrag(QGuiApplicationPrivate::platformIntegration()->drag()), m_object(0)
{
   Q_ASSERT(! m_instance);

   if (m_platformDrag) {
      m_platformDropData = m_platformDrag->platformDropData();
   }

}

QDragManager::~QDragManager()
{
   m_instance = 0;

}

QDragManager *QDragManager::self()
{
   if (!m_instance && !QGuiApplication::closingDown()) {
      m_instance = new QDragManager;
   }
   return m_instance;
}

QObject *QDragManager::source() const
{
   if (m_object) {
      return m_object->source();
   }
   return 0;
}

void QDragManager::setCurrentTarget(QObject *target, bool dropped)
{
   if (m_currentDropTarget == target) {
      return;
   }

   m_currentDropTarget = target;
   if (!dropped && m_object) {
      m_object->d_func()->target = target;
      emit m_object->targetChanged(target);
   }
}

QObject *QDragManager::currentTarget() const
{
   return m_currentDropTarget;
}

Qt::DropAction QDragManager::drag(QDrag *objDrag)
{
   if (! objDrag || m_object == objDrag) {
      return Qt::IgnoreAction;
   }

   if (! m_platformDrag || ! objDrag->source()) {
      objDrag->deleteLater();
      return Qt::IgnoreAction;
   }

   if (m_object) {
      qWarning("QDragManager::drag in possibly invalid state");
      return Qt::IgnoreAction;
   }

   m_object = objDrag;
   m_object->d_func()->target = nullptr;

   QGuiApplicationPrivate::instance()->notifyDragStarted(objDrag);

   const Qt::DropAction result = m_platformDrag->drag(m_object);
   m_object = nullptr;

   if (! m_platformDrag->ownsDragObject()) {
      objDrag->deleteLater();
   }

   return result;
}

#endif // QT_NO_DRAGANDDROP

#if !(defined(QT_NO_DRAGANDDROP) && defined(QT_NO_CLIPBOARD))

static QStringList imageReadMimeFormats()
{
   QStringList formats;
   QList<QByteArray> imageFormats = QImageReader::supportedImageFormats();

   for (int i = 0; i < imageFormats.size(); ++i) {
      QString format = "image/";
      format += QString::fromLatin1(imageFormats.at(i).toLower());
      formats.append(format);
   }

   // put png at the front because it is best
   int pngIndex = formats.indexOf(QLatin1String("image/png"));
   if (pngIndex != -1 && pngIndex != 0) {
      formats.move(pngIndex, 0);
   }

   return formats;
}


static QStringList imageWriteMimeFormats()
{
   QStringList formats;
   QList<QByteArray> imageFormats = QImageWriter::supportedImageFormats();
   for (int i = 0; i < imageFormats.size(); ++i) {
      QString format = QLatin1String("image/");
      format += QString::fromLatin1(imageFormats.at(i).toLower());
      formats.append(format);
   }

   //put png at the front because it is best
   int pngIndex = formats.indexOf(QLatin1String("image/png"));
   if (pngIndex != -1 && pngIndex != 0) {
      formats.move(pngIndex, 0);
   }

   return formats;
}

QInternalMimeData::QInternalMimeData()
   : QMimeData()
{
}

QInternalMimeData::~QInternalMimeData()
{
}

bool QInternalMimeData::hasFormat(const QString &mimeType) const
{
   bool foundFormat = hasFormat_sys(mimeType);
   if (!foundFormat && mimeType == QLatin1String("application/x-qt-image")) {
      QStringList imageFormats = imageReadMimeFormats();
      for (int i = 0; i < imageFormats.size(); ++i) {
         if ((foundFormat = hasFormat_sys(imageFormats.at(i)))) {
            break;
         }
      }
   }
   return foundFormat;
}

QStringList QInternalMimeData::formats() const
{
   QStringList realFormats = formats_sys();
   if (!realFormats.contains(QLatin1String("application/x-qt-image"))) {
      QStringList imageFormats = imageReadMimeFormats();
      for (int i = 0; i < imageFormats.size(); ++i) {
         if (realFormats.contains(imageFormats.at(i))) {
            realFormats += QLatin1String("application/x-qt-image");
            break;
         }
      }
   }
   return realFormats;
}

QVariant QInternalMimeData::retrieveData(const QString &mimeType, QVariant::Type type) const
{
   QVariant data = retrieveData_sys(mimeType, type);
   if (mimeType == QLatin1String("application/x-qt-image")) {
      if (data.isNull() || (data.type() == QVariant::ByteArray && data.toByteArray().isEmpty())) {
         // try to find an image
         QStringList imageFormats = imageReadMimeFormats();
         for (int i = 0; i < imageFormats.size(); ++i) {
            data = retrieveData_sys(imageFormats.at(i), type);
            if (data.isNull() || (data.type() == QVariant::ByteArray && data.toByteArray().isEmpty())) {
               continue;
            }
            break;
         }
      }
      // we wanted some image type, but all we got was a byte array. Convert it to an image.
      if (data.type() == QVariant::ByteArray
         && (type == QVariant::Image || type == QVariant::Pixmap || type == QVariant::Bitmap)) {
         data = QImage::fromData(data.toByteArray());
      }

   } else if (mimeType == QLatin1String("application/x-color") && data.type() == QVariant::ByteArray) {
      QColor c;
      QByteArray ba = data.toByteArray();
      if (ba.size() == 8) {
         ushort *colBuf = (ushort *)ba.data();
         c.setRgbF(qreal(colBuf[0]) / qreal(0xFFFF),
            qreal(colBuf[1]) / qreal(0xFFFF),
            qreal(colBuf[2]) / qreal(0xFFFF),
            qreal(colBuf[3]) / qreal(0xFFFF));
         data = c;
      } else {
         qWarning("Qt: Invalid color format");
      }
   } else if (data.type() != type && data.type() == QVariant::ByteArray) {
      // try to use mime data's internal conversion stuf.
      QInternalMimeData *that = const_cast<QInternalMimeData *>(this);
      that->setData(mimeType, data.toByteArray());
      data = QMimeData::retrieveData(mimeType, type);
      that->clear();
   }
   return data;
}

bool QInternalMimeData::canReadData(const QString &mimeType)
{
   return imageReadMimeFormats().contains(mimeType);
}

// helper functions for rendering mimedata to the system, this is needed because QMimeData is in core.
QStringList QInternalMimeData::formatsHelper(const QMimeData *data)
{
   QStringList realFormats = data->formats();
   if (realFormats.contains(QLatin1String("application/x-qt-image"))) {
      // add all supported image formats
      QStringList imageFormats = imageWriteMimeFormats();
      for (int i = 0; i < imageFormats.size(); ++i) {
         if (!realFormats.contains(imageFormats.at(i))) {
            realFormats.append(imageFormats.at(i));
         }
      }
   }
   return realFormats;
}

bool QInternalMimeData::hasFormatHelper(const QString &mimeType, const QMimeData *data)
{

   bool foundFormat = data->hasFormat(mimeType);
   if (!foundFormat) {
      if (mimeType == QLatin1String("application/x-qt-image")) {
         // check all supported image formats
         QStringList imageFormats = imageWriteMimeFormats();
         for (int i = 0; i < imageFormats.size(); ++i) {
            if ((foundFormat = data->hasFormat(imageFormats.at(i)))) {
               break;
            }
         }
      } else if (mimeType.startsWith(QLatin1String("image/"))) {
         return data->hasImage() && imageWriteMimeFormats().contains(mimeType);
      }
   }
   return foundFormat;
}

QByteArray QInternalMimeData::renderDataHelper(const QString &mimeType, const QMimeData *data)
{
   QByteArray ba;
   if (mimeType == QLatin1String("application/x-color")) {
      /* QMimeData can only provide colors as QColor or the name
         of a color as a QByteArray or a QString. So we need to do
         the conversion to application/x-color here.
         The application/x-color format is :
         type: application/x-color
         format: 16
         data[0]: red
         data[1]: green
         data[2]: blue
         data[3]: opacity
      */
      ba.resize(8);
      ushort *colBuf = (ushort *)ba.data();
      QColor c = qvariant_cast<QColor>(data->colorData());
      colBuf[0] = ushort(c.redF() * 0xFFFF);
      colBuf[1] = ushort(c.greenF() * 0xFFFF);
      colBuf[2] = ushort(c.blueF() * 0xFFFF);
      colBuf[3] = ushort(c.alphaF() * 0xFFFF);
   } else {
      ba = data->data(mimeType);
      if (ba.isEmpty()) {
         if (mimeType == QLatin1String("application/x-qt-image") && data->hasImage()) {
            QImage image = qvariant_cast<QImage>(data->imageData());
            QBuffer buf(&ba);
            buf.open(QBuffer::WriteOnly);
            // would there not be PNG ??
            image.save(&buf, "PNG");

         } else if (mimeType.startsWith(QLatin1String("image/")) && data->hasImage()) {
            QImage image = qvariant_cast<QImage>(data->imageData());
            QBuffer buf(&ba);
            buf.open(QBuffer::WriteOnly);
            image.save(&buf, mimeType.mid(mimeType.indexOf('/') + 1).toLatin1().toUpper().constData());
         }
      }
   }
   return ba;
}

#endif // QT_NO_DRAGANDDROP && QT_NO_CLIPBOARD
