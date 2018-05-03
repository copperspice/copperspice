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
#include <QtCore/QBitArray>
#include <QtCore/QDebug>
#include <QtCore/QFileInfo>
#include <QtCore/QStack>
#include <QtCore/QString>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QCoreApplication>

#include <iostream>
#include <ctype.h>              // for isXXX()

class LU
{
   Q_DECLARE_TR_FUNCTIONS(LUpdate)
};

static QString MagicComment("TRANSLATOR");

#define STRING(s) static QString str##s(QLatin1String(#s))

class HashString
{

 public:
   HashString() : m_hash(0x80000000) {}
   explicit HashString(const QString &str) : m_str(str), m_hash(0x80000000) {}

   void setValue(const QString &str) {
      m_str = str;
      m_hash = 0x80000000;
   }

   const QString &value() const {
      return m_str;
   }

   bool operator==(const HashString &other) const {
      return m_str == other.m_str;
   }

 private:
   QString m_str;

   mutable uint m_hash;
   friend uint qHash(const HashString &str);
};

uint qHash(const HashString &str)
{
   if (str.m_hash & 0x80000000) {
      str.m_hash = qHash(str.m_str) & 0x7fffffff;
   }
   return str.m_hash;
}

class HashStringList
{
 public:
   explicit HashStringList(const QList<HashString> &list)
      : m_list(list), m_hash(0x80000000)
   {}

   const QList<HashString> &value() const {
      return m_list;
   }

   bool operator==(const HashStringList &other) const {
      return m_list == other.m_list;
   }

 private:
   QList<HashString> m_list;
   mutable uint m_hash;
   friend uint qHash(const HashStringList &list);
};

uint qHash(const HashStringList &list)
{
   if (list.m_hash & 0x80000000) {
      uint hash = 0;

      for (const HashString & qs : list.m_list) {
         hash ^= qHash(qs) ^ 0x6ad9f526;
         hash = ((hash << 13) & 0x7fffffff) | (hash >> 18);
      }
      list.m_hash = hash;
   }
   return list.m_hash;
}

struct Namespace {

   Namespace() :
      classDef(this),
      hasTrFunctions(false), complained(false) {
   }

   ~Namespace() {
      qDeleteAll(children);
   }

   QHash<HashString, Namespace *> children;
   QHash<HashString, QList<HashString>> aliases;
   QList<HashStringList> usings;

   // Class declarations set no flags and create no namespaces, so they are ignored.
   // Class definitions may appear multiple times - but only because we are trying to
   // "compile" all sources irrespective of build configuration.
   // Nested classes may be forward-declared inside a definition, and defined in another file.
   // The latter will detach the class' child list, so clones need a backlink to the original
   // definition (either one in case of multiple definitions).
   // Namespaces can have tr() functions as well, so we need to track parent definitions for
   // them as well. The complication is that we may have to deal with a forrest instead of
   // a tree - in that case the parent will be arbitrary. However, it seem likely that
   // Q_DECLARE_TR_FUNCTIONS would be used either in "class-like" namespaces with a central
   // header or only locally in a file.

   Namespace *classDef;

   QString trQualification;

   bool hasTrFunctions;
   bool complained;             // that tr functions are missing.
};

static int nextFileId;

class VisitRecorder
{
 public:
   VisitRecorder() {
      m_ba.resize(nextFileId);
   }
   bool tryVisit(int fileId) {
      if (m_ba.at(fileId)) {
         return false;
      }
      m_ba[fileId] = true;
      return true;
   }

 private:
   QBitArray m_ba;
};

struct ParseResults {
   int fileId;
   Namespace rootNamespace;
   QSet<const ParseResults *> includes;
};

struct IncludeCycle {
   QSet<QString> fileNames;
   QSet<const ParseResults *> results;
};

typedef QHash<QString, IncludeCycle *>     IncludeCycleHash;
typedef QHash<QString, const Translator *> TranslatorHash;

class CppFiles
{

 public:
   static QSet<const ParseResults *> getResults(const QString &cleanFile);
   static void setResults(const QString &cleanFile, const ParseResults *results);
   static const Translator *getTranslator(const QString &cleanFile);
   static void setTranslator(const QString &cleanFile, const Translator *results);
   static bool isBlacklisted(const QString &cleanFile);
   static void setBlacklisted(const QString &cleanFile);
   static void addIncludeCycle(const QSet<QString> &fileNames);

 private:
   static IncludeCycleHash &includeCycles();
   static TranslatorHash &translatedFiles();
   static QSet<QString> &blacklistedFiles();
};

class CppParser
{

 public:
   CppParser(ParseResults *results = 0);
   void setInput(const QString &in);
   void setInput(QTextStream &ts, const QString &fileName);
   void setTranslator(Translator *_tor) {
      tor = _tor;
   }
   void parse(const QString &initialContext, ConversionData &cd, const QStringList &includeStack,
              QSet<QString> &inclusions);
   void parseInternal(ConversionData &cd, const QStringList &includeStack, QSet<QString> &inclusions);
   const ParseResults *recordResults(bool isHeader);
   void deleteResults() {
      delete results;
   }

   struct SavedState {
      QList<HashString> namespaces;
      QStack<int> namespaceDepths;
      QList<HashString> functionContext;
      QString functionContextUnresolved;
      QString pendingContext;
   };

 private:
   struct IfdefState {
      IfdefState() {}
      IfdefState(int _bracketDepth, int _braceDepth, int _parenDepth) :
         bracketDepth(_bracketDepth),
         braceDepth(_braceDepth),
         parenDepth(_parenDepth),
         elseLine(-1) {
      }

      SavedState state;
      int bracketDepth, bracketDepth1st;
      int braceDepth, braceDepth1st;
      int parenDepth, parenDepth1st;
      int elseLine;
   };

   std::ostream &yyMsg(int line = 0);

   QChar getChar();

   uint getToken();
   bool getMacroArgs();
   bool match(uint t);
   bool matchString(QString *s);
   bool matchEncoding(bool *utf8);
   bool matchStringOrNull(QString *s);
   bool matchExpression();

   QString transcode(const QString &str, bool utf8);
   void recordMessage(
      int line, const QString &context, const QString &text, const QString &comment,
      const QString &extracomment, const QString &msgid, const TranslatorMessage::ExtraData &extra,
      bool utf8, bool plural);

   void processInclude(const QString &file, ConversionData &cd,
                       const QStringList &includeStack, QSet<QString> &inclusions);

   void saveState(SavedState *state);
   void loadState(const SavedState *state);

   static QString stringifyNamespace(const QList<HashString> &namespaces);
   static QStringList stringListifyNamespace(const QList<HashString> &namespaces);

   typedef bool (CppParser::*VisitNamespaceCallback)(const Namespace *ns, void *context) const;

   bool visitNamespace(const QList<HashString> &namespaces, int nsCount,
                       VisitNamespaceCallback callback, void *context,
                       VisitRecorder &vr, const ParseResults *rslt) const;

   bool visitNamespace(const QList<HashString> &namespaces, int nsCount,
                       VisitNamespaceCallback callback, void *context) const;

   static QStringList stringListifySegments(const QList<HashString> &namespaces);
   bool qualifyOneCallbackOwn(const Namespace *ns, void *context) const;
   bool qualifyOneCallbackUsing(const Namespace *ns, void *context) const;

   bool qualifyOne(const QList<HashString> &namespaces, int nsCnt, const HashString &segment,
                   QList<HashString> *resolved, QSet<HashStringList> *visitedUsings) const;
   bool qualifyOne(const QList<HashString> &namespaces, int nsCnt, const HashString &segment,
                   QList<HashString> *resolved) const;
   bool fullyQualify(const QList<HashString> &namespaces, int nsCnt,
                     const QList<HashString> &segments, bool isDeclaration,
                     QList<HashString> *resolved, QStringList *unresolved) const;
   bool fullyQualify(const QList<HashString> &namespaces,
                     const QList<HashString> &segments, bool isDeclaration,
                     QList<HashString> *resolved, QStringList *unresolved) const;
   bool fullyQualify(const QList<HashString> &namespaces,
                     const QString &segments, bool isDeclaration,
                     QList<HashString> *resolved, QStringList *unresolved) const;

   bool findNamespaceCallback(const Namespace *ns, void *context) const;
   const Namespace *findNamespace(const QList<HashString> &namespaces, int nsCount = -1) const;
   void enterNamespace(QList<HashString> *namespaces, const HashString &name);
   void truncateNamespaces(QList<HashString> *namespaces, int length);
   Namespace *modifyNamespace(QList<HashString> *namespaces, bool haveLast = true);

