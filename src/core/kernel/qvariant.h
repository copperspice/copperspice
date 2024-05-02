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

// do not move include
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
#include <qnamespace.h>
#include <qvector.h>

#include <optional>
#include <variant>

// core
class QBitArray;
class QDataStream;
class QDate;
class QDateTime;
class QDebug;
class QEasingCurve;
class QJsonArray;
class QJsonDocument;
class QJsonObject;
class QJsonValue;
class QLine;
class QLineF;
class QLocale;
class QModelIndex;
class QObject;
class QPersistentModelIndex;
class QPoint;
class QPointF;
class QRect;
class QRectF;
class QSize;
class QSizeF;
class QStringList;
class QTime;
class QUrl;
class QUuid;
class QVariant;

// gui
class QBitmap;
class QBrush;
class QColor;
class QCursor;
class QFont;
class QIcon;
class QImage;
class QKeySequence;
class QMatrix4x4;
class QMatrix;
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

class Q_CORE_EXPORT QVariantBase
{
 public:
   virtual ~QVariantBase()
   { }

   virtual bool cs_internal_convert(uint current_userType, uint new_userType, QVariant &self) const = 0;
   virtual bool cs_internal_create(uint typeId, const void *other, QVariant &self) const = 0;
   virtual bool cs_internal_load(QDataStream &stream, uint type, QVariant &self) const = 0;
   virtual bool cs_internal_save(QDataStream &stream, uint type, const QVariant &self) const = 0;
};

class Q_CORE_EXPORT QVariant
{
 public:

   enum Type : uint {
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

      // used in QMetaProperty
      Variant,

      // must always be after all declared types
      UserType   = 256
   };

   struct NamesAndTypes {
      const char *meta_typeName;
      uint meta_typeId;
      std::type_index meta_typeT;
   };

   QVariant()
   {
   }

   QVariant(const char *)    = delete;      // prevents storing a "string literal"
   QVariant(void *)          = delete;      // prevents storing a "pointer" as a bool
   QVariant(bool, int)       = delete;      // prevents issue when using 2 parameter QVariant constructor

   QVariant(Qt::GlobalColor) = delete;
   QVariant(Qt::BrushStyle)  = delete;
   QVariant(Qt::PenStyle)    = delete;
   QVariant(Qt::CursorShape) = delete;

   QVariant(Type type);                      // used in CsDesigner
   QVariant(uint typeId, const void *copy);

   QVariant(const QVariant &other);

   QVariant(QVariant &&other) = default;

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
   QVariant(QHash<QString, QVariant> value);
   QVariant(QMap<QString, QVariant> value);
   QVariant(QMultiHash<QString, QVariant> value);
   QVariant(QMultiMap<QString, QVariant> value);

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

   QVariant(QDataStream &stream);

   ~QVariant() = default;

   void clear();

   bool canConvert(uint newType) const;

   template<typename T>
   bool canConvert() const {
      return canConvert( QVariant::typeToTypeId<T>() );
   }

   bool convert(uint newType);

   template<typename T>
   bool convert() {
      return convert( QVariant::typeToTypeId<T>() );
   }

   template<typename T>
   static QVariant fromValue(const T &value) {
      QVariant retval;
      retval.setValue<T>(value);

      return retval;
   }

   static QVariant fromValue(const QVariant &value) {
      return value;
   }

   template<typename T>
   T getData() const;

   template<typename T>
   std::optional<T> getDataOr() const;

   bool isValid() const {
      return ! std::holds_alternative<std::monostate>(m_data);
   }

   bool isEnum() const;
   int64_t enumToInt() const;
   uint64_t enumToUInt() const;

   std::optional<QVariant> maybeConvert(uint requested_type) const;

   template<typename T>
   void setValue(const T &value);

   void setValue(const QVariant &value);

   void swap(QVariant &other) {
      std::swap(m_data, other.m_data);
   }

   bool toBool(bool *ok = nullptr) const;
   int toInt(bool *ok = nullptr) const;
   uint toUInt(bool *ok = nullptr) const;
   long toLong(bool *ok = nullptr) const;
   ulong toULong(bool *ok = nullptr) const;
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
   QHash<QString, QVariant> toHash() const;
   QMap<QString, QVariant> toMap() const;
   QMultiHash<QString, QVariant> toMultiHash() const;
   QMultiMap<QString, QVariant> toMultiMap() const;

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
   QPersistentModelIndex toPersistentModelIndex() const;
   QUrl toUrl() const;
   QUuid toUuid() const;

