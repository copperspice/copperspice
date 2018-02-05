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

#include <qregexp.h>

#include <qalgorithms.h>
#include <qbitarray.h>
#include <qcache.h>
#include <qdatastream.h>
#include <qlist.h>
#include <qmap.h>
#include <qmutex.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvector.h>
#include <qfunctions_p.h>

#include <limits.h>
#include <algorithm>
#include <stdlib.h>

int qFindString(const QChar *haystack, int haystackLen, int from,
                const QChar *needle, int needleLen, Qt::CaseSensitivity cs);

// error strings for the regexp parser
#define RXERR_OK         QT_TRANSLATE_NOOP("QRegExp", "no error occurred")
#define RXERR_DISABLED   QT_TRANSLATE_NOOP("QRegExp", "disabled feature used")
#define RXERR_CHARCLASS  QT_TRANSLATE_NOOP("QRegExp", "bad char class syntax")
#define RXERR_LOOKAHEAD  QT_TRANSLATE_NOOP("QRegExp", "bad lookahead syntax")
#define RXERR_LOOKBEHIND QT_TRANSLATE_NOOP("QRegExp", "lookbehinds not supported, see QTBUG-2371")
#define RXERR_REPETITION QT_TRANSLATE_NOOP("QRegExp", "bad repetition syntax")
#define RXERR_OCTAL      QT_TRANSLATE_NOOP("QRegExp", "invalid octal value")
#define RXERR_LEFTDELIM  QT_TRANSLATE_NOOP("QRegExp", "missing left delim")
#define RXERR_END        QT_TRANSLATE_NOOP("QRegExp", "unexpected end")
#define RXERR_LIMIT      QT_TRANSLATE_NOOP("QRegExp", "met internal limit")
#define RXERR_INTERVAL   QT_TRANSLATE_NOOP("QRegExp", "invalid interval")
#define RXERR_CATEGORY   QT_TRANSLATE_NOOP("QRegExp", "invalid category")

const int NumBadChars = 64;
#define BadChar(ch) ((ch).unicode() % NumBadChars)

const int NoOccurrence = INT_MAX;
const int EmptyCapture = INT_MAX;
const int InftyLen = INT_MAX;
const int InftyRep = 1025;
const int EOS = -1;

static bool isWord(QChar ch)
{
   return ch.isLetterOrNumber() || ch.isMark() || ch == QLatin1Char('_');
}

/*
  Merges two vectors of ints and puts the result into the first one.
*/
static void mergeInto(QVector<int> *a, const QVector<int> &b)
{
   int asize = a->size();
   int bsize = b.size();

   if (asize == 0) {
      *a = b;

   } else if (bsize == 1 && a->at(asize - 1) < b.at(0)) {
      a->resize(asize + 1);
      (*a)[asize] = b.at(0);

   } else if (bsize >= 1) {
      int csize = asize + bsize;
      QVector<int> c(csize);
      int i = 0, j = 0, k = 0;

      while (i < asize) {
         if (j < bsize) {
            if (a->at(i) == b.at(j)) {
               ++i;
               --csize;
            } else if (a->at(i) < b.at(j)) {
               c[k++] = a->at(i++);
            } else {
               c[k++] = b.at(j++);
            }
         } else {
            memcpy(c.data() + k, a->constData() + i, (asize - i) * sizeof(int));
            break;
         }
      }

      c.resize(csize);
      if (j < bsize) {
         memcpy(c.data() + k, b.constData() + j, (bsize - j) * sizeof(int));
      }
      *a = c;
   }
}

#ifndef QT_NO_REGEXP_WILDCARD
/*
  Translates a wildcard pattern to an equivalent regular expression
  pattern (e.g., *.cpp to .*\.cpp).

  If enableEscaping is true, it is possible to escape the wildcard
  characters with \
*/
static QString wc2rx(const QString &wc_str, const bool enableEscaping)
{
   const int wclen = wc_str.length();
   QString rx;
   int i = 0;
   bool isEscaping = false; // the previous character is '\'
   const QChar *wc = wc_str.unicode();

   while (i < wclen) {
      const QChar c = wc[i++];
      switch (c.unicode()) {
         case '\\':
            if (enableEscaping) {
               if (isEscaping) {
                  rx += QLatin1String("\\\\");
               } // we insert the \\ later if necessary
               if (i == wclen) { // the end
                  rx += QLatin1String("\\\\");
               }
            } else {
               rx += QLatin1String("\\\\");
            }
            isEscaping = true;
            break;
         case '*':
            if (isEscaping) {
               rx += QLatin1String("\\*");
               isEscaping = false;
            } else {
               rx += QLatin1String(".*");
            }
            break;
         case '?':
            if (isEscaping) {
               rx += QLatin1String("\\?");
               isEscaping = false;
            } else {
               rx += QLatin1Char('.');
            }

            break;
         case '$':
         case '(':
         case ')':
         case '+':
         case '.':
         case '^':
         case '{':
         case '|':
         case '}':
            if (isEscaping) {
               isEscaping = false;
               rx += QLatin1String("\\\\");
            }
            rx += QLatin1Char('\\');
            rx += c;
            break;
         case '[':
            if (isEscaping) {
               isEscaping = false;
               rx += QLatin1String("\\[");
            } else {
               rx += c;
               if (wc[i] == QLatin1Char('^')) {
                  rx += wc[i++];
               }
               if (i < wclen) {
                  if (rx[i] == QLatin1Char(']')) {
                     rx += wc[i++];
                  }
                  while (i < wclen && wc[i] != QLatin1Char(']')) {
                     if (wc[i] == QLatin1Char('\\')) {
                        rx += QLatin1Char('\\');
                     }
                     rx += wc[i++];
                  }
               }
            }
            break;

         case ']':
            if (isEscaping) {
               isEscaping = false;
               rx += QLatin1String("\\");
            }
            rx += c;
            break;

         default:
            if (isEscaping) {
               isEscaping = false;
               rx += QLatin1String("\\\\");
            }
            rx += c;
      }
   }
   return rx;
}
#endif

static int caretIndex(int offset, QRegExp::CaretMode caretMode)
{
   if (caretMode == QRegExp::CaretAtZero) {
      return 0;
   } else if (caretMode == QRegExp::CaretAtOffset) {
      return offset;
   } else { // QRegExp::CaretWontMatch
      return -1;
   }
}

/*
    The QRegExpEngineKey struct uniquely identifies an engine.
*/
struct QRegExpEngineKey {
   QString pattern;
   QRegExp::PatternSyntax patternSyntax;
   Qt::CaseSensitivity cs;

   inline QRegExpEngineKey(const QString &pattern, QRegExp::PatternSyntax patternSyntax,
                           Qt::CaseSensitivity cs)
      : pattern(pattern), patternSyntax(patternSyntax), cs(cs) {}

   inline void clear() {
      pattern.clear();
      patternSyntax = QRegExp::RegExp;
      cs = Qt::CaseSensitive;
   }
};

static bool operator==(const QRegExpEngineKey &key1, const QRegExpEngineKey &key2)
{
   return key1.pattern == key2.pattern && key1.patternSyntax == key2.patternSyntax
          && key1.cs == key2.cs;
}

class QRegExpEngine;

/*
  This is the engine state during matching.
*/
struct QRegExpMatchState {
   const QChar *in; // a pointer to the input string data
   int pos; // the current position in the string
   int caretPos;
   int len; // the length of the input string
   bool minimal; // minimal matching?
   int *bigArray; // big array holding the data for the next pointers
   int *inNextStack; // is state is nextStack?
   int *curStack; // stack of current states
   int *nextStack; // stack of next states
   int *curCapBegin; // start of current states' captures
   int *nextCapBegin; // start of next states' captures
   int *curCapEnd; // end of current states' captures
   int *nextCapEnd; // end of next states' captures
   int *tempCapBegin; // start of temporary captures
   int *tempCapEnd; // end of temporary captures
   int *capBegin; // start of captures for a next state
   int *capEnd; // end of captures for a next state
   int *slideTab; // bump-along slide table for bad-character heuristic
   int *captured; // what match() returned last
   int slideTabSize; // size of slide table
   int capturedSize;

#ifndef QT_NO_REGEXP_BACKREF
   QList<QVector<int> > sleeping; // list of back-reference sleepers
#endif

   int matchLen;             // length of match
   int oneTestMatchedLen;    // length of partial match

   const QRegExpEngine *eng;

   inline QRegExpMatchState() : bigArray(0), captured(0) {}
   inline ~QRegExpMatchState() {
      free(bigArray);
   }

   void drain() {
      free(bigArray);   // to save memory
      bigArray = 0;
      captured = 0;
   }
   void prepareForMatch(QRegExpEngine *eng);
   void match(const QChar *str, int len, int pos, bool minimal,
              bool oneTest, int caretIndex);
   bool matchHere();
   bool testAnchor(int i, int a, const int *capBegin);
};

/*
  The struct QRegExpAutomatonState represents one state in a modified NFA. The
  input characters matched are stored in the state instead of on
  the transitions, something possible for an automaton
  constructed from a regular expression.
*/
struct QRegExpAutomatonState {
#ifndef QT_NO_REGEXP_CAPTURE
   int atom; // which atom does this state belong to?
#endif
   int match; // what does it match? (see CharClassBit and BackRefBit)
   QVector<int> outs; // out-transitions
   QMap<int, int> reenter; // atoms reentered when transiting out
   QMap<int, int> anchors; // anchors met when transiting out

   inline QRegExpAutomatonState() { }
#ifndef QT_NO_REGEXP_CAPTURE
   inline QRegExpAutomatonState(int a, int m)
      : atom(a), match(m) { }
#else
   inline QRegExpAutomatonState(int m)
      : match(m) { }
#endif
};

Q_DECLARE_TYPEINFO(QRegExpAutomatonState, Q_MOVABLE_TYPE);

/*
  The struct QRegExpCharClassRange represents a range of characters (e.g.,
  [0-9] denotes range 48 to 57).
*/
struct QRegExpCharClassRange {
   ushort from; // 48
   ushort len; // 10
};

Q_DECLARE_TYPEINFO(QRegExpCharClassRange, Q_PRIMITIVE_TYPE);

#ifndef QT_NO_REGEXP_CAPTURE
/*
  The struct QRegExpAtom represents one node in the hierarchy of regular
  expression atoms.
*/
struct QRegExpAtom {
   enum { NoCapture = -1, OfficialCapture = -2, UnofficialCapture = -3 };

   int parent; // index of parent in array of atoms
   int capture; // index of capture, from 1 to ncap - 1
};

Q_DECLARE_TYPEINFO(QRegExpAtom, Q_PRIMITIVE_TYPE);
#endif

struct QRegExpLookahead;

#ifndef QT_NO_REGEXP_ANCHOR_ALT
/*
  The struct QRegExpAnchorAlternation represents a pair of anchors with
  OR semantics.
*/
struct QRegExpAnchorAlternation {
   int a; // this anchor...
   int b; // ...or this one
};

Q_DECLARE_TYPEINFO(QRegExpAnchorAlternation, Q_PRIMITIVE_TYPE);
#endif

#define FLAG(x) (1 << (x))

#ifndef QT_NO_REGEXP_CCLASS
/*
  The class QRegExpCharClass represents a set of characters, such as can
  be found in regular expressions (e.g., [a-z] denotes the set
  {a, b, ..., z}).
*/
class QRegExpCharClass
{
 public:
   QRegExpCharClass();

   inline QRegExpCharClass(const QRegExpCharClass &cc) {
      operator=(cc);
   }

   QRegExpCharClass &operator=(const QRegExpCharClass &cc);

   void clear();
   bool negative() const {
      return n;
   }
   void setNegative(bool negative);
   void addCategories(uint cats);
   void addRange(ushort from, ushort to);

   void addSingleton(ushort ch) {
      addRange(ch, ch);
   }

   bool in(QChar ch) const;

   const QVector<int> &firstOccurrence() const {
      return occ1;
   }


#if defined(QT_DEBUG)
   void dump() const;
#endif

 private:
   int c; // character classes
   QVector<QRegExpCharClassRange> r; // character ranges
   bool n; // negative?

   QVector<int> occ1; // first-occurrence array
};

#else

struct QRegExpCharClass {
   int dummy;

   QRegExpCharClass() {
      occ1.fill(0, NumBadChars);
   }

   const QVector<int> &firstOccurrence() const {
      return occ1;
   }
   QVector<int> occ1;

};
#endif

Q_DECLARE_TYPEINFO(QRegExpCharClass, Q_MOVABLE_TYPE);

/*
  The QRegExpEngine class encapsulates a modified nondeterministic
  finite automaton (NFA).
*/
class QRegExpEngine
{
 public:
   QRegExpEngine(Qt::CaseSensitivity cs, bool greedyQuantifiers)
      : cs(cs), greedyQuantifiers(greedyQuantifiers) {
      setup();
   }

   QRegExpEngine(const QRegExpEngineKey &key);
   ~QRegExpEngine();

   bool isValid() const {
      return valid;
   }
   const QString &errorString() const {
      return yyError;
   }
   int captureCount() const {
      return officialncap;
   }

   int createState(QChar ch);
   int createState(const QRegExpCharClass &cc);
#ifndef QT_NO_REGEXP_BACKREF
   int createState(int bref);
#endif

   void addCatTransitions(const QVector<int> &from, const QVector<int> &to);
#ifndef QT_NO_REGEXP_CAPTURE
   void addPlusTransitions(const QVector<int> &from, const QVector<int> &to, int atom);
#endif

#ifndef QT_NO_REGEXP_ANCHOR_ALT
   int anchorAlternation(int a, int b);
   int anchorConcatenation(int a, int b);
#else
   int anchorAlternation(int a, int b) {
      return a & b;
   }
   int anchorConcatenation(int a, int b) {
      return a | b;
   }
#endif
   void addAnchors(int from, int to, int a);

   void heuristicallyChooseHeuristic();


#if defined(QT_DEBUG)
   void dump() const;
#endif

   QAtomicInt ref;

 private:
   enum { CharClassBit = 0x10000, BackRefBit = 0x20000 };
   enum { InitialState = 0, FinalState = 1 };

   void setup();
   int setupState(int match);

   /*
     Let's hope that 13 lookaheads and 14 back-references are
     enough.
    */
   enum { MaxLookaheads = 13, MaxBackRefs = 14 };
   enum { Anchor_Dollar = 0x00000001, Anchor_Caret = 0x00000002, Anchor_Word = 0x00000004,
          Anchor_NonWord = 0x00000008, Anchor_FirstLookahead = 0x00000010,
          Anchor_BackRef1Empty = Anchor_FirstLookahead << MaxLookaheads,
          Anchor_BackRef0Empty = Anchor_BackRef1Empty >> 1,
          Anchor_Alternation = unsigned(Anchor_BackRef1Empty) << MaxBackRefs,

          Anchor_LookaheadMask = (Anchor_FirstLookahead - 1) ^
                                 ((Anchor_FirstLookahead << MaxLookaheads) - 1)
        };
#ifndef QT_NO_REGEXP_CAPTURE
   int startAtom(bool officialCapture);
   void finishAtom(int atom, bool needCapture);
#endif

#ifndef QT_NO_REGEXP_LOOKAHEAD
   int addLookahead(QRegExpEngine *eng, bool negative);
#endif

