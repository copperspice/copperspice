/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

class QCssScanner_Generated
{
 public:
   QCssScanner_Generated(const QString &str);

   inline QChar next() {
      QChar retval;

      if (pos != input.end()) {
         retval = *pos;
         ++pos;
      }

      return retval;
   }

   int handleCommentStart();
   int lex();

   const QString &input;
   QString::const_iterator pos;
   QString::const_iterator lexemStart;

   int lexemLength;
};

QCssScanner_Generated::QCssScanner_Generated(const QString &str)
   : input(str)
{
   pos = input.begin();
   lexemStart  = input.begin();
   lexemLength = 0;
}

int QCssScanner_Generated::lex()
{
   lexemStart  = pos;
   lexemLength = 0;

   std::optional<QString::const_iterator> tmpIter;
   int token = -1;

   // initial state
   QChar ch = next();

   if (ch.unicode() >= 9 && ch.unicode() <= 10) {
      goto state_1;
   }

   if (ch.unicode() >= 12 && ch.unicode() <= 13) {
      goto state_1;
   }

   if (ch.unicode() == 32) {
      goto state_1;
   }

   if (ch.unicode() == 33) {
      token = QCss::EXCLAMATION_SYM;
      goto found;
   }

   if (ch.unicode() == 34) {
      goto state_3;
   }

   if (ch.unicode() == 35) {
      goto state_4;
   }

   if (ch.unicode() == 39) {
      goto state_5;
   }

   if (ch.unicode() == 40) {
      token = QCss::LPAREN;
      goto found;
   }

   if (ch.unicode() == 41) {
      token = QCss::RPAREN;
      goto found;
   }

   if (ch.unicode() == 42) {
      token = QCss::STAR;
      goto found;
   }

   if (ch.unicode() == 43) {
      goto state_9;
   }

   if (ch.unicode() == 44) {
      goto state_10;
   }

   if (ch.unicode() == 45) {
      goto state_11;
   }

   if (ch.unicode() == 46) {
      goto state_12;
   }

   if (ch.unicode() == 47) {
      goto state_13;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_14;
   }
   if (ch.unicode() == 58) {
      token = QCss::COLON;
      goto found;
   }

   if (ch.unicode() == 59) {
      token = QCss::SEMICOLON;
      goto found;
   }

   if (ch.unicode() == 60) {
      goto state_17;
   }

   if (ch.unicode() == 61) {
      token = QCss::EQUAL;
      goto found;
   }

   if (ch.unicode() == 62) {
      goto state_19;
   }
   if (ch.unicode() == 64) {
      goto state_20;
   }
   if (ch.unicode() == 91) {
      token = QCss::LBRACKET;
      goto found;
   }

   if (ch.unicode() == 92) {
      goto state_22;
   }

   if (ch.unicode() == 93) {
      token = QCss::RBRACKET;
      goto found;
   }

   if (ch.unicode() == 95) {
      goto state_24;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_24;
   }

   if (ch.unicode() == 123) {
      goto state_25;
   }

   if (ch.unicode() == 124) {
      goto state_26;
   }

   if (ch.unicode() == 125) {
      token = QCss::RBRACE;
      goto found;
   }

   if (ch.unicode() == 126) {
      goto state_28;
   }

   goto out;

state_1:
   tmpIter = pos;
   token   = QCss::S;
   ch      = next();

   if (ch.unicode() >= 9 && ch.unicode() <= 10) {
      goto state_29;
   }
   if (ch.unicode() >= 12 && ch.unicode() <= 13) {
      goto state_29;
   }
   if (ch.unicode() == 32) {
      goto state_29;
   }
   if (ch.unicode() == 43) {
      goto state_9;
   }
   if (ch.unicode() == 44) {
      goto state_10;
   }
   if (ch.unicode() == 62) {
      goto state_19;
   }
   if (ch.unicode() == 123) {
      goto state_25;
   }

   goto out;

state_3:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_30;
   }

   if (ch.unicode() == 11) {
      goto state_30;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 33) {
      goto state_30;
   }

   if (ch.unicode() == 34) {
      goto state_31;
   }

   if (ch.unicode() >= 35 && ch.unicode() <= 91) {
      goto state_30;
   }

   if (ch.unicode() == 92) {
      goto state_32;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_30;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_30;
   }

   if (ch.unicode() >= 123) {
      goto state_30;
   }

   goto out;

