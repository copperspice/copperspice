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

#include <qvariant.h>

#include <qabstractitemmodel.h>
#include <qbitarray.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qdatetime.h>
#include <qeasingcurve.h>
#include <qjsonvalue.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qline.h>
#include <qlocale.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring8.h>
#include <qstring16.h>
#include <qstringlist.h>
#include <qurl.h>
#include <quuid.h>

#include <limits>

QVector<QVariant::NamesAndTypes> QVariant::m_userTypes;
QVector<QVariantBase *> QVariant::m_variantClients;

namespace {

template <typename T>
T safe_cast(int64_t data, bool *ok)
{
   if constexpr(std::is_same_v<T, uint64_t>) {
      // uint64

      *ok = true;
      return static_cast<T>(data);

   } else if constexpr (std::numeric_limits<T>::is_signed) {
      // T is signed

      if (data >= std::numeric_limits<T>::min() && data <= std::numeric_limits<T>::max()) {
         // value in data will fit in the current T
         *ok = true;
         return static_cast<T>(data);

      } else {
         *ok = false;
         return 0;
      }

   } else {
      // T is unsigned

      if (data >= 0 && static_cast<uint64_t>(data) <= std::numeric_limits<T>::max()) {
         // value in data will fit in the current T
         *ok = true;
         return static_cast<T>(data);

      } else {
         *ok = false;
         return 0;
      }
   }
}

} // namespace

std::atomic<uint> &QVariant::currentUserType()
{
   // use case if this was not atomic
   //   uint & tmp = currentUserType();
   //   auto id = ++tmp;

   static std::atomic<uint> retval = QVariant::UserType + 1;

   return retval;
}

static const QVariant::NamesAndTypes builtinTypes[] = {

   { "bool",                   QVariant::Bool,                 typeid(bool *) },
   { "short",                  QVariant::Short,                typeid(short *) },
   { "unsigned short",         QVariant::UShort,               typeid(unsigned short *) },
   { "int",                    QVariant::Int,                  typeid(int *) },
   { "unsigned int",           QVariant::UInt,                 typeid(unsigned int *) },
   { "long",                   QVariant::Long,                 typeid(long *) },
   { "unsigned long",          QVariant::ULong,                typeid(unsigned long *) },
   { "long long",              QVariant::LongLong,             typeid(long long *) },
   { "unsigned long long",     QVariant::ULongLong,            typeid(unsigned long long *) },
   { "double",                 QVariant::Double,               typeid(double *) },
   { "float",                  QVariant::Float,                typeid(float *) },

   { "QChar",                  QVariant::QChar,                typeid(QChar *) },
   { "char",                   QVariant::Char,                 typeid(char *) },
   { "signed char",            QVariant::SChar,                typeid(signed char *) },
   { "unsigned char",          QVariant::UChar,                typeid(unsigned char *) },

#if defined(__cpp_char8_t)
   { "char8_t",                QVariant::Char8_t,              typeid(char8_t *) },
#endif

   { "char16_t",               QVariant::Char16_t,             typeid(char16_t *) },
   { "char32_t",               QVariant::Char32_t,             typeid(char32_t *) },

   { "QByteArray",             QVariant::ByteArray,            typeid(QByteArray *) },
   { "QBitArray",              QVariant::BitArray,             typeid(QBitArray *) },
   { "QString",                QVariant::String,               typeid(QString *) },
   { "QString8",               QVariant::String8,              typeid(QString8 *) },
   { "QString16",              QVariant::String16,             typeid(QString16 *) },
   { "QStringList",            QVariant::StringList,           typeid(QStringList *) },
   { "QStringView",            QVariant::StringView,           typeid(QStringView *) },
   { "QRegularExpression",     QVariant::RegularExpression,    typeid(QRegularExpression *) },

   { "QDate",                  QVariant::Date,                 typeid(QDate *) },
   { "QTime",                  QVariant::Time,                 typeid(QTime *) },
   { "QDateTime",              QVariant::DateTime,             typeid(QDateTime *) },
   { "QLocale",                QVariant::Locale,               typeid(QLocale *) },

   { "QJsonArray",             QVariant::JsonArray,            typeid(QJsonArray *) },
   { "QJsonDocument",          QVariant::JsonDocument,         typeid(QJsonDocument *) },
   { "QJsonObject",            QVariant::JsonObject,           typeid(QJsonObject *) },
   { "QJsonValue",             QVariant::JsonValue,            typeid(QJsonValue *) },

   { "QLine",                  QVariant::Line,                 typeid(QLine *) },
   { "QLineF",                 QVariant::LineF,                typeid(QLineF *) },
   { "QPoint",                 QVariant::Point,                typeid(QPoint *) },
   { "QPointF",                QVariant::PointF,               typeid(QPointF *) },
   { "QPolygon",               QVariant::Polygon,              typeid(QPolygon *) },
   { "QPolygonF",              QVariant::PolygonF,             typeid(QPolygonF *) },
   { "QRect",                  QVariant::Rect,                 typeid(QRect *) },
   { "QRectF",                 QVariant::RectF,                typeid(QRectF *) },
   { "QSize",                  QVariant::Size,                 typeid(QSize *) },
   { "QSizeF",                 QVariant::SizeF,                typeid(QSizeF *) },

   { "QVariantList",           QVariant::List,                 typeid(QVariantList *) },
   { "QVariantHash",           QVariant::Hash,                 typeid(QVariantHash *) },
   { "QVariantMap",            QVariant::Map,                  typeid(QVariantMap *) },
   { "QVariantMultiHash",      QVariant::MultiHash,            typeid(QVariantMultiHash *) },
   { "QVariantMultiMap",       QVariant::MultiMap,             typeid(QVariantMultiMap *) },

   { "void",                   QVariant::Void,                 typeid(void *) },
   { "void*",                  QVariant::VoidStar,             typeid(void **) },
   { "QObject*",               QVariant::ObjectStar,           typeid(QObject *) },
   { "QWidget*",               QVariant::WidgetStar,           typeid(QWidget *) },

   // core
   { "QEasingCurve",           QVariant::EasingCurve,          typeid(QEasingCurve *) },
   { "QModelIndex",            QVariant::ModelIndex,           typeid(QModelIndex *) },
   { "QPersistentModelIndex",  QVariant::PersistentModelIndex, typeid(QPersistentModelIndex *) },
   { "QUuid",                  QVariant::Uuid,                 typeid(QUuid *) },
   { "QUrl",                   QVariant::Url,                  typeid(QUrl *) },

   // CsGui
   { "QBitmap",                QVariant::Bitmap,               typeid(QBitmap *) },
   { "QBrush",                 QVariant::Brush,                typeid(QBrush *) },
   { "QColor",                 QVariant::Color,                typeid(QColor *) },
   { "QCursor",                QVariant::Cursor,               typeid(QCursor *) },
   { "QFont",                  QVariant::Font,                 typeid(QFont *) },
   { "QIcon",                  QVariant::Icon,                 typeid(QIcon *) },
   { "QImage",                 QVariant::Image,                typeid(QImage *) },
   { "QKeySequence",           QVariant::KeySequence,          typeid(QKeySequence *) },
   { "QMatrix",                QVariant::Matrix,               typeid(QMatrix *)},
   { "QMatrix4x4",             QVariant::Matrix4x4,            typeid(QMatrix4x4 *) },
   { "QPalette",               QVariant::Palette,              typeid(QPalette *) },
   { "QPen",                   QVariant::Pen,                  typeid(QPen *) },
   { "QPixmap",                QVariant::Pixmap,               typeid(QPixmap *) },
   { "QQuaternion",            QVariant::Quaternion,           typeid(QQuaternion *) },
   { "QRegion",                QVariant::Region,               typeid(QRegion *) },
   { "QSizePolicy",            QVariant::SizePolicy,           typeid(QSizePolicy *) },
   { "QTextLength",            QVariant::TextLength,           typeid(QTextLength *) },
   { "QTextFormat",            QVariant::TextFormat,           typeid(QTextFormat *) },
   { "QTransform",             QVariant::Transform,            typeid(QTransform *) },
   { "QVector2D",              QVariant::Vector2D,             typeid(QVector2D *) },
   { "QVector3D",              QVariant::Vector3D,             typeid(QVector3D *) },
   { "QVector4D",              QVariant::Vector4D,             typeid(QVector4D *) },

   { "QVariant",               QVariant::Variant,              typeid(QVariant *) },
};

/*
   // alias data types

   { "qint8",                         QVariant::Char,              typeid(Q *) },
   { "quint8",                        QVariant::UChar,             typeid(Q *) },
   { "qint16",                        QVariant::Short,             typeid(Q *) },
   { "quint16",                       QVariant::UShort,            typeid(Q *) },
   { "qint32",                        QVariant::Int,               typeid(Q *) },
   { "quint32",                       QVariant::UInt,              typeid(Q *) },
   { "qint64",                        QVariant::LongLong,          typeid(Q *) },
   { "quint64",                       QVariant::ULongLong,         typeid(Q *) },

   { "QList<QVariant>",               QVariant::List,              typeid(Q *) },
   { "QHash<QString,QVariant>",       QVariant::Hash,              typeid(Q *) },
   { "QMap<QString,QVariant>",        QVariant::Map,               typeid(Q *) },
   { "QMultiHash<QString,QVariant>",  QVariant::MultiHash,         typeid(Q *) },
   { "QMultiMap<QString,QVariant>",   QVariant::MultiMap,          typeid(Q *) },
*/

// constructors

QVariant::QVariant(const QVariant &other)
{
   // get a ptr to the requested alternative
   auto ptr = std::get_if<std::shared_ptr<CustomType>>(&(other.m_data));

   if (ptr == nullptr) {
      // simple type
      m_data = other.m_data;

   } else {
      // Custom type

      // ptr is a raw pointer to the shared pointer in the other Variant
      // clone() needs to do a deep copy
      m_data = (*ptr)->clone();
   }
}

QVariant::QVariant(QDataStream &stream)
{
   stream >> *this;
}

QVariant::QVariant(QVariant::Type type)
{
   // deprecated
   cs_internal_create(static_cast<uint>(type), nullptr);
}

QVariant::QVariant(uint typeId, const void *copy)
{
   cs_internal_create(typeId, copy);
}

QVariant::QVariant(bool value)
   : m_data(value)
{
}

QVariant::QVariant(int value)
   : m_data(value)
{
}

QVariant::QVariant(uint value)
   : m_data(value)
{
}

QVariant::QVariant(qint64 value)
   : m_data(value)
{
}

QVariant::QVariant(quint64 value)
   : m_data(value)
{
}

QVariant::QVariant(double value)
   : m_data(value)
{
}

QVariant::QVariant(float value)
   : m_data(value)
{
}

QVariant::QVariant(const QChar32 value)
   : m_data(value)
{
}

QVariant::QVariant(QString value)
   : m_data(std::move(value))
{
}

QVariant::QVariant(QByteArray value)
{
   m_data = std::make_shared<CustomType_T<QByteArray>>(std::move(value));
}

QVariant::QVariant(QBitArray value)
{
   m_data = std::make_shared<CustomType_T<QBitArray>>(std::move(value));
}

QVariant::QVariant(QString16 value)
{
   m_data = std::make_shared<CustomType_T<QString16>>(std::move(value));
}

QVariant::QVariant(QStringList value)
{
   m_data = std::make_shared<CustomType_T<QStringList>>(std::move(value));
}

QVariant::QVariant(QDate value)
{
   m_data = std::make_shared<CustomType_T<QDate>>(std::move(value));
}

QVariant::QVariant(QTime value)
{
   m_data = std::make_shared<CustomType_T<QTime>>(std::move(value));
}

QVariant::QVariant(QDateTime value)
{
   m_data = std::make_shared<CustomType_T<QDateTime>>(std::move(value));
}

QVariant::QVariant(QEasingCurve value)
{
   m_data = std::make_shared<CustomType_T<QEasingCurve>>(std::move(value));
}

QVariant::QVariant(QList<QVariant> value)
{
   m_data = std::make_shared<CustomType_T<QList<QVariant>>>(std::move(value));
}

QVariant::QVariant(QHash<QString, QVariant> value)
{
   m_data = std::make_shared<CustomType_T<QHash<QString, QVariant>>>(std::move(value));
}

