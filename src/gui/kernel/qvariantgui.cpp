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

#include <qvariantgui_p.h>

#include <qbitarray.h>
#include <qbitmap.h>
#include <qbytearray.h>
#include <qbrush.h>
#include <qcolor.h>
#include <qcursor.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qdebug.h>
#include <qeasingcurve.h>
#include <qfont.h>
#include <qicon.h>
#include <qimage.h>
#include <qkeysequence.h>
#include <qline.h>
#include <qlist.h>
#include <qlocale.h>
#include <qmap.h>
#include <qmatrix.h>
#include <qmatrix4x4.h>
#include <qpen.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qquaternion.h>
#include <qrect.h>
#include <qregion.h>
#include <qsize.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qsizepolicy.h>
#include <qtextformat.h>
#include <qtransform.h>
#include <qvariant.h>
#include <qvector2d.h>
#include <qvector3d.h>
#include <qvector4d.h>
#include <qurl.h>
#include <quuid.h>

bool QVariantGui::cs_internal_convert(uint current_userType, uint new_userType, QVariant &self) const
{
   bool retval = true;

   switch (new_userType) {

      case QVariant::ByteArray:
         if (current_userType == QVariant::Color) {
            self.setValue<QByteArray>(self.getData<QColor>().name().toLatin1());
         } else {
            retval = false;
         }

         break;

      case QVariant::String: {
         switch (current_userType) {

#ifndef QT_NO_SHORTCUT
            case QVariant::KeySequence:
               self.setValue<QString>(self.getData<QKeySequence>().toString(QKeySequence::NativeText));
               break;
#endif

            case QVariant::Font:
               self.setValue<QString>(self.getData<QFont>().toString());
               break;

            case QVariant::Color:
               self.setValue<QString>(self.getData<QColor>().name());
               break;

            default:
               break;
         }

         break;
      }

      case QVariant::Pixmap:
         if (current_userType == QVariant::Image) {
            self.setValue<QPixmap>(QPixmap::fromImage(self.getData<QImage>()));

         } else if (current_userType == QVariant::Bitmap) {
            self.setValue<QPixmap>(self.getData<QBitmap>());

         } else if (current_userType == QVariant::Brush) {
            QBrush tmp = self.getData<QBrush>();

            if (tmp.style() == Qt::TexturePattern) {
               self.setValue<QPixmap>(tmp.texture());
           }
         }

         break;

      case QVariant::Image:
         if (current_userType == QVariant::Pixmap) {
            self.setValue<QImage>(self.getData<QPixmap>().toImage());

         } else if (current_userType == QVariant::Bitmap) {
            self.setValue<QImage>(self.getData<QBitmap>().toImage());

         } else {
            retval = false;
         }

         break;

      case QVariant::Bitmap:
         if (current_userType == QVariant::Pixmap) {
            self.setValue<QBitmap>(self.getData<QPixmap>());

         } else if (current_userType == QVariant::Image) {
            self.setValue<QBitmap>(QBitmap::fromImage(self.getData<QImage>()));

         }

         break;

#ifndef QT_NO_SHORTCUT
      case QVariant::Int:
         if (current_userType == QVariant::KeySequence) {
            QKeySequence tmp = self.getData<QKeySequence>();

            if (tmp.isEmpty()) {
               self.setValue<int>(0);
            } else {
               self.setValue<int>(tmp[0]);
            }
         }

         break;
#endif

      case QVariant::Font:
         if (current_userType == QVariant::String) {
            QFont tmp;
            tmp.fromString(self.getData<QString>());

            self.setValue<QFont>(tmp);
         }

         break;

      case QVariant::Color:
         if (current_userType == QVariant::String) {
            QColor tmp;

            tmp.setNamedColor(self.getData<QString>());
            self.setValue<QColor>(tmp);

            retval = tmp.isValid();

         } else if (current_userType == QVariant::ByteArray) {
              QColor tmp;

              tmp.setNamedColor(QString::fromLatin1(self.getData<QByteArray>()));
              self.setValue<QColor>(tmp);

         } else if (current_userType == QVariant::Brush) {
            QBrush tmp = self.getData<QBrush>();

            if (tmp.style() == Qt::SolidPattern) {
               self.setValue<QColor>(tmp.color());
            }
         }

         break;

      case QVariant::Brush:
         if (current_userType == QVariant::Color) {
            self.setValue<QBrush>(self.getData<QColor>());

         } else if (current_userType == QVariant::Pixmap) {
            self.setValue<QBrush>(self.getData<QPixmap>());
         }

         break;

#ifndef QT_NO_SHORTCUT
      case QVariant::KeySequence: {
         switch (current_userType) {
            case QVariant::String:
               self.setValue<QKeySequence>(self.getData<QString>());
               break;

            case QVariant::Int:
               self.setValue<QKeySequence>(self.getData<int>());
               break;

            default:
               break;
         }
      }
#endif

      default:
         retval = false;
         break;

   }

   return retval;
}

