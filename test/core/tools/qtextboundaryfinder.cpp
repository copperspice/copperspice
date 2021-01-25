/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#include <qtextboundaryfinder.h>

#include <cs_catch2.h>

#include <iostream>

// http://www.unicode.org/reports/tr29/#Grapheme_Cluster_Boundaries
// http://www.unicode.org/reports/tr29/#Word_Boundaries
// http://www.unicode.org/reports/tr29/#Sentence_Boundaries

TEST_CASE("QTextBoundaryFinder construction", "[qtextboundaryfinder]")
{
   QTextBoundaryFinder finder(QTextBoundaryFinder::Word, "test");

   REQUIRE(finder.position() == 0);
   REQUIRE(finder.type() == QTextBoundaryFinder::Word);
}

TEST_CASE("QTextBoundaryFinder isValid()", "[qtextboundaryfinder]")
{
   QTextBoundaryFinder finder(QTextBoundaryFinder::Word, "");

   REQUIRE(finder.position() == 0);
}

TEST_CASE("QTextBoundaryFinder empty string", "[qtextboundaryfinder]")
{
   QTextBoundaryFinder finder(QTextBoundaryFinder::Word, "");

   finder.toStart();   
   REQUIRE(finder.position() == 0);

   finder.toEnd();   
   REQUIRE(finder.position() == 0);
}

TEST_CASE("QTextBoundaryFinder toStart()", "[qtextboundaryfinder]")
{
   QTextBoundaryFinder finder(QTextBoundaryFinder::Word, "test");

   finder.toEnd();
   REQUIRE(finder.position() == 4);

   finder.toStart();
   REQUIRE(finder.position() == 0);
}

TEST_CASE("QTextBoundaryFinder toEnd()", "[qtextboundaryfinder]")
{
   QString string = "this is a test";
   QTextBoundaryFinder finder(QTextBoundaryFinder::Word, string);

   finder.setPosition(string.length());
   int length = finder.position();

   finder.toStart();
   finder.toEnd();

   REQUIRE(length == finder.position());
}

TEST_CASE("QTextBoundaryFinder toNextBoundary()", "[qtextboundaryfinder]")
{
   QTextBoundaryFinder finder(QTextBoundaryFinder::Word, "this is a test");

   REQUIRE(finder.position() == 0);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::StartOfItem )== finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(4 == finder.position());
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::EndOfItem) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 5);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::StartOfItem) == finder.boundaryReasons());
   
   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 7  );
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::EndOfItem) == finder.boundaryReasons());
   
   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 8);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::StartOfItem) == finder.boundaryReasons());
   
   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 9);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::EndOfItem) == finder.boundaryReasons());
   
   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 10);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::StartOfItem) == finder.boundaryReasons());
   
   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 14);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::EndOfItem) == finder.boundaryReasons());

   // REQUIRE if moving past the last boundary returns -1
   REQUIRE(finder.toNextBoundary() == -1);
   REQUIRE(finder.position() == 14);
   REQUIRE(QTextBoundaryFinder::NotAtBoundary == finder.boundaryReasons());
   
   // REQUIRE if moving past the last boundary again still returns -1
   REQUIRE(finder.toNextBoundary() == -1);
   REQUIRE(finder.position() == 14);
   REQUIRE(QTextBoundaryFinder::NotAtBoundary == finder.boundaryReasons());
}

TEST_CASE("QTextBoundaryFinder toPreviousBoundary()", "[qtextboundaryfinder]")
{
   QTextBoundaryFinder finder(QTextBoundaryFinder::Word, "this is a test");

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.toPreviousBoundary() != -1);
   REQUIRE(finder.position() == 0);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::StartOfItem )== finder.boundaryReasons());
   REQUIRE(finder.toPreviousBoundary() == -1);
}

TEST_CASE("QTextBoundaryFinder a b c", "[qtextboundaryfinder]")
{
   QTextBoundaryFinder finder(QTextBoundaryFinder::Word, "a b c");

   REQUIRE(finder.position() == 0);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::StartOfItem )== finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 1);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::EndOfItem) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 2);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::StartOfItem )== finder.boundaryReasons());
   
   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 3);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::EndOfItem) == finder.boundaryReasons());
   
   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 4);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::StartOfItem )== finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 5);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::EndOfItem) == finder.boundaryReasons());
}