QVariant::QVariant(QMap<QString, QVariant> value)
{
   m_data = std::make_shared<CustomType_T<QMap<QString, QVariant>>>(std::move(value));
}

QVariant::QVariant(QMultiHash<QString, QVariant> value)
{
   m_data = std::make_shared<CustomType_T<QMultiHash<QString, QVariant>>>(std::move(value));
}

QVariant::QVariant(QMultiMap<QString, QVariant> value)
{
   m_data = std::make_shared<CustomType_T<QMultiMap<QString, QVariant>>>(std::move(value));
}

QVariant::QVariant(QLine value)
{
   m_data = std::make_shared<CustomType_T<QLine>>(std::move(value));
}

QVariant::QVariant(QLineF value)
{
   m_data = std::make_shared<CustomType_T<QLineF>>(std::move(value));
}

QVariant::QVariant(QPoint value)
{
   m_data = std::make_shared<CustomType_T<QPoint>>(std::move(value));
}

QVariant::QVariant(QPointF value)
{
   m_data = std::make_shared<CustomType_T<QPointF>>(std::move(value));
}

QVariant::QVariant(QRect value)
{
   m_data = std::make_shared<CustomType_T<QRect>>(std::move(value));
}

QVariant::QVariant(QRectF value)
{
   m_data = std::make_shared<CustomType_T<QRectF>>(std::move(value));
}

QVariant::QVariant(QSize value)
{
   m_data = std::make_shared<CustomType_T<QSize>>(std::move(value));
}

QVariant::QVariant(QSizeF value)
{
   m_data = std::make_shared<CustomType_T<QSizeF>>(std::move(value));
}

QVariant::QVariant(QUrl value)
{
   m_data = std::make_shared<CustomType_T<QUrl>>(std::move(value));
}

QVariant::QVariant(QLocale value)
{
   m_data = std::make_shared<CustomType_T<QLocale>>(std::move(value));
}

QVariant::QVariant(QRegularExpression8 value)
{
   m_data = std::make_shared<CustomType_T<QRegularExpression8>>(std::move(value));
}

QVariant::QVariant(QUuid value)
{
   m_data = std::make_shared<CustomType_T<QUuid>>(std::move(value));
}

QVariant::QVariant(QModelIndex value)
{
   m_data = std::make_shared<CustomType_T<QModelIndex>>(std::move(value));
}

QVariant::QVariant(QPersistentModelIndex value)
{
   m_data = std::make_shared<CustomType_T<QPersistentModelIndex>>(std::move(value));
}

QVariant::QVariant(QJsonValue value)
{
   m_data = std::make_shared<CustomType_T<QJsonValue>>(std::move(value));
}

QVariant::QVariant(QJsonArray value)
{
   m_data = std::make_shared<CustomType_T<QJsonArray>>(std::move(value));
}

QVariant::QVariant(QJsonObject value)
{
   m_data = std::make_shared<CustomType_T<QJsonObject>>(std::move(value));
}

QVariant::QVariant(QJsonDocument value)
{
   m_data = std::make_shared<CustomType_T<QJsonDocument>>(std::move(value));
}

static bool cs_internal_isNumericType(uint type)
{
   return (type == QVariant::Int  || type == QVariant::UInt ||
      type == QVariant::LongLong  || type == QVariant::ULongLong ||
      type == QVariant::Double    || type == QVariant::Float);
}

static bool cs_internal_isFloatingPoint(uint type)
{
   return type == QVariant::Double || type == QVariant::Float;
}

static int64_t cs_internal_convertToInt64(const QVariant &data, bool *ok)
{
   *ok = true;

   switch (data.type()) {

      case QVariant::Bool:
         return int64_t(data.getData<bool>());

      case QVariant::Int:
         return int64_t(data.getData<int>());

      case QVariant::UInt:
         return int64_t(int(data.getData<uint>()));

      case QVariant::Short:
         return int64_t(data.getData<short>());

      case QVariant::UShort:
         return int64_t(short(data.getData<ushort>()));

      case QVariant::Long:
         return int64_t(data.getData<long>());

      case QVariant::ULong:
         return int64_t(long(data.getData<ulong>()));

      case QVariant::LongLong:
         return int64_t(data.getData<qint64>());

      case QVariant::ULongLong:
         return int64_t(data.getData<quint64>());

      case QVariant::Double:
         return int64_t(data.getData<double>());

      case QVariant::Float:
         return int64_t(data.getData<float>());

      case QVariant::Char:
         return int64_t(static_cast<signed char>(data.getData<char>()));

      case QVariant::SChar:
         return int64_t(data.getData<signed char>());

      case QVariant::UChar:
         return int64_t(static_cast<signed char>(data.getData<uchar>()));

      case QVariant::QChar:
         return int64_t(static_cast<int32_t>(data.getData<QChar32>().unicode()));

      case QVariant::String8:
         return data.getData<QString>().toInteger<int64_t>(ok);

      case QVariant::String16:
         return data.getData<QString16>().toInteger<int64_t>(ok);

      case QVariant::ByteArray:
         return data.getData<QByteArray>().toLongLong(ok);

      case QVariant::JsonValue: {
         QJsonValue tmp = data.getData<QJsonValue>();

         if (tmp.isDouble()) {
            return int64_t(tmp.toDouble());

         } else {
            break;
         }
      }

      default:
         if (data.isEnum()) {
            return data.enumToInt();
         }

         break;
   }

   *ok = false;

   return 0;
}

static uint64_t cs_internal_convertToUInt64(const QVariant &data, bool *ok)
{
   *ok = true;

   switch (data.type()) {

      case QVariant::Bool:
         return uint64_t(data.getData<bool>());

      case QVariant::Int:
         return uint64_t(uint(data.getData<int>()));

      case QVariant::UInt:
         return uint64_t(data.getData<uint>());

      case QVariant::Short:
         return uint64_t(ushort(data.getData<short>()));

      case QVariant::UShort:
         return uint64_t(data.getData<ushort>());

      case QVariant::Long:
         return uint64_t(ulong(data.getData<long>()));

      case QVariant::ULong:
         return uint64_t(data.getData<ulong>());

      case QVariant::LongLong:
         return uint64_t(data.getData<qint64>());

      case QVariant::ULongLong:
         return uint64_t(data.getData<quint64>());

      case QVariant::Double:
         return uint64_t(data.getData<double>());

      case QVariant::Float:
         return uint64_t(data.getData<float>());

      case QVariant::Char:
         return uint64_t(static_cast<unsigned char>(data.getData<char>()));

      case QVariant::SChar:
         return uint64_t(static_cast<unsigned char>(data.getData<signed char>()));

      case QVariant::UChar:
         return uint64_t(data.getData<uchar>());

      case QVariant::QChar:
         return uint64_t(data.getData<QChar32>().unicode());

      case QVariant::String8:
         return data.getData<QString>().toInteger<uint64_t>(ok);

      case QVariant::String16:
         return data.getData<QString16>().toInteger<uint64_t>(ok);

      case QVariant::ByteArray:
         return data.getData<QByteArray>().toULongLong(ok);

      case QVariant::JsonValue: {
         QJsonValue tmp = data.getData<QJsonValue>();

         if (tmp.isDouble()) {
            return uint64_t(tmp.toDouble());

         } else {
            break;
         }
      }

      default:
         if (data.isEnum()) {
            return data.enumToUInt();
         }

         break;
   }

   *ok = false;

   return 0;
}

