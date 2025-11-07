/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qbitarray.h>
#include <qbytearray.h>
#include <qdatetime.h>
#include <qeasingcurve.h>
#include <qhash.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qlocale.h>
#include <qlist.h>
#include <qline.h>
#include <qmap.h>
#include <qmultihash.h>
#include <qmultimap.h>
#include <qpersistentmodelindex.h>
#include <qpoint.h>
#include <qrect.h>
#include <qstringlist.h>
#include <qsize.h>
#include <qtimezone.h>
#include <qurl.h>
#include <quuid.h>
#include <qvariant.h>

#include <cs_catch2.h>

TEST_CASE("QVariant traits", "[qvariant]")
{
   REQUIRE(std::is_copy_constructible_v<QVariant> == true);
   REQUIRE(std::is_move_constructible_v<QVariant> == true);

   REQUIRE(std::is_copy_assignable_v<QVariant> == true);
   REQUIRE(std::is_move_assignable_v<QVariant> == true);

   REQUIRE(std::has_virtual_destructor_v<QVariant> == false);
}

TEST_CASE("QVariant can_convert_t_bytearray", "[qvariant]")
{
   QByteArray value(11, ' ');

   value[0]  = 'C';
   value[1]  = 'o';
   value[2]  = 'p';
   value[3]  = 'p';
   value[4]  = 'e';
   value[5]  = 'r';
   value[6]  = 'S';
   value[7]  = 'p';
   value[8]  = 'i';
   value[9]  = 'c';
   value[10] = 'e';
   value[11] = '\0';

   QVariant data = value;

   REQUIRE(data.type() == QVariant::ByteArray);

   REQUIRE(data.canConvert<bool>() == true);
   REQUIRE(data.canConvert<short>() == true);
   REQUIRE(data.canConvert<ushort>() == true);
   REQUIRE(data.canConvert<int>() == true);
   REQUIRE(data.canConvert<uint>() == true);
   REQUIRE(data.canConvert<long>() == true);
   REQUIRE(data.canConvert<ulong>() == true);
   REQUIRE(data.canConvert<long long>() == true);
   REQUIRE(data.canConvert<unsigned long long>() == true);
   REQUIRE(data.canConvert<double>() == true);
   REQUIRE(data.canConvert<float>() == true);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == true);
   REQUIRE(data.canConvert<QChar>() == true);
   REQUIRE(data.canConvert<QString8>() == true);
   REQUIRE(data.canConvert<QString16>() == true);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_char", "[qvariant]")
{
   QVariant data = QChar('B');

   REQUIRE(data.type() == QVariant::QChar);

   REQUIRE(data.canConvert<bool>() == true);
   REQUIRE(data.canConvert<short>() == true);
   REQUIRE(data.canConvert<ushort>() == true);
   REQUIRE(data.canConvert<int>() == true);
   REQUIRE(data.canConvert<uint>() == true);
   REQUIRE(data.canConvert<long>() == true);
   REQUIRE(data.canConvert<ulong>() == true);
   REQUIRE(data.canConvert<long long>() == true);
   REQUIRE(data.canConvert<unsigned long long>() == true);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == true);
   REQUIRE(data.canConvert<QChar>() == true);
   REQUIRE(data.canConvert<QString8>() == true);
   REQUIRE(data.canConvert<QString16>() == true);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_string8", "[qvariant]")
{
   QVariant data = QString("apple");

   REQUIRE(data.type() == QVariant::String);

   REQUIRE(data.canConvert<bool>() == true);
   REQUIRE(data.canConvert<short>() == true);
   REQUIRE(data.canConvert<ushort>() == true);
   REQUIRE(data.canConvert<int>() == true);
   REQUIRE(data.canConvert<uint>() == true);
   REQUIRE(data.canConvert<long>() == true);
   REQUIRE(data.canConvert<ulong>() == true);
   REQUIRE(data.canConvert<long long>() == true);
   REQUIRE(data.canConvert<unsigned long long>() == true);
   REQUIRE(data.canConvert<double>() == true);
   REQUIRE(data.canConvert<float>() == true);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == true);
   REQUIRE(data.canConvert<QChar>() == true);
   REQUIRE(data.canConvert<QString8>() == true);
   REQUIRE(data.canConvert<QString16>() == true);
   REQUIRE(data.canConvert<QStringList>() == true);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == true);
   REQUIRE(data.canConvert<QTime>() == true);
   REQUIRE(data.canConvert<QDateTime>() == true);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == true);
   REQUIRE(data.canConvert<QUuid>() == true);

   // test two
   data = QString("53");

   REQUIRE(data.canConvert<bool>() == true);
   REQUIRE(data.canConvert<short>() == true);
   REQUIRE(data.canConvert<ushort>() == true);
   REQUIRE(data.canConvert<int>() == true);
   REQUIRE(data.canConvert<uint>() == true);
   REQUIRE(data.canConvert<long>() == true);
   REQUIRE(data.canConvert<ulong>() == true);
   REQUIRE(data.canConvert<long long>() == true);
   REQUIRE(data.canConvert<unsigned long long>() == true);
   REQUIRE(data.canConvert<double>() == true);
   REQUIRE(data.canConvert<float>() == true);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == true);
   REQUIRE(data.canConvert<QChar>() == true);
   REQUIRE(data.canConvert<QString8>() == true);
   REQUIRE(data.canConvert<QString16>() == true);
   REQUIRE(data.canConvert<QStringList>() == true);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == true);
   REQUIRE(data.canConvert<QTime>() == true);
   REQUIRE(data.canConvert<QDateTime>() == true);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == true);
   REQUIRE(data.canConvert<QUuid>() == true);

   // test three
   data = QString("2021-04-01");

   REQUIRE(data.canConvert<bool>() == true);
   REQUIRE(data.canConvert<short>() == true);
   REQUIRE(data.canConvert<ushort>() == true);
   REQUIRE(data.canConvert<int>() == true);
   REQUIRE(data.canConvert<uint>() == true);
   REQUIRE(data.canConvert<long>() == true);
   REQUIRE(data.canConvert<ulong>() == true);
   REQUIRE(data.canConvert<long long>() == true);
   REQUIRE(data.canConvert<unsigned long long>() == true);
   REQUIRE(data.canConvert<double>() == true);
   REQUIRE(data.canConvert<float>() == true);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == true);
   REQUIRE(data.canConvert<QChar>() == true);
   REQUIRE(data.canConvert<QString8>() == true);
   REQUIRE(data.canConvert<QString16>() == true);
   REQUIRE(data.canConvert<QStringList>() == true);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == true);
   REQUIRE(data.canConvert<QTime>() == true);
   REQUIRE(data.canConvert<QDateTime>() == true);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == true);
   REQUIRE(data.canConvert<QUuid>() == true);
}

TEST_CASE("QVariant can_convert_t_string16", "[qvariant]")
{
   QVariant data = QString16("apple");

   REQUIRE(data.type() == QVariant::String16);

   REQUIRE(data.canConvert<bool>() == true);
   REQUIRE(data.canConvert<short>() == true);
   REQUIRE(data.canConvert<ushort>() == true);
   REQUIRE(data.canConvert<int>() == true);
   REQUIRE(data.canConvert<uint>() == true);
   REQUIRE(data.canConvert<long>() == true);
   REQUIRE(data.canConvert<ulong>() == true);
   REQUIRE(data.canConvert<long long>() == true);
   REQUIRE(data.canConvert<unsigned long long>() == true);
   REQUIRE(data.canConvert<double>() == true);
   REQUIRE(data.canConvert<float>() == true);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == true);
   REQUIRE(data.canConvert<QChar>() == true);
   REQUIRE(data.canConvert<QString8>() == true);
   REQUIRE(data.canConvert<QString16>() == true);
   REQUIRE(data.canConvert<QStringList>() == true);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == true);
   REQUIRE(data.canConvert<QTime>() == true);
   REQUIRE(data.canConvert<QDateTime>() == true);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == true);
   REQUIRE(data.canConvert<QUuid>() == true);
}

