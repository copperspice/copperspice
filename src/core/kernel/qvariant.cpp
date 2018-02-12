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

#include <qvariant.h>
#include <qbitarray.h>
#include <qbytearray.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qmap.h>
#include <qmultihash.h>
#include <qdatetime.h>
#include <qeasingcurve.h>
#include <qlist.h>
#include <qstring.h>
#include <qstring8.h>
#include <qstring16.h>
#include <qstringlist.h>
#include <qurl.h>
#include <qlocale.h>
#include <quuid.h>

#include <qabstractitemmodel.h>
#include <qjsonvalue.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qvariant_p.h>
#include <qsize.h>
#include <qpoint.h>
#include <qrect.h>
#include <qline.h>

#include <float.h>

QT_BEGIN_NAMESPACE

#ifndef DBL_DIG
#  define DBL_DIG 10
#endif

#ifndef FLT_DIG
#  define FLT_DIG 6
#endif

static void construct(QVariant::Private *x, const void *copy)
{
   x->is_shared = false;

   switch (x->type) {

      case QVariant::Bool:
         x->data.b = copy ? *static_cast<const bool *>(copy) : false;
         break;

      case QVariant::Int:
         x->data.i = copy ? *static_cast<const int *>(copy) : 0;
         break;

      case QVariant::UInt:
         x->data.u = copy ? *static_cast<const uint *>(copy) : 0u;
         break;

      case QVariant::LongLong:
         x->data.ll = copy ? *static_cast<const qint64 *>(copy) : Q_INT64_C(0);
         break;

      case QVariant::ULongLong:
         x->data.ull = copy ? *static_cast<const quint64 *>(copy) : Q_UINT64_C(0);
         break;

      case QVariant::Double:
         x->data.d = copy ? *static_cast<const double *>(copy) : 0.0;
         break;

      case QVariant::Char:
         v_construct<QChar>(x, copy);
         break;

      case QVariant::Char32:
         // broom (wait) v_construct<QChar32>(x, copy);
         break;

      case QVariant::List:
         v_construct<QVariantList>(x, copy);
         break;

      // fell off
      case QMetaType::Float:
         x->data.f = copy ? *static_cast<const float *>(copy) : 0.0f;
         break;

      case QVariant::String:
         v_construct<QString>(x, copy);
         break;

      case QVariant::String8:
         v_construct<QString8>(x, copy);
         break;

      case QVariant::String16:
         // broom (wait) v_construct<QString16>(x, copy);
         break;

      case QVariant::StringList:
         v_construct<QStringList>(x, copy);
         break;

      case QVariant::ByteArray:
         v_construct<QByteArray>(x, copy);
         break;

      case QVariant::BitArray:
         v_construct<QBitArray>(x, copy);
         break;

      case QVariant::Date:
         v_construct<QDate>(x, copy);
         break;

      case QVariant::Time:
         v_construct<QTime>(x, copy);
         break;

      case QVariant::DateTime:
         v_construct<QDateTime>(x, copy);
         break;

      case QVariant::Url:
         v_construct<QUrl>(x, copy);
         break;

      case QVariant::Locale:
         v_construct<QLocale>(x, copy);
         break;

      case QVariant::Rect:
         v_construct<QRect>(x, copy);
         break;

      case QVariant::RectF:
         v_construct<QRectF>(x, copy);
         break;

      case QVariant::Size:
         v_construct<QSize>(x, copy);
         break;

      case QVariant::SizeF:
         v_construct<QSizeF>(x, copy);
         break;

      case QVariant::Line:
         v_construct<QLine>(x, copy);
         break;

      case QVariant::LineF:
         v_construct<QLineF>(x, copy);
         break;

      case QVariant::Point:
         v_construct<QPoint>(x, copy);
         break;

      case QVariant::PointF:
         v_construct<QPointF>(x, copy);
         break;

#ifndef QT_NO_REGEXP
      case QVariant::RegExp:
         v_construct<QRegExp>(x, copy);
         break;
#endif

      case QVariant::Map:
         v_construct<QVariantMap>(x, copy);
         break;

      case QVariant::MultiMap:
         v_construct<QVariantMultiMap>(x, copy);
         break;

      case QVariant::Hash:
         v_construct<QVariantHash>(x, copy);
         break;

      case QVariant::MultiHash:
         v_construct<QVariantMultiHash>(x, copy);
         break;

      case QVariant::EasingCurve:
         v_construct<QEasingCurve>(x, copy);
         break;

      case QVariant::Uuid:
         v_construct<QUuid>(x, copy);
         break;

      case QVariant::ModelIndex:
         v_construct<QModelIndex>(x, copy);
         break;

      case QMetaType::QObjectStar:
         x->data.o = copy ? *static_cast<QObject *const *>(copy) : 0;
         break;

      case QVariant::Invalid:
      case QVariant::UserType:
         break;

      default:
         void *ptr = QMetaType::construct(x->type, copy);

         if (! ptr) {
            x->type = QVariant::Invalid;

         } else {
            x->is_shared = true;
            x->data.shared = new QVariant::PrivateShared(ptr);
         }

         break;
   }

   x->is_null = ! copy;
}

static void clear(QVariant::Private *d)
{
   switch (d->type) {
      case QVariant::Char:
         v_clear<QChar>(d);
         break;

      case QVariant::Char32:
         // broom (wait) v_clear<QChar32>(d);
         break;

      case QVariant::String:
         v_clear<QString>(d);
         break;

      case QVariant::String8:
         v_clear<QString8>(d);
         break;

      case QVariant::String16:
         // broom (wait) v_clear<QString16>(d);
         break;

      case QVariant::StringList:
         v_clear<QStringList>(d);
         break;

      case QVariant::Map:
         v_clear<QVariantMap>(d);
         break;

      case QVariant::MultiMap:
         v_clear<QVariantMultiMap>(d);
         break;

      case QVariant::Hash:
         v_clear<QVariantHash>(d);
         break;

      case QVariant::MultiHash:
         v_clear<QVariantMultiHash>(d);
         break;

      case QVariant::List:
         v_clear<QVariantList>(d);
         break;

      case QVariant::Date:
         v_clear<QDate>(d);
         break;

      case QVariant::Time:
         v_clear<QTime>(d);
         break;

      case QVariant::DateTime:
         v_clear<QDateTime>(d);
         break;

      case QVariant::ByteArray:
         v_clear<QByteArray>(d);
         break;

      case QVariant::BitArray:
         v_clear<QBitArray>(d);
         break;

      case QVariant::Point:
         v_clear<QPoint>(d);
         break;

      case QVariant::PointF:
         v_clear<QPointF>(d);
         break;

      case QVariant::Size:
         v_clear<QSize>(d);
         break;

      case QVariant::SizeF:
         v_clear<QSizeF>(d);
         break;

      case QVariant::Rect:
         v_clear<QRect>(d);
         break;

      case QVariant::RectF:
         v_clear<QRectF>(d);
         break;

      case QVariant::LineF:
         v_clear<QLineF>(d);
         break;

      case QVariant::Line:
         v_clear<QLine>(d);
         break;

      case QVariant::Url:
         v_clear<QUrl>(d);
         break;

      case QVariant::Locale:
         v_clear<QLocale>(d);
         break;

#ifndef QT_NO_REGEXP
      case QVariant::RegExp:
         v_clear<QRegExp>(d);
         break;
#endif

      case QVariant::EasingCurve:
         v_clear<QEasingCurve>(d);
         break;

      case QVariant::Uuid:
         v_clear<QUuid>(d);
         break;

      case QVariant::ModelIndex:
         v_clear<QModelIndex>(d);
         break;

      case QVariant::LongLong:
      case QVariant::ULongLong:
      case QVariant::Double:
      case QMetaType::Float:
      case QMetaType::QObjectStar:
         break;

      case QVariant::Invalid:
      case QVariant::UserType:
      case QVariant::Int:
      case QVariant::UInt:
      case QVariant::Bool:
         break;

      default:
         QMetaType::destroy(d->type, d->data.shared->ptr);
         delete d->data.shared;
         break;
   }

   d->type = QVariant::Invalid;
   d->is_shared = false;
   d->is_null   = true;
   d->is_ptr    = false;
}