   enum {
      Tok_Eof, Tok_class, Tok_friend, Tok_namespace, Tok_using, Tok_return,
      Tok_tr, Tok_trUtf8, Tok_translate, Tok_translateUtf8, Tok_trid,
      Tok_CS_OBJECT, Tok_Q_DECLARE_TR_FUNCTIONS,
      Tok_Ident, Tok_Comment, Tok_String, Tok_Arrow, Tok_Colon, Tok_ColonColon,
      Tok_Equals, Tok_LeftBracket, Tok_RightBracket,
      Tok_LeftBrace, Tok_RightBrace, Tok_LeftParen, Tok_RightParen, Tok_Comma, Tok_Semicolon,
      Tok_Null, Tok_Integer,
      Tok_QuotedInclude, Tok_AngledInclude,
      Tok_Other
   };

   // Tokenizer state
   QString yyFileName;

   QChar yyCh;

   bool yyAtNewline;
   bool yyCodecIsUtf8;
   bool yyForceUtf8;

   QString yyWord;
   qint64 yyInteger;
   QStack<IfdefState> yyIfdefStack;

   int yyBracketDepth;
   int yyBraceDepth;
   int yyParenDepth;
   int yyLineNo;
   int yyCurLineNo;
   int yyBracketLineNo;
   int yyBraceLineNo;
   int yyParenLineNo;

   // the string to read from and current position in the string
   QTextCodec *yySourceCodec;

   QString yyInStr;
   QString::const_iterator yyInPtr;

   // Parser state
   uint yyTok;

   QList<HashString> namespaces;
   QStack<int> namespaceDepths;
   QList<HashString> functionContext;
   QString functionContextUnresolved;
   QString prospectiveContext;
   QString pendingContext;
   ParseResults *results;
   Translator *tor;
   bool directInclude;

   SavedState savedState;
   int yyMinBraceDepth;
   bool inDefine;
};

CppParser::CppParser(ParseResults *_results)
{
   tor = 0;

   if (_results) {
      results = _results;
      directInclude = true;
   } else {
      results = new ParseResults;
      directInclude = false;
   }

   yyBracketDepth = 0;
   yyBraceDepth = 0;
   yyParenDepth = 0;
   yyCurLineNo = 1;
   yyBracketLineNo = 1;
   yyBraceLineNo = 1;
   yyParenLineNo = 1;
   yyAtNewline = true;
   yyMinBraceDepth = 0;
   inDefine = false;
}

std::ostream &CppParser::yyMsg(int line)
{
   return std::cerr << qPrintable(yyFileName) << ':' << (line ? line : yyLineNo) << ": ";
}

void CppParser::setInput(const QString &in)
{
   yyInStr = in;
   yyFileName = QString();
   yySourceCodec = 0;
   yyForceUtf8 = true;
}

void CppParser::setInput(QTextStream &ts, const QString &fileName)
{
   yyInStr = ts.readAll();
   yyFileName = fileName;
   yySourceCodec = ts.codec();
   yyForceUtf8 = false;
}

/*
  The first part of this source file is the C++ tokenizer.  We skip
  most of C++; the only tokens that interest us are defined here.
  Thus, the code fragment

      int main()
      {
          printf("Hello, world!\n");
          return 0;
      }

  is broken down into the following tokens (Tok_ omitted):

      Ident Ident LeftParen RightParen
      LeftBrace
          Ident LeftParen String RightParen Semicolon
          return Semicolon
      RightBrace.

  The 0 doesn't produce any token.
*/

QChar CppParser::getChar()
{
   QString::const_iterator iter = yyInPtr;

   while (true)  {

      if (iter == yyInStr.end()) {
         yyInPtr = iter;
         return EOF;
      }

      QChar c1 = *iter;
      ++iter;

      if (c1 == '\\') {
         QChar c2 = *iter;

         if (c2 == '\n') {
            ++yyCurLineNo;
            ++iter;

            continue;
         }

         if (c2 == '\r') {
            ++yyCurLineNo;
            ++iter;

            if (*iter == '\n') {
               ++iter;
            }

            continue;
         }
      }

      if (c1 == '\r') {
         QChar c2 = *iter;

         if (c2 == '\n') {
            ++iter;
         }

         c1 = '\n';
         ++yyCurLineNo;
         yyAtNewline = true;

      } else if (c1 == '\n') {
         ++yyCurLineNo;
         yyAtNewline = true;

      } else if (c1 != ' ' && c1 != '\t' && c1 != '#') {
         yyAtNewline = false;

      }

      yyInPtr = iter;

      return c1;
   }
}

// ignores commas, parens and comments.
// understands only a single, simple argument.
bool CppParser::getMacroArgs()
{
   yyWord = "";

   while (yyCh.isSpace()) {
      yyCh = getChar();
   }

   if (yyCh != '(') {
      return false;
   }

   yyCh = getChar();

   while (yyCh.isSpace()) {
      yyCh = getChar();
   }

   while (yyCh != ')') {
      if (yyCh == EOF) {
         return false;
      }

      yyWord.append(yyCh);
      yyCh = getChar();
   }

   yyCh = getChar();
   yyWord = yyWord.trimmed();

   return true;
}

STRING(CS_OBJECT);
STRING(Q_DECLARE_TR_FUNCTIONS);
STRING(QT_TR_NOOP);
STRING(QT_TRID_NOOP);
STRING(QT_TRANSLATE_NOOP);
STRING(QT_TRANSLATE_NOOP3);
STRING(QT_TR_NOOP_UTF8);
STRING(QT_TRANSLATE_NOOP_UTF8);
STRING(QT_TRANSLATE_NOOP3_UTF8);
STRING(class);

// QTranslator::findMessage() has the same parameters as QApplication::translate()

STRING(findMessage);
STRING(friend);
STRING(namespace);
STRING(operator);
STRING(qtTrId);
STRING(return);
STRING(struct);
STRING(TR);
STRING(Tr);
STRING(tr);
STRING(trUtf8);
STRING(translate);
STRING(using);