TEST_CASE("QVariant can_convert_t_date", "[qvariant]")
{
   QVariant data = QDate(2021, 4, 1);

   REQUIRE(data.type() == QVariant::Date);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == true);
   REQUIRE(data.canConvert<QString16>() == true);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == true);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == true);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_time", "[qvariant]")
{
   QVariant data = QTime(14, 52, 3);

   REQUIRE(data.type() == QVariant::Time);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == true);
   REQUIRE(data.canConvert<QString16>() == true);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == true);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_datetime", "[qvariant]")
{
   QVariant data = QDateTime(QDate(2021, 4, 1), QTime(14, 52, 3));

   REQUIRE(data.type() == QVariant::DateTime);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == true);
   REQUIRE(data.canConvert<QString16>() == true);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == true);
   REQUIRE(data.canConvert<QTime>() == true);
   REQUIRE(data.canConvert<QDateTime>() == true);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_list", "[qvariant]")
{
   QList<QVariant> list1 = {QString("orange"), 17};
   QVariant data = list1;

   REQUIRE(data.type() == QVariant::List);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == true);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == true);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_hash", "[qvariant]")
{
   QHash<QString, QVariant> hash1 = { {"orange", 17} };
   QVariant data = hash1;

   REQUIRE(data.type() == QVariant::Hash);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == true);
   REQUIRE(data.canConvert<QVariantMap>() == true);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_multihash", "[qvariant]")
{
   QMultiHash<QString, QVariant> hash1 = { {"orange", 17}, {"orange", 42} };
   QVariant data = hash1;

   REQUIRE(data.type() == QVariant::MultiHash);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>()  == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == true);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_map", "[qvariant]")
{
   QMap<QString, QVariant> map1 = { {"orange", 17} };

   QVariant data    = map1;
   QVariantMap map2 = data.value<QVariantMap>();

   REQUIRE(data.type() == QVariant::Map);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == true);
   REQUIRE(data.canConvert<QVariantMap>() == true);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_multimap", "[qvariant]")
{
   QMultiMap<QString, QVariant> map = { {"orange", 17}, {"orange", 42} };
   QVariant data = map;

   REQUIRE(data.type() == QVariant::MultiMap);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantMap>()  == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == true);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_json_value", "[qvariant]")
{
   {
      QJsonValue value = QJsonValue(9.37);
      QVariant data = value;

      REQUIRE(data.type() == QVariant::JsonValue);

      REQUIRE(data.canConvert<bool>() == true);
      REQUIRE(data.canConvert<short>() == true);
      REQUIRE(data.canConvert<ushort>() == true);
      REQUIRE(data.canConvert<int>() == true);
      REQUIRE(data.canConvert<uint>() == true);
      REQUIRE(data.canConvert<long>() == true);
      REQUIRE(data.canConvert<ulong>() == true);
      REQUIRE(data.canConvert<long long>() == true);
      REQUIRE(data.canConvert<unsigned long long>() == true);
      REQUIRE(data.canConvert<double>() == true);
      REQUIRE(data.canConvert<float>() == true);

      REQUIRE(data.canConvert<QBitArray>() == false);
      REQUIRE(data.canConvert<QByteArray>() == false);
      REQUIRE(data.canConvert<QChar>() == true);
      REQUIRE(data.canConvert<QString8>() == true);
      REQUIRE(data.canConvert<QString16>() == true);
      REQUIRE(data.canConvert<QStringList>() == false);
      REQUIRE(data.canConvert<QStringView>() == false);

      REQUIRE(data.canConvert<QDate>() == false);
      REQUIRE(data.canConvert<QTime>() == false);
      REQUIRE(data.canConvert<QDateTime>() == false);
      REQUIRE(data.canConvert<QLocale>() == false);

      REQUIRE(data.canConvert<QVariantList>() == true);
      REQUIRE(data.canConvert<QVariantHash>() == true);
      REQUIRE(data.canConvert<QVariantMap>() == true);
      REQUIRE(data.canConvert<QVariantMultiHash>() == false);
      REQUIRE(data.canConvert<QVariantMultiMap>() == false);

      REQUIRE(data.canConvert<QJsonArray>() == false);
      REQUIRE(data.canConvert<QJsonDocument>() == false);
      REQUIRE(data.canConvert<QJsonValue>() == true);
      REQUIRE(data.canConvert<QJsonObject>() == false);

      REQUIRE(data.canConvert<QLine>() == false);
      REQUIRE(data.canConvert<QLineF>() == false);
      REQUIRE(data.canConvert<QPoint>() == false);
      REQUIRE(data.canConvert<QPointF>() == false);
      REQUIRE(data.canConvert<QRect>() == false);
      REQUIRE(data.canConvert<QRectF>() == false);
      REQUIRE(data.canConvert<QSize>() == false);
      REQUIRE(data.canConvert<QSizeF>() == false);

      REQUIRE(data.canConvert<QEasingCurve>() == false);
      REQUIRE(data.canConvert<QModelIndex>() == false);
      REQUIRE(data.canConvert<QUrl>() == false);
      REQUIRE(data.canConvert<QUuid>() == false);
   }

   {
      QJsonValue value = QJsonValue(QString("CopperSpice"));
      QVariant data = value;

      REQUIRE(data.type() == QVariant::JsonValue);

      REQUIRE(data.canConvert<bool>() == true);
      REQUIRE(data.canConvert<short>() == true);
      REQUIRE(data.canConvert<ushort>() == true);
      REQUIRE(data.canConvert<int>() == true);
      REQUIRE(data.canConvert<uint>() == true);
      REQUIRE(data.canConvert<long>() == true);
      REQUIRE(data.canConvert<ulong>() == true);
      REQUIRE(data.canConvert<long long>() == true);
      REQUIRE(data.canConvert<unsigned long long>() == true);
      REQUIRE(data.canConvert<double>() == true);
      REQUIRE(data.canConvert<float>() == true);

      REQUIRE(data.canConvert<QBitArray>() == false);
      REQUIRE(data.canConvert<QByteArray>() == false);
      REQUIRE(data.canConvert<QChar>() == true);
      REQUIRE(data.canConvert<QString8>() == true);
      REQUIRE(data.canConvert<QString16>() == true);
      REQUIRE(data.canConvert<QStringList>() == false);
      REQUIRE(data.canConvert<QStringView>() == false);

      REQUIRE(data.canConvert<QDate>() == false);
      REQUIRE(data.canConvert<QTime>() == false);
      REQUIRE(data.canConvert<QDateTime>() == false);
      REQUIRE(data.canConvert<QLocale>() == false);

      REQUIRE(data.canConvert<QVariantList>() == true);
      REQUIRE(data.canConvert<QVariantHash>() == true);
      REQUIRE(data.canConvert<QVariantMap>() == true);
      REQUIRE(data.canConvert<QVariantMultiHash>() == false);
      REQUIRE(data.canConvert<QVariantMultiMap>() == false);

      REQUIRE(data.canConvert<QJsonArray>() == false);
      REQUIRE(data.canConvert<QJsonDocument>() == false);
      REQUIRE(data.canConvert<QJsonValue>() == true);
      REQUIRE(data.canConvert<QJsonObject>() == false);

      REQUIRE(data.canConvert<QLine>() == false);
      REQUIRE(data.canConvert<QLineF>() == false);
      REQUIRE(data.canConvert<QPoint>() == false);
      REQUIRE(data.canConvert<QPointF>() == false);
      REQUIRE(data.canConvert<QRect>() == false);
      REQUIRE(data.canConvert<QRectF>() == false);
      REQUIRE(data.canConvert<QSize>() == false);
      REQUIRE(data.canConvert<QSizeF>() == false);

      REQUIRE(data.canConvert<QEasingCurve>() == false);
      REQUIRE(data.canConvert<QModelIndex>() == false);
      REQUIRE(data.canConvert<QUrl>() == false);
      REQUIRE(data.canConvert<QUuid>() == false);
   }
}

TEST_CASE("QVariant can_convert_t_line", "[qvariant]")
{
   QVariant data = QLine(6, 12, 0, 3);

   REQUIRE(data.type() == QVariant::Line);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == true);
   REQUIRE(data.canConvert<QLineF>() == true);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_linef", "[qvariant]")
{
   QVariant data = QLineF(6.4, 12.8, 0, 3.2);

   REQUIRE(data.type() == QVariant::LineF);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == true);
   REQUIRE(data.canConvert<QLineF>() == true);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_point", "[qvariant]")
{
   QVariant data = QPoint(17, 42);

   REQUIRE(data.type() == QVariant::Point);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == true);
   REQUIRE(data.canConvert<QPointF>() == true);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_pointf", "[qvariant]")
{
   QVariant data = QPointF(17.9, 42.0);

   REQUIRE(data.type() == QVariant::PointF);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == true);
   REQUIRE(data.canConvert<QPointF>() == true);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_rect", "[qvariant]")
{
   QVariant data = QRect(9, 12, 18, 7);

   REQUIRE(data.type() == QVariant::Rect);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == true);
   REQUIRE(data.canConvert<QRectF>() == true);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_rectf", "[qvariant]")
{
   QVariant data = QRectF(9.7, 12.1, 18.0, 7.4);

   REQUIRE(data.type() == QVariant::RectF);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == true);
   REQUIRE(data.canConvert<QRectF>() == true);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_size", "[qvariant]")
{
   QVariant data = QSize(9, 12);

   REQUIRE(data.type() == QVariant::Size);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == true);
   REQUIRE(data.canConvert<QSizeF>() == true);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_sizef", "[qvariant]")
{
   QVariant data = QSizeF(9.7, 12.1);

   REQUIRE(data.type() == QVariant::SizeF);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == true);
   REQUIRE(data.canConvert<QSizeF>() == true);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_url", "[qvariant]")
{
   QString str("https://www.copperspice.com");

   QUrl url(str);
   QVariant data(url);

   REQUIRE(data.type() == QVariant::Url);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == true);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == true);
   REQUIRE(data.canConvert<QString16>() == true);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == true);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant can_convert_t_uuid", "[qvariant]")
{
   QUuid value("{ba80a7c0-d463-e361-78eb-1394049152ba}");
   QVariant data = value;

   REQUIRE(data.type() == QVariant::Uuid);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == true);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == true);
   REQUIRE(data.canConvert<QString16>() == true);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == false);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == true);
}

TEST_CASE("QVariant can_convert_t_easing_curve", "[qvariant]")
{
   QEasingCurve curve(QEasingCurve::OutElastic);
   curve.setAmplitude(2.0);
   curve.setPeriod(0.5);

   QVariant data = QVariant::fromValue(curve);

   REQUIRE(data.type() == QVariant::EasingCurve);

   REQUIRE(data.canConvert<bool>() == false);
   REQUIRE(data.canConvert<short>() == false);
   REQUIRE(data.canConvert<ushort>() == false);
   REQUIRE(data.canConvert<int>() == false);
   REQUIRE(data.canConvert<uint>() == false);
   REQUIRE(data.canConvert<long>() == false);
   REQUIRE(data.canConvert<ulong>() == false);
   REQUIRE(data.canConvert<long long>() == false);
   REQUIRE(data.canConvert<unsigned long long>() == false);
   REQUIRE(data.canConvert<double>() == false);
   REQUIRE(data.canConvert<float>() == false);

   REQUIRE(data.canConvert<QBitArray>() == false);
   REQUIRE(data.canConvert<QByteArray>() == false);
   REQUIRE(data.canConvert<QChar>() == false);
   REQUIRE(data.canConvert<QString8>() == false);
   REQUIRE(data.canConvert<QString16>() == false);
   REQUIRE(data.canConvert<QStringList>() == false);
   REQUIRE(data.canConvert<QStringView>() == false);

   REQUIRE(data.canConvert<QDate>() == false);
   REQUIRE(data.canConvert<QTime>() == false);
   REQUIRE(data.canConvert<QDateTime>() == false);
   REQUIRE(data.canConvert<QLocale>() == false);

   REQUIRE(data.canConvert<QVariantList>() == false);
   REQUIRE(data.canConvert<QVariantHash>() == false);
   REQUIRE(data.canConvert<QVariantMap>() == false);
   REQUIRE(data.canConvert<QVariantMultiHash>() == false);
   REQUIRE(data.canConvert<QVariantMultiMap>() == false);

   REQUIRE(data.canConvert<QJsonArray>() == false);
   REQUIRE(data.canConvert<QJsonDocument>() == false);
   REQUIRE(data.canConvert<QJsonValue>() == false);
   REQUIRE(data.canConvert<QJsonObject>() == false);

   REQUIRE(data.canConvert<QLine>() == false);
   REQUIRE(data.canConvert<QLineF>() == false);
   REQUIRE(data.canConvert<QPoint>() == false);
   REQUIRE(data.canConvert<QPointF>() == false);
   REQUIRE(data.canConvert<QRect>() == false);
   REQUIRE(data.canConvert<QRectF>() == false);
   REQUIRE(data.canConvert<QSize>() == false);
   REQUIRE(data.canConvert<QSizeF>() == false);

   REQUIRE(data.canConvert<QEasingCurve>() == true);
   REQUIRE(data.canConvert<QModelIndex>() == false);
   REQUIRE(data.canConvert<QUrl>() == false);
   REQUIRE(data.canConvert<QUuid>() == false);
}

