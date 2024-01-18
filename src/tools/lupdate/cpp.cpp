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

#include <lupdate.h>
#include <translator.h>

#include <qalgorithms.h>
#include <qbitarray.h>
#include <qdebug.h>
#include <qfileinfo.h>
#include <qregularexpression.h>
#include <qstack.h>
#include <qstring.h>
#include <qtextcodec.h>
#include <qtextstream.h>

#include <iostream>
#include <ctype.h>

static QString MagicComment("TRANSLATOR");

static const QString text_CS_OBJECT        = "CS_OBJECT";

static const QString text_class            = "class";
static const QString text_final            = "final";
static const QString text_friend           = "friend";
static const QString text_namespace        = "namespace";
static const QString text_operator         = "operator";
static const QString text_public           = "public";
static const QString text_protected        = "protected";
static const QString text_private          = "private";
static const QString text_return           = "return";
static const QString text_struct           = "struct";
static const QString text_using            = "using";

static const QString text_null             = "NULL";
static const QString text_nullptr          = "nullptr";

static const QString text_findMessage      = "findMessage";

// declared in qcoreapplication
static const QString text_tr               = "tr";
static const QString text_translate        = "translate";

// calls tr(), for classes which do not inherit from QObject
static const QString text_Q_DECLARE_TR_FUNCTIONS = "Q_DECLARE_TR_FUNCTIONS";

// declared in qglobal.h
static const QString text_QT_TR_NOOP             = "QT_TR_NOOP";
static const QString text_QT_TRANSLATE_NOOP      = "QT_TRANSLATE_NOOP";
static const QString text_QT_TRANSLATE_NOOP3     = "QT_TRANSLATE_NOOP3";
static const QString text_QT_TRID_NOOP           = "QT_TRID_NOOP";

static const QString text_cs_mark_tr             = "cs_mark_tr";
static const QString text_cs_mark_string_tr      = "cs_mark_string_tr";
static const QString text_cs_mark_tr_id          = "cs_mark_tr_id";
static const QString text_qtTrId                 = "qtTrId";

class HashString
{
 public:
   HashString()
      : m_hash(0x80000000)
   {
   }

   explicit HashString(const QString &str)
      : m_str(str), m_hash(0x80000000)
   {
   }

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
      : m_list(list), m_hash(0x80000000) {
   }

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

      for (const HashString &qs : list.m_list) {
         hash ^= qHash(qs) ^ 0x6ad9f526;
         hash = ((hash << 13) & 0x7fffffff) | (hash >> 18);
      }

      list.m_hash = hash;
   }

   return list.m_hash;
}

struct Namespace {

   Namespace()
      : classDef(this), hasTrFunctions(false), complained(false)
   {
   }

   ~Namespace() {
      qDeleteAll(children);
   }

   QHash<HashString, Namespace *> children;
   QHash<HashString, QList<HashString>> aliases;
   QList<HashStringList> usings;

   Namespace *classDef;

   QString trQualification;

   bool hasTrFunctions;
   bool complained;             // tr functions are missing
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
   enum Group {
      Tr,
      Translate,
      TrId,
      CsMarkTr
   };

   CppParser(ParseResults *results = nullptr);
   void setInput(const QString &in);
   void setInput(QTextStream &ts, const QString &fileName);

   void setTranslator(Translator *obj) {
      m_translator = obj;
   }

   void parse(ConversionData &cd, const QStringList &includeStack, QSet<QString> &inclusions);

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

      IfdefState(int _bracketDepth, int _braceDepth, int _parenDepth)
         : bracketDepth(_bracketDepth), braceDepth(_braceDepth),
           parenDepth(_parenDepth), elseLine(-1) {
      }

      SavedState state;
      int bracketDepth;
      int bracketDepth1st;
      int braceDepth;
      int braceDepth1st;
      int parenDepth;
      int parenDepth1st;
      int elseLine;
   };

   enum TokenType {
      Tok_Eof, Tok_Class, Tok_Friend, Tok_Namespace, Tok_Using, Tok_Return,
      Tok_CS_OBJECT, Tok_Access, Tok_Cancel,
      Tok_Identifier, Tok_String, Tok_Arrow, Tok_Colon, Tok_ColonColon,
      Tok_LeftBracket, Tok_RightBracket, Tok_LeftBrace, Tok_RightBrace, Tok_LeftParen, Tok_RightParen,
      Tok_Comma, Tok_Semicolon, Tok_QuestionMark, Tok_Equals,
      Tok_Null, Tok_Integer,
      Tok_QuotedInclude, Tok_AngledInclude,
      Tok_Other
   };

   std::ostream &yyMsg(int line = 0);

   QChar getChar();

   TokenType getToken();
   std::pair<QStringList, bool> getList(Group kind);

   void processComment();

   bool match(TokenType t);
   bool matchExpression();

   QString transcode(const QString &str);
   void recordMessage(
      int line, const QString &context, const QString &text, const QString &comment,
      const QString &extracomment, const QString &msgid, const QHash<QString, QString> &extra,
      bool plural);

   void handleCsMarkTr(Group kind, QString prefix = QString());
   void handleTrId();
   void handleDeclareTrFunctions();

   void processInclude(const QString &file, ConversionData &cd,
                       const QStringList &includeStack, QSet<QString> &inclusions);

   void saveState(SavedState *state);
   void loadState(const SavedState *state);

   static QString stringifyNamespace(int start, const QList<HashString> &namespaces);

   static QString stringifyNamespace(const QList<HashString> &namespaces) {
      return stringifyNamespace(1, namespaces);
   }

   static QString joinNamespaces(const QString &one, const QString &two);
   typedef bool (CppParser::*VisitNamespaceCallback)(const Namespace *ns, void *context) const;

   bool visitNamespace(const QList<HashString> &namespaces, int nsCount,
                       VisitNamespaceCallback callback, void *context,
                       VisitRecorder &vr, const ParseResults *rslt) const;

   bool visitNamespace(const QList<HashString> &namespaces, int nsCount,
                       VisitNamespaceCallback callback, void *context) const;

   bool qualifyOneCallbackOwn(const Namespace *ns, void *context) const;
   bool qualifyOneCallbackUsing(const Namespace *ns, void *context) const;

   bool qualifyOne(const QList<HashString> &namespaces, int nsCnt, const HashString &segment,
               QList<HashString> *resolved, QSet<HashStringList> *visitedUsings) const;

   bool qualifyOne(const QList<HashString> &namespaces, int nsCnt, const HashString &segment,
               QList<HashString> *resolved) const;

   bool fullyQualify(const QList<HashString> &namespaces, int nsCnt, const QList<HashString> &segments,
               bool isDeclaration, QList<HashString> *resolved, QList<HashString> *unresolved) const;

   bool fullyQualify(const QList<HashString> &namespaces, const QList<HashString> &segments,
               bool isDeclaration, QList<HashString> *resolved, QList<HashString> *unresolved) const;

   bool fullyQualify(const QList<HashString> &namespaces, const QString &segments,
               bool isDeclaration, QList<HashString> *resolved, QList<HashString> *unresolved) const;

   bool findNamespaceCallback(const Namespace *ns, void *context) const;
   const Namespace *findNamespace(const QList<HashString> &namespaces, int nsCount = -1) const;
   void enterNamespace(QList<HashString> *namespaces, const HashString &name);
   void truncateNamespaces(QList<HashString> *namespaces, int length);
   Namespace *modifyNamespace(QList<HashString> *namespaces, bool haveLast = true);

   // Tokenizer state
   QString yyFileName;
   QChar yyCh;

   bool yyAtNewline;

   QString yyWord;
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
   QString::const_iterator yyIter;

   // Parser state
   TokenType yyTok;

   bool metaExpected;
   QString context;
   QString text;
   QString comment;
   QString extracomment;
   QString msgid;
   QString sourcetext;

   QHash<QString, QString> extra;
   QList<HashString> namespaces;
   QStack<int> namespaceDepths;
   QList<HashString> functionContext;
   QString functionContextUnresolved;
   QString prospectiveContext;
   QString pendingContext;
   ParseResults *results;
   Translator *m_translator;
   bool directInclude;

   SavedState savedState;
   int yyMinBraceDepth;
   bool inDefine;
};