bool QVariant::cs_internal_convert(uint current_userType, uint new_userType)
{
   bool retval = true;

   switch (new_userType) {

      case QVariant::Bool:

         switch (current_userType) {
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::Short:
            case QVariant::UShort:
            case QVariant::Long:
            case QVariant::ULong:
            case QVariant::LongLong:
            case QVariant::ULongLong:
            case QVariant::Double:
            case QVariant::Float:
            case QVariant::Char:
            case QVariant::SChar:
            case QVariant::UChar: {
               bool tmp = cs_internal_convertToUInt64(*this, &retval) != 0;
               setValue(tmp);
               break;
            }

            case QVariant::ByteArray: {

               QByteArray str = getData<QByteArray>().toLower();

               if (str == QByteArray("0") || str == QByteArray("false") || str.isEmpty()) {
                  setValue(false);
               } else {
                  setValue(true);
               }

               break;
            }

            case QVariant::String: {

               QString str = getData<QString>().toLower();

               if (str == "0" || str == "false" || str.isEmpty()) {
                  setValue(false);
               } else {
                  setValue(true);
               }

               break;
            }

            case QVariant::String16: {

               QString16 str = getData<QString16>().toLower();

               if (str == u"0" || str == u"false" || str.isEmpty()) {
                  setValue(false);
               } else {
                  setValue(true);
               }

               break;
            }

            case QVariant::QChar:

               if (getData<QChar32>().isNull()) {
                  setValue(false);
               } else {
                  setValue(true);
               }

               break;

            case QVariant::JsonValue: {

               QJsonValue tmp = getData<QJsonValue>();

               if (tmp.isBool())  {
                  setValue(tmp.toBool());

               } else if (tmp.isDouble())  {

                  if (tmp.toDouble() == 0) {
                     setValue(false);
                  } else {
                     setValue(true);
                  }

               } else if (tmp.isString())  {
                  QString str = tmp.toString().toLower();

                  if (str == QString("0") || str == QString("false") || str.isEmpty()) {
                     setValue(false);
                  } else {
                     setValue(true);
                  }

               } else {
                  setValue(false);
               }

               break;
            }

            default:
               setValue(false);
               retval = false;
         }

         break;

      case QVariant::Int: {
         int64_t tmp = cs_internal_convertToInt64(*this, &retval);
         int data    = 0;

         if (retval) {
            data = safe_cast<int>(tmp, &retval);
         }

         setValue(data);
         break;
      }

      case QVariant::UInt: {
         uint64_t tmp = cs_internal_convertToUInt64(*this, &retval);
         uint data    = 0;

         if (retval) {
            data = safe_cast<uint>(tmp, &retval);
         }

         setValue(data);
         break;
      }

      case QVariant::Short: {
         int64_t tmp = cs_internal_convertToInt64(*this, &retval);
         short data  = 0;

         if (retval) {
            data = safe_cast<short>(tmp, &retval);
         }

         setValue(data);
         break;
      }

      case QVariant::UShort: {
         uint64_t tmp = cs_internal_convertToUInt64(*this, &retval);
         ushort data  = 0;

         if (retval) {
            data = safe_cast<ushort>(tmp, &retval);
         }

         setValue(data);
         break;
      }

      case QVariant::Long: {
         int64_t tmp = cs_internal_convertToInt64(*this, &retval);
         long data   = 0;

         if (retval) {
            data = safe_cast<long>(tmp, &retval);
         }

         setValue(data);
         break;
      }

      case QVariant::ULong: {
         uint64_t tmp = cs_internal_convertToUInt64(*this, &retval);
         ulong data   = 0;

         if (retval) {
            data = safe_cast<ulong>(tmp, &retval);
         }

         setValue(data);
         break;
      }

      case QVariant::LongLong:  {
         int64_t tmp = cs_internal_convertToInt64(*this, &retval);
         qint64 data = 0;

         if (retval) {
            data = safe_cast<qint64>(tmp, &retval);
         }

         setValue(data);
         break;
      }

      case QVariant::ULongLong: {
         uint64_t tmp = cs_internal_convertToUInt64(*this, &retval);
         quint64 data = 0;

         if (retval) {
            data = safe_cast<quint64>(tmp, &retval);
         }

         setValue(data);
         break;
      }

      case QVariant::Double: {

         switch (current_userType) {

            case QVariant::Bool: {
               double tmp = getData<bool>();
               setValue(tmp);
               break;
            }

            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::Short:
            case QVariant::UShort:
            case QVariant::Long:
            case QVariant::ULong:
            case QVariant::LongLong:
            case QVariant::ULongLong:
            case QVariant::Char:
            case QVariant::SChar:
            case QVariant::UChar: {
               int64_t tmp = cs_internal_convertToInt64(*this, &retval);
               double data  = 0;

               if (retval) {
                  data = safe_cast<double>(tmp, &retval);
               }

               setValue(data);
               break;
            }

            case QVariant::Float: {
               double tmp = getData<float>();
               setValue(tmp);
               break;
            }

            case QVariant::ByteArray: {
               double tmp = getData<QByteArray>().toDouble();
               setValue(tmp);
               break;
            }

            case QVariant::String: {
               double tmp = getData<QString>().toDouble();
               setValue(tmp);
               break;
            }

            case QVariant::String16: {
               double tmp = getData<QString16>().toDouble();
               setValue(tmp);
               break;
            }

            case QVariant::JsonValue: {
               QJsonValue tmp = getData<QJsonValue>();

               if (tmp.isDouble()) {
                  setValue(tmp.toDouble());

               } else if (tmp.isString()) {
                  double value = tmp.toString().toDouble();
                  setValue(value);

               } else {
                  setValue(0.0);
                  retval = false;
               }

               break;
            }

            default:
               setValue(0.0);
               retval = false;
         }

         break;
      }

      case QVariant::Float: {

         switch (current_userType) {

            case QVariant::Bool: {
               float tmp = getData<bool>();
               setValue(tmp);
               break;
            }

            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::Short:
            case QVariant::UShort:
            case QVariant::Long:
            case QVariant::ULong:
            case QVariant::LongLong:
            case QVariant::ULongLong:
            case QVariant::Char:
            case QVariant::SChar:
            case QVariant::UChar:  {
               int64_t tmp = cs_internal_convertToInt64(*this, &retval);
               float data  = 0;

               if (retval) {
                  data = safe_cast<double>(tmp, &retval);
               }

               setValue(data);
               break;
            }

            case QVariant::Double:  {
               float tmp = getData<double>();
               setValue(tmp);
               break;
            }

            case QVariant::ByteArray: {
               float tmp = getData<QByteArray>().toFloat();
               setValue(tmp);
               break;
            }

            case QVariant::String:  {
               float tmp = getData<QString>().toFloat();
               setValue(tmp);
               break;
            }

            case QVariant::String16:  {
               float tmp = getData<QString16>().toFloat();
               setValue(tmp);
               break;
            }

            case QVariant::JsonValue: {
               QJsonValue tmp = getData<QJsonValue>();

               if (tmp.isDouble()) {
                  setValue<float>(tmp.toDouble());

               } else if (tmp.isString()) {
                  double value = tmp.toString().toDouble();
                  setValue<float>(value);

               } else {
                  setValue<float>(0.0);
                  retval = false;
               }

               break;
            }

            default:
               setValue<float>(0.0);
               retval = false;
         }

         break;
      }

      case QVariant::Char:  {
         char data = 0;

         if constexpr (std::is_signed_v<char>) {
            int64_t tmp = cs_internal_convertToInt64(*this, &retval);

            if (retval) {
               data = safe_cast<char>(tmp, &retval);
            }

         } else  {
            uint64_t tmp = cs_internal_convertToUInt64(*this, &retval);

            if (retval) {
               data = safe_cast<char>(tmp, &retval);
            }
         }

         setValue(data);
         break;
      }

      case QVariant::SChar: {
         int64_t tmp = cs_internal_convertToInt64(*this, &retval);
         signed char data = 0;

         if (retval) {
            data = safe_cast<signed char>(tmp, &retval);
         }

         setValue(data);
         break;
      }

      case QVariant::UChar: {
         uint64_t tmp = cs_internal_convertToUInt64(*this, &retval);
         uchar data   = 0;

         if (retval) {
            data = safe_cast<uchar>(tmp, &retval);
         }

         setValue(data);
         break;
      }

      case QVariant::QChar:

         switch (current_userType) {
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::Short:
            case QVariant::UShort:
            case QVariant::Long:
            case QVariant::ULong:
            case QVariant::LongLong:
            case QVariant::ULongLong:
            case QVariant::Float:
            case QVariant::Double:
            case QVariant::Char:
            case QVariant::SChar:
            case QVariant::UChar:
            case QVariant::ByteArray:
            case QVariant::String8:
            case QVariant::String16:
            case QVariant::JsonValue: {
               uint64_t tmp = cs_internal_convertToUInt64(*this, &retval);
               QChar32 data = 0;

               if (retval) {
                  data = safe_cast<char32_t>(tmp, &retval);
               }

               setValue(data);
               break;
            }

            default:
               setValue(QChar32());
               retval = false;
         }

         break;

      case QVariant::ByteArray: {

         switch (current_userType) {

            case QVariant::Bool: {
               bool tmp = getData<bool>();

               if (tmp) {
                  setValue<QByteArray>("true");
               } else {
                  setValue<QByteArray>("false");
               }

               break;
            }

            case QVariant::Int:
               setValue<QByteArray>( QByteArray::number(getData<int>()) );
               break;

            case QVariant::UInt:
               setValue<QByteArray>( QByteArray::number(getData<unsigned int>()) );
               break;

            case QVariant::Long:
               setValue<QByteArray>( QByteArray::number(static_cast<qint64>(getData<long>())) );
               break;

            case QVariant::ULong:
               setValue<QByteArray>( QByteArray::number(static_cast<quint64>(getData<unsigned long>())) );
               break;

            case QVariant::LongLong:
               setValue<QByteArray>( QByteArray::number(getData<long long>()) );
               break;

            case QVariant::ULongLong:
               setValue<QByteArray>( QByteArray::number(getData<unsigned long long>()) );
               break;

            case QVariant::Double:
               setValue<QByteArray>( QByteArray::number(getData<double>(), 'g', std::numeric_limits<double>::digits10) );
               break;

            case QVariant::Float:
               setValue<QByteArray>( QByteArray::number(getData<float>(), 'g', std::numeric_limits<float>::digits10) );
               break;

            case QVariant::Short:
               setValue<QByteArray>( QByteArray::number(getData<short>()) );
               break;

            case QVariant::UShort:
               setValue<QByteArray>( QByteArray::number(getData<unsigned short>()) );
               break;

            case QVariant::Char: {
               QByteArray tmp;

               auto ch = getData<char>();

               if (ch != 0) {
                  tmp.append(ch);
               }

               setValue<QByteArray>(tmp);
               break;
            }

            case QVariant::QChar: {
               auto tmp = getData<QChar32>();

               if (tmp.isNull()) {
                  setValue<QByteArray>("");
               } else {
                  setValue<QByteArray>(QString(tmp).toUtf8());
               }

               break;
            }

            case QVariant::SChar: {
               QByteArray tmp;

               auto ch = getData<signed char>();

               if (ch != 0) {
                  tmp.append(ch);
               }

               setValue<QByteArray>(tmp);
               break;
            }

            case QVariant::UChar: {
               QByteArray tmp;

               auto ch = getData<unsigned char>();

               if (ch != 0) {
                  tmp.append(ch);
               }

               setValue<QByteArray>(tmp);
               break;
            }

            case QVariant::String:
               setValue<QByteArray>(getData<QString>().toUtf8());
               break;

            case QVariant::String16:
               setValue<QByteArray>(getData<QString16>().toUtf8());
               break;

            case QVariant::Url:
               setValue<QByteArray>(getData<QUrl>().toEncoded());
               break;

            case QVariant::Uuid:
               setValue<QByteArray>(getData<QUuid>().toByteArray());
               break;

            default:
               retval = false;
         }

         break;
      }

      case QVariant::String: {
         switch (current_userType) {

            case QVariant::Bool: {
               bool tmp = getData<bool>();

               if (tmp) {
                  setValue<QString>("true");
               } else {
                  setValue<QString>("false");
               }

               break;
            }

            case QVariant::Int:
               setValue<QString>( QString::number(getData<int>()) );
               break;

            case QVariant::UInt:
               setValue<QString>( QString::number(getData<unsigned int>()) );
               break;

            case QVariant::Long:
               setValue<QString>( QString::number(getData<long>()) );
               break;

            case QVariant::ULong:
               setValue<QString>( QString::number(getData<unsigned long>()) );
               break;

            case QVariant::LongLong:
               setValue<QString>( QString::number(getData<long long>()) );
               break;

            case QVariant::ULongLong:
               setValue<QString>( QString::number(getData<unsigned long long>()) );
               break;

            case QVariant::Double:
               setValue<QString>( QString::number(getData<double>(), 'g', std::numeric_limits<double>::digits10) );
               break;

            case QVariant::Float:
               setValue<QString>( QString::number(getData<float>(), 'g', std::numeric_limits<float>::digits10) );
               break;

            case QVariant::Short:
               setValue<QString>( QString::number(getData<short>()) );
               break;

            case QVariant::UShort:
               setValue<QString>( QString::number(getData<unsigned short>()) );
               break;

            case QVariant::Char:
               setValue<QString>( QChar::fromLatin1(getData<char>()) );
               break;

            case QVariant::SChar:
               setValue<QString>( QChar::fromLatin1(getData<signed char>()) );
               break;

            case QVariant::UChar:
               setValue<QString>( QChar::fromLatin1(getData<unsigned char>()) );
               break;

            case QVariant::QChar: {
               auto tmp = getData<QChar32>();

               if (tmp.isNull()) {
                  setValue<QString>("");
               } else {
                  setValue<QString>(tmp);
               }

               break;
            }

            case QVariant::ByteArray:
               setValue<QString>(QString::fromUtf8(getData<QByteArray>()));
               break;

            case QVariant::String16:
               setValue<QString>(QString::fromUtf16(getData<QString16>()));
               break;

            case QVariant::StringList: {
               QStringList tmp = getData<QStringList>();

               if (tmp.count() == 1) {
                  setValue<QString>(tmp[0]);
               }

               break;
            }

            case QVariant::Date:
               setValue<QString>(getData<QDate>().toString(Qt::ISODate) );
               break;

            case QVariant::DateTime:
               setValue<QString>(getData<QDateTime>().toString(Qt::ISODate) );
               break;

            case QVariant::Time:
               setValue<QString>(getData<QTime>().toString(Qt::ISODate) );
               break;

            case QVariant::JsonValue: {
               QJsonValue tmp = getData<QJsonValue>();

               if (tmp.isBool())  {

                  if (tmp.toBool()) {
                     setValue<QString>("true");
                  } else {
                     setValue<QString>("false");
                  }

               } else if (tmp.isDouble())  {
                  setValue<QString>(QString::number(tmp.toDouble()));

               } else if (tmp.isString())  {
                  setValue<QString>(tmp.toString());

               } else {
                  retval = false;

               }

               break;
            }

            case QVariant::Url:
               setValue<QString>(getData<QUrl>().toString());
               break;

            case QVariant::Uuid:
               setValue<QString>(getData<QUuid>().toString());
               break;

            default:
               retval = false;
         }

         break;
      }

      case QVariant::String16: {

         switch (current_userType) {

            case QVariant::Bool: {
               bool tmp = getData<bool>();

               if (tmp) {
                  setValue<QString16>(u"true");
               } else {
                  setValue<QString16>(u"false");
               }

               break;
            }

            case QVariant::Int:
               setValue<QString16>( QString16::number(getData<int>()) );
               break;

            case QVariant::UInt:
               setValue<QString16>( QString16::number(getData<unsigned int>()) );
               break;

            case QVariant::Long:
               setValue<QString16>( QString16::number(getData<long>()) );
               break;

            case QVariant::ULong:
               setValue<QString16>( QString16::number(getData<unsigned long>()) );
               break;

            case QVariant::LongLong:
               setValue<QString16>( QString16::number(getData<long long>()) );
               break;

            case QVariant::ULongLong:
               setValue<QString16>( QString16::number(getData<unsigned long long>()) );
               break;

            case QVariant::Double:
               setValue<QString16>( QString16::number(getData<double>(), 'g', std::numeric_limits<double>::digits10) );
               break;

            case QVariant::Float:
               setValue<QString16>( QString16::number(getData<float>(), 'g', std::numeric_limits<float>::digits10) );
               break;

            case QVariant::Short:
               setValue<QString16>( QString16::number(getData<short>()) );
               break;

            case QVariant::UShort:
               setValue<QString16>( QString16::number(getData<unsigned short>()) );
               break;

            case QVariant::Char:
               setValue<QString16>( QChar::fromLatin1(getData<char>()) );
               break;

            case QVariant::SChar:
               setValue<QString16>( QChar::fromLatin1(getData<signed char>()) );
               break;

            case QVariant::UChar:
               setValue<QString16>( QChar::fromLatin1(getData<unsigned char>()) );
               break;

            case QVariant::QChar: {
               auto tmp = getData<QChar32>();

               if (tmp.isNull()) {
                  setValue<QString16>(u"");
               } else {
                  setValue<QString16>(tmp);
               }

               break;
            }

            case QVariant::ByteArray:
               setValue<QString16>(QString16::fromUtf8(getData<QByteArray>()));
               break;

            case QVariant::String:
               setValue<QString16>(QString16::fromUtf8(getData<QString>()));
               break;

            case QVariant::StringList: {
               QStringList tmp = getData<QStringList>();

               if (tmp.count() == 1) {
                  setValue<QString16>(tmp[0].toUtf16());
               }

               break;
            }

            case QVariant::Date:
               setValue<QString16>(getData<QDate>().toString(Qt::ISODate).toUtf16());
               break;

            case QVariant::DateTime:
               setValue<QString16>(getData<QDateTime>().toString(Qt::ISODate).toUtf16());
               break;

            case QVariant::Time:
               setValue<QString16>(getData<QTime>().toString(Qt::ISODate).toUtf16());
               break;

            case QVariant::JsonValue: {
               QJsonValue tmp = getData<QJsonValue>();

               if (tmp.isBool())  {

                  if (tmp.toBool()) {
                     setValue<QString16>(u"true");
                  } else {
                     setValue<QString16>(u"false");
                  }

               } else if (tmp.isDouble())  {
                  setValue<QString16>(QString16::number(tmp.toDouble()));

               } else if (tmp.isString())  {
                  setValue<QString16>(tmp.toString().toUtf16());

               } else {
                  retval = false;

               }

               break;
            }

            case QVariant::Url:
               setValue<QString16>(getData<QUrl>().toString().toUtf16());
               break;

            case QVariant::Uuid:
               setValue<QString16>(getData<QUuid>().toString().toUtf16());
               break;

            default:
               retval = false;
         }

         break;
      }

      case QVariant::StringList: {

         if (current_userType == QVariant::String) {
            QStringList list;
            list.append(getData<QString>());

            setValue<QStringList>(list);

         } else if (current_userType == QVariant::String16) {
            QStringList list;
            list.append(QString::fromUtf16(getData<QString16>()));

            setValue<QStringList>(list);

         } else if (current_userType == QVariant::List) {
            QList<QVariant> tmp = getData<QList<QVariant>>();

            QStringList list;

            for (const QVariant &item : tmp) {
               list.append(item.toString());
            }

            setValue<QStringList>(list);

         } else {
            retval = false;

         }

         break;
      }

      case QVariant::Date: {
         QDate tmp;

         switch (current_userType) {
            case QVariant::DateTime:
               tmp = getData<QDateTime>().date();
               break;

            case QVariant::String:
               tmp = QDate::fromString(getData<QString>(), Qt::ISODate);
               break;

            case QVariant::String16:
               tmp = QDate::fromString(QString::fromUtf16(getData<QString16>()), Qt::ISODate);
               break;

            // from QVariant::Time is invalid

            default:
               retval = false;
         }

         setValue<QDate>(tmp);
         return tmp.isValid();
      }

      case QVariant::Time: {
         QTime tmp;

         switch (current_userType) {
            // from QVariant::Date is invalid

            case QVariant::DateTime:
               tmp = getData<QDateTime>().time();
               break;

            case QVariant::String:
               tmp = QTime::fromString(getData<QString>(), Qt::ISODate);
               break;

            case QVariant::String16:
               tmp = QTime::fromString(QString::fromUtf16(getData<QString16>()), Qt::ISODate);
               break;

            default:
               retval = false;
         }

         setValue<QTime>(tmp);
         return tmp.isValid();
      }

      case QVariant::DateTime: {
         QDateTime tmp;

         switch (current_userType) {

            case QVariant::Date:
               tmp = QDateTime(getData<QDate>());
               break;

            case QVariant::String:
               tmp = QDateTime::fromString(getData<QString>(), Qt::ISODate);
               break;

            case QVariant::String16:
               tmp = QDateTime::fromString(QString::fromUtf16(getData<QString16>()), Qt::ISODate);
               break;

            // from QVariant::Time is invalid

            default:
               retval = false;
         }

         setValue<QDateTime>(tmp);
         return tmp.isValid();
      }

      // next 5 are Container<QString, QVariant>

      case QVariant::List:

         if (current_userType == QVariant::StringList) {
            QStringList tmp = getData<QStringList>();

            QVariantList list;

            for (const QString &item : tmp) {
               list.append(QVariant(item));
            }

            setValue<QVariantList>(list);

         } else if (current_userType == QVariant::JsonValue) {
            QJsonValue tmp = getData<QJsonValue>();

            if (! tmp.isArray()) {
               retval = false;
               break;
            }

            setValue<QVariantList>(tmp.toArray().toVariantList());

         } else if (current_userType == QVariant::JsonArray) {
            setValue<QVariantList>(getData<QJsonArray>().toVariantList());

         } else {
            retval = false;
         }

         break;

      case QVariant::Map:

         if (current_userType == QVariant::Hash) {
            QVariantHash tmp = getData<QVariantHash>();

            QVariantMap result;

            for (auto iter = tmp.cbegin(); iter != tmp.cend(); ++iter) {
               result.insert(iter.key(), std::move(iter.value()));
            }

            setValue<QVariantMap>(result);

         } else if (current_userType == QVariant::JsonValue) {
            QJsonValue tmp = getData<QJsonValue>();

            if (! tmp.isObject()) {
               retval = false;
               break;
            }

            setValue<QVariantMap>(tmp.toObject().toVariantMap());

         } else if (current_userType == QVariant::JsonObject) {
            setValue<QVariantMap>(getData<QJsonObject>().toVariantMap());

         } else {
            retval = false;
         }

         break;

      case QVariant::MultiMap:
         // do nothing
         break;

      case QVariant::Hash:

         if (current_userType == QVariant::Map) {
            QVariantMap tmp = getData<QVariantMap>();

            QVariantHash result;

            for (auto iter = tmp.cbegin(); iter != tmp.cend(); ++iter) {
               result.insert(iter.key(), std::move(iter.value()));
            }

            setValue<QVariantHash>(result);

         } else if (current_userType == QVariant::JsonValue) {
            QJsonValue tmp = getData<QJsonValue>();

            if (! tmp.isObject()) {
               retval = false;
               break;
            }

            setValue<QVariantHash>(tmp.toObject().toVariantHash());

         } else if (current_userType == QVariant::JsonObject) {
            setValue<QVariantHash>(getData<QJsonObject>().toVariantHash());

         } else {
            retval = false;

         }

         break;

      case QVariant::MultiHash:
         // do nothing
         break;

      case QVariant::Line:
         if (current_userType == QVariant::LineF) {
            setValue<QLine>(getData<QLineF>().toLine());
         } else {
            retval = false;
         }

         break;

      case QVariant::LineF:
         if (current_userType == QVariant::Line) {
            setValue<QLineF>(getData<QLine>());
         } else {
            retval = false;
         }

         break;

      case QVariant::Point:
         if (current_userType == QVariant::PointF) {
            setValue<QPoint>(getData<QPointF>().toPoint());
         } else {
            retval = false;
         }

         break;

      case QVariant::PointF:
         if (current_userType == QVariant::Point) {
            setValue<QPointF>(getData<QPoint>());
         } else {
            retval = false;
         }

         break;

      case QVariant::Rect:
         if (current_userType == QVariant::RectF) {
            setValue<QRect>(getData<QRectF>().toRect());
         } else {
            retval = false;
         }

         break;

      case QVariant::RectF:
         if (current_userType == QVariant::Rect) {
            setValue<QRectF>(getData<QRect>());
         } else {
            retval = false;
         }

         break;

      case QVariant::Size:
         if (current_userType == QVariant::SizeF) {
            setValue<QSize>(getData<QSizeF>().toSize());
         } else {
            retval = false;
         }

         break;

      case QVariant::SizeF:
         if (current_userType == QVariant::Size) {
            setValue<QSizeF>(getData<QSize>());
         } else {
            retval = false;
         }

         break;

      case QVariant::ModelIndex:
         if (current_userType == QVariant::PersistentModelIndex) {
            setValue<QModelIndex>(getData<QPersistentModelIndex>());
         }

         break;

      case QVariant::PersistentModelIndex:
         if (current_userType == QVariant::ModelIndex) {
            setValue<QPersistentModelIndex>(getData<QModelIndex>());
         }

         break;

      case QVariant::Url:
         switch (current_userType) {
            case QVariant::String:
               setValue<QUrl>(QUrl(getData<QString>()));
               break;

            case QVariant::String16:
               setValue<QUrl>(QUrl(QString::fromUtf16(getData<QString16>())));
               break;

            default:
               retval = false;
         }

         break;

      case QVariant::Uuid:
         switch (current_userType) {
            case QVariant::String:
               setValue<QUuid>(QUuid(getData<QString>()));
               break;

            case QVariant::String16:
               setValue<QUuid>(QUuid(QString::fromUtf16(getData<QString16>())));
               break;

            default:
               retval = false;
         }

         break;

      default:
         retval = false;
   }

   if (! retval) {
      // call cs_internal_convert() in gui, etc
      bool done = false;

      for (const auto ptr : m_variantClients) {
         done = ptr->cs_internal_convert(current_userType, new_userType, *this);

         if (done) {
            retval = true;
            break;
         }
      }
   }

   return retval;
}