TEST_CASE("QTextBoundaryFinder Hiragana()", "[qtextboundaryfinder]")
{
   QTextBoundaryFinder finder(QTextBoundaryFinder::Word, "しゃしん を もっと とりたいです"); // BreakOpportunity on every character

   REQUIRE(finder.position() == 0);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity)== finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 1);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 2);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());
   
   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 3);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());
   
   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 4);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 5);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 6);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 7);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 8);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 9);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 10);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 11);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());
}

TEST_CASE("QTextBoundaryFinder Kanij-kanakana-hiragana()", "[qtextboundaryfinder]")
{
   QTextBoundaryFinder finder(QTextBoundaryFinder::Word, "私はヤンです");  // ヤン is correctly found as a word

   REQUIRE(0 == finder.position());
   REQUIRE((QTextBoundaryFinder::BreakOpportunity)== finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(1 == finder.position());
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(2 == finder.position());
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::StartOfItem) == finder.boundaryReasons());
   
   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(4 == finder.position());
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::EndOfItem) == finder.boundaryReasons());
   
   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(5 == finder.position());
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(6 == finder.position());
   REQUIRE((QTextBoundaryFinder::BreakOpportunity) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() == -1);
   REQUIRE(6 == finder.position());
}

TEST_CASE("QTextBoundaryFinder QTextBoundaryFinder::Sentence", "[qtextboundaryfinder]")
{
   QString test("Yes, sir! Ready now, sir.  ");
   //                      ^-- 10           ^-- 27

   REQUIRE(test.size() == 27);
   QTextBoundaryFinder finder(QTextBoundaryFinder::Sentence, test);
   
   REQUIRE(finder.position() == 0);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::StartOfItem )== finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 10);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::StartOfItem | QTextBoundaryFinder::EndOfItem) == finder.boundaryReasons());

   REQUIRE(finder.toNextBoundary() != -1);
   REQUIRE(finder.position() == 27);
   REQUIRE((QTextBoundaryFinder::BreakOpportunity | QTextBoundaryFinder::EndOfItem) == finder.boundaryReasons());
   
   REQUIRE(finder.toNextBoundary() == -1);
}

QList<QString> Extract(QTextBoundaryFinder::BoundaryType boundaryType, QString text)
{
   QList<QString> extracted;
   int startIndex = 0;

   QTextBoundaryFinder finder(boundaryType, text);

   for (int index = finder.toNextBoundary(); index != -1; index = finder.toNextBoundary()) {
      if (finder.boundaryReasons() & QTextBoundaryFinder::StartOfItem) {
         startIndex = index;
      }

      if (finder.boundaryReasons() & QTextBoundaryFinder::EndOfItem) {
         extracted.append(text.mid(startIndex, index-startIndex));
      }
   }

   return extracted;
}

TEST_CASE("QTextBoundaryFinder tr29/#Word_Boundaries", "[qtextboundaryfinder]")
{
   // http://www.unicode.org/reports/tr29/#Word_Boundaries
   REQUIRE(
      Extract(QTextBoundaryFinder::Word, "The quick ('brown') fox can't jump 32.3 feet, right?") == 
      QList<QString>({"The", "quick", "brown", "fox", "can't", "jump", "32.3", "feet", "right"})
      );

   REQUIRE(Extract(QTextBoundaryFinder::Word, " ") == QList<QString>());

   REQUIRE(
      Extract(QTextBoundaryFinder::Word, "The number PI in Dutch number format is about 3,1415627, right?") == 
      QList<QString>({"The", "number", "PI", "in", "Dutch", "number", "format", "is", "about", "3,1415627", "right"})
   );

   QString personalComputer = "パーソナル・コンピューター";
   REQUIRE(personalComputer.length() == 13);

   QList<QString> words = Extract(QTextBoundaryFinder::Word, personalComputer);
   REQUIRE(words.size() == 2);  
   REQUIRE(words[0].size() == 5);
   REQUIRE(words[1].size() == 7);
}

TEST_CASE("QTextBoundaryFinder tr29/#Word_Boundaries emoij", "[qtextboundaryfinder]")
{
   QString text = QString::fromUtf8("This is 😱 ");

   // Emoji count in size() as letters/characters, so do spaces
   REQUIRE(text.size() == 10);
   
   QList<QString> words = Extract(QTextBoundaryFinder::Word, text);
   REQUIRE(words.size() == 2);

   REQUIRE(Extract(QTextBoundaryFinder::Word, QString::fromUtf8(" 😱 ")).size() == 0); // emoji are not words
}

