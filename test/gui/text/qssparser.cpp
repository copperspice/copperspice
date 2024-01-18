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

#include <qcssparser_p.h>

#include <cs_catch2.h>

void registerGuiVariant();

TEST_CASE("QCssParser parse_color", "[qcssparser]")
{
   registerGuiVariant();

   {
      QString css = "color: blue";
      QColor expectedColor = QColor("blue");

      QCss::Parser parser(css);

      QCss::Declaration decl;
      REQUIRE(parser.parseNextDeclaration(&decl));

      REQUIRE(decl.d->property == "color");
      REQUIRE(decl.d->values.count() == 1);

      const QColor actualColor = decl.colorValue();

      REQUIRE(expectedColor.isValid() == actualColor.isValid());
      REQUIRE(expectedColor == actualColor);
   }

   {
      QString css = "color: \"red\"";
      QColor expectedColor = QColor("red");

      QCss::Parser parser(css);

      QCss::Declaration decl;
      REQUIRE(parser.parseNextDeclaration(&decl));

      REQUIRE(decl.d->property == "color");
      REQUIRE(decl.d->values.count() == 1);

      const QColor actualColor = decl.colorValue();

      REQUIRE(expectedColor.isValid() == actualColor.isValid());
      REQUIRE(expectedColor == actualColor);
   }

   {
      QString css = "color: #12af0e";
      QColor  expectedColor = QColor(0x12, 0xaf, 0x0e);

      QCss::Parser parser(css);
      QCss::Declaration decl;

      REQUIRE(parser.parseNextDeclaration(&decl));

      const QColor actualColor = decl.colorValue();
      REQUIRE(expectedColor.isValid() == actualColor.isValid());
      REQUIRE(actualColor == expectedColor);
   }

   {
      QString css = "color: rgb(15)";
      QColor  expectedColor = QColor();

      QCss::Parser parser(css);
      QCss::Declaration decl;

      REQUIRE(parser.parseNextDeclaration(&decl));

      const QColor actualColor = decl.colorValue();
      REQUIRE(expectedColor.isValid() == actualColor.isValid());
      REQUIRE(actualColor == expectedColor);
   }

   {
      QString css = "color: transparent";
      QColor  expectedColor = QColor(Qt::transparent);

      QCss::Parser parser(css);
      QCss::Declaration decl;

      REQUIRE(parser.parseNextDeclaration(&decl));

      const QColor actualColor = decl.colorValue();
      REQUIRE(expectedColor.isValid() == actualColor.isValid());
      REQUIRE(actualColor == expectedColor);
   }
}

TEST_CASE("QCssParser parse_escape", "[qcssparser]")
{
   QCss::Parser parser("\\hello");
   parser.test(QCss::IDENT);

   REQUIRE(parser.lexem() == "hello");
}

TEST_CASE("QCssParser parse_import", "[qcssparser]")
{
   {
      QCss::Parser parser("@import \"plainstring\";");

      REQUIRE(parser.testImport() == true);
      REQUIRE(parser.testMedia()  == false);
      REQUIRE(parser.testPage()   == false);

      QCss::ImportRule rule;
      REQUIRE(parser.parseImport(&rule) == true);

      REQUIRE(rule.href == "plainstring");
      REQUIRE(rule.media.count() == 0);
   }

   {
      QCss::Parser parser = QCss::Parser("@import url(\"www.copperspice.com\") print/*comment*/,screen;");

      REQUIRE(parser.testImport() == true);
      REQUIRE(parser.testMedia()  == false);
      REQUIRE(parser.testPage()   == false);

      QCss::ImportRule rule;
      REQUIRE(parser.parseImport(&rule) == true);

      REQUIRE(rule.href == "www.copperspice.com");
      REQUIRE(rule.media.count() == 2);
      REQUIRE(rule.media.at(0)   == "print");
      REQUIRE(rule.media.at(1)   == "screen");
   }
}

TEST_CASE("QCssParser parse_media", "[qcssparser]")
{
   QCss::Parser parser("@media print/*comment*/,screen /*comment to ignore*/{ }");

   REQUIRE(parser.testImport() == false);
   REQUIRE(parser.testMedia()  == true);
   REQUIRE(parser.testPage()   == false);

   QCss::MediaRule rule;
   REQUIRE(parser.parseMedia(&rule)  == true);

   REQUIRE(rule.media.count() == 2);
   REQUIRE(rule.media.at(0)   == "print");
   REQUIRE(rule.media.at(1)   == "screen");
   REQUIRE(rule.styleRules.isEmpty() == true);
}

TEST_CASE("QCssParser parse_page", "[qcssparser]")
{
   QCss::Parser parser("@page :first/*comment to ignore*/{ }");

   REQUIRE(parser.testImport() == false);
   REQUIRE(parser.testMedia()  == false);
   REQUIRE(parser.testPage()   == true);

   QCss::PageRule rule;
   REQUIRE(parser.parsePage(&rule)  == true);

   REQUIRE(rule.selector == "first");
   REQUIRE(rule.declarations.isEmpty() == true);
}