static bool isNull(const QVariant::Private *d)
{
   switch (d->type) {

      case QVariant::Char:
         return v_cast<QChar>(d)->isNull();

      case QVariant::Char32:
         // broom (wait) return v_cast<QChar32>(d)->isNull();
         break;

      case QVariant::String:
         return v_cast<QString>(d)->isNull();

      case QVariant::String8:
         break;

      case QVariant::String16:
         break;

      case QVariant::Date:
         return v_cast<QDate>(d)->isNull();

      case QVariant::Time:
         return v_cast<QTime>(d)->isNull();

      case QVariant::DateTime:
         return v_cast<QDateTime>(d)->isNull();

      case QVariant::ByteArray:
         return v_cast<QByteArray>(d)->isNull();

      case QVariant::BitArray:
         return v_cast<QBitArray>(d)->isNull();

      case QVariant::Size:
         return v_cast<QSize>(d)->isNull();

      case QVariant::SizeF:
         return v_cast<QSizeF>(d)->isNull();

      case QVariant::Rect:
         return v_cast<QRect>(d)->isNull();

      case QVariant::RectF:
         return v_cast<QRectF>(d)->isNull();

      case QVariant::Line:
         return v_cast<QLine>(d)->isNull();

      case QVariant::LineF:
         return v_cast<QLineF>(d)->isNull();

      case QVariant::Point:
         return v_cast<QPoint>(d)->isNull();

      case QVariant::PointF:
         return v_cast<QPointF>(d)->isNull();

      case QVariant::EasingCurve:
      case QVariant::Uuid:
      case QVariant::ModelIndex:
      case QVariant::Url:
      case QVariant::Locale:
      case QVariant::RegExp:
      case QVariant::StringList:

      case QVariant::Map:
      case QVariant::MultiMap:
      case QVariant::Hash:
      case QVariant::MultiHash:

      case QVariant::List:
      case QVariant::Invalid:
      case QVariant::UserType:
      case QVariant::Int:
      case QVariant::UInt:
      case QVariant::LongLong:
      case QVariant::ULongLong:
      case QVariant::Bool:
      case QVariant::Double:

      case QMetaType::Float:
      case QMetaType::QObjectStar:
         break;
   }

   return d->is_null;
}

// internal
template<typename T>
inline bool compareNumericMetaType(const QVariant::Private *const a, const QVariant::Private *const b)
{
   return *static_cast<const T *>(a->data.shared->ptr) == *static_cast<const T *>(b->data.shared->ptr);
}

// internal
static bool compare(const QVariant::Private *a, const QVariant::Private *b)
{
   switch (a->type) {

      case QVariant::Char:
         return *v_cast<QChar>(a) == *v_cast<QChar>(b);

      case QVariant::Char32:
         // broom (wait)  return *v_cast<QChar32>(a) == *v_cast<QChar32>(b);
         return false;

      case QVariant::String:
         return *v_cast<QString>(a) == *v_cast<QString>(b);

      case QVariant::String8:
         return *v_cast<QString8>(a) == *v_cast<QString8>(b);

      case QVariant::String16:
         // broom (wait) return *v_cast<QString16>(a) == *v_cast<QString16>(b);
         return false;

      case QVariant::StringList:
         return *v_cast<QStringList>(a) == *v_cast<QStringList>(b);

     case QVariant::List:
         return *v_cast<QVariantList>(a) == *v_cast<QVariantList>(b);

      case QVariant::Map:
         return *v_cast<QVariantMap>(a) == *v_cast<QVariantMap>(b);

      case QVariant::MultiMap:
         return *v_cast<QVariantMultiMap>(a) == *v_cast<QVariantMultiMap>(b);

      case QVariant::Hash:
         return *v_cast<QVariantHash>(a) == *v_cast<QVariantHash>(b);

      case QVariant::MultiHash:
         return *v_cast<QVariantMultiHash>(a) == *v_cast<QVariantMultiHash>(b);

      case QVariant::Size:
         return *v_cast<QSize>(a) == *v_cast<QSize>(b);

      case QVariant::SizeF:
         return *v_cast<QSizeF>(a) == *v_cast<QSizeF>(b);

      case QVariant::Rect:
         return *v_cast<QRect>(a) == *v_cast<QRect>(b);

      case QVariant::Line:
         return *v_cast<QLine>(a) == *v_cast<QLine>(b);

      case QVariant::LineF:
         return *v_cast<QLineF>(a) == *v_cast<QLineF>(b);

      case QVariant::RectF:
         return *v_cast<QRectF>(a) == *v_cast<QRectF>(b);

      case QVariant::Point:
         return *v_cast<QPoint>(a) == *v_cast<QPoint>(b);

      case QVariant::PointF:
         return *v_cast<QPointF>(a) == *v_cast<QPointF>(b);

      case QVariant::Url:
         return *v_cast<QUrl>(a) == *v_cast<QUrl>(b);

      case QVariant::Locale:
         return *v_cast<QLocale>(a) == *v_cast<QLocale>(b);

#ifndef QT_NO_REGEXP
      case QVariant::RegExp:
         return *v_cast<QRegExp>(a) == *v_cast<QRegExp>(b);
#endif

      case QVariant::Int:
         return a->data.i == b->data.i;

      case QVariant::UInt:
         return a->data.u == b->data.u;

      case QVariant::LongLong:
         return a->data.ll == b->data.ll;

      case QVariant::ULongLong:
         return a->data.ull == b->data.ull;

      case QVariant::Bool:
         return a->data.b == b->data.b;

      case QVariant::Double:
         return a->data.d == b->data.d;

      case QMetaType::Float:
         return a->data.f == b->data.f;

      case QMetaType::QObjectStar:
         return a->data.o == b->data.o;

      case QVariant::Date:
         return *v_cast<QDate>(a) == *v_cast<QDate>(b);

      case QVariant::Time:
         return *v_cast<QTime>(a) == *v_cast<QTime>(b);

      case QVariant::DateTime:
         return *v_cast<QDateTime>(a) == *v_cast<QDateTime>(b);

      case QVariant::EasingCurve:
         return *v_cast<QEasingCurve>(a) == *v_cast<QEasingCurve>(b);

      case QVariant::Uuid:
         return *v_cast<QUuid>(a) == *v_cast<QUuid>(b);

      case QVariant::ModelIndex:
         return *v_cast<QModelIndex>(a) == *v_cast<QModelIndex>(b);

      case QVariant::ByteArray:
         return *v_cast<QByteArray>(a) == *v_cast<QByteArray>(b);

      case QVariant::BitArray:
         return *v_cast<QBitArray>(a) == *v_cast<QBitArray>(b);

      case QVariant::Invalid:
         return true;

      case QMetaType::Long:
         return compareNumericMetaType<long>(a, b);

      case QMetaType::ULong:
         return compareNumericMetaType<ulong>(a, b);

      case QMetaType::Short:
         return compareNumericMetaType<short>(a, b);

      case QMetaType::UShort:
         return compareNumericMetaType<ushort>(a, b);

      case QMetaType::UChar:
         return compareNumericMetaType<uchar>(a, b);

      case QMetaType::Char:
         return compareNumericMetaType<char>(a, b);

      default:
         break;
   }

   if (! QMetaType::isRegistered(a->type))   {
      qFatal("QVariant::compare: type %d unknown to QVariant.", a->type);
   }

   const void *a_ptr = a->is_shared ? a->data.shared->ptr : &(a->data.ptr);
   const void *b_ptr = b->is_shared ? b->data.shared->ptr : &(b->data.ptr);

   // ** unable to place this test in a case branch above, because user defined pointer type would not match
   const char *const typeName = QMetaType::typeName(a->type);
   uint typeNameLen = qstrlen(typeName);

   if (typeNameLen > 0 && typeName[typeNameLen - 1] == '*')  {
      return *static_cast<void *const *>(a_ptr) == *static_cast<void *const *>(b_ptr);
   }

   if (a->is_null && b->is_null)  {
      return true;
   }

   return a_ptr == b_ptr;
}