TEST_CASE("QTextBoundaryFinder tr29/#Grapheme emoij", "[qtextboundaryfinder]")
{
   // Grapheme include spaces, katakana and emoji
   QString text = QString::fromUtf8("パーソナル 😱 コンピューター 😱");
   REQUIRE(text.size() == 17);

   QList<QString> graphemes = Extract(QTextBoundaryFinder::Grapheme, text);
   REQUIRE(graphemes.size() == 17);
}

TEST_CASE("QTextBoundaryFinder tr29/#Sentence emoij", "[qtextboundaryfinder]")
{
   // spaces, katakana and emoji do NOT break sentenses
   QString text = QString::fromUtf8("パーソナル 😱 コンピューター 😱");
   {
      QList<QString> sentences = Extract(QTextBoundaryFinder::Sentence, text); //
      REQUIRE(sentences.size() == 1);
   }

   {
      // newline ends the sentense
      QList<QString> sentences = Extract(QTextBoundaryFinder::Sentence, text + "\n" + text);
      REQUIRE(sentences.size() == 2);
   }

   {
      // dot ends the sentense
      QList<QString> sentences = Extract(QTextBoundaryFinder::Sentence, text + "." + text);
      REQUIRE(sentences.size() == 2);
   }

   {
      // japanese word separator (does not start a new sentense)
      QList<QString> sentences = Extract(QTextBoundaryFinder::Sentence, text + "・" + text);
      REQUIRE(sentences.size() == 1);
   }
}

TEST_CASE("QTextBoundaryFinder tr29/#Word_Boundaries Dutch", "[qtextboundaryfinder]")
{
   QString text = QString::fromUtf8("We gaan met z'n allen?");
   REQUIRE(text.size() == 22);
   
   QList<QString> words = Extract(QTextBoundaryFinder::Word, text);
   REQUIRE(words.size() == 5);
   REQUIRE(words[3] == "z'n");
   REQUIRE(words[4] == "allen"); // shouldn't include the ?
}

TEST_CASE("QTextBoundaryFinder tr29/#Word_Boundaries German", "[qtextboundaryfinder]")
{
   QString text = QString::fromUtf8("Warum bloß meine Füße?");
   REQUIRE(text.size() == 22);

   QList<QString> words = Extract(QTextBoundaryFinder::Word, text);
   REQUIRE(words.size() == 4);
   REQUIRE(words[1].toStdString() == "bloß");
   REQUIRE(words[1] == "bloß");
   REQUIRE(words[3].toStdString() == "Füße");      // should not include the ?

   text = QString::fromUtf8("Können Sie Rechtsschutzversicherungsgesellschaften sagen?");
   REQUIRE(text.size() == 57);

   words = Extract(QTextBoundaryFinder::Word, text);
   REQUIRE(words.size() == 4);
   REQUIRE(words[0].toStdString() == "Können");
   REQUIRE(words[3].toStdString() == "sagen");     // should not include the ?
}

TEST_CASE("QTextBoundaryFinder tr29/#Word_Boundaries French", "[qtextboundaryfinder]")
{
   QString text = QString::fromUtf8("L'arbre est dans le jardin!");
   REQUIRE(text.size() == 27);

   QList<QString> words = Extract(QTextBoundaryFinder::Word, text);
   REQUIRE(words.size() == 5);
   REQUIRE(words[0].toStdString() == "L'arbre");
   REQUIRE(words[4].toStdString() == "jardin");    // should not include the ?

   text = QString::fromUtf8("Je trouve qu'il y a là quelque chose de bizarre!");
   REQUIRE(text.size() == 48);

   words = Extract(QTextBoundaryFinder::Word, text);
   REQUIRE(words.size() == 10);
   REQUIRE(words[2].toStdString() == "qu'il");
   REQUIRE(words[5].toStdString() == "là");
   REQUIRE(words[9].toStdString() == "bizarre");   // should not include the !
}

TEST_CASE("QTextBoundaryFinder tr29/#Word_Boundaries Urdu", "[qtextboundaryfinder]")
{
   QString text = QString::fromUtf8("پانی ایک صاف مائع ہے");   // Urdu: 'Water is a clear liquid'
   REQUIRE(text.size() == 20);

   QList<QString> words = Extract(QTextBoundaryFinder::Word, text);
   REQUIRE(words.size() == 5);
   REQUIRE(words[0].toStdString() == "پانی");
   REQUIRE(words[4].toStdString() == "ہے");                 // Right to left, so ہے is the last word.
}
