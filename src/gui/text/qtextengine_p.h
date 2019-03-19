/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QTEXTENGINE_P_H
#define QTEXTENGINE_P_H

#include <qglobal.h>
#include <qstring.h>
#include <qvarlengtharray.h>
#include <qnamespace.h>
#include <qtextlayout.h>
#include <qtextformat_p.h>
#include <qfont_p.h>
#include <qvector.h>
#include <qpaintengine.h>
#include <qtextobject.h>
#include <qtextoption.h>
#include <qtextcursor.h>
#include <qset.h>
#include <qdebug.h>

#ifndef QT_BUILD_COMPAT_LIB
#include <qtextdocument_p.h>
#endif

#include <qharfbuzz_p.h>
#include <qfixed_p.h>
#include <stdlib.h>

QT_BEGIN_NAMESPACE

class QFontPrivate;
class QFontEngine;
class QPainter;
class QAbstractTextDocumentLayout;

// this uses the same coordinate system as Qt, but a different one to freetype.
// * y is usually negative, and is equal to the ascent.
// * negative yoff means the following stuff is drawn higher up.
// the characters bounding rect is given by QRect(x,y,width,height), its advance by
// xoo and yoff
struct glyph_metrics_t {
   inline glyph_metrics_t()
      : x(100000),  y(100000) {}
   inline glyph_metrics_t(QFixed _x, QFixed _y, QFixed _width, QFixed _height, QFixed _xoff, QFixed _yoff)
      : x(_x),
        y(_y),
        width(_width),
        height(_height),
        xoff(_xoff),
        yoff(_yoff) {
   }
   QFixed x;
   QFixed y;
   QFixed width;
   QFixed height;
   QFixed xoff;
   QFixed yoff;

   glyph_metrics_t transformed(const QTransform &xform) const;
   inline bool isValid() const {
      return x != 100000 && y != 100000;
   }
};
Q_DECLARE_TYPEINFO(glyph_metrics_t, Q_PRIMITIVE_TYPE);

struct QScriptAnalysis {
   enum Flags {
      None = 0,
      Lowercase = 1,
      Uppercase = 2,
      SmallCaps = 3,
      LineOrParagraphSeparator = 4,
      Space = 5,
      SpaceTabOrObject = Space,
      Tab = 6,
      TabOrObject = Tab,
      Object = 7
   };
   unsigned short script    : 7;
   unsigned short bidiLevel : 6;  // Unicode Bidi algorithm embedding level (0-61)
   unsigned short flags     : 3;
   inline bool operator == (const QScriptAnalysis &other) const {
      return script == other.script && bidiLevel == other.bidiLevel && flags == other.flags;
   }
};
Q_DECLARE_TYPEINFO(QScriptAnalysis, Q_PRIMITIVE_TYPE);

struct QGlyphJustification {
   inline QGlyphJustification()
      : type(0), nKashidas(0), space_18d6(0) {
   }

   enum JustificationType {
      JustifyNone,
      JustifySpace,
      JustifyKashida
   };

   uint type : 2;
   uint nKashidas : 6; // more do not make sense...
   uint space_18d6 : 24;
};
Q_DECLARE_TYPEINFO(QGlyphJustification, Q_PRIMITIVE_TYPE);

struct QGlyphLayoutInstance {
   QFixedPoint offset;
   QFixedPoint advance;
   HB_Glyph glyph;
   QGlyphJustification justification;
   HB_GlyphAttributes attributes;
};

struct QGlyphLayout {
   // init to 0 not needed, done when shaping
   QFixedPoint *offsets;   // 8 bytes per element
   HB_Glyph *glyphs;       // 4 bytes per element
   QFixed *advances_x;     // 4 bytes per element
   QFixed *advances_y;     // 4 bytes per element
   QGlyphJustification *justifications; // 4 bytes per element
   HB_GlyphAttributes *attributes; // 2 bytes per element

   int numGlyphs;

   inline QGlyphLayout() : numGlyphs(0) {}

   inline explicit QGlyphLayout(char *address, int totalGlyphs) {
      offsets = reinterpret_cast<QFixedPoint *>(address);
      int offset = totalGlyphs * sizeof(HB_FixedPoint);

      glyphs = reinterpret_cast<HB_Glyph *>(address + offset);
      offset += totalGlyphs * sizeof(HB_Glyph);

      advances_x = reinterpret_cast<QFixed *>(address + offset);
      offset += totalGlyphs * sizeof(QFixed);

      advances_y = reinterpret_cast<QFixed *>(address + offset);
      offset += totalGlyphs * sizeof(QFixed);

      justifications = reinterpret_cast<QGlyphJustification *>(address + offset);
      offset += totalGlyphs * sizeof(QGlyphJustification);

      attributes = reinterpret_cast<HB_GlyphAttributes *>(address + offset);
      numGlyphs = totalGlyphs;
   }

