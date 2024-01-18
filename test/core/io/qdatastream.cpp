/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#include <qdate.h>
#include <qdatastream.h>
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