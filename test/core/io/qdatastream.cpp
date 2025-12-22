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

#include <qdatastream.h>

#include <qbuffer.h>
#include <qdate.h>
#include <qfile.h>
#include <qlocale.h>
#include <qmargins.h>
#include <qpoint.h>
#include <qtimezone.h>
#include <qurl.h>

#include <cs_catch2.h>

TEST_CASE("QDataStream traits", "[qdatastream]")
{
   REQUIRE(std::is_copy_constructible_v<QDataStream> == false);
   REQUIRE(std::is_move_constructible_v<QDataStream> == false);

   REQUIRE(std::is_copy_assignable_v<QDataStream> == false);
   REQUIRE(std::is_move_assignable_v<QDataStream> == false);

   REQUIRE(std::has_virtual_destructor_v<QDataStream> == true);
}

TEST_CASE("QDataStream byte_order", "[qdatastream]")
{
   QByteArray data;

   QBuffer buffer(&data);
   buffer.open(QIODevice::ReadWrite);

   QDataStream ds(&buffer);

   // default byte order
   REQUIRE(ds.byteOrder() == QDataStream::BigEndian);

   // little endian test
   ds.setByteOrder(QDataStream::LittleEndian);

   quint32 value = 0x12345678;
   ds << value;

   ds.device()->seek(0);

   quint32 result = 0;
   ds >> result;

   REQUIRE(result == value);
}

TEST_CASE("QDataStream constructor", "[qdatastream]")
{
   QByteArray data;

   //
   QDataStream ds_1;

   REQUIRE(ds_1.device() == nullptr);
   REQUIRE(ds_1.status() == QDataStream::Ok);

   //
   QBuffer buffer(&data);
   buffer.open(QIODevice::ReadWrite);

   QDataStream ds_2(&buffer);

   REQUIRE(ds_2.device() == &buffer);
   REQUIRE(ds_2.status() == QDataStream::Ok);
}

TEST_CASE("QDataStream containers", "[qdatastream]")
{
   QByteArray data;

   QBuffer buffer(&data);
   buffer.open(QIODevice::ReadWrite);

   QDataStream ds(&buffer);

   QList<int> list {1, 2, 3, 4};
   QSet<QString> set {"alpha", "beta", "gamma"};
   QMap<QString, int> map { {"one", 1}, {"two", 2}, {"three", 3} };

   ds << list << set << map;
   ds.device()->seek(0);

   //
   QList<int> result_list;
   QSet<QString> result_set;
   QMap<QString, int> result_map;

   ds >> result_list >> result_set >> result_map;

   REQUIRE(result_list == list);
   REQUIRE(result_set  == set);
   REQUIRE(result_map  == map);
}

TEST_CASE("QDataStream empty_status", "[qdatastream]")
{
   QByteArray data;

   QBuffer buffer(&data);
   buffer.open(QIODevice::ReadOnly);

   QDataStream ds(&buffer);

   qint32 value;
   ds >> value;

   // reading an empty stream
   REQUIRE(ds.status() == QDataStream::ReadPastEnd);
}

TEST_CASE("QDataStream floatingPoint_a", "[qdatastream]")
{
   QByteArray data;

   QBuffer buffer(&data);
   buffer.open(QIODevice::ReadWrite);

   QDataStream ds(&buffer);

   // single precision
   ds.setFloatingPointPrecision(QDataStream::SinglePrecision);

   float f = 3.1415926f;

   ds << f;
   ds.device()->seek(0);

   float outF = 0.0f;

   ds >> outF;

   REQUIRE_THAT(outF, Catch::Matchers::WithinAbs(f, 0.0001));
}

TEST_CASE("QDataStream floatingPoint_b", "[qdatastream]")
{
   QByteArray data;

   QBuffer buffer(&data);
   buffer.open(QIODevice::ReadWrite);

   QDataStream ds(&buffer);

   // double precision
   ds.setFloatingPointPrecision(QDataStream::DoublePrecision);

   double d = 3.141592653589793;

   ds << d;
   ds.device()->seek(0);

   double outD = 0.0;
   ds >> outD;

   REQUIRE_THAT(outD, Catch::Matchers::WithinAbs(d, 0.0001));
}

TEST_CASE("QDataStream margins", "[qdatastream]")
{
   QByteArray buffer;

   {
      // stream out
      QMargins marginsOut(1000, -20000, 30000, 7000);
      QDataStream streamOut(&buffer, QIODevice::WriteOnly);

      streamOut << marginsOut;
   }

   {
      // stream in
      QMargins marginsIn;
      QDataStream streamIn(&buffer, QIODevice::ReadOnly);

      streamIn >> marginsIn;

      REQUIRE(marginsIn.left() == 1000);
      REQUIRE(marginsIn.top() == -20000);
      REQUIRE(marginsIn.right() == 30000);
      REQUIRE(marginsIn.bottom() == 7000);
   }
}