// internal
static qint64 qMetaTypeNumber(const QVariant::Private *d)
{
   switch (d->type) {
      case QMetaType::Int:
         return d->data.i;

      case QMetaType::LongLong:
         return d->data.ll;

      case QMetaType::Char:
         return qint64(*static_cast<signed char *>(d->data.shared->ptr));

      case QMetaType::Short:
         return qint64(*static_cast<short *>(d->data.shared->ptr));

      case QMetaType::Long:
         return qint64(*static_cast<long *>(d->data.shared->ptr));

      case QMetaType::Float:
         return qRound64(d->data.f);

      case QVariant::Double:
         return qRound64(d->data.d);
   }

   Q_ASSERT(false);
   return 0;
}

static quint64 qMetaTypeUNumber(const QVariant::Private *d)
{
   switch (d->type) {
      case QVariant::UInt:
         return d->data.u;

      case QVariant::ULongLong:
         return d->data.ull;

      case QMetaType::UChar:
         return quint64(*static_cast<unsigned char *>(d->data.shared->ptr));

      case QMetaType::UShort:
         return quint64(*static_cast<ushort *>(d->data.shared->ptr));

      case QMetaType::ULong:
         return quint64(*static_cast<ulong *>(d->data.shared->ptr));
   }

   Q_ASSERT(false);
   return 0;
}

static qint64 qConvertToNumber(const QVariant::Private *d, bool *ok)
{
   *ok = true;

   switch (uint(d->type)) {

      case QVariant::Char:
         return v_cast<QChar>(d)->unicode();

      case QVariant::Char32:
         // broom (wait)  return v_cast<QChar32>(d)->unicode();
         return 0;

      case QVariant::String:
         return v_cast<QString>(d)->toLongLong(ok);

      case QVariant::String8:
         // broom (wait) return v_cast<QString8>(d)->toLongLong(ok);
         return 0;

      case QVariant::String16:
         // broom (wait) return v_cast<QString16>(d)->toLongLong(ok);
         return 0;

      case QVariant::ByteArray:
         return v_cast<QByteArray>(d)->toLongLong(ok);

      case QVariant::Bool:
         return qint64(d->data.b);

      case QVariant::Double:
      case QVariant::Int:
      case QMetaType::Char:
      case QMetaType::Short:
      case QMetaType::Long:
      case QMetaType::Float:
      case QMetaType::LongLong:
         return qMetaTypeNumber(d);

      case QVariant::ULongLong:
      case QVariant::UInt:
      case QMetaType::UChar:
      case QMetaType::UShort:
      case QMetaType::ULong:
         return qint64(qMetaTypeUNumber(d));
   }

   *ok = false;
   return Q_INT64_C(0);
}

static quint64 qConvertToUnsignedNumber(const QVariant::Private *d, bool *ok)
{
   *ok = true;

   switch (uint(d->type)) {
      case QVariant::Char:
         return v_cast<QChar>(d)->unicode();

      case QVariant::Char32:
         // broom (wait) return v_cast<QChar32>(d)->unicode();
         return 0;

      case QVariant::String:
         return v_cast<QString>(d)->toULongLong(ok);

      case QVariant::String8:
         // broom (wait) return v_cast<QString8>(d)->toULongLong(ok);
         return 0;

      case QVariant::String16:
         // broom (wait) return v_cast<QString16>(d)->toULongLong(ok);
         return 0;

      case QVariant::ByteArray:
         return v_cast<QByteArray>(d)->toULongLong(ok);

      case QVariant::Bool:
         return quint64(d->data.b);

      case QVariant::Double:
      case QVariant::Int:
      case QMetaType::Char:
      case QMetaType::Short:
      case QMetaType::Long:
      case QMetaType::Float:
      case QMetaType::LongLong:
         return quint64(qMetaTypeNumber(d));

      case QVariant::ULongLong:
      case QVariant::UInt:
      case QMetaType::UChar:
      case QMetaType::UShort:
      case QMetaType::ULong:
         return qMetaTypeUNumber(d);
   }

   *ok = false;
   return Q_UINT64_C(0);
}

template<typename TInput, typename LiteralWrapper>
inline bool qt_convertToBool(const QVariant::Private *const d)
{
   TInput str = v_cast<TInput>(d)->toLower();
   return !(str == LiteralWrapper("0") || str == LiteralWrapper("false") || str.isEmpty());
}