TEST_CASE("QVariant constructor_empty", "[qvariant]")
{
   QVariant data;

   REQUIRE(data.isValid() == false);
   REQUIRE(data.type() == QVariant::Invalid);

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_bool", "[qvariant]")
{
   QVariant data = true;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Bool);
   REQUIRE(data.typeName() == "bool");

   REQUIRE(data.toBool()              == true);
   REQUIRE(data.toInt()               == 1);
   REQUIRE(data.toUInt()              == 1);
   REQUIRE(data.toLong()              == 1);
   REQUIRE(data.toULong()             == 1);
   REQUIRE(data.toLongLong()          == 1);
   REQUIRE(data.toULongLong()         == 1);
   REQUIRE(data.toDouble()            == 1);
   REQUIRE(data.toFloat()             == 1.0f);
   REQUIRE(data.toReal()              == 1.0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == "true");
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == "true");
   REQUIRE(data.toString16()          == "true");
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());

   // test two
   data = false;

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0.0f);
   REQUIRE(data.toReal()              == 0.0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == "false");
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == "false");
   REQUIRE(data.toString16()          == "false");
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_int", "[qvariant]")
{
   QVariant data = 17;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Int);
   REQUIRE(data.typeName() == "int");

   REQUIRE(data.toBool()              == true);
   REQUIRE(data.toInt()               == 17);
   REQUIRE(data.toUInt()              == 17);
   REQUIRE(data.toLong()              == 17);
   REQUIRE(data.toULong()             == 17);
   REQUIRE(data.toLongLong()          == 17);
   REQUIRE(data.toULongLong()         == 17);
   REQUIRE(data.toDouble()            == 17.0);
   REQUIRE(data.toFloat()             == 17.0f);
   REQUIRE(data.toReal()              == 17.0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == "17");
   REQUIRE(data.toChar()              == '\x11');
   REQUIRE(data.toString()            == "17");
   REQUIRE(data.toString16()          == "17");
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_double", "[qvariant]")
{
   QVariant data = 3.14159;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Double);
   REQUIRE(data.typeName() == "double");

   REQUIRE(data.toBool()              == true);
   REQUIRE(data.toInt()               == 3);
   REQUIRE(data.toUInt()              == 3);
   REQUIRE(data.toLong()              == 3);
   REQUIRE(data.toULong()             == 3);
   REQUIRE(data.toLongLong()          == 3);
   REQUIRE(data.toULongLong()         == 3);
   REQUIRE(data.toDouble()            == 3.14159);
   REQUIRE(data.toFloat()             == 3.14159f);
   REQUIRE(data.toReal()              == 3.14159);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == "3.14159");
   REQUIRE(data.toChar()              == '\x03');
   REQUIRE(data.toString()            == "3.14159");
   REQUIRE(data.toString16()          == "3.14159");
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_bytearray", "[qvariant]")
{
   QByteArray value(11, ' ');

   value[0]  = 'C';
   value[1]  = 'o';
   value[2]  = 'p';
   value[3]  = 'p';
   value[4]  = 'e';
   value[5]  = 'r';
   value[6]  = 'S';
   value[7]  = 'p';
   value[8]  = 'i';
   value[9]  = 'c';
   value[10] = 'e';
   value[11] = '\0';

   QVariant data = value;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::ByteArray);
   REQUIRE(data.typeName() == "QByteArray");

   REQUIRE(data.toBool()              == true);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray("CopperSpice") + '\0');
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString("CopperSpice") + '\0');
   REQUIRE(data.toString16()          == QString16("CopperSpice") + '\0');
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());

   // test two
   data = QByteArray("53");

   REQUIRE(data.toBool()              == true);
   REQUIRE(data.toInt()               == 53);
   REQUIRE(data.toUInt()              == 53);
   REQUIRE(data.toLong()              == 53);
   REQUIRE(data.toULong()             == 53);
   REQUIRE(data.toLongLong()          == 53);
   REQUIRE(data.toULongLong()         == 53);
   REQUIRE(data.toDouble()            == 53);
   REQUIRE(data.toFloat()             == 53.0f);
   REQUIRE(data.toReal()              == 53.0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray("53"));
   REQUIRE(data.toChar()              == '\x35');
   REQUIRE(data.toString()            == QString("53"));
   REQUIRE(data.toString16()          == QString16("53"));
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_char", "[qvariant]")
{
   QVariant data = QChar('B');

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::QChar);
   REQUIRE(data.typeName() == "QChar");

   REQUIRE(data.toBool()              == true);
   REQUIRE(data.toInt()               == 66);
   REQUIRE(data.toUInt()              == 66);
   REQUIRE(data.toLong()              == 66);
   REQUIRE(data.toULong()             == 66);
   REQUIRE(data.toLongLong()          == 66);
   REQUIRE(data.toULongLong()         == 66);
   REQUIRE(data.toDouble()            == 0.0);
   REQUIRE(data.toFloat()             == 0.0f);
   REQUIRE(data.toReal()              == 0.0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == "B");
   REQUIRE(data.toChar()              == '\U00000042');
   REQUIRE(data.toString()            == "B");
   REQUIRE(data.toString16()          == "B");
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_string8", "[qvariant]")
{
   QVariant data = QString("apple");

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::String);
   REQUIRE(data.typeName() == "QString");

   REQUIRE(data.toBool()              == true);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == "apple");
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == "apple");
   REQUIRE(data.toString16()          == "apple");
   REQUIRE(data.toStringList()        == QStringList("apple"));
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl("apple"));
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());

   // test two
   data = QString("53");

   REQUIRE(data.toBool()              == true);
   REQUIRE(data.toInt()               == 53);
   REQUIRE(data.toUInt()              == 53);
   REQUIRE(data.toLong()              == 53);
   REQUIRE(data.toULong()             == 53);
   REQUIRE(data.toLongLong()          == 53);
   REQUIRE(data.toULongLong()         == 53);
   REQUIRE(data.toDouble()            == 53);
   REQUIRE(data.toFloat()             == 53.0f);
   REQUIRE(data.toReal()              == 53);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray("53"));
   REQUIRE(data.toChar()              == '\x35');
   REQUIRE(data.toString()            == QString("53"));
   REQUIRE(data.toString16()          == QString16("53"));
   REQUIRE(data.toStringList()        == QStringList("53"));
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl("53"));
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());

   // test three
   data = QString("2021-04-01");

   REQUIRE(data.toBool()              == true);
   REQUIRE(data.toInt()               == 2021);
   REQUIRE(data.toUInt()              == 2021);
   REQUIRE(data.toLong()              == 2021);
   REQUIRE(data.toULong()             == 2021);
   REQUIRE(data.toLongLong()          == 2021);
   REQUIRE(data.toULongLong()         == 2021);
   REQUIRE(data.toDouble()            == 2021);
   REQUIRE(data.toFloat()             == 2021);
   REQUIRE(data.toReal()              == 2021);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray("2021-04-01"));
   REQUIRE(data.toChar()              == u'\u07e5');
   REQUIRE(data.toString()            == QString("2021-04-01"));
   REQUIRE(data.toString16()          == QString16("2021-04-01"));
   REQUIRE(data.toStringList()        == QStringList("2021-04-01"));
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate(2021, 4, 1));
   REQUIRE(data.toTime()              == QTime(20, 01, 04));
   REQUIRE(data.toDateTime()          == QDateTime( QDate(2021, 4, 1) ));
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl("2021-04-01"));
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_string16", "[qvariant]")
{
   QVariant data = QString16("apple");

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::String16);
   REQUIRE(data.typeName() == "QString16");

   REQUIRE(data.toBool()              == true);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == "apple");
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == "apple");
   REQUIRE(data.toString16()          == "apple");
   REQUIRE(data.toStringList()        == QStringList("apple"));
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl("apple"));
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());

   // test two
   data = QString("53");

   REQUIRE(data.toBool()              == true);
   REQUIRE(data.toInt()               == 53);
   REQUIRE(data.toUInt()              == 53);
   REQUIRE(data.toLong()              == 53);
   REQUIRE(data.toULong()             == 53);
   REQUIRE(data.toLongLong()          == 53);
   REQUIRE(data.toULongLong()         == 53);
   REQUIRE(data.toDouble()            == 53);
   REQUIRE(data.toFloat()             == 53.0f);
   REQUIRE(data.toReal()              == 53);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray("53"));
   REQUIRE(data.toChar()              == '\x35');
   REQUIRE(data.toString()            == QString("53"));
   REQUIRE(data.toString16()          == QString16("53"));
   REQUIRE(data.toStringList()        == QStringList("53"));
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl("53"));
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_stringlist", "[qvariant]")
{
   QStringList list;
   list.append("apple");

   QVariant data = list;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::StringList);
   REQUIRE(data.typeName() == "QStringList");

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == "apple");
   REQUIRE(data.toString16()          == "apple");
   REQUIRE(data.toStringList()        == QStringList("apple"));
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>( {QString("apple")} ));
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_date", "[qvariant]")
{
   QVariant data = QDate(2021, 4, 1);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Date);
   REQUIRE(data.typeName() == "QDate");

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == "2021-04-01");
   REQUIRE(data.toString16()          == QString16("2021-04-01"));
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate(2021, 4, 1));
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime(QDate(2021, 4, 1), QTime()));
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_time", "[qvariant]")
{
   QVariant data = QTime(14, 52, 3);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Time);
   REQUIRE(data.typeName() == "QTime");

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == "14:52:03");
   REQUIRE(data.toString16()          == QString16("14:52:03"));
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime(14, 52, 3));
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_datetime", "[qvariant]")
{
   QVariant data = QDateTime(QDate(2021, 4, 1), QTime(14, 52, 3));

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::DateTime);
   REQUIRE(data.typeName() == "QDateTime");

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == "2021-04-01T14:52:03");
   REQUIRE(data.toString16()          == QString16("2021-04-01T14:52:03"));
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate(2021, 04, 01));
   REQUIRE(data.toTime()              == QTime(14, 52, 3));
   REQUIRE(data.toDateTime()          == QDateTime(QDate(2021, 4, 1), QTime(14, 52, 3)));
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_locale", "[qvariant]")
{
   QVariant data = QLocale(QLocale::Dutch, QLocale::Netherlands);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Locale);
   REQUIRE(data.typeName() == "QLocale");

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == "");
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == "");
   REQUIRE(data.toString16()          == "");
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale(QLocale::Dutch, QLocale::Netherlands));

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_list", "[qvariant]")
{
   QList<QVariant> list1 = {QString("orange"), 17};

   QVariant data      = list1;
   QVariantList list2 = data.value<QVariantList>();

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::List);
   REQUIRE(data.typeName() == "QVariantList");

   REQUIRE(list1    == list2);
   REQUIRE(list2[0] == QString("orange"));
   REQUIRE(list2[1] == 17);

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList( {"orange", "17"} ));
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>({QString("orange"), 17}));
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_hash", "[qvariant]")
{
   QHash<QString, QVariant> hash1 = { {"orange", 17} };

   QVariant data = hash1;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Hash);
   REQUIRE(data.typeName() == "QVariantHash");

   QVariantHash hash2 = data.value<QVariantHash>();
   REQUIRE(hash1 == hash2);
   REQUIRE(hash2.value("orange").toInt() == 17);

   QVariantHash hash3 = data.toHash();
   REQUIRE(hash1 == hash3);
   REQUIRE(hash3.value("orange").toInt() == 17);

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>( {{"orange", 17}} ));
   REQUIRE(data.toMap()               == QMap<QString, QVariant>(  {{"orange", 17}} ));
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_multihash", "[qvariant]")
{
   QMultiHash<QString, QVariant> hash1 = { {"orange", 17}, {"orange", 42} };

   QVariant data = hash1;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::MultiHash);
   REQUIRE(data.typeName() == "QVariantMultiHash");

   QVariantMultiHash hash2 = data.value<QVariantMultiHash>();
   REQUIRE(hash1 == hash2);

   REQUIRE( ((hash2.value("orange").toInt() == 17) || (hash2.value("orange").toInt() == 42)) );

   QVariantMultiHash hash3 = data.toMultiHash();
   REQUIRE(hash1 == hash3);
   REQUIRE( ((hash3.value("orange").toInt() == 17) || (hash3.value("orange").toInt() == 42)) );

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>( { {"orange", 17}, {"orange", 42} } ));
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_map", "[qvariant]")
{
   QMap<QString, QVariant> map1 = { {"orange", 17} };

   QVariant data    = map1;
   QVariantMap map2 = data.value<QVariantMap>();

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Map);
   REQUIRE(data.typeName() == "QVariantMap");

   REQUIRE(map2.contains("orange") == true);
   REQUIRE(map2.contains("17") == false);

   REQUIRE(map1 == map2);
   REQUIRE(map2.value("orange").toInt() == 17);

   QVariantMap map3 = data.toMap();
   REQUIRE(map1 == map3);
   REQUIRE(map3.value("orange").toInt() == 17);

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>( {{"orange", 17}} ));
   REQUIRE(data.toMap()               == QMap<QString, QVariant>(  {{"orange", 17}} ));
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_multimap", "[qvariant]")
{
   QMultiMap<QString, QVariant> map = { {"orange", 17}, {"orange", 42} };

   QVariant data = map;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::MultiMap);
   REQUIRE(data.typeName() == "QVariantMultiMap");

   REQUIRE(map.values("orange") == QVariantList( {17, 42} ));

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>( { {"orange", 17}, {"orange", 42} } ));

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_json_value", "[qvariant]")
{
   {
      QJsonValue value = QJsonValue(9.37);
      QVariant data = value;

      REQUIRE(data.isValid());
      REQUIRE(data.type() == QVariant::JsonValue);
      REQUIRE(data.typeName() == "QJsonValue");

      REQUIRE(data.toBool()              == true);
      REQUIRE(data.toInt()               == 9);
      REQUIRE(data.toUInt()              == 9);
      REQUIRE(data.toLong()              == 9);
      REQUIRE(data.toULong()             == 9);
      REQUIRE(data.toLongLong()          == 9);
      REQUIRE(data.toULongLong()         == 9);
      REQUIRE(data.toDouble()            == 9.37);
      REQUIRE(data.toFloat()             == 9.37f);
      REQUIRE(data.toReal()              == 9.37);

      REQUIRE(data.toBitArray()          == QBitArray());
      REQUIRE(data.toByteArray()         == QByteArray());
      REQUIRE(data.toChar()              == '\x09');
      REQUIRE(data.toString()            == QString("9.37"));
      REQUIRE(data.toString16()          == QString16("9.37"));
      REQUIRE(data.toStringList()        == QStringList());
      REQUIRE(data.value<QStringView>()  == QStringView());

      REQUIRE(data.toDate()              == QDate());
      REQUIRE(data.toTime()              == QTime());
      REQUIRE(data.toDateTime()          == QDateTime());
      REQUIRE(data.toLocale()            == QLocale());

      REQUIRE(data.toList()              == QList<QVariant>());
      REQUIRE(data.toHash()              == QHash<QString, QVariant>());
      REQUIRE(data.toMap()               == QMap<QString, QVariant>());
      REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
      REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

      REQUIRE(data.toJsonArray()         == QJsonArray());
      REQUIRE(data.toJsonDocument()      == QJsonDocument());
      REQUIRE(data.toJsonValue()         == QJsonValue(9.37));
      REQUIRE(data.toJsonObject()        == QJsonObject());

      REQUIRE(data.toLine()              == QLine());
      REQUIRE(data.toLineF()             == QLineF());
      REQUIRE(data.toPoint()             == QPoint());
      REQUIRE(data.toPointF()            == QPointF());
      REQUIRE(data.toRect()              == QRect());
      REQUIRE(data.toRectF()             == QRectF());
      REQUIRE(data.toSize()              == QSize());
      REQUIRE(data.toSizeF()             == QSizeF());

      REQUIRE(data.toEasingCurve()       == QEasingCurve());
      REQUIRE(data.toModelIndex()        == QModelIndex());
      REQUIRE(data.toUrl()               == QUrl());
      REQUIRE(data.toUuid()              == QUuid());

      REQUIRE(data.toRegularExpression().pattern() == QString());
      REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
   }

   {
      QJsonValue value = QJsonValue(QString("CopperSpice"));
      QVariant data = value;

      REQUIRE(data.isValid());
      REQUIRE(data.type() == QVariant::JsonValue);

      REQUIRE(data.toBool()              == true);
      REQUIRE(data.toInt()               == 0);
      REQUIRE(data.toUInt()              == 0);
      REQUIRE(data.toLong()              == 0);
      REQUIRE(data.toULong()             == 0);
      REQUIRE(data.toLongLong()          == 0);
      REQUIRE(data.toULongLong()         == 0);
      REQUIRE(data.toDouble()            == 0);
      REQUIRE(data.toFloat()             == 0);
      REQUIRE(data.toReal()              == 0);

      REQUIRE(data.toBitArray()          == QBitArray());
      REQUIRE(data.toByteArray()         == QByteArray());
      REQUIRE(data.toChar()              == '\0');
      REQUIRE(data.toString()            == QString("CopperSpice"));
      REQUIRE(data.toString16()          == QString16("CopperSpice"));
      REQUIRE(data.toStringList()        == QStringList());
      REQUIRE(data.value<QStringView>()  == QStringView());

      REQUIRE(data.toDate()              == QDate());
      REQUIRE(data.toTime()              == QTime());
      REQUIRE(data.toDateTime()          == QDateTime());
      REQUIRE(data.toLocale()            == QLocale());

      REQUIRE(data.toList()              == QList<QVariant>());
      REQUIRE(data.toHash()              == QHash<QString, QVariant>());
      REQUIRE(data.toMap()               == QMap<QString, QVariant>());
      REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
      REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

      REQUIRE(data.toJsonArray()         == QJsonArray());
      REQUIRE(data.toJsonDocument()      == QJsonDocument());
      REQUIRE(data.toJsonValue()         == QJsonValue(QString("CopperSpice")));
      REQUIRE(data.toJsonObject()        == QJsonObject());

      REQUIRE(data.toLine()              == QLine());
      REQUIRE(data.toLineF()             == QLineF());
      REQUIRE(data.toPoint()             == QPoint());
      REQUIRE(data.toPointF()            == QPointF());
      REQUIRE(data.toRect()              == QRect());
      REQUIRE(data.toRectF()             == QRectF());
      REQUIRE(data.toSize()              == QSize());
      REQUIRE(data.toSizeF()             == QSizeF());

      REQUIRE(data.toEasingCurve()       == QEasingCurve());
      REQUIRE(data.toModelIndex()        == QModelIndex());
      REQUIRE(data.toUrl()               == QUrl());
      REQUIRE(data.toUuid()              == QUuid());

      REQUIRE(data.toRegularExpression().pattern() == QString());
      REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
   }
}