   inline QGlyphLayout mid(int position, int n = -1) const {
      QGlyphLayout copy = *this;
      copy.glyphs += position;
      copy.advances_x += position;
      copy.advances_y += position;
      copy.offsets += position;
      copy.justifications += position;
      copy.attributes += position;
      if (n == -1) {
         copy.numGlyphs -= position;
      } else {
         copy.numGlyphs = n;
      }
      return copy;
   }

   static inline int spaceNeededForGlyphLayout(int totalGlyphs) {
      return totalGlyphs * (sizeof(HB_Glyph) + sizeof(HB_GlyphAttributes)
                            + sizeof(QFixed) + sizeof(QFixed) + sizeof(QFixedPoint)
                            + sizeof(QGlyphJustification));
   }

   inline QFixed effectiveAdvance(int item) const {
      return (advances_x[item] + QFixed::fromFixed(justifications[item].space_18d6)) * !attributes[item].dontPrint;
   }

   inline QGlyphLayoutInstance instance(int position) const {
      QGlyphLayoutInstance g;
      g.offset.x = offsets[position].x;
      g.offset.y = offsets[position].y;
      g.glyph = glyphs[position];
      g.advance.x = advances_x[position];
      g.advance.y = advances_y[position];
      g.justification = justifications[position];
      g.attributes = attributes[position];
      return g;
   }

   inline void setInstance(int position, const QGlyphLayoutInstance &g) {
      offsets[position].x = g.offset.x;
      offsets[position].y = g.offset.y;
      glyphs[position] = g.glyph;
      advances_x[position] = g.advance.x;
      advances_y[position] = g.advance.y;
      justifications[position] = g.justification;
      attributes[position] = g.attributes;
   }

   inline void clear(int first = 0, int last = -1) {
      if (last == -1) {
         last = numGlyphs;
      }

      if (first == 0 && last == numGlyphs && reinterpret_cast<char *>(offsets + numGlyphs) == reinterpret_cast<char *>(glyphs)) {
         memset(offsets, 0, spaceNeededForGlyphLayout(numGlyphs));

      } else {
         const int num = last - first;
         memset(offsets + first, 0, num * sizeof(QFixedPoint));
         memset(glyphs + first, 0, num * sizeof(HB_Glyph));
         memset(advances_x + first, 0, num * sizeof(QFixed));
         memset(advances_y + first, 0, num * sizeof(QFixed));
         memset(justifications + first, 0, num * sizeof(QGlyphJustification));
         memset(attributes + first, 0, num * sizeof(HB_GlyphAttributes));
      }
   }

   inline char *data() {
      return reinterpret_cast<char *>(offsets);
   }

   void grow(char *address, int totalGlyphs);
};

class QVarLengthGlyphLayoutArray : private QVarLengthArray<void *>, public QGlyphLayout
{
 private:
   typedef QVarLengthArray<void *> Array;

 public:
   QVarLengthGlyphLayoutArray(int totalGlyphs)
      : Array(spaceNeededForGlyphLayout(totalGlyphs) / sizeof(void *) + 1)
      , QGlyphLayout(reinterpret_cast<char *>(Array::data()), totalGlyphs) {
      memset(Array::data(), 0, Array::size() * sizeof(void *));
   }

   void resize(int totalGlyphs) {
      Array::resize(spaceNeededForGlyphLayout(totalGlyphs) / sizeof(void *) + 1);

      *((QGlyphLayout *)this) = QGlyphLayout(reinterpret_cast<char *>(Array::data()), totalGlyphs);
      memset(Array::data(), 0, Array::size() * sizeof(void *));
   }
};

template <int N> struct QGlyphLayoutArray : public QGlyphLayout {
 public:
   QGlyphLayoutArray()
      : QGlyphLayout(reinterpret_cast<char *>(buffer), N) {
      memset(buffer, 0, sizeof(buffer));
   }

 private:
   void *buffer[(N * (sizeof(HB_Glyph) + sizeof(HB_GlyphAttributes)
                      + sizeof(QFixed) + sizeof(QFixed) + sizeof(QFixedPoint)
                      + sizeof(QGlyphJustification))) / sizeof(void *) + 1];
};

struct QScriptItem;

