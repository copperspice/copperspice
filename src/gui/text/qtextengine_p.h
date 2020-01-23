/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QTEXTENGINE_P_H
#define QTEXTENGINE_P_H

#include <qglobal.h>

#include <qset.h>
#include <qstring.h>
#include <qvector.h>
#include <qvarlengtharray.h>

#include <qdebug.h>
#include <qnamespace.h>
#include <qtextlayout.h>
#include <qpaintengine.h>
#include <qtextobject.h>
#include <qtextoption.h>
#include <qtextcursor.h>

#include <qfixed_p.h>
#include <qfont_p.h>
#include <qtextformat_p.h>
#include <qtextdocument_p.h>
#include <qunicodetools_p.h>

#if ! defined(CS_BUILDING_CUPS)
#include <qharfbuzz_gui_p.h>
#endif

#include <stdlib.h>

class QTextFormatCollection;
class QFontEngine;
class QPainter;
class QAbstractTextDocumentLayout;

struct QScriptItem;
struct QScriptLine;

using QScriptItemArray = QVector<QScriptItem>;
using QScriptLineArray = QVector<QScriptLine>;

// uses the same coordinate system as CS, but different than freetype
// * y is usually negative and is equal to the ascent
// * negative yoff means the following glyphs are drawn higher on the screen
// bounding rect is QRect(x, y, width, height)
// advance to the next char by xoo and yoff

struct Q_GUI_EXPORT glyph_metrics_t {

   inline glyph_metrics_t()
      : x(100000),  y(100000) {}

   inline glyph_metrics_t(QFixed _x, QFixed _y, QFixed _width, QFixed _height, QFixed _xoff, QFixed _yoff)
      : x(_x), y(_y), width(_width), height(_height), xoff(_xoff), yoff(_yoff)
   { }

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

   inline QFixed leftBearing() const {
      if (! isValid()) {
         return QFixed();
      }

      return x;
   }

   inline QFixed rightBearing() const {
      if (! isValid()) {
         return QFixed();
      }

      return xoff - x - width;
   }
};
Q_DECLARE_TYPEINFO(glyph_metrics_t, Q_PRIMITIVE_TYPE);

struct QScriptAnalysis {
   enum Flags {
      None                     = 0,
      Lowercase                = 1,
      Uppercase                = 2,
      SmallCaps                = 3,
      LineOrParagraphSeparator = 4,
      Space                    = 5,
      SpaceTabOrObject         = Space,
      Tab                      = 6,
      TabOrObject              = Tab,
      Object                   = 7
   };

   QChar::Script script;

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
   uint nKashidas : 6;       // more does not make sense
   uint space_18d6 : 24;
};
Q_DECLARE_TYPEINFO(QGlyphJustification, Q_PRIMITIVE_TYPE);

struct QGlyphAttributes {
   uchar clusterStart  : 1;
   uchar dontPrint     : 1;
   uchar justification : 4;
   uchar reserved      : 2;
};
static_assert(sizeof(QGlyphAttributes) == 1, "Type mismatch");

#if defined(CS_BUILDING_CUPS)
   using glyph_t = uint32_t;

   using qt_destroy_func_ptr        = void *;
   using qt_get_font_table_func_ptr = void *;

#endif

struct QGlyphLayout {
   constexpr static const int SpaceRequired = sizeof(glyph_t) + sizeof(QFixed) + sizeof(QFixedPoint)
      + sizeof(QGlyphAttributes) + sizeof(QGlyphJustification);

   inline QGlyphLayout()
      : numGlyphs(0) {}

   inline explicit QGlyphLayout(char *address, int totalGlyphs) {
      offsets    = reinterpret_cast<QFixedPoint *>(address);
      int offset = totalGlyphs * sizeof(QFixedPoint);

      glyphs = reinterpret_cast<glyph_t *>(address + offset);
      offset += totalGlyphs * sizeof(glyph_t);

      advances = reinterpret_cast<QFixed *>(address + offset);
      offset += totalGlyphs * sizeof(QFixed);

      justifications = reinterpret_cast<QGlyphJustification *>(address + offset);
      offset += totalGlyphs * sizeof(QGlyphJustification);

      attributes = reinterpret_cast<QGlyphAttributes *>(address + offset);
      numGlyphs = totalGlyphs;
   }