TEST_CASE("QVariant constructor_json_object", "[qvariant]")
{
   QJsonObject child;
   child["key_1"] = 111;
   child["key_2"] = 222;

   QJsonObject root;
   root["key_0"] = child;

   // part 1
   QVariantMap map     = root.toVariantMap();
   QJsonObject newRoot = QJsonObject::fromVariantMap(map);

   QJsonDocument doc(newRoot);

   QString result_1 = doc.toJsonString().simplified();
   QString result_2 = "{ \"key_0\": { \"key_1\": 111, \"key_2\": 222 } }";

   REQUIRE(result_1 == result_2);

   // part 2
   QVariant data = map;

   QJsonObject root_2  = QJsonObject::fromVariantMap(data.toMap());
   QJsonObject child_2 = root_2["key_0"].toObject();

   REQUIRE(child_2["key_1"].toInt() == 111);
   REQUIRE(child_2["key_2"].toInt() == 222);
}

TEST_CASE("QVariant constructor_line", "[qvariant]")
{
   QVariant data = QLine(6, 12, 0, 3);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Line);
   REQUIRE(data.typeName() == "QLine");

   REQUIRE(data.value<QLine>()  == QLine(6, 12, 0, 3));
   REQUIRE(data.value<QLineF>() == QLineF(6, 12, 0, 3));

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine(6, 12, 0, 3));
   REQUIRE(data.toLineF()             == QLineF(6, 12, 0, 3));
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_linef", "[qvariant]")
{
   QVariant data = QLineF(6.4, 12.8, 0, 3.2);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::LineF);
   REQUIRE(data.typeName() == "QLineF");

   REQUIRE(data.value<QLine>()  == QLine(6, 13, 0, 3));
   REQUIRE(data.value<QLineF>() == QLineF(6.4, 12.8, 0, 3.2));

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine(6, 13, 0, 3));
   REQUIRE(data.toLineF()             == QLineF(6.4, 12.8, 0, 3.2));
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_point", "[qvariant]")
{
   QVariant data = QPoint(17, 42);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Point);
   REQUIRE(data.typeName() == "QPoint");

   REQUIRE(data.value<QPoint>()  == QPoint(17, 42));
   REQUIRE(data.value<QPointF>() == QPointF(17, 42));

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint(17, 42));
   REQUIRE(data.toPointF()            == QPointF(17, 42));
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_pointf", "[qvariant]")
{
   QVariant data = QPointF(17.9, 42.0);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::PointF);
   REQUIRE(data.typeName() == "QPointF");

   REQUIRE(data.value<QPoint>()  == QPoint(18, 42));
   REQUIRE(data.value<QPointF>() == QPointF(17.9, 42.0));

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint(18, 42));
   REQUIRE(data.toPointF()            == QPointF(17.9, 42.0));
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_rect", "[qvariant]")
{
   QVariant data = QRect(9, 12, 18, 7);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Rect);
   REQUIRE(data.typeName() == "QRect");

   REQUIRE(data.value<QRect>()  == QRect(9, 12, 18, 7));
   REQUIRE(data.value<QRectF>() == QRectF(9, 12, 18, 7));

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect(9, 12, 18, 7));
   REQUIRE(data.toRectF()             == QRectF(9, 12, 18, 7));
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_rectf", "[qvariant]")
{
   QVariant data = QRectF(9.7, 12.1, 18.0, 7.4);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::RectF);
   REQUIRE(data.typeName() == "QRectF");

   REQUIRE(data.value<QRect>()  == QRect(10, 12, 18, 7));
   REQUIRE(data.value<QRectF>() == QRectF(9.7, 12.1, 18.0, 7.4));

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect(10, 12, 18, 7));
   REQUIRE(data.toRectF()             == QRectF(9.7, 12.1, 18.0, 7.4));
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_size", "[qvariant]")
{
   QVariant data = QSize(9, 12);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Size);
   REQUIRE(data.typeName() == "QSize");

   REQUIRE(data.value<QSize>()  == QSize(9, 12));
   REQUIRE(data.value<QSizeF>() == QSizeF(9, 12));

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize(9, 12));
   REQUIRE(data.toSizeF()             == QSizeF(9, 12));

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_sizef", "[qvariant]")
{
   QVariant data = QSizeF(9.7, 12.1);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::SizeF);
   REQUIRE(data.typeName() == "QSizeF");

   REQUIRE(data.value<QSize>()  == QSize(10, 12));
   REQUIRE(data.value<QSizeF>() == QSizeF(9.7, 12.1));

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize(10, 12));
   REQUIRE(data.toSizeF()             == QSizeF(9.7, 12.1));

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_url", "[qvariant]")
{
   QString str("https://www.copperspice.com");

   QVariant tmpStr(str);

   REQUIRE(tmpStr.isValid());
   REQUIRE(tmpStr.type() == QVariant::String);

   QUrl url(str);
   QVariant data(url);

   REQUIRE(tmpStr == data);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Url);
   REQUIRE(data.typeName() == "QUrl");

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray("https://www.copperspice.com"));
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == str);
   REQUIRE(data.toString16()          == str);
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl("https://www.copperspice.com"));
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_uuid", "[qvariant]")
{
   QUuid value("{ba80a7c0-d463-e361-78eb-1394049152ba}");
   QVariant data = value;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Uuid);
   REQUIRE(data.typeName() == "QUuid");

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == "{ba80a7c0-d463-e361-78eb-1394049152ba}");
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == "{ba80a7c0-d463-e361-78eb-1394049152ba}");
   REQUIRE(data.toString16()          == "{ba80a7c0-d463-e361-78eb-1394049152ba}");
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid("{ba80a7c0-d463-e361-78eb-1394049152ba}"));

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_easing_curve", "[qvariant]")
{
   QEasingCurve curve(QEasingCurve::OutElastic);
   curve.setAmplitude(2.0);
   curve.setPeriod(0.5);

   QVariant data = QVariant::fromValue(curve);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::EasingCurve);
   REQUIRE(data.typeName() == "QEasingCurve");

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());

   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   QEasingCurve result = data.toEasingCurve();
   REQUIRE(result.type() == QEasingCurve::OutElastic);
   REQUIRE(result.amplitude() == 2.0);
   REQUIRE(result.period() == 0.5);

   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.toRegularExpression().pattern() == QString());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