// internal QTextItem
class QTextItemInt : public QTextItem
{
 public:
   inline QTextItemInt()
      : justified(false), underlineStyle(QTextCharFormat::NoUnderline), logClusters(0), f(0), fontEngine(0) {
   }

   QTextItemInt(const QScriptItem &si, QFont *font, const QTextCharFormat &format = QTextCharFormat());

   QTextItemInt(const QGlyphLayout &g, QFont *font, QString::const_iterator begin, QString::const_iterator end,
                  QFontEngine *fe, const QTextCharFormat &format = QTextCharFormat());

   // copy the structure items, adjusting the glyphs arrays to the right subarrays.
   // the width of the returned QTextItemInt is not adjusted, for speed reasons
   QTextItemInt midItem(QFontEngine *fontEngine, int firstGlyphIndex, int numGlyphs) const;
   void initWithScriptItem(const QScriptItem &si);

   QFixed descent;
   QFixed ascent;
   QFixed width;

   RenderFlags flags;
   bool justified;
   QTextCharFormat::UnderlineStyle underlineStyle;
   const QTextCharFormat charFormat;

   QString::const_iterator m_iter;
   QString::const_iterator m_end;

   const unsigned short *logClusters;
   const QFont *f;

   QGlyphLayout glyphs;
   QFontEngine *fontEngine;
};

inline bool qIsControlChar(QChar uc)
{
   return uc >= 0x200b && uc <= 0x206f
          && (uc <= 0x200f /* ZW Space, ZWNJ, ZWJ, LRM and RLM */
              || (uc >= 0x2028 && uc <= 0x202f /* LS, PS, LRE, RLE, PDF, LRO, RLO, NNBSP */)
              || uc >= 0x206a /* ISS, ASS, IAFS, AFS, NADS, NODS */);
}

struct QScriptItem {
   inline QScriptItem()
      : position(0),
        num_glyphs(0), descent(-1), ascent(-1), leading(-1), width(-1),
        glyph_data_offset(0) {}
   inline QScriptItem(int p, const QScriptAnalysis &a)
      : position(p), analysis(a),
        num_glyphs(0), descent(-1), ascent(-1), leading(-1), width(-1),
        glyph_data_offset(0) {}

   int position;
   QScriptAnalysis analysis;
   unsigned short num_glyphs;
   QFixed descent;
   QFixed ascent;
   QFixed leading;
   QFixed width;
   int glyph_data_offset;
   QFixed height() const {
      return ascent + descent;
   }
};


Q_DECLARE_TYPEINFO(QScriptItem, Q_MOVABLE_TYPE);

typedef QVector<QScriptItem> QScriptItemArray;

struct QScriptLine {
   // created and filled in QTextLine::layout_helper
   QScriptLine()
      : from(0), trailingSpaces(0), length(0),
        justified(0), gridfitted(0),
        hasTrailingSpaces(0), leadingIncluded(0) {}
   QFixed descent;
   QFixed ascent;
   QFixed leading;
   QFixed x;
   QFixed y;
   QFixed width;
   QFixed textWidth;
   QFixed textAdvance;
   int from;
   unsigned short trailingSpaces;
   signed int length : 28;
   mutable uint justified : 1;
   mutable uint gridfitted : 1;
   uint hasTrailingSpaces : 1;
   uint leadingIncluded : 1;
   QFixed height() const {
      return (ascent + descent).ceil()
             + (leadingIncluded ?  qMax(QFixed(), leading) : QFixed());
   }
   QFixed base() const {
      return ascent
             + (leadingIncluded ? qMax(QFixed(), leading) : QFixed());
   }
   void setDefaultHeight(QTextEngine *eng);
   void operator+=(const QScriptLine &other);
};
Q_DECLARE_TYPEINFO(QScriptLine, Q_PRIMITIVE_TYPE);


inline void QScriptLine::operator+=(const QScriptLine &other)
{
   leading = qMax(leading + ascent, other.leading + other.ascent) - qMax(ascent, other.ascent);
   descent = qMax(descent, other.descent);
   ascent = qMax(ascent, other.ascent);
   textWidth += other.textWidth;
   length += other.length;
}

typedef QVector<QScriptLine> QScriptLineArray;

class QFontPrivate;
class QTextFormatCollection;

class Q_GUI_EXPORT QTextEngine
{
 public:
   enum LayoutState {
      LayoutEmpty,
      InLayout,
      LayoutFailed,
   };

   struct LayoutData {
      LayoutData(const QString &str, void **stack_memory, int mem_size);
      LayoutData();
      ~LayoutData();