void QVariant::cs_internal_create(uint typeId, const void *other)
{
   switch (typeId) {

      case QVariant::Bool:

         if (other == nullptr) {
            setValue<bool>(false);
         } else {
            setValue<bool>(*static_cast<const bool *>(other) );
         }

         break;

      case QVariant::Int:

         if (other == nullptr) {
            setValue<int>(0);
         } else {
            setValue<int>(*static_cast<const int *>(other) );
         }

         break;

      case QVariant::UInt:

         if (other == nullptr) {
            setValue<uint>(0u);
         } else {
            setValue<uint>(*static_cast<const uint *>(other) );
         }

         break;

      case QVariant::LongLong:

         if (other == nullptr) {
            setValue<qint64>(0);
         } else {
            setValue<qint64>(*static_cast<const qint64 *>(other) );
         }

         break;

      case QVariant::ULongLong:

         if (other == nullptr) {
            setValue<quint64>(0u);
         } else {
            setValue<quint64>(*static_cast<const quint64 *>(other) );
         }

         break;

      case QVariant::Double:

         if (other == nullptr) {
            setValue<double>(0.0);
         } else {
            setValue<double>(*static_cast<const double *>(other) );
         }

         break;

      case QVariant::Float:
         if (other == nullptr) {
            setValue<float>(0.0f);
         } else {
            setValue<float>(*static_cast<const float *>(other) );
         }

         break;

      case QVariant::Char:
         if (other == nullptr) {
            setValue<char>('\0');
         } else {
            setValue<char>(*static_cast<const char *>(other) );
         }

         break;

      case QVariant::QChar:
         if (other == nullptr) {
            setValue<QChar32>('\0');
         } else {
            setValue<QChar32>(*static_cast<const QChar32 *>(other) );
         }

         break;

      case QVariant::String8:
         if (other == nullptr) {
            setValue<QString8>("");
         } else {
            setValue<QString8>(*static_cast<const QString8 *>(other) );
         }

         break;

      case QVariant::String16:
         if (other == nullptr) {
            setValue<QString16>(u"");
         } else {
            setValue<QString16>(*static_cast<const QString16 *>(other) );
         }

         break;

      case QVariant::StringList:
         if (other == nullptr) {
            setValue<QStringList>(QStringList());
         } else {
            setValue<QStringList>(*static_cast<const QStringList *>(other) );
         }

         break;

      case QVariant::ByteArray:
         if (other == nullptr) {
            setValue<QByteArray>(QByteArray());
         } else {
            setValue<QByteArray>(*static_cast<const QByteArray *>(other) );
         }

         break;

      case QVariant::BitArray:
         if (other == nullptr) {
            setValue<QBitArray>(QBitArray());
         } else {
            setValue<QBitArray>(*static_cast<const QBitArray *>(other) );
         }

         break;

      case QVariant::Date:
         if (other == nullptr) {
            setValue<QDate>(QDate());
         } else {
            setValue<QDate>(*static_cast<const QDate *>(other) );
         }

         break;

      case QVariant::Time:
         if (other == nullptr) {
            setValue<QTime>(QTime());
         } else {
            setValue<QTime>(*static_cast<const QTime *>(other) );
         }

         break;

      case QVariant::DateTime:
         if (other == nullptr) {
            setValue<QDateTime>(QDateTime());
         } else {
            setValue<QDateTime>(*static_cast<const QDateTime *>(other) );
         }

         break;

      case QVariant::EasingCurve:
         if (other == nullptr) {
            setValue<QEasingCurve>(QEasingCurve());
         } else {
            setValue<QEasingCurve>(*static_cast<const QEasingCurve *>(other) );
         }

         break;

      case QVariant::Locale:
         if (other == nullptr) {
            setValue<QLocale>(QLocale());
         } else {
            setValue<QLocale>(*static_cast<const QLocale *>(other) );
         }

         break;

      case QVariant::Rect:
         if (other == nullptr) {
            setValue<QRect>(QRect());
         } else {
            setValue<QRect>(*static_cast<const QRect *>(other) );
         }

         break;

      case QVariant::RectF:
         if (other == nullptr) {
            setValue<QRectF>(QRectF());
         } else {
            setValue<QRectF>(*static_cast<const QRectF *>(other) );
         }

         break;

      case QVariant::Size:
         if (other == nullptr) {
            setValue<QSize>(QSize());
         } else {
            setValue<QSize>(*static_cast<const QSize *>(other) );
         }

         break;

      case QVariant::SizeF:
         if (other == nullptr) {
            setValue<QSizeF>(QSizeF());
         } else {
            setValue<QSizeF>(*static_cast<const QSizeF *>(other) );
         }

         break;

      case QVariant::Line:
         if (other == nullptr) {
            setValue<QLine>(QLine());
         } else {
            setValue<QLine>(*static_cast<const QLine *>(other) );
         }

         break;

      case QVariant::LineF:
         if (other == nullptr) {
            setValue<QLineF>(QLineF());
         } else {
            setValue<QLineF>(*static_cast<const QLineF *>(other) );
         }

         break;

      case QVariant::Point:
         if (other == nullptr) {
            setValue<QPoint>(QPoint());
         } else {
            setValue<QPoint>(*static_cast<const QPoint *>(other) );
         }

         break;

      case QVariant::PointF:
         if (other == nullptr) {
            setValue<QPointF>(QPointF());
         } else {
            setValue<QPointF>(*static_cast<const QPointF *>(other) );
         }

         break;

      // next 5 are Container<QString, QVariant>

      case QVariant::List:
         if (other == nullptr) {
            setValue<QVariantList>(QVariantList());
         } else {
            setValue<QVariantList>(*static_cast<const QVariantList *>(other) );
         }

         break;

      case QVariant::Map:
         if (other == nullptr) {
            setValue<QVariantMap>(QVariantMap());
         } else {
            setValue<QVariantMap>(*static_cast<const QVariantMap *>(other) );
         }

         break;

      case QVariant::Hash:
         if (other == nullptr) {
            setValue<QVariantHash>(QVariantHash());
         } else {
            setValue<QVariantHash>(*static_cast<const QVariantHash *>(other) );
         }

         break;

      case QVariant::MultiMap:
         if (other == nullptr) {
            setValue<QVariantMultiMap>(QVariantMultiMap());
         } else {
            setValue<QVariantMultiMap>(*static_cast<const QVariantMultiMap *>(other) );
         }

         break;

      case QVariant::MultiHash:
         if (other == nullptr) {
            setValue<QVariantMultiHash>(QVariantMultiHash());
         } else {
            setValue<QVariantMultiHash>(*static_cast<const QVariantMultiHash *>(other) );
         }

         break;

      case QVariant::RegularExpression:
         if (other == nullptr) {
            setValue<QRegularExpression8>(QRegularExpression8());
         } else {
            setValue<QRegularExpression8>(*static_cast<const QRegularExpression8 *>(other) );
         }

         break;

      case QVariant::Uuid:
         if (other == nullptr) {
            setValue<QUuid>(QUuid());
         } else {
            setValue<QUuid>(*static_cast<const QUuid *>(other) );
         }

         break;

      case QVariant::Url:
         if (other == nullptr) {
            setValue<QUrl>(QUrl());
         } else {
            setValue<QUrl>(*static_cast<const QUrl *>(other) );
         }

         break;

      case QVariant::ModelIndex:
         if (other == nullptr) {
            setValue<QModelIndex>(QModelIndex());
         } else {
            setValue<QModelIndex>(*static_cast<const QModelIndex *>(other) );
         }

         break;

      case QVariant::PersistentModelIndex:
         if (other == nullptr) {
            setValue<QPersistentModelIndex>(QPersistentModelIndex());
         } else {
            setValue<QPersistentModelIndex>(*static_cast<const QPersistentModelIndex *>(other) );
         }

         break;

      case QVariant::ObjectStar:
         if (other == nullptr) {
            setValue<QObject *>(nullptr);
         } else {
            setValue<QObject *>(*static_cast<QObject * const *>(other));
         }

         break;

      case QVariant::Invalid:
         clear();
         break;

      default:
         // call cs_internal_create in gui, etc
         bool done = false;

         for (const auto ptr : m_variantClients) {
            done = ptr->cs_internal_create(typeId, other, *this);

            if (done) {
               break;
            }
         }

         if (! done)  {
            clear();
         }

         break;
   }
}

