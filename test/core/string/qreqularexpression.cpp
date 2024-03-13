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

#include <qregularexpression.h>

#include <cs_catch2.h>

TEST_CASE("QRegularExpression traits", "[qregularexpression]")
{
   REQUIRE(std::is_copy_constructible_v<QRegularExpression> == true);
   REQUIRE(std::is_move_constructible_v<QRegularExpression> == true);

   REQUIRE(std::is_copy_assignable_v<QRegularExpression> == true);
   REQUIRE(std::is_move_assignable_v<QRegularExpression> == true);

   REQUIRE(std::has_virtual_destructor_v<QRegularExpression> == false);
}

TEST_CASE("QRegularExpression empty", "[qregularexpression]")
{
   QRegularExpression regExp;

   REQUIRE(regExp.captureCount() == -1);
   REQUIRE(regExp.namedCaptureGroups().isEmpty());
   REQUIRE(regExp.pattern() == "");
// REQUIRE(regExp.patternErrorOffset() == 0);    // emerald, add soon to CS

   REQUIRE(! regExp.isValid());
}

TEST_CASE("QRegularExpression match_a", "[qregularexpression]")
{
   QString8 str = "A wacky fox and sizeable pig jumped halfway over a blue moon";
   QRegularExpression regExp("h(...)way");

   QRegularExpressionMatch match = regExp.match(str);

   REQUIRE(regExp.isValid());
   REQUIRE(match.hasMatch());
   REQUIRE(! match.hasPartialMatch());

   REQUIRE(regExp.captureCount() == 1);
   REQUIRE(match.captured(0)  == "halfway");
   REQUIRE(match.captured(1)  == "alf");
}

TEST_CASE("QRegularExpression match_b", "[qregularexpression]")
{
   QString8 str = "Version 1.7.0";
   QRegularExpression regExp("\\d+\\.\\d+");

   QRegularExpressionMatch match = regExp.match(str);

   REQUIRE(match.captured(0) == "1.7");
}

TEST_CASE("QRegularExpression look_ahead", "[qregularexpression]")
{
   QString8 str = "A wacky fox and sizeable pig jumped halfway over a blue moon";
   QRegularExpression regExp("f(..?)x(?!a)");

   QRegularExpressionMatch match = regExp.match(str);

   REQUIRE(match.hasMatch());
   REQUIRE(! match.hasPartialMatch());

   REQUIRE(match.captured(0) == "fox");
   REQUIRE(match.captured(1) == "o");
}

TEST_CASE("QRegularExpression quantifiers", "[qregularexpression]")
{
   QString8 str = "A wacky fox and sizeable pig jumped halfway over a blue moon";
   QRegularExpression regExp("(?<= )[a-z]{3} (.*) [a-z]{3}(?= )");

   QRegularExpressionMatch match = regExp.match(str);

   REQUIRE(match.captured(0) == "fox and sizeable pig");
   REQUIRE(match.captured(1) == "and sizeable");
}

TEST_CASE("QRegularExpression word_boundaries", "[qregularexpression]")
{
   QString8 str = "A wacky fox and sizeable pig jumped halfway over a blue moon";
   QRegularExpression regExp("\\bjumped\\b");

   QRegularExpressionMatch match = regExp.match(str);

   REQUIRE(match.captured(0) == "jumped");
}

TEST_CASE("QRegularExpression whitespace", "[qregularexpression]")
{
   QString8 str = "wacky fox and sizeable pig";
   QRegularExpression regExp("(\\w+)[ \t]*and[ \t]*(\\w*)");

   QRegularExpressionMatch match = regExp.match(str);

   REQUIRE(match.captured(0) == "fox and sizeable");
   REQUIRE(match.captured(1) == "fox");
   REQUIRE(match.captured(2) == "sizeable");
}

TEST_CASE("QRegularExpression name_captured", "[qregularexpression]")
{
   QString8 str = "Sunday Monday Tuesday";
   QRegularExpression regExp("(?<groupA>da)(?<groupB>y)");

   QRegularExpressionMatch match = regExp.match(str);

   QList<QString> list = regExp.namedCaptureGroups();

   REQUIRE(list.count() == 2);

   REQUIRE(match.captured("groupA") == "da");
   REQUIRE(match.captured("groupB") == "y");
}

TEST_CASE("QRegularExpression global_match", "[qregularexpression]")
{
   QString8 str = "Sunday Monday Tuesday";
   QRegularExpression regExp("(?<bob>\\w+)");

   QList<QRegularExpressionMatch> result = regExp.globalMatch(str);

   REQUIRE(result.size() == 3);

   REQUIRE(result[0].captured(0) == "Sunday");
   REQUIRE(result[1].captured(0) == "Monday");
   REQUIRE(result[2].captured(0) == "Tuesday");
}

TEST_CASE("QRegularExpression replace", "[qregularexpression]")
{
   QString8 str = "@3::A   A::@2::B   @A";

   QRegularExpression regex = QRegularExpression("(::)?@[0-9]+");
   str.replace(regex, "");

   REQUIRE(str == "::A   A::B   @A");
}

