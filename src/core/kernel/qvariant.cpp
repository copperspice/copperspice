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

#include <qvariant.h>

#include <qabstractitemmodel.h>
#include <qbitarray.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qdatetime.h>
#include <qeasingcurve.h>
#include <qstring8.h>
#include <qstring16.h>
#include <qstringlist.h>
#include <qlocale.h>
#include <qjsonvalue.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qpoint.h>
#include <qsize.h>
#include <qrect.h>
#include <qline.h>
#include <quuid.h>
#include <qurl.h>

#include <limits>

QVector<QVariant::NamesAndTypes> QVariant::m_userTypes;

#ifndef DBL_DIG
#  define DBL_DIG 10
#endif

#ifndef FLT_DIG
#  define FLT_DIG 6
#endif

std::atomic<uint> &QVariant::currentUserType()
{
   static std::atomic<uint> retval = QVariant::UserType;
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

   { "QByteArray",             QVariant::ByteArray,            typeid(QByteArray *) },
   { "QBitArray",              QVariant::BitArray,             typeid(QBitArray *) },
   { "QString",                QVariant::String,               typeid(QString *) },
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

   { "QVariant",               QVariant::Variant,              typeid(QVariant *) },
   { "QVariantList",           QVariant::List,                 typeid(QVariantList *) },
   { "QVariantMap",            QVariant::Map,                  typeid(QVariantMap *) },
   { "QVariantMultiMap",       QVariant::MultiMap,             typeid(QVariantMultiMap *) },
   { "QVariantHash",           QVariant::Hash,                 typeid(QVariantHash *) },
   { "QVariantMultiHash",      QVariant::MultiHash,            typeid(QVariantMultiHash *) },

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
   { "QMatrix",                QVariant::Matrix,               typeid(QMatrix *)} ,
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
};


// constructors
QVariant::QVariant(const QVariant &other)
   : m_data(other.m_data)
{
}

QVariant::QVariant(QDataStream &s)
{
   s >> *this;
}

QVariant::QVariant(QVariant::Type type)
{
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

QVariant::QVariant(QMap<QString, QVariant> value)
{
   m_data = std::make_shared<CustomType_T<QMap<QString, QVariant>>>(std::move(value));
}

QVariant::QVariant(QMultiMap<QString, QVariant> value)
{
   m_data = std::make_shared<CustomType_T<QMultiMap<QString, QVariant>>>(std::move(value));;
}

QVariant::QVariant(QHash<QString, QVariant> value)
{
   m_data = std::make_shared<CustomType_T<QHash<QString, QVariant>>>(std::move(value));
}

QVariant::QVariant(QMultiHash<QString, QVariant> value)
{
   m_data = std::make_shared<CustomType_T<QMultiHash<QString, QVariant>>>(std::move(value));
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

QVariant::QVariant(QLine value)
{
   m_data = std::make_shared<CustomType_T<QLine>>(std::move(value));
}

QVariant::QVariant(QLineF value)
{
   m_data = std::make_shared<CustomType_T<QLineF>>(std::move(value));
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

QVariant::~QVariant()
{
}
