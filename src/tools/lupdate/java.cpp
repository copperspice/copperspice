/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "lupdate.h"

#include <translator.h>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QRegExp>
#include <QtCore/QStack>
#include <QtCore/QStack>
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <QtCore/QCoreApplication>

#include <iostream>

#include <ctype.h>

QT_BEGIN_NAMESPACE

class LU
{
   Q_DECLARE_TR_FUNCTIONS(LUpdate)
};

enum { Tok_Eof, Tok_class, Tok_return, Tok_tr,
       Tok_translate, Tok_Ident, Tok_Package,
       Tok_Comment, Tok_String, Tok_Colon, Tok_Dot,
       Tok_LeftBrace, Tok_RightBrace, Tok_LeftParen,
       Tok_RightParen, Tok_Comma, Tok_Semicolon,
       Tok_Integer, Tok_Plus, Tok_PlusPlus, Tok_PlusEq, Tok_null
     };

class Scope
{
 public:
   QString name;
   enum Type {Clazz, Function, Other} type;
   int line;

   Scope(const QString &name, Type type, int line) :
      name(name),
      type(type),
      line(line) {
   }

   ~Scope() {
   }
};

/*
  The tokenizer maintains the following global variables. The names
  should be self-explanatory.
*/

static QString yyFileName;
static QChar yyCh;
static QString yyIdent;
static QString yyComment;
static QString yyString;


static qint64 yyInteger;
static int yyParenDepth;
static int yyLineNo;
static int yyCurLineNo;
static int yyParenLineNo;
static int yyTok;

// the string to read from and current position in the string
static QString yyInStr;
static int yyInPos;

// The parser maintains the following global variables.
static QString yyPackage;
static QStack<Scope *> yyScope;
static QString yyDefaultContext;

std::ostream &yyMsg(int line = 0)
{
   return std::cerr << qPrintable(yyFileName) << ':' << (line ? line : yyLineNo) << ": ";
}

static QChar getChar()
{
   if (yyInPos >= yyInStr.size()) {
      return EOF;
   }
   QChar c = yyInStr[yyInPos++];
   if (c.unicode() == '\n') {
      ++yyCurLineNo;
   }
   return c.unicode();
}