bool QVariant::canConvert(uint newType) const
{
   uint current_userType = userType();

   if (current_userType == newType) {
      // type did not change
      return true;
   }

   switch (newType) {

      case QVariant::Bool:

         if (current_userType == QVariant::Int || current_userType == QVariant::UInt) {
            return true;

         } else if (current_userType == QVariant::Long || current_userType == QVariant::ULong) {
            return true;

         } else if (current_userType == QVariant::LongLong || current_userType == QVariant::ULongLong) {
            return true;

         } else if (current_userType == QVariant::Double || current_userType == QVariant::Float) {
            return true;

         } else if (current_userType == QVariant::Short || current_userType == QVariant::UShort) {
            return true;

         } else if (current_userType == QVariant::Char || current_userType == QVariant::QChar) {
            return true;

         } else if (current_userType == QVariant::SChar || current_userType == QVariant::UChar) {
            return true;

         } else if (current_userType == QVariant::ByteArray) {
            return true;

         } else if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;

         }

         break;

      case QVariant::Int:

         if (current_userType == QVariant::Bool) {
            return true;

         } else if (current_userType == QVariant::UInt) {
            return true;

         } else if (current_userType == QVariant::Long || current_userType == QVariant::ULong) {
            return true;

         } else if (current_userType == QVariant::LongLong || current_userType == QVariant::ULongLong) {
            return true;

         } else if (current_userType == QVariant::Double || current_userType == QVariant::Float) {
            return true;

         } else if (current_userType == QVariant::Short || current_userType == QVariant::UShort) {
            return true;

         } else if (current_userType == QVariant::Char  || current_userType == QVariant::QChar) {
            return true;

         } else if (current_userType == QVariant::SChar || current_userType == QVariant::UChar) {
            return true;

         } else if (current_userType == QVariant::ByteArray) {
            return true;

         } else if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;

         } else {

            if (isEnum()) {
               return true;
            }
         }

         break;

      case QVariant::UInt:

         if (current_userType == QVariant::Bool) {
            return true;

         } else if (current_userType == QVariant::Int) {
            return true;

         } else if (current_userType == QVariant::Long || current_userType == QVariant::ULong) {
            return true;

         } else if (current_userType == QVariant::LongLong || current_userType == QVariant::ULongLong) {
            return true;

         } else if (current_userType == QVariant::Double || current_userType == QVariant::Float) {
            return true;

         } else if (current_userType == QVariant::Short || current_userType == QVariant::UShort) {
            return true;

         } else if (current_userType == QVariant::Char  || current_userType == QVariant::QChar) {
            return true;

         } else if (current_userType == QVariant::SChar || current_userType == QVariant::UChar) {
            return true;

         } else if (current_userType == QVariant::ByteArray) {
            return true;

         } else if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;
         }

         break;

      case QVariant::LongLong:

         if (current_userType == QVariant::Bool) {
            return true;

         } else if (current_userType == QVariant::Int || current_userType == QVariant::UInt) {
            return true;

         } else if (current_userType == QVariant::Long || current_userType == QVariant::ULong) {
            return true;

         } else if (current_userType == QVariant::ULongLong) {
            return true;

         } else if (current_userType == QVariant::Double || current_userType == QVariant::Float) {
            return true;

         } else if (current_userType == QVariant::Short || current_userType == QVariant::UShort) {
            return true;

         } else if (current_userType == QVariant::Char  || current_userType == QVariant::QChar) {
            return true;

         } else if (current_userType == QVariant::SChar || current_userType == QVariant::UChar) {
            return true;

         } else if (current_userType == QVariant::ByteArray) {
            return true;

         } else if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;

         }

         break;

      case QVariant::ULongLong:

         if (current_userType == QVariant::Bool) {
            return true;

         } else if (current_userType == QVariant::Int || current_userType == QVariant::UInt) {
            return true;

         } else if (current_userType == QVariant::Long || current_userType == QVariant::ULong) {
            return true;

         } else if (current_userType == QVariant::LongLong) {
            return true;

         } else if (current_userType == QVariant::Double || current_userType == QVariant::Float) {
            return true;

         } else if (current_userType == QVariant::Short || current_userType == QVariant::UShort) {
            return true;

         } else if (current_userType == QVariant::Char  || current_userType == QVariant::QChar) {
            return true;

         } else if (current_userType == QVariant::SChar || current_userType == QVariant::UChar) {
            return true;

         } else if (current_userType == QVariant::ByteArray) {
            return true;

         } else if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;

         }

         break;

      case QVariant::Short:
      case QVariant::UShort:
      case QVariant::Long:
      case QVariant::ULong:
      case QVariant::SChar:
      case QVariant::UChar:

         if (current_userType == QVariant::Bool) {
            return true;

         } else if (current_userType == QVariant::Int || current_userType == QVariant::UInt) {
            return true;

         } else if (current_userType == QVariant::Long || current_userType == QVariant::ULong) {
            return true;

         } else if (current_userType == QVariant::LongLong || current_userType == QVariant::ULongLong) {
            return true;

         } else if (current_userType == QVariant::Double || current_userType == QVariant::Float) {
            return true;

         } else if (current_userType == QVariant::Short || current_userType == QVariant::UShort) {
            return true;

         } else if (current_userType == QVariant::Char  || current_userType == QVariant::QChar) {
            return true;

         } else if (current_userType == QVariant::SChar || current_userType == QVariant::UChar) {
            return true;

         } else if (current_userType == QVariant::ByteArray) {
            return true;

         } else if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;

         }

         break;

      case QVariant::Double:

         if (current_userType == QVariant::Bool) {
            return true;

         } else if (current_userType == QVariant::Int || current_userType == QVariant::UInt) {
            return true;

         } else if (current_userType == QVariant::Long || current_userType == QVariant::ULong) {
            return true;

         } else if (current_userType == QVariant::LongLong || current_userType == QVariant::ULongLong) {
            return true;

         } else if (current_userType == QVariant::Float) {
            return true;

         } else if (current_userType == QVariant::Short || current_userType == QVariant::UShort) {
            return true;

         } else if (current_userType == QVariant::ByteArray) {
            return true;

         } else if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;
         }

         break;

      case QVariant::Float:

         if (current_userType == QVariant::Bool) {
            return true;

         } else if (current_userType == QVariant::Int || current_userType == QVariant::UInt) {
            return true;

         } else if (current_userType == QVariant::Long || current_userType == QVariant::ULong) {
            return true;

         } else if (current_userType == QVariant::LongLong || current_userType == QVariant::ULongLong) {
            return true;

         } else if (current_userType == QVariant::Double) {
            return true;

         } else if (current_userType == QVariant::Short || current_userType == QVariant::UShort) {
            return true;

         } else if (current_userType == QVariant::ByteArray) {
            return true;

         } else if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;

         }

         break;

      case QVariant::Char:

         if (current_userType == QVariant::Bool) {
            return true;

         } else if (current_userType == QVariant::Int || current_userType == QVariant::UInt) {
            return true;

         } else if (current_userType == QVariant::Long || current_userType == QVariant::ULong) {
            return true;

         } else if (current_userType == QVariant::LongLong || current_userType == QVariant::ULongLong) {
            return true;

         } else if (current_userType == QVariant::Double || current_userType == QVariant::Float) {
            return true;

         } else if (current_userType == QVariant::Short || current_userType == QVariant::UShort) {
            return true;

         } else if (current_userType == QVariant::QChar) {
            return true;

         } else if (current_userType == QVariant::SChar || current_userType == QVariant::UChar) {
            return true;

         } else if (current_userType == QVariant::ByteArray) {
            return true;

         } else if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;

         }

         break;

      case QVariant::QChar:

         if (current_userType == QVariant::Bool) {
            return true;

         } else if (current_userType == QVariant::Int || current_userType == QVariant::UInt) {
            return true;

         } else if (current_userType == QVariant::Long || current_userType == QVariant::ULong) {
            return true;

         } else if (current_userType == QVariant::LongLong || current_userType == QVariant::ULongLong) {
            return true;

         } else if (current_userType == QVariant::Double || current_userType == QVariant::Float) {
            return true;

         } else if (current_userType == QVariant::Short || current_userType == QVariant::UShort) {
            return true;

         } else if (current_userType == QVariant::Char) {
            return true;

         } else if (current_userType == QVariant::SChar || current_userType == QVariant::UChar) {
            return true;

         } else if (current_userType == QVariant::ByteArray) {
            return true;

         } else if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;

         }

         break;

      case QVariant::ByteArray:

         if (current_userType == QVariant::Bool) {
            return true;

         } else if (current_userType == QVariant::Int || current_userType == QVariant::UInt) {
            return true;

         } else if (current_userType == QVariant::Long || current_userType == QVariant::ULong) {
            return true;

         } else if (current_userType == QVariant::LongLong || current_userType == QVariant::ULongLong) {
            return true;

         } else if (current_userType == QVariant::Double || current_userType == QVariant::Float) {
            return true;

         } else if (current_userType == QVariant::Short || current_userType == QVariant::UShort) {
            return true;

         } else if (current_userType == QVariant::Char || current_userType == QVariant::QChar) {
            return true;

         } else if (current_userType == QVariant::SChar || current_userType == QVariant::UChar) {
            return true;

         } else if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::Color) {
            return true;

         } else if (current_userType == QVariant::Url) {
            return true;

         } else if (current_userType == QVariant::Uuid) {
            return true;
         }

         break;

      case QVariant::String8:
      case QVariant::String16:

         if (current_userType == QVariant::Bool) {
            return true;

         } else if (current_userType == QVariant::Int || current_userType == QVariant::UInt) {
            return true;

         } else if (current_userType == QVariant::Long || current_userType == QVariant::ULong) {
            return true;

         } else if (current_userType == QVariant::LongLong || current_userType == QVariant::ULongLong) {
            return true;

         } else if (current_userType == QVariant::Double || current_userType == QVariant::Float) {
            return true;

         } else if (current_userType == QVariant::Short || current_userType == QVariant::UShort) {
            return true;

         } else if (current_userType == QVariant::QChar) {
            return true;

         } else if (current_userType == QVariant::ByteArray) {
            return true;

         } else if (current_userType == QVariant::String || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::StringList) {
            return true;

         } else if (current_userType == QVariant::Date || current_userType == QVariant::DateTime) {
            return true;

         } else if (current_userType == QVariant::Time) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;

         } else if (current_userType == QVariant::Color) {
            return true;

         } else if (current_userType == QVariant::Font) {
            return true;

         } else if (current_userType == QVariant::KeySequence) {
            return true;

         } else if (current_userType == QVariant::Url) {
            return true;

         } else if (current_userType == QVariant::Uuid) {
            return true;

         }

         break;

      case QVariant::StringList:

         if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::List) {
            return true;
         }

         break;

      case QVariant::Image:

         if (current_userType == QVariant::Pixmap || current_userType == QVariant::Bitmap) {
            return true;
         }

         break;

      case QVariant::Pixmap:

         if (current_userType == QVariant::Image) {
            return true;

         } else if (current_userType == QVariant::Bitmap) {
            return true;

         } else if (current_userType == QVariant::Brush) {
            return true;
         }

         break;

      case QVariant::Bitmap:

         if (current_userType == QVariant::Pixmap || current_userType == QVariant::Image) {
            return true;
         }

         break;

      case QVariant::KeySequence:

         if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         }

         break;

      case QVariant::Font:

         if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;
         }

         break;

      case QVariant::Color:

         if (current_userType == QVariant::ByteArray) {
            return true;

         } else if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::Brush) {
            return true;

         }

         break;

      case QVariant::Brush:

         if (current_userType == QVariant::Color || current_userType == QVariant::Pixmap) {
            return true;
         }

         break;

      case QVariant::Date:
      case QVariant::Time:

         if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::DateTime) {
            return true;
         }

         break;

      case QVariant::DateTime:

         if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;

         } else if (current_userType == QVariant::Date) {
            return true;
         }

         break;

      case QVariant::List:

         if (current_userType == QVariant::StringList) {
            return true;

         } else if (current_userType == QVariant::JsonArray) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;

         }

         break;

      case QVariant::Map:

         if (current_userType == QVariant::Hash) {
            return true;

         } else if (current_userType == QVariant::JsonObject) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;

         }

         break;

      case QVariant::Hash:

         if (current_userType == QVariant::Map) {
            return true;

         } else if (current_userType == QVariant::JsonObject) {
            return true;

         } else if (current_userType == QVariant::JsonValue) {
            return true;

         }

         break;

      case QVariant::Line:

         if (current_userType == QVariant::LineF) {
            return true;
         }

         break;

      case QVariant::LineF:

         if (current_userType == QVariant::Line) {
            return true;
         }

         break;

      case QVariant::Point:

         if (current_userType == QVariant::PointF) {
            return true;
         }

         break;

      case QVariant::PointF:

         if (current_userType == QVariant::Point) {
            return true;
         }

         break;

      case QVariant::Rect:

         if (current_userType == QVariant::RectF) {
            return true;
         }

         break;

      case QVariant::RectF:

         if (current_userType == QVariant::Rect) {
            return true;
         }

         break;

      case QVariant::Size:

         if (current_userType == QVariant::SizeF) {
            return true;
         }

         break;

      case QVariant::SizeF:

         if (current_userType == QVariant::Size) {
            return true;
         }

         break;

      case QVariant::ModelIndex:

         if (current_userType == QVariant::PersistentModelIndex) {
            return true;
         }

         break;

      case QVariant::PersistentModelIndex:

         if (current_userType == QVariant::ModelIndex) {
            return true;
         }

         break;

      case QVariant::Url:

         if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;
         }

         break;

      case QVariant::Uuid:

         if (current_userType == QVariant::String8 || current_userType == QVariant::String16) {
            return true;
         }

         break;

      default:
         return false;
   }

   return false;
}