bool QVariantGui::cs_internal_create(uint newUserType, const void *other, QVariant &self) const
{
   switch (newUserType) {

      case QVariant::Bitmap:

         if (other == nullptr) {
            self.setValue<QBitmap>(QBitmap());
         } else {
            self.setValue<QBitmap>(*static_cast<const QBitmap *>(other) );
         }

         break;

      case QVariant::Region:

         if (other == nullptr) {
            self.setValue<QRegion>(QRegion());
         } else {
            self.setValue<QRegion>(*static_cast<const QRegion *>(other) );
         }

         break;

      case QVariant::Polygon:

         if (other == nullptr) {
            self.setValue<QPolygon>(QPolygon());
         } else {
            self.setValue<QPolygon>(*static_cast<const QPolygon *>(other) );
         }

         break;

      case QVariant::Font:

         if (other == nullptr) {
            self.setValue<QFont>(QFont());
         } else {
            self.setValue<QFont>(*static_cast<const QFont *>(other) );
         }

         break;

      case QVariant::Pixmap:

         if (other == nullptr) {
            self.setValue<QPixmap>(QPixmap());
         } else {
            self.setValue<QPixmap>(*static_cast<const QPixmap *>(other) );
         }

         break;

      case QVariant::Image:

         if (other == nullptr) {
            self.setValue<QImage>(QImage());
         } else {
            self.setValue<QImage>(*static_cast<const QImage *>(other) );
         }

         break;

      case QVariant::Brush:

         if (other == nullptr) {
            self.setValue<QBrush>(QBrush());
         } else {
            self.setValue<QBrush>(*static_cast<const QBrush *>(other) );
         }

         break;

      case QVariant::Color:

         if (other == nullptr) {
            self.setValue<QColor>(QColor());
         } else {
            self.setValue<QColor>(*static_cast<const QColor *>(other) );
         }

         break;

      case QVariant::Palette:

         if (other == nullptr) {
            self.setValue<QPalette>(QPalette());
         } else {
            self.setValue<QPalette>(*static_cast<const QPalette *>(other) );
         }

         break;

#ifndef QT_NO_ICON
      case QVariant::Icon:

         if (other == nullptr) {
            self.setValue<QIcon>(QIcon());
         } else {
            self.setValue<QIcon>(*static_cast<const QIcon *>(other) );
         }

         break;
#endif

      case QVariant::Matrix:
         if (other == nullptr) {
            self.setValue<QMatrix>(QMatrix());
         } else {
            self.setValue<QMatrix>(*static_cast<const QMatrix *>(other) );
         }

         break;

      case QVariant::Transform:
         if (other == nullptr) {
            self.setValue<QTransform>(QTransform());
         } else {
            self.setValue<QTransform>(*static_cast<const QTransform *>(other) );
         }

         break;

      case QVariant::TextFormat:
         if (other == nullptr) {
            self.setValue<QTextFormat>(QTextFormat());
         } else {
            self.setValue<QTextFormat>(*static_cast<const QTextFormat *>(other) );
         }

         break;

      case QVariant::TextLength:
         if (other == nullptr) {
            self.setValue<QTextLength>(QTextLength());
         } else {
            self.setValue<QTextLength>(*static_cast<const QTextLength *>(other) );
         }

         break;

#ifndef QT_NO_SHORTCUT
      case QVariant::KeySequence:
         if (other == nullptr) {
            self.setValue<QKeySequence>(QKeySequence());
         } else {
            self.setValue<QKeySequence>(*static_cast<const QKeySequence *>(other) );
         }

         break;
#endif

      case QVariant::Pen:
         if (other == nullptr) {
            self.setValue<QPen>(QPen());
         } else {
            self.setValue<QPen>(*static_cast<const QPen *>(other) );
         }

         break;

      case QVariant::SizePolicy:
         if (other == nullptr) {
            self.setValue<QSizePolicy>(QSizePolicy());
         } else {
            self.setValue<QSizePolicy>(*static_cast<const QSizePolicy *>(other) );
         }

         break;

#ifndef QT_NO_CURSOR
      case QVariant::Cursor:
         if (other == nullptr) {
            self.setValue<QCursor>(QCursor());
         } else {
            self.setValue<QCursor>(*static_cast<const QCursor *>(other) );
         }

         break;
#endif

#ifndef QT_NO_MATRIX4X4
      case QVariant::Matrix4x4:
         if (other == nullptr) {
            self.setValue<QMatrix4x4>(QMatrix4x4());
         } else {
            self.setValue<QMatrix4x4>(*static_cast<const QMatrix4x4 *>(other) );
         }

         break;
#endif

#ifndef QT_NO_VECTOR2D
      case QVariant::Vector2D:
         if (other == nullptr) {
            self.setValue<QVector2D>(QVector2D());
         } else {
            self.setValue<QVector2D>(*static_cast<const QVector2D *>(other) );
         }

         break;
#endif

#ifndef QT_NO_VECTOR3D
      case QVariant::Vector3D:
         if (other == nullptr) {
            self.setValue<QVector3D>(QVector3D());
         } else {
            self.setValue<QVector3D>(*static_cast<const QVector3D *>(other) );
         }

         break;
#endif

#ifndef QT_NO_VECTOR4D
      case QVariant::Vector4D:
         if (other == nullptr) {
            self.setValue<QVector4D>(QVector4D());
         } else {
            self.setValue<QVector4D>(*static_cast<const QVector4D *>(other) );
         }

         break;
#endif

#ifndef QT_NO_QUATERNION
      case QVariant::Quaternion:
         if (other == nullptr) {
            self.setValue<QQuaternion>(QQuaternion());
         } else {
            self.setValue<QQuaternion>(*static_cast<const QQuaternion *>(other) );
         }

         break;
#endif

      default:
         // no matching case
         return false;
   }

   return true;
}
