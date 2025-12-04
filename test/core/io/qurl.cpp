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

#include <qurl.h>

#include <cs_catch2.h>

TEST_CASE("QUrl traits", "[qurl]")
{
   REQUIRE(std::is_copy_constructible_v<QUrl> == true);
   REQUIRE(std::is_move_constructible_v<QUrl> == true);

   REQUIRE(std::is_copy_assignable_v<QUrl> == true);
   REQUIRE(std::is_move_assignable_v<QUrl> == true);

   REQUIRE(std::has_virtual_destructor_v<QUrl> == false);
}

TEST_CASE("QUrl adjusted", "[qurl]") {
   QUrl data_1("http://example.com/path/");
   QUrl data_2 = data_1.adjusted(QUrl::StripTrailingSlash);

   REQUIRE(data_2.toString() == "http://example.com/path");
}

TEST_CASE("QUrl comparison", "[qurl]")
{
   QUrl data_1("http://example.com");
   QUrl data_2("http://example.com");

   REQUIRE(data_1 == data_2);

   //
   QUrl data_3("http://example.com/a");
   QUrl data_4("http://example.com/b");

   REQUIRE(data_3 != data_4);
}

TEST_CASE("QUrl constructor", "[qurl]")
{
   QUrl data;

   REQUIRE(data.isEmpty() == true);
   REQUIRE(data.isValid() == false);
   REQUIRE(data.path().isEmpty()   == true);
   REQUIRE(data.scheme().isEmpty() == true);

   //
   data = QUrl("https://");
   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isValid() == true);
   REQUIRE(data.path().isEmpty()   == true);
   REQUIRE(data.scheme().isEmpty() == false);

   REQUIRE(data.scheme() == "https");

   // malformed percent encoding
   data = QUrl("http://example.com/%ZZ");

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isValid() == true);
   REQUIRE(data.path().isEmpty()   == false);
   REQUIRE(data.scheme().isEmpty() == false);

   REQUIRE(data.path()   == "/%ZZ");
   REQUIRE(data.scheme() == "http");
}

TEST_CASE("QUrl copy_assign", "[qurl]")
{
   QUrl data_a("http://example.com/path");
   QUrl data_b(data_a);

   REQUIRE(data_a == data_b);
   REQUIRE(data_b == QUrl("http://example.com/path"));

   //
   QUrl data_c;
   data_c = data_a;

   REQUIRE(data_a == data_c);
   REQUIRE(data_c == QUrl("http://example.com/path"));
}

TEST_CASE("QUrl fragment", "[qurl]")
{
   QUrl data("http://example.com/page#section1");

   REQUIRE(data.hasFragment());
   REQUIRE(data.fragment() == "section1");

   data.setFragment("section2");
   REQUIRE(data.fragment() == "section2");

   data.setFragment(QString());
   REQUIRE(data.hasFragment() == false);
}

TEST_CASE("QUrl fromLocalFile", "[qurl]")
{
   QUrl data = QUrl::fromLocalFile("/tmp/test.txt");

   REQUIRE(data.scheme() == "file");
   REQUIRE(data.path() == "/tmp/test.txt");

   REQUIRE(data.isLocalFile() == true);
   REQUIRE(data.toLocalFile().endsWith("test.txt"));
}

TEST_CASE("QUrl fromPercentEncoding", "[qurl]")
{
   QUrl data;

   data.setScheme("http");
   data.setHost("example.com");
   data.setPath("/a b");

   QString s = data.toString(QUrl::FullyEncoded);
   REQUIRE(s.contains("%20") == true);

   REQUIRE(QUrl::fromPercentEncoding("a%20b") == "a b");
}

TEST_CASE("QUrl host", "[qurl]")
{
   QUrl data("https://www.copperspice.com/about.html");

   REQUIRE(data.isEmpty() == false);
   REQUIRE(data.isValid() == true);
   REQUIRE(data.path().isEmpty() == false);

   REQUIRE(data.scheme() == "https");
   REQUIRE(data.host()   == "www.copperspice.com");
   REQUIRE(data.path()   == "/about.html");
}

TEST_CASE("QUrl isRelative", "[qurl]")
{
   QUrl data_a("http://example.com");

   REQUIRE(data_a.isRelative() == false);

   //
   QUrl data_b("file.txt");

   REQUIRE(data_b.isRelative() == true);
}