CppParser::CppParser(ParseResults *parseResults)
{
   m_translator = nullptr;

   if (parseResults) {
      results = parseResults;
      directInclude = true;

   } else {
      results = new ParseResults;
      directInclude = false;
   }

   yyBracketDepth  = 0;
   yyBraceDepth    = 0;
   yyParenDepth    = 0;
   yyCurLineNo     = 1;
   yyBracketLineNo = 1;
   yyBraceLineNo   = 1;
   yyParenLineNo   = 1;
   yyAtNewline     = true;
   yyMinBraceDepth = 0;
   inDefine        = false;
}

std::ostream &CppParser::yyMsg(int line)
{
   return std::cerr << csPrintable(yyFileName) << ':' << (line ? line : yyLineNo) << ": ";
}

void CppParser::setInput(const QString &in)
{
   yyInStr       = in;
   yyFileName    = QString();
   yySourceCodec = nullptr;
}

void CppParser::setInput(QTextStream &ts, const QString &fileName)
{
   yyInStr       = ts.readAll();
   yyFileName    = fileName;
   yySourceCodec = ts.codec();
}

/*
  First part. Skip most of C++, the only tokens that interest us are defined here.

      int main() {
          printf("Hello, world\n");
          return 0;
      }

  is broken down into the following tokens (Tok_ omitted):

      Identifier Identifier LeftParen RightParen
      LeftBrace
          Identifier LeftParen String RightParen Semicolon
          return Semicolon
      RightBrace
*/

