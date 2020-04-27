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

#include <optional>
#include <variant>

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
class QBitmap;
class QBrush;
class QColor;
class QCursor;
class QFont;
class QIcon;
class QImage;
class QKeySequence;
class QMatrix;
class QMatrix4x4;
class QPalette;
class QPen;
class QPixmap;
class QPolygon;
class QPolygonF;
class QQuaternion;
class QRegion;
class QSizePolicy;
class QTextFormat;
class QTextLength;
class QTransform;
class QVector2D;
class QVector3D;
class QVector4D;
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

   QVariant()
   { }

   QVariant(const char *)    = delete;      // prevents storing a "string literal"
   QVariant(void *)          = delete;      // prevents storing a "pointer" as a bool
   QVariant(bool, int)       = delete;      // prevents issue when using 2 parameter QVariant constructor

   QVariant(Qt::GlobalColor) = delete;
   QVariant(Qt::BrushStyle)  = delete;
   QVariant(Qt::PenStyle)    = delete;
   QVariant(Qt::CursorShape) = delete;

   QVariant(Type type);
   QVariant(uint typeId, const void *copy);

   QVariant(const QVariant &other);
   QVariant(QDataStream &stream);

   QVariant(QVariant &&other)
      : m_data(std::move(other.m_data))
   { }

   QVariant(bool value);
   QVariant(int value);
   QVariant(uint value);
   QVariant(qint64 value);
   QVariant(quint64 value);
   QVariant(double value);
   QVariant(float value);

   QVariant(QChar32 value);
   QVariant(QString value);

   QVariant(QByteArray value);
   QVariant(QBitArray value);
   QVariant(QString16 value);
   QVariant(QStringList value);
   QVariant(QRegularExpression8 value);

   QVariant(QDate value);
   QVariant(QTime value);
   QVariant(QDateTime value);
   QVariant(QLocale value);

   QVariant(QList<QVariant> value);
   QVariant(QMap<QString, QVariant> value);
   QVariant(QHash<QString, QVariant> value);
   QVariant(QMultiMap<QString, QVariant> value);
   QVariant(QMultiHash<QString, QVariant> value);

   QVariant(QJsonValue value);
   QVariant(QJsonObject value);
   QVariant(QJsonArray value);
   QVariant(QJsonDocument value);

   QVariant(QRect value);
   QVariant(QRectF value);
   QVariant(QSize value);
   QVariant(QSizeF value);
   QVariant(QLine value);
   QVariant(QLineF value);
   QVariant(QPoint value);
   QVariant(QPointF value);

   QVariant(QEasingCurve value);
   QVariant(QModelIndex value);
   QVariant(QPersistentModelIndex value);
   QVariant(QUuid value);
   QVariant(QUrl value);

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





   class CustomType {
      public:
         virtual ~CustomType()
         { }

         virtual std::shared_ptr<CustomType> clone() const = 0;

         virtual bool compare(const CustomType &other) const = 0;
         virtual uint userType() const = 0;

         virtual void saveToStream()   = 0;
         virtual void loadFromStream() = 0;
   };




 protected:
   friend int qRegisterGuiVariant();
   friend int qUnregisterGuiVariant();
   friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QVariant &);

 private:




   static uint getTypeId(const std::type_index &index);
   static uint getTypeId(QString name);

   static std::atomic<uint> &currentUserType();


   std::variant <std::monostate, bool, char, int, uint, qint64, quint64, double, float,
                 QChar32, QString, QObject *, void *, std::shared_ptr<CustomType> > m_data;
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
//
template <typename T>
class CustomType_T : public QVariant::CustomType
{
 public:
   CustomType_T(const T &value)
      : m_value(value)
   {
   }

   CustomType_T(T && value)
      : m_value(std::move(value))
   {
   }

   std::shared_ptr<CustomType> clone() const {
      return std::make_shared<CustomType_T<T>>(m_value);
   }

   const T &get() const {
      return m_value;
   }

   bool compare(const CustomType &other) const override {

      if constexpr(std::is_invocable_v<compare_test, T>) {
         auto ptr = dynamic_cast<const CustomType_T<T>*>(&other);

         if (ptr != nullptr) {
            return (m_value == ptr->m_value);
         }
      }

      return false;
   }

   uint userType() const override {
      return QVariant::typeToTypeId<T>();
   }

   void saveToStream() override {
      // emerald, might add this
   }

   void loadFromStream() override {
      // emerald, might add this
   }

 private:
   T m_value;

   struct compare_test {
      template <typename U>
      auto operator()(const U &value) -> decltype(value == value);
   };
};

template <typename T>
constexpr bool isType_Simple()
{
   if constexpr(std::is_same_v<T, std::monostate>  ||
         std::is_same_v<T, bool>      || std::is_same_v<T, char>    ||
         std::is_same_v<T, int>       || std::is_same_v<T, uint>    ||
         std::is_same_v<T, qint64>    || std::is_same_v<T, quint64> ||
         std::is_same_v<T, double>    || std::is_same_v<T, float>   ||
         std::is_same_v<T, QChar>     || std::is_same_v<T, QString> ||
         std::is_same_v<T, QObject *> || std::is_same_v<T, void *>)  {

      return true;
   }

   return false;
}


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
