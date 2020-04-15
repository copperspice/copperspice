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

// do not move include, if qvarient.h is included directly forward declarations are not sufficient 12/30/2013
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
      String8       = String,
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

      Variant,
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
   QVariant();


   // force compile error
   QVariant(const char *)    = delete;
   QVariant(void *)          = delete;
   QVariant(int typeOrUserType, const void *copy);
   QVariant(int typeOrUserType, const void *copy, uint flags);

   QVariant(const QVariant &other);
   QVariant(QDataStream &stream);

   QVariant(Type type);

   QVariant(bool value);
   QVariant(int value);
   QVariant(uint value);
   QVariant(qint64 value);
   QVariant(quint64 value);
   QVariant(double value);
   ~QVariant();

   void clear();
   QVariant(float value) {
      d.is_null = false;
      d.type = QMetaType::Float;
      d.data.f = value;
   }

   bool convert(Type t);
   QVariant(const QByteArray &bytearray);
   QVariant(const QBitArray &bitarray);

   QVariant(const QChar32 &ch);
   QVariant(const QString8 &string);
   QVariant(const QString16 &string);

   QVariant(const QRegularExpression8 &regExp);

   QVariant(const QDate &date);
   QVariant(const QTime &time);
   QVariant(const QDateTime &datetime);

   QVariant(const QList<QVariant> &list);
   QVariant(const QStringList &stringList);

   QVariant(const QMap<QString, QVariant> &map);
   QVariant(const QHash<QString, QVariant> &hash);
   QVariant(const QMultiMap<QString, QVariant> &map);
   QVariant(const QMultiHash<QString, QVariant> &hash);

   QVariant(const QSize &size);
   QVariant(const QSizeF &size);
   QVariant(const QPoint &point);
   QVariant(const QPointF &point);
   QVariant(const QLine &line);
   QVariant(const QLineF &line);
   QVariant(const QRect &rect);
   QVariant(const QRectF &rect);

   QVariant(const QLocale &locale);

   QVariant(const QUrl &url);
   QVariant(const QEasingCurve &easing);
   QVariant(const QUuid &uuid);
   QVariant(const QModelIndex &modelIndex);
   QVariant(const QJsonValue &jsonValue);
   QVariant(const QJsonObject &jsonObject);
   QVariant(const QJsonArray &jsonArray);
   QVariant(const QJsonDocument &jsonDocument);

   QVariant &operator=(const QVariant &other);
   QVariant(Qt::GlobalColor color);

   inline QVariant &operator=(QVariant && other) {
      qSwap(d, other.d);
      return *this;
   }

   inline void swap(QVariant &other) {
      qSwap(d, other.d);
   }

   void load(QDataStream &ds);
   void save(QDataStream &ds) const;
   int userType() const;
   const QString &typeName() const;

   bool canConvert(Type t) const;
   template<typename T>
   void setValue(const T &value);

   void setValue(const QVariant &value);
   inline bool isValid() const;
   bool isNull() const;


   bool toBool() const;
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

   template<typename T>
   T value() const;

   static const QString &typeToName(Type type);
   static Type nameToType(const QString &name);

   void *data();
   const void *constData() const;

   Type type() const;
   inline const void *data() const {
      return constData();
   }

   template<typename T>
   static inline QVariant fromValue(const T &value) {
      return QVariant(qMetaTypeId<T>(static_cast<T *>(nullptr)), &value, std::is_pointer_v<T>);
   }

   static inline QVariant fromValue(const QVariant &value) {
      return value;
   }

   template<typename T>
   bool canConvert() const {
      return canConvert(Type(qMetaTypeId<T>()));
   }
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









   inline bool operator==(const QVariant &other) const {
      return cmp(other);
   }

   inline bool operator!=(const QVariant &other) const {
      return ! cmp(other);
   }


 protected:
   friend int qRegisterGuiVariant();
   friend int qUnregisterGuiVariant();
   friend Q_CORE_EXPORT QDebug operator<<(QDebug, const QVariant &);
   //
   template <typename T>
   static uint registerType();



   void create(int type, const void *copy);
   bool cmp(const QVariant &other) const;

 private:
   bool clearRequired() const;

   static uint getTypeId(const std::type_index &index);
   static uint getTypeId(QString name);

   static std::atomic<uint> &currentUserType();

   static QVector<NamesAndTypes> m_userTypes;
   // force compile error, prevent QVariant(QVariant::Type, int) to be called
   QVariant(bool, int) = delete;
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


template <typename T>
QVariant::Type qMetaTypeVariant(T * = nullptr)
{
   return static_cast<QVariant::Type>(QMetaTypeId2<T>::qt_metatype_id());
}

//
inline QVariant::QVariant() {}

inline bool QVariant::isValid() const
{
   return d.type != Invalid;
}

template<typename T>
inline void QVariant::setValue(const T &v)
{
   // optimize when std::any is added

   const uint type = qMetaTypeId<T>(static_cast<T *>(nullptr));
   *this = QVariant(type, &v, std::is_pointer_v<T>);
}

inline void QVariant::setValue(const QVariant &v)
{
   *this = v;
}









template<typename T>
T QVariant::value() const
{
   const int id = qMetaTypeId<T>(static_cast<T *>(0));

   if (id == userType()) {
      return *reinterpret_cast<const T *>(this->constData());
   }

   if (id < int(QMetaType::User)) {
      T t;

      if (qvariant_cast_helper(*this, QVariant::Type(id), &t)) {
         return t;
      }
   }

   return T();
}

template<typename T>
T qvariant_cast(const QVariant &x)
{
   (void) x;
   static_assert(! std::is_same_v<T, T>, "qvariant_cast<T>(x) is obsolete, use x.value<T>()");

   return T();
}

template <typename T>
uint QVariant::registerType()
{
   static std::atomic<uint> userId = QVariant::Invalid;

   if (userId.load(std::memory_order_relaxed) == QVariant::Invalid) {
      uint newId = QVariant::currentUserType().fetch_add(1, std::memory_order_relaxed);
      uint oldId = QVariant::Invalid;

      if (userId.compare_exchange_strong(oldId, newId, std::memory_order_release, std::memory_order_acquire))  {
         static QString typeName = cs_typeToName<T>();
         m_userTypes.append(QVariant::NamesAndTypes{typeName.constData(), newId, typeid(T *)});

      } else {
         // already registered, maybe on a different thread
         return oldId;
      }
   }

   return userId.load(std::memory_order_acquire);
};

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