   bool goodStringMatch(QRegExpMatchState &matchState) const;
   bool badCharMatch(QRegExpMatchState &matchState) const;

   QVector<QRegExpAutomatonState> s; // array of states
#ifndef QT_NO_REGEXP_CAPTURE
   QVector<QRegExpAtom> f; // atom hierarchy
   int nf; // number of atoms
   int cf; // current atom
   QVector<int> captureForOfficialCapture;
#endif
   int officialncap; // number of captures, seen from the outside
   int ncap; // number of captures, seen from the inside

#ifndef QT_NO_REGEXP_CCLASS
   QVector<QRegExpCharClass> cl; // array of character classes
#endif

#ifndef QT_NO_REGEXP_LOOKAHEAD
   QVector<QRegExpLookahead *> ahead; // array of lookaheads
#endif

#ifndef QT_NO_REGEXP_ANCHOR_ALT
   QVector<QRegExpAnchorAlternation> aa; // array of (a, b) pairs of anchors
#endif

   bool caretAnchored; // does the regexp start with ^?
   bool trivial; // is the good-string all that needs to match?

   bool valid; // is the regular expression valid?
   Qt::CaseSensitivity cs; // case sensitive?
   bool greedyQuantifiers; // RegExp2?
   bool xmlSchemaExtensions;
#ifndef QT_NO_REGEXP_BACKREF
   int nbrefs; // number of back-references
#endif

   bool useGoodStringHeuristic; // use goodStringMatch? otherwise badCharMatch

   int goodEarlyStart; // the index where goodStr can first occur in a match
   int goodLateStart; // the index where goodStr can last occur in a match
   QString goodStr; // the string that any match has to contain

   int minl; // the minimum length of a match
   QVector<int> occ1; // first-occurrence array

   /*
     The class Box is an abstraction for a regular expression
     fragment. It can also be seen as one node in the syntax tree of
     a regular expression with synthetized attributes.

     Its interface is ugly for performance reasons.
   */
   class Box
   {
    public:
      Box(QRegExpEngine *engine);
      Box(const Box &b) {
         operator=(b);
      }

      Box &operator=(const Box &b);

      void clear() {
         operator=(Box(eng));
      }
      void set(QChar ch);
      void set(const QRegExpCharClass &cc);
#ifndef QT_NO_REGEXP_BACKREF
      void set(int bref);
#endif

      void cat(const Box &b);
      void orx(const Box &b);
      void plus(int atom);
      void opt();
      void catAnchor(int a);
#ifndef QT_NO_REGEXP_OPTIM
      void setupHeuristics();
#endif

#if defined(QT_DEBUG)
      void dump() const;
#endif

    private:
      void addAnchorsToEngine(const Box &to) const;

      QRegExpEngine *eng; // the automaton under construction
      QVector<int> ls; // the left states (firstpos)
      QVector<int> rs; // the right states (lastpos)
      QMap<int, int> lanchors; // the left anchors
      QMap<int, int> ranchors; // the right anchors
      int skipanchors; // the anchors to match if the box is skipped

#ifndef QT_NO_REGEXP_OPTIM
      int earlyStart; // the index where str can first occur
      int lateStart; // the index where str can last occur
      QString str; // a string that has to occur in any match
      QString leftStr; // a string occurring at the left of this box
      QString rightStr; // a string occurring at the right of this box
      int maxl; // the maximum length of this box (possibly InftyLen)
#endif

      int minl; // the minimum length of this box
#ifndef QT_NO_REGEXP_OPTIM
      QVector<int> occ1; // first-occurrence array
#endif
   };

   friend class Box;

   // This is the lexical analyzer for regular expressions.
   enum { Tok_Eos, Tok_Dollar, Tok_LeftParen, Tok_MagicLeftParen, Tok_PosLookahead,
          Tok_NegLookahead, Tok_RightParen, Tok_CharClass, Tok_Caret, Tok_Quantifier, Tok_Bar,
          Tok_Word, Tok_NonWord, Tok_Char = 0x10000, Tok_BackRef = 0x20000
        };
   int getChar();
   int getEscape();

#ifndef QT_NO_REGEXP_INTERVAL
   int getRep(int def);
#endif
#ifndef QT_NO_REGEXP_LOOKAHEAD
   void skipChars(int n);
#endif
   void error(const char *msg);
   void startTokenizer(const QChar *rx, int len);
   int getToken();

   const QChar *yyIn; // a pointer to the input regular expression pattern
   int yyPos0; // the position of yyTok in the input pattern
   int yyPos; // the position of the next character to read
   int yyLen; // the length of yyIn
   int yyCh; // the last character read
   QScopedPointer<QRegExpCharClass> yyCharClass; // attribute for Tok_CharClass tokens
   int yyMinRep; // attribute for Tok_Quantifier
   int yyMaxRep; // ditto
   QString yyError; // syntax error or overflow during parsing?

   /*
     This is the syntactic analyzer for regular expressions.
   */
   int parse(const QChar *rx, int len);
   void parseAtom(Box *box);
   void parseFactor(Box *box);
   void parseTerm(Box *box);
   void parseExpression(Box *box);

   int yyTok;          // the last token read
   bool yyMayCapture;  // set this to false to disable capturing

   friend struct QRegExpMatchState;
};

#ifndef QT_NO_REGEXP_LOOKAHEAD
/*
  The struct QRegExpLookahead represents a lookahead a la Perl (e.g.,
  (?=foo) and (?!bar)).
*/
struct QRegExpLookahead {
   QRegExpEngine *eng; // NFA representing the embedded regular expression
   bool neg; // negative lookahead?

   inline QRegExpLookahead(QRegExpEngine *eng0, bool neg0)
      : eng(eng0), neg(neg0) { }
   inline ~QRegExpLookahead() {
      delete eng;
   }
};
#endif

/*! \internal
    convert the pattern string to the RegExp syntax.

    This is also used by QScriptEngine::newRegExp to convert to a pattern that JavaScriptCore can understan
 */
Q_CORE_EXPORT QString qt_regexp_toCanonical(const QString &pattern, QRegExp::PatternSyntax patternSyntax)
{
   switch (patternSyntax) {
#ifndef QT_NO_REGEXP_WILDCARD
      case QRegExp::Wildcard:
         return wc2rx(pattern, false);

      case QRegExp::WildcardUnix:
         return wc2rx(pattern, true);

#endif
      case QRegExp::FixedString:
         return QRegExp::escape(pattern);

      case QRegExp::W3CXmlSchema11:
      default:
         return pattern;
   }
}

QRegExpEngine::QRegExpEngine(const QRegExpEngineKey &key)
   : cs(key.cs), greedyQuantifiers(key.patternSyntax == QRegExp::RegExp2),
     xmlSchemaExtensions(key.patternSyntax == QRegExp::W3CXmlSchema11)
{
   setup();

   QString rx = qt_regexp_toCanonical(key.pattern, key.patternSyntax);

   valid = (parse(rx.unicode(), rx.length()) == rx.length());
   if (!valid) {
#ifndef QT_NO_REGEXP_OPTIM
      trivial = false;
#endif
      error(RXERR_LEFTDELIM);
   }
}

QRegExpEngine::~QRegExpEngine()
{
#ifndef QT_NO_REGEXP_LOOKAHEAD
   qDeleteAll(ahead);
#endif
}

void QRegExpMatchState::prepareForMatch(QRegExpEngine *eng)
{
   /*
     We use one QVector<int> for all the big data used a lot in
     matchHere() and friends.
   */
   int ns = eng->s.size(); // number of states
   int ncap = eng->ncap;

#ifndef QT_NO_REGEXP_OPTIM
   int newSlideTabSize = qMax(eng->minl + 1, 16);
#else
   int newSlideTabSize = 0;
#endif

   int numCaptures = eng->captureCount();
   int newCapturedSize = 2 + 2 * numCaptures;

   bigArray = q_check_ptr((int *)realloc(bigArray,
                  ((3 + 4 * ncap) * ns + 4 * ncap + newSlideTabSize + newCapturedSize) * sizeof(int)));

   // set all internal variables only _after_ bigArray is realloc'ed
   // to prevent a broken regexp in oom case

   slideTabSize = newSlideTabSize;
   capturedSize = newCapturedSize;
   inNextStack = bigArray;
   memset(inNextStack, -1, ns * sizeof(int));
   curStack = inNextStack + ns;
   nextStack = inNextStack + 2 * ns;

   curCapBegin = inNextStack + 3 * ns;
   nextCapBegin = curCapBegin + ncap * ns;
   curCapEnd = curCapBegin + 2 * ncap * ns;
   nextCapEnd = curCapBegin + 3 * ncap * ns;

   tempCapBegin = curCapBegin + 4 * ncap * ns;
   tempCapEnd = tempCapBegin + ncap;
   capBegin = tempCapBegin + 2 * ncap;
   capEnd = tempCapBegin + 3 * ncap;

   slideTab = tempCapBegin + 4 * ncap;
   captured = slideTab + slideTabSize;
   memset(captured, -1, capturedSize * sizeof(int));
   this->eng = eng;
}

/*
  Tries to match in str and returns an array of (begin, length) pairs
  for captured text. If there is no match, all pairs are (-1, -1).
*/
void QRegExpMatchState::match(const QChar *str0, int len0, int pos0,
                              bool minimal0, bool oneTest, int caretIndex)
{
   bool matched = false;
   QChar char_null;

#ifndef QT_NO_REGEXP_OPTIM
   if (eng->trivial && !oneTest) {
      pos = qFindString(str0, len0, pos0, eng->goodStr.unicode(), eng->goodStr.length(), eng->cs);
      matchLen = eng->goodStr.length();
      matched = (pos != -1);
   } else
#endif
   {
      in = str0;
      if (in == 0) {
         in = &char_null;
      }
      pos = pos0;
      caretPos = caretIndex;
      len = len0;
      minimal = minimal0;
      matchLen = 0;
      oneTestMatchedLen = 0;

      if (eng->valid && pos >= 0 && pos <= len) {
#ifndef QT_NO_REGEXP_OPTIM
         if (oneTest) {
            matched = matchHere();
         } else {
            if (pos <= len - eng->minl) {
               if (eng->caretAnchored) {
                  matched = matchHere();
               } else if (eng->useGoodStringHeuristic) {
                  matched = eng->goodStringMatch(*this);
               } else {
                  matched = eng->badCharMatch(*this);
               }
            }
         }
#else
         matched = oneTest ? matchHere() : eng->bruteMatch(*this);
#endif
      }
   }

   if (matched) {
      int *c = captured;
      *c++ = pos;
      *c++ = matchLen;

      int numCaptures = (capturedSize - 2) >> 1;
#ifndef QT_NO_REGEXP_CAPTURE
      for (int i = 0; i < numCaptures; ++i) {
         int j = eng->captureForOfficialCapture.at(i);
         if (capBegin[j] != EmptyCapture) {
            int len = capEnd[j] - capBegin[j];
            *c++ = (len > 0) ? pos + capBegin[j] : 0;
            *c++ = len;
         } else {
            *c++ = -1;
            *c++ = -1;
         }
      }
#endif
   } else {
      // we rely on 2's complement here
      memset(captured, -1, capturedSize * sizeof(int));
   }
}

/*
  The three following functions add one state to the automaton and
  return the number of the state.
*/

int QRegExpEngine::createState(QChar ch)
{
   return setupState(ch.unicode());
}

int QRegExpEngine::createState(const QRegExpCharClass &cc)
{
#ifndef QT_NO_REGEXP_CCLASS
   int n = cl.size();
   cl += QRegExpCharClass(cc);
   return setupState(CharClassBit | n);
#else
   Q_UNUSED(cc);
   return setupState(CharClassBit);
#endif
}

#ifndef QT_NO_REGEXP_BACKREF
int QRegExpEngine::createState(int bref)
{
   if (bref > nbrefs) {
      nbrefs = bref;
      if (nbrefs > MaxBackRefs) {
         error(RXERR_LIMIT);
         return 0;
      }
   }
   return setupState(BackRefBit | bref);
}
#endif

/*
  The two following functions add a transition between all pairs of
  states (i, j) where i is found in from, and j is found in to.

  Cat-transitions are distinguished from plus-transitions for
  capturing.
*/

void QRegExpEngine::addCatTransitions(const QVector<int> &from, const QVector<int> &to)
{
   for (int i = 0; i < from.size(); i++) {
      mergeInto(&s[from.at(i)].outs, to);
   }
}

#ifndef QT_NO_REGEXP_CAPTURE
void QRegExpEngine::addPlusTransitions(const QVector<int> &from, const QVector<int> &to, int atom)
{
   for (int i = 0; i < from.size(); i++) {
      QRegExpAutomatonState &st = s[from.at(i)];

      const QVector<int> oldOuts = st.outs;
      mergeInto(&st.outs, to);

      if (f.at(atom).capture != QRegExpAtom::NoCapture) {
         for (int j = 0; j < to.size(); j++) {
            // ### st.reenter.contains(to.at(j)) check looks suspicious

            if (! st.reenter.contains(to.at(j)) &&
                     ! std::binary_search(oldOuts.constBegin(), oldOuts.constEnd(), to.at(j))) {
               st.reenter.insert(to.at(j), atom);
            }
         }
      }
   }
}
#endif

#ifndef QT_NO_REGEXP_ANCHOR_ALT
/*
  Returns an anchor that means a OR b.
*/
int QRegExpEngine::anchorAlternation(int a, int b)
{
   if (((a & b) == a || (a & b) == b) && ((a | b) & Anchor_Alternation) == 0) {
      return a & b;
   }

   int n = aa.size();
#ifndef QT_NO_REGEXP_OPTIM
   if (n > 0 && aa.at(n - 1).a == a && aa.at(n - 1).b == b) {
      return Anchor_Alternation | (n - 1);
   }
#endif

   QRegExpAnchorAlternation element = {a, b};
   aa.append(element);
   return Anchor_Alternation | n;
}

/*
  Returns an anchor that means a AND b.
*/
int QRegExpEngine::anchorConcatenation(int a, int b)
{
   if (((a | b) & Anchor_Alternation) == 0) {
      return a | b;
   }

   if ((b & Anchor_Alternation) != 0) {
      qSwap(a, b);
   }

   int aprime = anchorConcatenation(aa.at(a ^ Anchor_Alternation).a, b);
   int bprime = anchorConcatenation(aa.at(a ^ Anchor_Alternation).b, b);

   return anchorAlternation(aprime, bprime);
}
#endif

/*
  Adds anchor a on a transition caracterised by its from state and
  its to state.
*/
void QRegExpEngine::addAnchors(int from, int to, int a)
{
   QRegExpAutomatonState &st = s[from];
   if (st.anchors.contains(to)) {
      a = anchorAlternation(st.anchors.value(to), a);
   }
   st.anchors.insert(to, a);
}