uint CppParser::getToken()
{

restart:
   yyWord.resize(0);

   while (yyCh != EOF) {
      yyLineNo = yyCurLineNo;

      if (yyCh == '#' && yyAtNewline) {
         /*
           Early versions of lupdate complained about
           unbalanced braces in the following code:

               #ifdef ALPHA
                   while (beta) {
               #else
                   while (gamma) {
               #endif
                       delta;
                   }

           The code contains, indeed, two opening braces for
           one closing brace; yet there's no reason to panic.

           The solution is to remember yyBraceDepth as it was
           when #if, #ifdef or #ifndef was met, and to set
           yyBraceDepth to that value when meeting #elif or
           #else.
         */

         do {
            yyCh = getChar();
         } while (yyCh.isSpace() && yyCh != '\n');

         switch (yyCh.unicode()) {

            case 'd': // define
               // Skip over the name of the define to avoid it being interpreted as c++ code

               do { // Rest of "define"
                  yyCh = getChar();

                  if (yyCh == EOF) {
                     return Tok_Eof;
                  }

                  if (yyCh == '\n') {
                     goto restart;
                  }

               } while (! yyCh.isSpace());

               do { // Space beween "define" and macro name
                  yyCh = getChar();

                  if (yyCh == EOF) {
                     return Tok_Eof;
                  }

                  if (yyCh == '\n') {
                     goto restart;
                  }

               } while (yyCh.isSpace());

               do { // Macro name

                  if (yyCh == '(') {
                     // Argument list. Follows the name without a space, and no paren nesting is possible.

                     do {
                        yyCh = getChar();

                        if (yyCh == EOF) {
                           return Tok_Eof;
                        }

                        if (yyCh == '\n') {
                           goto restart;
                        }
                     } while (yyCh != ')');

                     break;
                  }

                  yyCh = getChar();

                  if (yyCh == EOF) {
                     return Tok_Eof;
                  }

                  if (yyCh == '\n') {
                     goto restart;
                  }

               } while (! yyCh.isSpace());

               do { // Shortcut the immediate newline case if no comments follow.
                  yyCh = getChar();

                  if (yyCh == EOF) {
                     return Tok_Eof;
                  }

                  if (yyCh == '\n') {
                     goto restart;
                  }
               } while (yyCh.isSpace());

               saveState(&savedState);
               yyMinBraceDepth = yyBraceDepth;
               inDefine = true;
               goto restart;

            case 'i':
               yyCh = getChar();

               if (yyCh == 'f') {
                  // if, ifdef, ifndef
                  yyIfdefStack.push(IfdefState(yyBracketDepth, yyBraceDepth, yyParenDepth));
                  yyCh = getChar();

               } else if (yyCh == 'n') {
                  // include
                  do {
                     yyCh = getChar();
                  } while (yyCh != EOF && ! yyCh.isSpace());

                  do {
                     yyCh = getChar();
                  } while (yyCh.isSpace());

                  int tChar;

                  if (yyCh == '"') {
                     tChar = '"';
                  } else if (yyCh == '<') {
                     tChar = '>';
                  } else {
                     break;
                  }

                  yyWord = "";

                  while (true) {
                     yyCh = getChar();

                     if (yyCh == EOF || yyCh == '\n')  {
                        break;
                     }

                     if (yyCh == tChar) {
                        yyCh = getChar();
                        break;
                     }

                     yyWord.append(yyCh);
                  }

                  return (tChar == '"') ? Tok_QuotedInclude : Tok_AngledInclude;
               }
               break;

            case 'e':
               yyCh = getChar();

               if (yyCh == 'l') {
                  // elif, else
                  if (!yyIfdefStack.isEmpty()) {
                     IfdefState &is = yyIfdefStack.top();

                     if (is.elseLine != -1) {

                        if (yyBracketDepth != is.bracketDepth1st || yyBraceDepth != is.braceDepth1st
                              || yyParenDepth != is.parenDepth1st) {

                           yyMsg(is.elseLine) << qPrintable(LU::tr("Parenthesis/bracket/brace mismatch between "
                                                 "#if and #else branches; using #if branch\n"));
                        }

                     } else {
                        is.bracketDepth1st = yyBracketDepth;
                        is.braceDepth1st = yyBraceDepth;
                        is.parenDepth1st = yyParenDepth;
                        saveState(&is.state);
                     }

                     is.elseLine = yyLineNo;
                     yyBracketDepth = is.bracketDepth;
                     yyBraceDepth = is.braceDepth;
                     yyParenDepth = is.parenDepth;
                  }

                  yyCh = getChar();

               } else if (yyCh == 'n') {
                  // endif
                  if (!yyIfdefStack.isEmpty()) {
                     IfdefState is = yyIfdefStack.pop();
                     if (is.elseLine != -1) {
                        if (yyBracketDepth != is.bracketDepth1st
                              || yyBraceDepth != is.braceDepth1st
                              || yyParenDepth != is.parenDepth1st)
                           yyMsg(is.elseLine)
                                 << qPrintable(LU::tr("Parenthesis/brace mismatch between "
                                                      "#if and #else branches; using #if branch\n"));
                        yyBracketDepth = is.bracketDepth1st;
                        yyBraceDepth = is.braceDepth1st;
                        yyParenDepth = is.parenDepth1st;
                        loadState(&is.state);
                     }
                  }
                  yyCh = getChar();
               }
               break;
         }
         // Optimization: skip over rest of preprocessor directive
         do {
            if (yyCh == '/') {
               yyCh = getChar();

               if (yyCh == '/') {
                  do {
                     yyCh = getChar();

                  } while (yyCh != EOF && yyCh != '\n');
                  break;
               } else if (yyCh == '*') {
                  bool metAster = false;

                  forever {
                     yyCh = getChar();
                     if (yyCh == EOF)
                     {
                        yyMsg() << qPrintable(LU::tr("Unterminated C++ comment\n"));
                        break;
                     }

                     if (yyCh == '*')
                     {
                        metAster = true;
                     } else if (metAster && yyCh == '/')
                     {
                        yyCh = getChar();
                        break;
                     } else {
                        metAster = false;
                     }
                  }
               }
            } else {
               yyCh = getChar();
            }
         } while (yyCh != '\n' && yyCh != EOF);

         yyCh = getChar();

      } else if ((yyCh >= 'A' && yyCh <= 'Z') || (yyCh >= 'a' && yyCh <= 'z') || yyCh == '_') {
         yyWord = "";

         do {
            yyWord.append(yyCh);
            yyCh = getChar();

         } while ((yyCh >= 'A' && yyCh <= 'Z') || (yyCh >= 'a' && yyCh <= 'z') || (yyCh >= '0' && yyCh <= '9') || yyCh == '_');


         switch (yyWord[0].unicode()) {

            case 'C':
               if (yyWord == strCS_OBJECT) {
                  return Tok_CS_OBJECT;
               }
               break;

            case 'Q':
               if (yyWord == strQ_DECLARE_TR_FUNCTIONS) {
                  return Tok_Q_DECLARE_TR_FUNCTIONS;
               }
               if (yyWord == strQT_TR_NOOP) {
                  return Tok_tr;
               }
               if (yyWord == strQT_TRID_NOOP) {
                  return Tok_trid;
               }
               if (yyWord == strQT_TRANSLATE_NOOP) {
                  return Tok_translate;
               }
               if (yyWord == strQT_TRANSLATE_NOOP3) {
                  return Tok_translate;
               }
               if (yyWord == strQT_TR_NOOP_UTF8) {
                  return Tok_trUtf8;
               }
               if (yyWord == strQT_TRANSLATE_NOOP_UTF8) {
                  return Tok_translateUtf8;
               }
               if (yyWord == strQT_TRANSLATE_NOOP3_UTF8) {
                  return Tok_translateUtf8;
               }
               break;

            case 'T':
               // TR() for when all else fails
               if (yyWord == strTR || yyWord == strTr) {
                  return Tok_tr;
               }
               break;

            case 'c':
               if (yyWord == strclass) {
                  return Tok_class;
               }
               break;

            case 'f':
               /*
                 QTranslator::findMessage() has the same parameters as
                 QApplication::translate().
               */
               if (yyWord == strfindMessage) {
                  return Tok_translate;
               }
               if (yyWord == strfriend) {
                  return Tok_friend;
               }
               break;
            case 'n':
               if (yyWord == strnamespace) {
                  return Tok_namespace;
               }
               break;
            case 'o':
               if (yyWord == stroperator) {
                  // Operator overload declaration/definition.
                  // We need to prevent those characters from confusing the followup
                  // parsing. Actually using them does not add value, so just eat them.

                  while (yyCh.isSpace()) {
                     yyCh = getChar();
                  }

                  while (yyCh == '+' || yyCh == '-' || yyCh == '*' || yyCh == '/' || yyCh == '%'
                         || yyCh == '=' || yyCh == '<' || yyCh == '>' || yyCh == '!'
                         || yyCh == '&' || yyCh == '|' || yyCh == '~' || yyCh == '^'
                         || yyCh == '[' || yyCh == ']') {
                     yyCh = getChar();
                  }
               }
               break;
            case 'q':
               if (yyWord == strqtTrId) {
                  return Tok_trid;
               }
               break;
            case 'r':
               if (yyWord == strreturn) {
                  return Tok_return;
               }
               break;
            case 's':
               if (yyWord == strstruct) {
                  return Tok_class;
               }
               break;
            case 't':
               if (yyWord == strtr) {
                  return Tok_tr;
               }
               if (yyWord == strtrUtf8) {
                  return Tok_trUtf8;
               }
               if (yyWord == strtranslate) {
                  return Tok_translate;
               }
               break;
            case 'u':
               if (yyWord == strusing) {
                  return Tok_using;
               }
               break;
         }
         return Tok_Ident;

      } else {
         switch (yyCh.unicode()) {

            case '\n':
               if (inDefine) {
                  loadState(&savedState);
                  prospectiveContext.clear();
                  yyBraceDepth = yyMinBraceDepth;
                  yyMinBraceDepth = 0;
                  inDefine = false;
               }
               yyCh = getChar();
               break;

            case '/':
               yyCh = getChar();

               if (yyCh == '/') {

                  do {
                     yyCh = getChar();

                     if (yyCh == EOF) {
                        break;
                     }

                     yyWord.append(yyCh);

                  } while (yyCh != '\n');

               } else if (yyCh == '*') {
                  bool metAster = false;

                  while (true) {
                     yyCh = getChar();

                     if (yyCh == EOF) {
                        yyMsg() << qPrintable(LU::tr("Unterminated C++ comment\n"));
                        break;
                     }

                     yyWord.append(yyCh);

                     if (yyCh == '*') {
                        metAster = true;
                     } else if (metAster && yyCh == '/') {
                        break;
                     } else {
                        metAster = false;
                     }

                  }
                  yyCh = getChar();
               }
               return Tok_Comment;

            case '"': {
               yyCh = getChar();

               while (yyCh != EOF && yyCh != '\n' && yyCh != '"') {
                  if (yyCh == '\\') {
                     yyCh = getChar();

                     if (yyCh == EOF || yyCh == '\n') {
                        break;
                     }
                     yyWord.append('\\');
                  }

                  yyWord.append(yyCh);
                  yyCh = getChar();
               }

               if (yyCh != '"') {
                  yyMsg() << qPrintable(LU::tr("Unterminated C++ string\n"));
               } else {
                  yyCh = getChar();
               }
               return Tok_String;
            }

            case '-':
               yyCh = getChar();
               if (yyCh == '>') {
                  yyCh = getChar();
                  return Tok_Arrow;
               }
               break;
            case ':':
               yyCh = getChar();
               if (yyCh == ':') {
                  yyCh = getChar();
                  return Tok_ColonColon;
               }
               return Tok_Colon;
            // Incomplete: '<' might be part of '<=' or of template syntax.
            // The main intent of not completely ignoring it is to break
            // parsing of things like   std::cout << QObject::tr()  as
            // context std::cout::QObject (see Task 161106)
            case '=':
               yyCh = getChar();
               return Tok_Equals;
            case '>':
            case '<':
               yyCh = getChar();
               return Tok_Other;
            case '\'':
               yyCh = getChar();
               if (yyCh == '\\') {
                  yyCh = getChar();
               }

               forever {
                  if (yyCh == EOF || yyCh == '\n')
                  {
                     yyMsg() << "Unterminated C++ character\n";
                     break;
                  }
                  yyCh = getChar();
                  if (yyCh == '\'')
                  {
                     yyCh = getChar();
                     break;
                  }
               }
               break;
            case '{':
               if (yyBraceDepth == 0) {
                  yyBraceLineNo = yyCurLineNo;
               }
               yyBraceDepth++;
               yyCh = getChar();
               return Tok_LeftBrace;
            case '}':
               if (yyBraceDepth == yyMinBraceDepth) {
                  if (!inDefine)
                     yyMsg(yyCurLineNo)
                           << qPrintable(LU::tr("Excess closing brace in C++ code"
                                                " (or abuse of the C++ preprocessor)\n"));
                  // Avoid things getting messed up even more
                  yyCh = getChar();
                  return Tok_Semicolon;
               }
               yyBraceDepth--;
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
               if (yyParenDepth == 0)
                  yyMsg(yyCurLineNo)
                        << qPrintable(LU::tr("Excess closing parenthesis in C++ code"
                                             " (or abuse of the C++ preprocessor)\n"));
               else {
                  yyParenDepth--;
               }
               yyCh = getChar();
               return Tok_RightParen;
            case '[':
               if (yyBracketDepth == 0) {
                  yyBracketLineNo = yyCurLineNo;
               }
               yyBracketDepth++;
               yyCh = getChar();
               return Tok_LeftBracket;
            case ']':
               if (yyBracketDepth == 0)
                  yyMsg(yyCurLineNo)
                        << qPrintable(LU::tr("Excess closing bracket in C++ code"
                                             " (or abuse of the C++ preprocessor)\n"));
               else {
                  yyBracketDepth--;
               }
               yyCh = getChar();
               return Tok_RightBracket;
            case ',':
               yyCh = getChar();
               return Tok_Comma;
            case ';':
               yyCh = getChar();
               return Tok_Semicolon;
            case '0':
               yyCh = getChar();
               if (yyCh == 'x') {
                  do {
                     yyCh = getChar();
                  } while ((yyCh >= '0' && yyCh <= '9')
                           || (yyCh >= 'a' && yyCh <= 'f') || (yyCh >= 'A' && yyCh <= 'F'));
                  return Tok_Integer;
               }
               if (yyCh < '0' || yyCh > '9') {
                  return Tok_Null;
               }
            // Fallthrough
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
               do {
                  yyCh = getChar();
               } while (yyCh >= '0' && yyCh <= '9');
               return Tok_Integer;
            default:
               yyCh = getChar();
               break;
         }
      }
   }
   return Tok_Eof;
}