static int getToken()
{
   const char tab[] = "bfnrt\"\'\\";
   const char backTab[] = "\b\f\n\r\t\"\'\\";

   yyIdent.clear();
   yyComment.clear();
   yyString.clear();

   while ( yyCh != EOF ) {
      yyLineNo = yyCurLineNo;

      if ( yyCh.isLetter() || yyCh.toLatin1() == '_' ) {
         do {
            yyIdent.append(yyCh);
            yyCh = getChar();
         } while ( yyCh.isLetterOrNumber() || yyCh.toLatin1() == '_' );

         if (yyTok != Tok_Dot) {
            switch ( yyIdent.at(0).toLatin1() ) {
               case 'r':
                  if ( yyIdent == QLatin1String("return") ) {
                     return Tok_return;
                  }
                  break;
               case 'c':
                  if ( yyIdent == QLatin1String("class") ) {
                     return Tok_class;
                  }
                  break;
               case 'n':
                  if ( yyIdent == QLatin1String("null") ) {
                     return Tok_null;
                  }
                  break;
            }
         }
         switch ( yyIdent.at(0).toLatin1() ) {
            case 'T':
               // TR() for when all else fails
               if ( yyIdent == QLatin1String("TR") ) {
                  return Tok_tr;
               }
               break;
            case 'p':
               if ( yyIdent == QLatin1String("package") ) {
                  return Tok_Package;
               }
               break;
            case 't':
               if ( yyIdent == QLatin1String("tr") ) {
                  return Tok_tr;
               }
               if ( yyIdent == QLatin1String("translate") ) {
                  return Tok_translate;
               }
         }
         return Tok_Ident;
      } else {
         switch ( yyCh.toLatin1() ) {

            case '/':
               yyCh = getChar();
               if ( yyCh == QLatin1Char('/') ) {
                  do {
                     yyCh = getChar();
                     if (yyCh == EOF) {
                        break;
                     }
                     yyComment.append(yyCh);
                  } while (yyCh != QLatin1Char('\n'));
                  return Tok_Comment;

               } else if ( yyCh == QLatin1Char('*') ) {
                  bool metAster = false;
                  bool metAsterSlash = false;

                  while ( !metAsterSlash ) {
                     yyCh = getChar();
                     if ( yyCh == EOF ) {
                        yyMsg() << qPrintable(LU::tr("Unterminated Java comment.\n"));
                        return Tok_Comment;
                     }

                     yyComment.append( yyCh );

                     if ( yyCh == QLatin1Char('*') ) {
                        metAster = true;
                     } else if ( metAster && yyCh == QLatin1Char('/') ) {
                        metAsterSlash = true;
                     } else {
                        metAster = false;
                     }
                  }
                  yyComment.chop(2);
                  yyCh = getChar();

                  return Tok_Comment;
               }
               break;
            case '"':
               yyCh = getChar();

               while ( yyCh != EOF && yyCh != QLatin1Char('\n') && yyCh != QLatin1Char('"') ) {
                  if ( yyCh == QLatin1Char('\\') ) {
                     yyCh = getChar();
                     if ( yyCh == QLatin1Char('u') ) {
                        yyCh = getChar();
                        uint unicode(0);
                        for (int i = 4; i > 0; --i) {
                           unicode = unicode << 4;
                           if ( yyCh.isDigit() ) {
                              unicode += yyCh.digitValue();
                           } else {
                              int sub(yyCh.toLower().toLatin1() - 87);
                              if ( sub > 15 || sub < 10) {
                                 yyMsg() << qPrintable(LU::tr("Invalid Unicode value.\n"));
                                 break;
                              }
                              unicode += sub;
                           }
                           yyCh = getChar();
                        }
                        yyString.append(QChar(unicode));
                     } else if ( yyCh == QLatin1Char('\n') ) {
                        yyCh = getChar();
                     } else {
                        yyString.append( QLatin1Char(backTab[strchr( tab, yyCh.toLatin1() ) - tab]) );
                        yyCh = getChar();
                     }
                  } else {
                     yyString.append(yyCh);
                     yyCh = getChar();
                  }
               }

               if ( yyCh != QLatin1Char('"') ) {
                  yyMsg() << qPrintable(LU::tr("Unterminated string.\n"));
               }

               yyCh = getChar();

               return Tok_String;

            case ':':
               yyCh = getChar();
               return Tok_Colon;
            case '\'':
               yyCh = getChar();

               if ( yyCh == QLatin1Char('\\') ) {
                  yyCh = getChar();
               }
               do {
                  yyCh = getChar();
               } while ( yyCh != EOF && yyCh != QLatin1Char('\'') );
               yyCh = getChar();
               break;
            case '{':
               yyCh = getChar();
               return Tok_LeftBrace;
            case '}':
               yyCh = getChar();
               return Tok_RightBrace;
            case '(':
               if (yyParenDepth == 0) {
                  yyParenLineNo = yyCurLineNo;
               }
               yyParenDepth++;
               yyCh = getChar();
               return Tok_LeftParen;
            case ')':
               if (yyParenDepth == 0) {
                  yyParenLineNo = yyCurLineNo;
               }
               yyParenDepth--;
               yyCh = getChar();
               return Tok_RightParen;
            case ',':
               yyCh = getChar();
               return Tok_Comma;
            case '.':
               yyCh = getChar();
               return Tok_Dot;
            case ';':
               yyCh = getChar();
               return Tok_Semicolon;
            case '+':
               yyCh = getChar();
               if (yyCh == QLatin1Char('+')) {
                  yyCh = getChar();
                  return Tok_PlusPlus;
               }
               if ( yyCh == QLatin1Char('=') ) {
                  yyCh = getChar();
                  return Tok_PlusEq;
               }
               return Tok_Plus;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
               QByteArray ba;
               ba += yyCh.toLatin1();
               yyCh = getChar();
               bool hex = yyCh == QLatin1Char('x');
               if ( hex ) {
                  ba += yyCh.toLatin1();
                  yyCh = getChar();
               }
               while ( hex ? isxdigit(yyCh.toLatin1()) : yyCh.isDigit() ) {
                  ba += yyCh.toLatin1();
                  yyCh = getChar();
               }
               bool ok;
               yyInteger = ba.toLongLong(&ok);
               if (ok) {
                  return Tok_Integer;
               }
               break;
            }
            default:
               yyCh = getChar();
         }
      }
   }
   return Tok_Eof;
}