// internal
static bool convert(const QVariant::Private *d, QVariant::Type t, void *result, bool *ok)
{
   Q_ASSERT(d->type != uint(t));
   Q_ASSERT(result);

   bool dummy;

   if (!ok)  {
      ok = &dummy;
   }

   switch (uint(t)) {

      case QVariant::Url:
         switch (d->type) {
            case QVariant::String:
               *static_cast<QUrl *>(result) = QUrl(*v_cast<QString>(d));
               break;
            default:
               return false;
         }

         break;

      case QVariant::String: {
         QString *str = static_cast<QString *>(result);

         switch (d->type) {
            case QVariant::Char:
               *str = QString(*v_cast<QChar>(d));
               break;

            case QMetaType::Char:
            case QMetaType::UChar:
               *str = QChar::fromLatin1(*static_cast<char *>(d->data.shared->ptr));
               break;

            case QMetaType::Short:
            case QMetaType::Long:
            case QVariant::Int:
            case QVariant::LongLong:
               *str = QString::number(qMetaTypeNumber(d));
               break;

            case QVariant::UInt:
            case QVariant::ULongLong:
            case QMetaType::UShort:
            case QMetaType::ULong:
               *str = QString::number(qMetaTypeUNumber(d));
               break;

            case QMetaType::Float:
               *str = QString::number(d->data.f, 'g', FLT_DIG);
               break;

            case QVariant::Double:
               *str = QString::number(d->data.d, 'g', DBL_DIG);
               break;

#if ! defined(QT_NO_DATESTRING)
            case QVariant::Date:
               *str = v_cast<QDate>(d)->toString(Qt::ISODate);
               break;

            case QVariant::Time:
               *str = v_cast<QTime>(d)->toString(Qt::ISODate);
               break;

            case QVariant::DateTime:
               *str = v_cast<QDateTime>(d)->toString(Qt::ISODate);
               break;
#endif

            case QVariant::Bool:
               *str = QLatin1String(d->data.b ? "true" : "false");
               break;

            case QVariant::ByteArray:
               *str = QString::fromLatin1(v_cast<QByteArray>(d)->constData());
               break;

            case QVariant::StringList:
               if (v_cast<QStringList>(d)->count() == 1) {
                  *str = v_cast<QStringList>(d)->at(0);
               }
               break;

            case QVariant::Url:
               *str = v_cast<QUrl>(d)->toString();
               break;

            case QVariant::Uuid:
               *str = v_cast<QUuid>(d)->toString();
               break;

            default:
               return false;
         }

         break;
      }

      case QVariant::Char: {
         QChar *c = static_cast<QChar *>(result);

         switch (d->type) {
            case QVariant::Int:
            case QVariant::LongLong:
            case QMetaType::Char:
            case QMetaType::Short:
            case QMetaType::Long:
            case QMetaType::Float:
               *c = QChar(ushort(qMetaTypeNumber(d)));
               break;

            case QVariant::UInt:
            case QVariant::ULongLong:
            case QMetaType::UChar:
            case QMetaType::UShort:
            case QMetaType::ULong:
               *c = QChar(ushort(qMetaTypeUNumber(d)));
               break;

            default:
               return false;
         }
         break;
      }

      case QVariant::Size: {
         QSize *s = static_cast<QSize *>(result);
         switch (d->type) {
            case QVariant::SizeF:
               *s = v_cast<QSizeF>(d)->toSize();
               break;
            default:
               return false;
         }
         break;
      }

      case QVariant::SizeF: {
         QSizeF *s = static_cast<QSizeF *>(result);
         switch (d->type) {
            case QVariant::Size:
               *s = QSizeF(*(v_cast<QSize>(d)));
               break;
            default:
               return false;
         }
         break;
      }

      case QVariant::Line: {
         QLine *s = static_cast<QLine *>(result);
         switch (d->type) {
            case QVariant::LineF:
               *s = v_cast<QLineF>(d)->toLine();
               break;
            default:
               return false;
         }
         break;
      }

      case QVariant::LineF: {
         QLineF *s = static_cast<QLineF *>(result);
         switch (d->type) {
            case QVariant::Line:
               *s = QLineF(*(v_cast<QLine>(d)));
               break;
            default:
               return false;
         }
         break;
      }

      case QVariant::StringList:
         if (d->type == QVariant::List) {
            QStringList *slst = static_cast<QStringList *>(result);
            const QVariantList *list = v_cast<QVariantList >(d);
            for (int i = 0; i < list->size(); ++i) {
               slst->append(list->at(i).toString());
            }

         } else if (d->type == QVariant::String) {
            QStringList *slst = static_cast<QStringList *>(result);
            *slst = QStringList(*v_cast<QString>(d));

         } else {
            return false;

         }

         break;

      case QVariant::Date: {
         QDate *dt = static_cast<QDate *>(result);

         if (d->type == QVariant::DateTime)  {
            *dt = v_cast<QDateTime>(d)->date();

#ifndef QT_NO_DATESTRING
         } else if (d->type == QVariant::String)  {
            *dt = QDate::fromString(*v_cast<QString>(d), Qt::ISODate);
#endif
         } else  {
            return false;

         }

         return dt->isValid();
      }

      case QVariant::Time: {
         QTime *t = static_cast<QTime *>(result);

         switch (d->type) {
            case QVariant::DateTime:
               *t = v_cast<QDateTime>(d)->time();
               break;

#ifndef QT_NO_DATESTRING
            case QVariant::String:
               *t = QTime::fromString(*v_cast<QString>(d), Qt::ISODate);
               break;
#endif
            default:
               return false;
         }

         return t->isValid();
      }

      case QVariant::DateTime: {
         QDateTime *dt = static_cast<QDateTime *>(result);

         switch (d->type) {

#ifndef QT_NO_DATESTRING
            case QVariant::String:
               *dt = QDateTime::fromString(*v_cast<QString>(d), Qt::ISODate);
               break;
#endif
            case QVariant::Date:
               *dt = QDateTime(*v_cast<QDate>(d));
               break;
            default:
               return false;
         }

         return dt->isValid();
      }

      case QVariant::ByteArray: {
         QByteArray *ba = static_cast<QByteArray *>(result);

         switch (d->type) {
            case QVariant::String:
               *ba = v_cast<QString>(d)->toLatin1();
               break;

            case QVariant::Double:
               *ba = QByteArray::number(d->data.d, 'g', DBL_DIG);
               break;

            case QMetaType::Float:
               *ba = QByteArray::number(d->data.f, 'g', FLT_DIG);
               break;

            case QMetaType::Char:
            case QMetaType::UChar:
               *ba = QByteArray(1, *static_cast<char *>(d->data.shared->ptr));
               break;

            case QVariant::Int:
            case QVariant::LongLong:
            case QMetaType::Short:
            case QMetaType::Long:
               *ba = QByteArray::number(qMetaTypeNumber(d));
               break;

            case QVariant::UInt:
            case QVariant::ULongLong:
            case QMetaType::UShort:
            case QMetaType::ULong:
               *ba = QByteArray::number(qMetaTypeUNumber(d));
               break;

            case QVariant::Bool:
               *ba = QByteArray(d->data.b ? "true" : "false");
               break;

            default:
               return false;
         }

      }
      break;

      case QMetaType::Short:
         *static_cast<short *>(result) = short(qConvertToNumber(d, ok));
         return *ok;

      case QMetaType::Long:
         *static_cast<long *>(result) = long(qConvertToNumber(d, ok));
         return *ok;

      case QMetaType::UShort:
         *static_cast<ushort *>(result) = ushort(qConvertToUnsignedNumber(d, ok));
         return *ok;

      case QMetaType::ULong:
         *static_cast<ulong *>(result) = ulong(qConvertToUnsignedNumber(d, ok));
         return *ok;

      case QVariant::Int:
         *static_cast<int *>(result) = int(qConvertToNumber(d, ok));
         return *ok;

      case QVariant::UInt:
         *static_cast<uint *>(result) = uint(qConvertToUnsignedNumber(d, ok));
         return *ok;

      case QVariant::LongLong:
         *static_cast<qint64 *>(result) = qConvertToNumber(d, ok);
         return *ok;

      case QVariant::ULongLong: {
         *static_cast<quint64 *>(result) = qConvertToUnsignedNumber(d, ok);
         return *ok;
      }

      case QMetaType::UChar: {
         *static_cast<uchar *>(result) = qConvertToUnsignedNumber(d, ok);
         return *ok;
      }

      case QVariant::Bool: {
         bool *b = static_cast<bool *>(result);

         switch (d->type) {
            case QVariant::ByteArray:
               *b = qt_convertToBool<QByteArray, QByteArray>(d);
               break;

            case QVariant::String:
               *b = qt_convertToBool<QString, QLatin1String>(d);
               break;

            case QVariant::Char:
               *b = !v_cast<QChar>(d)->isNull();
               break;

            case QVariant::Double:
            case QVariant::Int:
            case QVariant::LongLong:
            case QMetaType::Char:
            case QMetaType::Short:
            case QMetaType::Long:
            case QMetaType::Float:
               *b = qMetaTypeNumber(d) != Q_INT64_C(0);
               break;

            case QVariant::UInt:
            case QVariant::ULongLong:
            case QMetaType::UChar:
            case QMetaType::UShort:
            case QMetaType::ULong:
               *b = qMetaTypeUNumber(d) != Q_UINT64_C(0);
               break;

            default:
               *b = false;
               return false;
         }

         break;
      }

      case QVariant::Double: {
         double *f = static_cast<double *>(result);

         switch (d->type) {
            case QVariant::String:
               *f = v_cast<QString>(d)->toDouble(ok);
               break;

            case QVariant::ByteArray:
               *f = v_cast<QByteArray>(d)->toDouble(ok);
               break;

            case QVariant::Bool:
               *f = double(d->data.b);
               break;

            case QMetaType::Float:
               *f = double(d->data.f);
               break;

            case QVariant::LongLong:
            case QVariant::Int:
            case QMetaType::Char:
            case QMetaType::Short:
            case QMetaType::Long:
               *f = double(qMetaTypeNumber(d));
               break;

            case QVariant::UInt:
            case QVariant::ULongLong:
            case QMetaType::UChar:
            case QMetaType::UShort:
            case QMetaType::ULong:
               *f = double(qMetaTypeUNumber(d));
               break;

            default:
               *f = 0.0;
               return false;
         }
         break;
      }

      case QMetaType::Float: {
         float *f = static_cast<float *>(result);
         switch (d->type) {
            case QVariant::String:
               *f = v_cast<QString>(d)->toFloat(ok);
               break;

            case QVariant::ByteArray:
               *f = v_cast<QByteArray>(d)->toFloat(ok);
               break;

            case QVariant::Bool:
               *f = float(d->data.b);
               break;

            case QVariant::Double:
               *f = float(d->data.d);
               break;

            case QVariant::LongLong:
            case QVariant::Int:
            case QMetaType::Char:
            case QMetaType::Short:
            case QMetaType::Long:
               *f = float(qMetaTypeNumber(d));
               break;

            case QVariant::UInt:
            case QVariant::ULongLong:
            case QMetaType::UChar:
            case QMetaType::UShort:
            case QMetaType::ULong:
               *f = float(qMetaTypeUNumber(d));
               break;

            default:
               *f = 0.0f;
               return false;
         }
         break;
      }

      case QVariant::List:
         if (d->type == QVariant::StringList) {
            QVariantList *lst = static_cast<QVariantList *>(result);
            const QStringList *slist = v_cast<QStringList>(d);

            for (int i = 0; i < slist->size(); ++i) {
               lst->append(QVariant(slist->at(i)));
            }

         } else if (qstrcmp(QMetaType::typeName(d->type), "QList<QVariant>") == 0) {
            *static_cast<QVariantList *>(result) = *static_cast<QList<QVariant> *>(d->data.shared->ptr);

         } else {
            return false;
         }
         break;

      case QVariant::Map:
         if (qstrcmp(QMetaType::typeName(d->type), "QMap<QString, QVariant>") == 0) {
            *static_cast<QVariantMap *>(result) =
               *static_cast<QMap<QString, QVariant> *>(d->data.shared->ptr);
         } else {
            return false;
         }
         break;

      case QVariant::MultiMap:
         if (qstrcmp(QMetaType::typeName(d->type), "QMultiMap<QString, QVariant>") == 0) {
            *static_cast<QVariantMultiMap *>(result) =
               *static_cast<QMultiMap<QString, QVariant> *>(d->data.shared->ptr);
         } else {
            return false;
         }
         break;

      case QVariant::Hash:
         if (qstrcmp(QMetaType::typeName(d->type), "QHash<QString, QVariant>") == 0) {
            *static_cast<QVariantHash *>(result) =
               *static_cast<QHash<QString, QVariant> *>(d->data.shared->ptr);
         } else {
            return false;
         }
         break;

      case QVariant::MultiHash:
         if (qstrcmp(QMetaType::typeName(d->type), "QMultiHash<QString, QVariant>") == 0) {
            *static_cast<QVariantMultiHash *>(result) =
               *static_cast<QMultiHash<QString, QVariant> *>(d->data.shared->ptr);
         } else {
            return false;
         }
         break;

      case QVariant::Rect:
         if (d->type == QVariant::RectF) {
            *static_cast<QRect *>(result) = (v_cast<QRectF>(d))->toRect();
         } else {
            return false;
         }
         break;

      case QVariant::RectF:
         if (d->type == QVariant::Rect) {
            *static_cast<QRectF *>(result) = *v_cast<QRect>(d);
         } else {
            return false;
         }
         break;

      case QVariant::PointF:
         if (d->type == QVariant::Point) {
            *static_cast<QPointF *>(result) = *v_cast<QPoint>(d);
         } else {
            return false;
         }
         break;

      case QVariant::Point:
         if (d->type == QVariant::PointF) {
            *static_cast<QPoint *>(result) = (v_cast<QPointF>(d))->toPoint();
         } else {
            return false;
         }
         break;

      case QMetaType::Char: {
         *static_cast<qint8 *>(result) = qint8(qConvertToNumber(d, ok));
         return *ok;
      }

      default:
         return false;
   }
   return true;
}