/*
  The second part of this source file are namespace/class related
  utilities for the third part.
*/

void CppParser::saveState(SavedState *state)
{
   state->namespaces = namespaces;
   state->namespaceDepths = namespaceDepths;
   state->functionContext = functionContext;
   state->functionContextUnresolved = functionContextUnresolved;
   state->pendingContext = pendingContext;
}

void CppParser::loadState(const SavedState *state)
{
   namespaces = state->namespaces;
   namespaceDepths = state->namespaceDepths;
   functionContext = state->functionContext;
   functionContextUnresolved = state->functionContextUnresolved;
   pendingContext = state->pendingContext;
}

Namespace *CppParser::modifyNamespace(QList<HashString> *namespaces, bool haveLast)
{
   Namespace *pns, *ns = &results->rootNamespace;
   for (int i = 1; i < namespaces->count(); ++i) {
      pns = ns;
      if (!(ns = pns->children.value(namespaces->at(i)))) {
         do {
            ns = new Namespace;
            if (haveLast || i < namespaces->count() - 1)
               if (const Namespace *ons = findNamespace(*namespaces, i + 1)) {
                  ns->classDef = ons->classDef;
               }
            pns->children.insert(namespaces->at(i), ns);
            pns = ns;
         } while (++i < namespaces->count());
         break;
      }
   }
   return ns;
}

QString CppParser::stringifyNamespace(const QList<HashString> &namespaces)
{
   QString ret;
   for (int i = 1; i < namespaces.count(); ++i) {
      if (i > 1) {
         ret += QLatin1String("::");
      }
      ret += namespaces.at(i).value();
   }
   return ret;
}

QStringList CppParser::stringListifyNamespace(const QList<HashString> &namespaces)
{
   QStringList ret;
   for (int i = 1; i < namespaces.count(); ++i) {
      ret << namespaces.at(i).value();
   }
   return ret;
}

bool CppParser::visitNamespace(const QList<HashString> &namespaces, int nsCount,
                               VisitNamespaceCallback callback, void *context,
                               VisitRecorder &vr, const ParseResults *rslt) const
{
   const Namespace *ns = &rslt->rootNamespace;
   for (int i = 1; i < nsCount; ++i)
      if (!(ns = ns->children.value(namespaces.at(i)))) {
         goto supers;
      }
   if ((this->*callback)(ns, context)) {
      return true;
   }
supers:
   foreach (const ParseResults * sup, rslt->includes)
   if (vr.tryVisit(sup->fileId)
         && visitNamespace(namespaces, nsCount, callback, context, vr, sup)) {
      return true;
   }
   return false;
}

bool CppParser::visitNamespace(const QList<HashString> &namespaces, int nsCount,
                               VisitNamespaceCallback callback, void *context) const
{
   VisitRecorder vr;
   return visitNamespace(namespaces, nsCount, callback, context, vr, results);
}

QStringList CppParser::stringListifySegments(const QList<HashString> &segments)
{
   QStringList ret;
   for (int i = 0; i < segments.count(); ++i) {
      ret << segments.at(i).value();
   }
   return ret;
}

struct QualifyOneData {
   QualifyOneData(const QList<HashString> &ns, int nsc, const HashString &seg, QList<HashString> *rslvd,
                  QSet<HashStringList> *visited)
      : namespaces(ns), nsCount(nsc), segment(seg), resolved(rslvd), visitedUsings(visited) {
   }

   const QList<HashString> &namespaces;
   int nsCount;
   const HashString &segment;
   QList<HashString> *resolved;
   QSet<HashStringList> *visitedUsings;
};

bool CppParser::qualifyOneCallbackOwn(const Namespace *ns, void *context) const
{
   QualifyOneData *data = (QualifyOneData *)context;
   if (ns->children.contains(data->segment)) {
      *data->resolved = data->namespaces.mid(0, data->nsCount);
      *data->resolved << data->segment;
      return true;
   }
   QHash<HashString, QList<HashString>>::const_iterator nsai = ns->aliases.constFind(data->segment);

   if (nsai != ns->aliases.constEnd()) {
      const QList<HashString> &nsl = *nsai;

      if (nsl.last().value().isEmpty()) { // Delayed alias resolution
         QList<HashString> &nslIn = *const_cast<QList<HashString> *>(&nsl);
         nslIn.removeLast();
         QList<HashString> nslOut;

         if (!fullyQualify(data->namespaces, data->nsCount, nslIn, false, &nslOut, 0)) {
            const_cast<Namespace *>(ns)->aliases.remove(data->segment);
            return false;
         }
         nslIn = nslOut;
      }
      *data->resolved = nsl;
      return true;
   }
   return false;
}

bool CppParser::qualifyOneCallbackUsing(const Namespace *ns, void *context) const
{
   QualifyOneData *data = (QualifyOneData *)context;
   foreach (const HashStringList & use, ns->usings)
   if (!data->visitedUsings->contains(use)) {
      data->visitedUsings->insert(use);
      if (qualifyOne(use.value(), use.value().count(), data->segment, data->resolved,
                     data->visitedUsings)) {
         return true;
      }
   }
   return false;
}

bool CppParser::qualifyOne(const QList<HashString> &namespaces, int nsCnt, const HashString &segment,
                           QList<HashString> *resolved, QSet<HashStringList> *visitedUsings) const
{
   QualifyOneData data(namespaces, nsCnt, segment, resolved, visitedUsings);

   if (visitNamespace(namespaces, nsCnt, &CppParser::qualifyOneCallbackOwn, &data)) {
      return true;
   }

   return visitNamespace(namespaces, nsCnt, &CppParser::qualifyOneCallbackUsing, &data);
}

bool CppParser::qualifyOne(const QList<HashString> &namespaces, int nsCnt, const HashString &segment,
                           QList<HashString> *resolved) const
{
   QSet<HashStringList> visitedUsings;

   return qualifyOne(namespaces, nsCnt, segment, resolved, &visitedUsings);
}