TEST_CASE("QCssParser parse_declarations", "[qcssparser]")
{
   QCss::Parser parser("#navigation ul.nav > li:after {\n"
                       "   content: '\\\\'\n"
                       "}\n");

   REQUIRE(parser.testImport() == false);
   REQUIRE(parser.testMedia()  == false);
   REQUIRE(parser.testPage()   == false);

   REQUIRE(parser.testRuleset() == true);

   QCss::StyleRule rule;
   REQUIRE(parser.parseRuleset(&rule) == true);

   REQUIRE(rule.selectors.count() == 1);
   REQUIRE(rule.selectors.at(0).basicSelectors.count() == 3);

   REQUIRE(rule.selectors.at(0).basicSelectors.at(0).ids.count() == 1);
   REQUIRE(rule.selectors.at(0).basicSelectors.at(0).ids.at(0) == "navigation");

   REQUIRE(rule.selectors.at(0).basicSelectors.at(1).elementName == "ul");

   REQUIRE(rule.selectors.at(0).basicSelectors.at(2).elementName == "li");

   REQUIRE(rule.declarations.count() == 1);
   REQUIRE(rule.declarations.at(0).d->property == "content");
   REQUIRE(rule.declarations.at(0).d->values.at(0).type == QCss::Value::String);
   REQUIRE(rule.declarations.at(0).d->values.at(0).toString() == "\\");
}

TEST_CASE("QCssParser parse_quoted", "[qcssparser]")
{
   QCss::Parser parser("foo { font-style: \"italic\"; font-weight: bold }");

   QCss::StyleSheet sheet;
   REQUIRE(parser.parse(&sheet) == true);

   REQUIRE(sheet.styleRules.count() == 0);
   REQUIRE(sheet.nameIndex.count()  == 1);

   auto iter = sheet.nameIndex.begin();
   QCss::StyleRule rule = *iter;

   const QVector<QCss::Declaration> &decls = rule.declarations;
   REQUIRE(decls.size() == 2);

   REQUIRE(decls.at(0).d->values.first().type == QCss::Value::String);
   REQUIRE(decls.at(0).d->property == "font-style");
   REQUIRE(decls.at(0).d->values.first().toString() == "italic");

   REQUIRE(decls.at(1).d->values.first().type == QCss::Value::KnownIdentifier);
   REQUIRE(decls.at(1).d->property == "font-weight");
   REQUIRE(decls.at(1).d->values.first().toString() == "bold");
}

TEST_CASE("QCssParser parse_ruleset", "[qcssparser]")
{
   {
      QCss::Parser parser("p/*foo*/{ }");

      REQUIRE(parser.testRuleset() == true);

      QCss::StyleRule rule;
      REQUIRE(parser.parseRuleset(&rule) == true);

      REQUIRE(rule.selectors.count() == 1);
      REQUIRE(rule.selectors.at(0).basicSelectors.count() == 1);
      REQUIRE(rule.selectors.at(0).basicSelectors.at(0).elementName == "p");

      REQUIRE(rule.declarations.isEmpty() == true);
   }

   {
      QCss::Parser parser("p/*comment*/,div{ }");

      REQUIRE(parser.testRuleset() == true);

      QCss::StyleRule rule;
      REQUIRE(parser.parseRuleset(&rule) == true);

      REQUIRE(rule.selectors.count() == 2);
      REQUIRE(rule.selectors.at(0).basicSelectors.count() == 1);
      REQUIRE(rule.selectors.at(0).basicSelectors.at(0).elementName == "p");

      REQUIRE(rule.selectors.at(1).basicSelectors.count() == 1);
      REQUIRE(rule.selectors.at(1).basicSelectors.at(0).elementName == "div");

      REQUIRE(rule.declarations.isEmpty() == true);
   }

   {
      QCss::Parser parser(":before, :after { }");

      REQUIRE(parser.testRuleset() == true);

      QCss::StyleRule rule;
      REQUIRE(parser.parseRuleset(&rule) == true);

      REQUIRE(rule.selectors.count() == 2);

      REQUIRE(rule.selectors.at(0).basicSelectors.count() == 1);
      REQUIRE(rule.selectors.at(0).basicSelectors.at(0).pseudos.count() ==  1);
      REQUIRE(rule.selectors.at(0).basicSelectors.at(0).pseudos.at(0).name == "before");

      REQUIRE(rule.selectors.at(1).basicSelectors.count() == 1);
      REQUIRE(rule.selectors.at(1).basicSelectors.at(0).pseudos.count() == 1);
      REQUIRE(rule.selectors.at(1).basicSelectors.at(0).pseudos.at(0).name == "after");

      REQUIRE(rule.declarations.isEmpty() == true);
   }
}

TEST_CASE("QCssParser parse_scan", "[qcssparser]")
{
   QString data = "/* let's see if  comments actually work *//*foo*/ \"here is /*a \\\\\\\"comment \\\"inside a string*/ done\"";

   QVector<QCss::Symbol> symbols;
   QCss::Scanner::scan(QCss::Scanner::preprocess(data), &symbols);

   REQUIRE(symbols.count() == 4);
   REQUIRE(symbols.at(0).token == QCss::S);
   REQUIRE(symbols.at(1).token == QCss::S);
   REQUIRE(symbols.at(2).token == QCss::S);
   REQUIRE(symbols.at(3).token == QCss::STRING);

   REQUIRE(symbols.at(0).lexem() == "/* let's see if  comments actually work */");
   REQUIRE(symbols.at(1).lexem() == "/*foo*/");
   REQUIRE(symbols.at(2).lexem() == " ");
   REQUIRE(symbols.at(3).lexem() == "\"here is /*a \\\"comment \"inside a string*/ done\"");
}