#ifndef QT_NO_REGEXP_OPTIM
/*
  This function chooses between the good-string and the bad-character
  heuristics. It computes two scores and chooses the heuristic with
  the highest score.

  Here are some common-sense constraints on the scores that should be
  respected if the formulas are ever modified: (1) If goodStr is
  empty, the good-string heuristic scores 0. (2) If the regular
  expression is trivial, the good-string heuristic should be used.
  (3) If the search is case insensitive, the good-string heuristic
  should be used, unless it scores 0. (Case insensitivity turns all
  entries of occ1 to 0.) (4) If (goodLateStart - goodEarlyStart) is
  big, the good-string heuristic should score less.
*/
void QRegExpEngine::heuristicallyChooseHeuristic()
{
   if (minl == 0) {
      useGoodStringHeuristic = false;
   } else if (trivial) {
      useGoodStringHeuristic = true;
   } else {
      /*
        Magic formula: The good string has to constitute a good
        proportion of the minimum-length string, and appear at a
        more-or-less known index.
      */
      int goodStringScore = (64 * goodStr.length() / minl) -
                            (goodLateStart - goodEarlyStart);
      /*
        Less magic formula: We pick some characters at random, and
        check whether they are good or bad.
      */
      int badCharScore = 0;
      int step = qMax(1, NumBadChars / 32);
      for (int i = 1; i < NumBadChars; i += step) {
         if (occ1.at(i) == NoOccurrence) {
            badCharScore += minl;
         } else {
            badCharScore += occ1.at(i);
         }
      }
      badCharScore /= minl;
      useGoodStringHeuristic = (goodStringScore > badCharScore);
   }
}
#endif

#if defined(QT_DEBUG)
void QRegExpEngine::dump() const
{
   int i, j;
   qDebug("Case %ssensitive engine", cs ? "" : "in");
   qDebug("  States");
   for (i = 0; i < s.size(); i++) {
      qDebug("  %d%s", i, i == InitialState ? " (initial)" : i == FinalState ? " (final)" : "");
#ifndef QT_NO_REGEXP_CAPTURE
      if (nf > 0) {
         qDebug("    in atom %d", s[i].atom);
      }
#endif
      int m = s[i].match;
      if ((m & CharClassBit) != 0) {
         qDebug("    match character class %d", m ^ CharClassBit);
#ifndef QT_NO_REGEXP_CCLASS
         cl[m ^ CharClassBit].dump();
#else
         qDebug("    negative character class");
#endif
      } else if ((m & BackRefBit) != 0) {
         qDebug("    match back-reference %d", m ^ BackRefBit);
      } else if (m >= 0x20 && m <= 0x7e) {
         qDebug("    match 0x%.4x (%c)", m, m);
      } else {
         qDebug("    match 0x%.4x", m);
      }
      for (j = 0; j < s[i].outs.size(); j++) {
         int next = s[i].outs[j];
         qDebug("    -> %d", next);
         if (s[i].reenter.contains(next)) {
            qDebug("       [reenter %d]", s[i].reenter[next]);
         }
         if (s[i].anchors.value(next) != 0) {
            qDebug("       [anchors 0x%.8x]", s[i].anchors[next]);
         }
      }
   }
#ifndef QT_NO_REGEXP_CAPTURE
   if (nf > 0) {
      qDebug("  Atom    Parent  Capture");
      for (i = 0; i < nf; i++) {
         if (f[i].capture == QRegExpAtom::NoCapture) {
            qDebug("  %6d  %6d     nil", i, f[i].parent);
         } else {
            int cap = f[i].capture;
            bool official = captureForOfficialCapture.contains(cap);
            qDebug("  %6d  %6d  %6d  %s", i, f[i].parent, f[i].capture,
                   official ? "official" : "");
         }
      }
   }
#endif
#ifndef QT_NO_REGEXP_ANCHOR_ALT
   for (i = 0; i < aa.size(); i++) {
      qDebug("  Anchor alternation 0x%.8x: 0x%.8x 0x%.9x", i, aa[i].a, aa[i].b);
   }
#endif
}
#endif

void QRegExpEngine::setup()
{
   ref.store(1);
#ifndef QT_NO_REGEXP_CAPTURE
   f.resize(32);
   nf = 0;
   cf = -1;
#endif
   officialncap = 0;
   ncap = 0;
#ifndef QT_NO_REGEXP_OPTIM
   caretAnchored = true;
   trivial = true;
#endif
   valid = false;
#ifndef QT_NO_REGEXP_BACKREF
   nbrefs = 0;
#endif
#ifndef QT_NO_REGEXP_OPTIM
   useGoodStringHeuristic = true;
   minl = 0;
   occ1.fill(0, NumBadChars);
#endif
}

int QRegExpEngine::setupState(int match)
{
#ifndef QT_NO_REGEXP_CAPTURE
   s += QRegExpAutomatonState(cf, match);
#else
   s += QRegExpAutomatonState(match);
#endif
   return s.size() - 1;
}

#ifndef QT_NO_REGEXP_CAPTURE
/*
  Functions startAtom() and finishAtom() should be called to delimit
  atoms. When a state is created, it is assigned to the current atom.
  The information is later used for capturing.
*/
int QRegExpEngine::startAtom(bool officialCapture)
{
   if ((nf & (nf + 1)) == 0 && nf + 1 >= f.size()) {
      f.resize((nf + 1) << 1);
   }
   f[nf].parent = cf;
   cf = nf++;
   f[cf].capture = officialCapture ? QRegExpAtom::OfficialCapture : QRegExpAtom::NoCapture;
   return cf;
}

void QRegExpEngine::finishAtom(int atom, bool needCapture)
{
   if (greedyQuantifiers && needCapture && f[atom].capture == QRegExpAtom::NoCapture) {
      f[atom].capture = QRegExpAtom::UnofficialCapture;
   }
   cf = f.at(atom).parent;
}
#endif

#ifndef QT_NO_REGEXP_LOOKAHEAD
/*
  Creates a lookahead anchor.
*/
int QRegExpEngine::addLookahead(QRegExpEngine *eng, bool negative)
{
   int n = ahead.size();
   if (n == MaxLookaheads) {
      error(RXERR_LIMIT);
      return 0;
   }
   ahead += new QRegExpLookahead(eng, negative);
   return Anchor_FirstLookahead << n;
}
#endif

#ifndef QT_NO_REGEXP_CAPTURE
/*
  We want the longest leftmost captures.
*/
static bool isBetterCapture(int ncap, const int *begin1, const int *end1, const int *begin2,
                            const int *end2)
{
   for (int i = 0; i < ncap; i++) {
      int delta = begin2[i] - begin1[i]; // it has to start early...
      if (delta == 0) {
         delta = end1[i] - end2[i];   // ...and end late
      }

      if (delta != 0) {
         return delta > 0;
      }
   }
   return false;
}
#endif

/*
  Returns true if anchor a matches at position pos + i in the input
  string, otherwise false.
*/
bool QRegExpMatchState::testAnchor(int i, int a, const int *capBegin)
{
   int j;

#ifndef QT_NO_REGEXP_ANCHOR_ALT
   if ((a & QRegExpEngine::Anchor_Alternation) != 0)
      return testAnchor(i, eng->aa.at(a ^ QRegExpEngine::Anchor_Alternation).a, capBegin)
             || testAnchor(i, eng->aa.at(a ^ QRegExpEngine::Anchor_Alternation).b, capBegin);
#endif

   if ((a & QRegExpEngine::Anchor_Caret) != 0) {
      if (pos + i != caretPos) {
         return false;
      }
   }
   if ((a & QRegExpEngine::Anchor_Dollar) != 0) {
      if (pos + i != len) {
         return false;
      }
   }
#ifndef QT_NO_REGEXP_ESCAPE
   if ((a & (QRegExpEngine::Anchor_Word | QRegExpEngine::Anchor_NonWord)) != 0) {
      bool before = false;
      bool after = false;
      if (pos + i != 0) {
         before = isWord(in[pos + i - 1]);
      }
      if (pos + i != len) {
         after = isWord(in[pos + i]);
      }
      if ((a & QRegExpEngine::Anchor_Word) != 0 && (before == after)) {
         return false;
      }
      if ((a & QRegExpEngine::Anchor_NonWord) != 0 && (before != after)) {
         return false;
      }
   }
#endif
#ifndef QT_NO_REGEXP_LOOKAHEAD
   if ((a & QRegExpEngine::Anchor_LookaheadMask) != 0) {
      const QVector<QRegExpLookahead *> &ahead = eng->ahead;
      for (j = 0; j < ahead.size(); j++) {
         if ((a & (QRegExpEngine::Anchor_FirstLookahead << j)) != 0) {
            QRegExpMatchState matchState;
            matchState.prepareForMatch(ahead[j]->eng);
            matchState.match(in + pos + i, len - pos - i, 0,
                             true, true, caretPos - pos - i);
            if ((matchState.captured[0] == 0) == ahead[j]->neg) {
               return false;
            }
         }
      }
   }
#endif
#ifndef QT_NO_REGEXP_CAPTURE
#ifndef QT_NO_REGEXP_BACKREF
   for (j = 0; j < eng->nbrefs; j++) {
      if ((a & (QRegExpEngine::Anchor_BackRef1Empty << j)) != 0) {
         int i = eng->captureForOfficialCapture.at(j);
         if (capBegin[i] != EmptyCapture) {
            return false;
         }
      }
   }
#endif
#endif
   return true;
}

#ifndef QT_NO_REGEXP_OPTIM

bool QRegExpEngine::goodStringMatch(QRegExpMatchState &matchState) const
{
   int k = matchState.pos + goodEarlyStart;

   while (true) {
      QString tmp(matchState.in, matchState.len);
      k = tmp.indexOf(goodStr, k, cs);

      if (k == -1)  {
         break;
      }

      int from = k - goodLateStart;
      int to   = k - goodEarlyStart;

      if (from > matchState.pos) {
         matchState.pos = from;
      }

      while (matchState.pos <= to) {
         if (matchState.matchHere()) {
            return true;
         }
         ++matchState.pos;
      }

      ++k;
   }

   return false;
}

bool QRegExpEngine::badCharMatch(QRegExpMatchState &matchState) const
{
   int slideHead = 0;
   int slideNext = 0;
   int i;
   int lastPos = matchState.len - minl;
   memset(matchState.slideTab, 0, matchState.slideTabSize * sizeof(int));

   /*
     Set up the slide table, used for the bad-character heuristic,
     using the table of first occurrence of each character.
   */
   for (i = 0; i < minl; i++) {
      int sk = occ1[BadChar(matchState.in[matchState.pos + i])];
      if (sk == NoOccurrence) {
         sk = i + 1;
      }
      if (sk > 0) {
         int k = i + 1 - sk;
         if (k < 0) {
            sk = i + 1;
            k = 0;
         }
         if (sk > matchState.slideTab[k]) {
            matchState.slideTab[k] = sk;
         }
      }
   }

   if (matchState.pos > lastPos) {
      return false;
   }

   for (;;) {
      if (++slideNext >= matchState.slideTabSize) {
         slideNext = 0;
      }
      if (matchState.slideTab[slideHead] > 0) {
         if (matchState.slideTab[slideHead] - 1 > matchState.slideTab[slideNext]) {
            matchState.slideTab[slideNext] = matchState.slideTab[slideHead] - 1;
         }
         matchState.slideTab[slideHead] = 0;
      } else {
         if (matchState.matchHere()) {
            return true;
         }
      }

      if (matchState.pos == lastPos) {
         break;
      }

      /*
        Update the slide table. This code has much in common with
        the initialization code.
      */
      int sk = occ1[BadChar(matchState.in[matchState.pos + minl])];
      if (sk == NoOccurrence) {
         matchState.slideTab[slideNext] = minl;
      } else if (sk > 0) {
         int k = slideNext + minl - sk;
         if (k >= matchState.slideTabSize) {
            k -= matchState.slideTabSize;
         }
         if (sk > matchState.slideTab[k]) {
            matchState.slideTab[k] = sk;
         }
      }
      slideHead = slideNext;
      ++matchState.pos;
   }
   return false;
}
#else
bool QRegExpEngine::bruteMatch(QRegExpMatchState &matchState) const
{
   while (matchState.pos <= matchState.len) {
      if (matchState.matchHere()) {
         return true;
      }
      ++matchState.pos;
   }
   return false;
}
#endif