bool CppParser::fullyQualify(const QList<HashString> &namespaces, int nsCnt,
                             const QList<HashString> &segments, bool isDeclaration,
                             QList<HashString> *resolved, QStringList *unresolved) const
{
   int nsIdx;
   int initSegIdx;

   if (segments.first().value().isEmpty()) {
      // fully qualified
      if (segments.count() == 1) {
         resolved->clear();
         *resolved << HashString(QString());
         return true;
      }
      initSegIdx = 1;
      nsIdx = 0;
   } else {
      initSegIdx = 0;
      nsIdx = nsCnt - 1;
   }

   do {
      if (qualifyOne(namespaces, nsIdx + 1, segments[initSegIdx], resolved)) {
         int segIdx = initSegIdx;
         while (++segIdx < segments.count()) {
            if (!qualifyOne(*resolved, resolved->count(), segments[segIdx], resolved)) {
               if (unresolved) {
                  *unresolved = stringListifySegments(segments.mid(segIdx));
               }
               return false;
            }
         }
         return true;
      }
   } while (!isDeclaration && --nsIdx >= 0);
   resolved->clear();
   *resolved << HashString(QString());
   if (unresolved) {
      *unresolved = stringListifySegments(segments.mid(initSegIdx));
   }
   return false;
}

bool CppParser::fullyQualify(const QList<HashString> &namespaces,
                             const QList<HashString> &segments, bool isDeclaration,
                             QList<HashString> *resolved, QStringList *unresolved) const
{
   return fullyQualify(namespaces, namespaces.count(),
                       segments, isDeclaration, resolved, unresolved);
}

bool CppParser::fullyQualify(const QList<HashString> &namespaces,
                             const QString &quali, bool isDeclaration,
                             QList<HashString> *resolved, QStringList *unresolved) const
{
   static QString strColons(QLatin1String("::"));

   QList<HashString> segments;
   foreach (const QString & str, quali.split(strColons)) // XXX slow, but needs to be fast(?)
   segments << HashString(str);
   return fullyQualify(namespaces, segments, isDeclaration, resolved, unresolved);
}

bool CppParser::findNamespaceCallback(const Namespace *ns, void *context) const
{
   *((const Namespace **)context) = ns;
   return true;
}

const Namespace *CppParser::findNamespace(const QList<HashString> &namespaces, int nsCount) const
{
   const Namespace *ns = 0;
   if (nsCount == -1) {
      nsCount = namespaces.count();
   }
   visitNamespace(namespaces, nsCount, &CppParser::findNamespaceCallback, &ns);
   return ns;
}

void CppParser::enterNamespace(QList<HashString> *namespaces, const HashString &name)
{
   *namespaces << name;
   if (!findNamespace(*namespaces)) {
      modifyNamespace(namespaces, false);
   }
}

void CppParser::truncateNamespaces(QList<HashString> *namespaces, int length)
{
   if (namespaces->count() > length) {
      namespaces->erase(namespaces->begin() + length, namespaces->end());
   }
}

/*
  Functions for processing include files.
*/

IncludeCycleHash &CppFiles::includeCycles()
{
   static IncludeCycleHash cycles;

   return cycles;
}

TranslatorHash &CppFiles::translatedFiles()
{
   static TranslatorHash tors;

   return tors;
}

QSet<QString> &CppFiles::blacklistedFiles()
{
   static QSet<QString> blacklisted;

   return blacklisted;
}

QSet<const ParseResults *> CppFiles::getResults(const QString &cleanFile)
{
   IncludeCycle *const cycle = includeCycles().value(cleanFile);

   if (cycle) {
      return cycle->results;
   } else {
      return QSet<const ParseResults *>();
   }
}

void CppFiles::setResults(const QString &cleanFile, const ParseResults *results)
{
   IncludeCycle *cycle = includeCycles().value(cleanFile);

   if (!cycle) {
      cycle = new IncludeCycle;
      includeCycles().insert(cleanFile, cycle);
   }

   cycle->fileNames.insert(cleanFile);
   cycle->results.insert(results);
}

const Translator *CppFiles::getTranslator(const QString &cleanFile)
{
   return translatedFiles().value(cleanFile);
}

void CppFiles::setTranslator(const QString &cleanFile, const Translator *tor)
{
   translatedFiles().insert(cleanFile, tor);
}

bool CppFiles::isBlacklisted(const QString &cleanFile)
{
   return blacklistedFiles().contains(cleanFile);
}

void CppFiles::setBlacklisted(const QString &cleanFile)
{
   blacklistedFiles().insert(cleanFile);
}

void CppFiles::addIncludeCycle(const QSet<QString> &fileNames)
{
   IncludeCycle *const cycle = new IncludeCycle;
   cycle->fileNames = fileNames;

   QSet<IncludeCycle *> intersectingCycles;
   foreach (const QString & fileName, fileNames) {
      IncludeCycle *intersectingCycle = includeCycles().value(fileName);

      if (intersectingCycle && !intersectingCycles.contains(intersectingCycle)) {
         intersectingCycles.insert(intersectingCycle);

         cycle->fileNames.unite(intersectingCycle->fileNames);
         cycle->results.unite(intersectingCycle->results);
      }
   }
   qDeleteAll(intersectingCycles);

   foreach (const QString & fileName, cycle->fileNames)
   includeCycles().insert(fileName, cycle);
}

static bool isHeader(const QString &name)
{
   QString fileExt = QFileInfo(name).suffix();
   return fileExt.isEmpty() || fileExt.startsWith(QLatin1Char('h'), Qt::CaseInsensitive);
}

void CppParser::processInclude(const QString &file, ConversionData &cd, const QStringList &includeStack,
                               QSet<QString> &inclusions)
{
   QString cleanFile = QDir::cleanPath(file);

   const int index = includeStack.indexOf(cleanFile);
   if (index != -1) {
      CppFiles::addIncludeCycle(includeStack.mid(index).toSet());
      return;
   }

   // If the #include is in any kind of namespace, has been blacklisted previously,
   // or is not a header file (stdc++ extensionless or *.h*), then really include
   // it. Otherwise it is safe to process it stand-alone and re-use the parsed
   // namespace data for inclusion into other files.
   bool isIndirect = false;
   if (namespaces.count() == 1 && functionContext.count() == 1
         && functionContextUnresolved.isEmpty() && pendingContext.isEmpty()
         && !CppFiles::isBlacklisted(cleanFile)
         && isHeader(cleanFile)) {

      QSet<const ParseResults *> res = CppFiles::getResults(cleanFile);
      if (!res.isEmpty()) {
         results->includes.unite(res);
         return;
      }

      isIndirect = true;
   }

   QFile f(cleanFile);
   if (!f.open(QIODevice::ReadOnly)) {
      yyMsg() << qPrintable(LU::tr("Cannot open %1: %2\n").formatArgs(cleanFile, f.errorString()));
      return;
   }

   QTextStream ts(&f);
   ts.setCodec(yySourceCodec);
   ts.setAutoDetectUnicode(true);

   inclusions.insert(cleanFile);
   if (isIndirect) {
      CppParser parser;
      foreach (const QString & projectRoot, cd.m_projectRoots)
      if (cleanFile.startsWith(projectRoot)) {
         parser.setTranslator(new Translator);
         break;
      }
      parser.setInput(ts, cleanFile);
      QStringList stack = includeStack;
      stack << cleanFile;
      parser.parse(cd.m_defaultContext, cd, stack, inclusions);
      results->includes.insert(parser.recordResults(true));
   } else {
      CppParser parser(results);
      parser.namespaces = namespaces;
      parser.functionContext = functionContext;
      parser.functionContextUnresolved = functionContextUnresolved;
      parser.pendingContext = pendingContext;
      parser.setInput(ts, cleanFile);
      parser.setTranslator(tor);
      QStringList stack = includeStack;
      stack << cleanFile;
      parser.parseInternal(cd, stack, inclusions);
      // Avoid that messages obtained by direct scanning are used
      CppFiles::setBlacklisted(cleanFile);
   }
   inclusions.remove(cleanFile);
}

/*
  The third part of this source file is the parser. It accomplishes
  a very easy task: It finds all strings inside a tr() or translate()
  call, and possibly finds out the context of the call. It supports
  three cases: (1) the context is specified, as in
  FunnyDialog::tr("Hello") or translate("FunnyDialog", "Hello");
  (2) the call appears within an inlined function; (3) the call
  appears within a function defined outside the class definition.
*/

bool CppParser::match(uint t)
{
   bool matches = (yyTok == t);
   if (matches) {
      yyTok = getToken();
   }
   return matches;
}

bool CppParser::matchString(QString *s)
{
   bool matches = false;
   s->clear();

   while (true) {
      while (yyTok == Tok_Comment) {
         yyTok = getToken();
      }

      if (yyTok != Tok_String) {
         return matches;
      }

      matches = true;

      *s += yyWord;
      yyTok = getToken();
   }
}

STRING(QApplication);
STRING(QCoreApplication);
STRING(UnicodeUTF8);
STRING(DefaultCodec);
STRING(CodecForTr);