   inline QGlyphLayout mid(int position, int n = -1) const {
      QGlyphLayout copy = *this;

      copy.glyphs         += position;
      copy.advances       += position;
      copy.offsets        += position;
      copy.justifications += position;
      copy.attributes     += position;

      if (n == -1) {
         copy.numGlyphs -= position;
      } else {
         copy.numGlyphs = n;
      }

      return copy;
   }

   inline QFixed effectiveAdvance(int item) const {
      return (advances[item] + QFixed::fromFixed(justifications[item].space_18d6)) * ! attributes[item].dontPrint;
   }

   inline void clear(int first = 0, int last = -1) {
      if (last == -1) {
         last = numGlyphs;
      }

      if (first == 0 && last == numGlyphs && reinterpret_cast<char *>(offsets + numGlyphs) == reinterpret_cast<char *>(glyphs)) {
         memset(offsets, 0, numGlyphs * SpaceRequired);

      } else {
         const int num = last - first;
         memset(offsets + first, 0,        num * sizeof(QFixedPoint));
         memset(glyphs + first, 0,         num * sizeof(glyph_t));
         memset(advances + first, 0,       num * sizeof(QFixed));
         memset(justifications + first, 0, num * sizeof(QGlyphJustification));
         memset(attributes + first, 0,     num * sizeof(QGlyphAttributes));
      }
   }

   inline char *data() {
      return reinterpret_cast<char *>(offsets);
   }

   void grow(char *address, int totalGlyphs);

 public:
   QFixedPoint *offsets;                    // 8 bytes per element
   glyph_t *glyphs;                         // 4 bytes per element
   QFixed *advances;                        // 4 bytes per element

   QGlyphAttributes *attributes;            // 1 byte  per element
   QGlyphJustification *justifications;     // 4 bytes per element

   int numGlyphs;
};

class QVarLengthGlyphLayoutArray : private QVarLengthArray<void *>, public QGlyphLayout
{
   using Array = QVarLengthArray<void *>;

 public:
   QVarLengthGlyphLayoutArray(int totalGlyphs)
      : Array((totalGlyphs * SpaceRequired) / sizeof(void *) + 1)
      , QGlyphLayout(reinterpret_cast<char *>(Array::data()), totalGlyphs) {
      memset(Array::data(), 0, Array::size() * sizeof(void *));
   }

   void resize(int totalGlyphs) {
      Array::resize((totalGlyphs * SpaceRequired) / sizeof(void *) + 1);

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
   void *buffer[(N * SpaceRequired) / sizeof(void *) + 1];
};

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

struct QScriptItem {
   inline QScriptItem()
      : position(0), num_glyphs(0), descent(-1), ascent(-1), leading(-1), width(-1), glyph_data_offset(0)
   {}

   inline QScriptItem(int p, const QScriptAnalysis &a)
      : position(p), analysis(a), num_glyphs(0), descent(-1), ascent(-1), leading(-1), width(-1), glyph_data_offset(0)
   {}

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

struct QScriptLine {
   // created and filled in QTextLine::layout_helper
   QScriptLine()
      : from(0), trailingSpaces(0), length(0), justified(0), gridfitted(0), hasTrailingSpaces(0), leadingIncluded(0)
   {}

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
      return ascent + descent  + (leadingIncluded ?  qMax(QFixed(), leading) : QFixed());
   }

   QFixed base() const {
      return ascent;
   }