state_4:
   ch = next();

   if (ch.unicode() == 45) {
      goto state_33;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_33;
   }

   if (ch.unicode() == 92) {
      goto state_34;
   }

   if (ch.unicode() == 95) {
      goto state_33;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_33;
   }

   goto out;

state_5:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_35;
   }

   if (ch.unicode() == 11) {
      goto state_35;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 38) {
      goto state_35;
   }

   if (ch.unicode() == 39) {
      goto state_36;
   }

   if (ch.unicode() >= 40 && ch.unicode() <= 91) {
      goto state_35;
   }

   if (ch.unicode() == 92) {
      goto state_37;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_35;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_35;
   }

   if (ch.unicode() >= 123) {
      goto state_35;
   }

   goto out;

state_9:
   tmpIter = pos;
   token   = QCss::PLUS;
   goto out;

state_10:
   tmpIter = pos;
   token   = QCss::COMMA;
   goto out;

state_11:
   tmpIter = pos;
   token   = QCss::MINUS;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_38;
   }

   if (ch.unicode() == 92) {
      goto state_22;
   }

   if (ch.unicode() == 95) {
      goto state_24;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_24;
   }

   goto out;

state_12:
   tmpIter = pos;
   token   = QCss::DOT;
   ch      = next();

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_39;
   }

   goto out;

state_13:
   tmpIter = pos;
   token   = QCss::SLASH;
   ch      = next();

   if (ch.unicode() == 42) {
      token = handleCommentStart();
      goto found;
   }

   goto out;

state_14:
   tmpIter = pos;
   token   = QCss::NUMBER;
   ch      = next();

   if (ch.unicode() == 37) {
      goto state_41;
   }

   if (ch.unicode() == 45) {
      goto state_42;
   }

   if (ch.unicode() == 46) {
      goto state_43;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_44;
   }

   if (ch.unicode() == 92) {
      goto state_45;
   }

   if (ch.unicode() == 95) {
      goto state_46;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_46;
   }

   goto out;

state_17:
   ch = next();

   if (ch.unicode() == 33) {
      goto state_47;
   }

   goto out;

state_19:
   tmpIter = pos;
   token   = QCss::GREATER;
   goto out;

state_20:
   ch = next();

   if (ch.unicode() == 45) {
      goto state_48;
   }

   if (ch.unicode() == 92) {
      goto state_49;
   }

   if (ch.unicode() == 95) {
      goto state_50;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_50;
   }

   goto out;

state_22:
   ch = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_51;
   }

   if (ch.unicode() == 11) {
      goto state_51;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 47) {
      goto state_51;
   }

   if (ch.unicode() >= 58 && ch.unicode() <= 96) {
      goto state_51;
   }

   if (ch.unicode() >= 103) {
      goto state_51;
   }

   goto out;

state_24:
   tmpIter = pos;
   token   = QCss::IDENT;
   ch      = next();

   if (ch.unicode() == 40) {
      goto state_52;
   }

   if (ch.unicode() == 45) {
      goto state_53;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_53;
   }

   if (ch.unicode() == 92) {
      goto state_54;
   }

   if (ch.unicode() == 95) {
      goto state_53;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_53;
   }

   goto out;

state_25:
   tmpIter = pos;
   token   = QCss::LBRACE;
   goto out;

state_26:
   tmpIter = pos;
   token   = QCss::OR;
   ch      = next();

   if (ch.unicode() == 61) {
      token = QCss::DASHMATCH;
      goto found;
   }

   goto out;

state_28:
   ch = next();

   if (ch.unicode() == 61) {
      token = QCss::INCLUDES;
      goto found;
   }

   goto out;

state_29:
   tmpIter = pos;
   token   = QCss::S;
   ch      = next();

   if (ch.unicode() >= 9 && ch.unicode() <= 10) {
      goto state_29;
   }

   if (ch.unicode() >= 12 && ch.unicode() <= 13) {
      goto state_29;
   }

   if (ch.unicode() == 32) {
      goto state_29;
   }

   if (ch.unicode() == 43) {
      goto state_9;
   }

   if (ch.unicode() == 44) {
      goto state_10;
   }

   if (ch.unicode() == 62) {
      goto state_19;
   }

   if (ch.unicode() == 123) {
      goto state_25;
   }

   goto out;