bool CppParser::matchEncoding(bool *utf8)
{
   if (yyTok != Tok_Ident) {
      return false;
   }

   if (yyWord == strQApplication || yyWord == strQCoreApplication) {
      yyTok = getToken();
      if (yyTok == Tok_ColonColon) {
         yyTok = getToken();
      }
   }

   if (yyWord == strUnicodeUTF8) {
      *utf8 = true;
      yyTok = getToken();
      return true;
   }

   if (yyWord == strDefaultCodec || yyWord == strCodecForTr) {
      *utf8 = false;
      yyTok = getToken();
      return true;
   }

   return false;
}

bool CppParser::matchStringOrNull(QString *s)
{
   return matchString(s) || match(Tok_Null);
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
bool CppParser::matchExpression()
{
   if (match(Tok_Null) || match(Tok_Integer)) {
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
      } else if (yyTok == Tok_Arrow) {
         yyTok = getToken();
      } else if (parenlevel == 0) {
         return false;
      }
   }
   return true;
}

QString CppParser::transcode(const QString &str, bool utf8)
{
   static const char tab[] = "abfnrtv";
   static const char backTab[] = "\a\b\f\n\r\t\v";
   // This function has to convert back to bytes, as C's \0* sequences work at that level.
   const QByteArray in = yyForceUtf8 ? str.toUtf8() : tor->codec()->fromUnicode(str);
   QByteArray out;

   out.reserve(in.length());
   for (int i = 0; i < in.length();) {
      uchar c = in[i++];
      if (c == '\\') {
         if (i >= in.length()) {
            break;
         }
         c = in[i++];

         if (c == '\n') {
            continue;
         }

         if (c == 'x') {
            QByteArray hex;
            while (i < in.length() && isxdigit((c = in[i]))) {
               hex += c;
               i++;
            }
            out += hex.toUInt(0, 16);
         } else if (c >= '0' && c < '8') {
            QByteArray oct;
            int n = 0;
            oct += c;
            while (n < 2 && i < in.length() && (c = in[i]) >= '0' && c < '8') {
               i++;
               n++;
               oct += c;
            }
            out += oct.toUInt(0, 8);
         } else {
            const char *p = strchr(tab, c);
            out += !p ? c : backTab[p - tab];
         }
      } else {
         out += c;
      }
   }
   return (utf8 || yyForceUtf8) ? QString::fromUtf8(out.constData(), out.length())
          : tor->codec()->toUnicode(out);
}

void CppParser::recordMessage(
   int line, const QString &context, const QString &text, const QString &comment,
   const QString &extracomment, const QString &msgid, const TranslatorMessage::ExtraData &extra,
   bool utf8, bool plural)
{
   TranslatorMessage msg(
      transcode(context, utf8), transcode(text, utf8), transcode(comment, utf8), QString(),
      yyFileName, line, QStringList(),
      TranslatorMessage::Unfinished, plural);
   msg.setExtraComment(transcode(extracomment.simplified(), utf8));
   msg.setId(msgid);
   msg.setExtras(extra);
   if ((utf8 || yyForceUtf8) && !yyCodecIsUtf8 && msg.needs8Bit()) {
      msg.setUtf8(true);
   }
   tor->append(msg);
}

void CppParser::parse(const QString &initialContext, ConversionData &cd, const QStringList &includeStack,
                      QSet<QString> &inclusions)
{
   if (tor) {
      yyCodecIsUtf8 = (tor->codecName() == "UTF-8");
   }

   namespaces << HashString();
   functionContext = namespaces;
   functionContextUnresolved = initialContext;

   parseInternal(cd, includeStack, inclusions);
}