/*
  Here's the core of the engine. It tries to do a match here and now.
*/
bool QRegExpMatchState::matchHere()
{
   int ncur = 1, nnext = 0;
   int i = 0, j, k, m;
   bool stop = false;

   matchLen = -1;
   oneTestMatchedLen = -1;
   curStack[0] = QRegExpEngine::InitialState;

   int ncap = eng->ncap;
#ifndef QT_NO_REGEXP_CAPTURE
   if (ncap > 0) {
      for (j = 0; j < ncap; j++) {
         curCapBegin[j] = EmptyCapture;
         curCapEnd[j] = EmptyCapture;
      }
   }
#endif

#ifndef QT_NO_REGEXP_BACKREF
   while ((ncur > 0 || !sleeping.isEmpty()) && i <= len - pos && !stop)
#else
   while (ncur > 0 && i <= len - pos && !stop)
#endif
   {
      int ch = (i < len - pos) ? in[pos + i].unicode() : 0;
      for (j = 0; j < ncur; j++) {
         int cur = curStack[j];
         const QRegExpAutomatonState &scur = eng->s.at(cur);
         const QVector<int> &outs = scur.outs;
         for (k = 0; k < outs.size(); k++) {
            int next = outs.at(k);
            const QRegExpAutomatonState &snext = eng->s.at(next);
            bool inside = true;
#if !defined(QT_NO_REGEXP_BACKREF) && !defined(QT_NO_REGEXP_CAPTURE)
            int needSomeSleep = 0;
#endif

            /*
              First, check if the anchors are anchored properly.
            */
            int a = scur.anchors.value(next);
            if (a != 0 && !testAnchor(i, a, curCapBegin + j * ncap)) {
               inside = false;
            }

            /*
              If indeed they are, check if the input character is
              correct for this transition.
            */
            if (inside) {
               m = snext.match;
               if ((m & (QRegExpEngine::CharClassBit | QRegExpEngine::BackRefBit)) == 0) {
                  if (eng->cs) {
                     inside = (m == ch);
                  } else {
                     inside = (QChar(m).toLower() == QChar(ch).toLower());
                  }
               } else if (next == QRegExpEngine::FinalState) {
                  matchLen = i;
                  stop = minimal;
                  inside = true;
               } else if ((m & QRegExpEngine::CharClassBit) != 0) {
#ifndef QT_NO_REGEXP_CCLASS
                  const QRegExpCharClass &cc = eng->cl.at(m ^ QRegExpEngine::CharClassBit);
                  if (eng->cs) {
                     inside = cc.in(ch);
                  } else if (cc.negative())
                     inside = cc.in(QChar(ch).toLower()) &&
                              cc.in(QChar(ch).toUpper());
                  else
                     inside = cc.in(QChar(ch).toLower()) ||
                              cc.in(QChar(ch).toUpper());
#endif
#if !defined(QT_NO_REGEXP_BACKREF) && !defined(QT_NO_REGEXP_CAPTURE)
               } else { /* ((m & QRegExpEngine::BackRefBit) != 0) */
                  int bref = m ^ QRegExpEngine::BackRefBit;
                  int ell = j * ncap + eng->captureForOfficialCapture.at(bref - 1);

                  inside = bref <= ncap && curCapBegin[ell] != EmptyCapture;
                  if (inside) {
                     if (eng->cs) {
                        inside = (in[pos + curCapBegin[ell]] == QChar(ch));
                     } else
                        inside = (in[pos + curCapBegin[ell]].toLower()
                                  == QChar(ch).toLower());
                  }

                  if (inside) {
                     int delta;
                     if (curCapEnd[ell] == EmptyCapture) {
                        delta = i - curCapBegin[ell];
                     } else {
                        delta = curCapEnd[ell] - curCapBegin[ell];
                     }

                     inside = (delta <= len - (pos + i));
                     if (inside && delta > 1) {
                        int n = 1;
                        if (eng->cs) {
                           while (n < delta) {
                              if (in[pos + curCapBegin[ell] + n]
                                    != in[pos + i + n]) {
                                 break;
                              }
                              ++n;
                           }
                        } else {
                           while (n < delta) {
                              QChar a = in[pos + curCapBegin[ell] + n];
                              QChar b = in[pos + i + n];
                              if (a.toLower() != b.toLower()) {
                                 break;
                              }
                              ++n;
                           }
                        }
                        inside = (n == delta);
                        if (inside) {
                           needSomeSleep = delta - 1;
                        }
                     }
                  }
#endif
               }
            }

            /*
              We must now update our data structures.
            */
            if (inside) {
#ifndef QT_NO_REGEXP_CAPTURE
               int *capBegin, *capEnd;
#endif
               /*
                 If the next state was not encountered yet, all
                 is fine.
               */
               if ((m = inNextStack[next]) == -1) {
                  m = nnext++;
                  nextStack[m] = next;
                  inNextStack[next] = m;
#ifndef QT_NO_REGEXP_CAPTURE
                  capBegin = nextCapBegin + m * ncap;
                  capEnd = nextCapEnd + m * ncap;

                  /*
                    Otherwise, we'll first maintain captures in
                    temporary arrays, and decide at the end whether
                    it's best to keep the previous capture zones or
                    the new ones.
                  */
               } else {
                  capBegin = tempCapBegin;
                  capEnd = tempCapEnd;
#endif
               }

#ifndef QT_NO_REGEXP_CAPTURE
               /*
                 Updating the capture zones is much of a task.
               */
               if (ncap > 0) {
                  memcpy(capBegin, curCapBegin + j * ncap, ncap * sizeof(int));
                  memcpy(capEnd, curCapEnd + j * ncap, ncap * sizeof(int));
                  int c = scur.atom, n = snext.atom;
                  int p = -1, q = -1;
                  int cap;

                  /*
                    Lemma 1. For any x in the range [0..nf), we
                    have f[x].parent < x.

                    Proof. By looking at startAtom(), it is
                    clear that cf < nf holds all the time, and
                    thus that f[nf].parent < nf.
                  */

                  /*
                    If we are reentering an atom, we empty all
                    capture zones inside it.
                  */
                  if ((q = scur.reenter.value(next)) != 0) {
                     QBitArray b(eng->nf, false);
                     b.setBit(q, true);
                     for (int ell = q + 1; ell < eng->nf; ell++) {
                        if (b.testBit(eng->f.at(ell).parent)) {
                           b.setBit(ell, true);
                           cap = eng->f.at(ell).capture;
                           if (cap >= 0) {
                              capBegin[cap] = EmptyCapture;
                              capEnd[cap] = EmptyCapture;
                           }
                        }
                     }
                     p = eng->f.at(q).parent;

                     /*
                       Otherwise, close the capture zones we are
                       leaving. We are leaving f[c].capture,
                       f[f[c].parent].capture,
                       f[f[f[c].parent].parent].capture, ...,
                       until f[x].capture, with x such that
                       f[x].parent is the youngest common ancestor
                       for c and n.

                       We go up along c's and n's ancestry until
                       we find x.
                     */
                  } else {
                     p = c;
                     q = n;
                     while (p != q) {
                        if (p > q) {
                           cap = eng->f.at(p).capture;
                           if (cap >= 0) {
                              if (capBegin[cap] == i) {
                                 capBegin[cap] = EmptyCapture;
                                 capEnd[cap] = EmptyCapture;
                              } else {
                                 capEnd[cap] = i;
                              }
                           }
                           p = eng->f.at(p).parent;
                        } else {
                           q = eng->f.at(q).parent;
                        }
                     }
                  }

                  /*
                    In any case, we now open the capture zones
                    we are entering. We work upwards from n
                    until we reach p (the parent of the atom we
                    reenter or the youngest common ancestor).
                  */
                  while (n > p) {
                     cap = eng->f.at(n).capture;
                     if (cap >= 0) {
                        capBegin[cap] = i;
                        capEnd[cap] = EmptyCapture;
                     }
                     n = eng->f.at(n).parent;
                  }
                  /*
                    If the next state was already in
                    nextStack, we must choose carefully which
                    capture zones we want to keep.
                  */
                  if (capBegin == tempCapBegin &&
                        isBetterCapture(ncap, capBegin, capEnd, nextCapBegin + m * ncap,
                                        nextCapEnd + m * ncap)) {
                     memcpy(nextCapBegin + m * ncap, capBegin, ncap * sizeof(int));
                     memcpy(nextCapEnd + m * ncap, capEnd, ncap * sizeof(int));
                  }
               }
#ifndef QT_NO_REGEXP_BACKREF
               /*
                 We are done with updating the capture zones.
                 It's now time to put the next state to sleep,
                 if it needs to, and to remove it from
                 nextStack.
               */
               if (needSomeSleep > 0) {
                  QVector<int> zzZ(2 + 2 * ncap);
                  zzZ[0] = i + needSomeSleep;
                  zzZ[1] = next;
                  if (ncap > 0) {
                     memcpy(zzZ.data() + 2, capBegin, ncap * sizeof(int));
                     memcpy(zzZ.data() + 2 + ncap, capEnd, ncap * sizeof(int));
                  }
                  inNextStack[nextStack[--nnext]] = -1;
                  sleeping.append(zzZ);
               }
#endif
#endif
            }
         }
      }
#ifndef QT_NO_REGEXP_CAPTURE
      /*
        If we reached the final state, hurray! Copy the captured
        zone.
      */
      if (ncap > 0 && (m = inNextStack[QRegExpEngine::FinalState]) != -1) {
         memcpy(capBegin, nextCapBegin + m * ncap, ncap * sizeof(int));
         memcpy(capEnd, nextCapEnd + m * ncap, ncap * sizeof(int));
      }
#ifndef QT_NO_REGEXP_BACKREF
      /*
        It's time to wake up the sleepers.
      */
      j = 0;
      while (j < sleeping.count()) {
         if (sleeping.at(j)[0] == i) {
            const QVector<int> &zzZ = sleeping.at(j);
            int next = zzZ[1];
            const int *capBegin = zzZ.data() + 2;
            const int *capEnd = zzZ.data() + 2 + ncap;
            bool copyOver = true;

            if ((m = inNextStack[next]) == -1) {
               m = nnext++;
               nextStack[m] = next;
               inNextStack[next] = m;
            } else {
               copyOver = isBetterCapture(ncap, nextCapBegin + m * ncap, nextCapEnd + m * ncap,
                                          capBegin, capEnd);
            }
            if (copyOver) {
               memcpy(nextCapBegin + m * ncap, capBegin, ncap * sizeof(int));
               memcpy(nextCapEnd + m * ncap, capEnd, ncap * sizeof(int));
            }

            sleeping.removeAt(j);
         } else {
            ++j;
         }
      }
#endif
#endif
      for (j = 0; j < nnext; j++) {
         inNextStack[nextStack[j]] = -1;
      }

      // avoid needless iteration that confuses oneTestMatchedLen
      if (nnext == 1 && nextStack[0] == QRegExpEngine::FinalState
#ifndef QT_NO_REGEXP_BACKREF
            && sleeping.isEmpty()
#endif
         ) {
         stop = true;
      }

      qSwap(curStack, nextStack);
#ifndef QT_NO_REGEXP_CAPTURE
      qSwap(curCapBegin, nextCapBegin);
      qSwap(curCapEnd, nextCapEnd);
#endif
      ncur = nnext;
      nnext = 0;
      ++i;
   }

#ifndef QT_NO_REGEXP_BACKREF
   /*
     If minimal matching is enabled, we might have some sleepers
     left.
   */
   if (!sleeping.isEmpty()) {
      sleeping.clear();
   }
#endif

   oneTestMatchedLen = i - 1;
   return (matchLen >= 0);
}

#ifndef QT_NO_REGEXP_CCLASS

QRegExpCharClass::QRegExpCharClass()
   : c(0), n(false)
{
#ifndef QT_NO_REGEXP_OPTIM
   occ1.fill(NoOccurrence, NumBadChars);
#endif
}

QRegExpCharClass &QRegExpCharClass::operator=(const QRegExpCharClass &cc)
{
   c = cc.c;
   r = cc.r;
   n = cc.n;
#ifndef QT_NO_REGEXP_OPTIM
   occ1 = cc.occ1;
#endif
   return *this;
}

void QRegExpCharClass::clear()
{
   c = 0;
   r.resize(0);
   n = false;
}

void QRegExpCharClass::setNegative(bool negative)
{
   n = negative;
#ifndef QT_NO_REGEXP_OPTIM
   occ1.fill(0, NumBadChars);
#endif
}

void QRegExpCharClass::addCategories(uint cats)
{
   static const uint all_cats = FLAG(QChar::Mark_NonSpacing) |
                                FLAG(QChar::Mark_SpacingCombining) |
                                FLAG(QChar::Mark_Enclosing) |
                                FLAG(QChar::Number_DecimalDigit) |
                                FLAG(QChar::Number_Letter) |
                                FLAG(QChar::Number_Other) |
                                FLAG(QChar::Separator_Space) |
                                FLAG(QChar::Separator_Line) |
                                FLAG(QChar::Separator_Paragraph) |
                                FLAG(QChar::Other_Control) |
                                FLAG(QChar::Other_Format) |
                                FLAG(QChar::Other_Surrogate) |
                                FLAG(QChar::Other_PrivateUse) |
                                FLAG(QChar::Other_NotAssigned) |
                                FLAG(QChar::Letter_Uppercase) |
                                FLAG(QChar::Letter_Lowercase) |
                                FLAG(QChar::Letter_Titlecase) |
                                FLAG(QChar::Letter_Modifier) |
                                FLAG(QChar::Letter_Other) |
                                FLAG(QChar::Punctuation_Connector) |
                                FLAG(QChar::Punctuation_Dash) |
                                FLAG(QChar::Punctuation_Open) |
                                FLAG(QChar::Punctuation_Close) |
                                FLAG(QChar::Punctuation_InitialQuote) |
                                FLAG(QChar::Punctuation_FinalQuote) |
                                FLAG(QChar::Punctuation_Other) |
                                FLAG(QChar::Symbol_Math) |
                                FLAG(QChar::Symbol_Currency) |
                                FLAG(QChar::Symbol_Modifier) |
                                FLAG(QChar::Symbol_Other);
    c |= (all_cats & cats);

#ifndef QT_NO_REGEXP_OPTIM
   occ1.fill(0, NumBadChars);
#endif
}

void QRegExpCharClass::addRange(ushort from, ushort to)
{
   if (from > to) {
      qSwap(from, to);
   }
   int m = r.size();
   r.resize(m + 1);
   r[m].from = from;
   r[m].len = to - from + 1;

#ifndef QT_NO_REGEXP_OPTIM
   int i;

   if (to - from < NumBadChars) {
      if (from % NumBadChars <= to % NumBadChars) {
         for (i = from % NumBadChars; i <= to % NumBadChars; i++) {
            occ1[i] = 0;
         }
      } else {
         for (i = 0; i <= to % NumBadChars; i++) {
            occ1[i] = 0;
         }
         for (i = from % NumBadChars; i < NumBadChars; i++) {
            occ1[i] = 0;
         }
      }
   } else {
      occ1.fill(0, NumBadChars);
   }
#endif
}

bool QRegExpCharClass::in(QChar ch) const
{
#ifndef QT_NO_REGEXP_OPTIM
   if (occ1.at(BadChar(ch)) == NoOccurrence) {
      return n;
   }
#endif
   if (c != 0 && (c & FLAG(ch.category())) != 0) {
      return !n;
   }

   const int uc = ch.unicode();
   int size = r.size();

   for (int i = 0; i < size; ++i) {
      const QRegExpCharClassRange &range = r.at(i);
      if (uint(uc - range.from) < uint(r.at(i).len)) {
         return !n;
      }
   }
   return n;
}

#if defined(QT_DEBUG)
void QRegExpCharClass::dump() const
{
   int i;
   qDebug("    %stive character class", n ? "nega" : "posi");
#ifndef QT_NO_REGEXP_CCLASS
   if (c != 0) {
      qDebug("      categories 0x%.8x", c);
   }
#endif
   for (i = 0; i < r.size(); i++) {
      qDebug("      0x%.4x through 0x%.4x", r[i].from, r[i].from + r[i].len - 1);
   }
}
#endif
#endif

QRegExpEngine::Box::Box(QRegExpEngine *engine)
   : eng(engine), skipanchors(0)
#ifndef QT_NO_REGEXP_OPTIM
   , earlyStart(0), lateStart(0), maxl(0)
#endif
{
#ifndef QT_NO_REGEXP_OPTIM
   occ1.fill(NoOccurrence, NumBadChars);
#endif
   minl = 0;
}

QRegExpEngine::Box &QRegExpEngine::Box::operator=(const Box &b)
{
   eng = b.eng;
   ls = b.ls;
   rs = b.rs;
   lanchors = b.lanchors;
   ranchors = b.ranchors;
   skipanchors = b.skipanchors;
#ifndef QT_NO_REGEXP_OPTIM
   earlyStart = b.earlyStart;
   lateStart = b.lateStart;
   str = b.str;
   leftStr = b.leftStr;
   rightStr = b.rightStr;
   maxl = b.maxl;
   occ1 = b.occ1;
#endif
   minl = b.minl;
   return *this;
}

void QRegExpEngine::Box::set(QChar ch)
{
   ls.resize(1);
   ls[0] = eng->createState(ch);
   rs = ls;
#ifndef QT_NO_REGEXP_OPTIM
   str = ch;
   leftStr = ch;
   rightStr = ch;
   maxl = 1;
   occ1[BadChar(ch)] = 0;
#endif
   minl = 1;
}

void QRegExpEngine::Box::set(const QRegExpCharClass &cc)
{
   ls.resize(1);
   ls[0] = eng->createState(cc);
   rs = ls;
#ifndef QT_NO_REGEXP_OPTIM
   maxl = 1;
   occ1 = cc.firstOccurrence();
#endif
   minl = 1;
}

#ifndef QT_NO_REGEXP_BACKREF
void QRegExpEngine::Box::set(int bref)
{
   ls.resize(1);
   ls[0] = eng->createState(bref);
   rs = ls;
   if (bref >= 1 && bref <= MaxBackRefs) {
      skipanchors = Anchor_BackRef0Empty << bref;
   }
#ifndef QT_NO_REGEXP_OPTIM
   maxl = InftyLen;
#endif
   minl = 0;
}
#endif