   void setDefaultHeight(QTextEngine *eng);
   void operator+=(const QScriptLine &other);
};

Q_DECLARE_TYPEINFO(QScriptLine, Q_PRIMITIVE_TYPE);


inline void QScriptLine::operator+=(const QScriptLine &other)
{
   leading = qMax(leading + ascent, other.leading + other.ascent) - qMax(ascent, other.ascent);
   descent = qMax(descent, other.descent);
   ascent  = qMax(ascent, other.ascent);

   textWidth += other.textWidth;
   length += other.length;
}

class Q_GUI_EXPORT QTextEngine
{
 public:
   enum LayoutState {
      LayoutEmpty,
      InLayout,
      LayoutFailed,
   };

   struct Q_GUI_EXPORT LayoutData {
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

   struct ItemDecoration {
      // for QVector, do not use
      ItemDecoration() {}

      ItemDecoration(qreal x1, qreal x2, qreal y, const QPen &pen)
         : x1(x1), x2(x2), y(y), pen(pen)
      {}

      qreal x1;
      qreal x2;
      qreal y;
      QPen pen;
   };

   typedef QVector<ItemDecoration> ItemDecorationList;

   QTextEngine();
   QTextEngine(const QString &str, const QFont &f);

   ~QTextEngine();

   enum Mode {
      WidthOnly = 0x07
   };

   void clearLineData();
   void invalidate();
   bool isRightToLeft() const;
   void itemize() const;
   void justify(const QScriptLine &si);
   void shape(int item) const;
   void validate() const;

   static bool isRightToLeft(QStringView str);
   static void bidiReorder(int numRuns, const quint8 *levels, int *visualOrder);

   const QCharAttributes *attributes() const;
   QFixed alignLine(const QScriptLine &line);

   QFixed width(int charFrom, int numChars) const;
   glyph_metrics_t boundingBox(int from,  int len) const;
   glyph_metrics_t tightBoundingBox(int from,  int len) const;