      mutable QScriptItemArray items;
      int allocated;
      int available_glyphs;
      void **memory;
      unsigned short *logClustersPtr;
      QGlyphLayout glyphLayout;
      mutable int used;
      uint hasBidi : 1;
      uint layoutState : 2;
      uint memory_on_stack : 1;
      uint haveCharAttributes : 1;
      QString string;
      bool reallocate(int totalGlyphs);
   };

   QTextEngine(LayoutData *data);
   QTextEngine();
   QTextEngine(const QString &str, const QFont &f);
   ~QTextEngine();

   enum Mode {
      WidthOnly = 0x07
   };

   // keep in sync with QAbstractFontEngine::TextShapingFlag!!
   enum ShaperFlag {
      RightToLeft = 0x0001,
      DesignMetrics = 0x0002,
      GlyphIndicesOnly = 0x0004
   };
   using ShaperFlags = QFlags<ShaperFlag>;

   void invalidate();
   void clearLineData();

   void validate() const;
   void itemize() const;

   bool isRightToLeft() const;
   static bool isRightToLeft(QStringView str);

   static void bidiReorder(int numRuns, const quint8 *levels, int *visualOrder);

   const HB_CharAttributes *attributes() const;

   void shape(int item) const;

   void justify(const QScriptLine &si);
   QFixed alignLine(const QScriptLine &line);

   QFixed width(int charFrom, int numChars) const;
   glyph_metrics_t boundingBox(int from,  int len) const;
   glyph_metrics_t tightBoundingBox(int from,  int len) const;

   int length(int item) const {
      const QScriptItem &si = layoutData->items[item];
      int from = si.position;
      item++;

      return (item < layoutData->items.size() ? layoutData->items[item].position : layoutData->string.size()) - from;
   }

   int length(const QScriptItem *si) const {
      int end;

      if (si + 1 < layoutData->items.constData() + layoutData->items.size()) {
         end = (si + 1)->position;
      } else {
         end = layoutData->string.size();
      }

      return end - si->position;
   }

   QFontEngine *fontEngine(const QScriptItem &si, QFixed *ascent = 0, QFixed *descent = 0, QFixed *leading = 0) const;
   QFont font(const QScriptItem &si) const;
   inline QFont font() const {
      return fnt;
   }

   /*
    * Returns a pointer to an array of log clusters, offset at the script item.
    * Each item in the array is a unsigned short.  For each character in the original string there is an entry in the table
    * so there is a one to one correlation in indexes between the original text and the index in the logcluster.
    * The value of each item is the position in the glyphs array. Multiple similar pointers in the logclusters array imply
    * that one glyph is used for more than one character.
    * \sa glyphs()
    */
   inline unsigned short *logClusters(const QScriptItem *si) const {
      return layoutData->logClustersPtr + si->position;
   }

   /*
    * Returns an array of QGlyphLayout items, offset at the script item.
    * Each item in the array matches one glyph in the text, storing the advance, position etc.
    * The returned item's length equals to the number of available glyphs. This may be more
    * than what was actually shaped.
    * \sa logClusters()
    */
   inline QGlyphLayout availableGlyphs(const QScriptItem *si) const {
      return layoutData->glyphLayout.mid(si->glyph_data_offset);
   }

   /*
    * Returns an array of QGlyphLayout items, offset at the script item.
    * Each item in the array matches one glyph in the text, storing the advance, position etc.
    * The returned item's length equals to the number of shaped glyphs.
    * \sa logClusters()
    */
   inline QGlyphLayout shapedGlyphs(const QScriptItem *si) const {
      return layoutData->glyphLayout.mid(si->glyph_data_offset, si->num_glyphs);
   }

   bool ensureSpace(int nGlyphs) const;

   void freeMemory();

   int findItem(int strPos) const;
   inline QTextFormatCollection *formats() const {
#ifdef QT_BUILD_COMPAT_LIB
      return 0; // Compat should never reference this symbol
#else
      return block.docHandle()->formatCollection();
#endif
   }
   QTextCharFormat format(const QScriptItem *si) const;
   inline QAbstractTextDocumentLayout *docLayout() const {
#ifdef QT_BUILD_COMPAT_LIB
      return 0; // Compat should never reference this symbol
#else
      return block.docHandle()->document()->documentLayout();
#endif
   }
   int formatIndex(const QScriptItem *si) const;

   // returns the width of tab at index (in the tabs array) with the tab-start at position x
   QFixed calculateTabWidth(int index, QFixed x) const;