state_30:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_30;
   }

   if (ch.unicode() == 11) {
      goto state_30;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 33) {
      goto state_30;
   }

   if (ch.unicode() == 34) {
      goto state_31;
   }

   if (ch.unicode() >= 35 && ch.unicode() <= 91) {
      goto state_30;
   }

   if (ch.unicode() == 92) {
      goto state_32;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_30;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_30;
   }

   if (ch.unicode() >= 123) {
      goto state_30;
   }

   goto out;

state_31:
   tmpIter = pos;
   token   = QCss::STRING;
   goto out;

state_32:
   ch = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_57;
   }

   if (ch.unicode() == 10) {
      goto state_58;
   }

   if (ch.unicode() == 11) {
      goto state_57;
   }

   if (ch.unicode() == 12) {
      goto state_59;
   }

   if (ch.unicode() == 13) {
      goto state_60;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 47) {
      goto state_57;
   }

   if (ch.unicode() >= 58 && ch.unicode() <= 96) {
      goto state_57;
   }

   if (ch.unicode() >= 103) {
      goto state_57;
   }

   goto out;

state_33:
   tmpIter = pos;
   token   = QCss::HASH;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_61;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_61;
   }

   if (ch.unicode() == 92) {
      goto state_62;
   }

   if (ch.unicode() == 95) {
      goto state_61;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_61;
   }

   goto out;

state_34:
   ch = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_63;
   }

   if (ch.unicode() == 11) {
      goto state_63;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 47) {
      goto state_63;
   }

   if (ch.unicode() >= 58 && ch.unicode() <= 96) {
      goto state_63;
   }

   if (ch.unicode() >= 103) {
      goto state_63;
   }

   goto out;

state_35:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_35;
   }

   if (ch.unicode() == 11) {
      goto state_35;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 38) {
      goto state_35;
   }

   if (ch.unicode() == 39) {
      goto state_36;
   }

   if (ch.unicode() >= 40 && ch.unicode() <= 91) {
      goto state_35;
   }

   if (ch.unicode() == 92) {
      goto state_37;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_35;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_35;
   }

   if (ch.unicode() >= 123) {
      goto state_35;
   }

   goto out;

state_36:
   tmpIter = pos;
   token   = QCss::STRING;
   goto out;

state_37:
   ch = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_64;
   }

   if (ch.unicode() == 10) {
      goto state_65;
   }

   if (ch.unicode() == 11) {
      goto state_64;
   }

   if (ch.unicode() == 12) {
      goto state_66;
   }

   if (ch.unicode() == 13) {
      goto state_67;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 47) {
      goto state_64;
   }

   if (ch.unicode() >= 58 && ch.unicode() <= 96) {
      goto state_64;
   }

   if (ch.unicode() >= 103) {
      goto state_64;
   }

   goto out;

state_38:
   ch = next();

   if (ch.unicode() == 62) {
      token = QCss::CDC;
      goto found;
   }

   goto out;

state_39:
   tmpIter = pos;
   token   = QCss::NUMBER;
   ch      = next();

   if (ch.unicode() == 37) {
      goto state_41;
   }

   if (ch.unicode() == 45) {
      goto state_42;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_69;
   }

   if (ch.unicode() == 92) {
      goto state_45;
   }

   if (ch.unicode() == 95) {
      goto state_46;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_46;
   }

   goto out;

state_41:
   tmpIter = pos;
   token   = QCss::PERCENTAGE;

   goto out;

state_42:
   ch = next();

   if (ch.unicode() == 92) {
      goto state_45;
   }

   if (ch.unicode() == 95) {
      goto state_46;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_46;
   }

   goto out;

state_43:
   ch = next();

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_39;
   }

   goto out;

state_44:
   tmpIter = pos;
   token   = QCss::NUMBER;
   ch      = next();

   if (ch.unicode() == 37) {
      goto state_41;
   }

   if (ch.unicode() == 45) {
      goto state_42;
   }

   if (ch.unicode() == 46) {
      goto state_43;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_44;
   }

   if (ch.unicode() == 92) {
      goto state_45;
   }

   if (ch.unicode() == 95) {
      goto state_46;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_46;
   }

   goto out;