static void streamDebug(QDebug dbg, const QVariant &v)
{
   switch (v.userType()) {
      case QVariant::Int:
         dbg.nospace() << v.toInt();
         break;
      case QVariant::UInt:
         dbg.nospace() << v.toUInt();
         break;
      case QVariant::LongLong:
         dbg.nospace() << v.toLongLong();
         break;
      case QVariant::ULongLong:
         dbg.nospace() << v.toULongLong();
         break;
      case QMetaType::Float:
         dbg.nospace() << v.toFloat();
         break;
      case QMetaType::QObjectStar:
         dbg.nospace() << qvariant_cast<QObject *>(v);
         break;
      case QVariant::Double:
         dbg.nospace() << v.toDouble();
         break;
      case QVariant::Bool:
         dbg.nospace() << v.toBool();
         break;
      case QVariant::String:
         dbg.nospace() << v.toString();
         break;
      case QVariant::Char:
         dbg.nospace() << v.toChar();
         break;
      case QVariant::StringList:
         dbg.nospace() << v.toStringList();
         break;

      case QVariant::Map:
         dbg.nospace() << v.toMap();
         break;

      case QVariant::MultiMap:
         dbg.nospace() << v.toMultiMap();
         break;

      case QVariant::Hash:
         dbg.nospace() << v.toHash();
         break;

      case QVariant::MultiHash:
         dbg.nospace() << v.toMultiHash();
         break;

      case QVariant::List:
         dbg.nospace() << v.toList();
         break;

      case QVariant::Date:
         dbg.nospace() << v.toDate();
         break;

      case QVariant::Time:
         dbg.nospace() << v.toTime();
         break;

      case QVariant::DateTime:
         dbg.nospace() << v.toDateTime();
         break;

      case QVariant::EasingCurve:
         dbg.nospace() << v.toEasingCurve();
         break;

      // case QVariant::Uuid:

      // case QVariant::ModelIndex:

      case QVariant::ByteArray:
         dbg.nospace() << v.toByteArray();
         break;

      case QVariant::Url:
         dbg.nospace() << v.toUrl();
         break;

      case QVariant::Point:
         dbg.nospace() << v.toPoint();
         break;

      case QVariant::PointF:
         dbg.nospace() << v.toPointF();
         break;

      case QVariant::Rect:
         dbg.nospace() << v.toRect();
         break;
      case QVariant::Size:
         dbg.nospace() << v.toSize();
         break;
      case QVariant::SizeF:
         dbg.nospace() << v.toSizeF();
         break;

      case QVariant::Line:
         dbg.nospace() << v.toLine();
         break;

      case QVariant::LineF:
         dbg.nospace() << v.toLineF();
         break;

      case QVariant::RectF:
         dbg.nospace() << v.toRectF();
         break;

      case QVariant::BitArray:
         //dbg.nospace() << v.toBitArray();
         break;

      default:
         break;
   }
}

const QVariant::Handler qt_kernel_variant_handler = {
   construct,
   clear,
   isNull,
#ifndef QT_NO_DATASTREAM
   0,
   0,
#endif
   compare,
   convert,
   0,
   streamDebug

};

Q_CORE_EXPORT const QVariant::Handler *qcoreVariantHandler()
{
   return &qt_kernel_variant_handler;
}

const QVariant::Handler *QVariant::handler = &qt_kernel_variant_handler;

void QVariant::create(int type, const void *copy)
{
   d.type = type;
   handler->construct(&d, copy);
}

bool QVariant::clearRequired() const
{
   if (! d.is_shared) {

      if (d.type >= UserType) {
         return false;
      }

      switch (d.type) {
         case Invalid:
         case Bool:
         case Int:
         case UInt:
         case LongLong:
         case ULongLong:
         case Double:
         case Char:
         case Char32:
            return false;
      }

      return true;
   }

   return false;
}

QVariant::~QVariant()
{
   if (d.is_ptr) {
      // create was not called so no construtor was called
      return;
   }

   if ((d.is_shared && ! d.data.shared->ref.deref()) || clearRequired()) {
      handler->clear(&d);
   }
}

QVariant::QVariant(const QVariant &p)
   : d(p.d)
{
   if (d.is_shared) {
      d.data.shared->ref.ref();

   } else if (p.d.type > Char && p.d.type < QVariant::UserType) {
      handler->construct(&d, p.constData());
      d.is_null = p.d.is_null;
   }
}

#ifndef QT_NO_DATASTREAM

QVariant::QVariant(QDataStream &s)
{
   d.is_null = true;
   s >> *this;
}
#endif

QVariant::QVariant(Type type)
{
   create(type, 0);
}

QVariant::QVariant(int typeOrUserType, const void *copy)
{
   create(typeOrUserType, copy);
   d.is_null = false;
}

// internal
QVariant::QVariant(int typeOrUserType, const void *copy, uint flags)
{
   if (flags) {
      // type is a pointer type
      d.is_ptr   = true;
      d.type     = typeOrUserType;

      d.data.ptr = *reinterpret_cast<void *const *>(copy);

   } else {
      create(typeOrUserType, copy);
   }

   d.is_null = false;
}

QVariant::QVariant(int val)
{
   d.is_null = false;
   d.type    = Int;
   d.data.i  = val;
}
QVariant::QVariant(uint val)
{
   d.is_null = false;
   d.type = UInt;
   d.data.u = val;
}
QVariant::QVariant(qint64 val)
{
   d.is_null = false;
   d.type = LongLong;
   d.data.ll = val;
}
QVariant::QVariant(quint64 val)
{
   d.is_null = false;
   d.type = ULongLong;
   d.data.ull = val;
}
QVariant::QVariant(bool val)
{
   d.is_null = false;
   d.type = Bool;
   d.data.b = val;
}
QVariant::QVariant(double val)
{
   d.is_null = false;
   d.type = Double;
   d.data.d = val;
}