bool QVariant::cs_internal_compare(const QVariant &other) const
{
   QVariant::Type current_type = type();
   QVariant::Type other_type   = other.type();

   if (current_type != other_type) {

      if (cs_internal_isNumericType(current_type) && cs_internal_isNumericType(other_type)) {

         if (cs_internal_isFloatingPoint(current_type) || cs_internal_isFloatingPoint(other_type)) {
            return qFuzzyCompare(toReal(), other.toReal());

         } else {
            return toLongLong() == other.toLongLong();
         }
      }

      std::optional<QVariant> tmp = other.maybeConvert( static_cast<uint>(current_type));

      if (tmp.has_value()) {
         return compareValues(*this, *tmp);
      } else {
         return false;
      }
   }

   return compareValues(*this, other);
}

// private method
bool QVariant::compareValues(const QVariant &a, const QVariant &b)
{
   if (std::holds_alternative<std::shared_ptr<CustomType>>(a.m_data)) {
      // compare custom types

      auto a_ptr = std::get_if<std::shared_ptr<CustomType>>(&a.m_data);
      auto b_ptr = std::get_if<std::shared_ptr<CustomType>>(&b.m_data);

      // b_ptr is a raw pointer to a shared pointer to a CustomType
      // **b_ptr is a value of type CustomType
      return (*a_ptr)->compare(**b_ptr);

   } else {
      return a.m_data == b.m_data;

   }
}

// private
void QVariant::load(QDataStream &stream)
{
   clear();

   quint32 inputType;
   stream >> inputType;

   if (inputType == static_cast<quint32>(QVariant::UserType)) {
      QString name;
      stream >> name;

      inputType = static_cast<quint32>(QVariant::nameToType(name));

      if (inputType == 0) {
         stream.setStatus(QDataStream::ReadCorruptData);
         return;
      }
   }

   cs_internal_create(static_cast<uint>(inputType), nullptr);

   if (! isValid()) {
      return;
   }

   if (! cs_internal_load(stream, userType()) ) {
      stream.setStatus(QDataStream::ReadCorruptData);
      qWarning("QVariant::load() Unable to load Variant::Type %d from stream", userType());
   }
}

// private
void QVariant::save(QDataStream &stream) const
{
   QVariant::Type current_type = type();

   stream << static_cast<quint32>(current_type);

   if (current_type == QVariant::UserType) {
      stream << QVariant::typeToName(userType());
   }

   if (! isValid()) {
      return;
   }

   if (! cs_internal_save(stream, userType()) ) {
      qWarning("QVariant::save() Unable to save Variant::Type %d to data stream", userType());
   }
}