void QRegExpEngine::Box::cat(const Box &b)
{
   eng->addCatTransitions(rs, b.ls);
   addAnchorsToEngine(b);
   if (minl == 0) {
      lanchors.unite(b.lanchors);
      if (skipanchors != 0) {
         for (int i = 0; i < b.ls.size(); i++) {
            int a = eng->anchorConcatenation(lanchors.value(b.ls.at(i), 0), skipanchors);
            lanchors.insert(b.ls.at(i), a);
         }
      }
      mergeInto(&ls, b.ls);
   }
   if (b.minl == 0) {
      ranchors.unite(b.ranchors);
      if (b.skipanchors != 0) {
         for (int i = 0; i < rs.size(); i++) {
            int a = eng->anchorConcatenation(ranchors.value(rs.at(i), 0), b.skipanchors);
            ranchors.insert(rs.at(i), a);
         }
      }
      mergeInto(&rs, b.rs);
   } else {
      ranchors = b.ranchors;
      rs = b.rs;
   }

#ifndef QT_NO_REGEXP_OPTIM
   if (maxl != InftyLen) {
      if (rightStr.length() + b.leftStr.length() >
            qMax(str.length(), b.str.length())) {
         earlyStart = minl - rightStr.length();
         lateStart = maxl - rightStr.length();
         str = rightStr + b.leftStr;
      } else if (b.str.length() > str.length()) {
         earlyStart = minl + b.earlyStart;
         lateStart = maxl + b.lateStart;
         str = b.str;
      }
   }

   if (leftStr.length() == maxl) {
      leftStr += b.leftStr;
   }

   if (b.rightStr.length() == b.maxl) {
      rightStr += b.rightStr;
   } else {
      rightStr = b.rightStr;
   }

   if (maxl == InftyLen || b.maxl == InftyLen) {
      maxl = InftyLen;
   } else {
      maxl += b.maxl;
   }

   for (int i = 0; i < NumBadChars; i++) {
      if (b.occ1.at(i) != NoOccurrence && minl + b.occ1.at(i) < occ1.at(i)) {
         occ1[i] = minl + b.occ1.at(i);
      }
   }
#endif

   minl += b.minl;
   if (minl == 0) {
      skipanchors = eng->anchorConcatenation(skipanchors, b.skipanchors);
   } else {
      skipanchors = 0;
   }
}

void QRegExpEngine::Box::orx(const Box &b)
{
   mergeInto(&ls, b.ls);
   lanchors.unite(b.lanchors);
   mergeInto(&rs, b.rs);
   ranchors.unite(b.ranchors);

   if (b.minl == 0) {
      if (minl == 0) {
         skipanchors = eng->anchorAlternation(skipanchors, b.skipanchors);
      } else {
         skipanchors = b.skipanchors;
      }
   }

#ifndef QT_NO_REGEXP_OPTIM
   for (int i = 0; i < NumBadChars; i++) {
      if (occ1.at(i) > b.occ1.at(i)) {
         occ1[i] = b.occ1.at(i);
      }
   }
   earlyStart = 0;
   lateStart = 0;
   str = QString();
   leftStr = QString();
   rightStr = QString();
   if (b.maxl > maxl) {
      maxl = b.maxl;
   }
#endif
   if (b.minl < minl) {
      minl = b.minl;
   }
}

void QRegExpEngine::Box::plus(int atom)
{
#ifndef QT_NO_REGEXP_CAPTURE
   eng->addPlusTransitions(rs, ls, atom);
#else
   Q_UNUSED(atom);
   eng->addCatTransitions(rs, ls);
#endif
   addAnchorsToEngine(*this);
#ifndef QT_NO_REGEXP_OPTIM
   maxl = InftyLen;
#endif
}

void QRegExpEngine::Box::opt()
{
#ifndef QT_NO_REGEXP_OPTIM
   earlyStart = 0;
   lateStart = 0;
   str = QString();
   leftStr = QString();
   rightStr = QString();
#endif
   skipanchors = 0;
   minl = 0;
}

void QRegExpEngine::Box::catAnchor(int a)
{
   if (a != 0) {
      for (int i = 0; i < rs.size(); i++) {
         a = eng->anchorConcatenation(ranchors.value(rs.at(i), 0), a);
         ranchors.insert(rs.at(i), a);
      }
      if (minl == 0) {
         skipanchors = eng->anchorConcatenation(skipanchors, a);
      }
   }
}

#ifndef QT_NO_REGEXP_OPTIM
void QRegExpEngine::Box::setupHeuristics()
{
   eng->goodEarlyStart = earlyStart;
   eng->goodLateStart = lateStart;
   eng->goodStr = eng->cs ? str : str.toLower();

   eng->minl = minl;
   if (eng->cs) {
      /*
        A regular expression such as 112|1 has occ1['2'] = 2 and minl =
        1 at this point. An entry of occ1 has to be at most minl or
        infinity for the rest of the algorithm to go well.

        We waited until here before normalizing these cases (instead of
        doing it in Box::orx()) because sometimes things improve by
        themselves. Consider for example (112|1)34.
      */
      for (int i = 0; i < NumBadChars; i++) {
         if (occ1.at(i) != NoOccurrence && occ1.at(i) >= minl) {
            occ1[i] = minl;
         }
      }
      eng->occ1 = occ1;
   } else {
      eng->occ1.fill(0, NumBadChars);
   }

   eng->heuristicallyChooseHeuristic();
}
#endif

#if defined(QT_DEBUG)
void QRegExpEngine::Box::dump() const
{
   int i;
   qDebug("Box of at least %d character%s", minl, minl == 1 ? "" : "s");
   qDebug("  Left states:");
   for (i = 0; i < ls.size(); i++) {
      if (lanchors.value(ls[i], 0) == 0) {
         qDebug("    %d", ls[i]);
      } else {
         qDebug("    %d [anchors 0x%.8x]", ls[i], lanchors[ls[i]]);
      }
   }
   qDebug("  Right states:");
   for (i = 0; i < rs.size(); i++) {
      if (ranchors.value(rs[i], 0) == 0) {
         qDebug("    %d", rs[i]);
      } else {
         qDebug("    %d [anchors 0x%.8x]", rs[i], ranchors[rs[i]]);
      }
   }
   qDebug("  Skip anchors: 0x%.8x", skipanchors);
}
#endif

void QRegExpEngine::Box::addAnchorsToEngine(const Box &to) const
{
   for (int i = 0; i < to.ls.size(); i++) {
      for (int j = 0; j < rs.size(); j++) {
         int a = eng->anchorConcatenation(ranchors.value(rs.at(j), 0),
                                          to.lanchors.value(to.ls.at(i), 0));
         eng->addAnchors(rs[j], to.ls[i], a);
      }
   }
}

#ifndef QT_NO_REGEXP_CCLASS
// fast lookup hash for xml schema extensions
// sorted by name for b-search

struct CategoriesRangeMapEntry {
    const char name[40];
    uint first, second;
};

static const CategoriesRangeMapEntry categoriesRangeMap[] = {
    { "AegeanNumbers",                        0x10100, 0x1013F },
    { "AlphabeticPresentationForms",          0xFB00, 0xFB4F },
    { "AncientGreekMusicalNotation",          0x1D200, 0x1D24F },
    { "AncientGreekNumbers",                  0x10140, 0x1018F },
    { "Arabic",                               0x0600, 0x06FF },
    { "ArabicPresentationForms-A",            0xFB50, 0xFDFF },
    { "ArabicPresentationForms-B",            0xFE70, 0xFEFF },
    { "ArabicSupplement",                     0x0750, 0x077F },
    { "Armenian",                             0x0530, 0x058F },
    { "Arrows",                               0x2190, 0x21FF },
    { "BasicLatin",                           0x0000, 0x007F },
    { "Bengali",                              0x0980, 0x09FF },
    { "BlockElements",                        0x2580, 0x259F },
    { "Bopomofo",                             0x3100, 0x312F },
    { "BopomofoExtended",                     0x31A0, 0x31BF },
    { "BoxDrawing",                           0x2500, 0x257F },
    { "BraillePatterns",                      0x2800, 0x28FF },
    { "Buginese",                             0x1A00, 0x1A1F },
    { "Buhid",                                0x1740, 0x175F },
    { "ByzantineMusicalSymbols",              0x1D000, 0x1D0FF },
    { "CJKCompatibility",                     0x3300, 0x33FF },
    { "CJKCompatibilityForms",                0xFE30, 0xFE4F },
    { "CJKCompatibilityIdeographs",           0xF900, 0xFAFF },
    { "CJKCompatibilityIdeographsSupplement", 0x2F800, 0x2FA1F },
    { "CJKRadicalsSupplement",                0x2E80, 0x2EFF },
    { "CJKStrokes",                           0x31C0, 0x31EF },
    { "CJKSymbolsandPunctuation",             0x3000, 0x303F },
    { "CJKUnifiedIdeographs",                 0x4E00, 0x9FFF },
    { "CJKUnifiedIdeographsExtensionA",       0x3400, 0x4DB5 },
    { "CJKUnifiedIdeographsExtensionB",       0x20000, 0x2A6DF },
    { "Cherokee",                             0x13A0, 0x13FF },
    { "CombiningDiacriticalMarks",            0x0300, 0x036F },
    { "CombiningDiacriticalMarksSupplement",  0x1DC0, 0x1DFF },
    { "CombiningHalfMarks",                   0xFE20, 0xFE2F },
    { "CombiningMarksforSymbols",             0x20D0, 0x20FF },
    { "ControlPictures",                      0x2400, 0x243F },
    { "Coptic",                               0x2C80, 0x2CFF },
    { "CurrencySymbols",                      0x20A0, 0x20CF },
    { "CypriotSyllabary",                     0x10800, 0x1083F },
    { "Cyrillic",                             0x0400, 0x04FF },
    { "CyrillicSupplement",                   0x0500, 0x052F },
    { "Deseret",                              0x10400, 0x1044F },
    { "Devanagari",                           0x0900, 0x097F },
    { "Dingbats",                             0x2700, 0x27BF },
    { "EnclosedAlphanumerics",                0x2460, 0x24FF },
    { "EnclosedCJKLettersandMonths",          0x3200, 0x32FF },
    { "Ethiopic",                             0x1200, 0x137F },
    { "EthiopicExtended",                     0x2D80, 0x2DDF },
    { "EthiopicSupplement",                   0x1380, 0x139F },
    { "GeneralPunctuation",                   0x2000, 0x206F },
    { "GeometricShapes",                      0x25A0, 0x25FF },
    { "Georgian",                             0x10A0, 0x10FF },
    { "GeorgianSupplement",                   0x2D00, 0x2D2F },
    { "Glagolitic",                           0x2C00, 0x2C5F },
    { "Gothic",                               0x10330, 0x1034F },
    { "Greek",                                0x0370, 0x03FF },
    { "GreekExtended",                        0x1F00, 0x1FFF },
    { "Gujarati",                             0x0A80, 0x0AFF },
    { "Gurmukhi",                             0x0A00, 0x0A7F },
    { "HalfwidthandFullwidthForms",           0xFF00, 0xFFEF },
    { "HangulCompatibilityJamo",              0x3130, 0x318F },
    { "HangulJamo",                           0x1100, 0x11FF },
    { "HangulSyllables",                      0xAC00, 0xD7A3 },
    { "Hanunoo",                              0x1720, 0x173F },
    { "Hebrew",                               0x0590, 0x05FF },
    { "Hiragana",                             0x3040, 0x309F },
    { "IPAExtensions",                        0x0250, 0x02AF },
    { "IdeographicDescriptionCharacters",     0x2FF0, 0x2FFF },
    { "Kanbun",                               0x3190, 0x319F },
    { "KangxiRadicals",                       0x2F00, 0x2FDF },
    { "Kannada",                              0x0C80, 0x0CFF },
    { "Katakana",                             0x30A0, 0x30FF },
    { "KatakanaPhoneticExtensions",           0x31F0, 0x31FF },
    { "Kharoshthi",                           0x10A00, 0x10A5F },
    { "Khmer",                                0x1780, 0x17FF },
    { "KhmerSymbols",                         0x19E0, 0x19FF },
    { "Lao",                                  0x0E80, 0x0EFF },
    { "Latin-1Supplement",                    0x0080, 0x00FF },
    { "LatinExtended-A",                      0x0100, 0x017F },
    { "LatinExtended-B",                      0x0180, 0x024F },
    { "LatinExtendedAdditional",              0x1E00, 0x1EFF },
    { "LetterlikeSymbols",                    0x2100, 0x214F },
    { "Limbu",                                0x1900, 0x194F },
    { "LinearBIdeograms",                     0x10080, 0x100FF },
    { "LinearBSyllabary",                     0x10000, 0x1007F },
    { "Malayalam",                            0x0D00, 0x0D7F },
    { "MathematicalAlphanumericSymbols",      0x1D400, 0x1D7FF },
    { "MathematicalOperators",                0x2200, 0x22FF },
    { "MiscellaneousMathematicalSymbols-A",   0x27C0, 0x27EF },
    { "MiscellaneousMathematicalSymbols-B",   0x2980, 0x29FF },
    { "MiscellaneousSymbols",                 0x2600, 0x26FF },
    { "MiscellaneousSymbolsandArrows",        0x2B00, 0x2BFF },
    { "MiscellaneousTechnical",               0x2300, 0x23FF },
    { "ModifierToneLetters",                  0xA700, 0xA71F },
    { "Mongolian",                            0x1800, 0x18AF },
    { "MusicalSymbols",                       0x1D100, 0x1D1FF },
    { "Myanmar",                              0x1000, 0x109F },
    { "NewTaiLue",                            0x1980, 0x19DF },
    { "NumberForms",                          0x2150, 0x218F },
    { "Ogham",                                0x1680, 0x169F },
    { "OldItalic",                            0x10300, 0x1032F },
    { "OldPersian",                           0x103A0, 0x103DF },
    { "OpticalCharacterRecognition",          0x2440, 0x245F },
    { "Oriya",                                0x0B00, 0x0B7F },
    { "Osmanya",                              0x10480, 0x104AF },
    { "PhoneticExtensions",                   0x1D00, 0x1D7F },
    { "PhoneticExtensionsSupplement",         0x1D80, 0x1DBF },
    { "PrivateUse",                           0xE000, 0xF8FF },
    { "Runic",                                0x16A0, 0x16FF },
    { "Shavian",                              0x10450, 0x1047F },
    { "Sinhala",                              0x0D80, 0x0DFF },
    { "SmallFormVariants",                    0xFE50, 0xFE6F },
    { "SpacingModifierLetters",               0x02B0, 0x02FF },
    { "Specials",                             0xFFF0, 0xFFFF },
    { "SuperscriptsandSubscripts",            0x2070, 0x209F },
    { "SupplementalArrows-A",                 0x27F0, 0x27FF },
    { "SupplementalArrows-B",                 0x2900, 0x297F },
    { "SupplementalMathematicalOperators",    0x2A00, 0x2AFF },
    { "SupplementalPunctuation",              0x2E00, 0x2E7F },
    { "SupplementaryPrivateUseArea-A",        0xF0000, 0xFFFFF },
    { "SupplementaryPrivateUseArea-B",        0x100000, 0x10FFFF },
    { "SylotiNagri",                          0xA800, 0xA82F },
    { "Syriac",                               0x0700, 0x074F },
    { "Tagalog",                              0x1700, 0x171F },
    { "Tagbanwa",                             0x1760, 0x177F },
    { "Tags",                                 0xE0000, 0xE007F },
    { "TaiLe",                                0x1950, 0x197F },
    { "TaiXuanJingSymbols",                   0x1D300, 0x1D35F },
    { "Tamil",                                0x0B80, 0x0BFF },
    { "Telugu",                               0x0C00, 0x0C7F },
    { "Thaana",                               0x0780, 0x07BF },
    { "Thai",                                 0x0E00, 0x0E7F },
    { "Tibetan",                              0x0F00, 0x0FFF },
    { "Tifinagh",                             0x2D30, 0x2D7F },
    { "Ugaritic",                             0x10380, 0x1039F },
    { "UnifiedCanadianAboriginalSyllabics",   0x1400, 0x167F },
    { "VariationSelectors",                   0xFE00, 0xFE0F },
    { "VariationSelectorsSupplement",         0xE0100, 0xE01EF },
    { "VerticalForms",                        0xFE10, 0xFE1F },
    { "YiRadicals",                           0xA490, 0xA4CF },
    { "YiSyllables",                          0xA000, 0xA48F },
    { "YijingHexagramSymbols",                0x4DC0, 0x4DFF }
};