static bool match( int t )
{
   bool matches = ( yyTok == t );
   if ( matches ) {
      yyTok = getToken();
   }
   return matches;
}

static bool matchString( QString &s )
{
   if ( yyTok != Tok_String ) {
      return false;
   }

   s = yyString;
   yyTok = getToken();
   while ( yyTok == Tok_Plus ) {
      yyTok = getToken();
      if (yyTok == Tok_String) {
         s += yyString;
      } else {
         yyMsg() << qPrintable(LU::tr(
                                  "String used in translation can contain only literals"
                                  " concatenated with other literals, not expressions or numbers.\n"));
         return false;
      }
      yyTok = getToken();
   }
   return true;
}

static bool matchStringOrNull(QString &s)
{
   bool matches = matchString(s);
   if (!matches) {
      matches = (yyTok == Tok_null);
      if (matches) {
         yyTok = getToken();
      }
   }
   return matches;
}

/*
 * match any expression that can return a number, which can be
 * 1. Literal number (e.g. '11')
 * 2. simple identifier (e.g. 'm_count')
 * 3. simple function call (e.g. 'size()' )
 * 4. function call on an object (e.g. 'list.size()')
 * 5. function call on an object (e.g. 'list->size()')
 *
 * Other cases:
 * size(2,4)
 * list().size()
 * list(a,b).size(2,4)
 * etc...
 */
static bool matchExpression()
{
   if (match(Tok_Integer)) {
      return true;
   }

   int parenlevel = 0;
   while (match(Tok_Ident) || parenlevel > 0) {
      if (yyTok == Tok_RightParen) {
         if (parenlevel == 0) {
            break;
         }
         --parenlevel;
         yyTok = getToken();
      } else if (yyTok == Tok_LeftParen) {
         yyTok = getToken();
         if (yyTok == Tok_RightParen) {
            yyTok = getToken();
         } else {
            ++parenlevel;
         }
      } else if (yyTok == Tok_Ident) {
         continue;
      } else if (parenlevel == 0) {
         return false;
      }
   }
   return true;
}

static const QString context()
{
   QString context(yyPackage);
   bool innerClass = false;
   for (int i = 0; i < yyScope.size(); ++i) {
      if (yyScope.at(i)->type == Scope::Clazz) {
         if (innerClass) {
            context.append(QLatin1String("$"));
         } else {
            context.append(QLatin1String("."));
         }

         context.append(yyScope.at(i)->name);
         innerClass = true;
      }
   }
   return context.isEmpty() ? yyDefaultContext : context;
}

static void recordMessage(
   Translator *tor, const QString &context, const QString &text, const QString &comment,
   const QString &extracomment, bool plural)
{
   TranslatorMessage msg(
      context, text, comment, QString(),
      yyFileName, yyLineNo, QStringList(),
      TranslatorMessage::Unfinished, plural);
   msg.setExtraComment(extracomment.simplified());
   tor->extend(msg);
}

