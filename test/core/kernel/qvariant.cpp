/***********************************************************************
*
* Copyright (c) 2012-2022 Barbara Geller
* Copyright (c) 2012-2022 Ansel Sermersheim
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

   REQUIRE(std::is_nothrow_move_constructible_v<QVariant> == false);
   REQUIRE(std::is_nothrow_move_assignable_v<QVariant> == false);

   REQUIRE(std::has_virtual_destructor_v<QVariant> == false);
}

TEST_CASE("QVariant empty", "[qvariant]")
{
   QVariant data;

   REQUIRE(! data.isValid());
   REQUIRE(data.type() == QVariant::Invalid);

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);
   REQUIRE(data.toUInt() == 0);
   REQUIRE(data.toLong() == 0);
   REQUIRE(data.toULong() == 0);
   REQUIRE(data.toLongLong() == 0);
   REQUIRE(data.toULongLong() == 0);
   REQUIRE(data.toDouble() == 0);
   REQUIRE(data.toFloat() == 0);
   REQUIRE(data.toReal() == 0);

   REQUIRE(data.toBitArray() == QBitArray());
   REQUIRE(data.toByteArray() == QByteArray());
   REQUIRE(data.toChar() == '\0');
   REQUIRE(data.toString() == QString());
   REQUIRE(data.toString16() == QString16());
   REQUIRE(data.toStringList() == QStringList());

   REQUIRE(data.toDate() == QDate());
   REQUIRE(data.toTime() == QTime());
   REQUIRE(data.toDateTime() == QDateTime());
   REQUIRE(data.toLocale() == QLocale());

   REQUIRE(data.toList() == QList<QVariant>());
   REQUIRE(data.toHash() == QHash<QString, QVariant>());
   REQUIRE(data.toMap() == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash() == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap() == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray() == QJsonArray());
   REQUIRE(data.toJsonDocument() == QJsonDocument());
   REQUIRE(data.toJsonValue() == QJsonValue());
   REQUIRE(data.toJsonObject() == QJsonObject());

   REQUIRE(data.toLine() == QLine());
   REQUIRE(data.toLineF() == QLineF());
   REQUIRE(data.toPoint() == QPoint());
   REQUIRE(data.toPointF() == QPointF());
   REQUIRE(data.toRect() == QRect());
   REQUIRE(data.toRectF() == QRectF());
   REQUIRE(data.toSize() == QSize());
   REQUIRE(data.toSizeF() == QSizeF());

   REQUIRE(data.toEasingCurve() == QEasingCurve());
   REQUIRE(data.toModelIndex() == QModelIndex());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
   REQUIRE(data.toUrl() == QUrl());
   REQUIRE(data.toUuid() == QUuid());
}

TEST_CASE("QVariant constructor_bool", "[qvariant]")
{
   QVariant data = false;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Bool);

   REQUIRE(data.toBool() == false);

   data = true;
   REQUIRE(data.toBool() == true);
   REQUIRE(data.toInt() == 1);
   REQUIRE(data.toUInt() == 1);
   REQUIRE(data.toLong() == 1);
   REQUIRE(data.toULong() == 1);
   REQUIRE(data.toLongLong() == 1);
   REQUIRE(data.toULongLong() == 1);
   REQUIRE(data.toDouble() == 1);
   REQUIRE(data.toFloat() == 1.0f);
   REQUIRE(data.toReal() == 1.0);

   REQUIRE(data.toByteArray() == "true");
   REQUIRE(data.toChar() == '\0');
   REQUIRE(data.toString() == "true");
   REQUIRE(data.toString16() == "true");
}

TEST_CASE("QVariant constructor_int", "[qvariant]")
{
   QVariant data = 17;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Int);

   REQUIRE(data.toBool() == true);
   REQUIRE(data.toInt() == 17);
   REQUIRE(data.toUInt() == 17);
   REQUIRE(data.toLong() == 17);
   REQUIRE(data.toULong() == 17);
   REQUIRE(data.toLongLong() == 17);
   REQUIRE(data.toULongLong() == 17);
   REQUIRE(data.toDouble() == 17.0);
   REQUIRE(data.toFloat() == 17.0f);
   REQUIRE(data.toReal() == 17.0);

   REQUIRE(data.toBitArray() == QBitArray());
   REQUIRE(data.toByteArray() == "17");
   REQUIRE(data.toChar() == '\x11');
   REQUIRE(data.toString() == "17");
   REQUIRE(data.toString16() == "17");
   REQUIRE(data.toStringList() == QStringList());

   REQUIRE(data.toDate() == QDate());
   REQUIRE(data.toTime() == QTime());
   REQUIRE(data.toDateTime() == QDateTime());
   REQUIRE(data.toLocale() == QLocale());

   REQUIRE(data.toList() == QList<QVariant>());
   REQUIRE(data.toHash() == QHash<QString, QVariant>());
   REQUIRE(data.toMap() == QMap<QString, QVariant>());
   REQUIRE(data.toMultiHash() == QMultiHash<QString, QVariant>());
   REQUIRE(data.toMultiMap() == QMultiMap<QString, QVariant>());

   REQUIRE(data.toJsonArray() == QJsonArray());
   REQUIRE(data.toJsonDocument() == QJsonDocument());
   REQUIRE(data.toJsonObject() == QJsonObject());
   REQUIRE(data.toJsonValue() == QJsonValue());

   REQUIRE(data.toLine() == QLine());
   REQUIRE(data.toLineF() == QLineF());
   REQUIRE(data.toPoint() == QPoint());
   REQUIRE(data.toPointF() == QPointF());
   REQUIRE(data.toRect() == QRect());
   REQUIRE(data.toRectF() == QRectF());
   REQUIRE(data.toSize() == QSize());
   REQUIRE(data.toSizeF() == QSizeF());

   REQUIRE(data.toEasingCurve() == QEasingCurve());
   REQUIRE(data.toModelIndex() == QModelIndex());
   REQUIRE(data.toPersistentModelIndex() == QPersistentModelIndex());
   REQUIRE(data.toUrl() == QUrl());
   REQUIRE(data.toUuid() == QUuid());
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

   REQUIRE(data.canConvert<bool>());
   REQUIRE(data.canConvert<int>());
   REQUIRE(data.canConvert<double>());
   REQUIRE(data.canConvert<QString>());
   REQUIRE(data.canConvert<QString16>());

   REQUIRE(data.toBool() == true);
   REQUIRE(data.toInt() == 0);
   REQUIRE(data.toDouble() == 0.0);

   REQUIRE(data.toByteArray() == QByteArray("CopperSpice") + '\0');
   REQUIRE(data.toString() == QString("CopperSpice") + '\0');
   REQUIRE(data.toString16() == QString16("CopperSpice") + '\0');
}

TEST_CASE("QVariant constructor_date", "[qvariant]")
{
   QVariant data = QDate(2021, 4, 1);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Date);

   REQUIRE(data.canConvert<QDate>());
   REQUIRE(! data.canConvert<QTime>());
   REQUIRE(data.canConvert<QDateTime>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(data.toDate() == QDate(2021, 4, 1));
   REQUIRE(data.toTime() == QTime());
   REQUIRE(data.toDateTime() == QDateTime(QDate(2021, 4, 1), QTime()));
   REQUIRE(data.toString() == "2021-04-01");
}

TEST_CASE("QVariant constructor_time", "[qvariant]")
{
   QVariant data = QTime(14, 52, 3);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Time);

   REQUIRE(! data.canConvert<QDate>());
   REQUIRE(data.canConvert<QTime>());
   REQUIRE(! data.canConvert<QDateTime>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(data.toDate() == QDate());
   REQUIRE(data.toTime() == QTime(14, 52, 3));
   REQUIRE(data.toDateTime() == QDateTime());
   REQUIRE(data.toString() == "14:52:03");
}

TEST_CASE("QVariant constructor_datetime", "[qvariant]")
{
   QVariant data = QDateTime(QDate(2021, 4, 1), QTime(14, 52, 3));

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::DateTime);

   REQUIRE(data.canConvert<QDate>());
   REQUIRE(data.canConvert<QTime>());
   REQUIRE(data.canConvert<QDateTime>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(data.toDate() == QDate(2021, 04, 01));
   REQUIRE(data.toTime() == QTime(14, 52, 3));
   REQUIRE(data.toDateTime() == QDateTime(QDate(2021, 4, 1), QTime(14, 52, 3)));
   REQUIRE(data.toString() == "2021-04-01T14:52:03");
}

TEST_CASE("QVariant constructor_json_value", "[qvariant]")
{
   {
      QJsonValue value = QJsonValue(9.37);
      QVariant data = value;

      REQUIRE(data.isValid());
      REQUIRE(data.type() == QVariant::JsonValue);

      REQUIRE(data.canConvert<bool>());
      REQUIRE(data.canConvert<double>());
      REQUIRE(data.canConvert<int>());

      REQUIRE(data.canConvert<QString>());
      REQUIRE(data.canConvert<QString16>());

      REQUIRE(data.toBool() == true);
      REQUIRE(data.toDouble() == 9.37);
      REQUIRE(data.toInt() == 9);

      REQUIRE(data.toString() == QString("9.37"));
      REQUIRE(data.toString16() == QString16("9.37"));

      REQUIRE(data.toUrl() == QUrl());
      REQUIRE(data.toUuid() == QUuid());
   }

   {
      QJsonValue value = QJsonValue(QString("CopperSpice"));
      QVariant data = value;

      REQUIRE(data.isValid());
      REQUIRE(data.type() == QVariant::JsonValue);

      REQUIRE(data.canConvert<bool>());
      REQUIRE(data.canConvert<double>());
      REQUIRE(data.canConvert<int>());

      REQUIRE(data.canConvert<QString>());
      REQUIRE(data.canConvert<QString16>());

      REQUIRE(data.toBool() == true);
      REQUIRE(data.toDouble() == 0);
      REQUIRE(data.toInt() == 0);

      REQUIRE(data.toString() == QString("CopperSpice"));
      REQUIRE(data.toString16() == QString16("CopperSpice"));

      REQUIRE(data.toUrl() == QUrl());
      REQUIRE(data.toUuid() == QUuid());
   }
}

TEST_CASE("QVariant constructor_line", "[qvariant]")
{
   QVariant data = QLine(6, 12, 0, 3);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Line);

   REQUIRE(data.canConvert<QLineF>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(data.toLine() == QLine(6, 12, 0, 3));
   REQUIRE(data.value<QLine>() == QLine(6, 12, 0, 3));

   REQUIRE(data.toLineF() == QLineF(6, 12, 0, 3));
   REQUIRE(data.value<QLineF>() == QLineF(6, 12, 0, 3));
}

TEST_CASE("QVariant constructor_linef", "[qvariant]")
{
   QVariant data = QLineF(6.4, 12.8, 0, 3.2);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::LineF);

   REQUIRE(data.canConvert<QLine>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(data.toLine() == QLine(6, 13, 0, 3));
   REQUIRE(data.value<QLine>() == QLine(6, 13, 0, 3));

   REQUIRE(data.toLineF() == QLineF(6.4, 12.8, 0, 3.2));
   REQUIRE(data.value<QLineF>() == QLineF(6.4, 12.8, 0, 3.2));
}

TEST_CASE("QVariant constructor_list", "[qvariant]")
{
   QList<QVariant> list1 = {QString("orange"), 17};

   QVariant data      = list1;
   QVariantList list2 = data.value<QVariantList>();

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::List);

   REQUIRE(! data.canConvert<QString>());
   REQUIRE(data.canConvert<QStringList>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(list1 == list2);

   REQUIRE(list2[0] == QString("orange"));
   REQUIRE(list2[1] == 17);
}

TEST_CASE("QVariant constructor_hash", "[qvariant]")
{
   QHash<QString, QVariant> hash1 = { {"orange", 17} };

   QVariant data      = hash1;
   QVariantHash hash2 = data.value<QVariantHash>();

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Hash);

   REQUIRE(data.canConvert<QVariantMap>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(hash1 == hash2);
   REQUIRE(hash2.value("orange").toInt() == 17);

   QVariantHash hash3 = data.toHash();
   REQUIRE(hash1 == hash3);
   REQUIRE(hash3.value("orange").toInt() == 17);
}

TEST_CASE("QVariant constructor_map", "[qvariant]")
{
   QMap<QString, QVariant> map1 = { {"orange", 17} };

   QVariant data    = map1;
   QVariantMap map2 = data.value<QVariantMap>();

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Map);

   REQUIRE(data.canConvert<QVariantHash>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(map1 == map2);
   REQUIRE(map2.value("orange").toInt() == 17);

   QVariantMap map3 = data.toMap();
   REQUIRE(map1 == map3);
   REQUIRE(map3.value("orange").toInt() == 17);
}

TEST_CASE("QVariant constructor_point", "[qvariant]")
{
   QVariant data = QPoint(17, 42);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Point);

   REQUIRE(data.canConvert<QPointF>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(data.toPoint() == QPoint(17, 42));
   REQUIRE(data.value<QPoint>() == QPoint(17, 42));

   REQUIRE(data.toPointF() == QPointF(17, 42));
   REQUIRE(data.value<QPointF>() == QPointF(17, 42));
}

TEST_CASE("QVariant constructor_pointf", "[qvariant]")
{
   QVariant data = QPointF(17.9, 42.0);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::PointF);

   REQUIRE(data.canConvert<QPoint>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(data.toPoint() == QPoint(18, 42));
   REQUIRE(data.value<QPoint>() == QPoint(18, 42));

   REQUIRE(data.toPointF() == QPointF(17.9, 42.0));
   REQUIRE(data.value<QPointF>() == QPointF(17.9, 42.0));
}

TEST_CASE("QVariant constructor_rect", "[qvariant]")
{
   QVariant data = QRect(9, 12, 18, 7);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Rect);

   REQUIRE(data.canConvert<QRectF>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(data.toRect() == QRect(9, 12, 18, 7));
   REQUIRE(data.value<QRect>() == QRect(9, 12, 18, 7));

   REQUIRE(data.toRectF() == QRectF(9, 12, 18, 7));
   REQUIRE(data.value<QRectF>() == QRectF(9, 12, 18, 7));
}

TEST_CASE("QVariant constructor_rectf", "[qvariant]")
{
   QVariant data = QRectF(9.7, 12.1, 18.0, 7.4);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::RectF);

   REQUIRE(data.canConvert<QRect>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(data.toRect() == QRect(10, 12, 18, 7));
   REQUIRE(data.value<QRect>() == QRect(10, 12, 18, 7));

   REQUIRE(data.toRectF() == QRectF(9.7, 12.1, 18.0, 7.4));
   REQUIRE(data.value<QRectF>() == QRectF(9.7, 12.1, 18.0, 7.4));
}

TEST_CASE("QVariant constructor_size", "[qvariant]")
{
   QVariant data = QSize(9, 12);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Size);

   REQUIRE(data.canConvert<QSizeF>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(data.toSize() == QSize(9, 12));
   REQUIRE(data.value<QSize>() == QSize(9, 12));

   REQUIRE(data.toSizeF() == QSizeF(9, 12));
   REQUIRE(data.value<QSizeF>() == QSizeF(9, 12));
}

TEST_CASE("QVariant constructor_sizef", "[qvariant]")
{
   QVariant data = QSizeF(9.7, 12.1);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::SizeF);

   REQUIRE(data.canConvert<QSize>());

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(data.toSize() == QSize(10, 12));
   REQUIRE(data.value<QSize>() == QSize(10, 12));

   REQUIRE(data.toSizeF() == QSizeF(9.7, 12.1));
   REQUIRE(data.value<QSizeF>() == QSizeF(9.7, 12.1));
}

TEST_CASE("QVariant constructor_string8", "[qvariant]")
{
   QVariant data = QString("apple");

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::String);

   REQUIRE(data.canConvert<QString16>());

   REQUIRE(data.toBool() == true);
   REQUIRE(data.toInt() == 0);
   REQUIRE(data.toUInt() == 0);
   REQUIRE(data.toLong() == 0);
   REQUIRE(data.toULong() == 0);
   REQUIRE(data.toLongLong() == 0);
   REQUIRE(data.toULongLong() == 0);
   REQUIRE(data.toDouble() == 0.0);
   REQUIRE(data.toFloat() == 0.0f);
   REQUIRE(data.toReal() == 0.0);

   REQUIRE(data.toByteArray() == "apple");
   REQUIRE(data.toChar() == '\0');
   REQUIRE(data.toString() == "apple");
   REQUIRE(data.toString16() == "apple");
   REQUIRE(data.toStringList() == QStringList("apple"));

   REQUIRE(data.toDate() == QDate());
   REQUIRE(data.toTime() == QTime());
   REQUIRE(data.toDateTime() == QDateTime());
   REQUIRE(data.toLocale() == QLocale());

   REQUIRE(data.toUrl() == QUrl("apple"));
   REQUIRE(data.toUuid() == QUuid("apple"));

   // another test
   data = QString("53");

   REQUIRE(data.toChar() == '5');

   REQUIRE(data.toInt() == 53);
   REQUIRE(data.toUInt() == 53);
   REQUIRE(data.toLong() == 53);
   REQUIRE(data.toULong() == 53);
   REQUIRE(data.toLongLong() == 53);
   REQUIRE(data.toULongLong() == 53);
   REQUIRE(data.toDouble() == 53.0);
   REQUIRE(data.toFloat() == 53.0f);
   REQUIRE(data.toReal() == 53.0);
}

TEST_CASE("QVariant constructor_string16", "[qvariant]")
{
   QVariant data = QString16("apple");

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::String16);

   REQUIRE(data.canConvert<QString>());

   REQUIRE(data.toBool() == true);
   REQUIRE(data.toInt() == 0);
   REQUIRE(data.toUInt() == 0);
   REQUIRE(data.toLong() == 0);
   REQUIRE(data.toULong() == 0);
   REQUIRE(data.toLongLong() == 0);
   REQUIRE(data.toULongLong() == 0);
   REQUIRE(data.toDouble() == 0.0);
   REQUIRE(data.toFloat() == 0.0f);
   REQUIRE(data.toReal() == 0.0);

   REQUIRE(data.toByteArray() == "apple");
   REQUIRE(data.toChar() == '\0');
   REQUIRE(data.toString() == "apple");
   REQUIRE(data.toString16() == "apple");
   REQUIRE(data.toStringList() == QStringList("apple"));

   REQUIRE(data.toDate() == QDate());
   REQUIRE(data.toTime() == QTime());
   REQUIRE(data.toDateTime() == QDateTime());
   REQUIRE(data.toLocale() == QLocale());

   REQUIRE(data.toUrl() == QUrl("apple"));
   REQUIRE(data.toUuid() == QUuid("apple"));

   // another test
   data = QString("53");

   REQUIRE(data.toChar() == '5');

   REQUIRE(data.toInt() == 53);
   REQUIRE(data.toUInt() == 53);
   REQUIRE(data.toLong() == 53);
   REQUIRE(data.toULong() == 53);
   REQUIRE(data.toLongLong() == 53);
   REQUIRE(data.toULongLong() == 53);
   REQUIRE(data.toDouble() == 53.0);
   REQUIRE(data.toFloat() == 53.0f);
   REQUIRE(data.toReal() == 53.0);
}

TEST_CASE("QVariant constructor_stringlist", "[qvariant]")
{
   QStringList list;
   list.append("apple");

   QVariant data = list;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::StringList);

   REQUIRE(data.toBool() == false);
   REQUIRE(data.toInt() == 0);

   REQUIRE(data.toString() == "apple");
   REQUIRE(data.toString16() == "apple");

   REQUIRE(data.toDate() == QDate());
   REQUIRE(data.toTime() == QTime());
   REQUIRE(data.toDateTime() == QDateTime());
   REQUIRE(data.toLocale() == QLocale());
}

TEST_CASE("QVariant constructor_url", "[qvariant]")
{
   QString str("https://www.copperspice.com");
   QUrl url(str);

   QVariant data1(str);
   QVariant data2(url);

   REQUIRE(data1 == data2);

   //
   REQUIRE(data1.isValid());
   REQUIRE(data1.type() == QVariant::String);

   REQUIRE(data1.canConvert<QUrl>());

   REQUIRE(data1.toBool() == true);
   REQUIRE(data1.toInt() == 0);

   REQUIRE(data1.toDate() == QDate());
   REQUIRE(data1.toTime() == QTime());
   REQUIRE(data1.toDateTime() == QDateTime());
   REQUIRE(data1.toLocale() == QLocale());

   REQUIRE(data1.toUrl() == url);

   //
   REQUIRE(data2.isValid());
   REQUIRE(data2.type() == QVariant::Url);

   REQUIRE(data2.canConvert<QString8>());
   REQUIRE(data2.canConvert<QString16>());

   REQUIRE(data2.toBool() == false);
   REQUIRE(data2.toInt() == 0);

   REQUIRE(data2.toDate() == QDate());
   REQUIRE(data2.toTime() == QTime());
   REQUIRE(data2.toDateTime() == QDateTime());
   REQUIRE(data2.toLocale() == QLocale());

   REQUIRE(data2.toString() == str);
   REQUIRE(data2.toString16() == str);
}

TEST_CASE("QVariant constructor_uuid", "[qvariant]")
{
   QUuid value("{ba80a7c0-d463-e361-78eb-1394049152ba}");
   QVariant data = value;

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::Uuid);

   REQUIRE(data.canConvert<QString>());

   REQUIRE(data.toBitArray()   == QBitArray());
   REQUIRE(data.toByteArray()  == "{ba80a7c0-d463-e361-78eb-1394049152ba}");
   REQUIRE(data.toChar()       == '\0');
   REQUIRE(data.toString()     == "{ba80a7c0-d463-e361-78eb-1394049152ba}");
   REQUIRE(data.toString16()   == "{ba80a7c0-d463-e361-78eb-1394049152ba}");
   REQUIRE(data.toStringList() == QStringList());

   REQUIRE(data.toDate() == QDate());
   REQUIRE(data.toTime() == QTime());
   REQUIRE(data.toDateTime() == QDateTime());
   REQUIRE(data.toLocale() == QLocale());
}

struct MyCustomType
{
   QString dataStr;
   QList<int> dataList;
};

CS_DECLARE_METATYPE(MyCustomType);

TEST_CASE("QVariant constructor_user_type", "[qvariant]")
{
   MyCustomType input = {"On a clear day", {8, 17} };

   QVariant data1 = QVariant::fromValue(input);
   QVariant data2 = data1;

   REQUIRE(data2.isValid());
   REQUIRE(data1.userType() == QVariant::typeToTypeId<MyCustomType>());
   REQUIRE(data2.typeName() == "MyCustomType");

   REQUIRE(! data2.canConvert(QVariant::Date));

   MyCustomType data3 = data2.value<MyCustomType>();

   REQUIRE(data3.dataStr == "On a clear day");
   REQUIRE(data3.dataList == QList<int>{8, 17});
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

TEST_CASE("QVariant can_convert_string8", "[qvariant]")
{
   QVariant data = QString("apple");

   REQUIRE(! data.canConvert(QVariant::Invalid));

   REQUIRE(data.canConvert(QVariant::Bool));
   REQUIRE(data.canConvert(QVariant::Double));
   REQUIRE(data.canConvert(QVariant::Float));
   REQUIRE(data.canConvert(QVariant::Int));
   REQUIRE(data.canConvert(QVariant::Long));
   REQUIRE(data.canConvert(QVariant::LongLong));
   REQUIRE(data.canConvert(QVariant::Short));
   REQUIRE(data.canConvert(QVariant::UInt));
   REQUIRE(data.canConvert(QVariant::ULong));
   REQUIRE(data.canConvert(QVariant::ULongLong));
   REQUIRE(data.canConvert(QVariant::UShort));

   REQUIRE(data.canConvert(QVariant::QChar));
   REQUIRE(data.canConvert(QVariant::Char));
   REQUIRE(data.canConvert(QVariant::SChar));
   REQUIRE(data.canConvert(QVariant::UChar));
   REQUIRE(! data.canConvert(QVariant::Char8_t));
   REQUIRE(! data.canConvert(QVariant::Char16_t));
   REQUIRE(! data.canConvert(QVariant::Char32_t));

   REQUIRE(data.canConvert(QVariant::ByteArray));
   REQUIRE(! data.canConvert(QVariant::BitArray));
   REQUIRE(data.canConvert(QVariant::String));
   REQUIRE(data.canConvert(QVariant::String16));
   REQUIRE(data.canConvert(QVariant::StringList));
   REQUIRE(! data.canConvert(QVariant::StringView));

   REQUIRE(data.canConvert(QVariant::Date));
   REQUIRE(data.canConvert(QVariant::DateTime));
   REQUIRE(data.canConvert(QVariant::Time));
   REQUIRE(! data.canConvert(QVariant::Locale));

   REQUIRE(! data.canConvert(QVariant::JsonArray));
   REQUIRE(! data.canConvert(QVariant::JsonDocument));
   REQUIRE(! data.canConvert(QVariant::JsonObject));
   REQUIRE(! data.canConvert(QVariant::JsonValue));

   REQUIRE(! data.canConvert(QVariant::Line));
   REQUIRE(! data.canConvert(QVariant::LineF));
   REQUIRE(! data.canConvert(QVariant::Point));
   REQUIRE(! data.canConvert(QVariant::PointF));
   REQUIRE(! data.canConvert(QVariant::Polygon));
   REQUIRE(! data.canConvert(QVariant::PolygonF));
   REQUIRE(! data.canConvert(QVariant::Rect));
   REQUIRE(! data.canConvert(QVariant::RectF));
   REQUIRE(! data.canConvert(QVariant::Size));
   REQUIRE(! data.canConvert(QVariant::SizeF));

   REQUIRE(! data.canConvert(QVariant::List));
   REQUIRE(! data.canConvert(QVariant::Hash));
   REQUIRE(! data.canConvert(QVariant::MultiHash));
   REQUIRE(! data.canConvert(QVariant::Map));
   REQUIRE(! data.canConvert(QVariant::MultiMap));

   REQUIRE(! data.canConvert(QVariant::Void));
   REQUIRE(! data.canConvert(QVariant::VoidStar));
   REQUIRE(! data.canConvert(QVariant::ObjectStar));
   REQUIRE(! data.canConvert(QVariant::WidgetStar));

   REQUIRE(! data.canConvert(QVariant::EasingCurve));
   REQUIRE(! data.canConvert(QVariant::ModelIndex));
   REQUIRE(! data.canConvert(QVariant::PersistentModelIndex));
   REQUIRE(data.canConvert(QVariant::Url));
   REQUIRE(data.canConvert(QVariant::Uuid));

   REQUIRE(! data.canConvert(QVariant::Bitmap));
   REQUIRE(! data.canConvert(QVariant::Brush));
   REQUIRE(data.canConvert(QVariant::Color));
   REQUIRE(! data.canConvert(QVariant::Cursor));
   REQUIRE(data.canConvert(QVariant::Font));
   REQUIRE(! data.canConvert(QVariant::Icon));
   REQUIRE(! data.canConvert(QVariant::Image));
   REQUIRE(data.canConvert(QVariant::KeySequence));
   REQUIRE(! data.canConvert(QVariant::Matrix));
   REQUIRE(! data.canConvert(QVariant::Matrix4x4));
   REQUIRE(! data.canConvert(QVariant::Palette));
   REQUIRE(! data.canConvert(QVariant::Pen));
   REQUIRE(! data.canConvert(QVariant::Pixmap));
   REQUIRE(! data.canConvert(QVariant::Quaternion));
   REQUIRE(! data.canConvert(QVariant::Region));
   REQUIRE(! data.canConvert(QVariant::RegularExpression));
   REQUIRE(! data.canConvert(QVariant::SizePolicy));
   REQUIRE(! data.canConvert(QVariant::TextFormat));
   REQUIRE(! data.canConvert(QVariant::TextLength));
   REQUIRE(! data.canConvert(QVariant::Transform));
   REQUIRE(! data.canConvert(QVariant::Variant));
   REQUIRE(! data.canConvert(QVariant::Vector2D));
   REQUIRE(! data.canConvert(QVariant::Vector3D));
   REQUIRE(! data.canConvert(QVariant::Vector4D));
}

TEST_CASE("QVariant can_convert_string16", "[qvariant]")
{
   QVariant data = QString16("apple");

    REQUIRE(! data.canConvert(QVariant::Invalid));
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

   QVariant data = QVariant::fromValue(Qt::WheelFocus);

   REQUIRE(data.isValid());
   REQUIRE(data.type() == QVariant::UserType);

   REQUIRE(data.value<Qt::FocusPolicy>() == Qt::WheelFocus);
   REQUIRE(data.value<int>() == Qt::WheelFocus);
   REQUIRE(data.toInt() == Qt::WheelFocus);

   REQUIRE(data.toInt() != Qt::StrongFocus);

   REQUIRE(data.typeName() == "Qt::FocusPolicy");      // name of the enum

   {
      QVariant data = QVariant::fromValue(Qt::NoFocus);

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

TEST_CASE("QVariant type_char8_t", "[qvariant] [!mayfail]")
{
#if defined(__cpp_char8_t)
   // test one
   REQUIRE(QVariant::typeToName(QVariant::Char8_t) == "char8_t");

   // test two
   REQUIRE(QVariant::nameToType("char8_t") == QVariant::Char8_t);

#else
   // printf("\nC++20 mode not enabled, char8_t checks omitted\n");

#endif
}

TEST_CASE("QVariant equality_string8", "[qvariant]")
{
   QVariant data1 = 5;
   QVariant data2 = QString("5");

   REQUIRE(data1 == data2);
}

TEST_CASE("QVariant equality_string16", "[qvariant]")
{
   QVariant data1 = 5;
   QVariant data2 = QString16("5");

   REQUIRE(data1 == data2);
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
