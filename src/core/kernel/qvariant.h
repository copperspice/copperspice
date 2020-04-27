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

// do not move include, if qvarient.h is included directly forward declarations are not sufficient
#include <qobject.h>

#ifndef QVARIANT_H
#define QVARIANT_H

#include <csmetafwd.h>

// can not include qstring.h since it includes qstringparser.h, which then includes qlocale.h (circular dependency)
#include <qstring8.h>

#include <qatomic.h>
#include <qbytearray.h>
#include <qcontainerfwd.h>
#include <qlist.h>
#include <qmap.h>
#include <qmetatype.h>
#include <qnamespace.h>
#include <qvector.h>


class QDataStream;
class QDebug;
class QObject;
class QVariant;

class QBitArray;
class QStringList;

class QDate;
class QDateTime;
class QTime;
class QLocale;

class QJsonArray;
class QJsonDocument;
class QJsonObject;
class QJsonValue;

class QLine;
class QLineF;
class QPoint;
class QPointF;
class QRect;
class QRectF;
class QSize;
class QSizeF;

// core
class QEasingCurve;
class QModelIndex;
class QPersistentModelIndex;
class QUuid;
class QUrl;

// gui
class QMatrix;
class QTextFormat;
class QTextLength;
class QTransform;
class QWidget;


class Q_CORE_EXPORT QVariant
{
 public:

   enum Type {
      Invalid,

      Bool,
      Short,
      UShort,
      Int,
      UInt,
      Long,
      ULong,
      LongLong,
      ULongLong,
      Double,
      Float,

      QChar,
      Char,
      SChar,
      UChar,
      Char8_t,
      Char16_t,
      Char32_t,

      BitArray,
      ByteArray,
      String,
      String8 = String,
      String16,
      StringList,
      StringView,
      RegularExpression,

      Date,
      Time,
      DateTime,
      Locale,

      JsonValue,
      JsonArray,
      JsonObject,
      JsonDocument,

      Line,
      LineF,
      Point,
      PointF,
      Polygon,
      PolygonF,
      Rect,
      RectF,
      Size,
      SizeF,

      Hash,
      List,
      Map,
      MultiHash,
      MultiMap,

      Void,
      VoidStar,
      ObjectStar,
      WidgetStar,

      // core
      EasingCurve,
      ModelIndex,
      PersistentModelIndex,
      Url,
      Uuid,

      // gui
      Bitmap,
      Brush,
      Color,
      Cursor,
      Font,
      Icon,
      Image,
      KeySequence,
      Matrix,
      Matrix4x4,
      Palette,
      Pen,
      Pixmap,
      Quaternion,
      Region,
      SizePolicy,
      TextLength,
      TextFormat,
      Transform,
      Vector2D,
      Vector3D,
      Vector4D,

      // temporary code, delete after transistion
      Char32               = QChar,
      FirstConstructedType = QChar,
      // used in QMetaProperty
      Variant,

      // must always be after all declared types
      UserType   = 256,

      // force compiler to allocate 32 bits
      LastType   = 0xffffffff
   };

   struct NamesAndTypes {
      const char *meta_typeName;
      uint meta_typeId;
      std::type_index meta_typeT;
   };



   QVariant(const QVariant &other);
   QVariant(QDataStream &stream);


   QVariant(bool value);
   QVariant(int value);
   QVariant(uint value);
   QVariant(qint64 value);
   QVariant(quint64 value);
   QVariant(double value);
   ~QVariant();

   void clear();












   template<typename T>
   void setValue(const T &value);

   void setValue(const QVariant &value);


   int toInt(bool *ok = nullptr) const;
   uint toUInt(bool *ok = nullptr) const;
   qint64 toLongLong(bool *ok = nullptr) const;
   quint64 toULongLong(bool *ok = nullptr) const;
   double toDouble(bool *ok = nullptr) const;
   float toFloat(bool *ok = nullptr) const;
   qreal toReal(bool *ok = nullptr) const;

   QByteArray toByteArray() const;
   QBitArray toBitArray() const;

   QChar32 toChar() const;
   QString8 toString() const;
   QString16 toString16() const;

   QStringList toStringList() const;
   QRegularExpression8 toRegularExpression() const;

   QDate toDate() const;
   QTime toTime() const;
   QDateTime toDateTime() const;
   QLocale toLocale() const;

   QJsonValue toJsonValue() const;
   QJsonObject toJsonObject() const;
   QJsonArray toJsonArray() const;
   QJsonDocument toJsonDocument() const;

   QList<QVariant> toList() const;
   QMap<QString, QVariant> toMap() const;
   QHash<QString, QVariant> toHash() const;
   QMultiMap<QString, QVariant> toMultiMap() const;
   QMultiHash<QString, QVariant> toMultiHash() const;

   QPoint toPoint() const;
   QPointF toPointF() const;
   QRect toRect() const;
   QSize toSize() const;
   QSizeF toSizeF() const;
   QLine toLine() const;
   QLineF toLineF() const;
   QRectF toRectF() const;

   QEasingCurve toEasingCurve() const;
   QModelIndex toModelIndex() const;
   QUrl toUrl() const;
   QUuid toUuid() const;




   Type type() const;


   template<typename T>
   static uint typeToTypeId()
   {
      // typeid() part of RTTI, core language
      uint retval = QVariant::getTypeId(typeid(T *));

      if (retval == QVariant::Invalid) {
         // T is a user defined data type

         // auto register and generate a type id for the given T
         retval = QVariant::registerType<T>();
      }

      return retval;
   };












 protected:
   friend int qRegisterGuiVariant();
   friend int qUnregisterGuiVariant();
   friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QVariant &);




 private:

   static uint getTypeId(const std::type_index &index);
   static uint getTypeId(QString name);

   static std::atomic<uint> &currentUserType();

};

using QVariantList      = QList<QVariant>;
using QVariantMap       = QMap<QString, QVariant>;
using QVariantHash      = QHash<QString, QVariant>;
using QVariantMultiMap  = QMultiMap<QString, QVariant>;
using QVariantMultiHash = QMultiHash<QString, QVariant>;

Q_CORE_EXPORT QDataStream &operator>> (QDataStream &s, QVariant &p);
Q_CORE_EXPORT QDataStream &operator<< (QDataStream &s, const QVariant &p);
Q_CORE_EXPORT QDataStream &operator>> (QDataStream &s, QVariant::Type &p);
Q_CORE_EXPORT QDataStream &operator<< (QDataStream &s, const QVariant::Type p);




#define CS_DECLARE_METATYPE(TYPE)                  \
   template<>                                      \
   inline const QString &cs_typeToName<TYPE>() {   \
      static const QString retval = #TYPE;         \
      return retval;                               \
   }

#define Q_DECLARE_METATYPE(TYPE)                   \
   static_assert(false, "Macro Q_DECLARE_METATYPE(TYPE) is obsolete, use CS_DECLARE_METATYPE(TYPE)")
Q_CORE_EXPORT QDebug operator<<(QDebug, const QVariant &);
Q_CORE_EXPORT QDebug operator<<(QDebug, const QVariant::Type);

#endif