static void parse( Translator *tor )
{
   QString text;
   QString com;
   QString extracomment;

   yyCh = getChar();

   yyTok = getToken();
   while ( yyTok != Tok_Eof ) {
      switch ( yyTok ) {
         case Tok_class:
            yyTok = getToken();
            if (yyTok == Tok_Ident) {
               yyScope.push(new Scope(yyIdent, Scope::Clazz, yyLineNo));
            } else {
               yyMsg() << qPrintable(LU::tr("'class' must be followed by a class name.\n"));
               break;
            }
            while (!match(Tok_LeftBrace)) {
               yyTok = getToken();
            }
            break;

         case Tok_tr:
            yyTok = getToken();
            if ( match(Tok_LeftParen) && matchString(text) ) {
               com.clear();
               bool plural = false;

               if ( match(Tok_RightParen) ) {
                  // no comment
               } else if (match(Tok_Comma) && matchStringOrNull(com)) {   //comment
                  if ( match(Tok_RightParen)) {
                     // ok,
                  } else if (match(Tok_Comma)) {
                     plural = true;
                  }
               }
               if (!text.isEmpty()) {
                  recordMessage(tor, context(), text, com, extracomment, plural);
               }
            }
            break;
         case Tok_translate: {
            QString contextOverride;
            yyTok = getToken();
            if ( match(Tok_LeftParen) &&
                  matchString(contextOverride) &&
                  match(Tok_Comma) &&
                  matchString(text) ) {

               com.clear();
               bool plural = false;
               if (!match(Tok_RightParen)) {
                  // look for comment
                  if ( match(Tok_Comma) && matchStringOrNull(com)) {
                     if (!match(Tok_RightParen)) {
                        if (match(Tok_Comma) && matchExpression() && match(Tok_RightParen)) {
                           plural = true;
                        } else {
                           break;
                        }
                     }
                  } else {
                     break;
                  }
               }
               if (!text.isEmpty()) {
                  recordMessage(tor, contextOverride, text, com, extracomment, plural);
               }
            }
         }
         break;

         case Tok_Ident:
            yyTok = getToken();
            break;

         case Tok_Comment:
            if (yyComment.startsWith(QLatin1Char(':'))) {
               yyComment.remove(0, 1);
               extracomment.append(yyComment);
            }
            yyTok = getToken();
            break;

         case Tok_RightBrace:
            if ( yyScope.isEmpty() ) {
               yyMsg() << qPrintable(LU::tr("Excess closing brace.\n"));
            } else {
               delete (yyScope.pop());
            }
            extracomment.clear();
            yyTok = getToken();
            break;

         case Tok_LeftBrace:
            yyScope.push(new Scope(QString(), Scope::Other, yyLineNo));
            yyTok = getToken();
            break;

         case Tok_Semicolon:
            extracomment.clear();
            yyTok = getToken();
            break;

         case Tok_Package:
            yyTok = getToken();
            while (!match(Tok_Semicolon)) {
               switch (yyTok) {
                  case Tok_Ident:
                     yyPackage.append(yyIdent);
                     break;
                  case Tok_Dot:
                     yyPackage.append(QLatin1String("."));
                     break;
                  default:
                     yyMsg() << qPrintable(LU::tr("'package' must be followed by package name.\n"));
                     break;
               }
               yyTok = getToken();
            }
            break;

         default:
            yyTok = getToken();
      }
   }

   if ( !yyScope.isEmpty() ) {
      yyMsg(yyScope.top()->line) << qPrintable(LU::tr("Unbalanced opening brace.\n"));
   } else if ( yyParenDepth != 0 ) {
      yyMsg(yyParenLineNo) << qPrintable(LU::tr("Unbalanced opening parenthesis.\n"));
   }
}


bool loadJava(Translator &translator, const QString &filename, ConversionData &cd)
{
   QFile file(filename);
   if (!file.open(QIODevice::ReadOnly)) {
      cd.appendError(LU::tr("Cannot open %1: %2").arg(filename, file.errorString()));
      return false;
   }

   yyDefaultContext = cd.m_defaultContext;
   yyInPos = -1;
   yyFileName = filename;
   yyPackage.clear();
   yyScope.clear();
   yyTok = -1;
   yyParenDepth = 0;
   yyCurLineNo = 0;
   yyParenLineNo = 1;

   QTextStream ts(&file);
   QByteArray codecName;
   if (!cd.m_codecForSource.isEmpty()) {
      codecName = cd.m_codecForSource;
   } else {
      codecName = translator.codecName();   // Just because it should be latin1 already
   }
   ts.setCodec(QTextCodec::codecForName(codecName));
   ts.setAutoDetectUnicode(true);
   yyInStr = ts.readAll();
   yyInPos = 0;
   yyFileName = filename;
   yyCurLineNo = 1;
   yyParenLineNo = 1;

   parse(&translator);

   // Java uses UTF-16 internally and Jambi makes UTF-8 for tr() purposes of it.
   translator.setCodecName("UTF-8");
   return true;
}

QT_END_NAMESPACE