TEST_CASE("QDataStream marginsf", "[qdatastream]")
{
   QByteArray buffer;

   {
      // stream out
      QMarginsF marginsOut(1000, -20000, 30000, 7000);
      QDataStream streamOut(&buffer, QIODevice::WriteOnly);

      streamOut << marginsOut;
   }

   {
      // stream in
      QMarginsF marginsIn;
      QDataStream streamIn(&buffer, QIODevice::ReadOnly);

      streamIn >> marginsIn;

      REQUIRE(marginsIn.left() == 1000);
      REQUIRE(marginsIn.top() == -20000);
      REQUIRE(marginsIn.right() == 30000);
      REQUIRE(marginsIn.bottom() == 7000);
   }
}

TEST_CASE("QDataStream point", "[qdatastream]")
{
   QPoint data1(-10000, 30000);

   QByteArray buffer;

   //
   QDataStream streamOut(&buffer, QIODevice::WriteOnly);
   streamOut << data1;

   //
   QPoint data2;

   QDataStream streamIn(&buffer, QIODevice::ReadOnly);
   streamIn >> data2;

   REQUIRE(data1 == data2);
}

TEST_CASE("QDataStream pointf", "[qdatastream]")
{
   QPointF data1(-10000.50, 30000.50);

   QByteArray buffer;

   //
   QDataStream streamOut(&buffer, QIODevice::WriteOnly);
   streamOut << data1;

   //
   QPointF data2;

   QDataStream streamIn(&buffer, QIODevice::ReadOnly);
   streamIn >> data2;

   REQUIRE(data1 == data2);
}

TEST_CASE("QDataStream primitive_types", "[qdatastream]")
{
   QByteArray data;

   QBuffer buffer(&data);
   buffer.open(QIODevice::ReadWrite);

   QDataStream ds(&buffer);

   bool    b      = true;
   qint8   int8   = -42;
   quint8  uint8  = 42;
   qint16  int16  = -32000;
   quint16 uint16 = 65000;
   qint32  int32  = -2000000000;
   quint32 uint32 = 4000000000U;
   qint64  int64  = -9000000000000000000LL;
   quint64 uint64 = 18000000000000000000ULL;

   ds << b << int8 << uint8 << int16 << uint16 << int32 << uint32 << int64 << uint64;
   ds.device()->seek(0);

   bool    result_b;
   qint8   result_int8;
   quint8  result_uint8;
   qint16  result_int16;
   quint16 result_uint16;
   qint32  result_int32;
   quint32 result_uint32;
   qint64  result_int64;
   quint64 result_uint64;

   ds >> result_b >> result_int8 >> result_uint8 >> result_int16 >> result_uint16
      >> result_int32 >> result_uint32 >> result_int64 >> result_uint64;

   REQUIRE(result_b      == b);
   REQUIRE(result_int8   == int8);
   REQUIRE(result_uint8  == uint8);
   REQUIRE(result_int16  == int16);
   REQUIRE(result_uint16 == uint16);
   REQUIRE(result_int32  == int32);
   REQUIRE(result_uint32 == uint32);
   REQUIRE(result_int64  == int64);
   REQUIRE(result_uint64 == uint64);
}

TEST_CASE("QDataStream rect", "[qdatastream]")
{
   QRect data1(5, 6, 7, 8);

   QByteArray buffer;

   //
   QDataStream streamOut(&buffer, QIODevice::WriteOnly);
   streamOut << data1;

   //
   QRect data2;

   QDataStream streamIn(&buffer, QIODevice::ReadOnly);
   streamIn >> data2;

   REQUIRE(data1 == data2);
}

TEST_CASE("QDataStream rectf", "[qdatastream]")
{
   QRectF data1(5.3, 6.7, 7.46, 8.91);

   QByteArray buffer;

   //
   QDataStream streamOut(&buffer, QIODevice::WriteOnly);
   streamOut << data1;

   //
   QRectF data2;

   QDataStream streamIn(&buffer, QIODevice::ReadOnly);
   streamIn >> data2;

   REQUIRE(data1 == data2);
}

TEST_CASE("QDataStream raw_data", "[qdatastream]")
{
   QByteArray data;

   QBuffer buffer(&data);
   buffer.open(QIODevice::ReadWrite);

   QDataStream ds(&buffer);

   QByteArray input = "RAW_BYTES";
   int textLength = input.size();

   ds.writeRawData(input.constData(), textLength);
   ds.device()->seek(0);

   QByteArray output(textLength, '\0');

   int readLength = ds.readRawData(output.data(), output.size());

   REQUIRE(textLength == readLength);
   REQUIRE(input == output);
}

TEST_CASE("QDataStream size", "[qdatastream]")
{
   QSize data1(640, 480);

   QByteArray buffer;

   //
   QDataStream streamOut(&buffer, QIODevice::WriteOnly);
   streamOut << data1;

   //
   QSize data2;

   QDataStream streamIn(&buffer, QIODevice::ReadOnly);
   streamIn >> data2;

   REQUIRE(data1 == data2);
}