bool QVariant::cs_internal_load(QDataStream &stream, uint type)
{
   bool retval = true;

   switch (type) {

      case QVariant::Bool: {
         qint8 tmp;
         stream >> tmp;

         setValue<bool>(tmp);
         break;
      }

      case QVariant::Short: {
         qint16 tmp;
         stream >> tmp;

         setValue<short>(tmp);
         break;
      }

      case QVariant::UShort: {
         quint16 tmp;
         stream >> tmp;

         setValue<unsigned short>(tmp);
         break;
      }

      case QVariant::Int: {
         qint32 tmp;
         stream >> tmp;

         setValue<int>(tmp);
         break;
      }

      case QVariant::UInt: {
         quint32 tmp;
         stream >> tmp;

         setValue<unsigned int>(tmp);
         break;
      }

      case QVariant::Long: {
         qint64 tmp;
         stream >> tmp;

         setValue<long>(tmp);
         break;
      }

      case QVariant::ULong: {
         quint64 tmp;
         stream >> tmp;

         setValue<unsigned long>(tmp);
         break;
      }

      case QVariant::LongLong: {
         qint64 tmp;
         stream >> tmp;

         setValue<long long>(tmp);
         break;
      }

      case QVariant::ULongLong: {
         quint64 tmp;
         stream >> tmp;

         setValue<unsigned long long>(tmp);
         break;
      }

      case QVariant::Double: {
         double tmp;
         stream >> tmp;

         setValue<double>(tmp);
         break;
      }

      case QVariant::Float: {
         float tmp;
         stream >> tmp;

         setValue<float>(tmp);
         break;
      }

      case QVariant::Char: {
         // force a char to be signed
         signed char tmp;
         stream >> tmp;

         setValue<char>(tmp);
         break;
      }

      case QVariant::UChar: {
         unsigned char tmp;
         stream >> tmp;

         setValue<unsigned char>(tmp);
         break;
      }

      case QVariant::SChar: {
         // force a char to be signed
         signed char tmp;
         stream >> tmp;

         setValue<signed char>(tmp);
         break;
      }

      case QVariant::QChar: {
         QChar32 tmp;
         stream >> tmp;

         setValue<QChar32>(tmp);
         break;
      }

      case QVariant::BitArray: {
         QBitArray tmp;
         stream >> tmp;

         setValue<QBitArray>(tmp);
         break;
      }

      case QVariant::ByteArray: {
         QByteArray tmp;
         stream >> tmp;

         setValue<QByteArray>(tmp);
         break;
      }

      case QVariant::String8: {
         QString tmp;
         stream >> tmp;

         setValue<QString>(tmp);
         break;
      }

      case QVariant::String16: {
         QString16 tmp;
         stream >> tmp;

         setValue<QString16>(tmp);
         break;
      }

      case QVariant::RegularExpression: {
         QRegularExpression tmp;
         stream >> tmp;

         setValue<QRegularExpression>(tmp);
         break;
      }

      case QVariant::StringList: {
         QStringList tmp;
         stream >> tmp;

         setValue<QStringList>(tmp);
         break;
      }

      case QVariant::List: {
         QVariantList tmp;
         stream >> tmp;

         setValue<QVariantList>(tmp);
         break;
      }

      case QVariant::Map: {
         QVariantMap tmp;
         stream >> tmp;

         setValue<QVariantMap>(tmp);
         break;
      }

      case QVariant::MultiMap: {
         QVariantMultiMap tmp;
         stream >> tmp;

         setValue<QVariantMultiMap>(tmp);
         break;
      }

      case QVariant::Hash: {
         QVariantHash tmp;
         stream >> tmp;

         setValue<QVariantHash>(tmp);
         break;
      }

      case QVariant::MultiHash: {
         QVariantMultiHash tmp;
         stream >> tmp;

         setValue<QVariantMultiHash>(tmp);
         break;
      }

      case QVariant::Date: {
         QDate tmp;
         stream >> tmp;

         setValue<QDate>(tmp);
         break;
      }

      case QVariant::Time:  {
         QTime tmp;
         stream >> tmp;

         setValue<QTime>(tmp);
         break;
      }

      case QVariant::DateTime:  {
         QDateTime tmp;
         stream >> tmp;

         setValue<QDateTime>(tmp);
         break;
      }

      case QVariant::Locale: {
         QLocale tmp;
         stream >> tmp;

         setValue<QLocale>(tmp);
         break;
      }

      case QVariant::JsonValue:
      case QVariant::JsonObject:
      case QVariant::JsonArray:
      case QVariant::JsonDocument:
         return false;

      case QVariant::Line:  {
         QLine tmp;
         stream >> tmp;

         setValue<QLine>(tmp);
         break;
      }

      case QVariant::LineF:  {
         QLineF tmp;
         stream >> tmp;

         setValue<QLineF>(tmp);
         break;
      }

      case QVariant::Point:  {
         QPoint tmp;
         stream >> tmp;

         setValue<QPoint>(tmp);
         break;
      }

      case QVariant::PointF:  {
         QPointF tmp;
         stream >> tmp;

         setValue<QPointF>(tmp);
         break;
      }

      case QVariant::Rect: {
         QRect tmp;
         stream >> tmp;

         setValue<QRect>(tmp);
         break;
      }

      case QVariant::RectF:  {
         QRectF tmp;
         stream >> tmp;

         setValue<QRectF>(tmp);
         break;
      }

      case QVariant::Size:  {
         QSize tmp;
         stream >> tmp;

         setValue<QSize>(tmp);
         break;
      }

      case QVariant::SizeF:  {
         QSizeF tmp;
         stream >> tmp;

         setValue<QSizeF>(tmp);
         break;
      }

      case QVariant::EasingCurve:  {
         QEasingCurve tmp;
         stream >> tmp;

         setValue<QEasingCurve>(tmp);
         break;
      }

      case QVariant::ModelIndex:
      case QVariant::PersistentModelIndex:
         return false;

      case QVariant::Url:  {
         QUrl tmp;
         stream >> tmp;

         setValue<QUrl>(tmp);
         break;
      }

      case QVariant::Uuid:  {
         QUuid tmp;
         stream >> tmp;

         setValue<QUuid>(tmp);
         break;
      }

      case QVariant::Invalid:
      case QVariant::Void:
      case QVariant::VoidStar:
      case QVariant::ObjectStar:
      case QVariant::WidgetStar:
         return false;

      default:
         // call cs_internal_save in gui, etc
         bool done = false;

         for (const auto ptr : m_variantClients) {
            done = ptr->cs_internal_load(stream, type, *this);

            if (done) {
               break;
            }
         }

         if (! done)  {
            return false;
         }

         break;
   }

   return retval;
}

bool QVariant::cs_internal_save(QDataStream &stream, uint type) const
{
   bool retval = true;

   switch (type) {

      case QVariant::Bool:
         stream << static_cast<qint8>(getData<bool>());
         break;

      case QVariant::Short:
         stream << static_cast<qint16>(getData<short>());
         break;

      case QVariant::UShort:
         stream << static_cast<quint16>(getData<unsigned short>());
         break;

      case QVariant::Int:
         stream << static_cast<qint32>(getData<int>());
         break;

      case QVariant::UInt:
         stream << static_cast<qint32>(getData<unsigned int>());
         break;

      case QVariant::Long:
         stream << static_cast<qint64>(getData<long>());
         break;

      case QVariant::ULong:
         stream << static_cast<qint64>(getData<unsigned long>());
         break;

      case QVariant::LongLong:
         stream << static_cast<qint64>(getData<long long>());
         break;

      case QVariant::ULongLong:
         stream << static_cast<qint64>(getData<unsigned long long>());
         break;

      case QVariant::Double:
         stream << static_cast<double>(getData<double>());
         break;

      case QVariant::Float:
         stream << static_cast<float>(getData<float>());
         break;

      case QVariant::Char:
         // force a char to be signed
         stream << static_cast<signed char>(getData<char>());
         break;

      case QVariant::UChar:
         stream << static_cast<unsigned char>(getData<unsigned char>());
         break;

      case QVariant::SChar:
         stream << static_cast<signed char>(getData<signed char>());
         break;

      case QVariant::QChar:
         stream << static_cast<QChar32>(getData<QChar32>());
         break;

      case QVariant::BitArray:
         stream << static_cast<QBitArray>(getData<QBitArray>());
         break;

      case QVariant::ByteArray:
         stream << static_cast<QByteArray>(getData<QByteArray>());
         break;

      case QVariant::String8:
         stream << static_cast<QString>(getData<QString>());
         break;

      case QVariant::String16:
         stream << static_cast<QString16>(getData<QString16>());
         break;

      case QVariant::RegularExpression:
         stream << static_cast<QRegularExpression>(getData<QRegularExpression>());
         break;

      case QVariant::StringList:
         stream << static_cast<QStringList>(getData<QStringList>());
         break;

      case QVariant::List:
         stream << static_cast<QVariantList>(getData<QVariantList>());
         break;

      case QVariant::Map:
         stream << static_cast<QVariantMap>(getData<QVariantMap>());
         break;

      case QVariant::MultiMap:
         stream << static_cast<QVariantMultiMap>(getData<QVariantMultiMap>());
         break;

      case QVariant::Hash:
         stream << static_cast<QVariantHash>(getData<QVariantHash>());
         break;

      case QVariant::MultiHash:
         stream << static_cast<QVariantMultiHash>(getData<QVariantMultiHash>());
         break;

      case QVariant::Date:
         stream << static_cast<QDate>(getData<QDate>());
         break;

      case QVariant::Time:
         stream << static_cast<QTime>(getData<QTime>());
         break;

      case QVariant::DateTime:
         stream << static_cast<QDateTime>(getData<QDateTime>());
         break;

      case QVariant::Locale:
         stream << static_cast<QLocale>(getData<QLocale>());
         break;

      case QVariant::JsonValue:
      case QVariant::JsonObject:
      case QVariant::JsonArray:
      case QVariant::JsonDocument:
         return false;

      case QVariant::Line:
         stream << static_cast<QLine>(getData<QLine>());
         break;

      case QVariant::LineF:
         stream << static_cast<QLineF>(getData<QLineF>());
         break;

      case QVariant::Point:
         stream << static_cast<QPoint>(getData<QPoint>());
         break;

      case QVariant::PointF:
         stream << static_cast<QPointF>(getData<QPointF>());
         break;

      case QVariant::Rect:
         stream << static_cast<QRect>(getData<QRect>());
         break;

      case QVariant::RectF:
         stream << static_cast<QRectF>(getData<QRectF>());
         break;

      case QVariant::Size:
         stream << static_cast<QSize>(getData<QSize>());
         break;

      case QVariant::SizeF:
         stream << static_cast<QSizeF>(getData<QSizeF>());
         break;

      case QVariant::EasingCurve:
         stream << static_cast<QEasingCurve>(getData<QEasingCurve>());
         break;

      case QVariant::ModelIndex:
      case QVariant::PersistentModelIndex:
         return false;

      case QVariant::Url:
         stream << static_cast<QUrl>(getData<QUrl>());
         break;

      case QVariant::Uuid:
         stream << static_cast<QUuid>(getData<QUuid>());
         break;

      case QVariant::Invalid:
      case QVariant::Void:
      case QVariant::VoidStar:
      case QVariant::ObjectStar:
      case QVariant::WidgetStar:
         return false;

      default:
         // call cs_internal_save in gui, etc
         bool done = false;

         for (const auto ptr : m_variantClients) {
            done = ptr->cs_internal_save(stream, type, *this);

            if (done) {
               break;
            }
         }

         if (! done)  {
            return false;
         }

         break;
   }

   return retval;
}

template <typename T>
T QVariant::cs_internal_VariantToType(QVariant::Type type, bool *ok) const
{
   uint new_userType = static_cast<uint>(type);

   if (ok != nullptr) {
      *ok = true;
   }

   if (userType() == new_userType) {
      return getData<T>();
   }

   QVariant newVariant = *this;

   T retval{};

   if (newVariant.convert(type)) {
      retval = newVariant.value<T>();

   } else if (ok != nullptr)  {
      *ok = false;

   }

   return retval;
}

void QVariant::clear()
{
   m_data = std::monostate();
}

bool QVariant::convert(uint newType)
{
   uint current_userType = userType();

   if (current_userType == newType) {
      // type did not change
      return true;
   }

   if (! isValid()) {
      cs_internal_create(newType, nullptr);
      return false;

   } else if (! canConvert(newType)) {
      cs_internal_create(newType, nullptr);
      return false;
   }

   bool isOk = true;

   if (! cs_internal_convert(current_userType, newType)) {
      cs_internal_create(newType, nullptr);
      isOk = false;
   }

   return isOk;
}

std::optional<QVariant> QVariant::maybeConvert(uint requested_type) const
{
   std::optional<QVariant> retval;

   uint current_userType = userType();

   if (current_userType == requested_type) {
      // types did not change
      retval = *this;

   } else if (! isValid()) {
      // do nothing

   } else if (! canConvert(requested_type)) {
      // do nothing

   } else {
      QVariant tmp = *this;

      if (tmp.cs_internal_convert(current_userType, requested_type)) {
         retval = tmp;
      }
   }

   return retval;
}

// private method
uint QVariant::getTypeId(const std::type_index &index)
{
   uint retval = QVariant::Invalid;

   for (const auto &item : builtinTypes) {

#if defined(Q_OS_DARWIN) || defined(Q_OS_FREEBSD)
      // does not support comparing hash_code()

      if (strcmp(item.meta_typeT.name(), index.name()) == 0) {
         retval = item.meta_typeId;
         break;
      }

#else

      if (item.meta_typeT == index)  {
         retval = item.meta_typeId;
         break;
      }

#endif

   }

   if (retval == QVariant::Invalid) {
      for (const auto &item : m_userTypes) {
         if (item.meta_typeT == index)  {
            retval = item.meta_typeId;
            break;
         }
      }
   }

   return retval;
}

// private method
uint QVariant::getTypeId(QString name)
{
   uint retval = QVariant::Invalid;

   for (const auto &item : builtinTypes) {
      if (strcmp(item.meta_typeName, name.constData()) == 0)  {
         retval = item.meta_typeId;
         break;
      }
   }

   if (retval == QVariant::Invalid) {
      for (const auto &item : m_userTypes) {
         if (strcmp(item.meta_typeName, name.constData()) == 0)  {
            retval = item.meta_typeId;
            break;
         }
      }
   }

   return retval;
}

// private method
QString QVariant::getTypeName(uint typeId)
{
   QString retval;

   for (const auto &item : builtinTypes) {
      if (item.meta_typeId == typeId)  {
         retval = QString::fromLatin1(item.meta_typeName);
         break;
      }
   }

   if (retval.isEmpty()) {
      for (const auto &item : m_userTypes) {
         if (item.meta_typeId == typeId)  {
            retval = QString::fromLatin1(item.meta_typeName);
            break;
         }
      }
   }

   return retval;
}

bool QVariant::isEnum()  const
{
   // get a ptr to the requested alternative
   auto ptr = std::get_if<std::shared_ptr<CustomType>>(&(m_data));

   if (ptr == nullptr) {
      // simple type
      return false;

   } else {
      // custom type
      return (*ptr)->isEnum();
   }
}