TEST_CASE("QUrl isValid", "[qurl]")
{
   QUrl data("https://www.copperspice.com/style_guide/source_code_style.html");
   REQUIRE(data.isValid() == true);

   data = QUrl("https://www.copperspice.com/some_random_name");
   REQUIRE(data.isValid() == true);
}

TEST_CASE("QUrl mailto", "[qurl]")
{
   QUrl data("mailto:user@example.com");

   REQUIRE(data.scheme() == "mailto");
   REQUIRE(data.host().isEmpty());
   REQUIRE(data.path() == "user@example.com");
}

TEST_CASE("QUrl move_assign", "[qurl]")
{
   QUrl data_a("http://example.com/path");
   QUrl data_b(std::move(data_a));

   REQUIRE(data_b == QUrl("http://example.com/path"));
   REQUIRE(data_b.host() == "example.com");

   //
   QUrl data_c;
   data_c = std::move(data_b);

   REQUIRE(data_c == QUrl("http://example.com/path"));
   REQUIRE(data_c.host() == "example.com");
}

TEST_CASE("QUrl password", "[qurl]")
{
   QUrl data("ftp://user:pass@example.com:21/dir/file");

   REQUIRE(data.scheme()   == "ftp");
   REQUIRE(data.userName() == "user");
   REQUIRE(data.password() == "pass");
   REQUIRE(data.host()     == "example.com");
   REQUIRE(data.port()     == 21);
}

TEST_CASE("QUrl path", "[qurl]")
{
   QUrl data_1("https://www.copperspice.com/about.html");
   REQUIRE(data_1.path() == "/about.html");

   //
   QUrl data_2("http://example.com/a/b/c");
   REQUIRE(data_2.path() == "/a/b/c");

   data_2.setPath("/x/y");
   REQUIRE(data_2.path() == "/x/y");

   //
   QUrl data_3("mailto:postmaster@example.com");
   REQUIRE(data_3.path() == "postmaster@example.com");
}

TEST_CASE("QUrl query", "[qurl]")
{
   QUrl data("http://example.com");
   data.setQuery("a=1&b=2");

   REQUIRE(data.hasQuery());
   REQUIRE(data.query() == "a=1&b=2");

   data.setQuery(QString());
   REQUIRE(data.hasQuery() == false);

   //
   data = QUrl("http://example.com/search?q=test&n=10");

   REQUIRE(data.hasQuery());

   REQUIRE(data.query().contains("q=test"));
   REQUIRE(data.query().contains("n=10"));
}

TEST_CASE("QUrl resolved", "[qurl]") {
   QUrl data_1("http://example.com/dir/");
   QUrl data_2("file.txt");

   QUrl resolved = data_1.resolved(data_2);
   REQUIRE(resolved.toString() == "http://example.com/dir/file.txt");

   //
   data_1 = QUrl("http://example.com/dir/");
   data_2 = QUrl("https://other.com/x");

   resolved = data_1.resolved(data_2);
   REQUIRE(resolved.host() == "other.com");
}

TEST_CASE("QUrl scheme", "[qurl]")
{
   QUrl data("https://www.copperspice.com/about.html");
   REQUIRE(data.scheme() == "https");
}

TEST_CASE("QUrl to_encoded", "[qurl]")
{
   QString str("http://www.example.com/List of Holidays.xml");

   QUrl data = QUrl(str, QUrl::StrictMode);
   REQUIRE(data.toEncoded() == "");

   data = QUrl(str, QUrl::TolerantMode);
   REQUIRE(data.toEncoded() == "http://www.example.com/List%20of%20Holidays.xml");
}

TEST_CASE("QUrl to_string", "[qurl]")
{
   QUrl data_1("https://www.copperspice.com/about.html");

   REQUIRE(data_1.toString() == "https://www.copperspice.com/about.html");

   //
   QUrl data_2("http://user:pass@example.com/path with spaces");

   QString full   = data_2.toString(QUrl::FullyEncoded);
   QString pretty = data_2.toString(QUrl::PrettyDecoded);

   REQUIRE(full.contains("%") == true);
   REQUIRE(full   == "http://user:pass@example.com/path%20with%20spaces");
   REQUIRE(pretty == "http://user:pass@example.com/path with spaces");
}

TEST_CASE("QUrl user_info", "[qurl]")
{
   QUrl data_1("https://www.copperspice.com/about.html");
   REQUIRE(data_1.userInfo() == "");

   QUrl data_2("https://user:password@www.copperspice.com/about.html");
   REQUIRE(data_2.userInfo() == "user:password");
}