TEST_CASE("QVariant constructor_regex", "[qvariant]")
{
   QRegularExpression regexp = QRegularExpression("[abc]\\w+");

   QVariant data(regexp);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::RegularExpression);
   REQUIRE(data.typeName() == "QRegularExpression");

   REQUIRE(data.toBool()              == false);
   REQUIRE(data.toInt()               == 0);
   REQUIRE(data.toUInt()              == 0);
   REQUIRE(data.toLong()              == 0);
   REQUIRE(data.toULong()             == 0);
   REQUIRE(data.toLongLong()          == 0);
   REQUIRE(data.toULongLong()         == 0);
   REQUIRE(data.toDouble()            == 0);
   REQUIRE(data.toFloat()             == 0);
   REQUIRE(data.toReal()              == 0);

   REQUIRE(data.toBitArray()          == QBitArray());
   REQUIRE(data.toByteArray()         == QByteArray());
   REQUIRE(data.toChar()              == '\0');
   REQUIRE(data.toString()            == QString());
   REQUIRE(data.toString16()          == QString16());
   REQUIRE(data.toStringList()        == QStringList());
   REQUIRE(data.value<QStringView>()  == QStringView());

   REQUIRE(data.toDate()              == QDate());
   REQUIRE(data.toTime()              == QTime());
   REQUIRE(data.toDateTime()          == QDateTime());
   REQUIRE(data.toLocale()            == QLocale());

   REQUIRE(data.toList()              == QList<QVariant>());
   REQUIRE(data.toHash()              == QHash<QString, QVariant>());
   REQUIRE(data.toMap()               == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash()         == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap()          == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray()         == QJsonArray());
   REQUIRE(data.toJsonDocument()      == QJsonDocument());
   REQUIRE(data.toJsonValue()         == QJsonValue());
   REQUIRE(data.toJsonObject()        == QJsonObject());

   REQUIRE(data.toLine()              == QLine());
   REQUIRE(data.toLineF()             == QLineF());
   REQUIRE(data.toPoint()             == QPoint());
   REQUIRE(data.toPointF()            == QPointF());
   REQUIRE(data.toRect()              == QRect());
   REQUIRE(data.toRectF()             == QRectF());
   REQUIRE(data.toSize()              == QSize());
   REQUIRE(data.toSizeF()             == QSizeF());

   REQUIRE(data.toEasingCurve()       == QEasingCurve());
   REQUIRE(data.toModelIndex()        == QModelIndex());
   REQUIRE(data.toUrl()               == QUrl());
   REQUIRE(data.toUuid()              == QUuid());

   REQUIRE(data.value<QRegularExpression>().pattern() == "[abc]\\w+");
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
}

struct MyCustomType
{
   QString dataStr;
   QList<int> dataList;
};

CS_DECLARE_METATYPE(MyCustomType)

TEST_CASE("QVariant constructor_user_type", "[qvariant]")
{
   MyCustomType input = {"On a clear day", {8, 17} };

   QVariant data1 = QVariant::fromValue(input);
   QVariant data2 = data1;

   REQUIRE(data2.isValid());
   REQUIRE(data2.userType() == QVariant::typeToTypeId<MyCustomType>());
   REQUIRE(data2.typeName() == "MyCustomType");

   REQUIRE(data2.canConvert(QVariant::Bool) == false);
   REQUIRE(data2.canConvert(QVariant::Short) == false);
   REQUIRE(data2.canConvert(QVariant::UShort) == false);
   REQUIRE(data2.canConvert(QVariant::Int) == false);
   REQUIRE(data2.canConvert(QVariant::UInt) == false);
   REQUIRE(data2.canConvert(QVariant::Long) == false);
   REQUIRE(data2.canConvert(QVariant::ULong) == false);
   REQUIRE(data2.canConvert(QVariant::LongLong) == false);
   REQUIRE(data2.canConvert(QVariant::ULongLong) == false);
   REQUIRE(data2.canConvert(QVariant::Double) == false);
   REQUIRE(data2.canConvert(QVariant::Float) == false);

   REQUIRE(data2.canConvert(QVariant::QChar) == false);
   REQUIRE(data2.canConvert(QVariant::Char) == false);
   REQUIRE(data2.canConvert(QVariant::SChar) == false);
   REQUIRE(data2.canConvert(QVariant::UChar) == false);
   REQUIRE(data2.canConvert(QVariant::Char8_t) == false);
   REQUIRE(data2.canConvert(QVariant::Char16_t) == false);
   REQUIRE(data2.canConvert(QVariant::Char32_t) == false);

   REQUIRE(data2.canConvert(QVariant::ByteArray) == false);
   REQUIRE(data2.canConvert(QVariant::BitArray) == false);
   REQUIRE(data2.canConvert(QVariant::String) == false);
   REQUIRE(data2.canConvert(QVariant::String16) == false);
   REQUIRE(data2.canConvert(QVariant::StringList) == false);
   REQUIRE(data2.canConvert(QVariant::StringView) == false);

   REQUIRE(data2.canConvert(QVariant::Date) == false);
   REQUIRE(data2.canConvert(QVariant::DateTime) == false);
   REQUIRE(data2.canConvert(QVariant::Time) == false);
   REQUIRE(data2.canConvert(QVariant::Locale) == false);

   REQUIRE(data2.canConvert(QVariant::JsonArray) == false);
   REQUIRE(data2.canConvert(QVariant::JsonDocument) == false);
   REQUIRE(data2.canConvert(QVariant::JsonObject) == false);
   REQUIRE(data2.canConvert(QVariant::JsonValue) == false);

   REQUIRE(data2.canConvert(QVariant::Line) == false);
   REQUIRE(data2.canConvert(QVariant::LineF) == false);
   REQUIRE(data2.canConvert(QVariant::Point) == false);
   REQUIRE(data2.canConvert(QVariant::PointF) == false);
   REQUIRE(data2.canConvert(QVariant::Polygon) == false);
   REQUIRE(data2.canConvert(QVariant::PolygonF) == false);
   REQUIRE(data2.canConvert(QVariant::Rect) == false);
   REQUIRE(data2.canConvert(QVariant::RectF) == false);
   REQUIRE(data2.canConvert(QVariant::Size) == false);
   REQUIRE(data2.canConvert(QVariant::SizeF) == false);

   REQUIRE(data2.canConvert(QVariant::List) == false);
   REQUIRE(data2.canConvert(QVariant::Hash) == false);
   REQUIRE(data2.canConvert(QVariant::MultiHash) == false);
   REQUIRE(data2.canConvert(QVariant::Map) == false);
   REQUIRE(data2.canConvert(QVariant::MultiMap) == false);

   REQUIRE(data2.canConvert(QVariant::Void) == false);
   REQUIRE(data2.canConvert(QVariant::VoidStar) == false);
   REQUIRE(data2.canConvert(QVariant::ObjectStar) == false);
   REQUIRE(data2.canConvert(QVariant::WidgetStar) == false);

   REQUIRE(data2.canConvert(QVariant::EasingCurve) == false);
   REQUIRE(data2.canConvert(QVariant::ModelIndex) == false);
   REQUIRE(data2.canConvert(QVariant::PersistentModelIndex) == false);
   REQUIRE(data2.canConvert(QVariant::Url) == false);
   REQUIRE(data2.canConvert(QVariant::Uuid) == false);

   REQUIRE(data2.canConvert(QVariant::Bitmap) == false);
   REQUIRE(data2.canConvert(QVariant::Brush) == false);
   REQUIRE(data2.canConvert(QVariant::Color) == false);
   REQUIRE(data2.canConvert(QVariant::Cursor) == false);
   REQUIRE(data2.canConvert(QVariant::Font) == false);
   REQUIRE(data2.canConvert(QVariant::Icon) == false);
   REQUIRE(data2.canConvert(QVariant::Image) == false);
   REQUIRE(data2.canConvert(QVariant::KeySequence) == false);
   REQUIRE(data2.canConvert(QVariant::Matrix) == false);
   REQUIRE(data2.canConvert(QVariant::Matrix4x4) == false);
   REQUIRE(data2.canConvert(QVariant::Palette) == false);
   REQUIRE(data2.canConvert(QVariant::Pen) == false);
   REQUIRE(data2.canConvert(QVariant::Pixmap) == false);
   REQUIRE(data2.canConvert(QVariant::Quaternion) == false);
   REQUIRE(data2.canConvert(QVariant::Region) == false);
   REQUIRE(data2.canConvert(QVariant::RegularExpression) == false);
   REQUIRE(data2.canConvert(QVariant::SizePolicy) == false);
   REQUIRE(data2.canConvert(QVariant::TextFormat) == false);
   REQUIRE(data2.canConvert(QVariant::TextLength) == false);
   REQUIRE(data2.canConvert(QVariant::Transform) == false);
   REQUIRE(data2.canConvert(QVariant::Variant) == false);
   REQUIRE(data2.canConvert(QVariant::Vector2D) == false);
   REQUIRE(data2.canConvert(QVariant::Vector3D) == false);
   REQUIRE(data2.canConvert(QVariant::Vector4D) == false);

   MyCustomType data3 = data2.value<MyCustomType>();

   REQUIRE(data3.dataStr == "On a clear day");
   REQUIRE(data3.dataList == QList<int>{8, 17});
}

TEST_CASE("QVariant copy_assign", "[qvariant]")
{
   QVariant data_a = QString("apple");
   QVariant data_b(data_a);

   REQUIRE(data_a.isValid() == true);
   REQUIRE(data_b.isValid() == true);

   REQUIRE(data_a == data_b);
   REQUIRE(data_b == QVariant{QString("apple")});

   //
   QVariant data_c;
   data_c = data_a;

   REQUIRE(data_a.isValid() == true);
   REQUIRE(data_c.isValid() == true);

   REQUIRE(data_a == data_c);
   REQUIRE(data_c == QVariant{QString("apple")});
}

TEST_CASE("QVariant copy_constructor_string8", "[qvariant]")
{
   QVariant data1 = QString("apple");

   QVariant data2 = data1;

   REQUIRE(data1.isValid());
   REQUIRE(data2.isValid());

   data1 = QString("pineapple");

   REQUIRE(data1.toString() == "pineapple");
   REQUIRE(data2.toString() == "apple");
}

TEST_CASE("QVariant copy_constructor_string16", "[qvariant]")
{
   QVariant data1 = QString16("apple");

   QVariant data2 = data1;

   REQUIRE(data1.isValid());
   REQUIRE(data2.isValid());

   data1 = QString16("pineapple");

   REQUIRE(data1.toString16() == "pineapple");
   REQUIRE(data2.toString16() == "apple");
}

TEST_CASE("QVariant copy_constructor_list", "[qvariant]")
{
   QList<QVariant> list = {QString("orange"), 17};

   QVariant data1 = list;
   QVariant data2 = data1;

   REQUIRE(data1.isValid());
   REQUIRE(data1.type() == QVariant::List);

   REQUIRE(data2.isValid());
   REQUIRE(data2.type() == QVariant::List);

   REQUIRE(data1 == data2);

   REQUIRE(data1.toList()[0].toString() == QString("orange"));
   REQUIRE(data2.toList()[0].toString() == QString("orange"));

   REQUIRE(data1.toList()[1].toInt() == 17);
   REQUIRE(data2.toList()[1].toInt() == 17);
}