state_45:
   ch = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_70;
   }

   if (ch.unicode() == 11) {
      goto state_70;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 47) {
      goto state_70;
   }

   if (ch.unicode() >= 58 && ch.unicode() <= 96) {
      goto state_70;
   }

   if (ch.unicode() >= 103) {
      goto state_70;
   }

   goto out;

state_46:
   tmpIter = pos;
   token   = QCss::LENGTH;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_71;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_71;
   }

   if (ch.unicode() == 92) {
      goto state_72;
   }

   if (ch.unicode() == 95) {
      goto state_71;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_71;
   }

   goto out;

state_47:
   ch = next();

   if (ch.unicode() == 45) {
      goto state_73;
   }

   goto out;

state_48:
   ch = next();

   if (ch.unicode() == 92) {
      goto state_49;
   }

   if (ch.unicode() == 95) {
      goto state_50;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_50;
   }

   goto out;

state_49:
   ch = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_74;
   }

   if (ch.unicode() == 11) {
      goto state_74;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 47) {
      goto state_74;
   }

   if (ch.unicode() >= 58 && ch.unicode() <= 96) {
      goto state_74;
   }

   if (ch.unicode() >= 103) {
      goto state_74;
   }

   goto out;

state_50:
   tmpIter = pos;
   token   = QCss::ATKEYWORD_SYM;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_75;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_75;
   }

   if (ch.unicode() == 92) {
      goto state_76;
   }

   if (ch.unicode() == 95) {
      goto state_75;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_75;
   }

   goto out;

state_51:
   tmpIter = pos;
   token   = QCss::IDENT;
   ch      = next();

   if (ch.unicode() == 40) {
      goto state_52;
   }

   if (ch.unicode() == 45) {
      goto state_53;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_53;
   }

   if (ch.unicode() == 92) {
      goto state_54;
   }

   if (ch.unicode() == 95) {
      goto state_53;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_53;
   }

   goto out;

state_52:
   tmpIter = pos;
   token   =  QCss::FUNCTION;
   goto out;

state_53:
   tmpIter = pos;
   token   = QCss::IDENT;
   ch      = next();

   if (ch.unicode() == 40) {
      goto state_52;
   }

   if (ch.unicode() == 45) {
      goto state_53;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_53;
   }

   if (ch.unicode() == 92) {
      goto state_54;
   }

   if (ch.unicode() == 95) {
      goto state_53;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_53;
   }

   goto out;

state_54:
   ch = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_77;
   }

   if (ch.unicode() == 11) {
      goto state_77;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 47) {
      goto state_77;
   }

   if (ch.unicode() >= 58 && ch.unicode() <= 96) {
      goto state_77;
   }

   if (ch.unicode() >= 103) {
      goto state_77;
   }

   goto out;

state_57:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_30;
   }

   if (ch.unicode() == 11) {
      goto state_30;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 33) {
      goto state_30;
   }

   if (ch.unicode() == 34) {
      goto state_31;
   }

   if (ch.unicode() >= 35 && ch.unicode() <= 91) {
      goto state_30;
   }

   if (ch.unicode() == 92) {
      goto state_32;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_30;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_30;
   }

   if (ch.unicode() >= 123) {
      goto state_30;
   }

   goto out;

state_58:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_30;
   }

   if (ch.unicode() == 11) {
      goto state_30;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 33) {
      goto state_30;
   }

   if (ch.unicode() == 34) {
      goto state_31;
   }

   if (ch.unicode() >= 35 && ch.unicode() <= 91) {
      goto state_30;
   }

   if (ch.unicode() == 92) {
      goto state_32;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_30;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_30;
   }

   if (ch.unicode() >= 123) {
      goto state_30;
   }

   goto out;

state_59:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_30;
   }

   if (ch.unicode() == 11) {
      goto state_30;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 33) {
      goto state_30;
   }

   if (ch.unicode() == 34) {
      goto state_31;
   }

   if (ch.unicode() >= 35 && ch.unicode() <= 91) {
      goto state_30;
   }

   if (ch.unicode() == 92) {
      goto state_32;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_30;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_30;
   }

   if (ch.unicode() >= 123) {
      goto state_30;
   }

   goto out;