void CppParser::parseInternal(ConversionData &cd, const QStringList &includeStack, QSet<QString> &inclusions)
{
   static QString strColons("::");

   QString context;
   QString text;
   QString comment;
   QString extracomment;
   QString msgid;
   QString sourcetext;

   TranslatorMessage::ExtraData extra;

   QString prefix;

   int line;
   bool utf8;
   bool yyTokColonSeen = false; // Start of c'tor's initializer list

   yyInPtr = yyInStr.begin();
   yyCh    = getChar();
   yyTok   = getToken();

   while (yyTok != Tok_Eof) {
      // these are array indexing operations. we ignore them entirely
      // so they don't confuse our scoping of static initializers.
      // we enter the loop by either reading a left bracket or by an
      // #else popping the state.

      while (yyBracketDepth) {
         yyTok = getToken();
      }

      switch (yyTok) {
         case Tok_QuotedInclude: {
            text = QDir(QFileInfo(yyFileName).absolutePath()).absoluteFilePath(yyWord);

            if (QFileInfo(text).isFile()) {
               processInclude(text, cd, includeStack, inclusions);
               yyTok = getToken();
               break;
            }
         }

         /* fall through */
         case Tok_AngledInclude: {
            QStringList cSources = cd.m_allCSources.values(yyWord);

            if (! cSources.isEmpty()) {
               for (const QString & cSource : cSources) {
                  processInclude(cSource, cd, includeStack, inclusions);
               }

               goto incOk;
            }

            for (const QString & incPath : cd.m_includePath) {
               text = QDir(incPath).absoluteFilePath(yyWord);

               if (QFileInfo(text).isFile()) {
                  processInclude(text, cd, includeStack, inclusions);
                  goto incOk;
               }
            }

         incOk:
            yyTok = getToken();
            break;
         }
         case Tok_friend:
            yyTok = getToken();
            // These are forward declarations, so ignore them.
            if (yyTok == Tok_class) {
               yyTok = getToken();
            }
            break;
         case Tok_class:
            yyTokColonSeen = false;
            /*
              Partial support for inlined functions.
            */
            yyTok = getToken();
            if (yyBraceDepth == namespaceDepths.count() && yyParenDepth == 0) {
               QList<HashString> quali;
               HashString fct;

               do {
                  //  This code should execute only once, but we play safe with impure definitions such as
                  //  'class Q_EXPORT QMessageBox', in which case 'QMessageBox' is the class name, not 'Q_EXPORT'.

                  text = yyWord;
                  fct.setValue(text);
                  yyTok = getToken();
               } while (yyTok == Tok_Ident);

               while (yyTok == Tok_ColonColon) {
                  yyTok = getToken();
                  if (yyTok != Tok_Ident) {
                     break;   // Oops ...
                  }
                  quali << fct;
                  text = yyWord;

                  fct.setValue(text);
                  yyTok = getToken();
               }
               while (yyTok == Tok_Comment) {
                  yyTok = getToken();
               }
               if (yyTok == Tok_Colon) {
                  // Skip any token until '{' since we might do things wrong if we find
                  // a '::' token here.
                  do {
                     yyTok = getToken();
                  } while (yyTok != Tok_LeftBrace && yyTok != Tok_Eof);
               } else {
                  if (yyTok != Tok_LeftBrace) {
                     // Obviously a forward declaration. We skip those, as they
                     // don't create actually usable namespaces.
                     break;
                  }
               }

               if (!quali.isEmpty()) {
                  // Forward-declared class definitions can be namespaced.
                  QList<HashString> nsl;
                  if (!fullyQualify(namespaces, quali, true, &nsl, 0)) {
                     yyMsg() << "Ignoring definition of undeclared qualified class\n";
                     break;
                  }
                  namespaceDepths.push(namespaces.count());
                  namespaces = nsl;
               } else {
                  namespaceDepths.push(namespaces.count());
               }
               enterNamespace(&namespaces, fct);

               functionContext = namespaces;
               functionContextUnresolved.clear(); // Pointless
               prospectiveContext.clear();
               pendingContext.clear();

               yyTok = getToken();
            }
            break;

         case Tok_namespace:
            yyTokColonSeen = false;
            yyTok = getToken();

            if (yyTok == Tok_Ident) {
               text = yyWord;

               HashString ns = HashString(text);
               yyTok = getToken();

               if (yyTok == Tok_LeftBrace) {
                  namespaceDepths.push(namespaces.count());
                  enterNamespace(&namespaces, ns);

                  functionContext = namespaces;
                  functionContextUnresolved.clear();
                  prospectiveContext.clear();
                  pendingContext.clear();
                  yyTok = getToken();

               } else if (yyTok == Tok_Equals) {
                  // e.g. namespace Is = OuterSpace::InnerSpace;
                  QList<HashString> fullName;
                  yyTok = getToken();

                  if (yyTok == Tok_ColonColon) {
                     fullName.append(HashString(QString()));
                  }

                  while (yyTok == Tok_ColonColon || yyTok == Tok_Ident) {
                     if (yyTok == Tok_Ident) {
                        text = yyWord;
                        fullName.append(HashString(text));
                     }
                     yyTok = getToken();
                  }

                  if (fullName.isEmpty()) {
                     break;
                  }

                  fullName.append(HashString(QString())); // Mark as unresolved
                  modifyNamespace(&namespaces)->aliases[ns] = fullName;
               }
            } else if (yyTok == Tok_LeftBrace) {
               // Anonymous namespace
               namespaceDepths.push(namespaces.count());
               yyTok = getToken();
            }
            break;

         case Tok_using:
            yyTok = getToken();

            // XXX this should affect only the current scope, not the entire current namespace
            if (yyTok == Tok_namespace) {
               QList<HashString> fullName;
               yyTok = getToken();
               if (yyTok == Tok_ColonColon) {
                  fullName.append(HashString(QString()));
               }
               while (yyTok == Tok_ColonColon || yyTok == Tok_Ident) {
                  if (yyTok == Tok_Ident) {
                     text = yyWord;
                     fullName.append(HashString(text));
                  }
                  yyTok = getToken();
               }

               QList<HashString> nsl;
               if (fullyQualify(namespaces, fullName, false, &nsl, 0)) {
                  modifyNamespace(&namespaces)->usings << HashStringList(nsl);
               }

            } else {
               QList<HashString> fullName;
               if (yyTok == Tok_ColonColon) {
                  fullName.append(HashString(QString()));
               }
               while (yyTok == Tok_ColonColon || yyTok == Tok_Ident) {
                  if (yyTok == Tok_Ident) {
                     text = yyWord;
                     fullName.append(HashString(text));
                  }
                  yyTok = getToken();
               }

               if (fullName.isEmpty()) {
                  break;
               }

               // using-declarations cannot rename classes, so the last element of
               // fullName is already the resolved name we actually want.
               // As we do no resolution here, we'll collect useless usings of data
               // members and methods as well. This is no big deal.

               HashString &ns = fullName.last();
               fullName.append(HashString(QString())); // Mark as unresolved
               modifyNamespace(&namespaces)->aliases[ns] = fullName;
            }
            break;

         case Tok_tr:
         case Tok_trUtf8:
            if (!tor) {
               goto case_default;
            }
            if (!sourcetext.isEmpty()) {
               yyMsg() << qPrintable(LU::tr("//% cannot be used with tr() / QT_TR_NOOP(). Ignoring\n"));
            }
            utf8 = (yyTok == Tok_trUtf8);
            line = yyLineNo;
            yyTok = getToken();
            if (match(Tok_LeftParen) && matchString(&text) && !text.isEmpty()) {
               comment.clear();
               bool plural = false;

               if (match(Tok_RightParen)) {
                  // no comment
               } else if (match(Tok_Comma) && matchStringOrNull(&comment)) {   //comment
                  if (match(Tok_RightParen)) {
                     // ok,
                  } else if (match(Tok_Comma)) {
                     plural = true;
                  }
               }
               if (!pendingContext.isEmpty() && !prefix.startsWith(strColons)) {
                  QStringList unresolved;
                  if (!fullyQualify(namespaces, pendingContext, true, &functionContext, &unresolved)) {
                     functionContextUnresolved = unresolved.join(strColons);

                     yyMsg() << qPrintable(LU::tr("Qualifying with unknown namespace/class %1::%2\n")
                                           .formatArg(stringifyNamespace(functionContext)).formatArg(unresolved.first()));
                  }
                  pendingContext.clear();
               }
               if (prefix.isEmpty()) {
                  if (functionContextUnresolved.isEmpty()) {
                     int idx = functionContext.length();
                     if (idx < 2) {
                        yyMsg() << qPrintable(LU::tr("tr() cannot be called without context\n"));
                        break;
                     }

                     Namespace *fctx;

                     while (! (fctx = findNamespace(functionContext, idx)->classDef)->hasTrFunctions) {
                        if (idx == 1) {
                           context = stringifyNamespace(functionContext);
                           fctx = findNamespace(functionContext)->classDef;

                           if (!fctx->complained) {
                              yyMsg() << qPrintable(LU::tr("  Class '%1' is missing CS_OBJECT\n").formatArg(context));
                              fctx->complained = true;
                           }
                           goto gotctx;
                        }
                        --idx;
                     }

                     if (fctx->trQualification.isEmpty()) {
                        context.clear();
                        for (int i = 1;;) {
                           context += functionContext.at(i).value();
                           if (++i == idx) {
                              break;
                           }
                           context += strColons;
                        }
                        fctx->trQualification = context;
                     } else {
                        context = fctx->trQualification;
                     }
                  } else {
                     context = (stringListifyNamespace(functionContext)
                                << functionContextUnresolved).join(strColons);
                  }

               } else {
                  prefix.chop(2);
                  QList<HashString> nsl;
                  QStringList unresolved;

                  if (fullyQualify(functionContext, prefix, false, &nsl, &unresolved)) {
                     Namespace *fctx = findNamespace(nsl)->classDef;

                     if (fctx->trQualification.isEmpty()) {
                        context = stringifyNamespace(nsl);
                        fctx->trQualification = context;
                     } else {
                        context = fctx->trQualification;
                     }

                     if (!fctx->hasTrFunctions && !fctx->complained) {
                        yyMsg() << qPrintable(LU::tr("  Class '%1' is missing CS_OBJECT\n").formatArg(context));
                        fctx->complained = true;
                     }

                  } else {
                     context = (stringListifyNamespace(nsl) + unresolved).join(strColons);
                  }
                  prefix.clear();
               }

            gotctx:
               recordMessage(line, context, text, comment, extracomment, msgid, extra, utf8, plural);
            }
            sourcetext.clear(); // Will have warned about that already
            extracomment.clear();
            msgid.clear();
            extra.clear();
            break;
         case Tok_translateUtf8:
         case Tok_translate:
            if (!tor) {
               goto case_default;
            }
            if (!sourcetext.isEmpty()) {
               yyMsg() << qPrintable(LU::tr("//% cannot be used with translate() / QT_TRANSLATE_NOOP(). Ignoring\n"));
            }
            utf8 = (yyTok == Tok_translateUtf8);
            line = yyLineNo;
            yyTok = getToken();
            if (match(Tok_LeftParen)
                  && matchString(&context)
                  && match(Tok_Comma)
                  && matchString(&text) && !text.isEmpty()) {
               comment.clear();
               bool plural = false;
               if (!match(Tok_RightParen)) {
                  // look for comment
                  if (match(Tok_Comma) && matchStringOrNull(&comment)) {
                     if (!match(Tok_RightParen)) {
                        // look for encoding
                        if (match(Tok_Comma)) {
                           if (matchEncoding(&utf8)) {
                              if (!match(Tok_RightParen)) {
                                 // look for the plural quantifier,
                                 // this can be a number, an identifier or
                                 // a function call,
                                 // so for simplicity we mark it as plural if
                                 // we know we have a comma instead of an
                                 // right parentheses.
                                 plural = match(Tok_Comma);
                              }
                           } else {
                              // This can be a QTranslator::translate("context",
                              // "source", "comment", n) plural translation
                              if (matchExpression() && match(Tok_RightParen)) {
                                 plural = true;
                              } else {
                                 break;
                              }
                           }
                        } else {
                           break;
                        }
                     }
                  } else {
                     break;
                  }
               }
               recordMessage(line, context, text, comment, extracomment, msgid, extra, utf8, plural);
            }
            sourcetext.clear(); // Will have warned about that already
            extracomment.clear();
            msgid.clear();
            extra.clear();
            break;

         case Tok_trid:
            if (!tor) {
               goto case_default;
            }
            if (!msgid.isEmpty()) {
               yyMsg() << qPrintable(LU::tr("//= cannot be used with qtTrId() / QT_TRID_NOOP(). Ignoring\n"));
            }
            //utf8 = false; // Maybe use //%% or something like that
            line = yyLineNo;
            yyTok = getToken();
            if (match(Tok_LeftParen) && matchString(&msgid) && !msgid.isEmpty()) {
               bool plural = match(Tok_Comma);
               recordMessage(line, QString(), sourcetext, QString(), extracomment,
                             msgid, extra, false, plural);
            }
            sourcetext.clear();
            extracomment.clear();
            msgid.clear();
            extra.clear();
            break;

         case Tok_Q_DECLARE_TR_FUNCTIONS:
            if (getMacroArgs()) {
               Namespace *ns = modifyNamespace(&namespaces);
               ns->hasTrFunctions = true;
               ns->trQualification = yyWord;
            }
            yyTok = getToken();
            break;

         case Tok_CS_OBJECT:
            modifyNamespace(&namespaces)->hasTrFunctions = true;
            yyTok = getToken();
            break;

         case Tok_Ident:
            prefix += yyWord;
            yyTok = getToken();

            if (yyTok != Tok_ColonColon) {
               prefix.clear();
               if (yyTok == Tok_Ident && !yyParenDepth) {
                  prospectiveContext.clear();
               }
            }
            break;

         case Tok_Comment: {
            if (! tor) {
               goto case_default;
            }

            QChar c;
            bool foundSpace = false;

            if (yyWord.length() >= 2)  {
               c = yyWord[0];

               if (yyWord[1].isSpace()) {
                  foundSpace = true;
               }
            }

            if (c == ':' && foundSpace) {
               yyWord.remove(0, 2);
               extracomment += yyWord;

            } else if (c == '=' && foundSpace) {
               yyWord.remove(0, 2);
               msgid = yyWord.simplified();

            } else if (c == '~' && foundSpace) {
               yyWord.remove(0, 2);
               text  = yyWord.trimmed();
               int k = text.indexOf(' ');

               if (k > -1) {
                  extra.insert(text.left(k), text.mid(k + 1).trimmed());
               }
               text.clear();

            } else if (c == '%' && foundSpace) {

               QString::const_iterator iter = yyWord.begin() + 2;
               QChar c;

               while (true) {
                  if (iter == yyWord.end()) {
                     break;
                  }

                  c = *iter;
                  ++iter;

                  if (c.isSpace()) {
                     continue;
                  }

                  if (c != '"') {
                     yyMsg() << qPrintable(LU::tr("Unexpected character in meta string\n"));
                     break;
                  }

                  while (true) {

                     if (iter == yyWord.end()) {
                        yyMsg() << qPrintable(LU::tr("Unterminated meta string\n"));
                        break;
                     }

                     c = *iter;
                     ++iter;

                     if (c == '"') {
                        break;
                     }

                     if (c == '\\') {
                        if (iter == yyWord.end()) {
                           yyMsg() << qPrintable(LU::tr("Unterminated meta string\n"));
                           break;
                        }

                        c = *iter;
                        ++iter;

                        if (c == '\n') {
                           yyMsg() << qPrintable(LU::tr("Unterminated meta string\n"));
                           break;
                        }

                        sourcetext.append('\\');
                     }

                     sourcetext.append(c);
                  }
               }

            } else {
               QString::const_iterator iter = yyWord.begin();
               QChar c;

               while (iter!= yyWord.end()) {
                  c = *iter;

                  if (c == ' ' || c == '\t' || c == '\n') {
                     ++iter;
                  } else {
                     break;
                  }
               }

               QStringView tmpWord(iter, yyWord.end());

               if (tmpWord.startsWith(MagicComment)) {

                  iter += MagicComment.length();
                  comment = QString(iter, yyWord.end()).simplified();

                  auto tmpIter = comment.indexOfFast(' ');

                  if (tmpIter == comment.end()) {
                     context = comment;

                  } else {
                     context = QString(comment.begin(), tmpIter);
                     comment.erase(comment.begin(), tmpIter + 1);

                     TranslatorMessage msg( transcode(context, false), QString(), transcode(comment, false), QString(),
                        yyFileName, yyLineNo, QStringList(), TranslatorMessage::Finished, false);

                     msg.setExtraComment(transcode(extracomment.simplified(), false));
                     extracomment.clear();
                     tor->append(msg);
                     tor->setExtras(extra);
                     extra.clear();
                  }
               }
            }

            yyTok = getToken();
            break;
         }

         case Tok_Arrow:
            yyTok = getToken();
            if (yyTok == Tok_tr || yyTok == Tok_trUtf8) {
               yyMsg() << qPrintable(LU::tr("Cannot invoke tr() like this\n"));
            }
            break;

         case Tok_ColonColon:
            if (yyBraceDepth == namespaceDepths.count() && yyParenDepth == 0 && !yyTokColonSeen) {
               prospectiveContext = prefix;
            }

            prefix += strColons;
            yyTok = getToken();
            break;

         case Tok_RightBrace:
            if (yyBraceDepth + 1 == namespaceDepths.count()) { // class or namespace
               truncateNamespaces(&namespaces, namespaceDepths.pop());
            }

            if (yyBraceDepth == namespaceDepths.count()) {
               // function, class or namespace
               if (!yyBraceDepth && !directInclude) {
                  truncateNamespaces(&functionContext, 1);
                  functionContextUnresolved = cd.m_defaultContext;
               } else {
                  functionContext = namespaces;
                  functionContextUnresolved.clear();
               }
               pendingContext.clear();
            }
         // fallthrough
         case Tok_Semicolon:
            prospectiveContext.clear();
            prefix.clear();
            if (!sourcetext.isEmpty() || !extracomment.isEmpty() || !msgid.isEmpty() || !extra.isEmpty()) {
               yyMsg() << qPrintable(LU::tr("Discarding unconsumed meta data\n"));
               sourcetext.clear();
               extracomment.clear();
               msgid.clear();
               extra.clear();
            }
            yyTokColonSeen = false;
            yyTok = getToken();
            break;

         case Tok_Colon:
            if (!prospectiveContext.isEmpty()
                  && yyBraceDepth == namespaceDepths.count() && yyParenDepth == 0) {
               pendingContext = prospectiveContext;
            }
            yyTokColonSeen = true;
            yyTok = getToken();
            break;
         case Tok_LeftBrace:
            if (!prospectiveContext.isEmpty()
                  && yyBraceDepth == namespaceDepths.count() + 1 && yyParenDepth == 0) {
               pendingContext = prospectiveContext;
            }
         // fallthrough
         case Tok_LeftParen:
         case Tok_RightParen:
            yyTokColonSeen = false;
            yyTok = getToken();
            break;

         default:
            if (!yyParenDepth) {
               prospectiveContext.clear();
            }
         // fallthrough
         case Tok_Equals: // for static initializers; other cases make no difference
         case Tok_RightBracket: // ignoring indexing; same reason
         case_default:
            yyTok = getToken();
            break;
      }
   }

   if (yyBraceDepth != 0)
      yyMsg(yyBraceLineNo)
            << qPrintable(LU::tr("Unbalanced opening brace in C++ code"
                                 " (or abuse of the C++ preprocessor)\n"));
   else if (yyParenDepth != 0)
      yyMsg(yyParenLineNo)
            << qPrintable(LU::tr("Unbalanced opening parenthesis in C++ code"
                                 " (or abuse of the C++ preprocessor)\n"));
   else if (yyBracketDepth != 0)
      yyMsg(yyBracketLineNo)
            << qPrintable(LU::tr("Unbalanced opening bracket in C++ code"
                                 " (or abuse of the C++ preprocessor)\n"));
}