TEST_CASE("QVariant convert_int", "[qvariant]")
{
   {
      QVariant data = 17;

      REQUIRE(data.type() == QVariant::Int);
      REQUIRE(data.type() != QVariant::UInt);
      REQUIRE(data.type() != QVariant::LongLong);
      REQUIRE(data.type() != QVariant::Double);
      REQUIRE(data.type() != QVariant::Char);

      REQUIRE(data.canConvert(QVariant::Bool) == true);
      REQUIRE(data.canConvert(QVariant::Short) == true);
      REQUIRE(data.canConvert(QVariant::UShort) == true);
      REQUIRE(data.canConvert(QVariant::Int) == true);
      REQUIRE(data.canConvert(QVariant::UInt)== true);
      REQUIRE(data.canConvert(QVariant::Long) == true);
      REQUIRE(data.canConvert(QVariant::ULong) == true);
      REQUIRE(data.canConvert(QVariant::LongLong)== true);
      REQUIRE(data.canConvert(QVariant::ULongLong) == true);
      REQUIRE(data.canConvert(QVariant::Double)== true);
      REQUIRE(data.canConvert(QVariant::Float) == true);

      REQUIRE(data.canConvert(QVariant::QChar) == true);
      REQUIRE(data.canConvert(QVariant::Char) == true);
      REQUIRE(data.canConvert(QVariant::SChar) == true);
      REQUIRE(data.canConvert(QVariant::UChar) == true);
      REQUIRE(data.canConvert(QVariant::Char8_t) == false);
      REQUIRE(data.canConvert(QVariant::Char16_t) == false);
      REQUIRE(data.canConvert(QVariant::Char32_t) == false);

      REQUIRE(data.toUInt()     == 17);
      REQUIRE(data.toUInt()     == 17);
      REQUIRE(data.toLongLong() == 17);
      REQUIRE(data.toDouble()   == 17);

      REQUIRE(data.toChar().unicode() == 0x11);
   }

   {
      QVariant data = -65536;

      REQUIRE(data.type() == QVariant::Int);
      REQUIRE(data.type() != QVariant::UInt);
      REQUIRE(data.type() != QVariant::LongLong);
      REQUIRE(data.type() != QVariant::Char);

      REQUIRE(data.canConvert(QVariant::Bool) == true);
      REQUIRE(data.canConvert(QVariant::Short) == true);
      REQUIRE(data.canConvert(QVariant::UShort) == true);
      REQUIRE(data.canConvert(QVariant::Int) == true);
      REQUIRE(data.canConvert(QVariant::UInt)== true);
      REQUIRE(data.canConvert(QVariant::Long) == true);
      REQUIRE(data.canConvert(QVariant::ULong) == true);
      REQUIRE(data.canConvert(QVariant::LongLong)== true);
      REQUIRE(data.canConvert(QVariant::ULongLong) == true);
      REQUIRE(data.canConvert(QVariant::Double)== true);
      REQUIRE(data.canConvert(QVariant::Float) == true);

      REQUIRE(data.canConvert(QVariant::QChar) == true);
      REQUIRE(data.canConvert(QVariant::Char) == true);
      REQUIRE(data.canConvert(QVariant::SChar) == true);
      REQUIRE(data.canConvert(QVariant::UChar) == true);
      REQUIRE(data.canConvert(QVariant::Char8_t) == false);
      REQUIRE(data.canConvert(QVariant::Char16_t) == false);
      REQUIRE(data.canConvert(QVariant::Char32_t) == false);

      REQUIRE(data.toUInt()     == 0xFFFF0000);
      REQUIRE(data.toUInt()     == 4294901760);
      REQUIRE(data.toLongLong() == -65536);

      REQUIRE(data.toChar().unicode() == 0xffff0000);
   }

   {
      QVariant data = QVariant::fromValue<char>('a');

      REQUIRE(data.type() != QVariant::Int);
      REQUIRE(data.type() != QVariant::UInt);
      REQUIRE(data.type() != QVariant::LongLong);
      REQUIRE(data.type() == QVariant::Char);

      REQUIRE(data.canConvert(QVariant::Bool) == true);
      REQUIRE(data.canConvert(QVariant::Short) == true);
      REQUIRE(data.canConvert(QVariant::UShort) == true);
      REQUIRE(data.canConvert(QVariant::Int) == true);
      REQUIRE(data.canConvert(QVariant::UInt)== true);
      REQUIRE(data.canConvert(QVariant::Long) == true);
      REQUIRE(data.canConvert(QVariant::ULong) == true);
      REQUIRE(data.canConvert(QVariant::LongLong)== true);
      REQUIRE(data.canConvert(QVariant::ULongLong) == true);
      REQUIRE(data.canConvert(QVariant::Double)== false);
      REQUIRE(data.canConvert(QVariant::Float) == false);

      REQUIRE(data.canConvert(QVariant::QChar) == true);
      REQUIRE(data.canConvert(QVariant::Char) == true);
      REQUIRE(data.canConvert(QVariant::SChar) == true);
      REQUIRE(data.canConvert(QVariant::UChar) == true);
      REQUIRE(data.canConvert(QVariant::Char8_t) == false);
      REQUIRE(data.canConvert(QVariant::Char16_t) == false);
      REQUIRE(data.canConvert(QVariant::Char32_t) == false);

      REQUIRE(data.toInt()      == 97);
      REQUIRE(data.toUInt()     == 97);
      REQUIRE(data.toLongLong() == 97);
      REQUIRE(data.toChar()     == 'a');
   }

   {
      QVariant data = QVariant::fromValue<char>('\xF0');

      REQUIRE(data.type() != QVariant::Int);
      REQUIRE(data.type() != QVariant::UInt);
      REQUIRE(data.type() != QVariant::LongLong);
      REQUIRE(data.type() == QVariant::Char);

      REQUIRE(data.canConvert(QVariant::Bool) == true);
      REQUIRE(data.canConvert(QVariant::Short) == true);
      REQUIRE(data.canConvert(QVariant::UShort) == true);
      REQUIRE(data.canConvert(QVariant::Int) == true);
      REQUIRE(data.canConvert(QVariant::UInt)== true);
      REQUIRE(data.canConvert(QVariant::Long) == true);
      REQUIRE(data.canConvert(QVariant::ULong) == true);
      REQUIRE(data.canConvert(QVariant::LongLong)== true);
      REQUIRE(data.canConvert(QVariant::ULongLong) == true);
      REQUIRE(data.canConvert(QVariant::Double)== false);
      REQUIRE(data.canConvert(QVariant::Float) == false);

      REQUIRE(data.canConvert(QVariant::QChar) == true);
      REQUIRE(data.canConvert(QVariant::Char) == true);
      REQUIRE(data.canConvert(QVariant::SChar) == true);
      REQUIRE(data.canConvert(QVariant::UChar) == true);
      REQUIRE(data.canConvert(QVariant::Char8_t) == false);
      REQUIRE(data.canConvert(QVariant::Char16_t) == false);
      REQUIRE(data.canConvert(QVariant::Char32_t) == false);

      REQUIRE(data.toInt()      == -16);
      REQUIRE(data.toUInt()     == 240);
      REQUIRE(data.toLongLong() == -16);
      REQUIRE(data.toChar()     == U'\xF0');
   }

   {
      QVariant data = QVariant::fromValue<long long>(4294967297);

      REQUIRE(data.type() != QVariant::Int);
      REQUIRE(data.type() != QVariant::UInt);
      REQUIRE(data.type() == QVariant::LongLong);
      REQUIRE(data.type() != QVariant::Char);

      REQUIRE(data.canConvert(QVariant::Bool) == true);
      REQUIRE(data.canConvert(QVariant::Short) == true);
      REQUIRE(data.canConvert(QVariant::UShort) == true);
      REQUIRE(data.canConvert(QVariant::Int) == true);
      REQUIRE(data.canConvert(QVariant::UInt)== true);
      REQUIRE(data.canConvert(QVariant::Long) == true);
      REQUIRE(data.canConvert(QVariant::ULong) == true);
      REQUIRE(data.canConvert(QVariant::LongLong)== true);
      REQUIRE(data.canConvert(QVariant::ULongLong) == true);
      REQUIRE(data.canConvert(QVariant::Double)== true);
      REQUIRE(data.canConvert(QVariant::Float) == true);

      REQUIRE(data.canConvert(QVariant::QChar) == true);
      REQUIRE(data.canConvert(QVariant::Char) == true);
      REQUIRE(data.canConvert(QVariant::SChar) == true);
      REQUIRE(data.canConvert(QVariant::UChar) == true);
      REQUIRE(data.canConvert(QVariant::Char8_t) == false);
      REQUIRE(data.canConvert(QVariant::Char16_t) == false);
      REQUIRE(data.canConvert(QVariant::Char32_t) == false);

      REQUIRE(data.toInt()      == 0);
      REQUIRE(data.toUInt()     == 0);
      REQUIRE(data.toLongLong() == 4294967297);
      REQUIRE(data.toChar()     == '\0');
   }
}

TEST_CASE("QVariant convert_uint", "[qvariant]")
{
   {
      QVariant data = 0xFFFF0000;

      REQUIRE(data.type() != QVariant::Int);
      REQUIRE(data.type() == QVariant::UInt);
      REQUIRE(data.type() != QVariant::LongLong);

      REQUIRE(data.canConvert(QVariant::Bool) == true);
      REQUIRE(data.canConvert(QVariant::Short) == true);
      REQUIRE(data.canConvert(QVariant::UShort) == true);
      REQUIRE(data.canConvert(QVariant::Int) == true);
      REQUIRE(data.canConvert(QVariant::UInt)== true);
      REQUIRE(data.canConvert(QVariant::Long) == true);
      REQUIRE(data.canConvert(QVariant::ULong) == true);
      REQUIRE(data.canConvert(QVariant::LongLong)== true);
      REQUIRE(data.canConvert(QVariant::ULongLong) == true);
      REQUIRE(data.canConvert(QVariant::Double)== true);
      REQUIRE(data.canConvert(QVariant::Float) == true);

      REQUIRE(data.canConvert(QVariant::QChar) == true);
      REQUIRE(data.canConvert(QVariant::Char) == true);
      REQUIRE(data.canConvert(QVariant::SChar) == true);
      REQUIRE(data.canConvert(QVariant::UChar) == true);
      REQUIRE(data.canConvert(QVariant::Char8_t) == false);
      REQUIRE(data.canConvert(QVariant::Char16_t) == false);
      REQUIRE(data.canConvert(QVariant::Char32_t) == false);

      REQUIRE(data.toInt()      == -65536);
      REQUIRE(data.toUInt()     == 0xFFFF0000);
      REQUIRE(data.toLongLong() == -65536);
   }
}

TEST_CASE("QVariant convert_double", "[qvariant]")
{
   {
      QVariant data = 3.14159;

      REQUIRE(data.type() != QVariant::Int);
      REQUIRE(data.type() != QVariant::UInt);
      REQUIRE(data.type() != QVariant::LongLong);
      REQUIRE(data.type() == QVariant::Double);

      REQUIRE(data.canConvert(QVariant::Bool) == true);
      REQUIRE(data.canConvert(QVariant::Short) == true);
      REQUIRE(data.canConvert(QVariant::UShort) == true);
      REQUIRE(data.canConvert(QVariant::Int) == true);
      REQUIRE(data.canConvert(QVariant::UInt)== true);
      REQUIRE(data.canConvert(QVariant::Long) == true);
      REQUIRE(data.canConvert(QVariant::ULong) == true);
      REQUIRE(data.canConvert(QVariant::LongLong)== true);
      REQUIRE(data.canConvert(QVariant::ULongLong) == true);
      REQUIRE(data.canConvert(QVariant::Double)== true);
      REQUIRE(data.canConvert(QVariant::Float) == true);

      REQUIRE(data.canConvert(QVariant::QChar) == true);
      REQUIRE(data.canConvert(QVariant::Char) == true);
      REQUIRE(data.canConvert(QVariant::SChar) == true);
      REQUIRE(data.canConvert(QVariant::UChar) == true);
      REQUIRE(data.canConvert(QVariant::Char8_t) == false);
      REQUIRE(data.canConvert(QVariant::Char16_t) == false);
      REQUIRE(data.canConvert(QVariant::Char32_t) == false);

      REQUIRE(data.toInt()      == 3);
      REQUIRE(data.toUInt()     == 3);
      REQUIRE(data.toLongLong() == 3);
      REQUIRE(data.toDouble() == 3.14159);
   }
}