TEST_CASE("QDataStream sizef", "[qdatastream]")
{
   QSizeF data1(640.30, 480.92);

   QByteArray buffer;

   //
   QDataStream streamOut(&buffer, QIODevice::WriteOnly);
   streamOut << data1;

   //
   QSizeF data2;

   QDataStream streamIn(&buffer, QIODevice::ReadOnly);
   streamIn >> data2;

   REQUIRE(data1 == data2);
}

TEST_CASE("QDataStream string", "[qdatastream]")
{
   // write
   QByteArray data;

   QDataStream streamOut(&data, QIODevice::WriteOnly);

   QString strOut("On a clear day you see forever");
   streamOut << strOut;

   QDate dateOut(2017, 4, 1);
   streamOut << dateOut;

   QTime timeOut(10, 17, 0);
   streamOut << timeOut;

   double dOut = 5.96;
   streamOut << dOut;

   float fOut = 3.14f;
   streamOut << fOut;

   // read
   QDataStream streamIn(&data, QIODevice::ReadOnly);

   QString strIn;
   streamIn >> strIn;

   QDate dateIn;
   streamIn >> dateIn;

   QTime timeIn;
   streamIn >> timeIn;

   double dIn;
   streamIn >> dIn;

   float fIn;
   streamIn >> fIn;

   REQUIRE(strOut == strIn);
   REQUIRE(dateOut == dateIn);
   REQUIRE(timeOut == timeIn);
   REQUIRE(dOut == dIn);
   REQUIRE(fOut == fIn);
}

TEST_CASE("QDataStream timezone", "[qdatastream]")
{
   QByteArray buffer;

   QTimeZone data1("PST", 123456, "CS Standard Time", "CSST", QLocale::UnitedStates, "Some Test");

   //
   QDataStream streamOut(&buffer, QIODevice::WriteOnly);
   streamOut << data1;

   //
   QTimeZone data2;

   QDataStream streamIn(&buffer, QIODevice::ReadOnly);
   streamIn >> data2;

   REQUIRE(data2.id() == QByteArray("PST") );
   REQUIRE(data2.comment() == "Some Test");
   REQUIRE(data2.country() == QLocale::UnitedStates);

   REQUIRE(data2.displayName(QTimeZone::StandardTime, QTimeZone::LongName, QString()) == QString("CS Standard Time"));
   REQUIRE(data2.displayName(QTimeZone::DaylightTime, QTimeZone::LongName, QString()) == QString("CS Standard Time"));

   REQUIRE(data2.abbreviation(QDateTime::currentDateTime())  == "CSST");
   REQUIRE(data2.offsetFromUtc(QDateTime::currentDateTime()) == 123456);
}

TEST_CASE("QDataStream url_a", "[qdatastream]")
{
   QByteArray buffer;
   QUrl data1("https://copperspice.com/documentation-lib.html");

   QDataStream streamOut(&buffer, QIODevice::WriteOnly);
   streamOut << data1;

   QDataStream streamIn(buffer);

   QUrl data2;
   streamIn >> data2;

   REQUIRE(data1.url() == data2.url());
}

TEST_CASE("QDataStream url_b", "[qdatastream]")
{
   QByteArray buffer;
   QUrl data1("http://[::ffff:8.8.8.8]:81?query");

   QDataStream streamOut(&buffer, QIODevice::WriteOnly);
   streamOut << data1;

   QDataStream streamIn(buffer);

   QUrl data2;
   streamIn >> data2;

   REQUIRE(data1.url() == data2.url());
}

TEST_CASE("QDataStream url_c", "[qdatastream]")
{
   QByteArray buffer;
   QUrl data1("ftp://host/path%25path?%3Fque%25ry#%23ref%25");

   QDataStream streamOut(&buffer, QIODevice::WriteOnly);
   streamOut << data1;

   QDataStream streamIn(buffer);

   QUrl data2;
   streamIn >> data2;

   REQUIRE(data1.url() == data2.url());
}

TEST_CASE("QDataStream qvariant", "[qdatastream]")
{
   QByteArray data;

   QBuffer buffer(&data);
   buffer.open(QIODevice::ReadWrite);

   QDataStream ds(&buffer);

   //
   QVariant v1 = 123;
   QVariant v2 = QString("Variant String");
   QVariant v3 = QPoint(7, 9);

   ds << v1 << v2 << v3;
   ds.device()->seek(0);

   QVariant result_1;
   QVariant result_2;
   QVariant result_3;

   ds >> result_1 >> result_2 >> result_3;

   REQUIRE(result_1 == v1);
   REQUIRE(result_2 == v2);
   REQUIRE(result_3 == v3);
}

TEST_CASE("QDataStream version", "[qdatastream]")
{
   QByteArray data;

   QBuffer buffer(&data);
   buffer.open(QIODevice::ReadWrite);

   QDataStream ds(&buffer);

   ds.setVersion(QDataStream::CS_2_0);
   REQUIRE(ds.version() == QDataStream::CS_2_0);

   qint32 v = 12345;
   ds << v;

   ds.device()->seek(0);

   qint32 out = 0;
   ds >> out;

   REQUIRE(out == v);
}