   int length(int item) const {
      const QScriptItem &si = layoutData->items[item];
      item++;

      int retval;

      if (item < layoutData->items.size()) {
         retval = layoutData->items[item].position - si.position;

      } else {
         retval = layoutData->string.size() - si.position;
      }

      return retval;
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

   QFontEngine *fontEngine(const QScriptItem &si, QFixed *ascent = nullptr, QFixed *descent = nullptr,
                  QFixed *leading = nullptr) const;

   QFont font(const QScriptItem &si) const;
   inline QFont font() const {
      return fnt;
   }

   inline unsigned short *logClusters(const QScriptItem *si) const {
      return layoutData->logClustersPtr + si->position;
   }

   inline QGlyphLayout availableGlyphs(const QScriptItem *si) const {
      return layoutData->glyphLayout.mid(si->glyph_data_offset);
   }

   inline QGlyphLayout shapedGlyphs(const QScriptItem *si) const {
      return layoutData->glyphLayout.mid(si->glyph_data_offset, si->num_glyphs);
   }

   inline bool ensureSpace(int nGlyphs) const {
      if (layoutData->glyphLayout.numGlyphs - layoutData->used < nGlyphs) {
         return layoutData->reallocate((((layoutData->used + nGlyphs) * 3 / 2 + 15) >> 4) << 4);
      }

      return true;
   }

   void freeMemory();
   int findItem(int strPos, int firstItem = 0) const;

   inline QTextFormatCollection *formatCollection() const {
      if (block.docHandle()) {
         return block.docHandle()->formatCollection();
      }
      return specialData ? specialData->formatCollection.data() : 0;
   }

   QTextCharFormat format(const QScriptItem *si) const;

   inline QAbstractTextDocumentLayout *docLayout() const {
      Q_ASSERT(block.docHandle());
      return block.docHandle()->document()->documentLayout();
   }

   int formatIndex(const QScriptItem *si) const;

   // returns the width of tab at index (in the tabs array) with the tab-start at position x
   QFixed calculateTabWidth(int index, QFixed x) const;

   mutable QScriptLineArray lines;

   QString text;
   mutable QFont fnt;

   QRawFont rawFont;

   QTextBlock block;
   QTextOption option;

   QFixed minWidth;
   QFixed maxWidth;
   QPointF position;
   uint ignoreBidi  : 1;
   uint cacheGlyphs : 1;
   uint stackEngine : 1;
   uint forceJustification : 1;
   uint visualMovement : 1;
   uint delayDecorations: 1;
   uint useRawFont : 1;

   mutable LayoutData *layoutData;

   ItemDecorationList underlineList;
   ItemDecorationList strikeOutList;
   ItemDecorationList overlineList;

   inline bool visualCursorMovement() const {
      return visualMovement || (block.docHandle() && block.docHandle()->defaultCursorMoveStyle == Qt::VisualMoveStyle);
   }

   inline int preeditAreaPosition() const {
      return specialData ? specialData->preeditPosition : -1;
   }
   inline QString preeditAreaText() const {
      return specialData ? specialData->preeditText : QString();
   }
   void setPreeditArea(int position, const QString &text);

   inline bool hasFormats() const {
      return block.docHandle() || (specialData && !specialData->formats.isEmpty());
   }

   inline QVector<QTextLayout::FormatRange> formats() const {
      return specialData ? specialData->formats : QVector<QTextLayout::FormatRange>();
   }

   void setFormats(const QVector<QTextLayout::FormatRange> &formats);

   bool atWordSeparator(int position) const;

   QString elidedText(Qt::TextElideMode mode, const QFixed &width, int flags = 0, int from = 0, int count = -1) const;

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

   void enableDelayDecorations(bool enable = true) {
      delayDecorations = enable;
   }

   void addUnderline(QPainter *painter, const QLineF &line);
   void addStrikeOut(QPainter *painter, const QLineF &line);
   void addOverline(QPainter *painter, const QLineF &line);

   void drawDecorations(QPainter *painter);
   void clearDecorations();
   void adjustUnderlines();

 private:
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

   static void init(QTextEngine *e);

   struct SpecialData {
      int preeditPosition;
      QString preeditText;
      QVector<QTextLayout::FormatRange> formats;
      QVector<QTextCharFormat> resolvedFormats;

      // only used when no docHandle is available
      QScopedPointer<QTextFormatCollection> formatCollection;
   };
   SpecialData *specialData;

   void indexFormats();
   void resolveFormats() const;

   void addItemDecoration(QPainter *painter, const QLineF &line, ItemDecorationList *decorationList);
   void adjustUnderlines(ItemDecorationList::iterator start, ItemDecorationList::iterator end,
      qreal underlinePos, qreal penWidth);

   void drawItemDecorationList(QPainter *painter, const ItemDecorationList &decorationList);
   void setBoundary(int strPos) const;
   void addRequiredBoundaries() const;
   void shapeText(int item) const;

#if ! defined(CS_BUILDING_CUPS)
   int shapeTextWithHarfbuzz(const QScriptItem &si, QStringView str,
      QFontEngine *fontEngine, const QVector<uint> &itemBoundaries,
      bool kerningEnabled,  bool hasLetterSpacing) const;
#endif

   int endOfLine(int lineNum);
   int beginningOfLine(int lineNum);

   int getClusterLength(unsigned short *logClusters, const QCharAttributes *attributes, int from, int to,
      int glyph_pos, int *start);
};

class QStackTextEngine : public QTextEngine
{
 public:
   constexpr static const int MemSize = 256 * 40 / sizeof(void *);

   QStackTextEngine(const QString &string, const QFont &f);

   LayoutData _layoutData;
   void *_memory[MemSize];
};

struct QTextLineItemIterator {
   QTextLineItemIterator(QTextEngine *eng, int lineNum, const QPointF &pos = QPointF(),
      const QTextLayout::FormatRange *_selection = nullptr);

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
   const QScriptLine &line;
   QScriptItem *si;

   const int lineNum;
   const int lineEnd;
   const int firstItem;
   const int lastItem;
   const int nItems;

   int logicalItem;
   int item;
   int itemLength;

   int glyphsStart;
   int glyphsEnd;
   int itemStart;
   int itemEnd;

   QFixed itemWidth;

   QVarLengthArray<int> visualOrder;

   const QTextLayout::FormatRange *selection;
};

#endif