inline bool operator<(const CategoriesRangeMapEntry &entry1, const CategoriesRangeMapEntry &entry2)
{
   return qstrcmp(entry1.name, entry2.name) < 0;
}

inline bool operator<(const char *name, const CategoriesRangeMapEntry &entry)
{
   return qstrcmp(name, entry.name) < 0;
}

inline bool operator<(const CategoriesRangeMapEntry &entry, const char *name)
{
   return qstrcmp(entry.name, name) < 0;
}

#endif // QT_NO_REGEXP_CCLASS

int QRegExpEngine::getChar()
{
   return (yyPos == yyLen) ? EOS : yyIn[yyPos++].unicode();
}

int QRegExpEngine::getEscape()
{
#ifndef QT_NO_REGEXP_ESCAPE
   const char tab[] = "afnrtv"; // no b, as \b means word boundary
   const char backTab[] = "\a\f\n\r\t\v";
   ushort low;
   int i;
#endif

   ushort val;
   int prevCh = yyCh;

   if (prevCh == EOS) {
      error(RXERR_END);
      return Tok_Char | '\\';
   }

   yyCh = getChar();

#ifndef QT_NO_REGEXP_ESCAPE
   if ((prevCh & ~0xff) == 0) {
      const char *p = strchr(tab, prevCh);
      if (p != 0) {
         return Tok_Char | backTab[p - tab];
      }
   }
#endif

   switch (prevCh) {
#ifndef QT_NO_REGEXP_ESCAPE
      case '0':
         val = 0;
         for (i = 0; i < 3; i++) {
            if (yyCh >= '0' && yyCh <= '7') {
               val = (val << 3) | (yyCh - '0');
            } else {
               break;
            }
            yyCh = getChar();
         }

         if ((val & ~0377) != 0) {
            error(RXERR_OCTAL);
         }
         return Tok_Char | val;
#endif

#ifndef QT_NO_REGEXP_ESCAPE
      case 'B':
         return Tok_NonWord;
#endif

#ifndef QT_NO_REGEXP_CCLASS
      case 'D':
         // see QChar::isDigit()
         yyCharClass->addCategories(uint(-1) ^ FLAG(QChar::Number_DecimalDigit));
         return Tok_CharClass;

      case 'S':
         // see QChar::isSpace()
        yyCharClass->addCategories(uint(-1) ^ (FLAG(QChar::Separator_Space) |
                                               FLAG(QChar::Separator_Line) |
                                               FLAG(QChar::Separator_Paragraph) |
                                               FLAG(QChar::Other_Control)));
        yyCharClass->addRange(0x0000, 0x0008);
        yyCharClass->addRange(0x000e, 0x001f);
        yyCharClass->addRange(0x007f, 0x0084);
        yyCharClass->addRange(0x0086, 0x009f);

        return Tok_CharClass;

      case 'W':
         // see QChar::isLetterOrNumber() and QChar::isMark()
        yyCharClass->addCategories(uint(-1) ^ (FLAG(QChar::Mark_NonSpacing) |
                                               FLAG(QChar::Mark_SpacingCombining) |
                                               FLAG(QChar::Mark_Enclosing) |
                                               FLAG(QChar::Number_DecimalDigit) |
                                               FLAG(QChar::Number_Letter) |
                                               FLAG(QChar::Number_Other) |
                                               FLAG(QChar::Letter_Uppercase) |
                                               FLAG(QChar::Letter_Lowercase) |
                                               FLAG(QChar::Letter_Titlecase) |
                                               FLAG(QChar::Letter_Modifier) |
                                               FLAG(QChar::Letter_Other) |
                                               FLAG(QChar::Punctuation_Connector)));
        yyCharClass->addRange(0x203f, 0x2040);
        yyCharClass->addSingleton(0x2040);
        yyCharClass->addSingleton(0x2054);
        yyCharClass->addSingleton(0x30fb);
        yyCharClass->addRange(0xfe33, 0xfe34);
        yyCharClass->addRange(0xfe4d, 0xfe4f);
        yyCharClass->addSingleton(0xff3f);
        yyCharClass->addSingleton(0xff65);
        return Tok_CharClass;
#endif

#ifndef QT_NO_REGEXP_ESCAPE
      case 'b':
         return Tok_Word;
#endif

#ifndef QT_NO_REGEXP_CCLASS
      case 'd':
         // see QChar::isDigit()
         yyCharClass->addCategories(FLAG(QChar::Number_DecimalDigit));
         return Tok_CharClass;

      case 's':
         // see QChar::isSpace()
         yyCharClass->addCategories(FLAG(QChar::Separator_Space) |
                                    FLAG(QChar::Separator_Line) |
                                    FLAG(QChar::Separator_Paragraph));

         yyCharClass->addRange(0x0009, 0x000d);
         yyCharClass->addSingleton(0x0085);
         return Tok_CharClass;

      case 'w':
         // see QChar::isLetterOrNumber() and QChar::isMark()
         yyCharClass->addCategories(FLAG(QChar::Mark_NonSpacing) |
                                    FLAG(QChar::Mark_SpacingCombining) |
                                    FLAG(QChar::Mark_Enclosing) |
                                    FLAG(QChar::Number_DecimalDigit) |
                                    FLAG(QChar::Number_Letter) |
                                    FLAG(QChar::Number_Other) |
                                    FLAG(QChar::Letter_Uppercase) |
                                    FLAG(QChar::Letter_Lowercase) |
                                    FLAG(QChar::Letter_Titlecase) |
                                    FLAG(QChar::Letter_Modifier) |
                                    FLAG(QChar::Letter_Other));
         yyCharClass->addSingleton(0x005f); // '_'

         return Tok_CharClass;

      case 'I':
         if (xmlSchemaExtensions) {
            yyCharClass->setNegative(! yyCharClass->negative());
            // fall through
         } else {
            break;
         }

      case 'i':
        if (xmlSchemaExtensions) {
            yyCharClass->addCategories(FLAG(QChar::Mark_NonSpacing) |
                                       FLAG(QChar::Mark_SpacingCombining) |
                                       FLAG(QChar::Mark_Enclosing) |
                                       FLAG(QChar::Number_DecimalDigit) |
                                       FLAG(QChar::Number_Letter) |
                                       FLAG(QChar::Number_Other) |
                                       FLAG(QChar::Letter_Uppercase) |
                                       FLAG(QChar::Letter_Lowercase) |
                                       FLAG(QChar::Letter_Titlecase) |
                                       FLAG(QChar::Letter_Modifier) |
                                       FLAG(QChar::Letter_Other));
            yyCharClass->addSingleton(0x003a); // ':'
            yyCharClass->addSingleton(0x005f); // '_'
            yyCharClass->addRange(0x0041, 0x005a); // [A-Z]
            yyCharClass->addRange(0x0061, 0x007a); // [a-z]
            yyCharClass->addRange(0xc0, 0xd6);
            yyCharClass->addRange(0xd8, 0xf6);
            yyCharClass->addRange(0xf8, 0x2ff);
            yyCharClass->addRange(0x370, 0x37d);
            yyCharClass->addRange(0x37f, 0x1fff);
            yyCharClass->addRange(0x200c, 0x200d);
            yyCharClass->addRange(0x2070, 0x218f);
            yyCharClass->addRange(0x2c00, 0x2fef);
            yyCharClass->addRange(0x3001, 0xd7ff);
            yyCharClass->addRange(0xf900, 0xfdcf);
            yyCharClass->addRange(0xfdf0, 0xfffd);
            yyCharClass->addRange((ushort)0x10000, (ushort)0xeffff);
            return Tok_CharClass;
         } else {
             break;
         }

      case 'C':
         if (xmlSchemaExtensions) {
            yyCharClass->setNegative(!yyCharClass->negative());
            // fall through
         } else {
            break;
         }

      case 'c':
         if (xmlSchemaExtensions) {
            yyCharClass->addCategories(FLAG(QChar::Mark_NonSpacing) |
                                       FLAG(QChar::Mark_SpacingCombining) |
                                       FLAG(QChar::Mark_Enclosing) |
                                       FLAG(QChar::Number_DecimalDigit) |
                                       FLAG(QChar::Number_Letter) |
                                       FLAG(QChar::Number_Other) |
                                       FLAG(QChar::Letter_Uppercase) |
                                       FLAG(QChar::Letter_Lowercase) |
                                       FLAG(QChar::Letter_Titlecase) |
                                       FLAG(QChar::Letter_Modifier) |
                                       FLAG(QChar::Letter_Other));
            yyCharClass->addSingleton(0x002d); // '-'
            yyCharClass->addSingleton(0x002e); // '.'
            yyCharClass->addSingleton(0x003a); // ':'
            yyCharClass->addSingleton(0x005f); // '_'
            yyCharClass->addSingleton(0xb7);
            yyCharClass->addRange(0x0030, 0x0039); // [0-9]
            yyCharClass->addRange(0x0041, 0x005a); // [A-Z]
            yyCharClass->addRange(0x0061, 0x007a); // [a-z]
            yyCharClass->addRange(0xc0, 0xd6);
            yyCharClass->addRange(0xd8, 0xf6);
            yyCharClass->addRange(0xf8, 0x2ff);
            yyCharClass->addRange(0x370, 0x37d);
            yyCharClass->addRange(0x37f, 0x1fff);
            yyCharClass->addRange(0x200c, 0x200d);
            yyCharClass->addRange(0x2070, 0x218f);
            yyCharClass->addRange(0x2c00, 0x2fef);
            yyCharClass->addRange(0x3001, 0xd7ff);
            yyCharClass->addRange(0xf900, 0xfdcf);
            yyCharClass->addRange(0xfdf0, 0xfffd);
            yyCharClass->addRange((ushort)0x10000, (ushort)0xeffff);
            yyCharClass->addRange(0x0300, 0x036f);
            yyCharClass->addRange(0x203f, 0x2040);
            return Tok_CharClass;
        } else {
            break;
        }

      case 'P':
         if (xmlSchemaExtensions) {
            yyCharClass->setNegative(! yyCharClass->negative());
            // fall through
         } else {
            break;
        }

      case 'p':
         if (xmlSchemaExtensions) {
            if (yyCh != '{') {
               error(RXERR_CHARCLASS);
               return Tok_CharClass;
            }

            QByteArray category;
            yyCh = getChar();

            while (yyCh != '}') {
               if (yyCh == EOS) {
                  error(RXERR_END);
                  return Tok_CharClass;
               }
               category.append(yyCh);
               yyCh = getChar();
            }

            yyCh = getChar();                // skip closing '}'
            int catlen = category.length();

            if (catlen == 1 || catlen == 2) {
                switch (category.at(0)) {
                case 'M':
                    if (catlen == 1) {
                        yyCharClass->addCategories(FLAG(QChar::Mark_NonSpacing) |
                                                   FLAG(QChar::Mark_SpacingCombining) |
                                                   FLAG(QChar::Mark_Enclosing));
                    } else {
                        switch (category.at(1)) {
                        case 'n': yyCharClass->addCategories(FLAG(QChar::Mark_NonSpacing)); break; // Mn
                        case 'c': yyCharClass->addCategories(FLAG(QChar::Mark_SpacingCombining)); break; // Mc
                        case 'e': yyCharClass->addCategories(FLAG(QChar::Mark_Enclosing)); break; // Me
                        default: error(RXERR_CATEGORY); break;
                        }
                    }
                    break;

                case 'N':
                    if (catlen == 1) {
                        yyCharClass->addCategories(FLAG(QChar::Number_DecimalDigit) |
                                                   FLAG(QChar::Number_Letter) |
                                                   FLAG(QChar::Number_Other));
                    } else {
                        switch (category.at(1)) {
                        case 'd': yyCharClass->addCategories(FLAG(QChar::Number_DecimalDigit)); break; // Nd
                        case 'l': yyCharClass->addCategories(FLAG(QChar::Number_Letter)); break; // Hl
                        case 'o': yyCharClass->addCategories(FLAG(QChar::Number_Other)); break; // No
                        default: error(RXERR_CATEGORY); break;
                        }
                    }
                    break;

                case 'Z':
                    if (catlen == 1) {
                        yyCharClass->addCategories(FLAG(QChar::Separator_Space) |
                                                   FLAG(QChar::Separator_Line) |
                                                   FLAG(QChar::Separator_Paragraph));
                    } else {
                        switch (category.at(1)) {
                        case 's': yyCharClass->addCategories(FLAG(QChar::Separator_Space)); break; // Zs
                        case 'l': yyCharClass->addCategories(FLAG(QChar::Separator_Line)); break; // Zl
                        case 'p': yyCharClass->addCategories(FLAG(QChar::Separator_Paragraph)); break; // Zp
                        default: error(RXERR_CATEGORY); break;
                        }
                    }
                    break;

                case 'C':
                    if (catlen == 1) {
                        yyCharClass->addCategories(FLAG(QChar::Other_Control) |
                                                   FLAG(QChar::Other_Format) |
                                                   FLAG(QChar::Other_Surrogate) |
                                                   FLAG(QChar::Other_PrivateUse) |
                                                   FLAG(QChar::Other_NotAssigned));
                    } else {
                        switch (category.at(1)) {
                        case 'c': yyCharClass->addCategories(FLAG(QChar::Other_Control)); break; // Cc
                        case 'f': yyCharClass->addCategories(FLAG(QChar::Other_Format)); break; // Cf
                        case 's': yyCharClass->addCategories(FLAG(QChar::Other_Surrogate)); break; // Cs
                        case 'o': yyCharClass->addCategories(FLAG(QChar::Other_PrivateUse)); break; // Co
                        case 'n': yyCharClass->addCategories(FLAG(QChar::Other_NotAssigned)); break; // Cn
                        default: error(RXERR_CATEGORY); break;
                        }
                    }
                    break;

                case 'L':
                    if (catlen == 1) {
                        yyCharClass->addCategories(FLAG(QChar::Letter_Uppercase) |
                                                   FLAG(QChar::Letter_Lowercase) |
                                                   FLAG(QChar::Letter_Titlecase) |
                                                   FLAG(QChar::Letter_Modifier) |
                                                   FLAG(QChar::Letter_Other));
                    } else {
                        switch (category.at(1)) {
                        case 'u': yyCharClass->addCategories(FLAG(QChar::Letter_Uppercase)); break; // Lu
                        case 'l': yyCharClass->addCategories(FLAG(QChar::Letter_Lowercase)); break; // Ll
                        case 't': yyCharClass->addCategories(FLAG(QChar::Letter_Titlecase)); break; // Lt
                        case 'm': yyCharClass->addCategories(FLAG(QChar::Letter_Modifier)); break; // Lm
                        case 'o': yyCharClass->addCategories(FLAG(QChar::Letter_Other)); break; // Lo
                        default: error(RXERR_CATEGORY); break;
                        }
                    }
                    break;

                case 'P':
                    if (catlen == 1) {
                        yyCharClass->addCategories(FLAG(QChar::Punctuation_Connector) |
                                                   FLAG(QChar::Punctuation_Dash) |
                                                   FLAG(QChar::Punctuation_Open) |
                                                   FLAG(QChar::Punctuation_Close) |
                                                   FLAG(QChar::Punctuation_InitialQuote) |
                                                   FLAG(QChar::Punctuation_FinalQuote) |
                                                   FLAG(QChar::Punctuation_Other));
                    } else {
                        switch (category.at(1)) {
                        case 'c': yyCharClass->addCategories(FLAG(QChar::Punctuation_Connector)); break; // Pc
                        case 'd': yyCharClass->addCategories(FLAG(QChar::Punctuation_Dash)); break; // Pd
                        case 's': yyCharClass->addCategories(FLAG(QChar::Punctuation_Open)); break; // Ps
                        case 'e': yyCharClass->addCategories(FLAG(QChar::Punctuation_Close)); break; // Pe
                        case 'i': yyCharClass->addCategories(FLAG(QChar::Punctuation_InitialQuote)); break; // Pi
                        case 'f': yyCharClass->addCategories(FLAG(QChar::Punctuation_FinalQuote)); break; // Pf
                        case 'o': yyCharClass->addCategories(FLAG(QChar::Punctuation_Other)); break; // Po
                        default: error(RXERR_CATEGORY); break;
                        }
                    }
                    break;

                case 'S':
                    if (catlen == 1) {
                        yyCharClass->addCategories(FLAG(QChar::Symbol_Math) |
                                                   FLAG(QChar::Symbol_Currency) |
                                                   FLAG(QChar::Symbol_Modifier) |
                                                   FLAG(QChar::Symbol_Other));
                    } else {
                        switch (category.at(1)) {
                        case 'm': yyCharClass->addCategories(FLAG(QChar::Symbol_Math)); break;     // Sm
                        case 'c': yyCharClass->addCategories(FLAG(QChar::Symbol_Currency)); break; // Sc
                        case 'k': yyCharClass->addCategories(FLAG(QChar::Symbol_Modifier)); break; // Sk
                        case 'o': yyCharClass->addCategories(FLAG(QChar::Symbol_Other)); break;    // So
                        default: error(RXERR_CATEGORY); break;
                        }
                    }
                    break;

                default:
                    error(RXERR_CATEGORY);
                    break;
                }

            } else if (catlen > 2 && category.at(0) == 'I' && category.at(1) == 's') {
                static const int N = sizeof(categoriesRangeMap) / sizeof(categoriesRangeMap[0]);
                const char * const categoryFamily = category.constData() + 2;
                const CategoriesRangeMapEntry *r = std::lower_bound(categoriesRangeMap,
                     categoriesRangeMap + N, categoryFamily);

                if (r != categoriesRangeMap + N && qstrcmp(r->name, categoryFamily) == 0) {
                    yyCharClass->addRange(r->first, r->second);
                } else {
                    error(RXERR_CATEGORY);
                }

            } else {
                error(RXERR_CATEGORY);
            }

            return Tok_CharClass;
        } else {
            break;
        }
#endif

#ifndef QT_NO_REGEXP_ESCAPE
      case 'x':
         val = 0;
         for (i = 0; i < 4; i++) {
            low = QChar(yyCh).toLower().unicode();
            if (low >= '0' && low <= '9') {
               val = (val << 4) | (low - '0');
            } else if (low >= 'a' && low <= 'f') {
               val = (val << 4) | (low - 'a' + 10);
            } else {
               break;
            }
            yyCh = getChar();
         }
         return Tok_Char | val;
#endif

      default:
         break;
   }

   if (prevCh >= '1' && prevCh <= '9') {
#ifndef QT_NO_REGEXP_BACKREF
      val = prevCh - '0';

      while (yyCh >= '0' && yyCh <= '9') {
         val = (val * 10) + (yyCh - '0');
         yyCh = getChar();
      }

      return Tok_BackRef | val;
#else
      error(RXERR_DISABLED);

#endif
   }

   return Tok_Char | prevCh;
}