int64_t QVariant::enumToInt() const
{
   // get a ptr to the requested alternative
   auto ptr = std::get_if<std::shared_ptr<CustomType>>(&(m_data));

   if (ptr == nullptr) {
      // simple type
      return false;

   } else {
      // custom type
      return (*ptr)->enumToInt();
   }
}

uint64_t QVariant::enumToUInt() const
{
   // get a ptr to the requested alternative
   auto ptr = std::get_if<std::shared_ptr<CustomType>>(&(m_data));

   if (ptr == nullptr) {
      // simple type
      return false;

   } else {
      // custom type
      return (*ptr)->enumToUInt();
   }
}

uint QVariant::nameToType(const QString &name)
{
   if (name.isEmpty())  {
      return QVariant::Invalid;

   } else if (name == "UserType")  {
      return QVariant::UserType;

   }

   uint retval = QVariant::getTypeId(name);

   return retval;
}

//
bool QVariant::toBool(bool *ok) const
{
   return cs_internal_VariantToType<bool>(QVariant::Bool, ok);
}

int QVariant::toInt(bool *ok) const
{
   return cs_internal_VariantToType<int>(QVariant::Int, ok);
}

uint QVariant::toUInt(bool *ok) const
{
   return cs_internal_VariantToType<uint>(QVariant::UInt, ok);
}

long QVariant::toLong(bool *ok) const
{
   return cs_internal_VariantToType<long>(QVariant::Long, ok);
}

ulong QVariant::toULong(bool *ok) const
{
   return cs_internal_VariantToType<ulong>(QVariant::ULong, ok);
}

qint64 QVariant::toLongLong(bool *ok) const
{
   return cs_internal_VariantToType<qint64>(QVariant::LongLong, ok);
}

quint64 QVariant::toULongLong(bool *ok) const
{
   return cs_internal_VariantToType<quint64>(QVariant::ULongLong, ok);
}

double QVariant::toDouble(bool *ok) const
{
   return cs_internal_VariantToType<double>(QVariant::Double, ok);
}

float QVariant::toFloat(bool *ok) const
{
   return cs_internal_VariantToType<float>(QVariant::Float, ok);
}

qreal QVariant::toReal(bool *ok) const
{
   return cs_internal_VariantToType<double>(QVariant::Double, ok);
}

QChar32 QVariant::toChar() const
{
   return cs_internal_VariantToType<QChar32>(QVariant::QChar);
}

QString QVariant::toString() const
{
   return cs_internal_VariantToType<QString>(QVariant::String);
}

QString16 QVariant::toString16() const
{
   return cs_internal_VariantToType<QString16>(QVariant::String16);
}

QByteArray QVariant::toByteArray() const
{
   return cs_internal_VariantToType<QByteArray>(QVariant::ByteArray);
}

QBitArray QVariant::toBitArray() const
{
   return cs_internal_VariantToType<QBitArray>(QVariant::BitArray);
}

QStringList QVariant::toStringList() const
{
   return cs_internal_VariantToType<QStringList>(QVariant::StringList);
}

QRegularExpression8 QVariant::toRegularExpression() const
{
   return cs_internal_VariantToType<QRegularExpression8>(QVariant::RegularExpression);
}

QDate QVariant::toDate() const
{
   return cs_internal_VariantToType<QDate>(QVariant::Date);
}

QTime QVariant::toTime() const
{
   return cs_internal_VariantToType<QTime>(QVariant::Time);
}

QDateTime QVariant::toDateTime() const
{
   return cs_internal_VariantToType<QDateTime>(QVariant::DateTime);
}

QLocale QVariant::toLocale() const
{
   return cs_internal_VariantToType<QLocale>(QVariant::Locale);
}

QVariantList QVariant::toList() const
{
   return cs_internal_VariantToType<QVariantList>(QVariant::List);
}

QVariantMap QVariant::toMap() const
{
   return cs_internal_VariantToType<QVariantMap>(QVariant::Map);
}

QVariantMultiMap QVariant::toMultiMap() const
{
   return cs_internal_VariantToType<QVariantMultiMap>(QVariant::MultiMap);
}

QVariantHash QVariant::toHash() const
{
   return cs_internal_VariantToType<QVariantHash>(QVariant::Hash);
}

QVariantMultiHash QVariant::toMultiHash() const
{
   return cs_internal_VariantToType<QVariantMultiHash>(QVariant::MultiHash);
}

QJsonValue QVariant::toJsonValue() const
{
   return cs_internal_VariantToType<QJsonValue>(QVariant::JsonValue);
}

QJsonObject QVariant::toJsonObject() const
{
   return cs_internal_VariantToType<QJsonObject>(QVariant::JsonObject);
}

QJsonArray QVariant::toJsonArray() const
{
   return cs_internal_VariantToType<QJsonArray>(JsonArray);
}

QJsonDocument QVariant::toJsonDocument() const
{
   return cs_internal_VariantToType<QJsonDocument>(QVariant::JsonDocument);
}

QLine QVariant::toLine() const
{
   return cs_internal_VariantToType<QLine>(QVariant::Line);
}

QLineF QVariant::toLineF() const
{
   return cs_internal_VariantToType<QLineF>(QVariant::LineF);
}

QPoint QVariant::toPoint() const
{
   return cs_internal_VariantToType<QPoint>(QVariant::Point);
}

QPointF QVariant::toPointF() const
{
   return cs_internal_VariantToType<QPointF>(QVariant::PointF);
}

QRect QVariant::toRect() const
{
   return cs_internal_VariantToType<QRect>(QVariant::Rect);
}

QRectF QVariant::toRectF() const
{
   return cs_internal_VariantToType<QRectF>(QVariant::RectF);
}

QSize QVariant::toSize() const
{
   return cs_internal_VariantToType<QSize>(QVariant::Size);
}

QSizeF QVariant::toSizeF() const
{
   return cs_internal_VariantToType<QSizeF>(QVariant::SizeF);
}

QEasingCurve QVariant::toEasingCurve() const
{
   return cs_internal_VariantToType<QEasingCurve>(QVariant::EasingCurve);
}

QModelIndex QVariant::toModelIndex() const
{
   return cs_internal_VariantToType<QModelIndex>(QVariant::ModelIndex);
}

QPersistentModelIndex QVariant::toPersistentModelIndex() const
{
   return cs_internal_VariantToType<QPersistentModelIndex>(QVariant::PersistentModelIndex);
}

QUrl QVariant::toUrl() const
{
   return cs_internal_VariantToType<QUrl>(QVariant::Url);
}

QUuid QVariant::toUuid() const
{
   return cs_internal_VariantToType<QUuid>(QVariant::Uuid);
}

QVariant::Type QVariant::type() const
{
   return std::visit([](const auto &arg) {
      using T = std::decay_t<decltype(arg)>;

      if constexpr(std::is_same_v<T, std::monostate>) {
         return QVariant::Invalid;

      } else if constexpr(std::is_same_v<T, bool>) {
         return QVariant::Bool;

      } else if constexpr(std::is_same_v<T, char>) {
         return QVariant::Char;

      } else if constexpr(std::is_same_v<T, int>) {
         return QVariant::Int;

      } else if constexpr(std::is_same_v<T, uint>) {
         return QVariant::UInt;

      } else if constexpr(std::is_same_v<T, qint64>) {
         return QVariant::LongLong;

      } else if constexpr(std::is_same_v<T, quint64>) {
         return QVariant::ULongLong;

      } else if constexpr(std::is_same_v<T, double>) {
         return QVariant::Double;

      } else if constexpr(std::is_same_v<T, float>) {
         return QVariant::Float;

      } else if constexpr(std::is_same_v<T, QChar32>)  {
         return QVariant::QChar;

      } else if constexpr(std::is_same_v<T, QString>) {
         return QVariant::String;

      } else if constexpr(std::is_same_v<T, QObject *>) {
         return QVariant::ObjectStar;

      } else if constexpr(std::is_same_v<T, void *>) {
         return QVariant::VoidStar;

      } else if constexpr(std::is_same_v<T, std::shared_ptr<CustomType>>)  {
         uint tmp = arg->userType();

         if (tmp >= QVariant::UserType) {
            return QVariant::UserType;

         } else {
            return static_cast<QVariant::Type>(tmp);
         }

      } else {
         static_assert(! std::is_same_v<T, T>, "QVariant::type() Unsupported data type");
      }

   }, m_data);
}

QString8 QVariant::typeName() const
{
   return typeToName(userType());
}

QString8 QVariant::typeToName(uint typeId)
{
   if (typeId == static_cast<uint>(QVariant::Invalid)) {
      static const QString8 retval;
      return retval;

   } else if (typeId == static_cast<uint>(QVariant::UserType)) {
      static const QString8 retval("UserType");
      return retval;
   }

   return QVariant::getTypeName(typeId);
}

uint QVariant::userType() const
{
   return std::visit([](const auto &arg) {
      using T = std::decay_t<decltype(arg)>;

      if constexpr(std::is_same_v<T, std::monostate>) {
         return static_cast<uint>(QVariant::Invalid);

      } else if constexpr(std::is_same_v<T, bool>) {
         return static_cast<uint>(QVariant::Bool);

      } else if constexpr(std::is_same_v<T, char>) {
         return static_cast<uint>(QVariant::Char);

      } else if constexpr(std::is_same_v<T, int>) {
         return static_cast<uint>(QVariant::Int);

      } else if constexpr(std::is_same_v<T, uint>) {
         return static_cast<uint>(QVariant::UInt);

      } else if constexpr(std::is_same_v<T, qint64>) {
         return static_cast<uint>(QVariant::LongLong);

      } else if constexpr(std::is_same_v<T, quint64>) {
         return static_cast<uint>(QVariant::ULongLong);

      } else if constexpr(std::is_same_v<T, double>) {
         return static_cast<uint>(QVariant::Double);

      } else if constexpr(std::is_same_v<T, float>) {
         return static_cast<uint>(QVariant::Float);

      } else if constexpr(std::is_same_v<T, QChar32>)  {
         return static_cast<uint>(QVariant::QChar);

      } else if constexpr(std::is_same_v<T, QString>) {
         return static_cast<uint>(QVariant::String);

      } else if constexpr(std::is_same_v<T, QObject *>) {
         return static_cast<uint>(QVariant::ObjectStar);

      } else if constexpr(std::is_same_v<T, void *>) {
         return static_cast<uint>(QVariant::VoidStar);

      } else if constexpr(std::is_same_v<T, std::shared_ptr<CustomType>>)  {
         uint tmp = arg->userType();
         return tmp;

      } else {
         static_assert(! std::is_same_v<T, T>, "QVariant::userType() Unsupported data type");
      }

   }, m_data);
}

QVariant &QVariant::operator=(const QVariant &other)
{
   if (this == &other) {
      return *this;
   }

   // get a ptr to the requested alternative
   auto ptr = std::get_if<std::shared_ptr<CustomType>>(&(other.m_data));

   if (ptr == nullptr) {
      // simple type
      m_data = other.m_data;

   } else {
      // Custom type

      // ptr is a raw pointer to the shared pointer in the other Variant
      // clone() needs to do a deep copy
      m_data = (*ptr)->clone();
   }

   return *this;
}

QDataStream &operator>>(QDataStream &stream, QVariant &data)
{
   data.load(stream);
   return stream;
}

QDataStream &operator>>(QDataStream &stream, QVariant::Type &typeId)
{
   quint32 tmp;

   // load an enum value
   stream >> tmp;
   typeId = static_cast<QVariant::Type>(tmp);

   return stream;
}

QDataStream &operator<<(QDataStream &stream, const QVariant &data)
{
   data.save(stream);
   return stream;
}

QDataStream &operator<<(QDataStream &stream, const QVariant::Type typeId)
{
   // save an enum value
   stream << static_cast<quint32>(typeId);

   return stream;
}

QDebug &operator<<(QDebug &debug, const QVariant &value)
{
   if (! value.isValid()) {
      debug << "QVariant(Invalid)";

   } else if (value.canConvert<QString>()) {
      debug << "QVariant(" << value.typeName() << "," << value.toString() << ")";

   } else {
      debug << "QVariant(" << value.typeName() << ", unknown)";

   }

   return debug;
}