QVariant::QVariant(const QByteArray &val)
{
   d.is_null = false;
   d.type = ByteArray;
   v_construct<QByteArray>(&d, val);
}
QVariant::QVariant(const QBitArray &val)
{
   d.is_null = false;
   d.type = BitArray;
   v_construct<QBitArray>(&d, val);
}
QVariant::QVariant(const QString &val)
{
   d.is_null = false;
   d.type = String;
   v_construct<QString>(&d, val);
}
QVariant::QVariant(const QChar &val)
{
   d.is_null = false;
   d.type = Char;
   v_construct<QChar>(&d, val);
}
QVariant::QVariant(const QLatin1String &val)
{
   QString str(val);
   d.is_null = false;
   d.type = String;
   v_construct<QString>(&d, str);
}
QVariant::QVariant(const QStringList &val)
{
   d.is_null = false;
   d.type = StringList;
   v_construct<QStringList>(&d, val);
}

QVariant::QVariant(const QDate &val)
{
   d.is_null = false;
   d.type = Date;
   v_construct<QDate>(&d, val);
}
QVariant::QVariant(const QTime &val)
{
   d.is_null = false;
   d.type = Time;
   v_construct<QTime>(&d, val);
}
QVariant::QVariant(const QDateTime &val)
{
   d.is_null = false;
   d.type = DateTime;
   v_construct<QDateTime>(&d, val);
}

QVariant::QVariant(const QEasingCurve &val)
{
   d.is_null = false;
   d.type = EasingCurve;
   v_construct<QEasingCurve>(&d, val);
}

QVariant::QVariant(const QList<QVariant> &list)
{
   d.is_null = false;
   d.type = List;
   v_construct<QVariantList>(&d, list);
}

QVariant::QVariant(const QMap<QString, QVariant> &map)
{
   d.is_null = false;
   d.type = Map;
   v_construct<QVariantMap>(&d, map);
}

QVariant::QVariant(const QMultiMap<QString, QVariant> &map)
{
   d.is_null = false;
   d.type = MultiMap;
   v_construct<QVariantMultiMap>(&d, map);
}

QVariant::QVariant(const QHash<QString, QVariant> &hash)
{
   d.is_null = false;
   d.type = Hash;
   v_construct<QVariantHash>(&d, hash);
}

QVariant::QVariant(const QMultiHash<QString, QVariant> &hash)
{
   d.is_null = false;
   d.type = MultiHash;
   v_construct<QVariantMultiHash>(&d, hash);
}

QVariant::QVariant(const QPoint &pt)
{
   d.is_null = false;
   d.type = Point;
   v_construct<QPoint>(&d, pt);
}
QVariant::QVariant(const QPointF &pt)
{
   d.is_null = false;
   d.type = PointF;
   v_construct<QPointF>(&d, pt);
}

QVariant::QVariant(const QRect &r)
{
   d.is_null = false;
   d.type = Rect;
   v_construct<QRect>(&d, r);
}
QVariant::QVariant(const QRectF &r)
{
   d.is_null = false;
   d.type = RectF;
   v_construct<QRectF>(&d, r);
}

QVariant::QVariant(const QLine &l)
{
   d.is_null = false;
   d.type = Line;
   v_construct<QLine>(&d, l);
}
QVariant::QVariant(const QLineF &l)
{
   d.is_null = false;
   d.type = LineF;
   v_construct<QLineF>(&d, l);
}

QVariant::QVariant(const QSize &s)
{
   d.is_null = false;
   d.type = Size;
   v_construct<QSize>(&d, s);
}
QVariant::QVariant(const QSizeF &s)
{
   d.is_null = false;
   d.type = SizeF;
   v_construct<QSizeF>(&d, s);
}

QVariant::QVariant(const QUrl &u)
{
   d.is_null = false;
   d.type = Url;
   v_construct<QUrl>(&d, u);
}
QVariant::QVariant(const QLocale &l)
{
   d.is_null = false;
   d.type = Locale;
   v_construct<QLocale>(&d, l);
}

#ifndef QT_NO_REGEXP
QVariant::QVariant(const QRegExp &regExp)
{
   d.is_null = false;
   d.type = RegExp;
   v_construct<QRegExp>(&d, regExp);
}
#endif

QVariant::QVariant(Qt::GlobalColor color)
{
   create(62, &color);
}

QVariant::Type QVariant::type() const
{
   return d.type >= QMetaType::User ? UserType : static_cast<Type>(d.type);
}

int QVariant::userType() const
{
   return d.type;
}

QVariant &QVariant::operator=(const QVariant &variant)
{
   if (this == &variant) {
      return *this;
   }

   clear();
   if (variant.d.is_shared) {
      variant.d.data.shared->ref.ref();
      d = variant.d;

   } else if (variant.d.type > Char && variant.d.type < UserType) {
      d.type = variant.d.type;
      handler->construct(&d, variant.constData());
      d.is_null = variant.d.is_null;

   } else {
      d = variant.d;
   }

   return *this;
}

// internal
void QVariant::detach()
{
   if (! d.is_shared || d.data.shared->ref.load() == 1)  {
      return;
   }

   Private dd;
   dd.type = d.type;
   handler->construct(&dd, constData());

   if (!d.data.shared->ref.deref()) {
      handler->clear(&d);
   }

   d.data.shared = dd.data.shared;
}

const char *QVariant::typeName() const
{
   return typeToName(Type(d.type));
}

void QVariant::clear()
{
   if ((d.is_shared && !d.data.shared->ref.deref()) || (!d.is_shared && d.type < UserType && d.type > Char)) {
      handler->clear(&d);
   }

   d.type       = Invalid;
   d.is_null    = true;
   d.is_shared  = false;
}

const char *QVariant::typeToName(Type enumData)
{
   if (enumData == Invalid) {
      return 0;
   }

   if (enumData == UserType) {
      return "UserType";
   }

   return QMetaType::typeName(enumData);
}

QVariant::Type QVariant::nameToType(const char *name)
{
   if (! name || ! *name)  {
      return Invalid;
   }

   if (strcmp(name, "UserType") == 0)  {
      return UserType;
   }

   int metaType = QMetaType::type(name);

   if (metaType < int(UserType))  {
      metaType = QVariant::Type(metaType);

   } else {
      metaType = UserType;

   }

   return Type(metaType);
}

#ifndef QT_NO_DATASTREAM
enum { MapFromThreeCount = 36 };
static const ushort map_from_three[MapFromThreeCount] = {
   QVariant::Invalid,
   QVariant::Map,
   QVariant::List,
   QVariant::String,
   QVariant::StringList,
   QVariant::Font,
   QVariant::Pixmap,
   QVariant::Brush,
   QVariant::Rect,
   QVariant::Size,
   QVariant::Color,
   QVariant::Palette,
   63, // ColorGroup
   QVariant::Icon,
   QVariant::Point,
   QVariant::Image,
   QVariant::Int,
   QVariant::UInt,
   QVariant::Bool,
   QVariant::Double,
   QVariant::ByteArray,
   QVariant::Polygon,
   QVariant::Region,
   QVariant::Bitmap,
   QVariant::Cursor,
   QVariant::SizePolicy,
   QVariant::Date,
   QVariant::Time,
   QVariant::DateTime,
   QVariant::ByteArray,
   QVariant::BitArray,
   QVariant::KeySequence,
   QVariant::Pen,
   QVariant::LongLong,
   QVariant::ULongLong,
   QVariant::EasingCurve
};

// internal
void QVariant::load(QDataStream &s)
{
   clear();

   quint32 u;
   s >> u;

   qint8 is_null = false;
   if (s.version() >= QDataStream::Qt_4_2) {
      s >> is_null;
   }

   if (u == QVariant::UserType) {
      QByteArray name;
      s >> name;
      u = QMetaType::type(name.constData());

      if (!u) {
         s.setStatus(QDataStream::ReadCorruptData);
         return;
      }
   }
   create(static_cast<int>(u), 0);
   d.is_null = is_null;

   if (!isValid()) {
      // Since we wrote something, we should read something
      QString x;
      s >> x;
      d.is_null = true;
      return;
   }

   // const cast is safe since we operate on a newly constructed variant
   if (! QMetaType::load(s, d.type, const_cast<void *>(constData()))) {
      s.setStatus(QDataStream::ReadCorruptData);
      qWarning("QVariant::load(): Unable to load type %d.", d.type);
   }
}