   template<typename Requested>
   Requested value() const;

   // ** next 6 are a group
   QString typeName() const;

   Type type() const;
   uint userType() const;

   static uint nameToType(const QString &name);
   static QString typeToName(uint typeId);

   template<typename T>
   static uint typeToTypeId() {
      // typeid() part of RTTI, core language
      uint retval = QVariant::getTypeId(typeid(T *));

      if (retval == QVariant::Invalid) {
         // T is a user defined data type

         // auto register and generate a type id for the given T
         retval = QVariant::registerType<T>();
      }

      return retval;
   }

   // **
   QVariant &operator=(const QVariant &other);

   QVariant &operator=(QVariant && other) {
      swap(other);
      return *this;
   }

   bool operator==(const QVariant &other) const {
      return cs_internal_compare(other);
   }

   bool operator!=(const QVariant &other) const {
      return ! cs_internal_compare(other);
   }

   class CustomType
   {
    public:
      virtual ~CustomType()
      {
      }

      virtual std::shared_ptr<CustomType> clone() const = 0;
      virtual bool compare(const CustomType &other) const = 0;
      virtual bool isEnum() const = 0;

      virtual int64_t enumToInt() const = 0;
      virtual uint64_t enumToUInt() const = 0;

      virtual void loadFromStream() = 0;
      virtual void saveToStream()   = 0;

      virtual uint userType() const = 0;
   };

   //
   template <typename T>
   static uint registerType();

   static void registerClient(QVariantBase *ptr) {
      if (! m_variantClients.contains(ptr)) {
         m_variantClients.append(ptr);
      }
   }

   static void unRegisterClient(QVariantBase *ptr) {
      m_variantClients.removeOne(ptr);
   }

 protected:
   static bool compareValues(const QVariant &a, const QVariant &b);

 private:
   bool cs_internal_convert(uint current_userType, uint new_userType);
   void cs_internal_create(uint typeId, const void *other);
   bool cs_internal_compare(const QVariant &other) const;

   void load(QDataStream &stream);
   void save(QDataStream &stream) const;

   bool cs_internal_load(QDataStream &stream, uint type);
   bool cs_internal_save(QDataStream &stream, uint type) const;

   template <typename T>
   T cs_internal_VariantToType(QVariant::Type type, bool *ok = nullptr) const;

   static uint getTypeId(const std::type_index &index);
   static uint getTypeId(QString name);

   static QString getTypeName(uint typeId);
   static std::atomic<uint> &currentUserType();

   static QVector<NamesAndTypes>  m_userTypes;
   static QVector<QVariantBase *> m_variantClients;

   std::variant <std::monostate, bool, char, int, uint, qint64, quint64, double, float,
       QChar32, QString, QObject *, void *, std::shared_ptr<CustomType>> m_data;

   friend Q_CORE_EXPORT QDataStream &operator>> (QDataStream &stream, QVariant &data);
   friend Q_CORE_EXPORT QDataStream &operator<< (QDataStream &stream, const QVariant &data);
};

using QVariantList      = QList<QVariant>;
using QVariantHash      = QHash<QString, QVariant>;
using QVariantMap       = QMap<QString, QVariant>;
using QVariantMultiHash = QMultiHash<QString, QVariant>;
using QVariantMultiMap  = QMultiMap<QString, QVariant>;

Q_CORE_EXPORT QDataStream &operator>> (QDataStream &stream, QVariant &data);
Q_CORE_EXPORT QDataStream &operator>> (QDataStream &stream, QVariant::Type &typeId);

Q_CORE_EXPORT QDataStream &operator<< (QDataStream &stream, const QVariant &data);
Q_CORE_EXPORT QDataStream &operator<< (QDataStream &stream, const QVariant::Type typeId);

Q_CORE_EXPORT QDebug &operator<<(QDebug &debug, const QVariant &value);

//
template <class T>
struct cs_is_flag : public std::integral_constant<bool, false> {
};

template <class T>
struct cs_is_flag<QFlags<T>>
   : public std::integral_constant<bool, true> {
};

template <class T>
constexpr const bool cs_is_flag_v = cs_is_flag<T>::value;

//
template <class T>
struct cs_flagEnum {
};

template <class T>
struct cs_flagEnum<QFlags<T>> {
   using type = T;
};

