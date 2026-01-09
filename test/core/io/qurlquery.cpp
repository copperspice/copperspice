/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <qurl.h>
#include <qurlquery.h>

#include <cs_catch2.h>

TEST_CASE("QUrlQuery traits", "[qurlquery]")
{
   REQUIRE(std::is_copy_constructible_v<QUrlQuery> == true);
   REQUIRE(std::is_move_constructible_v<QUrlQuery> == true);

   REQUIRE(std::is_copy_assignable_v<QUrlQuery> == true);
   REQUIRE(std::is_move_assignable_v<QUrlQuery> == true);

   REQUIRE(std::has_virtual_destructor_v<QUrlQuery> == false);
}

TEST_CASE("QUrlQuery addQueryItem", "[qurlquery]")
{
   QUrlQuery data;

   data.addQueryItem("a", "1");
   data.addQueryItem("b", "2");

   REQUIRE(data.queryItemValue("a") == "1");
   REQUIRE(data.queryItemValue("b") == "2");

   // multiple values
   data.addQueryItem("c", "1");
   data.addQueryItem("c", "2");

   REQUIRE(data.allQueryItemValues("c") == QList<QString>({"1","2"}));

   // empty key with an empty value
   data.addQueryItem("d", "");

   REQUIRE(data.queryItemValue("d") == "");

   // url encoded
   data.clear();
   data.addQueryItem("e", "f g");

   REQUIRE(data.query(QUrl::FullyEncoded) == "e=f%20g");

   //
   data.clear();
   data.addQueryItem("width", "inches");

   REQUIRE(data.query().contains("width=inches") == true);
}

TEST_CASE("QUrlQuery clear", "[qurlquery]")
{
   QUrlQuery data("width=1&height=2");

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.queryItems().isEmpty() == false);
   REQUIRE(data.query().isEmpty() == false);

   REQUIRE(data.query() == "width=1&height=2");

   REQUIRE(data.hasQueryItem("width")  == true);
   REQUIRE(data.hasQueryItem("height") == true);

   REQUIRE(data.queryItemValue("width")  == "1");
   REQUIRE(data.queryItemValue("height") == "2");

   //
   data.clear();

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.queryItems().isEmpty() == true);
   REQUIRE(data.query().isEmpty() == true);

   REQUIRE(data.query() == "");

   REQUIRE(data.hasQueryItem("width")  == false);
   REQUIRE(data.hasQueryItem("height") == false);

   REQUIRE(data.queryItemValue("width")  == "");
   REQUIRE(data.queryItemValue("height") == "");
}

TEST_CASE("QUrlQuery comparison", "[qurlquery]")
{
   QUrlQuery data_a("x=1&y=2");
   QUrlQuery data_b("x=1&y=2");
   QUrlQuery data_c("x=1&y=3");

   REQUIRE(data_a == data_b);
   REQUIRE(data_a != data_c);

   REQUIRE(! (data_a != data_b));
   REQUIRE(! (data_a == data_c));
}

TEST_CASE("QUrlQuery constructor", "[qurlquery]")
{
   QUrlQuery data("result=a%20b");

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.queryItemValue("result") == "a b");

   //
   data = QUrlQuery("a=1&b=2");

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.queryItemValue("a") == "1");
   REQUIRE(data.queryItemValue("b") == "2");

   //
   QUrl url("http://www.example.com/?type=pdf");

   data = QUrlQuery(url);

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.queryItemValue("type") == "pdf");
}

TEST_CASE("QUrlQuery copy_assign", "[qurlquery]")
{
   QUrlQuery data_a("width=1");
   QUrlQuery data_b(data_a);

   REQUIRE(data_a.queryItemValue("width") == "1");
   REQUIRE(data_b.queryItemValue("width") == "1");

   //
   QUrlQuery data_c;
   data_c = data_a;

   REQUIRE(data_a.queryItemValue("width") == "1");
   REQUIRE(data_c.queryItemValue("width") == "1");
}