   mutable QScriptLineArray lines;

   struct FontEngineCache {
      FontEngineCache();
      mutable QFontEngine *prevFontEngine;
      mutable QFontEngine *prevScaledFontEngine;
      mutable int prevScript;
      mutable int prevPosition;
      mutable int prevLength;
      inline void reset() {
         prevFontEngine = 0;
         prevScaledFontEngine = 0;
         prevScript = -1;
         prevPosition = -1;
         prevLength = -1;
      }
   };
   mutable FontEngineCache feCache;

   QString text;
   QFont fnt;
   QTextBlock block;

   QTextOption option;

   QFixed minWidth;
   QFixed maxWidth;
   QPointF position;
   uint ignoreBidi : 1;
   uint cacheGlyphs : 1;
   uint stackEngine : 1;
   uint forceJustification : 1;
   uint visualMovement : 1;

   int *underlinePositions;

   mutable LayoutData *layoutData;

   inline bool hasFormats() const {
      return (block.docHandle() || specialData);
   }
   inline bool visualCursorMovement() const {
      return (visualMovement ||
              (block.docHandle() ? block.docHandle()->defaultCursorMoveStyle == Qt::VisualMoveStyle : false));
   }

   struct SpecialData {
      int preeditPosition;
      QString preeditText;
      QList<QTextLayout::FormatRange> addFormats;
      QVector<int> addFormatIndices;
      QVector<int> resolvedFormatIndices;
   };
   SpecialData *specialData;

   bool atWordSeparator(int position) const;
   bool atSpace(int position) const;
   void indexAdditionalFormats();

   QString elidedText(Qt::TextElideMode mode, const QFixed &width, int flags = 0) const;

   void shapeLine(const QScriptLine &line);
   QFixed leadingSpaceWidth(const QScriptLine &line);

   QFixed offsetInLigature(const QScriptItem *si, int pos, int max, int glyph_pos);
   int positionInLigature(const QScriptItem *si, int end, QFixed x, QFixed edge, int glyph_pos, bool cursorOnCharacter);
   int previousLogicalPosition(int oldPos) const;
   int nextLogicalPosition(int oldPos) const;
   int lineNumberForTextPosition(int pos);
   int positionAfterVisualMovement(int oldPos, QTextCursor::MoveOperation op);
   void insertionPointsForLine(int lineNum, QVector<int> &insertionPoints);
   void resetFontEngineCache();

 private:
   void setBoundary(int strPos) const;
   void addRequiredBoundaries() const;
   void shapeText(int item) const;
   void shapeTextWithHarfbuzz(int item) const;

#if defined(Q_OS_MAC)
   void shapeTextMac(int item) const;
#endif
   void splitItem(int item, int pos) const;

   void resolveAdditionalFormats() const;
   int endOfLine(int lineNum);
   int beginningOfLine(int lineNum);
   int getClusterLength(unsigned short *logClusters, const HB_CharAttributes *attributes, int from, int to, int glyph_pos,
                        int *start);
};

class QStackTextEngine : public QTextEngine
{
 public:
   enum { MemSize = 256 * 40 / sizeof(void *) };
   QStackTextEngine(const QString &string, const QFont &f);
   LayoutData _layoutData;
   void *_memory[MemSize];
};

struct QTextLineItemIterator {
   QTextLineItemIterator(QTextEngine *eng, int lineNum, const QPointF &pos = QPointF(),
                         const QTextLayout::FormatRange *_selection = 0);

   inline bool atEnd() const {
      return logicalItem >= nItems - 1;
   }
   inline bool atBeginning() const {
      return logicalItem <= 0;
   }
   QScriptItem &next();

   bool getSelectionBounds(QFixed *selectionX, QFixed *selectionWidth) const;
   inline bool isOutsideSelection() const {
      QFixed tmp1, tmp2;
      return !getSelectionBounds(&tmp1, &tmp2);
   }

   QTextEngine *eng;

   QFixed x;
   QFixed pos_x;
   const QScriptLine &line;
   QScriptItem *si;

   int lineNum;
   int lineEnd;
   int firstItem;
   int lastItem;
   int nItems;
   int logicalItem;
   int item;
   int itemLength;

   int glyphsStart;
   int glyphsEnd;
   int itemStart;
   int itemEnd;

   QFixed itemWidth;

   QVarLengthArray<int> visualOrder;
   QVarLengthArray<uchar> levels;

   const QTextLayout::FormatRange *selection;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextEngine::ShaperFlags)

QT_END_NAMESPACE

#endif // QTEXTENGINE_P_H