#ifndef QT_NO_REGEXP_INTERVAL
int QRegExpEngine::getRep(int def)
{
   if (yyCh >= '0' && yyCh <= '9') {
      int rep = 0;
      do {
         rep = 10 * rep + yyCh - '0';
         if (rep >= InftyRep) {
            error(RXERR_REPETITION);
            rep = def;
         }
         yyCh = getChar();

      } while (yyCh >= '0' && yyCh <= '9');
      return rep;

   } else {
      return def;
   }
}
#endif

#ifndef QT_NO_REGEXP_LOOKAHEAD
void QRegExpEngine::skipChars(int n)
{
   if (n > 0) {
      yyPos += n - 1;
      yyCh = getChar();
   }
}
#endif

void QRegExpEngine::error(const char *msg)
{
   if (yyError.isEmpty()) {
      yyError = QLatin1String(msg);
   }
}

void QRegExpEngine::startTokenizer(const QChar *rx, int len)
{
   yyIn = rx;
   yyPos0 = 0;
   yyPos = 0;
   yyLen = len;
   yyCh = getChar();
   yyCharClass.reset(new QRegExpCharClass);
   yyMinRep = 0;
   yyMaxRep = 0;
   yyError = QString();
}

int QRegExpEngine::getToken()
{
#ifndef QT_NO_REGEXP_CCLASS
   ushort pendingCh = 0;
   bool charPending;
   bool rangePending;
   int tok;
#endif
   int prevCh = yyCh;

   yyPos0 = yyPos - 1;
#ifndef QT_NO_REGEXP_CCLASS
   yyCharClass->clear();
#endif
   yyMinRep = 0;
   yyMaxRep = 0;
   yyCh = getChar();

   switch (prevCh) {
      case EOS:
         yyPos0 = yyPos;
         return Tok_Eos;
      case '$':
         return Tok_Dollar;
      case '(':
         if (yyCh == '?') {
            prevCh = getChar();
            yyCh = getChar();
            switch (prevCh) {
#ifndef QT_NO_REGEXP_LOOKAHEAD
               case '!':
                  return Tok_NegLookahead;
               case '=':
                  return Tok_PosLookahead;
#endif
               case ':':
                  return Tok_MagicLeftParen;
               case '<':
                  error(RXERR_LOOKBEHIND);
                  return Tok_MagicLeftParen;
               default:
                  error(RXERR_LOOKAHEAD);
                  return Tok_MagicLeftParen;
            }
         } else {
            return Tok_LeftParen;
         }
      case ')':
         return Tok_RightParen;
      case '*':
         yyMinRep = 0;
         yyMaxRep = InftyRep;
         return Tok_Quantifier;
      case '+':
         yyMinRep = 1;
         yyMaxRep = InftyRep;
         return Tok_Quantifier;
      case '.':
#ifndef QT_NO_REGEXP_CCLASS
         yyCharClass->setNegative(true);
#endif
         return Tok_CharClass;
      case '?':
         yyMinRep = 0;
         yyMaxRep = 1;
         return Tok_Quantifier;
      case '[':
#ifndef QT_NO_REGEXP_CCLASS
         if (yyCh == '^') {
            yyCharClass->setNegative(true);
            yyCh = getChar();
         }
         charPending = false;
         rangePending = false;
         do {
            if (yyCh == '-' && charPending && !rangePending) {
               rangePending = true;
               yyCh = getChar();
            } else {
               if (charPending && !rangePending) {
                  yyCharClass->addSingleton(pendingCh);
                  charPending = false;
               }
               if (yyCh == '\\') {
                  yyCh = getChar();
                  tok = getEscape();
                  if (tok == Tok_Word) {
                     tok = '\b';
                  }
               } else {
                  tok = Tok_Char | yyCh;
                  yyCh = getChar();
               }
               if (tok == Tok_CharClass) {
                  if (rangePending) {
                     yyCharClass->addSingleton('-');
                     yyCharClass->addSingleton(pendingCh);
                     charPending = false;
                     rangePending = false;
                  }
               } else if ((tok & Tok_Char) != 0) {
                  if (rangePending) {
                     yyCharClass->addRange(pendingCh, tok ^ Tok_Char);
                     charPending = false;
                     rangePending = false;
                  } else {
                     pendingCh = tok ^ Tok_Char;
                     charPending = true;
                  }
               } else {
                  error(RXERR_CHARCLASS);
               }
            }
         }  while (yyCh != ']' && yyCh != EOS);
         if (rangePending) {
            yyCharClass->addSingleton('-');
         }
         if (charPending) {
            yyCharClass->addSingleton(pendingCh);
         }
         if (yyCh == EOS) {
            error(RXERR_END);
         } else {
            yyCh = getChar();
         }
         return Tok_CharClass;
#else
         error(RXERR_END);
         return Tok_Char | '[';
#endif
      case '\\':
         return getEscape();
      case ']':
         error(RXERR_LEFTDELIM);
         return Tok_Char | ']';
      case '^':
         return Tok_Caret;
      case '{':
#ifndef QT_NO_REGEXP_INTERVAL
         yyMinRep = getRep(0);
         yyMaxRep = yyMinRep;
         if (yyCh == ',') {
            yyCh = getChar();
            yyMaxRep = getRep(InftyRep);
         }
         if (yyMaxRep < yyMinRep) {
            error(RXERR_INTERVAL);
         }
         if (yyCh != '}') {
            error(RXERR_REPETITION);
         }
         yyCh = getChar();
         return Tok_Quantifier;
#else
         error(RXERR_DISABLED);
         return Tok_Char | '{';
#endif
      case '|':
         return Tok_Bar;
      case '}':
         error(RXERR_LEFTDELIM);
         return Tok_Char | '}';
      default:
         return Tok_Char | prevCh;
   }
}

int QRegExpEngine::parse(const QChar *pattern, int len)
{
   valid = true;
   startTokenizer(pattern, len);
   yyTok = getToken();
#ifndef QT_NO_REGEXP_CAPTURE
   yyMayCapture = true;
#else
   yyMayCapture = false;
#endif

#ifndef QT_NO_REGEXP_CAPTURE
   int atom = startAtom(false);
#endif
   QRegExpCharClass anything;
   Box box(this); // create InitialState
   box.set(anything);
   Box rightBox(this); // create FinalState
   rightBox.set(anything);

   Box middleBox(this);
   parseExpression(&middleBox);
#ifndef QT_NO_REGEXP_CAPTURE
   finishAtom(atom, false);
#endif
#ifndef QT_NO_REGEXP_OPTIM
   middleBox.setupHeuristics();
#endif
   box.cat(middleBox);
   box.cat(rightBox);
   yyCharClass.reset(0);

#ifndef QT_NO_REGEXP_CAPTURE
   for (int i = 0; i < nf; ++i) {
      switch (f[i].capture) {
         case QRegExpAtom::NoCapture:
            break;
         case QRegExpAtom::OfficialCapture:
            f[i].capture = ncap;
            captureForOfficialCapture.append(ncap);
            ++ncap;
            ++officialncap;
            break;
         case QRegExpAtom::UnofficialCapture:
            f[i].capture = greedyQuantifiers ? ncap++ : QRegExpAtom::NoCapture;
      }
   }

#ifndef QT_NO_REGEXP_BACKREF
#ifndef QT_NO_REGEXP_OPTIM
   if (officialncap == 0 && nbrefs == 0) {
      ncap = nf = 0;
      f.clear();
   }
#endif
   // handle the case where there's a \5 with no corresponding capture
   // (captureForOfficialCapture.size() != officialncap)
   for (int i = 0; i < nbrefs - officialncap; ++i) {
      captureForOfficialCapture.append(ncap);
      ++ncap;
   }
#endif
#endif

   if (!yyError.isEmpty()) {
      return -1;
   }

#ifndef QT_NO_REGEXP_OPTIM
   const QRegExpAutomatonState &sinit = s.at(InitialState);
   caretAnchored = !sinit.anchors.isEmpty();
   if (caretAnchored) {
      const QMap<int, int> &anchors = sinit.anchors;
      QMap<int, int>::const_iterator a;
      for (a = anchors.constBegin(); a != anchors.constEnd(); ++a) {
         if (
#ifndef QT_NO_REGEXP_ANCHOR_ALT
            (*a & Anchor_Alternation) != 0 ||
#endif
            (*a & Anchor_Caret) == 0) {
            caretAnchored = false;
            break;
         }
      }
   }
#endif

   // cleanup anchors
   int numStates = s.count();
   for (int i = 0; i < numStates; ++i) {
      QRegExpAutomatonState &state = s[i];
      if (!state.anchors.isEmpty()) {
         QMap<int, int>::iterator a = state.anchors.begin();
         while (a != state.anchors.end()) {
            if (a.value() == 0) {
               a = state.anchors.erase(a);
            } else {
               ++a;
            }
         }
      }
   }

   return yyPos0;
}

void QRegExpEngine::parseAtom(Box *box)
{
#ifndef QT_NO_REGEXP_LOOKAHEAD
   QRegExpEngine *eng = 0;
   bool neg;
   int len;
#endif

   if ((yyTok & Tok_Char) != 0) {
      box->set(QChar(yyTok ^ Tok_Char));
   } else {
#ifndef QT_NO_REGEXP_OPTIM
      trivial = false;
#endif
      switch (yyTok) {
         case Tok_Dollar:
            box->catAnchor(Anchor_Dollar);
            break;
         case Tok_Caret:
            box->catAnchor(Anchor_Caret);
            break;
#ifndef QT_NO_REGEXP_LOOKAHEAD
         case Tok_PosLookahead:
         case Tok_NegLookahead:
            neg = (yyTok == Tok_NegLookahead);
            eng = new QRegExpEngine(cs, greedyQuantifiers);
            len = eng->parse(yyIn + yyPos - 1, yyLen - yyPos + 1);
            if (len >= 0) {
               skipChars(len);
            } else {
               error(RXERR_LOOKAHEAD);
            }
            box->catAnchor(addLookahead(eng, neg));
            yyTok = getToken();
            if (yyTok != Tok_RightParen) {
               error(RXERR_LOOKAHEAD);
            }
            break;
#endif
#ifndef QT_NO_REGEXP_ESCAPE
         case Tok_Word:
            box->catAnchor(Anchor_Word);
            break;
         case Tok_NonWord:
            box->catAnchor(Anchor_NonWord);
            break;
#endif
         case Tok_LeftParen:
         case Tok_MagicLeftParen:
            yyTok = getToken();
            parseExpression(box);
            if (yyTok != Tok_RightParen) {
               error(RXERR_END);
            }
            break;
         case Tok_CharClass:
            box->set(*yyCharClass);
            break;
         case Tok_Quantifier:
            error(RXERR_REPETITION);
            break;
         default:
#ifndef QT_NO_REGEXP_BACKREF
            if ((yyTok & Tok_BackRef) != 0) {
               box->set(yyTok ^ Tok_BackRef);
            } else
#endif
               error(RXERR_DISABLED);
      }
   }
   yyTok = getToken();
}