state_60:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_30;
   }

   if (ch.unicode() == 10) {
      goto state_78;
   }

   if (ch.unicode() == 11) {
      goto state_30;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 33) {
      goto state_30;
   }

   if (ch.unicode() == 34) {
      goto state_31;
   }

   if (ch.unicode() >= 35 && ch.unicode() <= 91) {
      goto state_30;
   }

   if (ch.unicode() == 92) {
      goto state_32;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_30;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_30;
   }

   if (ch.unicode() >= 123) {
      goto state_30;
   }

   goto out;

state_61:
   tmpIter = pos;
   token   = QCss::HASH;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_61;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_61;
   }

   if (ch.unicode() == 92) {
      goto state_62;
   }

   if (ch.unicode() == 95) {
      goto state_61;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_61;
   }

   goto out;

state_62:
   ch = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_79;
   }

   if (ch.unicode() == 11) {
      goto state_79;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 47) {
      goto state_79;
   }

   if (ch.unicode() >= 58 && ch.unicode() <= 96) {
      goto state_79;
   }

   if (ch.unicode() >= 103) {
      goto state_79;
   }

   goto out;

state_63:
   tmpIter = pos;
   token   = QCss::HASH;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_61;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_61;
   }

   if (ch.unicode() == 92) {
      goto state_62;
   }

   if (ch.unicode() == 95) {
      goto state_61;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_61;
   }

   goto out;

state_64:
   tmpIter = pos;
   token = QCss::INVALID;
   ch = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_35;
   }

   if (ch.unicode() == 11) {
      goto state_35;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 38) {
      goto state_35;
   }

   if (ch.unicode() == 39) {
      goto state_36;
   }

   if (ch.unicode() >= 40 && ch.unicode() <= 91) {
      goto state_35;
   }

   if (ch.unicode() == 92) {
      goto state_37;
   }
   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_35;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_35;
   }

   if (ch.unicode() >= 123) {
      goto state_35;
   }

   goto out;

state_65:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_35;
   }

   if (ch.unicode() == 11) {
      goto state_35;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 38) {
      goto state_35;
   }

   if (ch.unicode() == 39) {
      goto state_36;
   }

   if (ch.unicode() >= 40 && ch.unicode() <= 91) {
      goto state_35;
   }

   if (ch.unicode() == 92) {
      goto state_37;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_35;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_35;
   }

   if (ch.unicode() >= 123) {
      goto state_35;
   }

   goto out;

state_66:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_35;
   }

   if (ch.unicode() == 11) {
      goto state_35;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 38) {
      goto state_35;
   }

   if (ch.unicode() == 39) {
      goto state_36;
   }

   if (ch.unicode() >= 40 && ch.unicode() <= 91) {
      goto state_35;
   }

   if (ch.unicode() == 92) {
      goto state_37;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_35;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_35;
   }

   if (ch.unicode() >= 123) {
      goto state_35;
   }

   goto out;

state_67:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_35;
   }

   if (ch.unicode() == 10) {
      goto state_80;
   }

   if (ch.unicode() == 11) {
      goto state_35;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 38) {
      goto state_35;
   }

   if (ch.unicode() == 39) {
      goto state_36;
   }

   if (ch.unicode() >= 40 && ch.unicode() <= 91) {
      goto state_35;
   }

   if (ch.unicode() == 92) {
      goto state_37;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_35;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_35;
   }

   if (ch.unicode() >= 123) {
      goto state_35;
   }

   goto out;

state_69:
   tmpIter = pos;
   token   = QCss::NUMBER;
   ch      = next();

   if (ch.unicode() == 37) {
      goto state_41;
   }

   if (ch.unicode() == 45) {
      goto state_42;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_69;
   }

   if (ch.unicode() == 92) {
      goto state_45;
   }

   if (ch.unicode() == 95) {
      goto state_46;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_46;
   }

   goto out;

state_70:
   tmpIter = pos;
   token   = QCss::LENGTH;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_71;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_71;
   }

   if (ch.unicode() == 92) {
      goto state_72;
   }

   if (ch.unicode() == 95) {
      goto state_71;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_71;
   }

   goto out;

state_71:
   tmpIter = pos;
   token   = QCss::LENGTH;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_71;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_71;
   }

   if (ch.unicode() == 92) {
      goto state_72;
   }

   if (ch.unicode() == 95) {
      goto state_71;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_71;
   }

   goto out;