TEST_CASE("QVariant can_convert_string8", "[qvariant]")
{
   QVariant data = QString("apple");

   REQUIRE(data.canConvert(QVariant::Invalid) == false);

   REQUIRE(data.canConvert(QVariant::Bool) == true);
   REQUIRE(data.canConvert(QVariant::Short) == true);
   REQUIRE(data.canConvert(QVariant::UShort) == true);
   REQUIRE(data.canConvert(QVariant::Int) == true);
   REQUIRE(data.canConvert(QVariant::UInt) == true);
   REQUIRE(data.canConvert(QVariant::Long) == true);
   REQUIRE(data.canConvert(QVariant::ULong) == true);
   REQUIRE(data.canConvert(QVariant::LongLong) == true);
   REQUIRE(data.canConvert(QVariant::ULongLong) == true);
   REQUIRE(data.canConvert(QVariant::Double) == true);
   REQUIRE(data.canConvert(QVariant::Float) == true);

   REQUIRE(data.canConvert(QVariant::QChar) == true);
   REQUIRE(data.canConvert(QVariant::Char) == true);
   REQUIRE(data.canConvert(QVariant::SChar) == true);
   REQUIRE(data.canConvert(QVariant::UChar) == true);
   REQUIRE(data.canConvert(QVariant::Char8_t) == false);
   REQUIRE(data.canConvert(QVariant::Char16_t) == false);
   REQUIRE(data.canConvert(QVariant::Char32_t) == false);

   REQUIRE(data.canConvert(QVariant::ByteArray) == true);
   REQUIRE(data.canConvert(QVariant::BitArray) == false);
   REQUIRE(data.canConvert(QVariant::String) == true);
   REQUIRE(data.canConvert(QVariant::String16) == true);
   REQUIRE(data.canConvert(QVariant::StringList) == true);
   REQUIRE(data.canConvert(QVariant::StringView) == false);

   REQUIRE(data.canConvert(QVariant::Date) == true);
   REQUIRE(data.canConvert(QVariant::DateTime) == true);
   REQUIRE(data.canConvert(QVariant::Time) == true);
   REQUIRE(data.canConvert(QVariant::Locale) == false);

   REQUIRE(data.canConvert(QVariant::JsonArray) == false);
   REQUIRE(data.canConvert(QVariant::JsonDocument) == false);
   REQUIRE(data.canConvert(QVariant::JsonObject) == false);
   REQUIRE(data.canConvert(QVariant::JsonValue) == false);

   REQUIRE(data.canConvert(QVariant::Line) == false);
   REQUIRE(data.canConvert(QVariant::LineF) == false);
   REQUIRE(data.canConvert(QVariant::Point) == false);
   REQUIRE(data.canConvert(QVariant::PointF) == false);
   REQUIRE(data.canConvert(QVariant::Polygon) == false);
   REQUIRE(data.canConvert(QVariant::PolygonF) == false);
   REQUIRE(data.canConvert(QVariant::Rect) == false);
   REQUIRE(data.canConvert(QVariant::RectF) == false);
   REQUIRE(data.canConvert(QVariant::Size) == false);
   REQUIRE(data.canConvert(QVariant::SizeF) == false);

   REQUIRE(data.canConvert(QVariant::List) == false);
   REQUIRE(data.canConvert(QVariant::Hash) == false);
   REQUIRE(data.canConvert(QVariant::MultiHash) == false);
   REQUIRE(data.canConvert(QVariant::Map) == false);
   REQUIRE(data.canConvert(QVariant::MultiMap) == false);

   REQUIRE(data.canConvert(QVariant::Void) == false);
   REQUIRE(data.canConvert(QVariant::VoidStar) == false);
   REQUIRE(data.canConvert(QVariant::ObjectStar) == false);
   REQUIRE(data.canConvert(QVariant::WidgetStar) == false);

   REQUIRE(data.canConvert(QVariant::EasingCurve) == false);
   REQUIRE(data.canConvert(QVariant::ModelIndex) == false);
   REQUIRE(data.canConvert(QVariant::PersistentModelIndex) == false);
   REQUIRE(data.canConvert(QVariant::Url) == true);
   REQUIRE(data.canConvert(QVariant::Uuid) == true);

   REQUIRE(data.canConvert(QVariant::Bitmap) == false);
   REQUIRE(data.canConvert(QVariant::Brush) == false);
   REQUIRE(data.canConvert(QVariant::Color) == true);
   REQUIRE(data.canConvert(QVariant::Cursor) == false);
   REQUIRE(data.canConvert(QVariant::Font) == true);
   REQUIRE(data.canConvert(QVariant::Icon) == false);
   REQUIRE(data.canConvert(QVariant::Image) == false);
   REQUIRE(data.canConvert(QVariant::KeySequence) == true);
   REQUIRE(data.canConvert(QVariant::Matrix) == false);
   REQUIRE(data.canConvert(QVariant::Matrix4x4) == false);
   REQUIRE(data.canConvert(QVariant::Palette) == false);
   REQUIRE(data.canConvert(QVariant::Pen) == false);
   REQUIRE(data.canConvert(QVariant::Pixmap) == false);
   REQUIRE(data.canConvert(QVariant::Quaternion) == false);
   REQUIRE(data.canConvert(QVariant::Region) == false);
   REQUIRE(data.canConvert(QVariant::RegularExpression) == false);
   REQUIRE(data.canConvert(QVariant::SizePolicy) == false);
   REQUIRE(data.canConvert(QVariant::TextFormat) == false);
   REQUIRE(data.canConvert(QVariant::TextLength) == false);
   REQUIRE(data.canConvert(QVariant::Transform) == false);
   REQUIRE(data.canConvert(QVariant::Variant) == false);
   REQUIRE(data.canConvert(QVariant::Vector2D) == false);
   REQUIRE(data.canConvert(QVariant::Vector3D) == false);
   REQUIRE(data.canConvert(QVariant::Vector4D) == false);
}

TEST_CASE("QVariant can_convert_string16", "[qvariant]")
{
   QVariant data = QString16("apple");

   REQUIRE(data.canConvert(QVariant::Invalid) == false);
}

TEST_CASE("QVariant swap_string", "[qvariant]")
{
   QVariant data1 = QString("apple");
   QVariant data2 = QString("pineapple");

   data1.swap(data2);

   REQUIRE(data1.toString() == "pineapple");
   REQUIRE(data2.toString() == "apple");
}

TEST_CASE("QVariant type_name", "[qvariant]")
{
   QVariant data = QByteArray();
   REQUIRE(data.typeName() == "QByteArray");

   data = QUrl();
   REQUIRE(data.typeName() == "QUrl");

   data = QUuid();
   REQUIRE(data.typeName() == "QUuid");
}

TEST_CASE("QVariant type_to_name", "[qvariant]")
{
   REQUIRE(QVariant::typeToName(QVariant::Invalid) == "");
   REQUIRE(QVariant::typeToName(QVariant::Bool) == "bool");
   REQUIRE(QVariant::typeToName(QVariant::Short) == "short");
   REQUIRE(QVariant::typeToName(QVariant::UShort) == "unsigned short");
   REQUIRE(QVariant::typeToName(QVariant::Int) == "int");
   REQUIRE(QVariant::typeToName(QVariant::UInt) == "unsigned int");
   REQUIRE(QVariant::typeToName(QVariant::Long) == "long");
   REQUIRE(QVariant::typeToName(QVariant::ULong) == "unsigned long");
   REQUIRE(QVariant::typeToName(QVariant::LongLong) == "long long");
   REQUIRE(QVariant::typeToName(QVariant::ULongLong) == "unsigned long long");
   REQUIRE(QVariant::typeToName(QVariant::Double) == "double");
   REQUIRE(QVariant::typeToName(QVariant::Float) == "float");

   REQUIRE(QVariant::typeToName(QVariant::QChar) == "QChar");
   REQUIRE(QVariant::typeToName(QVariant::Char) == "char");
   REQUIRE(QVariant::typeToName(QVariant::SChar) == "signed char");
   REQUIRE(QVariant::typeToName(QVariant::UChar) == "unsigned char");

   REQUIRE(QVariant::typeToName(QVariant::Char16_t) == "char16_t");
   REQUIRE(QVariant::typeToName(QVariant::Char32_t) == "char32_t");

   REQUIRE(QVariant::typeToName(QVariant::BitArray) == "QBitArray");
   REQUIRE(QVariant::typeToName(QVariant::ByteArray) == "QByteArray");
   REQUIRE(QVariant::typeToName(QVariant::String) == "QString");
   REQUIRE(QVariant::typeToName(QVariant::String8) == "QString");
   REQUIRE(QVariant::typeToName(QVariant::String16) == "QString16");
   REQUIRE(QVariant::typeToName(QVariant::StringList) == "QStringList");
   REQUIRE(QVariant::typeToName(QVariant::StringView) == "QStringView");
   REQUIRE(QVariant::typeToName(QVariant::RegularExpression) == "QRegularExpression");

   REQUIRE(QVariant::typeToName(QVariant::Date) == "QDate");
   REQUIRE(QVariant::typeToName(QVariant::Time) == "QTime");
   REQUIRE(QVariant::typeToName(QVariant::DateTime) == "QDateTime");
   REQUIRE(QVariant::typeToName(QVariant::Locale) == "QLocale");

   REQUIRE(QVariant::typeToName(QVariant::JsonArray) == "QJsonArray");
   REQUIRE(QVariant::typeToName(QVariant::JsonDocument) == "QJsonDocument");
   REQUIRE(QVariant::typeToName(QVariant::JsonObject) == "QJsonObject");
   REQUIRE(QVariant::typeToName(QVariant::JsonValue) == "QJsonValue");

   REQUIRE(QVariant::typeToName(QVariant::Line) == "QLine");
   REQUIRE(QVariant::typeToName(QVariant::LineF) == "QLineF");
   REQUIRE(QVariant::typeToName(QVariant::Point) == "QPoint");
   REQUIRE(QVariant::typeToName(QVariant::PointF) == "QPointF");
   REQUIRE(QVariant::typeToName(QVariant::Polygon) == "QPolygon");
   REQUIRE(QVariant::typeToName(QVariant::PolygonF) == "QPolygonF");
   REQUIRE(QVariant::typeToName(QVariant::Rect) == "QRect");
   REQUIRE(QVariant::typeToName(QVariant::RectF) == "QRectF");
   REQUIRE(QVariant::typeToName(QVariant::Size) == "QSize");
   REQUIRE(QVariant::typeToName(QVariant::SizeF) == "QSizeF");

   REQUIRE(QVariant::typeToName(QVariant::List) == "QVariantList");
   REQUIRE(QVariant::typeToName(QVariant::Hash) == "QVariantHash");
   REQUIRE(QVariant::typeToName(QVariant::MultiHash) == "QVariantMultiHash");
   REQUIRE(QVariant::typeToName(QVariant::Map) == "QVariantMap");
   REQUIRE(QVariant::typeToName(QVariant::MultiMap) == "QVariantMultiMap");

   REQUIRE(QVariant::typeToName(QVariant::Void) == "void");
   REQUIRE(QVariant::typeToName(QVariant::VoidStar) == "void*");
   REQUIRE(QVariant::typeToName(QVariant::ObjectStar) == "QObject*");
   REQUIRE(QVariant::typeToName(QVariant::WidgetStar) == "QWidget*");

   REQUIRE(QVariant::typeToName(QVariant::EasingCurve) == "QEasingCurve");
   REQUIRE(QVariant::typeToName(QVariant::ModelIndex) == "QModelIndex");
   REQUIRE(QVariant::typeToName(QVariant::PersistentModelIndex) == "QPersistentModelIndex");
   REQUIRE(QVariant::typeToName(QVariant::Url) == "QUrl");
   REQUIRE(QVariant::typeToName(QVariant::Uuid) == "QUuid");

   REQUIRE(QVariant::typeToName(QVariant::Bitmap) == "QBitmap");
   REQUIRE(QVariant::typeToName(QVariant::Brush) == "QBrush");
   REQUIRE(QVariant::typeToName(QVariant::Color) == "QColor");
   REQUIRE(QVariant::typeToName(QVariant::Cursor) == "QCursor");
   REQUIRE(QVariant::typeToName(QVariant::Font) == "QFont");
   REQUIRE(QVariant::typeToName(QVariant::Icon) == "QIcon");
   REQUIRE(QVariant::typeToName(QVariant::Image) == "QImage");
   REQUIRE(QVariant::typeToName(QVariant::KeySequence) == "QKeySequence");
   REQUIRE(QVariant::typeToName(QVariant::Matrix) == "QMatrix");
   REQUIRE(QVariant::typeToName(QVariant::Matrix4x4) == "QMatrix4x4");
   REQUIRE(QVariant::typeToName(QVariant::Palette) == "QPalette");
   REQUIRE(QVariant::typeToName(QVariant::Pen) == "QPen");
   REQUIRE(QVariant::typeToName(QVariant::Pixmap) == "QPixmap");
   REQUIRE(QVariant::typeToName(QVariant::Quaternion) == "QQuaternion");
   REQUIRE(QVariant::typeToName(QVariant::Region) == "QRegion");
   REQUIRE(QVariant::typeToName(QVariant::SizePolicy) == "QSizePolicy");
   REQUIRE(QVariant::typeToName(QVariant::TextLength) == "QTextLength");
   REQUIRE(QVariant::typeToName(QVariant::TextFormat) == "QTextFormat");
   REQUIRE(QVariant::typeToName(QVariant::Transform) == "QTransform");
   REQUIRE(QVariant::typeToName(QVariant::Vector2D) == "QVector2D");
   REQUIRE(QVariant::typeToName(QVariant::Vector3D) == "QVector3D");
   REQUIRE(QVariant::typeToName(QVariant::Vector4D) == "QVector4D");

   REQUIRE(QVariant::typeToName(QVariant::Variant) == "QVariant");
}