void QRegExpEngine::parseFactor(Box *box)
{
#ifndef QT_NO_REGEXP_CAPTURE
   int outerAtom = greedyQuantifiers ? startAtom(false) : -1;
   int innerAtom = startAtom(yyMayCapture && yyTok == Tok_LeftParen);
   bool magicLeftParen = (yyTok == Tok_MagicLeftParen);
#else
   const int innerAtom = -1;
#endif

#ifndef QT_NO_REGEXP_INTERVAL
#define YYREDO() \
        yyIn = in, yyPos0 = pos0, yyPos = pos, yyLen = len, yyCh = ch, \
        *yyCharClass = charClass, yyMinRep = 0, yyMaxRep = 0, yyTok = tok

   const QChar *in = yyIn;
   int pos0 = yyPos0;
   int pos = yyPos;
   int len = yyLen;
   int ch = yyCh;
   QRegExpCharClass charClass;
   if (yyTok == Tok_CharClass) {
      charClass = *yyCharClass;
   }
   int tok = yyTok;
   bool mayCapture = yyMayCapture;
#endif

   parseAtom(box);
#ifndef QT_NO_REGEXP_CAPTURE
   finishAtom(innerAtom, magicLeftParen);
#endif

   bool hasQuantifier = (yyTok == Tok_Quantifier);
   if (hasQuantifier) {
#ifndef QT_NO_REGEXP_OPTIM
      trivial = false;
#endif
      if (yyMaxRep == InftyRep) {
         box->plus(innerAtom);
#ifndef QT_NO_REGEXP_INTERVAL
      } else if (yyMaxRep == 0) {
         box->clear();
#endif
      }
      if (yyMinRep == 0) {
         box->opt();
      }

#ifndef QT_NO_REGEXP_INTERVAL
      yyMayCapture = false;
      int alpha = (yyMinRep == 0) ? 0 : yyMinRep - 1;
      int beta = (yyMaxRep == InftyRep) ? 0 : yyMaxRep - (alpha + 1);

      Box rightBox(this);
      int i;

      for (i = 0; i < beta; i++) {
         YYREDO();
         Box leftBox(this);
         parseAtom(&leftBox);
         leftBox.cat(rightBox);
         leftBox.opt();
         rightBox = leftBox;
      }
      for (i = 0; i < alpha; i++) {
         YYREDO();
         Box leftBox(this);
         parseAtom(&leftBox);
         leftBox.cat(rightBox);
         rightBox = leftBox;
      }
      rightBox.cat(*box);
      *box = rightBox;
#endif
      yyTok = getToken();
#ifndef QT_NO_REGEXP_INTERVAL
      yyMayCapture = mayCapture;
#endif
   }
#undef YYREDO
#ifndef QT_NO_REGEXP_CAPTURE
   if (greedyQuantifiers) {
      finishAtom(outerAtom, hasQuantifier);
   }
#endif
}

void QRegExpEngine::parseTerm(Box *box)
{
#ifndef QT_NO_REGEXP_OPTIM
   if (yyTok != Tok_Eos && yyTok != Tok_RightParen && yyTok != Tok_Bar) {
      parseFactor(box);
   }
#endif
   while (yyTok != Tok_Eos && yyTok != Tok_RightParen && yyTok != Tok_Bar) {
      Box rightBox(this);
      parseFactor(&rightBox);
      box->cat(rightBox);
   }
}

void QRegExpEngine::parseExpression(Box *box)
{
   parseTerm(box);
   while (yyTok == Tok_Bar) {
#ifndef QT_NO_REGEXP_OPTIM
      trivial = false;
#endif
      Box rightBox(this);
      yyTok = getToken();
      parseTerm(&rightBox);
      box->orx(rightBox);
   }
}

/*
  The struct QRegExpPrivate contains the private data of a regular
  expression other than the automaton. It makes it possible for many
  QRegExp objects to use the same QRegExpEngine object with different
  QRegExpPrivate objects.
*/
struct QRegExpPrivate {
   QRegExpEngine *eng;
   QRegExpEngineKey engineKey;
   bool minimal;
#ifndef QT_NO_REGEXP_CAPTURE
   QString t; // last string passed to QRegExp::indexIn() or lastIndexIn()
   QStringList capturedCache; // what QRegExp::capturedTexts() returned last
#endif
   QRegExpMatchState matchState;

   inline QRegExpPrivate()
      : eng(0), engineKey(QString(), QRegExp::RegExp, Qt::CaseSensitive), minimal(false) { }
   inline QRegExpPrivate(const QRegExpEngineKey &key)
      : eng(0), engineKey(key), minimal(false) {}
};

uint qHash(const QRegExpEngineKey &key, uint seed)
{
   return qHash(key.pattern, seed);
}

typedef QCache<QRegExpEngineKey, QRegExpEngine> EngineCache;
Q_GLOBAL_STATIC(EngineCache, globalEngineCache)
Q_GLOBAL_STATIC(QMutex, mutex)


static void derefEngine(QRegExpEngine *eng, const QRegExpEngineKey &key)
{
   if (! eng->ref.deref()) {

      if (globalEngineCache()) {
         QMutexLocker locker(mutex());

         QT_TRY {
            globalEngineCache()->insert(key, eng, 4 + key.pattern.length() / 4);

         } QT_CATCH(const std::bad_alloc &) {
            // in case of an exception (e.g. oom), just delete the engine
            delete eng;
         }

      } else {
         delete eng;
      }

   }
}

static void prepareEngine_helper(QRegExpPrivate *priv)
{
   bool initMatchState = !priv->eng;

   if (!priv->eng && globalEngineCache()) {
      QMutexLocker locker(mutex());
      priv->eng = globalEngineCache()->take(priv->engineKey);
      if (priv->eng != 0) {
         priv->eng->ref.ref();
      }
   }

   if (!priv->eng) {
      priv->eng = new QRegExpEngine(priv->engineKey);
   }

   if (initMatchState) {
      priv->matchState.prepareForMatch(priv->eng);
   }
}

inline static void prepareEngine(QRegExpPrivate *priv)
{
   if (priv->eng) {
      return;
   }
   prepareEngine_helper(priv);
}

static void prepareEngineForMatch(QRegExpPrivate *priv, const QString &str)
{
   prepareEngine(priv);
   priv->matchState.prepareForMatch(priv->eng);
#ifndef QT_NO_REGEXP_CAPTURE
   priv->t = str;
   priv->capturedCache.clear();
#else
   Q_UNUSED(str);
#endif
}

static void invalidateEngine(QRegExpPrivate *priv)
{
   if (priv->eng != 0) {
      derefEngine(priv->eng, priv->engineKey);
      priv->eng = 0;
      priv->matchState.drain();
   }
}

QRegExp::QRegExp()
{
   priv = new QRegExpPrivate;
   prepareEngine(priv);
}

QRegExp::QRegExp(const QString &pattern, Qt::CaseSensitivity cs, PatternSyntax syntax)
{
   priv = new QRegExpPrivate(QRegExpEngineKey(pattern, syntax, cs));
   prepareEngine(priv);
}

QRegExp::QRegExp(const QRegExp &rx)
{
   priv = new QRegExpPrivate;
   operator=(rx);
}

QRegExp::~QRegExp()
{
   invalidateEngine(priv);
   delete priv;
}

QRegExp &QRegExp::operator=(const QRegExp &rx)
{
   prepareEngine(rx.priv); // to allow sharing
   QRegExpEngine *otherEng = rx.priv->eng;
   if (otherEng) {
      otherEng->ref.ref();
   }
   invalidateEngine(priv);
   priv->eng = otherEng;
   priv->engineKey = rx.priv->engineKey;
   priv->minimal = rx.priv->minimal;
#ifndef QT_NO_REGEXP_CAPTURE
   priv->t = rx.priv->t;
   priv->capturedCache = rx.priv->capturedCache;
#endif
   if (priv->eng) {
      priv->matchState.prepareForMatch(priv->eng);
   }
   priv->matchState.captured = rx.priv->matchState.captured;
   return *this;
}

bool QRegExp::operator==(const QRegExp &rx) const
{
   return priv->engineKey == rx.priv->engineKey && priv->minimal == rx.priv->minimal;
}

bool QRegExp::isEmpty() const
{
   return priv->engineKey.pattern.isEmpty();
}

bool QRegExp::isValid() const
{
   if (priv->engineKey.pattern.isEmpty()) {
      return true;
   } else {
      prepareEngine(priv);
      return priv->eng->isValid();
   }
}

QString QRegExp::pattern() const
{
   return priv->engineKey.pattern;
}

void QRegExp::setPattern(const QString &pattern)
{
   if (priv->engineKey.pattern != pattern) {
      invalidateEngine(priv);
      priv->engineKey.pattern = pattern;
   }
}

Qt::CaseSensitivity QRegExp::caseSensitivity() const
{
   return priv->engineKey.cs;
}

void QRegExp::setCaseSensitivity(Qt::CaseSensitivity cs)
{
   if ((bool)cs != (bool)priv->engineKey.cs) {
      invalidateEngine(priv);
      priv->engineKey.cs = cs;
   }
}

QRegExp::PatternSyntax QRegExp::patternSyntax() const
{
   return priv->engineKey.patternSyntax;
}

void QRegExp::setPatternSyntax(PatternSyntax syntax)
{
   if (syntax != priv->engineKey.patternSyntax) {
      invalidateEngine(priv);
      priv->engineKey.patternSyntax = syntax;
   }
}

bool QRegExp::isMinimal() const
{
   return priv->minimal;
}

void QRegExp::setMinimal(bool minimal)
{
   priv->minimal = minimal;
}

// ### Qt5: make non-const

bool QRegExp::exactMatch(const QString &str) const
{
   prepareEngineForMatch(priv, str);
   priv->matchState.match(str.unicode(), str.length(), 0, priv->minimal, true, 0);

   if (priv->matchState.captured[1] == str.length()) {
      return true;
   } else {
      priv->matchState.captured[0] = 0;
      priv->matchState.captured[1] = priv->matchState.oneTestMatchedLen;
      return false;
   }
}

// ### Qt5: make non-const

int QRegExp::indexIn(const QString &str, int offset, CaretMode caretMode) const
{
   prepareEngineForMatch(priv, str);

   if (offset < 0) {
      offset += str.length();
   }

   priv->matchState.match(str.unicode(), str.length(), offset,
                  priv->minimal, false, caretIndex(offset, caretMode));

   return priv->matchState.captured[0];
}

// ### Qt5: make non-const

int QRegExp::lastIndexIn(const QString &str, int offset, CaretMode caretMode) const
{
   prepareEngineForMatch(priv, str);
   if (offset < 0) {
      offset += str.length();
   }
   if (offset < 0 || offset > str.length()) {
      memset(priv->matchState.captured, -1, priv->matchState.capturedSize * sizeof(int));
      return -1;
   }

   while (offset >= 0) {
      priv->matchState.match(str.unicode(), str.length(), offset,
                             priv->minimal, true, caretIndex(offset, caretMode));
      if (priv->matchState.captured[0] == offset) {
         return offset;
      }
      --offset;
   }
   return -1;
}

int QRegExp::matchedLength() const
{
   return priv->matchState.captured[1];
}

#ifndef QT_NO_REGEXP_CAPTURE

int QRegExp::captureCount() const
{
   prepareEngine(priv);
   return priv->eng->captureCount();
}

QStringList QRegExp::capturedTexts() const
{
   if (priv->capturedCache.isEmpty()) {
      prepareEngine(priv);
      const int *captured = priv->matchState.captured;
      int n = priv->matchState.capturedSize;

      for (int i = 0; i < n; i += 2) {
         QString m;

         if (captured[i + 1] == 0) {
            m = QLatin1String("");        // ### Qt5: do not distinguish between null and empty

         } else if (captured[i] >= 0) {
            m = priv->t.mid(captured[i], captured[i + 1]);
         }

         priv->capturedCache.append(m);
      }

      priv->t.clear();
   }
   return priv->capturedCache;
}

/*!
    \internal
*/
QStringList QRegExp::capturedTexts()
{
   return const_cast<const QRegExp *>(this)->capturedTexts();
}

QString QRegExp::cap(int nth) const
{
   return capturedTexts().value(nth);
}

/*!
    \internal
*/
QString QRegExp::cap(int nth)
{
   return const_cast<const QRegExp *>(this)->cap(nth);
}

int QRegExp::pos(int nth) const
{
   if (nth < 0 || nth >= priv->matchState.capturedSize / 2) {
      return -1;
   } else {
      return priv->matchState.captured[2 * nth];
   }
}

/*!
    \internal
*/
int QRegExp::pos(int nth)
{
   return const_cast<const QRegExp *>(this)->pos(nth);
}

QString QRegExp::errorString() const
{
   if (isValid()) {
      return QString::fromLatin1(RXERR_OK);
   } else {
      return priv->eng->errorString();
   }
}

/*!
    \internal
*/
QString QRegExp::errorString()
{
   return const_cast<const QRegExp *>(this)->errorString();
}
#endif


QString QRegExp::escape(const QString &str)
{
   QString quoted;
   const int count = str.count();
   quoted.reserve(count * 2);
   const QLatin1Char backslash('\\');
   for (int i = 0; i < count; i++) {
      switch (str.at(i).toLatin1()) {
         case '$':
         case '(':
         case ')':
         case '*':
         case '+':
         case '.':
         case '?':
         case '[':
         case '\\':
         case ']':
         case '^':
         case '{':
         case '|':
         case '}':
            quoted.append(backslash);
      }
      quoted.append(str.at(i));
   }
   return quoted;
}

#ifndef QT_NO_DATASTREAM

QDataStream &operator<<(QDataStream &out, const QRegExp &regExp)
{
   return out << regExp.pattern() << (quint8)regExp.caseSensitivity()
          << (quint8)regExp.patternSyntax()
          << (quint8)!!regExp.isMinimal();
}

QDataStream &operator>>(QDataStream &in, QRegExp &regExp)
{
   QString pattern;
   quint8 cs;
   quint8 patternSyntax;
   quint8 isMinimal;

   in >> pattern >> cs >> patternSyntax >> isMinimal;

   QRegExp newRegExp(pattern, Qt::CaseSensitivity(cs),
                     QRegExp::PatternSyntax(patternSyntax));

   newRegExp.setMinimal(isMinimal);
   regExp = newRegExp;
   return in;
}
#endif // QT_NO_DATASTREAM