// internal
void QVariant::save(QDataStream &s) const
{
   quint32 tp = type();
   if (s.version() < QDataStream::Qt_4_0) {
      int i;
      for (i = MapFromThreeCount - 1; i >= 0; i--) {
         if (map_from_three[i] == tp) {
            tp = i;
            break;
         }
      }
      if (i == -1) {
         s << QVariant();
         return;
      }
   }
   s << tp;
   if (s.version() >= QDataStream::Qt_4_2) {
      s << qint8(d.is_null);
   }
   if (tp == QVariant::UserType) {
      s << QMetaType::typeName(userType());
   }

   if (!isValid()) {
      s << QString();
      return;
   }

   if (!QMetaType::save(s, d.type, constData())) {
      Q_ASSERT_X(false, "QVariant::save", "Invalid type to save");
      qWarning("QVariant::save: unable to save type %d.", d.type);
   }
}

QDataStream &operator>>(QDataStream &s, QVariant &p)
{
   p.load(s);
   return s;
}

QDataStream &operator<<(QDataStream &s, const QVariant &p)
{
   p.save(s);
   return s;
}

QDataStream &operator>>(QDataStream &s, QVariant::Type &p)
{
   quint32 u;
   s >> u;
   p = (QVariant::Type)u;

   return s;
}

QDataStream &operator<<(QDataStream &s, const QVariant::Type p)
{
   s << static_cast<quint32>(p);

   return s;
}

#endif //QT_NO_DATASTREAM


template <typename T>
inline T qVariantToHelper(const QVariant::Private &d, QVariant::Type t,
                          const QVariant::Handler *handler, T * = 0)
{
   if (d.type == t) {
      return *v_cast<T>(&d);
   }

   T ret;
   handler->convert(&d, t, &ret, 0);
   return ret;
}

QStringList QVariant::toStringList() const
{
   return qVariantToHelper<QStringList>(d, StringList, handler);
}

QString QVariant::toString() const
{
   return qVariantToHelper<QString>(d, String, handler);
}

// containers
QVariantMap QVariant::toMap() const
{
   return qVariantToHelper<QVariantMap>(d, Map, handler);
}

QVariantMultiMap QVariant::toMultiMap() const
{
   return qVariantToHelper<QVariantMultiMap>(d, MultiMap, handler);
}

QVariantHash QVariant::toHash() const
{
   return qVariantToHelper<QVariantHash>(d, Hash, handler);
}

QVariantMultiHash QVariant::toMultiHash() const
{
   return qVariantToHelper<QVariantMultiHash>(d, MultiHash, handler);
}

//
QDate QVariant::toDate() const
{
   return qVariantToHelper<QDate>(d, Date, handler);
}

QTime QVariant::toTime() const
{
   return qVariantToHelper<QTime>(d, Time, handler);
}

QDateTime QVariant::toDateTime() const
{
   return qVariantToHelper<QDateTime>(d, DateTime, handler);
}

QEasingCurve QVariant::toEasingCurve() const
{
   return qVariantToHelper<QEasingCurve>(d, EasingCurve, handler);
}

QByteArray QVariant::toByteArray() const
{
   return qVariantToHelper<QByteArray>(d, ByteArray, handler);
}

QPoint QVariant::toPoint() const
{
   return qVariantToHelper<QPoint>(d, Point, handler);
}

QRect QVariant::toRect() const
{
   return qVariantToHelper<QRect>(d, Rect, handler);
}

QSize QVariant::toSize() const
{
   return qVariantToHelper<QSize>(d, Size, handler);
}

QSizeF QVariant::toSizeF() const
{
   return qVariantToHelper<QSizeF>(d, SizeF, handler);
}

QRectF QVariant::toRectF() const
{
   return qVariantToHelper<QRectF>(d, RectF, handler);
}

QLineF QVariant::toLineF() const
{
   return qVariantToHelper<QLineF>(d, LineF, handler);
}

QLine QVariant::toLine() const
{
   return qVariantToHelper<QLine>(d, Line, handler);
}

QPointF QVariant::toPointF() const
{
   return qVariantToHelper<QPointF>(d, PointF, handler);
}

QUrl QVariant::toUrl() const
{
   return qVariantToHelper<QUrl>(d, Url, handler);
}

QLocale QVariant::toLocale() const
{
   return qVariantToHelper<QLocale>(d, Locale, handler);
}

#ifndef QT_NO_REGEXP
QRegExp QVariant::toRegExp() const
{
   return qVariantToHelper<QRegExp>(d, RegExp, handler);
}
#endif

QChar QVariant::toChar() const
{
   return qVariantToHelper<QChar>(d, Char, handler);
}

QBitArray QVariant::toBitArray() const
{
   return qVariantToHelper<QBitArray>(d, BitArray, handler);
}

template <typename T>
inline T qNumVariantToHelper(const QVariant::Private &d,
                             const QVariant::Handler *handler, bool *ok, const T &val)
{
   uint t = qMetaTypeId<T>();
   if (ok) {
      *ok = true;
   }
   if (d.type == t) {
      return val;
   }

   T ret;
   if (!handler->convert(&d, QVariant::Type(t), &ret, ok) && ok) {
      *ok = false;
   }
   return ret;
}

int QVariant::toInt(bool *ok) const
{
   return qNumVariantToHelper<int>(d, handler, ok, d.data.i);
}

uint QVariant::toUInt(bool *ok) const
{
   return qNumVariantToHelper<uint>(d, handler, ok, d.data.u);
}

qint64 QVariant::toLongLong(bool *ok) const
{
   return qNumVariantToHelper<qint64>(d, handler, ok, d.data.ll);
}

quint64 QVariant::toULongLong(bool *ok) const
{
   return qNumVariantToHelper<quint64>(d, handler, ok, d.data.ull);
}

bool QVariant::toBool() const
{
   if (d.type == Bool) {
      return d.data.b;
   }

   bool res = false;
   handler->convert(&d, Bool, &res, 0);

   return res;
}

double QVariant::toDouble(bool *ok) const
{
   return qNumVariantToHelper<double>(d, handler, ok, d.data.d);
}

float QVariant::toFloat(bool *ok) const
{
   return qNumVariantToHelper<float>(d, handler, ok, d.data.f);
}

qreal QVariant::toReal(bool *ok) const
{
   return qNumVariantToHelper<qreal>(d, handler, ok, d.data.real);
}

QVariantList QVariant::toList() const
{
   return qVariantToHelper<QVariantList>(d, List, handler);
}