QChar CppParser::getChar()
{
   QString::const_iterator iter = yyIter;

   while (true)  {

      if (iter == yyInStr.cend()) {
         yyIter = iter;
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

      yyIter = iter;

      return c1;
   }
}

CppParser::TokenType CppParser::getToken()
{
restart:
   while (yyCh != EOF) {
      yyLineNo = yyCurLineNo;

      if (yyCh == '#' && yyAtNewline) {

         while (true) {
            yyCh = getChar();

            if (! yyCh.isSpace() || yyCh == '\n') {
               break;
            }
         }

         switch (yyCh.unicode()) {

            case 'd':
               // skip over the name of the define

               do {
                  // Rest of "define"
                  yyCh = getChar();

                  if (yyCh == EOF) {
                     return Tok_Eof;
                  }

                  if (yyCh == '\n') {
                     goto restart;
                  }

               } while (! yyCh.isSpace());

               do {
                  // Space beween "define" and macro name
                  yyCh = getChar();

                  if (yyCh == EOF) {
                     return Tok_Eof;
                  }

                  if (yyCh == '\n') {
                     goto restart;
                  }

               } while (yyCh.isSpace());

               do {
                  // Macro name

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

               do {
                  // Shortcut the immediate newline case if no comments follow

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
                  } while (yyCh != EOF && ! yyCh.isSpace() && yyCh != '"' && yyCh != '<');

                  while (yyCh.isSpace()) {
                     yyCh = getChar();
                  }

                  int tChar;

                  if (yyCh == '"') {
                     tChar = '"';

                  } else if (yyCh == '<') {
                     tChar = '>';

                  } else {
                     break;
                  }

                  yyWord.clear();

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

                           yyMsg(is.elseLine) << "Parenthesis/bracket/brace mismatch between "
                                                 "#if and #else branches; using #if branch\n";
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
                  if (! yyIfdefStack.isEmpty()) {
                     IfdefState is = yyIfdefStack.pop();

                     if (is.elseLine != -1) {
                        if (yyBracketDepth != is.bracketDepth1st || yyBraceDepth != is.braceDepth1st
                              || yyParenDepth != is.parenDepth1st) {

                           yyMsg(is.elseLine) << "Parenthesis/brace mismatch between "
                                                 "#if and #else branches; using #if branch\n";
                        }

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

         // skip remaining preprocessor directives
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

                  while (true) {
                     yyCh = getChar();
                     if (yyCh == EOF) {
                        yyMsg() << "Unterminated C++ comment\n";
                        break;
                     }

                     if (yyCh == '*') {
                        metAster = true;

                     } else if (metAster && yyCh == '/') {
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
         // identifiers

         yyWord.clear();

         do {
            yyWord.append(yyCh);
            yyCh = getChar();

         } while ((yyCh >= 'A' && yyCh <= 'Z') || (yyCh >= 'a' && yyCh <= 'z') || (yyCh >= '0' && yyCh <= '9') || yyCh == '_');

         switch (yyWord[0].unicode()) {
            case 'N':
               if (yyWord == text_null) {
                  return Tok_Null;
               }
               break;

            case 'C':
               if (yyWord == text_CS_OBJECT) {
                  return Tok_CS_OBJECT;
               }
               break;

            case 'c':
               if (yyWord == text_class) {
                  return Tok_Class;
               }
               break;

            case 'f':
               if (yyWord == text_friend) {
                  return Tok_Friend;
               }
               break;

            case 'n':
               if (yyWord == text_namespace) {
                  return Tok_Namespace;
               }

               if (yyWord == text_nullptr) {
                  return Tok_Null;
               }

               break;

            case 'o':
               if (yyWord == text_operator) {
                  // Operator overload declaration/definition
                  // need to prevent those characters from confusing the followup parsing

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

            case 'p':
               if (yyWord == text_public || yyWord == text_protected || yyWord == text_private) {
                  return Tok_Access;
               }
               break;

            case 'r':
               if (yyWord == text_return) {
                  return Tok_Return;
               }
               break;

            case 's':
               if (yyWord == text_struct) {
                  return Tok_Class;
               }
               break;

            case 'u':
               if (yyWord == text_using) {
                  return Tok_Using;
               }
               break;
         }

         // gui_cs_object, net_cs_object, etc
         if (yyWord.contains(text_CS_OBJECT)) {
            return Tok_CS_OBJECT;
         }

         return Tok_Identifier;

      } else {
         switch (yyCh.unicode()) {

            case '\n':
               if (inDefine) {
                  loadState(&savedState);
                  prospectiveContext.clear();
                  yyBraceDepth = yyMinBraceDepth;
                  yyMinBraceDepth = 0;
                  inDefine = false;

                  metaExpected = true;
                  yyCh = getChar();

                  // break out of any multi-token constructs
                  return Tok_Cancel;
               }

               yyCh = getChar();
               break;

            case '/':
               yyCh = getChar();

               if (yyCh == '/') {
                  yyWord.clear();

                  do {
                     yyCh = getChar();

                     if (yyCh == EOF) {
                        break;
                     }

                     yyWord.append(yyCh);

                  } while (yyCh != '\n');

                  processComment();

               } else if (yyCh == '*') {
                  bool metAster = false;
                  yyWord.clear();

                  while (true) {
                     yyCh = getChar();

                     if (yyCh == EOF) {
                        yyMsg() << "Unterminated C++ comment\n";
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
                     processComment();

                  }
                  yyCh = getChar();
               }
               break;

            case '"': {
               // literal strings

               yyWord.clear();
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
                  yyMsg() << "Unterminated C++ string\n";

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
            // parsing of things like   std::cout << QObject::tr() as context std::cout::QObject

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

               while (true) {
                  if (yyCh == EOF || yyCh == '\n') {
                     yyMsg() << "Unterminated C++ character\n";
                     break;
                  }

                  yyCh = getChar();

                  if (yyCh == '\'') {
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
                  if (! inDefine) {
                     yyMsg(yyCurLineNo) << "Excess closing brace in C++ code"
                                           " (or misuse of the preprocessor)\n";
                  }

                  yyCh = getChar();
                  return Tok_Semicolon;
               }

               --yyBraceDepth;

               yyCh = getChar();
               return Tok_RightBrace;

            case '(':
               if (yyParenDepth == 0) {
                  yyParenLineNo = yyCurLineNo;
               }

               ++yyParenDepth;
               yyCh = getChar();
               return Tok_LeftParen;

            case ')':
               if (yyParenDepth == 0) {
                  yyMsg(yyCurLineNo) << "Excess closing parenthesis in C++ code"
                                        " (or misuse of the preprocessor)\n";

               } else {
                  --yyParenDepth;
               }

               yyCh = getChar();
               return Tok_RightParen;

            case '[':
               if (yyBracketDepth == 0) {
                  yyBracketLineNo = yyCurLineNo;
               }

               ++yyBracketDepth;
               yyCh = getChar();
               return Tok_LeftBracket;

            case ']':
               if (yyBracketDepth == 0) {
                  yyMsg(yyCurLineNo) << "Excess closing bracket in C++ code"
                                        " (or misuse of the preprocessor)\n";

               } else {
                  --yyBracketDepth;
               }

               yyCh = getChar();
               return Tok_RightBracket;

            case ',':
               yyCh = getChar();
               return Tok_Comma;

            case ';':
               yyCh = getChar();
               return Tok_Semicolon;

            case '?':
               yyCh = getChar();
               return Tok_QuestionMark;

            case '0':
               yyCh = getChar();

               if (yyCh == 'x') {
                  do {
                     yyCh = getChar();

                  } while ((yyCh >= '0' && yyCh <= '9') || (yyCh >= 'a' && yyCh <= 'f') || (yyCh >= 'A' && yyCh <= 'F'));
                  return Tok_Integer;
               }

               if (yyCh < '0' || yyCh > '9') {
                  return Tok_Null;
               }

               [[fallthrough]];

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


//  The second part of this source file are namespace/class related utilities for the third part.

void CppParser::saveState(SavedState *state)
{
   state->namespaces      = namespaces;
   state->namespaceDepths = namespaceDepths;
   state->functionContext = functionContext;
   state->functionContextUnresolved = functionContextUnresolved;
   state->pendingContext  = pendingContext;
}

void CppParser::loadState(const SavedState *state)
{
   namespaces      = state->namespaces;
   namespaceDepths = state->namespaceDepths;
   functionContext = state->functionContext;
   functionContextUnresolved = state->functionContextUnresolved;
   pendingContext  = state->pendingContext;
}

Namespace *CppParser::modifyNamespace(QList<HashString> *namespaces, bool haveLast)
{
   Namespace *pns;
   Namespace *ns = &results->rootNamespace;

   for (int i = 1; i < namespaces->count(); ++i) {
      pns = ns;

      if (! (ns = pns->children.value(namespaces->at(i)))) {
         do {
            ns = new Namespace;

            if (haveLast || i < namespaces->count() - 1) {
               if (const Namespace *ons = findNamespace(*namespaces, i + 1)) {
                  ns->classDef = ons->classDef;
               }
            }

            pns->children.insert(namespaces->at(i), ns);
            pns = ns;

         } while (++i < namespaces->count());

         break;
      }
   }

   return ns;
}

QString CppParser::stringifyNamespace(int start, const QList<HashString> &namespaces)
{
   QString retval;

   for (int i = start; i < namespaces.count(); ++i) {
      if (i > start) {
         retval += "::";
      }

      retval += namespaces.at(i).value();
   }

   return retval;
}

QString CppParser::joinNamespaces(const QString &one, const QString &two)
{
   return two.isEmpty() ? one : one.isEmpty() ? two : one + "::" + two;
}

bool CppParser::visitNamespace(const QList<HashString> &namespaces, int nsCount,
               VisitNamespaceCallback callback, void *context, VisitRecorder &vr, const ParseResults *rslt) const
{
   const Namespace *ns = &rslt->rootNamespace;

   for (int i = 1; i < nsCount; ++i) {
      if (! (ns = ns->children.value(namespaces.at(i)))) {
         goto supers;
      }
   }

   if ((this->*callback)(ns, context)) {
      return true;
   }

supers:
   for (const ParseResults *sup : rslt->includes) {

      if (vr.tryVisit(sup->fileId) && visitNamespace(namespaces, nsCount, callback, context, vr, sup)) {
         return true;
      }
   }

   return false;
}

bool CppParser::visitNamespace(const QList<HashString> &namespaces, int nsCount,
               VisitNamespaceCallback callback, void *context) const
{
   VisitRecorder vr;
   return visitNamespace(namespaces, nsCount, callback, context, vr, results);
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

      if (nsl.last().value().isEmpty()) {
         // delayed alias resolution
         QList<HashString> &nslIn = *const_cast<QList<HashString> *>(&nsl);

         nslIn.removeLast();
         QList<HashString> nslOut;

         if (! fullyQualify(data->namespaces, data->nsCount, nslIn, false, &nslOut, nullptr)) {
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

   for (const HashStringList &use : ns->usings) {
      if (! data->visitedUsings->contains(use)) {
         data->visitedUsings->insert(use);

         if (qualifyOne(use.value(), use.value().count(), data->segment, data->resolved,
                        data->visitedUsings)) {
            return true;
         }
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
               QList<HashString> *resolved, QList<HashString> *unresolved) const
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
            if (! qualifyOne(*resolved, resolved->count(), segments[segIdx], resolved)) {

               if (unresolved) {
                  *unresolved = segments.mid(segIdx);
               }

               return false;
            }
         }

         return true;
      }
   } while (! isDeclaration && --nsIdx >= 0);

   resolved->clear();
   *resolved << HashString(QString());

   if (unresolved) {
      *unresolved = segments.mid(initSegIdx);
   }

   return false;
}

bool CppParser::fullyQualify(const QList<HashString> &namespaces, const QList<HashString> &segments,
               bool isDeclaration, QList<HashString> *resolved, QList<HashString> *unresolved) const
{
   return fullyQualify(namespaces, namespaces.count(), segments, isDeclaration, resolved, unresolved);
}

bool CppParser::fullyQualify(const QList<HashString> &namespaces, const QString &quali,
               bool isDeclaration, QList<HashString> *resolved, QList<HashString> *unresolved) const
{
   static QString strColons("::");

   QList<HashString> segments;

   for (const QString &str : quali.split(strColons))   {
      segments << HashString(str);
   }

   return fullyQualify(namespaces, segments, isDeclaration, resolved, unresolved);
}

bool CppParser::findNamespaceCallback(const Namespace *ns, void *context) const
{
   *((const Namespace **)context) = ns;
   return true;
}

const Namespace *CppParser::findNamespace(const QList<HashString> &namespaces, int nsCount) const
{
   const Namespace *ns = nullptr;

   if (nsCount == -1) {
      nsCount = namespaces.count();
   }

   visitNamespace(namespaces, nsCount, &CppParser::findNamespaceCallback, &ns);

   return ns;
}

void CppParser::enterNamespace(QList<HashString> *namespaces, const HashString &name)
{
   *namespaces << name;

   if (! findNamespace(*namespaces)) {
      modifyNamespace(namespaces, false);
   }
}

void CppParser::truncateNamespaces(QList<HashString> *namespaces, int length)
{
   if (namespaces->count() > length) {
      namespaces->erase(namespaces->begin() + length, namespaces->end());
   }
}

// Functions for processing include files.

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

   if (! cycle) {
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

   for (const QString &fileName : fileNames) {
      IncludeCycle *intersectingCycle = includeCycles().value(fileName);

      if (intersectingCycle && !intersectingCycles.contains(intersectingCycle)) {
         intersectingCycles.insert(intersectingCycle);

         cycle->fileNames.unite(intersectingCycle->fileNames);
         cycle->results.unite(intersectingCycle->results);
      }
   }

   qDeleteAll(intersectingCycles);

   for (const QString &fileName : cycle->fileNames) {
      includeCycles().insert(fileName, cycle);
   }
}

static bool isHeader(const QString &name)
{
   QString fileExt = QFileInfo(name).suffix();
   return fileExt.isEmpty() || fileExt.startsWith('h', Qt::CaseInsensitive);
}

void CppParser::processInclude(const QString &file, ConversionData &cd, const QStringList &includeStack,
            QSet<QString> &inclusions)
{
   QString cleanFile = QDir::cleanPath(file);

   for (const QString &item : cd.m_excludes) {
      QRegularExpression regex(item, QPatternOption::WildcardOption | QPatternOption::ExactMatchOption);

      if (regex.match(cleanFile).hasMatch()) {
         return;
      }
   }

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
         && ! CppFiles::isBlacklisted(cleanFile) && isHeader(cleanFile)) {

      QSet<const ParseResults *> res = CppFiles::getResults(cleanFile);
      if (! res.isEmpty()) {
         results->includes.unite(res);
         return;
      }

      isIndirect = true;
   }

   QFile f(cleanFile);
   if (! f.open(QIODevice::ReadOnly)) {
      yyMsg() << csPrintable(QString("Unable to open %1: %2\n").formatArgs(cleanFile, f.errorString()));
      return;
   }

   QTextStream ts(&f);
   ts.setCodec(yySourceCodec);
   ts.setAutoDetectUnicode(true);

   inclusions.insert(cleanFile);

   if (isIndirect) {
      CppParser parser;

      for (const QString &projectRoot : cd.m_projectRoots) {
         if (cleanFile.startsWith(projectRoot)) {
            parser.setTranslator(new Translator);
            break;
         }
      }

      parser.setInput(ts, cleanFile);

      QStringList stack = includeStack;
      stack << cleanFile;

      parser.parse(cd, stack, inclusions);
      results->includes.insert(parser.recordResults(true));

   } else {
      CppParser parser(results);
      parser.namespaces = namespaces;
      parser.functionContext = functionContext;
      parser.functionContextUnresolved = functionContextUnresolved;
      parser.setInput(ts, cleanFile);
      parser.setTranslator(m_translator);

      QStringList stack = includeStack;
      stack << cleanFile;
      parser.parseInternal(cd, stack, inclusions);

      // Avoid using messages obtained by direct scanning
      CppFiles::setBlacklisted(cleanFile);
   }

   inclusions.remove(cleanFile);
   prospectiveContext.clear();
   pendingContext.clear();
}

bool CppParser::match(TokenType t)
{
   if (yyTok == t) {
      yyTok = getToken();
      return true;
   }

   return false;
}

std::pair<QStringList, bool> CppParser::getList(Group kind)
{
   QStringList list;

   bool plural   = false;
   bool foundStr = false;

   while (true) {
      yyTok = getToken();

      if (yyTok == Tok_RightParen) {
         break;

      } else if (yyTok == Tok_Comma) {
         foundStr = false;
         continue;

      } else if (yyTok == Tok_Null) {
         list.append(QString());
         foundStr = true;

      } else if (yyTok == Tok_String) {
         if (foundStr) {
            list.last().append(yyWord);

         } else {
            list.append(yyWord);
         }

         foundStr = true;

       } else if (yyTok == Tok_Identifier) {

         if ((kind == Group::Tr && list.size() == 2) || (kind == Group::Translate && list.size() == 3)) {
            foundStr = false;
            plural   = true;

         } else {
            // context, text, or comment

            if (! foundStr) {
               list.append(QString());
            }

            foundStr = true;
         }

      } else if (yyTok == Tok_Integer && ( (kind == Group::Tr && list.size() == 2) ||
               (kind == Group::Translate && list.size() == 3) ||
               (kind == Group::TrId && list.size() == 1) )) {
         foundStr = false;
         plural   = true;

      }
   }

   return {list, plural};
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

   while (match(Tok_Identifier) || parenlevel > 0) {
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

      } else if (yyTok == Tok_Identifier) {
         continue;

      } else if (yyTok == Tok_Arrow) {
         yyTok = getToken();

      } else if (parenlevel == 0 || yyTok == Tok_Cancel) {
         return false;
      }
   }

   return true;
}

QString CppParser::transcode(const QString &str)
{
   static const char tab[]     = "abfnrtv";
   static const char backTab[] = "\a\b\f\n\r\t\v";

   const QByteArray in = str.toUtf8();

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

         if (c == 'x' || c == 'u' || c == 'U') {
            int hexCount = 2;

            if (c == 'u') {
               hexCount = 4;

            } else if (c == 'U') {
               hexCount = 8;
            }

            QByteArray hex;

            for (int count = 0; count < hexCount; ++count) {

               if (i >= in.length() || ! isxdigit((c = in[i]))) {
                  break;
               }

               hex += c;
               ++i;
            }

            out += QString(char32_t(hex.toUInt(nullptr, 16))).toUtf8();


         } else if (c >= '0' && c < '8') {
            QByteArray oct;
            int n = 0;
            oct += c;

            while (n < 2 && i < in.length() && (c = in[i]) >= '0' && c < '8') {
               ++i;
               ++n;
               oct += c;
            }

            out += oct.toUInt(nullptr, 8);

         } else {
            const char *p = strchr(tab, c);
            out += !p ? c : backTab[p - tab];
         }

      } else {
         out += c;
      }
   }

   return QString::fromUtf8(out.constData(), out.length());
}

void CppParser::recordMessage(int line, const QString &context, const QString &text, const QString &comment,
                              const QString &extracomment, const QString &msgid, const QHash<QString, QString> &extra, bool plural)
{
   TranslatorMessage msg(transcode(context), transcode(text), transcode(comment), QString(),
                         yyFileName, line, QStringList(), TranslatorMessage::Type::Unfinished, plural);

   msg.setExtraComment(transcode(extracomment.simplified()));
   msg.setId(msgid);
   msg.setExtras(extra);
   m_translator->append(msg);
}

void CppParser::handleCsMarkTr(Group kind, QString prefix)
{
   if (! sourcetext.isEmpty()) {
      yyMsg() << "//% can not be used with the tr() function\n";
   }

   int line = yyLineNo;

   auto [list, plural] = getList(kind);
   int listLen = list.length();

   switch (listLen) {
      case 4:
         context = list.takeFirst();
         text    = list.takeFirst();
         comment = list.takeFirst();
         plural  = true;

         break;

      case 3:
         if (kind == Group::Translate || kind == Group::CsMarkTr) {
            context = list.takeFirst();
            text    = list.takeFirst();
            comment = list.takeFirst();

            kind = Group::Translate;

         } else {
            text    = list.takeFirst();
            comment = list.takeFirst();
            plural  = true;

         }

         break;

      case 2:
         if (kind == Group::Translate || kind == Group::CsMarkTr) {
            context = list.takeFirst();
            text    = list.takeFirst();

            kind = Group::Translate;

         } else {
            text    = list.takeFirst();
            comment = list.takeFirst();
         }

         break;

      case 1:
         text = list.takeFirst();
         kind = Group::Tr;

         break;
   }

   if (text.isEmpty()) {
      // do nothing

   } else if (kind == Group::Translate)  {
      recordMessage(line, context, text, comment, extracomment, msgid, extra, plural);

   } else if (kind == Group::Tr)  {

      if (! pendingContext.isEmpty() && ! prefix.startsWith("::")) {
         QList<HashString> unresolved;

         if (! fullyQualify(namespaces, pendingContext, true, &functionContext, &unresolved)) {
            functionContextUnresolved = stringifyNamespace(0, unresolved);

            yyMsg() << csPrintable(QString("Trying to use unknown namespace or class %1 %2\n")
                                   .formatArg(stringifyNamespace(functionContext)).formatArg(unresolved.first().value()));
         }

         pendingContext.clear();
      }

      bool done = false;

      if (prefix.isEmpty()) {
         if (functionContextUnresolved.isEmpty()) {
            int idx = functionContext.length();

            if (idx < 2) {
               yyMsg() << "Function tr() can not be called without a context\n";
               return;
            }

            Namespace *fctx;

            while (! (fctx = findNamespace(functionContext, idx)->classDef)->hasTrFunctions) {

               if (idx == 1) {
                  context = stringifyNamespace(functionContext);
                  fctx    = findNamespace(functionContext)->classDef;

                  if (! fctx->complained) {
                     yyMsg() << csPrintable(QString("Class %1 is missing a call to the CS_OBJECT macro\n").formatArg(context));
                     fctx->complained = true;
                  }

                  done = true;
                  break;
               }

               --idx;
            }

            if (! done) {
               if (fctx->trQualification.isEmpty()) {
                  context.clear();

                  int i = 1;

                  while (true) {
                     context += functionContext.at(i).value();
                     ++i;

                     if (i == idx) {
                        break;
                     }

                     context += "::";
                  }

                  fctx->trQualification = context;

               } else {
                  context = fctx->trQualification;
               }
            }

         } else {
            context = joinNamespaces(stringifyNamespace(functionContext), functionContextUnresolved);
         }

      } else {
         // use prefix

         prefix.chop(2);
         QList<HashString> nsl;
         QList<HashString> unresolved;

         if (fullyQualify(functionContext, prefix, false, &nsl, &unresolved)) {
            Namespace *fctx = findNamespace(nsl)->classDef;

            if (fctx->trQualification.isEmpty()) {
               context = stringifyNamespace(nsl);
               fctx->trQualification = context;

            } else {
               context = fctx->trQualification;
            }

            if (! fctx->hasTrFunctions && ! fctx->complained) {
               yyMsg() << csPrintable(QString("Class %1 is missing a call to the CS_OBJECT macro\n").formatArg(context));
               fctx->complained = true;
            }

         } else {
            context = joinNamespaces(stringifyNamespace(nsl), stringifyNamespace(0, unresolved));
         }

         prefix.clear();
      }

      recordMessage(line, context, text, comment, extracomment, msgid, extra, plural);
   }

   sourcetext.clear();           // will have warned about that already
   extracomment.clear();
   msgid.clear();
   extra.clear();
   metaExpected = false;
}

void CppParser::handleTrId()
{
   if (! msgid.isEmpty()) {
      yyMsg() << "//= can not be used with the qtTrId() function\n";
   }

   int line = yyLineNo;

   auto [list, plural] = getList(Group::TrId);
   int listLen = list.length();

   if (listLen > 0) {
      msgid = list.takeFirst();

      if (! msgid.isEmpty()) {
         recordMessage(line, QString(), sourcetext, QString(), extracomment, msgid, extra, plural);
      }
   }

   sourcetext.clear();
   extracomment.clear();
   msgid.clear();
   extra.clear();
   metaExpected = false;
}

void CppParser::handleDeclareTrFunctions()
{
   QString name;

   while (true) {
      yyTok = getToken();

      if (yyTok != Tok_Identifier) {
         return;
      }

      name += yyWord;
      yyTok = getToken();

      if (yyTok == Tok_RightParen) {
         break;
      }

      if (yyTok != Tok_ColonColon) {
         return;
      }

      name += "::";
   }

   Namespace *ns = modifyNamespace(&namespaces);

   ns->hasTrFunctions    = true;
   ns->trQualification   = name;
}

void CppParser::parse(ConversionData &cd, const QStringList &includeStack, QSet<QString> &inclusions)
{
   namespaces << HashString();
   functionContext = namespaces;
   functionContextUnresolved.clear();

   parseInternal(cd, includeStack, inclusions);
}

void CppParser::parseInternal(ConversionData &cd, const QStringList &includeStack, QSet<QString> &inclusions)
{
   static QString strColons("::");

   QString prefix;

   bool yyTokColonSeen = false;    // Start of m_translator initializer list
   bool yyTokIdentSeen = false;    // Start of initializer (member or base class)
   metaExpected = true;

   prospectiveContext.clear();
   pendingContext.clear();

   yyIter = yyInStr.cbegin();
   yyCh   = getChar();
   yyTok  = getToken();

   while (yyTok != Tok_Eof) {
      // these are array indexing operations. we ignore them entirely
      // so they don't confuse our scoping of static initializers.
      // we enter the loop by either reading a left bracket or by an
      // #else popping the state.

      if (yyBracketDepth && yyBraceDepth == namespaceDepths.count()) {
         yyTok = getToken();
         continue;
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
         [[fallthrough]];

         case Tok_AngledInclude: {
            QStringList cSources = cd.m_allCSources.values(yyWord);

            if (! cSources.isEmpty()) {
               for (const QString &cSource : cSources) {
                  processInclude(cSource, cd, includeStack, inclusions);
               }

               goto incOk;
            }

            for (const QString &incPath : cd.m_includePath) {
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

         case Tok_Friend:
            yyTok = getToken();

            // these are forward declarations so ignore them
            if (yyTok == Tok_Class) {
               yyTok = getToken();
            }
            break;

         case Tok_Class:
            // Partial support for inlined functions

            yyTok = getToken();

            if (yyBraceDepth == namespaceDepths.count() && yyParenDepth == 0) {
               QList<HashString> quali;
               HashString fct;

               // Find class name including qualification

               while (true) {
                  text = yyWord;

                  fct.setValue(text);
                  yyTok = getToken();

                  if (yyTok == Tok_ColonColon) {
                     quali << fct;
                     yyTok = getToken();

                  } else if (yyTok == Tok_Identifier) {
                     if (yyWord == text_final) {
                        // final may appear immediately after the name of the class
                        yyTok = getToken();
                        break;
                     }

                     // Handle impure definitions such as 'class Q_EXPORT QMessageBox', in
                     // which case 'QMessageBox' is the class name, not 'Q_EXPORT', by
                     // abandoning any qualification collected so far.
                     quali.clear();

                  } else {
                     break;

                  }
               }

               if (yyTok == Tok_Colon || yyTok == Tok_Other) {
                  // Skip any token until '{' since we might do things wrong if we find
                  // a '::' token here

                  do {
                     yyTok = getToken();

                     if (yyTok == Tok_Eof) {
                        goto goteof;
                     }

                     if (yyTok == Tok_Cancel) {
                        goto case_default;
                     }

                  } while (yyTok != Tok_LeftBrace && yyTok != Tok_Semicolon);

               } else {
                  if (yyTok != Tok_LeftBrace) {
                     // forward declaration, skip those, as they do not create actually usable namespaces
                     break;
                  }
               }

               if (! quali.isEmpty()) {
                  // Forward-declared class definitions can be namespaced
                  QList<HashString> nsl;

                  if (! fullyQualify(namespaces, quali, true, &nsl, nullptr)) {
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
               functionContextUnresolved.clear();
               prospectiveContext.clear();
               pendingContext.clear();

               metaExpected = true;
               yyTok = getToken();
            }
            break;

         case Tok_Namespace:
            yyTok = getToken();

            if (yyTok == Tok_Identifier) {
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
                  metaExpected = true;
                  yyTok = getToken();

               } else if (yyTok == Tok_Equals) {
                  // for example, namespace Is = OuterSpace::InnerSpace;
                  QList<HashString> fullName;
                  yyTok = getToken();

                  if (yyTok == Tok_ColonColon) {
                     fullName.append(HashString(QString()));
                  }

                  while (yyTok == Tok_ColonColon || yyTok == Tok_Identifier) {
                     if (yyTok == Tok_Identifier) {
                        text = yyWord;
                        fullName.append(HashString(text));
                     }

                     yyTok = getToken();
                  }

                  if (fullName.isEmpty()) {
                     break;
                  }

                  fullName.append(HashString(QString()));   // Mark as unresolved
                  modifyNamespace(&namespaces)->aliases[ns] = fullName;
               }

            } else if (yyTok == Tok_LeftBrace) {
               // Anonymous namespace
               namespaceDepths.push(namespaces.count());
               metaExpected = true;
               yyTok = getToken();
            }
            break;

         case Tok_Using:
            yyTok = getToken();

            // should affect only the current scope, not the entire current namespace
            if (yyTok == Tok_Namespace) {
               QList<HashString> fullName;
               yyTok = getToken();

               if (yyTok == Tok_ColonColon) {
                  fullName.append(HashString(QString()));
               }

               while (yyTok == Tok_ColonColon || yyTok == Tok_Identifier) {
                  if (yyTok == Tok_Identifier) {
                     text = yyWord;
                     fullName.append(HashString(text));
                  }
                  yyTok = getToken();
               }

               QList<HashString> nsl;
               if (fullyQualify(namespaces, fullName, false, &nsl, nullptr)) {
                  modifyNamespace(&namespaces)->usings << HashStringList(nsl);
               }

            } else {
               QList<HashString> fullName;

               if (yyTok == Tok_ColonColon) {
                  fullName.append(HashString(QString()));
               }

               while (yyTok == Tok_ColonColon || yyTok == Tok_Identifier) {
                  if (yyTok == Tok_Identifier) {
                     text = yyWord;
                     fullName.append(HashString(text));
                  }
                  yyTok = getToken();
               }

               if (fullName.isEmpty()) {
                  break;
               }

               // using-declarations can not rename classes, so the last element of
               // fullName is already the resolved name we actually want.
               // As we do no resolution here, we'll collect useless usings of data
               // members and methods as well. This is no big deal.

               fullName.append(HashString(QString())); // Mark as unresolved
               const HashString &ns = *(fullName.constEnd() - 2);
               modifyNamespace(&namespaces)->aliases[ns] = fullName;
            }
            break;

         case Tok_CS_OBJECT:
            modifyNamespace(&namespaces)->hasTrFunctions = true;
            yyTok = getToken();
            break;

         case Tok_Identifier:
            if (yyTokColonSeen && yyBraceDepth == namespaceDepths.count() && yyParenDepth == 0) {
               // member or base class identifier
               yyTokIdentSeen = true;
            }

            yyTok = getToken();

            if (yyTok == Tok_LeftParen) {

               if (yyWord == text_Q_DECLARE_TR_FUNCTIONS) {
                  handleDeclareTrFunctions();

               } else if (yyWord == text_cs_mark_tr || yyWord == text_cs_mark_string_tr) {

                  if (m_translator != nullptr) {
                     handleCsMarkTr(Group::CsMarkTr, prefix);
                  }

               } else if (yyWord == text_tr || yyWord == text_QT_TR_NOOP) {

                  if (m_translator != nullptr) {
                     handleCsMarkTr(Group::Tr, prefix);
                  }

               } else if (yyWord == text_findMessage || yyWord == text_translate ||
                          yyWord == text_QT_TRANSLATE_NOOP || yyWord == text_QT_TRANSLATE_NOOP3) {

                  if (m_translator != nullptr) {
                     handleCsMarkTr(Group::Translate);
                  }

               } else if (yyWord == text_qtTrId || yyWord == text_QT_TRID_NOOP || yyWord == text_cs_mark_tr_id) {

                  if (m_translator != nullptr) {
                     handleTrId();
                  }

               } else {
                  goto notrfunc;
               }

               yyTok = getToken();
               break;
            }

            if (yyTok == Tok_ColonColon) {
               prefix += yyWord;

            } else {

            notrfunc:
               prefix.clear();

               if (yyTok == Tok_Identifier && ! yyParenDepth) {
                  prospectiveContext.clear();
               }
            }

            metaExpected = false;

            break;

         case Tok_Arrow:
            yyTok = getToken();

            if (yyTok == Tok_Identifier) {
               if (yyWord == text_tr) {
                  yyMsg() << "Unable to invoke tr()\n";
               }
            }
            break;

         case Tok_ColonColon:
            if (yyTokIdentSeen) {
               // member or base class identifier

               yyTok = getToken();
               break;
            }

            if (yyBraceDepth == namespaceDepths.count() && yyParenDepth == 0 && ! yyTokColonSeen) {
               prospectiveContext = prefix;
            }

            prefix += strColons;
            yyTok = getToken();
            break;

         case Tok_RightBrace:
            if (! yyTokColonSeen) {
               if (yyBraceDepth + 1 == namespaceDepths.count()) {
                  // class or namespace
                  truncateNamespaces(&namespaces, namespaceDepths.pop());
               }

               if (yyBraceDepth == namespaceDepths.count()) {
                  // function, class or namespace

                  if (! yyBraceDepth && !directInclude) {
                     truncateNamespaces(&functionContext, 1);

                  } else {
                     functionContext = namespaces;
                     functionContextUnresolved.clear();
                  }
                  pendingContext.clear();
               }
            }
            [[fallthrough]];

         case Tok_Semicolon:
            prospectiveContext.clear();
            prefix.clear();

            if (! sourcetext.isEmpty() || !extracomment.isEmpty() || !msgid.isEmpty() || !extra.isEmpty()) {
               yyMsg() << "Discarding unconsumed meta data\n";
               sourcetext.clear();
               extracomment.clear();
               msgid.clear();
               extra.clear();
            }

            metaExpected = true;
            yyTok = getToken();
            break;

         case Tok_Access:
            // consume access specifiers so their colons are not mistaken for ctor initializer list starts
            do {
               yyTok = getToken();
            } while (yyTok == Tok_Access);   // Multiple specifiers are possible

            metaExpected = true;

            if (yyTok == Tok_Colon) {
               goto case_default;
            }
            break;

         case Tok_Colon:
         case Tok_Equals:
            if (yyBraceDepth == namespaceDepths.count() && yyParenDepth == 0) {

               if (! prospectiveContext.isEmpty()) {
                  pendingContext = prospectiveContext;
                  prospectiveContext.clear();
               }

               if (yyTok == Tok_Colon) {
                  yyTokColonSeen = true;
               }
            }

            metaExpected = true;
            yyTok = getToken();
            break;

         case Tok_LeftBrace:
            if (yyBraceDepth == namespaceDepths.count() + 1 && yyParenDepth == 0) {

               if (! prospectiveContext.isEmpty()) {
                  pendingContext = prospectiveContext;
                  prospectiveContext.clear();
               }

               if (! yyTokIdentSeen) {
                  // Function body
                  yyTokColonSeen = false;
               }
            }
            [[fallthrough]];

         case Tok_LeftParen:
            yyTokIdentSeen = false;
            [[fallthrough]];

         case Tok_Comma:
         case Tok_QuestionMark:
            metaExpected = true;
            yyTok = getToken();
            break;

         case Tok_RightParen:
            metaExpected = false;
            yyTok = getToken();
            break;

         default:
            if (! yyParenDepth) {
               prospectiveContext.clear();
            }
            [[fallthrough]];

         case Tok_RightBracket:
         case_default:
            yyTok = getToken();
            break;
      }
   }

goteof:
   if (yyBraceDepth != 0) {
      yyMsg(yyBraceLineNo) << "Unbalanced opening brace in C++ code"
                              " (or misuse of the preprocessor)\n";

   } else if (yyParenDepth != 0) {
      yyMsg(yyParenLineNo) << "Unbalanced opening parenthesis in C++ code"
                              " (or misuse of the preprocessor)\n";

   } else if (yyBracketDepth != 0) {
      yyMsg(yyBracketLineNo) << "Unbalanced opening bracket in C++ code"
                                " (or misuse of the preprocessor)\n";
   }
}

void CppParser::processComment()
{
   if (m_translator == nullptr || ! metaExpected) {
      return;
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

      QString::const_iterator iter = yyWord.cbegin() + 2;
      QChar c;

      while (true) {
         if (iter == yyWord.cend()) {
            break;
         }

         c = *iter;
         ++iter;

         if (c.isSpace()) {
            continue;
         }

         if (c != '"') {
            yyMsg() << "Unexpected character in meta string\n";
            break;
         }

         while (true) {

            if (iter == yyWord.cend()) {
               yyMsg() << "Unterminated meta string\n";
               break;
            }

            c = *iter;
            ++iter;

            if (c == '"') {
               break;
            }

            if (c == '\\') {
               if (iter == yyWord.cend()) {
                  yyMsg() << "Unterminated meta string\n";
                  break;
               }

               c = *iter;
               ++iter;

               if (c == '\n') {
                  yyMsg() << "Unterminated meta string\n";
                  break;
               }

               sourcetext.append('\\');
            }

            sourcetext.append(c);
         }
      }

   } else {
      QString::const_iterator iter = yyWord.cbegin();
      QChar c;

      while (iter != yyWord.cend()) {
         c = *iter;

         if (c == ' ' || c == '\t' || c == '\n') {
            ++iter;
         } else {
            break;
         }
      }

      QStringView tmpWord(iter, yyWord.cend());

      if (tmpWord.startsWith(MagicComment)) {

         iter += MagicComment.length();
         comment = QString(iter, yyWord.cend()).simplified();

         auto tmpIter = comment.indexOfFast(' ');

         if (tmpIter == comment.cend()) {
            context = comment;

         } else {
            context = QString(comment.begin(), tmpIter);
            comment.erase(comment.begin(), tmpIter + 1);

            TranslatorMessage msg( transcode(context), QString(), transcode(comment), QString(),
                                   yyFileName, yyLineNo, QStringList(), TranslatorMessage::Type::Finished, false);

            msg.setExtraComment(transcode(extracomment.simplified()));
            extracomment.clear();
            m_translator->append(msg);
            m_translator->setExtras(extra);
            extra.clear();
         }
      }
   }
}

const ParseResults *CppParser::recordResults(bool isHeader)
{
   if (m_translator != nullptr) {
      if (m_translator->messageCount()) {
         CppFiles::setTranslator(yyFileName, m_translator);

      } else {
         delete m_translator;
         m_translator = nullptr;
      }
   }

   if (isHeader) {
      const ParseResults *pr;

      if (m_translator == nullptr && results->includes.count() == 1 && results->rootNamespace.children.isEmpty()
            && results->rootNamespace.aliases.isEmpty() && results->rootNamespace.usings.isEmpty()) {

         // forwarding header
         pr = *results->includes.cbegin();
         delete results;

      } else {
         results->fileId = nextFileId++;
         pr = results;
      }

      CppFiles::setResults(yyFileName, pr);
      return pr;

   } else {
      delete results;
      return nullptr;
   }
}

void loadCPP(Translator &translator, const QStringList &filenames, ConversionData &cd)
{
   QTextCodec *codec = QTextCodec::codecForName("UTF-8");

   for (const QString &filename : filenames) {

      if (! CppFiles::getResults(filename).isEmpty() || CppFiles::isBlacklisted(filename)) {
         continue;
      }

      QFile file(filename);

      if (! file.open(QIODevice::ReadOnly)) {
         cd.appendError(QString("Unable to open %1: %2").formatArgs(filename, file.errorString()));
         continue;
      }

      CppParser parser;

      QTextStream ts(&file);
      ts.setCodec(codec);
      ts.setAutoDetectUnicode(true);
      parser.setInput(ts, filename);

      Translator *obj = new Translator;
      parser.setTranslator(obj);

      QSet<QString> inclusions;
      parser.parse(cd, QStringList(), inclusions);
      parser.recordResults(isHeader(filename));
   }

   for (const QString &filename : filenames) {
      if (! CppFiles::isBlacklisted(filename)) {

         if (const Translator *obj = CppFiles::getTranslator(filename)) {
            for (const TranslatorMessage &msg : obj->messages()) {
               translator.extend(msg, cd);
            }
         }
      }
   }
}