const ParseResults *CppParser::recordResults(bool isHeader)
{
   if (tor) {
      if (tor->messageCount()) {
         CppFiles::setTranslator(yyFileName, tor);
      } else {
         delete tor;
         tor = 0;
      }
   }

   if (isHeader) {
      const ParseResults *pr;

      if (!tor && results->includes.count() == 1
            && results->rootNamespace.children.isEmpty()
            && results->rootNamespace.aliases.isEmpty()
            && results->rootNamespace.usings.isEmpty()) {

         // This is a forwarding header. Slash it.
         pr = *results->includes.begin();
         delete results;

      } else {
         results->fileId = nextFileId++;
         pr = results;
      }

      CppFiles::setResults(yyFileName, pr);
      return pr;

   } else {
      delete results;
      return 0;
   }
}

/*
  Fetches tr() calls in C++ code in UI files (inside "<function>"
  tag). This mechanism is obsolete.
*/
void fetchtrInlinedCpp(const QString &in, Translator &translator, const QString &context)
{
   CppParser parser;
   parser.setInput(in);
   ConversionData cd;
   QSet<QString> inclusions;
   parser.setTranslator(&translator);
   parser.parse(context, cd, QStringList(), inclusions);
   parser.deleteResults();
}

void loadCPP(Translator &translator, const QStringList &filenames, ConversionData &cd)
{
   QByteArray codecName = cd.m_codecForSource.isEmpty() ? translator.codecName() : cd.m_codecForSource;
   QTextCodec *codec = QTextCodec::codecForName(codecName);

   for (const QString & filename : filenames) {
      if (!CppFiles::getResults(filename).isEmpty() || CppFiles::isBlacklisted(filename)) {
         continue;
      }

      QFile file(filename);
      if (!file.open(QIODevice::ReadOnly)) {
         cd.appendError(LU::tr("Can not open %1: %2").formatArgs(filename, file.errorString()));
         continue;
      }

      CppParser parser;
      QTextStream ts(&file);
      ts.setCodec(codec);
      ts.setAutoDetectUnicode(true);
      parser.setInput(ts, filename);

      if (cd.m_outputCodec.isEmpty() && ts.codec()->name() == "UTF-16") {
         translator.setCodecName("System");
      }

      Translator *tor = new Translator;
      tor->setCodecName(translator.codecName());
      parser.setTranslator(tor);
      QSet<QString> inclusions;
      parser.parse(cd.m_defaultContext, cd, QStringList(), inclusions);
      parser.recordResults(isHeader(filename));
   }

   for (const QString & filename : filenames) {
      if (! CppFiles::isBlacklisted(filename)) {

         if (const Translator *tor = CppFiles::getTranslator(filename)) {
            for (const TranslatorMessage & msg : tor->messages()) {
               translator.extend(msg);
            }
         }
      }
   }
}