TEST_CASE("QUrlQuery empty", "[qurlquery]")
{
   QUrlQuery data;

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.query().isEmpty() == true);
   REQUIRE(data.query() == QString());

   REQUIRE(data.queryItems().isEmpty()   == true);
   REQUIRE(data.queryItemValue("result") == "");

   data = QUrlQuery("a=1");

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.query().isEmpty() == false);
   REQUIRE(data.query() == QString("a=1"));
}

TEST_CASE("QUrlQuery move_assign", "[qurlquery]")
{
   QUrlQuery data_a("width=1");
   QUrlQuery data_b(std::move(data_a));

   REQUIRE(data_b.queryItemValue("width") == "1");

   //
   QUrlQuery data_c;
   data_c = std::move(data_b);

   REQUIRE(data_c.queryItemValue("width") == "1");
}

TEST_CASE("QUrlQuery query", "[qurlquery]")
{
   {
      QUrlQuery data("x=%ZZ");

      // invalid % encoding
      REQUIRE(data.queryItemValue("x") == "%25ZZ");
   }

   {
      QUrlQuery data("a=1&&b=2&&");

      // extra separators
      REQUIRE(data.queryItemValue("a") == "1");
      REQUIRE(data.queryItemValue("b") == "2");

      REQUIRE(data.queryItems().size() == 4);
   }

   {
      QUrlQuery data("length");

      REQUIRE(data.queryItemValue("length") == "");
   }

   {
      QUrlQuery data("length=");

      REQUIRE(data.queryItemValue("length") == "");
   }

   {
      QUrlQuery data("=value");

      REQUIRE(data.queryItems().size() == 1);

      REQUIRE(data.queryItems()[0].first.isEmpty());
      REQUIRE(data.queryItems()[0].second == "value");
   }

   {
      QUrlQuery data;
      data.addQueryItem("white space", "ok good");

      REQUIRE(data.query(QUrl::FullyEncoded) == "white%20space=ok%20good");
      REQUIRE(data.query(QUrl::EncodeSpaces) == "white%20space=ok%20good");
      REQUIRE(data.query() == "white space=ok good");
   }
}

TEST_CASE("QUrlQuery removeQueryItem", "[qurlquery]")
{
   {
      QUrlQuery data("a=1&a=2&b=3");
      data.removeQueryItem("a");

      REQUIRE(data.allQueryItemValues("a") == QList<QString>{"2"});
   }

   {
      QUrlQuery data("a=1&a=2&a=3");
      data.removeAllQueryItems("a");

      REQUIRE(data.hasQueryItem("a") == false);
      REQUIRE(data.queryItems().isEmpty() == true);
   }
}

TEST_CASE("QUrlQuery setQueryDelimiters", "[qurlquery]")
{
   QUrlQuery data("width=1&height=2");
   data.setQueryDelimiters(';', '#');

   REQUIRE(data.query() == "width;1#height;2");

   //
   data.clear();

   data.setQueryDelimiters('=', '^');
   data.setQuery("a=1^^b=2^^^c=3");

   REQUIRE(data.queryItems().size() == 6);
}

TEST_CASE("QUrlQuery setQueryItem", "[qurlquery]")
{
   QList<QPair<QString, QString>> items = { {"x","1"}, {"y","2"} };

   QUrlQuery data;
   data.setQueryItems(items);

   REQUIRE(data.queryItemValue("x") == "1");
   REQUIRE(data.queryItemValue("y") == "2");

   REQUIRE(data.queryItems().size() == 2);
}

TEST_CASE("QUrlQuery swap", "[qurlquery]")
{
   QUrlQuery data_a("x=1&y=2");
   QUrlQuery data_b("z=9");

   data_a.swap(data_b);

   REQUIRE(data_a.queryItemValue("x") == "");
   REQUIRE(data_a.queryItemValue("y") == "");
   REQUIRE(data_a.queryItemValue("z") == "9");

   REQUIRE(data_b.queryItemValue("x") == "1");
   REQUIRE(data_b.queryItemValue("y") == "2");
   REQUIRE(data_b.queryItemValue("z") == "");
}