state_72:
   ch = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_81;
   }

   if (ch.unicode() == 11) {
      goto state_81;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 47) {
      goto state_81;
   }

   if (ch.unicode() >= 58 && ch.unicode() <= 96) {
      goto state_81;
   }

   if (ch.unicode() >= 103) {
      goto state_81;
   }

   goto out;

state_73:
   ch = next();

   if (ch.unicode() == 45) {
      token = QCss::CDO;
      goto found;
   }

   goto out;

state_74:
   tmpIter = pos;
   token   = QCss::ATKEYWORD_SYM;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_75;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_75;
   }

   if (ch.unicode() == 92) {
      goto state_76;
   }

   if (ch.unicode() == 95) {
      goto state_75;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_75;
   }

   goto out;

state_75:
   tmpIter = pos;
   token   = QCss::ATKEYWORD_SYM;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_75;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_75;
   }

   if (ch.unicode() == 92) {
      goto state_76;
   }

   if (ch.unicode() == 95) {
      goto state_75;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_75;
   }

   goto out;

state_76:
   ch = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_83;
   }

   if (ch.unicode() == 11) {
      goto state_83;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 47) {
      goto state_83;
   }

   if (ch.unicode() >= 58 && ch.unicode() <= 96) {
      goto state_83;
   }
   if (ch.unicode() >= 103) {
      goto state_83;
   }

   goto out;

state_77:
   tmpIter = pos;
   token   = QCss::IDENT;
   ch      = next();

   if (ch.unicode() == 40) {
      goto state_52;
   }

   if (ch.unicode() == 45) {
      goto state_53;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_53;
   }

   if (ch.unicode() == 92) {
      goto state_54;
   }

   if (ch.unicode() == 95) {
      goto state_53;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_53;
   }

   goto out;

state_78:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_30;
   }   if (ch.unicode() == 11) {
      goto state_30;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 33) {
      goto state_30;
   }

   if (ch.unicode() == 34) {
      goto state_31;
   }

   if (ch.unicode() >= 35 && ch.unicode() <= 91) {
      goto state_30;
   }

   if (ch.unicode() == 92) {
      goto state_32;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_30;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_30;
   }

   if (ch.unicode() >= 123) {
      goto state_30;
   }

   goto out;

state_79:
   tmpIter = pos;
   token   = QCss::HASH;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_61;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_61;
   }

   if (ch.unicode() == 92) {
      goto state_62;
   }

   if (ch.unicode() == 95) {
      goto state_61;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_61;
   }

   goto out;

state_80:
   tmpIter = pos;
   token   = QCss::INVALID;
   ch      = next();

   if (ch.unicode() >= 1 && ch.unicode() <= 9) {
      goto state_35;
   }

   if (ch.unicode() == 11) {
      goto state_35;
   }

   if (ch.unicode() >= 14 && ch.unicode() <= 38) {
      goto state_35;
   }

   if (ch.unicode() == 39) {
      goto state_36;
   }

   if (ch.unicode() >= 40 && ch.unicode() <= 91) {
      goto state_35;
   }

   if (ch.unicode() == 92) {
      goto state_37;
   }

   if (ch.unicode() >= 93 && ch.unicode() <= 96) {
      goto state_35;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_35;
   }

   if (ch.unicode() >= 123) {
      goto state_35;
   }

   goto out;

state_81:
   tmpIter = pos;
   token   = QCss::LENGTH;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_71;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_71;
   }

   if (ch.unicode() == 92) {
      goto state_72;
   }

   if (ch.unicode() == 95) {
      goto state_71;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_71;
   }

   goto out;

state_83:
   tmpIter = pos;
   token   = QCss::ATKEYWORD_SYM;
   ch      = next();

   if (ch.unicode() == 45) {
      goto state_75;
   }

   if (ch.unicode() >= 48 && ch.unicode() <= 57) {
      goto state_75;
   }

   if (ch.unicode() == 92) {
      goto state_76;
   }

   if (ch.unicode() == 95) {
      goto state_75;
   }

   if ((ch.unicode() >= 'a' && ch.unicode() <= 'z') ||
         (ch.unicode() >= 'A' && ch.unicode() <= 'Z') || ch.unicode() >= 256) {
      goto state_75;
   }

   goto out;

found:
   tmpIter = pos;

out:
   if (tmpIter.has_value()) {
      lexemLength = tmpIter.value() - lexemStart;
      pos = tmpIter.value();
   }

   return token;
}