static const quint32 qCanConvertMatrix[QMetaType::User + 1] = {
   /*UnknownType*/   0,

   /*Bool*/          1 << QVariant::Double     | 1 << QVariant::Int        | 1 << QVariant::UInt
   | 1 << QVariant::LongLong   | 1 << QVariant::ULongLong  | 1 << QVariant::ByteArray
   | 1 << QVariant::String     | 1 << QVariant::Char,

   /*Int*/           1 << QVariant::UInt       | 1 << QVariant::String     | 1 << QVariant::Double
   | 1 << QVariant::Bool       | 1 << QVariant::LongLong   | 1 << QVariant::ULongLong
   | 1 << QVariant::Char       | 1 << QVariant::ByteArray,

   /*UInt*/          1 << QVariant::Int        | 1 << QVariant::String     | 1 << QVariant::Double
   | 1 << QVariant::Bool       | 1 << QVariant::LongLong   | 1 << QVariant::ULongLong
   | 1 << QVariant::Char       | 1 << QVariant::ByteArray,

   /*LLong*/         1 << QVariant::Int        | 1 << QVariant::String     | 1 << QVariant::Double
   | 1 << QVariant::Bool       | 1 << QVariant::UInt       | 1 << QVariant::ULongLong
   | 1 << QVariant::Char       | 1 << QVariant::ByteArray,

   /*ULlong*/        1 << QVariant::Int        | 1 << QVariant::String     | 1 << QVariant::Double
   | 1 << QVariant::Bool       | 1 << QVariant::UInt       | 1 << QVariant::LongLong
   | 1 << QVariant::Char       | 1 << QVariant::ByteArray,

   /*double*/        1 << QVariant::Int        | 1 << QVariant::String     | 1 << QVariant::ULongLong
   | 1 << QVariant::Bool       | 1 << QVariant::UInt       | 1 << QVariant::LongLong
   | 1 << QVariant::ByteArray,

   /*QChar*/         1 << QVariant::Int        | 1 << QVariant::UInt       | 1 << QVariant::LongLong
   | 1 << QVariant::ULongLong,

   /*QString*/       1 << QVariant::StringList | 1 << QVariant::ByteArray  | 1 << QVariant::Int
   | 1 << QVariant::UInt       | 1 << QVariant::Bool       | 1 << QVariant::Double
   | 1 << QVariant::Date       | 1 << QVariant::Time       | 1 << QVariant::DateTime
   | 1 << QVariant::LongLong   | 1 << QVariant::ULongLong  | 1 << QVariant::Char
   | 1 << QVariant::Url,

   /*QStringList*/   1 << QVariant::List       | 1 << QVariant::String,

   /*QByteArray*/    1 << QVariant::String     | 1 << QVariant::Int        | 1 << QVariant::UInt | 1 << QVariant::Bool
   | 1 << QVariant::Double     | 1 << QVariant::LongLong   | 1 << QVariant::ULongLong,

   /*QBitArray*/     0,


   /*QChar32*/       1 << QVariant::Int        | 1 << QVariant::UInt       | 1 << QVariant::LongLong
   | 1 << QVariant::ULongLong,

   /*QString8*/      1 << QVariant::StringList | 1 << QVariant::ByteArray  | 1 << QVariant::Int
   | 1 << QVariant::UInt       | 1 << QVariant::Bool       | 1 << QVariant::Double
   | 1 << QVariant::Date       | 1 << QVariant::Time       | 1 << QVariant::DateTime
   | 1 << QVariant::LongLong   | 1 << QVariant::ULongLong  | 1 << QVariant::Char
   | 1 << QVariant::Url,

   /*QString16*/     1 << QVariant::StringList | 1 << QVariant::ByteArray  | 1 << QVariant::Int
   | 1 << QVariant::UInt       | 1 << QVariant::Bool       | 1 << QVariant::Double
   | 1 << QVariant::Date       | 1 << QVariant::Time       | 1 << QVariant::DateTime
   | 1 << QVariant::LongLong   | 1 << QVariant::ULongLong  | 1 << QVariant::Char
   | 1 << QVariant::Url,

   /*QList*/         1 << QVariant::StringList,

   /*QHash*/         0,

   /*QMultiHash*/    0,

   /*QMap*/          0,

   /*QMultiMap*/     0,


   /*QDate*/         1 << QVariant::String     | 1 << QVariant::DateTime,

   /*QTime*/         1 << QVariant::String     | 1 << QVariant::DateTime,

   /*QDateTime*/     1 << QVariant::String     | 1 << QVariant::Date,

   /*QUrl*/          1 << QVariant::String,

   /*QLocale*/       0,

   /*QRect*/         1 << QVariant::RectF,

   /*QRectF*/        1 << QVariant::Rect,

   /*QSize*/         1 << QVariant::SizeF,

   /*QSizeF*/        1 << QVariant::Size,

   /*QLine*/         1 << QVariant::LineF,

   /*QLineF*/        1 << QVariant::Line,

   /*QPoint*/        0,

   /*QPointF*/       0,

   /*QRegExp*/       0,

   /*QEasingCurve*/  0,

   /*QUuid*/         1 << QVariant::String
};

bool QVariant::canConvert(Type t) const
{
   // treat floats as double, QMetaType::Float's value is 135 which is not handled by qCanConvertMatrix
   // QVariant::Type does not have a float value, so we use QMetaType::Float

   uint currentType = d.type;

   if (currentType == QMetaType::Float) {
      currentType = QVariant::Double;
   }

   if (uint(t) == uint(QMetaType::Float)) {
      t = QVariant::Double;
   }

   // same type of issue as above ( will be resolved a diffferent way in a future version )
   if (currentType == QVariant::Point && uint(t) == uint(QVariant::PointF)) {
      return true;
   }

   // same type of issue as above ( will be resolved a diffferent way in a future version )
   if (currentType == QVariant::PointF && uint(t) == uint(QVariant::Point)) {
      return true;
   }

   // **
   if (currentType == uint(t)) {
      // type did not change
      return true;

   } else if (t == String && currentType == StringList) {
      return v_cast<QStringList>(&d)->count() == 1;

   } else if (qCanConvertMatrix[t] & (1 << currentType)) {
      // found a match in the matrix
      return true;

   }

   switch (t) {

      case QVariant::Int:
         return currentType == QVariant::KeySequence || currentType == QMetaType::ULong   ||
                currentType == QMetaType::Long  || currentType == QMetaType::UShort  ||
                currentType == QMetaType::UChar || currentType == QMetaType::Char || currentType == QMetaType::Short;

      case QVariant::Image:
         return currentType == QVariant::Pixmap || currentType == QVariant::Bitmap;

      case QVariant::Pixmap:
         return currentType == QVariant::Image || currentType == QVariant::Bitmap || currentType == QVariant::Brush;

      case QVariant::Bitmap:
         return currentType == QVariant::Pixmap || currentType == QVariant::Image;

      case QVariant::ByteArray:
         return currentType == QVariant::Color;

      case QVariant::String:
         return currentType == QVariant::KeySequence || currentType == QVariant::Font || currentType == QVariant::Color;

      case QVariant::KeySequence:
         return currentType == QVariant::String || currentType == QVariant::Int;

      case QVariant::Font:
         return currentType == QVariant::String;

      case QVariant::Color:
         return currentType == QVariant::String || currentType == QVariant::ByteArray || currentType == QVariant::Brush;

      case QVariant::Brush:
         return currentType == QVariant::Color || currentType == QVariant::Pixmap;

      case QMetaType::Long:
      case QMetaType::Char:
      case QMetaType::UChar:
      case QMetaType::ULong:
      case QMetaType::Short:
      case QMetaType::UShort:
         return qCanConvertMatrix[QVariant::Int] & (1 << currentType) || currentType == QVariant::Int;

      default:
         break;
   }

   return false;
}

bool QVariant::convert(Type t)
{
   if (d.type == uint(t)) {
      return true;
   }

   QVariant oldValue = *this;

   clear();
   if (!oldValue.canConvert(t)) {
      return false;
   }

   create(t, 0);
   if (oldValue.isNull()) {
      return false;
   }

   bool isOk = true;
   if (!handler->convert(&oldValue.d, t, data(), &isOk)) {
      isOk = false;
   }
   d.is_null = !isOk;
   return isOk;
}

static bool qIsNumericType(uint tp)
{
   return (tp >= QVariant::Bool && tp <= QVariant::Double)
          || (tp >= QMetaType::Long && tp <= QMetaType::Float);
}

static bool qIsFloatingPoint(uint tp)
{
   return tp == QVariant::Double || tp == QMetaType::Float;
}

// internal
bool QVariant::cmp(const QVariant &v) const
{
   QVariant v2 = v;
   if (d.type != v2.d.type) {
      if (qIsNumericType(d.type) && qIsNumericType(v.d.type)) {
         if (qIsFloatingPoint(d.type) || qIsFloatingPoint(v.d.type)) {
            return qFuzzyCompare(toReal(), v.toReal());
         } else {
            return toLongLong() == v.toLongLong();
         }
      }
      if (!v2.canConvert(Type(d.type)) || !v2.convert(Type(d.type))) {
         return false;
      }
   }
   return handler->compare(&d, &v2.d);
}

// internal
const void *QVariant::constData() const
{
   return d.is_shared ? d.data.shared->ptr : reinterpret_cast<const void *>(&d.data.ptr);
}

// internal
void *QVariant::data()
{
   detach();
   return const_cast<void *>(constData());
}

bool QVariant::isNull() const
{
   return handler->isNull(&d);
}

QDebug operator<<(QDebug dbg, const QVariant &v)
{
   dbg.nospace() << "QVariant(" << v.typeName() << ", ";
   QVariant::handler->debugStream(dbg, v);
   dbg.nospace() << ')';
   return dbg.space();
}

QDebug operator<<(QDebug dbg, const QVariant::Type p)
{
   dbg.nospace() << "QVariant::" << QVariant::typeToName(p);
   return dbg.space();
}

QT_END_NAMESPACE