TEST_CASE("QVariant name_to_type", "[qvariant]")
{
   REQUIRE(QVariant::nameToType("SillName") == QVariant::Invalid);
   REQUIRE(QVariant::nameToType("bool") == QVariant::Bool);
   REQUIRE(QVariant::nameToType("short") == QVariant::Short);
   REQUIRE(QVariant::nameToType("unsigned short") == QVariant::UShort);
   REQUIRE(QVariant::nameToType("int") == QVariant::Int);
   REQUIRE(QVariant::nameToType("unsigned int") == QVariant::UInt);
   REQUIRE(QVariant::nameToType("long") == QVariant::Long);
   REQUIRE(QVariant::nameToType("unsigned long") == QVariant::ULong);
   REQUIRE(QVariant::nameToType("long long") == QVariant::LongLong);
   REQUIRE(QVariant::nameToType("unsigned long long") == QVariant::ULongLong);
   REQUIRE(QVariant::nameToType("double") == QVariant::Double);
   REQUIRE(QVariant::nameToType("float") == QVariant::Float);

   REQUIRE(QVariant::nameToType("QChar") == QVariant::QChar);
   REQUIRE(QVariant::nameToType("char") == QVariant::Char);
   REQUIRE(QVariant::nameToType("signed char") == QVariant::SChar);
   REQUIRE(QVariant::nameToType("unsigned char") == QVariant::UChar);

   REQUIRE(QVariant::nameToType("char16_t") == QVariant::Char16_t);
   REQUIRE(QVariant::nameToType("char32_t") == QVariant::Char32_t);

   REQUIRE(QVariant::nameToType("QBitArray") == QVariant::BitArray);
   REQUIRE(QVariant::nameToType("QByteArray") == QVariant::ByteArray);
   REQUIRE(QVariant::nameToType("QString") == QVariant::String);
   REQUIRE(QVariant::nameToType("QString8") == QVariant::String8);
   REQUIRE(QVariant::nameToType("QString16") == QVariant::String16);
   REQUIRE(QVariant::nameToType("QStringList") == QVariant::StringList);
   REQUIRE(QVariant::nameToType("QStringView") == QVariant::StringView);
   REQUIRE(QVariant::nameToType("QRegularExpression") == QVariant::RegularExpression);

   REQUIRE(QVariant::nameToType("QDate") == QVariant::Date);
   REQUIRE(QVariant::nameToType("QTime") == QVariant::Time);
   REQUIRE(QVariant::nameToType("QDateTime") == QVariant::DateTime);
   REQUIRE(QVariant::nameToType("QLocale") == QVariant::Locale);

   REQUIRE(QVariant::nameToType("QJsonArray") == QVariant::JsonArray);
   REQUIRE(QVariant::nameToType("QJsonDocument") ==QVariant::JsonDocument);
   REQUIRE(QVariant::nameToType("QJsonObject") == QVariant::JsonObject);
   REQUIRE(QVariant::nameToType("QJsonValue") == QVariant::JsonValue);

   REQUIRE(QVariant::nameToType("QLine") == QVariant::Line);
   REQUIRE(QVariant::nameToType("QLineF") == QVariant::LineF);
   REQUIRE(QVariant::nameToType("QPoint") == QVariant::Point);
   REQUIRE(QVariant::nameToType("QPointF") == QVariant::PointF);
   REQUIRE(QVariant::nameToType("QPolygon") == QVariant::Polygon);
   REQUIRE(QVariant::nameToType("QPolygonF") == QVariant::PolygonF);
   REQUIRE(QVariant::nameToType("QRectF") == QVariant::RectF);
   REQUIRE(QVariant::nameToType("QRect") == QVariant::Rect);
   REQUIRE(QVariant::nameToType("QRectF") == QVariant::RectF);
   REQUIRE(QVariant::nameToType("QSize") == QVariant::Size);
   REQUIRE(QVariant::nameToType("QSizeF") == QVariant::SizeF);

   REQUIRE(QVariant::nameToType("QVariantList") == QVariant::List);
   REQUIRE(QVariant::nameToType("QVariantHash") == QVariant::Hash);
   REQUIRE(QVariant::nameToType("QVariantMultiHash") == QVariant::MultiHash);
   REQUIRE(QVariant::nameToType("QVariantMap") == QVariant::Map);
   REQUIRE(QVariant::nameToType("QVariantMultiMap") == QVariant::MultiMap);

   REQUIRE(QVariant::nameToType("void") == QVariant::Void);
   REQUIRE(QVariant::nameToType("void*") == QVariant::VoidStar);
   REQUIRE(QVariant::nameToType("QObject*") == QVariant::ObjectStar);
   REQUIRE(QVariant::nameToType("QWidget*") == QVariant::WidgetStar);

   REQUIRE(QVariant::nameToType("QEasingCurve") == QVariant::EasingCurve);
   REQUIRE(QVariant::nameToType("QModelIndex") == QVariant::ModelIndex);
   REQUIRE(QVariant::nameToType("QPersistentModelIndex") == QVariant::PersistentModelIndex);
   REQUIRE(QVariant::nameToType("QUrl") == QVariant::Url);
   REQUIRE(QVariant::nameToType("QUuid") == QVariant::Uuid);

   REQUIRE(QVariant::nameToType("QBitmap") == QVariant::Bitmap);
   REQUIRE(QVariant::nameToType("QBrush") == QVariant::Brush);
   REQUIRE(QVariant::nameToType("QColor") == QVariant::Color);
   REQUIRE(QVariant::nameToType("QCursor") == QVariant::Cursor);
   REQUIRE(QVariant::nameToType("QFont") == QVariant::Font);
   REQUIRE(QVariant::nameToType("QIcon") == QVariant::Icon);
   REQUIRE(QVariant::nameToType("QImage") == QVariant::Image);
   REQUIRE(QVariant::nameToType("QKeySequence") == QVariant::KeySequence);
   REQUIRE(QVariant::nameToType("QMatrix") == QVariant::Matrix);
   REQUIRE(QVariant::nameToType("QMatrix4x4") == QVariant::Matrix4x4);
   REQUIRE(QVariant::nameToType("QPalette") == QVariant::Palette);
   REQUIRE(QVariant::nameToType("QPen") == QVariant::Pen);
   REQUIRE(QVariant::nameToType("QPixmap") == QVariant::Pixmap);
   REQUIRE(QVariant::nameToType("QQuaternion") == QVariant::Quaternion);
   REQUIRE(QVariant::nameToType("QRegion") == QVariant::Region);
   REQUIRE(QVariant::nameToType("QSizePolicy") == QVariant::SizePolicy);
   REQUIRE(QVariant::nameToType("QTextLength") == QVariant::TextLength);
   REQUIRE(QVariant::nameToType("QTextFormat") == QVariant::TextFormat);
   REQUIRE(QVariant::nameToType("QTransform") == QVariant::Transform);
   REQUIRE(QVariant::nameToType("QVector2D") == QVariant::Vector2D);
   REQUIRE(QVariant::nameToType("QVector3D") == QVariant::Vector3D);
   REQUIRE(QVariant::nameToType("QVector4D") == QVariant::Vector4D);

   REQUIRE(QVariant::nameToType("QVariant") == QVariant::Variant);
}

TEST_CASE("QVariant enum_to_variant", "[qvariant]")
{
   // register enums
   Qt::staticMetaObject();

   QVariant data;

   {
      data = QVariant::fromValue(Qt::WheelFocus);

      REQUIRE(data.isValid());
      REQUIRE(data.type() == QVariant::UserType);

      REQUIRE(data.value<Qt::FocusPolicy>() == Qt::WheelFocus);
      REQUIRE(data.value<int>() == Qt::WheelFocus);
      REQUIRE(data.toInt() == Qt::WheelFocus);

      REQUIRE(data.toInt() != Qt::StrongFocus);

      REQUIRE(data.typeName() == "Qt::FocusPolicy");      // name of the enum
   }

   {
      data = QVariant::fromValue(Qt::NoFocus);

      REQUIRE(data.value<Qt::FocusPolicy>() == Qt::NoFocus);
      REQUIRE(data.value<int>() == Qt::NoFocus);
      REQUIRE(data.toInt() == Qt::Qt::NoFocus);

      REQUIRE(data.toInt() != Qt::StrongFocus);

      REQUIRE(data.typeName() == "Qt::FocusPolicy");   // name of the enum
   }
}

TEST_CASE("QVariant flag_to_variant", "[qvariant]")
{
   Qt::staticMetaObject();

   QVariant data = QVariant::fromValue(Qt::AlignLeft | Qt::AlignTop);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::UserType);

   REQUIRE(data.value<Qt::Alignment>() == Qt::Alignment(Qt::AlignLeft | Qt::AlignTop));
   REQUIRE(data.value<int>() == int(Qt::AlignLeft | Qt::AlignTop));
   REQUIRE(data.toInt() == int(Qt::AlignLeft | Qt::AlignTop));

   REQUIRE(data.typeName() == "Qt::Alignment");      // name of the flag
}

TEST_CASE("QVariant type_char8_t", "[qvariant]")
{
   REQUIRE(QVariant::typeToName(QVariant::Char8_t) == "char8_t");
   REQUIRE(QVariant::nameToType("char8_t") == QVariant::Char8_t);
}

TEST_CASE("QVariant comparison_string8", "[qvariant]")
{
   QVariant data1 = 5;
   QVariant data2 = QString("5");

   REQUIRE(data1 == data2);
}

TEST_CASE("QVariant comparison_string16", "[qvariant]")
{
   QVariant data1 = 5;
   QVariant data2 = QString16("5");

   REQUIRE(data1 == data2);
}

TEST_CASE("QVariant move_assign", "[qvariant]")
{
   QVariant data_a = QString("apple");
   QVariant data_b(std::move(data_a));

   REQUIRE(data_b.isValid() == true);
   REQUIRE(data_b == QVariant{QString("apple")});

   //
   QVariant data_c;
   data_c = std::move(data_b);

   REQUIRE(data_c.isValid() == true);
   REQUIRE(data_c == QVariant{QString("apple")});
}

TEST_CASE("QVariant move_operations", "[qvariant]")
{
   // emerald
}

TEST_CASE("QVariant get_data", "[qvariant]")
{
   // emerald
}

TEST_CASE("QVariant get_data_or", "[qvariant]")
{
   // emerald
}