template <class T>
using cs_flagEnum_t = typename cs_flagEnum<T>::type;

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

   const T &get() const {
      return m_value;
   }

   std::shared_ptr<CustomType> clone() const override {
      return std::make_shared<CustomType_T<T>>(m_value);
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

   bool isEnum() const override {
      return std::is_enum_v<T> || cs_is_flag_v<T>;
   }

   int64_t enumToInt() const override {
      if constexpr (std::is_enum_v<T>) {
         using SType = std::make_signed_t<std::underlying_type_t<T>>;
         return static_cast<int64_t>(static_cast<SType>(m_value));

      } else if constexpr (cs_is_flag_v<T>) {
         using SType = std::make_signed_t<std::underlying_type_t<cs_flagEnum_t<T>>>;
         return static_cast<int64_t>(static_cast<SType>(m_value));

      } else {
         return 0;

      }
   }

   uint64_t enumToUInt() const override {
      if constexpr (std::is_enum_v<T>) {
         return static_cast<uint64_t>(m_value);

      } else if constexpr (cs_is_flag_v<T>) {
         return static_cast<uint64_t>(m_value);

      } else {
         return 0;

      }
   }

   void loadFromStream() override {
      // emerald, might add this
   }

   void saveToStream() override {
      // emerald, might add this
   }

   uint userType() const override {
      return QVariant::typeToTypeId<T>();
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

template<typename T>
T QVariant::getData() const
{
   std::optional<T> retval = getDataOr<T>();

   if (retval.has_value()) {
      return retval.value();

   } else {
      return T();
   }
}

template<typename T>
std::optional<T> QVariant::getDataOr() const
{
   if constexpr(isType_Simple<T>())  {

      if (std::holds_alternative<T>(m_data)) {
         return std::get<T>(m_data);

      } else {
         // variant is empty or T is not the type in the variant
         // printf("\n QVariant::getDataOr  requested T = %s  variant holds = %s",
         //    typeid(T).name(), csPrintable(typeName()) );

      }

   } else {
      // custom type
      auto ptr = std::get_if<std::shared_ptr<CustomType>>(&m_data);

      if (ptr == nullptr) {
         // T is not the data type in the variant

      } else {
         auto newPtr = std::dynamic_pointer_cast<CustomType_T<T>>(*ptr);

         if (newPtr == nullptr) {
            // T is the wrong custom type
            // printf("\n QVariant::getDataOr  requested T = %s  variant holds = %s",
            //    typeid(T).name(), typeid(**ptr).name() );

         } else {
            return newPtr->get();

         }
      }
   }

   return std::optional<T>();
}

template <typename T>
void QVariant::setValue(const T &value)
{
   if constexpr(isType_Simple<T>())  {
      m_data = value;

   } else {
      m_data = std::make_shared<CustomType_T<T>>(value);
   }
}

inline void QVariant::setValue(const QVariant &value)
{
   *this = value;
}

template<typename Requested>
Requested QVariant::value() const
{
   if constexpr(isType_Simple<Requested>())  {

      if (std::holds_alternative<Requested>(m_data)) {
         return std::get<Requested>(m_data);

      } else {
         // variant is empty or Requested is not the type in the variant

      }

   } else {
      // custom type
      auto ptr = std::get_if<std::shared_ptr<CustomType>>(&m_data);

      if (ptr == nullptr) {
         // variant is empty or a simple type

      } else {
         auto newPtr = std::dynamic_pointer_cast<CustomType_T<Requested>>(*ptr);

         if (newPtr == nullptr) {
            // variant and Requested are not the same custom type

         } else {
            return newPtr->get();

         }
      }
   }

   uint requested_type = typeToTypeId<Requested>();
   std::optional<QVariant> tmp = maybeConvert(requested_type);

   if (tmp.has_value()) {
      return tmp->getData<Requested>();
   }

   return Requested();
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

      std::atomic<uint> &currentId = QVariant::currentUserType();

      uint newId = currentId.fetch_add(1, std::memory_order_relaxed);
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
}

#define CS_DECLARE_METATYPE(TYPE)                  \
   template<>                                      \
   inline const QString &cs_typeToName<TYPE>() {   \
      static const QString retval = #TYPE;         \
      return retval;                               \
   }

#define Q_DECLARE_METATYPE(TYPE)                   \
   static_assert(false, "Macro Q_DECLARE_METATYPE(TYPE) is obsolete, use CS_DECLARE_METATYPE(TYPE)");

#endif
