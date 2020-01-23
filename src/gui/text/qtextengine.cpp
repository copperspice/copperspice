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

#include <qtextengine_p.h>

#include <qfont.h>
#include <qstring.h>
#include <qvarlengtharray.h>

#include <qapplication.h>
#include <qabstracttextdocumentlayout.h>
#include <qdebug.h>
#include <qtextformat.h>
#include <qtextlayout.h>
#include <qtextboundaryfinder.h>

#include <qfont_p.h>
#include <qfontengine_p.h>
#include <qrawfont_p.h>
#include <qtextformat_p.h>
#include <qunicodetables_p.h>
#include <qtextdocument_p.h>

#include <algorithm>
#include <stdlib.h>

static const float smallCapsFraction = 0.7f;

namespace {

enum JustificationClass {
   Justification_Prohibited      = 0,   // Justification can not be applied after this glyph
   Justification_Arabic_Space    = 1,   // This glyph represents a space inside arabic text
   Justification_Character       = 2,   // Inter-character justification point follows this glyph
   Justification_Space           = 4,   // This glyph represents a blank outside an Arabic run
   Justification_Arabic_Normal   = 7,   // Normal Middle-Of-Word glyph that connects to the right (begin)
   Justification_Arabic_Waw      = 8,   // Next character is final form of Waw/Ain/Qaf/Feh
   Justification_Arabic_BaRa     = 9,   // Next two characters are Ba + Ra/Ya/AlefMaksura
   Justification_Arabic_Alef     = 10,  // Next character is final form of Alef/Tah/Lam/Kaf/Gaf
   Justification_Arabic_HahDal   = 11,  // Next character is final form of Hah/Dal/Teh Marbuta
   Justification_Arabic_Seen     = 12,  // Initial or medial form of Seen/Sad
   Justification_Arabic_Kashida  = 13   // User-inserted Kashida(U+0640)
};

struct QJustificationPoint {
   int type;
   QFixed kashidaWidth;
   QGlyphLayout glyph;
};

// Helper class used in QTextEngine::itemize
// keep it out here to allow us to keep supporting various compilers.

class Itemizer
{
 public:
   Itemizer(const QString &string, const QScriptAnalysis *analysis, QScriptItemArray &items)
      : m_string(string), m_analysis(analysis), m_items(items), m_splitter(0) {
   }

   ~Itemizer() {
      delete m_splitter;
   }

   /// generate the script items
   /// The caps parameter is used to choose the algoritm of splitting text and assiging roles to the textitems
   void generate(int start, int length, QFont::Capitalization caps) {

      if (caps == QFont::SmallCaps) {
         generateScriptItemsSmallCaps(start, length);

      } else if (caps == QFont::Capitalize) {
         generateScriptItemsCapitalize(start, length);

      } else if (caps != QFont::MixedCase) {
         generateScriptItemsAndChangeCase(start, length,
            caps == QFont::AllLowercase ? QScriptAnalysis::Lowercase : QScriptAnalysis::Uppercase);
      } else {
         generateScriptItems(start, length);
      }
   }

 private:
   enum { MaxItemLength = 4096 };

   void generateScriptItemsAndChangeCase(int start, int length, QScriptAnalysis::Flags flags) {
      generateScriptItems(start, length);

      if (m_items.isEmpty()) {
         // the next loop will not work in that case
         return;
      }

      QScriptItemArray::iterator iter = m_items.end();
      do {
         iter--;

         if (iter->analysis.flags < QScriptAnalysis::LineOrParagraphSeparator) {
            iter->analysis.flags = flags;
         }

      } while (iter->position > start);
   }

   void generateScriptItems(int start, int length) {
      if (! length) {
         return;
      }

      const int end = start + length;

      for (int i = start + 1; i < end; ++i) {

         if (m_analysis[i].bidiLevel == m_analysis[start].bidiLevel
            && m_analysis[i].flags == m_analysis[start].flags
            && (m_analysis[i].script == m_analysis[start].script || m_string[i] == QChar('.'))
            && m_analysis[i].flags < QScriptAnalysis::SpaceTabOrObject
            && i - start < MaxItemLength) {
            continue;
         }

         m_items.append(QScriptItem(start, m_analysis[start]));
         start = i;
      }

      m_items.append(QScriptItem(start, m_analysis[start]));
   }

   void generateScriptItemsCapitalize(int start, int length) {
      if (! length) {
         return;
      }

      if (! m_splitter) {
         m_splitter = new QTextBoundaryFinder(QTextBoundaryFinder::Word, m_string);
      }

      m_splitter->setPosition(start);
      QScriptAnalysis itemAnalysis = m_analysis[start];

      if (m_splitter->boundaryReasons() & QTextBoundaryFinder::StartOfItem) {
         itemAnalysis.flags = QScriptAnalysis::Uppercase;
         m_splitter->toNextBoundary();
      }

      const int end = start + length;
      for (int i = start + 1; i < end; ++i) {

         bool atWordBoundary = false;

         if (i == m_splitter->position()) {
            if (m_splitter->boundaryReasons() & QTextBoundaryFinder::StartOfItem
               && m_analysis[i].flags < QScriptAnalysis::TabOrObject) {
               atWordBoundary = true;
            }

            m_splitter->toNextBoundary();
         }

         if (m_analysis[i] == itemAnalysis
            && m_analysis[i].flags < QScriptAnalysis::TabOrObject
            && ! atWordBoundary
            && i - start < MaxItemLength) {
            continue;
         }

         m_items.append(QScriptItem(start, itemAnalysis));
         start = i;
         itemAnalysis = m_analysis[start];

         if (atWordBoundary) {
            itemAnalysis.flags = QScriptAnalysis::Uppercase;
         }
      }
      m_items.append(QScriptItem(start, itemAnalysis));
   }

   void generateScriptItemsSmallCaps(int start, int length) {
      if (! length) {
         return;
      }

      QString::const_iterator iter = m_string.begin() + start;

      bool prev_lower = iter->isLower();
      const int end   = start + length;

      // split text into parts that are already uppercase and parts that are lowercase, and mark the latter to be uppercased later.
      for (int i = start + 1; i < end; ++i) {
         ++iter;

         bool isLower = iter->isLower();

         if ((m_analysis[i] == m_analysis[start]) && m_analysis[i].flags < QScriptAnalysis::TabOrObject &&
            isLower == prev_lower && i - start < MaxItemLength) {
            continue;
         }

         m_items.append(QScriptItem(start, m_analysis[start]));

         if (prev_lower) {
            m_items.last().analysis.flags = QScriptAnalysis::SmallCaps;
         }

         start      = i;
         prev_lower = isLower;
      }

      m_items.append(QScriptItem(start, m_analysis[start]));

      if (prev_lower) {
         m_items.last().analysis.flags = QScriptAnalysis::SmallCaps;
      }
   }

   const QString &m_string;
   const QScriptAnalysis *const m_analysis;
   QScriptItemArray &m_items;
   QTextBoundaryFinder *m_splitter;
};

} // end namespace


// The BiDi algorithm
#define BIDI_DEBUG 0

#if (BIDI_DEBUG >= 1)

#include <iostream>

static const char *directions[] = {
   "DirL", "DirR", "DirEN", "DirES", "DirET", "DirAN", "DirCS", "DirB", "DirS", "DirWS", "DirON",
   "DirLRE", "DirLRO", "DirAL", "DirRLE", "DirRLO", "DirPDF", "DirNSM", "DirBN",
   "DirLRI", "DirRLI", "DirFSI", "DirPDI"
};

#endif

struct QBidiStatus {
   QBidiStatus() {
      eor = QChar::DirON;
      lastStrong = QChar::DirON;
      last = QChar:: DirON;
      dir = QChar::DirON;
   }

   QChar::Direction eor;
   QChar::Direction lastStrong;
   QChar::Direction last;
   QChar::Direction dir;
};

enum { MaxBidiLevel = 61 };

struct QBidiControl {
   inline QBidiControl(bool rtl)
      : cCtx(0), base(rtl ? 1 : 0), level(rtl ? 1 : 0), override(false) {}

   inline void embed(bool rtl, bool o = false) {
      unsigned int toAdd = 1;
      if ((level % 2 != 0) == rtl ) {
         ++toAdd;
      }
      if (level + toAdd <= MaxBidiLevel) {
         ctx[cCtx].level = level;
         ctx[cCtx].override = override;
         cCtx++;
         override = o;
         level += toAdd;
      }
   }
   inline bool canPop() const {
      return cCtx != 0;
   }

   inline void pdf() {
      Q_ASSERT(cCtx);
      --cCtx;
      level = ctx[cCtx].level;
      override = ctx[cCtx].override;
   }

   inline QChar::Direction basicDirection() const {
      return (base ? QChar::DirR : QChar:: DirL);
   }

   inline unsigned int baseLevel() const {
      return base;
   }

   inline QChar::Direction direction() const {
      return ((level % 2) ? QChar::DirR : QChar:: DirL);
   }

   struct {
      unsigned int level;
      bool override;
   } ctx[MaxBidiLevel];
   unsigned int cCtx;
   const unsigned int base;
   unsigned int level;
   bool override;
};

static void appendItems(QScriptAnalysis *analysis, int &start, int &stop, const QBidiControl &control, QChar::Direction dir)
{
   if (start > stop) {
      return;
   }

   int level = control.level;

   if (dir != QChar::DirON && !control.override) {
      // add level of run (cases I1 & I2)
      if (level % 2) {
         if (dir == QChar::DirL || dir == QChar::DirAN || dir == QChar::DirEN) {
            level++;
         }
      } else {
         if (dir == QChar::DirR) {
            level++;
         } else if (dir == QChar::DirAN || dir == QChar::DirEN) {
            level += 2;
         }
      }
   }

#if (BIDI_DEBUG >= 1)
   qDebug("new run: dir=%s from %d, to %d level = %d override=%d", directions[dir], start, stop, level, control.override);
#endif

   QScriptAnalysis *s = analysis + start;
   const QScriptAnalysis *e = analysis + stop;

   while (s <= e) {
      s->bidiLevel = level;
      ++s;
   }
   ++stop;
   start = stop;
}

static QChar::Direction skipBoundryNeutrals(QScriptAnalysis *analysis, QStringView str, int &sor, int &eor, QBidiControl &control)
{
   QChar::Direction dir = control.basicDirection();
   int level = sor > 0 ? analysis[sor - 1].bidiLevel : control.level;
   int len   = str.size();

   while (sor < len) {
      dir =  str[sor].direction();

      // Keep skipping DirBN as if it does not exist
      if (dir != QChar::DirBN) {
         break;
      }

      analysis[sor++].bidiLevel = level;
   }

   eor = sor;

   if (eor == len) {
      dir = control.basicDirection();
   }

   return dir;
}

// creates the next QScript items.
static bool bidiItemize(QTextEngine *engine, QScriptAnalysis *analysis, QBidiControl &control)
{
   bool rightToLeft = (control.basicDirection() == 1);
   bool hasBidi = rightToLeft;

#if BIDI_DEBUG >= 2
   qDebug() << "bidiItemize: rightToLeft=" << rightToLeft << engine->layoutData->string;
#endif

   int sor = 0;
   int eor = -1;

   QStringView str_view(engine->layoutData->string);
   int length = str_view.size();

   int current = 0;

   QChar::Direction dir = rightToLeft ? QChar::DirR : QChar::DirL;
   QBidiStatus status;

   QChar::Direction sdir = str_view[0].direction();

   if (sdir != QChar::DirL && sdir != QChar::DirR && sdir != QChar::DirEN && sdir != QChar::DirAN) {
      sdir = QChar::DirON;
   } else {
      dir = QChar::DirON;
   }

   status.eor        = sdir;
   status.lastStrong = rightToLeft ? QChar::DirR : QChar::DirL;
   status.last       = status.lastStrong;
   status.dir        = sdir;

   while (current <= length) {
      QChar::Direction dirCurrent;

      if (current == length) {
         dirCurrent = control.basicDirection();
      } else {
         dirCurrent = str_view[current].direction();
      }

#if (BIDI_DEBUG >= 2)
      //         qDebug() << "pos=" << current << " dir=" << directions[dir]
      //                  << " current=" << directions[dirCurrent] << " last=" << directions[status.last]
      //                  << " eor=" << eor << '/' << directions[status.eor]
      //                  << " sor=" << sor << " lastStrong="
      //                  << directions[status.lastStrong]
      //                  << " level=" << (int)control.level << " override=" << (bool)control.override;
#endif

      switch (dirCurrent) {

         // embedding and overrides (X1-X9 in the BiDi specs)
         case QChar::DirRLE:
         case QChar::DirRLO:
         case QChar::DirLRE:
         case QChar::DirLRO: {
            bool rtl = (dirCurrent == QChar::DirRLE || dirCurrent == QChar::DirRLO);
            hasBidi |= rtl;
            bool override = (dirCurrent == QChar::DirLRO || dirCurrent == QChar::DirRLO);

            unsigned int level = control.level + 1;
            if ((level % 2 != 0) == rtl) {
               ++level;
            }

            if (level < MaxBidiLevel) {
               eor = current - 1;
               appendItems(analysis, sor, eor, control, dir);
               eor = current;
               control.embed(rtl, override);
               QChar::Direction edir = (rtl ? QChar::DirR : QChar::DirL);
               dir = status.eor = edir;
               status.lastStrong = edir;
            }
            break;
         }
         case QChar::DirPDF: {
            if (control.canPop()) {
               if (dir != control.direction()) {
                  eor = current - 1;
                  appendItems(analysis, sor, eor, control, dir);
                  dir = control.direction();
               }

               eor = current;
               appendItems(analysis, sor, eor, control, dir);
               control.pdf();
               dir = QChar::DirON;
               status.eor = QChar::DirON;
               status.last = control.direction();

               if (control.override) {
                  dir = control.direction();
               } else {
                  dir = QChar::DirON;
               }
               status.lastStrong = control.direction();
            }
            break;
         }

         // strong types
         case QChar::DirL:
            if (dir == QChar::DirON) {
               dir = QChar::DirL;
            }
            switch (status.last) {
               case QChar::DirL:
                  eor = current;
                  status.eor = QChar::DirL;
                  break;

               case QChar::DirR:
               case QChar::DirAL:
               case QChar::DirEN:
               case QChar::DirAN:
                  if (eor >= 0) {
                     appendItems(analysis, sor, eor, control, dir);
                     status.eor = dir = skipBoundryNeutrals(analysis, str_view, sor, eor, control);
                  } else {
                     eor = current;
                     status.eor = dir;
                  }
                  break;

               case QChar::DirES:
               case QChar::DirET:
               case QChar::DirCS:
               case QChar::DirBN:
               case QChar::DirB:
               case QChar::DirS:
               case QChar::DirWS:
               case QChar::DirON:
                  if (dir != QChar::DirL) {
                     //last stuff takes embedding dir
                     if (control.direction() == QChar::DirR) {
                        if (status.eor != QChar::DirR) {
                           // AN or EN
                           appendItems(analysis, sor, eor, control, dir);
                           status.eor = QChar::DirON;
                           dir = QChar::DirR;
                        }
                        eor = current - 1;
                        appendItems(analysis, sor, eor, control, dir);
                        status.eor = dir = skipBoundryNeutrals(analysis, str_view, sor, eor, control);

                     } else {
                        if (status.eor != QChar::DirL) {
                           appendItems(analysis, sor, eor, control, dir);
                           status.eor = QChar::DirON;
                           dir = QChar::DirL;
                        } else {
                           eor = current;
                           status.eor = QChar::DirL;
                           break;
                        }
                     }
                  } else {
                     eor = current;
                     status.eor = QChar::DirL;
                  }
               default:
                  break;
            }
            status.lastStrong = QChar::DirL;
            break;

         case QChar::DirAL:
         case QChar::DirR:
            hasBidi = true;
            if (dir == QChar::DirON) {
               dir = QChar::DirR;
            }

            switch (status.last) {
               case QChar::DirL:
               case QChar::DirEN:
               case QChar::DirAN:
                  if (eor >= 0) {
                     appendItems(analysis, sor, eor, control, dir);
                  }
               // fall through
               case QChar::DirR:
               case QChar::DirAL:
                  dir = QChar::DirR;
                  eor = current;
                  status.eor = QChar::DirR;
                  break;

               case QChar::DirES:
               case QChar::DirET:
               case QChar::DirCS:
               case QChar::DirBN:
               case QChar::DirB:
               case QChar::DirS:
               case QChar::DirWS:
               case QChar::DirON:
                  if (status.eor != QChar::DirR && status.eor != QChar::DirAL) {
                     //last stuff takes embedding dir
                     if (control.direction() == QChar::DirR
                        || status.lastStrong == QChar::DirR || status.lastStrong == QChar::DirAL) {
                        appendItems(analysis, sor, eor, control, dir);
                        dir = QChar::DirR;
                        status.eor = QChar::DirON;
                        eor = current;
                     } else {
                        eor = current - 1;
                        appendItems(analysis, sor, eor, control, dir);
                        dir = QChar::DirR;
                        status.eor = QChar::DirON;
                     }
                  } else {
                     eor = current;
                     status.eor = QChar::DirR;
                  }
               default:
                  break;
            }
            status.lastStrong = dirCurrent;
            break;

         // weak types:

         case QChar::DirNSM:
            if (eor == current - 1) {
               eor = current;
            }
            break;

         case QChar::DirEN:
            // if last strong was AL change EN to AN
            if (status.lastStrong != QChar::DirAL) {
               if (dir == QChar::DirON) {
                  if (status.lastStrong == QChar::DirL) {
                     dir = QChar::DirL;
                  } else {
                     dir = QChar::DirEN;
                  }
               }

               switch (status.last) {
                  case QChar::DirET:
                     if (status.lastStrong == QChar::DirR || status.lastStrong == QChar::DirAL) {
                        appendItems(analysis, sor, eor, control, dir);
                        status.eor = QChar::DirON;
                        dir = QChar::DirAN;
                     }
                  // fall through
                  case QChar::DirEN:
                  case QChar::DirL:
                     eor = current;
                     status.eor = dirCurrent;
                     break;

                  case QChar::DirR:
                  case QChar::DirAL:
                  case QChar::DirAN:
                     if (eor >= 0) {
                        appendItems(analysis, sor, eor, control, dir);
                     } else {
                        eor = current;
                     }
                     status.eor = QChar::DirEN;
                     dir = QChar::DirAN;
                     break;

                  case QChar::DirES:
                  case QChar::DirCS:
                     if (status.eor == QChar::DirEN || dir == QChar::DirAN) {
                        eor = current;
                        break;
                     }

                  case QChar::DirBN:
                  case QChar::DirB:
                  case QChar::DirS:
                  case QChar::DirWS:
                  case QChar::DirON:
                     if (status.eor == QChar::DirR) {
                        // neutrals go to R
                        eor = current - 1;
                        appendItems(analysis, sor, eor, control, dir);
                        dir = QChar::DirON;
                        status.eor = QChar::DirEN;
                        dir = QChar::DirAN;
                     } else if (status.eor == QChar::DirL ||
                        (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
                        eor = current;
                        status.eor = dirCurrent;
                     } else {
                        // numbers on both sides, neutrals get right to left direction
                        if (dir != QChar::DirL) {
                           appendItems(analysis, sor, eor, control, dir);
                           dir = QChar::DirON;
                           status.eor = QChar::DirON;
                           eor = current - 1;
                           dir = QChar::DirR;
                           appendItems(analysis, sor, eor, control, dir);
                           dir = QChar::DirON;
                           status.eor = QChar::DirON;
                           dir = QChar::DirAN;
                        } else {
                           eor = current;
                           status.eor = dirCurrent;
                        }
                     }
                  default:
                     break;
               }
               break;
            }

         case QChar::DirAN:
            hasBidi = true;
            dirCurrent = QChar::DirAN;
            if (dir == QChar::DirON) {
               dir = QChar::DirAN;
            }
            switch (status.last) {
               case QChar::DirL:
               case QChar::DirAN:
                  eor = current;
                  status.eor = QChar::DirAN;
                  break;
               case QChar::DirR:
               case QChar::DirAL:
               case QChar::DirEN:
                  if (eor >= 0) {
                     appendItems(analysis, sor, eor, control, dir);
                  } else {
                     eor = current;
                  }
                  dir = QChar::DirAN;
                  status.eor = QChar::DirAN;
                  break;
               case QChar::DirCS:
                  if (status.eor == QChar::DirAN) {
                     eor = current;
                     break;
                  }
               case QChar::DirES:
               case QChar::DirET:
               case QChar::DirBN:
               case QChar::DirB:
               case QChar::DirS:
               case QChar::DirWS:
               case QChar::DirON:
                  if (status.eor == QChar::DirR) {
                     // neutrals go to R
                     eor = current - 1;
                     appendItems(analysis, sor, eor, control, dir);
                     status.eor = QChar::DirAN;
                     dir = QChar::DirAN;

                  } else if (status.eor == QChar::DirL ||
                     (status.eor == QChar::DirEN && status.lastStrong == QChar::DirL)) {
                     eor = current;
                     status.eor = dirCurrent;

                  } else {
                     // numbers on both sides, neutrals get right to left direction
                     if (dir != QChar::DirL) {
                        appendItems(analysis, sor, eor, control, dir);
                        status.eor = QChar::DirON;
                        eor = current - 1;
                        dir = QChar::DirR;
                        appendItems(analysis, sor, eor, control, dir);
                        status.eor = QChar::DirAN;
                        dir = QChar::DirAN;
                     } else {
                        eor = current;
                        status.eor = dirCurrent;
                     }
                  }
               default:
                  break;
            }
            break;

         case QChar::DirES:
         case QChar::DirCS:
            break;

         case QChar::DirET:
            if (status.last == QChar::DirEN) {
               dirCurrent = QChar::DirEN;
               eor = current;
               status.eor = dirCurrent;
            }
            break;

         // boundary neutrals should be ignored
         case QChar::DirBN:
            break;

         // neutrals
         case QChar::DirB:
            // ### what do we do with newline and paragraph separators that come to here?
            break;

         case QChar::DirS:
            // ### implement rule L1
            break;

         case QChar::DirWS:
         case QChar::DirON:
            break;
         default:
            break;
      }

      if (current >= (int)length) {
         break;
      }

      // set status.last as needed.
      switch (dirCurrent) {
         case QChar::DirET:
         case QChar::DirES:
         case QChar::DirCS:
         case QChar::DirS:
         case QChar::DirWS:
         case QChar::DirON:
            switch (status.last) {
               case QChar::DirL:
               case QChar::DirR:
               case QChar::DirAL:
               case QChar::DirEN:
               case QChar::DirAN:
                  status.last = dirCurrent;
                  break;

               default:
                  status.last = QChar::DirON;
            }
            break;

         case QChar::DirNSM:
         case QChar::DirBN:
            // ignore these
            break;

         case QChar::DirLRO:
         case QChar::DirLRE:
            status.last = QChar::DirL;
            break;

         case QChar::DirRLO:
         case QChar::DirRLE:
            status.last = QChar::DirR;
            break;

         case QChar::DirEN:
            if (status.last == QChar::DirL) {
               status.last = QChar::DirL;
               break;
            }
         // fall through

         default:
            status.last = dirCurrent;
      }

      ++current;
   }

#if (BIDI_DEBUG >= 1)
   qDebug() << "reached end of line current=" << current << ", eor=" << eor;
#endif
   eor = current - 1; // remove dummy char

   if (sor <= eor) {
      appendItems(analysis, sor, eor, control, dir);
   }

   return hasBidi;
}

void QTextEngine::bidiReorder(int numItems, const quint8 *levels, int *visualOrder)
{
   // first find highest and lowest levels
   quint8 levelLow  = 128;
   quint8 levelHigh = 0;
   int i = 0;

   while (i < numItems) {
      //printf("level = %d\n", r->level);
      if (levels[i] > levelHigh) {
         levelHigh = levels[i];
      }
      if (levels[i] < levelLow) {
         levelLow = levels[i];
      }
      i++;
   }

   // implements reordering of the line (L2 according to BiDi spec):
   // L2. From the highest level found in the text to the lowest odd level on each line,
   // reverse any contiguous sequence of characters that are at that level or higher.

   // reversing is only done up to the lowest odd level
   if (! (levelLow % 2)) {
      levelLow++;
   }

   int count = numItems - 1;
   for (i = 0; i < numItems; i++) {
      visualOrder[i] = i;
   }

   while (levelHigh >= levelLow) {
      int i = 0;

      while (i < count) {
         while (i < count && levels[i] < levelHigh) {
            i++;
         }

         int start = i;
         while (i <= count && levels[i] >= levelHigh) {
            i++;
         }
         int end = i - 1;

         if (start != end) {
            //qDebug() << "reversing from " << start << " to " << end;
            for (int j = 0; j < (end - start + 1) / 2; j++) {
               int tmp = visualOrder[start + j];
               visualOrder[start + j] = visualOrder[end - j];
               visualOrder[end - j] = tmp;
            }
         }
         i++;
      }
      levelHigh--;
   }
}

static inline void qt_getDefaultJustificationOpportunities(QStringView str,
                  QGlyphLayout glyphs, const ushort *logClusters, int spaceAs)
{
   auto iter  = str.begin();
   uint index = 0;

   while (iter != str.end()) {

      QChar ch = *iter;

      int position = logClusters[index];

      // skip rest of  glyph
      do {
         ++index;
         ++iter;
      } while (iter != str.end() && logClusters[index] == position);

      do {
         ++position;
      } while (position < glyphs.numGlyphs && ! glyphs.attributes[position].clusterStart);

     --position;

      // justification opportunity at the end of cluster
      if (ch.isLetterOrNumber()) {
         glyphs.attributes[position].justification = Justification_Character;

      } else if (ch.isSpace()) {
         glyphs.attributes[position].justification = spaceAs;

      }
   }
}

static inline void qt_getJustificationOpportunities(QStringView str, const QScriptItem &si, QGlyphLayout g, ushort *log_clusters)
{
   Q_ASSERT(! str.empty() && g.numGlyphs > 0);

   for (int glyph_pos = 0; glyph_pos < g.numGlyphs; ++glyph_pos) {
      g.attributes[glyph_pos].justification = Justification_Prohibited;
   }

   int spaceAs;

   switch (si.analysis.script) {
      case QChar::Script_Arabic:
      case QChar::Script_Syriac:
      case QChar::Script_Nko:
      case QChar::Script_Mandaic:
      case QChar::Script_Mongolian:
      case QChar::Script_PhagsPa:
      case QChar::Script_Manichaean:
      case QChar::Script_PsalterPahlavi:
         // same as default but inter character justification takes precedence
         spaceAs = Justification_Arabic_Space;
         break;

      case QChar::Script_Tibetan:
      case QChar::Script_Hiragana:
      case QChar::Script_Katakana:
      case QChar::Script_Bopomofo:
      case QChar::Script_Han:
         // same as default but inter character justification is the only option
         spaceAs = Justification_Character;
         break;

      default:
         spaceAs = Justification_Space;
         break;
   }

   qt_getDefaultJustificationOpportunities(str, g, log_clusters, spaceAs);
}

// shape all the items that intersect with the line, taking tab widths into account to find out what
// text actually fits in the line
void QTextEngine::shapeLine(const QScriptLine &line)
{
   int item = findItem(line.from);

   if (item == -1) {
      return;
   }

   QFixed x;
   bool first = true;
   const int end = findItem(line.from + line.length + line.trailingSpaces - 1, item);

   for ( ; item <= end; ++item) {
      QScriptItem &si = layoutData->items[item];

      if (si.analysis.flags == QScriptAnalysis::Tab) {
         ensureSpace(1);
         si.width = calculateTabWidth(item, x);
      } else {
         shape(item);
      }

      if (first && si.position != line.from) {
         // that means our x position has to be offset

         QGlyphLayout glyphs = shapedGlyphs(&si);
         Q_ASSERT(line.from > si.position);

         for (int i = line.from - si.position - 1; i >= 0; i--) {
            x -= glyphs.effectiveAdvance(i);
         }
      }
      first = false;

      x += si.width;
   }
}

void QTextEngine::shapeText(int item) const
{
   Q_ASSERT(item < layoutData->items.size());

   QScriptItem &si = layoutData->items[item];

   if (si.num_glyphs) {
      return;
   }

   si.width = 0;
   si.glyph_data_offset = layoutData->used;

   const int itemLength = length(item);

   QStringView str = layoutData->string.midView(si.position, itemLength);
   QString casedString;

   if (si.analysis.flags && si.analysis.flags <= QScriptAnalysis::SmallCaps) {

      if (si.analysis.flags == QScriptAnalysis::Lowercase) {
         casedString = str.toLower();

      } else {
         casedString = str.toUpper();

      }

      str = casedString;
   }

   if (! ensureSpace(itemLength)) {
      // error case, goes away when layoutData struc is cleaned up
      return;
   }

   QFontEngine *fontEngine = this->fontEngine(si, &si.ascent, &si.descent, &si.leading);

   // split up the item into parts that come from 3 different font engines
   // array[k + 0] == index in string
   // array[k + 1] == index in glyphs
   // array[k + 2] == engine index

   QVector<uint> itemBoundaries;
   itemBoundaries.reserve(24);

   if (fontEngine->type() == QFontEngine::Multi) {
      // ask the font engine to find out which glyphs (as an index in the specific font)
      // to use for the text in one item
      QGlyphLayout initialGlyphs = availableGlyphs(&si);

      int nGlyphs = initialGlyphs.numGlyphs;
      QFontEngine::ShaperFlags shaperFlags(QFontEngine::GlyphIndicesOnly);

      if (! fontEngine->stringToCMap(str, &initialGlyphs, &nGlyphs, shaperFlags)) {
         // error case
      }

      uint lastEngine = ~0u;
      for (int i = 0, glyph_pos = 0; i < itemLength; ++i, ++glyph_pos) {
         const uint engineIdx = initialGlyphs.glyphs[glyph_pos] >> 24;

         if (lastEngine != engineIdx) {
            itemBoundaries.append(i);
            itemBoundaries.append(glyph_pos);
            itemBoundaries.append(engineIdx);

            if (engineIdx != 0) {
               QFontEngine *actualFontEngine = static_cast<QFontEngineMulti *>(fontEngine)->engine(engineIdx);
               si.ascent  = qMax(actualFontEngine->ascent(),  si.ascent);
               si.descent = qMax(actualFontEngine->descent(), si.descent);
               si.leading = qMax(actualFontEngine->leading(), si.leading);
            }

            lastEngine = engineIdx;
         }
      }

   } else {
      itemBoundaries.append(0);
      itemBoundaries.append(0);
      itemBoundaries.append(0);
   }

   bool kerningEnabled;
   bool letterSpacingIsAbsolute;

   QFixed letterSpacing;
   QFixed wordSpacing;

   if (useRawFont) {
      QTextCharFormat f = format(&si);
      kerningEnabled    = f.fontKerning();
      wordSpacing       = QFixed::fromReal(f.fontWordSpacing());
      letterSpacing     = QFixed::fromReal(f.fontLetterSpacing());

      letterSpacingIsAbsolute = true;

   } else {
      QFont font = this->font(si);

      kerningEnabled          = font.d->kerning;
      letterSpacingIsAbsolute = font.d->letterSpacingIsAbsolute;
      letterSpacing           = font.d->letterSpacing;
      wordSpacing             = font.d->wordSpacing;

      if (letterSpacingIsAbsolute && letterSpacing.value()) {
         letterSpacing *= font.d->dpi / qt_defaultDpiY();
      }
   }

   si.num_glyphs = shapeTextWithHarfbuzz(si, str, fontEngine, itemBoundaries, kerningEnabled, letterSpacing != 0);

   if (si.num_glyphs == 0) {
      // emerald - report shaping error
      return;
   }

   layoutData->used += si.num_glyphs;

   QGlyphLayout glyphs = shapedGlyphs(&si);
   qt_getJustificationOpportunities(str, si, glyphs, logClusters(&si));

   if (letterSpacing != 0) {
      for (int i = 1; i < si.num_glyphs; ++i) {

         if (glyphs.attributes[i].clusterStart) {

            if (letterSpacingIsAbsolute) {
               glyphs.advances[i - 1] += letterSpacing;
            } else {
               QFixed &advance = glyphs.advances[i - 1];
               advance += (letterSpacing - 100) * advance / 100;
            }
         }
      }

      if (letterSpacingIsAbsolute) {
         glyphs.advances[si.num_glyphs - 1] += letterSpacing;
      } else {
         QFixed &advance = glyphs.advances[si.num_glyphs - 1];
         advance += (letterSpacing - 100) * advance / 100;
      }
   }

   if (wordSpacing != 0) {
      for (int i = 0; i < si.num_glyphs; ++i) {
         if (glyphs.attributes[i].justification == Justification_Space ||
            glyphs.attributes[i].justification == Justification_Arabic_Space) {

            // word spacing only gets added once to a consecutive run of spaces (see CSS spec)

            if (i + 1 == si.num_glyphs || (glyphs.attributes[i + 1].justification != Justification_Space
                  && glyphs.attributes[i + 1].justification != Justification_Arabic_Space)) {
               glyphs.advances[i] += wordSpacing;
            }
         }
      }
   }

   for (int i = 0; i < si.num_glyphs; ++i) {
      si.width += glyphs.advances[i] * ! glyphs.attributes[i].dontPrint;
   }
}

int QTextEngine::shapeTextWithHarfbuzz(const QScriptItem &si, QStringView str, QFontEngine *fontEngine,
   const QVector<uint> &itemBoundaries, bool kerningEnabled, bool hasLetterSpacing) const
{
   uint glyphs_shaped = 0;
   auto strLength     = str.size();

   hb_buffer_t *buffer = hb_buffer_create();
   hb_buffer_set_unicode_funcs(buffer, cs_get_unicode_funcs());
   hb_buffer_pre_allocate(buffer, strLength);

   if (! hb_buffer_allocation_successful(buffer)) {
      hb_buffer_destroy(buffer);
      return 0;
   }

   hb_segment_properties_t props = HB_SEGMENT_PROPERTIES_DEFAULT;
   props.direction = si.analysis.bidiLevel % 2 ? HB_DIRECTION_RTL : HB_DIRECTION_LTR;

   QChar::Script script = QChar::Script(si.analysis.script);
   props.script = cs_script_to_hb_script(script);

   uint current_cluster  = 0;
   uint expected_cluster = 0;
   int offset_cluster    = 0;

   int delta = 0;

   for (int k = 0; k < itemBoundaries.size(); k += 3) {

      const uint item_pos    = itemBoundaries[k];
      const uint item_length = (k + 4 < itemBoundaries.size() ? itemBoundaries[k + 3] : strLength) - item_pos;
      const uint engineIdx   = itemBoundaries[k + 2];

      QFontEngine *actualFontEngine = fontEngine;

      if (fontEngine->type() == QFontEngine::Multi) {
         actualFontEngine = static_cast<QFontEngineMulti *>(fontEngine)->engine(engineIdx);
      }

      // prepare buffer
      hb_buffer_clear_contents(buffer);

      QStringView strBlock = str.mid(item_pos, item_length);
      hb_buffer_add_utf8(buffer, strBlock.charData(), strBlock.size_storage(), 0, strBlock.size_storage());

      hb_buffer_set_segment_properties(buffer, &props);
      hb_buffer_guess_segment_properties(buffer);

      uint buffer_flags = HB_BUFFER_FLAG_DEFAULT;

      // symbol encoding used to encode various values in the 32..255 character code range,
      // and thus might override U+00AD [SHY]; avoid hiding default ignorables

      if (actualFontEngine->symbol) {
         buffer_flags |= HB_BUFFER_FLAG_PRESERVE_DEFAULT_IGNORABLES;
      }

      hb_buffer_set_flags(buffer, hb_buffer_flags_t(buffer_flags));

      // shape
      {
         hb_font_t *hb_font = cs_font_get_for_engine(actualFontEngine);
         Q_ASSERT(hb_font);

         cs_font_set_use_design_metrics(hb_font, option.useDesignMetrics() ? uint(QFontEngine::DesignMetrics) : 0);

         // ligatures are incompatible with custom letter spacing, so when a letter spacing is set
         // disable them for writing systems where they are purely cosmetic

         bool scriptRequiresOpenType = ((script >= QChar::Script_Syriac && script <= QChar::Script_Sinhala)
               || script == QChar::Script_Khmer || script == QChar::Script_Nko);

         bool dontLigate = hasLetterSpacing && ! scriptRequiresOpenType;

         const hb_feature_t features[5] = {
            { HB_TAG('k', 'e', 'r', 'n'), !! kerningEnabled, 0, uint(-1) },
            { HB_TAG('l', 'i', 'g', 'a'), ! dontLigate, 0, uint(-1) },
            { HB_TAG('c', 'l', 'i', 'g'), ! dontLigate, 0, uint(-1) },
            { HB_TAG('d', 'l', 'i', 'g'), ! dontLigate, 0, uint(-1) },
            { HB_TAG('h', 'l', 'i', 'g'), ! dontLigate, 0, uint(-1) }
         };

         const int num_features = dontLigate ? 5 : 1;

         const char *const *shaper_list = nullptr;

#if defined(Q_OS_DARWIN)
         // What is behind QFontEngine::FaceData::user_data is not compatible between different font engines,
         // specifically functions in hb-coretext.cc would run into undefined behavior with data
         // from non-CoreText engine. The other shapers work with that engine just fine.

         if (actualFontEngine->type() != QFontEngine::Mac) {
            static const char *s_shaper_list_without_coretext[] = {
               "graphite2",
               "ot",
               "fallback",
               nullptr
            };
            shaper_list = s_shaper_list_without_coretext;
         }
#endif

         bool shapedOk = hb_shape_full(hb_font, buffer, features, num_features, shaper_list);
         if (! shapedOk) {
            hb_buffer_destroy(buffer);
            return 0;
         }

         if (HB_DIRECTION_IS_BACKWARD(props.direction)) {
            hb_buffer_reverse(buffer);
         }
      }

      const uint num_glyphs = hb_buffer_get_length(buffer);

      // multiple code points were turned into a single glyph
      const uint spaceForClusters = qMax(num_glyphs, strBlock.size());

      // ensure there is enough space for shaped glyphs and metrics
      if (num_glyphs == 0 || ! ensureSpace(glyphs_shaped + spaceForClusters)) {
         hb_buffer_destroy(buffer);
         return 0;
      }

      // fetch the shaped glyphs and metrics
      QGlyphLayout g       = availableGlyphs(&si).mid(glyphs_shaped, num_glyphs);
      ushort *log_clusters = logClusters(&si) + item_pos;

      uint hb_len;

      hb_glyph_info_t *infos         = hb_buffer_get_glyph_infos(buffer, &hb_len);
      hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer, &hb_len);

      uint infos_position = 0;
      uint merge_count    = 0;

      for (QChar ch : strBlock) {
         // save the glyph id
         g.glyphs[infos_position]    = infos[infos_position].codepoint;

         g.advances[infos_position]  = QFixed::fromFixed(positions[infos_position].x_advance);
         g.offsets[infos_position].x = QFixed::fromFixed(positions[infos_position].x_offset);
         g.offsets[infos_position].y = QFixed::fromFixed(positions[infos_position].y_offset);

         // hide characters that should normally be invisible
         switch (ch.unicode()) {
            case QChar::LineFeed:
            case 0x000c:                      // FormFeed
            case QChar::CarriageReturn:
            case QChar::LineSeparator:
            case QChar::ParagraphSeparator:
               g.attributes[infos_position].dontPrint = true;
               break;

            case QChar::SoftHyphen:
               if (! actualFontEngine->symbol) {
                  // U+00AD [SOFT HYPHEN] is a default ignorable codepoint,
                  // replace its glyph and metrics with ones for
                  // U+002D [HYPHEN-MINUS] and make it visible if it appears at line-break
                  g.glyphs[infos_position] = actualFontEngine->glyphIndex('-');

                  if (g.glyphs[infos_position] != 0) {
                     QGlyphLayout tmp = g.mid(infos_position, 1);
                     actualFontEngine->recalcAdvances(&tmp, 0);
                  }
                  g.attributes[infos_position].dontPrint = true;
               }
               break;

            default:
               break;
         }

         current_cluster = infos[infos_position].cluster + offset_cluster;

         if (current_cluster > expected_cluster ) {
            // one

            // multiple code points were turned into a single glyph
            log_clusters[infos_position + merge_count] = log_clusters[infos_position + merge_count - 1];

            ++merge_count;

         } else if (current_cluster < expected_cluster) {
            // two
            g.attributes[infos_position].clusterStart = false;

            // code point is part of the same cluster
            log_clusters[infos_position + merge_count] = log_clusters[infos_position + merge_count - 1];

            ++infos_position;

         } else {
            // three
            g.attributes[infos_position].clusterStart = true;

            // save 'value' to logClusters
            log_clusters[infos_position + merge_count] = current_cluster - merge_count - delta;

            ++infos_position;
         }

         if (infos_position == num_glyphs)  {
            // ran out of glyphs, all done
            break;
         }

         if (ch < 0x80) {
            expected_cluster += 1;

         } else if (ch < 0x800) {
            expected_cluster += 2;
            delta += 1;

         } else if (ch < 0x10000) {
            expected_cluster += 3;
            delta += 2;

         } else {
            expected_cluster += 4;
            delta += 3;
         }
      }

      // keep for the next possible loop
      offset_cluster = expected_cluster;

      // pad log clusters when there are more code points than glyphs
      while (infos_position + merge_count < item_length) {
         log_clusters[infos_position + merge_count] = log_clusters[infos_position + merge_count - 1];
         ++merge_count;
      }

      if (engineIdx != 0) {
         for (quint32 i = 0; i < num_glyphs; ++i) {
            g.glyphs[i] |= (engineIdx << 24);
         }
      }

#ifdef Q_OS_DARWIN
      if (actualFontEngine->type() == QFontEngine::Mac) {

         if (actualFontEngine->fontDef.stretch != 100) {
            QFixed stretch = QFixed(int(actualFontEngine->fontDef.stretch)) / QFixed(100);

            for (uint i = 0; i < num_glyphs; ++i) {
               g.advances[i] *= stretch;
            }
         }
      }
#endif

      if (! actualFontEngine->supportsSubPixelPositions() ||
                  (actualFontEngine->fontDef.styleStrategy & QFont::ForceIntegerMetrics)) {
         for (uint i = 0; i < num_glyphs; ++i) {
            g.advances[i] = g.advances[i].round();
         }
      }

      glyphs_shaped += num_glyphs;
   }

   hb_buffer_destroy(buffer);

   return glyphs_shaped;
}

static inline void moveGlyphData(const QGlyphLayout &destination, const QGlyphLayout &source, int num)
{
   if (num > 0 && destination.glyphs != source.glyphs) {
      memmove(destination.glyphs, source.glyphs, num * sizeof(glyph_t));
   }
}

void QTextEngine::init(QTextEngine *e)
{
   e->ignoreBidi         = false;
   e->cacheGlyphs        = false;
   e->forceJustification = false;
   e->visualMovement     = false;
   e->delayDecorations   = false;

   e->layoutData = 0;
   e->minWidth   = 0;
   e->maxWidth   = 0;

   e->specialData        = 0;
   e->stackEngine        = false;
   e->useRawFont         = false;
}

QTextEngine::QTextEngine()
{
   init(this);
}

QTextEngine::QTextEngine(const QString &str, const QFont &f)
   : text(str), fnt(f)
{
   init(this);
}

QTextEngine::~QTextEngine()
{
   if (! stackEngine) {
      delete layoutData;
   }

   delete specialData;
   resetFontEngineCache();
}

const QCharAttributes *QTextEngine::attributes() const
{
   if (layoutData && layoutData->haveCharAttributes) {
      return (QCharAttributes *) layoutData->memory;
   }

   itemize();

   QStringView tmpStr = layoutData->string;

   if (! ensureSpace(tmpStr.size())) {
      return nullptr;
   }

   QVector<QUnicodeTools::ScriptItem> scriptItems(layoutData->items.size());

   for (int i = 0; i < layoutData->items.size(); ++i) {
      const QScriptItem &si   = layoutData->items[i];
      scriptItems[i].position = si.position;
      scriptItems[i].script   = si.analysis.script;
   }

   QUnicodeTools::initCharAttributes(tmpStr, scriptItems, (QCharAttributes *)layoutData->memory);
   layoutData->haveCharAttributes = true;

   return (QCharAttributes *) layoutData->memory;
}

void QTextEngine::shape(int item) const
{
   if (layoutData->items[item].analysis.flags == QScriptAnalysis::Object) {
      ensureSpace(1);

      if (block.docHandle()) {

         docLayout()->resizeInlineObject(QTextInlineObject(item, const_cast<QTextEngine *>(this)),
            layoutData->items[item].position + block.position(),
            format(&layoutData->items[item]));
      }

   } else if (layoutData->items[item].analysis.flags == QScriptAnalysis::Tab) {
      // set up at least the ascent/descent/leading of the script item for the tab
      fontEngine(layoutData->items[item],
         &layoutData->items[item].ascent,
         &layoutData->items[item].descent,
         &layoutData->items[item].leading);

   } else {
      shapeText(item);
   }
}

static inline void releaseCachedFontEngine(QFontEngine *fontEngine)
{
   if (fontEngine && ! fontEngine->ref.deref()) {
      delete fontEngine;
   }
}

void QTextEngine::resetFontEngineCache()
{
   releaseCachedFontEngine(feCache.prevFontEngine);
   releaseCachedFontEngine(feCache.prevScaledFontEngine);
   feCache.reset();
}

void QTextEngine::invalidate()
{
   freeMemory();
   minWidth = 0;
   maxWidth = 0;

   resetFontEngineCache();
}

void QTextEngine::clearLineData()
{
   lines.clear();
}

void QTextEngine::validate() const
{
   if (layoutData) {
      return;
   }

   layoutData = new LayoutData();

   if (block.docHandle()) {
      layoutData->string = block.text();

      if (option.flags() & QTextOption::ShowLineAndParagraphSeparators) {
         layoutData->string += block.next().isValid() ? 0xb6 : 0x20;
      }

   } else {
      layoutData->string = text;
   }

   if (specialData && specialData->preeditPosition != -1) {
      layoutData->string.insert(specialData->preeditPosition, specialData->preeditText);
   }
}

void QTextEngine::itemize() const
{
   validate();
   if (layoutData->items.size()) {
      return;
   }

   int length = layoutData->string.size();
   if (! length) {
      return;
   }

   bool ignore = ignoreBidi;
   bool rtl    = isRightToLeft();

   if (! ignore && ! rtl) {
      ignore = true;

      for (QChar c : layoutData->string) {
         if (c >= 0x590) {
            ignore = false;
            break;
         }
      }
   }

   QVarLengthArray<QScriptAnalysis, 4096> scriptAnalysis(length);
   QScriptAnalysis *analysis = scriptAnalysis.data();

   QBidiControl control(rtl);

   if (ignore) {
      memset(analysis, 0, length * sizeof(QScriptAnalysis));

      if (option.textDirection() == Qt::RightToLeft) {
         for (int i = 0; i < length; ++i) {
            analysis[i].bidiLevel = 1;
         }

         layoutData->hasBidi = true;
      }

   } else {
      layoutData->hasBidi = bidiItemize(const_cast<QTextEngine *>(this), analysis, control);
   }

   {
      QVector<QChar::Script> scripts;
      QUnicodeTools::initScripts(layoutData->string, scripts);

      for (int i = 0; i < scripts.length(); ++i) {
         analysis[i].script = scripts.at(i);
      }
   }

   QString tmp;

   for (QChar c : layoutData->string)  {
      tmp.append(c);

      switch (c.unicode()) {
         case QChar::ObjectReplacementCharacter:
            analysis->flags = QScriptAnalysis::Object;
            break;

         case QChar::LineSeparator:
            if (analysis->bidiLevel % 2) {
               --analysis->bidiLevel;
            }

            analysis->flags = QScriptAnalysis::LineOrParagraphSeparator;

            if (option.flags() & QTextOption::ShowLineAndParagraphSeparators) {
               // visual line separator
               tmp.chop(1);
               tmp.append(UCHAR('\u21B5'));
            }
            break;

         case QChar::Tabulation:
            analysis->flags     = QScriptAnalysis::Tab;
            analysis->bidiLevel = control.baseLevel();
            break;

         case QChar::Space:
         case QChar::Nbsp:
            if (option.flags() & QTextOption::ShowTabsAndSpaces) {
               analysis->flags     = QScriptAnalysis::Space;
               analysis->bidiLevel = control.baseLevel();
               break;
            }

         // fall through
         default:
            analysis->flags  = QScriptAnalysis::None;
            break;
      }

      ++analysis;
   }

   if (layoutData->string != tmp)  {
      layoutData->string = std::move(tmp);
   }

   if (option.flags() & QTextOption::ShowLineAndParagraphSeparators) {
      // to exclude it from width
      (analysis - 1)->flags = QScriptAnalysis::LineOrParagraphSeparator;
   }

   analysis = scriptAnalysis.data();

   // emerald - simulated HB old behavior, enhance

   for (int i = 0; i < length; ++i) {
      switch (analysis[i].script) {
         case QChar::Script_Latin:
         case QChar::Script_Hiragana:
         case QChar::Script_Katakana:
         case QChar::Script_Bopomofo:
         case QChar::Script_Han:
            analysis[i].script = QChar::Script_Common;
            break;

         default:
            break;
      }
   }

   Itemizer itemizer(layoutData->string, scriptAnalysis.data(), layoutData->items);
   const QTextDocumentPrivate *p = block.docHandle();

   if (p) {
      SpecialData *s = specialData;

      QTextDocumentPrivate::FragmentIterator it = p->find(block.position());

      // -1 to omit the block separator char
      QTextDocumentPrivate::FragmentIterator end = p->find(block.position() + block.length() - 1);
      int format = it.value()->format;

      int prevPosition = 0;
      int position     = prevPosition;

      while (true) {
         const QTextFragmentData *const frag = it.value();

         if (it == end || format != frag->format) {

            if (s && position >= s->preeditPosition) {
               position += s->preeditText.length();
               s = 0;
            }

            Q_ASSERT(position <= length);

            QFont::Capitalization capitalization =
               formatCollection()->charFormat(format).hasProperty(QTextFormat::FontCapitalization)
               ? formatCollection()->charFormat(format).fontCapitalization()
               : formatCollection()->defaultFont().capitalization();

            itemizer.generate(prevPosition, position - prevPosition, capitalization);

            if (it == end) {
               if (position < length) {
                  itemizer.generate(position, length - position, capitalization);
               }

               break;
            }

            format = frag->format;
            prevPosition = position;
         }

         position += frag->size_array[0];
         ++it;
      }

   } else {

      if (useRawFont && specialData) {
         int lastIndex = 0;

         for (int i = 0; i < specialData->formats.size(); ++i) {
            const QTextLayout::FormatRange &range = specialData->formats.at(i);
            const QTextCharFormat &format = range.format;

            if (format.hasProperty(QTextFormat::FontCapitalization)) {
               itemizer.generate(lastIndex, range.start - lastIndex, QFont::MixedCase);
               itemizer.generate(range.start, range.length, format.fontCapitalization());
               lastIndex = range.start + range.length;
            }
         }

         itemizer.generate(lastIndex, length - lastIndex, QFont::MixedCase);

      } else {
         itemizer.generate(0, length, static_cast<QFont::Capitalization> (fnt.d->capital));
      }

   }

   addRequiredBoundaries();
   resolveFormats();
}

bool QTextEngine::isRightToLeft() const
{
   switch (option.textDirection()) {
      case Qt::LeftToRight:
         return false;

      case Qt::RightToLeft:
         return true;

      default:
         break;
   }

   if (! layoutData) {
      itemize();
   }

   // this places the cursor in the right position depending on the keyboard layout
   if (layoutData->string.isEmpty()) {
      return QGuiApplication::inputMethod()->inputDirection() == Qt::RightToLeft;
   }

   return QTextEngine::isRightToLeft(layoutData->string);
}

bool QTextEngine::isRightToLeft(QStringView str)
{
   for (QChar c : str) {

      switch (c.direction()) {
         case QChar::DirL:
            return false;

         case QChar::DirR:
         case QChar::DirAL:
            return true;

         default:
            break;
      }
   }

   return false;
}

int QTextEngine::findItem(int strPos, int firstItem) const
{
   itemize();

   if (strPos < 0 || strPos >= layoutData->string.size() || firstItem < 0) {
      return -1;
   }

   int left  = firstItem + 1;
   int right = layoutData->items.size() - 1;

   while (left <= right) {
      int middle = ((right - left) / 2) + left;

      if (strPos > layoutData->items[middle].position) {
         left = middle + 1;
      } else if (strPos < layoutData->items[middle].position) {
         right = middle - 1;
      } else {
         return middle;
      }
   }

   return right;
}

QFixed QTextEngine::width(int from, int len) const
{
   itemize();

   QFixed w = 0;

   for (int i = 0; i < layoutData->items.size(); i++) {
      const QScriptItem *si = layoutData->items.constData() + i;
      int pos = si->position;
      int ilen = length(i);

      // qDebug("item %d: from %d len %d", i, pos, ilen);

      if (pos >= from + len) {
         break;
      }

      if (pos + ilen > from) {
         if (! si->num_glyphs) {
            shape(i);
         }

         if (si->analysis.flags == QScriptAnalysis::Object) {
            w += si->width;
            continue;

         } else if (si->analysis.flags == QScriptAnalysis::Tab) {
            w += calculateTabWidth(i, w);
            continue;
         }


         QGlyphLayout glyphs = shapedGlyphs(si);
         unsigned short *logClusters = this->logClusters(si);

         //             fprintf(stderr, "  logclusters:");
         //             for (int k = 0; k < ilen; k++)
         //                 fprintf(stderr, " %d", logClusters[k]);
         //             fprintf(stderr, "\n");
         // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
         int charFrom = from - pos;
         if (charFrom < 0) {
            charFrom = 0;
         }

         int glyphStart = logClusters[charFrom];
         if (charFrom > 0 && logClusters[charFrom - 1] == glyphStart)
            while (charFrom < ilen && logClusters[charFrom] == glyphStart) {
               charFrom++;
            }

         if (charFrom < ilen) {
            glyphStart = logClusters[charFrom];

            int charEnd = from + len - 1 - pos;
            if (charEnd >= ilen) {
               charEnd = ilen - 1;
            }

            int glyphEnd = logClusters[charEnd];
            while (charEnd < ilen && logClusters[charEnd] == glyphEnd) {
               charEnd++;
            }
            glyphEnd = (charEnd == ilen) ? si->num_glyphs : logClusters[charEnd];

            //                 qDebug("char: start=%d end=%d / glyph: start = %d, end = %d", charFrom, charEnd, glyphStart, glyphEnd);
            for (int i = glyphStart; i < glyphEnd; i++) {
               w += glyphs.advances[i] * !glyphs.attributes[i].dontPrint;
            }
         }
      }
   }
   //     qDebug("   --> w= %d ", w);
   return w;
}

glyph_metrics_t QTextEngine::boundingBox(int from,  int len) const
{
   itemize();

   glyph_metrics_t gm;

   for (int i = 0; i < layoutData->items.size(); i++) {
      const QScriptItem *si = layoutData->items.constData() + i;

      int pos  = si->position;
      int ilen = length(i);
      if (pos > from + len) {
         break;
      }

      if (pos + ilen > from) {
         if (! si->num_glyphs) {
            shape(i);
         }

         if (si->analysis.flags == QScriptAnalysis::Object) {
            gm.width += si->width;
            continue;
         } else if (si->analysis.flags == QScriptAnalysis::Tab) {
            gm.width += calculateTabWidth(i, gm.width);
            continue;
         }

         unsigned short *logClusters = this->logClusters(si);
         QGlyphLayout glyphs = shapedGlyphs(si);

         // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
         int charFrom = from - pos;
         if (charFrom < 0) {
            charFrom = 0;
         }

         int glyphStart = logClusters[charFrom];
         if (charFrom > 0 && logClusters[charFrom - 1] == glyphStart)
            while (charFrom < ilen && logClusters[charFrom] == glyphStart) {
               charFrom++;
            }

         if (charFrom < ilen) {
            QFontEngine *fe = fontEngine(*si);
            glyphStart = logClusters[charFrom];
            int charEnd = from + len - 1 - pos;
            if (charEnd >= ilen) {
               charEnd = ilen - 1;
            }

            int glyphEnd = logClusters[charEnd];
            while (charEnd < ilen && logClusters[charEnd] == glyphEnd) {
               charEnd++;
            }

            glyphEnd = (charEnd == ilen) ? si->num_glyphs : logClusters[charEnd];
            if (glyphStart <= glyphEnd ) {
               glyph_metrics_t m = fe->boundingBox(glyphs.mid(glyphStart, glyphEnd - glyphStart));
               gm.x = qMin(gm.x, m.x + gm.xoff);
               gm.y = qMin(gm.y, m.y + gm.yoff);
               gm.width = qMax(gm.width, m.width + gm.xoff);
               gm.height = qMax(gm.height, m.height + gm.yoff);
               gm.xoff += m.xoff;
               gm.yoff += m.yoff;
            }
         }
      }
   }
   return gm;
}

glyph_metrics_t QTextEngine::tightBoundingBox(int from,  int len) const
{
   itemize();

   glyph_metrics_t gm;

   for (int i = 0; i < layoutData->items.size(); i++) {
      const QScriptItem *si = layoutData->items.constData() + i;
      int pos  = si->position;
      int ilen = length(i);

      if (pos > from + len) {
         break;
      }

      if (pos + len > from) {
         if (!si->num_glyphs) {
            shape(i);
         }

         unsigned short *logClusters = this->logClusters(si);
         QGlyphLayout glyphs = shapedGlyphs(si);

         // do the simple thing for now and give the first glyph in a cluster the full width, all other ones 0.
         int charFrom = from - pos;
         if (charFrom < 0) {
            charFrom = 0;
         }

         int glyphStart = logClusters[charFrom];
         if (charFrom > 0 && logClusters[charFrom - 1] == glyphStart)
            while (charFrom < ilen && logClusters[charFrom] == glyphStart) {
               charFrom++;
            }

         if (charFrom < ilen) {
            glyphStart = logClusters[charFrom];
            int charEnd = from + len - 1 - pos;
            if (charEnd >= ilen) {
               charEnd = ilen - 1;
            }

            int glyphEnd = logClusters[charEnd];
            while (charEnd < ilen && logClusters[charEnd] == glyphEnd) {
               charEnd++;
            }

            glyphEnd = (charEnd == ilen) ? si->num_glyphs : logClusters[charEnd];
            if (glyphStart <= glyphEnd ) {
               QFontEngine *fe = fontEngine(*si);
               glyph_metrics_t m = fe->tightBoundingBox(glyphs.mid(glyphStart, glyphEnd - glyphStart));
               gm.x = qMin(gm.x, m.x + gm.xoff);
               gm.y = qMin(gm.y, m.y + gm.yoff);
               gm.width = qMax(gm.width, m.width + gm.xoff);
               gm.height = qMax(gm.height, m.height + gm.yoff);
               gm.xoff += m.xoff;
               gm.yoff += m.yoff;
            }
         }
      }
   }
   return gm;
}

QFont QTextEngine::font(const QScriptItem &si) const
{
   QFont font = fnt;
   if (hasFormats()) {
      QTextCharFormat f = format(&si);
      font = f.font();

      if (block.docHandle() && block.docHandle()->layout()) {
         // Make sure we get the right dpi on printers
         QPaintDevice *pdev = block.docHandle()->layout()->paintDevice();
         if (pdev) {
            font = QFont(font, pdev);
         }

      } else {
         font = font.resolve(fnt);
      }

      QTextCharFormat::VerticalAlignment valign = f.verticalAlignment();
      if (valign == QTextCharFormat::AlignSuperScript || valign == QTextCharFormat::AlignSubScript) {
         if (font.pointSize() != -1) {
            font.setPointSize((font.pointSize() * 2) / 3);
         } else {
            font.setPixelSize((font.pixelSize() * 2) / 3);
         }
      }
   }

   if (si.analysis.flags == QScriptAnalysis::SmallCaps) {
      font = font.d->smallCapsFont();
   }

   return font;
}

QTextEngine::FontEngineCache::FontEngineCache()
{
   reset();
}

//we cache the previous results of this function, as calling it numerous times with the same effective
//input is common (and hard to cache at a higher level)
QFontEngine *QTextEngine::fontEngine(const QScriptItem &si, QFixed *ascent, QFixed *descent, QFixed *leading) const
{
   QFontEngine *engine = 0;
   QFontEngine *scaledEngine = 0;
   int script = si.analysis.script;

   QFont font = fnt;

   if (useRawFont && rawFont.isValid()) {
      if (feCache.prevFontEngine && feCache.prevFontEngine->type() == QFontEngine::Multi && feCache.prevScript == script) {
         engine = feCache.prevFontEngine;

      } else {
         engine = QFontEngineMulti::createMultiFontEngine(rawFont.d->fontEngine, script);
         feCache.prevFontEngine = engine;
         feCache.prevScript = script;
         engine->ref.ref();

         if (feCache.prevScaledFontEngine) {
            releaseCachedFontEngine(feCache.prevScaledFontEngine);
            feCache.prevScaledFontEngine = 0;
         }
      }

      if (si.analysis.flags == QScriptAnalysis::SmallCaps) {
         if (feCache.prevScaledFontEngine) {
            scaledEngine = feCache.prevScaledFontEngine;
         } else {
            QFontEngine *scEngine = rawFont.d->fontEngine->cloneWithSize(smallCapsFraction * rawFont.pixelSize());
            scEngine->ref.ref();
            scaledEngine = QFontEngineMulti::createMultiFontEngine(scEngine, script);
            scaledEngine->ref.ref();
            feCache.prevScaledFontEngine = scaledEngine;

            // If scEngine is not ref'ed by scaledEngine, make sure it is deallocated and not leaked.
            if (!scEngine->ref.deref()) {
               delete scEngine;
            }

         }
      }

   } else {

      if (hasFormats()) {
         if (feCache.prevFontEngine && feCache.prevPosition == si.position && feCache.prevLength == length(&si) &&
            feCache.prevScript == script) {
            engine = feCache.prevFontEngine;
            scaledEngine = feCache.prevScaledFontEngine;

         } else {
            QTextCharFormat f = format(&si);
            font = f.font();

            if (block.docHandle() && block.docHandle()->layout()) {
               // Make sure we get the right dpi on printers
               QPaintDevice *pdev = block.docHandle()->layout()->paintDevice();
               if (pdev) {
                  font = QFont(font, pdev);
               }
            } else {
               font = font.resolve(fnt);
            }
            engine = font.d->engineForScript(script);

            if (engine) {
               engine->ref.ref();
            }

            QTextCharFormat::VerticalAlignment valign = f.verticalAlignment();
            if (valign == QTextCharFormat::AlignSuperScript || valign == QTextCharFormat::AlignSubScript) {
               if (font.pointSize() != -1) {
                  font.setPointSize((font.pointSize() * 2) / 3);
               } else {
                  font.setPixelSize((font.pixelSize() * 2) / 3);
               }
               scaledEngine = font.d->engineForScript(script);
            }
            if (scaledEngine) {
               scaledEngine->ref.ref();
            }
            if (feCache.prevFontEngine) {
               releaseCachedFontEngine(feCache.prevFontEngine);
            }
            feCache.prevFontEngine = engine;


            if (feCache.prevScaledFontEngine) {
               releaseCachedFontEngine(feCache.prevScaledFontEngine);
            }
            feCache.prevScaledFontEngine = scaledEngine;
            feCache.prevScript = script;
            feCache.prevPosition = si.position;
            feCache.prevLength = length(&si);
         }

      } else {
         if (feCache.prevFontEngine && feCache.prevScript == script && feCache.prevPosition == -1) {
            engine = feCache.prevFontEngine;

         } else {
            engine = font.d->engineForScript(script);
            if (engine) {
               engine->ref.ref();
            }

            if (feCache.prevFontEngine) {
               releaseCachedFontEngine(feCache.prevFontEngine);
            }
            feCache.prevFontEngine = engine;
            feCache.prevScript = script;
            feCache.prevPosition = -1;
            feCache.prevLength = -1;
            feCache.prevScaledFontEngine = 0;
         }
      }

      if (si.analysis.flags == QScriptAnalysis::SmallCaps) {
         QFontPrivate *p = font.d->smallCapsFontPrivate();
         scaledEngine = p->engineForScript(script);
      }
   }

   if (ascent) {
      *ascent = engine->ascent();
      *descent = engine->descent();
      *leading = engine->leading();
   }

   if (scaledEngine) {
      return scaledEngine;
   }

   return engine;
}


static void set(QJustificationPoint *point, int type, const QGlyphLayout &glyph, QFontEngine *fe)
{
   point->type = type;
   point->glyph = glyph;

   if (type >= Justification_Arabic_Normal) {
      QChar ch(0x640); // Kashida character

      glyph_t kashidaGlyph = fe->glyphIndex(ch.unicode());

      if (kashidaGlyph != 0) {
         QGlyphLayout g;
         g.numGlyphs = 1;
         g.glyphs = &kashidaGlyph;
         g.advances = &point->kashidaWidth;
         fe->recalcAdvances(&g, 0);

         if (point->kashidaWidth == 0) {
            point->type = Justification_Prohibited;
         }
      } else {
         point->type = Justification_Prohibited;
         point->kashidaWidth = 0;
      }
   }
}

void QTextEngine::justify(const QScriptLine &line)
{
   //     qDebug("justify: line.gridfitted = %d, line.justified=%d", line.gridfitted, line.justified);
   if (line.gridfitted && line.justified) {
      return;
   }

   if (! line.gridfitted) {
      // redo layout in device metrics, then adjust
      const_cast<QScriptLine &>(line).gridfitted = true;
   }

   if ((option.alignment() & Qt::AlignHorizontal_Mask) != Qt::AlignJustify) {
      return;
   }

   itemize();

   if (! forceJustification) {
      int end = line.from + (int)line.length + line.trailingSpaces;

      if (end == layoutData->string.size()) {
         return;   // no justification at end of paragraph
      }
      if (end && layoutData->items[findItem(end - 1)].analysis.flags == QScriptAnalysis::LineOrParagraphSeparator) {
         return;   // no justification at the end of an explicitly separated line
      }
   }

   // justify line
   int maxJustify = 0;

   // do not include trailing white spaces when doing justification
   int line_length = line.length;
   const QCharAttributes *tmpAttr = attributes();

   if (! tmpAttr) {
      return;
   }

   tmpAttr += line.from;
   while (line_length && tmpAttr[line_length - 1].whiteSpace) {
      --line_length;
   }

   // subtract one char more, as we can't justfy after the last character
   --line_length;

   if (line_length <= 0) {
      return;
   }

   int firstItem = findItem(line.from);
   int lastItem = findItem(line.from + line_length - 1, firstItem);

   int nItems = (firstItem >= 0 && lastItem >= firstItem) ? (lastItem - firstItem + 1) : 0;

   QVarLengthArray<QJustificationPoint> justificationPoints;
   int nPoints = 0;

   QFixed minKashida = 0x100000;

   // need to do all shaping before we go into the next loop, as we there
   // store pointers to the glyph data that could get reallocated by the shaping process

   for (int i = 0; i < nItems; ++i) {
      QScriptItem &si = layoutData->items[firstItem + i];

      if (! si.num_glyphs) {
         shape(firstItem + i);
      }
   }

   for (int i = 0; i < nItems; ++i) {
      QScriptItem &si = layoutData->items[firstItem + i];

      int kashida_type = Justification_Arabic_Normal;
      int kashida_pos = -1;

      int start = qMax(line.from - si.position, 0);
      int end   = qMin(line.from + line_length - (int)si.position, length(firstItem + i));

      unsigned short *log_clusters = logClusters(&si);

      int gs = log_clusters[start];
      int ge = (end == length(firstItem + i) ? si.num_glyphs : log_clusters[end]);

      Q_ASSERT(ge <= si.num_glyphs);

      const QGlyphLayout g = shapedGlyphs(&si);

      for (int i = gs; i < ge; ++i) {
         g.justifications[i].type = QGlyphJustification::JustifyNone;
         g.justifications[i].nKashidas  = 0;
         g.justifications[i].space_18d6 = 0;

         justificationPoints.resize(nPoints + 3);
         int justification = g.attributes[i].justification;

         switch (justification) {
            case Justification_Prohibited:
               break;

            case Justification_Space:
            // fall through

            case Justification_Arabic_Space:
               if (kashida_pos >= 0) {
                  //                     qDebug("kashida position at %d in word", kashida_pos);
                  set(&justificationPoints[nPoints], kashida_type, g.mid(kashida_pos), fontEngine(si));
                  if (justificationPoints[nPoints].kashidaWidth > 0) {
                     minKashida = qMin(minKashida, justificationPoints[nPoints].kashidaWidth);
                     maxJustify = qMax(maxJustify, justificationPoints[nPoints].type);
                     ++nPoints;
                  }
               }
               kashida_pos = -1;
               kashida_type = Justification_Arabic_Normal;
            // fall through

            case Justification_Character:
               set(&justificationPoints[nPoints++], justification, g.mid(i), fontEngine(si));
               maxJustify = qMax(maxJustify, justification);
               break;

            case Justification_Arabic_Normal  :
            case Justification_Arabic_Waw     :
            case Justification_Arabic_BaRa    :
            case Justification_Arabic_Alef    :
            case Justification_Arabic_HahDal  :
            case Justification_Arabic_Seen    :
            case Justification_Arabic_Kashida :
               if (justification >= kashida_type) {
                  kashida_pos = i;
                  kashida_type = justification;
               }
         }
      }

      if (kashida_pos >= 0) {
         set(&justificationPoints[nPoints], kashida_type, g.mid(kashida_pos), fontEngine(si));

         if (justificationPoints[nPoints].kashidaWidth > 0) {
            minKashida = qMin(minKashida, justificationPoints[nPoints].kashidaWidth);
            maxJustify = qMax(maxJustify, justificationPoints[nPoints].type);
            ++nPoints;
         }
      }
   }

   QFixed leading = leadingSpaceWidth(line);
   QFixed need    = line.width - line.textWidth - leading;

   if (need < 0) {
      // line overflows already
      const_cast<QScriptLine &>(line).justified = true;
      return;
   }

   //  qDebug("doing justification: textWidth=%x, requested=%x, maxJustify=%d", line.textWidth.value(), line.width.value(), maxJustify);
   //  qDebug("     minKashida=%f, need=%f", minKashida.toReal(), need.toReal());

   // distribute in priority order
   if (maxJustify >= Justification_Arabic_Normal) {
      while (need >= minKashida) {
         for (int type = maxJustify; need >= minKashida && type >= Justification_Arabic_Normal; --type) {
            for (int i = 0; need >= minKashida && i < nPoints; ++i) {

               if (justificationPoints[i].type == type && justificationPoints[i].kashidaWidth <= need) {
                  justificationPoints[i].glyph.justifications->nKashidas++;

                  // ############
                  justificationPoints[i].glyph.justifications->space_18d6 += justificationPoints[i].kashidaWidth.value();
                  need -= justificationPoints[i].kashidaWidth;
                  // qDebug("adding kashida type %d with width %x, neednow %x", type, justificationPoints[i].kashidaWidth, need.value());
               }
            }
         }
      }
   }

   Q_ASSERT(need >= 0);
   if (! need) {
      goto end;
   }

   maxJustify = qMin(maxJustify, int(Justification_Space));

   for (int type = maxJustify; need != 0 && type > 0; --type) {
      int n = 0;

      for (int i = 0; i < nPoints; ++i) {
         if (justificationPoints[i].type == type) {
            ++n;
         }
      }

      // qDebug("number of points for justification type %d: %d", type, n);

      if (! n) {
         continue;
      }

      for (int i = 0; i < nPoints; ++i) {
         if (justificationPoints[i].type == type) {
            QFixed add = need / n;
            //                  qDebug("adding %x to glyph %x", add.value(), justificationPoints[i].glyph->glyph);
            justificationPoints[i].glyph.justifications[0].space_18d6 = add.value();
            need -= add;
            --n;
         }
      }

      Q_ASSERT(!need);
   }

end:
   const_cast<QScriptLine &>(line).justified = true;
}

void QScriptLine::setDefaultHeight(QTextEngine *eng)
{
   QFont f;
   QFontEngine *e;

   if (eng->block.docHandle() && eng->block.docHandle()->layout()) {
      f = eng->block.charFormat().font();
      // Make sure we get the right dpi on printers

      QPaintDevice *pdev = eng->block.docHandle()->layout()->paintDevice();
      if (pdev) {
         f = QFont(f, pdev);
      }

      e = f.d->engineForScript(QChar::Script_Common);

   } else {
      e = eng->fnt.d->engineForScript(QChar::Script_Common);
   }

   QFixed other_ascent = e->ascent();
   QFixed other_descent = e->descent();
   QFixed other_leading = e->leading();
   leading = qMax(leading + ascent, other_leading + other_ascent) - qMax(ascent, other_ascent);
   ascent = qMax(ascent, other_ascent);
   descent = qMax(descent, other_descent);
}

QTextEngine::LayoutData::LayoutData()
{
   memory = 0;
   allocated = 0;
   memory_on_stack = false;
   used = 0;
   hasBidi = false;
   layoutState = LayoutEmpty;
   haveCharAttributes = false;
   logClustersPtr = 0;
   available_glyphs = 0;
}

QTextEngine::LayoutData::LayoutData(const QString &str, void **stack_memory, int _allocated)
   : string(str)
{
   allocated = _allocated;

   int strSize = str.size();

   int space_charAttributes = sizeof(QCharAttributes) * strSize / sizeof(void *) + 1;
   int space_logClusters    = sizeof(unsigned short)  * strSize / sizeof(void *) + 1;

   available_glyphs = ((int)allocated - space_charAttributes - space_logClusters) *
      (int)sizeof(void *) / (int)QGlyphLayout::SpaceRequired;

   if (available_glyphs < strSize) {
      // need to allocate on the heap
      allocated = 0;

      memory_on_stack = false;
      memory = 0;
      logClustersPtr = 0;

   } else {
      memory_on_stack = true;
      memory = stack_memory;
      logClustersPtr = (unsigned short *)(memory + space_charAttributes);

      void *m = memory + space_charAttributes + space_logClusters;
      glyphLayout = QGlyphLayout(reinterpret_cast<char *>(m), strSize);
      glyphLayout.clear();
      memset(memory, 0, space_charAttributes * sizeof(void *));
   }

   used = 0;
   hasBidi = false;
   layoutState = LayoutEmpty;
   haveCharAttributes = false;
}

QTextEngine::LayoutData::~LayoutData()
{
   if (!memory_on_stack) {
      free(memory);
   }
   memory = 0;
}

bool QTextEngine::LayoutData::reallocate(int totalGlyphs)
{
   Q_ASSERT(totalGlyphs >= glyphLayout.numGlyphs);

   if (memory_on_stack && available_glyphs >= totalGlyphs) {
      glyphLayout.grow(glyphLayout.data(), totalGlyphs);
      return true;
   }

   int strSize = string.size();

   int space_charAttributes = sizeof(QCharAttributes) * strSize / sizeof(void *) + 1;
   int space_logClusters    = sizeof(unsigned short)    * strSize / sizeof(void *) + 1;
   int space_glyphs         = (totalGlyphs * QGlyphLayout::SpaceRequired) / sizeof(void *) + 2;;

   int newAllocated = space_charAttributes + space_glyphs + space_logClusters;

   // These values can be negative if the length of string/glyphs causes overflow,
   // we can't layout such a long string all at once, so return false here to
   // indicate there is a failure

   if (space_charAttributes < 0 || space_logClusters < 0 || space_glyphs < 0 || newAllocated < allocated) {
      layoutState = LayoutFailed;
      return false;
   }

   void **newMem = memory;
   newMem = (void **)::realloc(memory_on_stack ? 0 : memory, newAllocated * sizeof(void *));

   if (!newMem) {
      layoutState = LayoutFailed;
      return false;
   }

   if (memory_on_stack) {
      memcpy(newMem, memory, allocated * sizeof(void *));
   }
   memory = newMem;
   memory_on_stack = false;

   void **m = memory;
   m += space_charAttributes;
   logClustersPtr = (unsigned short *) m;
   m += space_logClusters;

   const int space_preGlyphLayout = space_charAttributes + space_logClusters;
   if (allocated < space_preGlyphLayout) {
      memset(memory + allocated, 0, (space_preGlyphLayout - allocated)*sizeof(void *));
   }

   glyphLayout.grow(reinterpret_cast<char *>(m), totalGlyphs);

   allocated = newAllocated;
   return true;
}

// grow to the new size, copying the existing data to the new layout
void QGlyphLayout::grow(char *address, int totalGlyphs)
{
   QGlyphLayout oldLayout(address, numGlyphs);
   QGlyphLayout newLayout(address, totalGlyphs);

   if (numGlyphs) {
      // move the existing data
      memmove(newLayout.attributes, oldLayout.attributes, numGlyphs * sizeof(QGlyphAttributes));
      memmove(newLayout.justifications, oldLayout.justifications, numGlyphs * sizeof(QGlyphJustification));
      memmove(newLayout.advances, oldLayout.advances, numGlyphs * sizeof(QFixed));
      memmove(newLayout.glyphs, oldLayout.glyphs, numGlyphs * sizeof(glyph_t));
   }

   // clear the new data
   newLayout.clear(numGlyphs);

   *this = newLayout;
}

void QTextEngine::freeMemory()
{
   if (!stackEngine) {
      delete layoutData;
      layoutData = 0;
   } else {
      layoutData->used = 0;
      layoutData->hasBidi = false;
      layoutData->layoutState = LayoutEmpty;
      layoutData->haveCharAttributes = false;
      layoutData->items.clear();
   }

   if (specialData) {
      specialData->resolvedFormats.clear();
   }

   for (int i = 0; i < lines.size(); ++i) {
      lines[i].justified = 0;
      lines[i].gridfitted = 0;
   }
}

int QTextEngine::formatIndex(const QScriptItem *si) const
{
   if (specialData && !specialData->resolvedFormats.isEmpty()) {
      QTextFormatCollection *collection = formatCollection();
      Q_ASSERT(collection);
      return collection->indexForFormat(specialData->resolvedFormats.at(si - &layoutData->items[0]));
   }

   QTextDocumentPrivate *p = block.docHandle();

   if (! p) {
      return -1;
   }

   int pos = si->position;

   if (specialData && si->position >= specialData->preeditPosition) {
      if (si->position < specialData->preeditPosition + specialData->preeditText.length()) {
         pos = qMax(qMin(block.length(), specialData->preeditPosition) - 1, 0);
      } else {
         pos -= specialData->preeditText.length();
      }
   }

   QTextDocumentPrivate::FragmentIterator it = p->find(block.position() + pos);
   return it.value()->format;
}

QTextCharFormat QTextEngine::format(const QScriptItem *si) const
{
   if (const QTextFormatCollection *collection = formatCollection()) {
      return collection->charFormat(formatIndex(si));
   }

   return QTextCharFormat();
}

void QTextEngine::addRequiredBoundaries() const
{
   if (specialData) {
      for (int i = 0; i < specialData->formats.size(); ++i) {
         const QTextLayout::FormatRange &r = specialData->formats.at(i);
         setBoundary(r.start);
         setBoundary(r.start + r.length);
         //qDebug("adding boundaries %d %d", r.start, r.start+r.length);
      }
   }
}

bool QTextEngine::atWordSeparator(int position) const
{
   const QChar c = layoutData->string.at(position);

   switch (c.unicode()) {
      case '.':
      case ',':
      case '?':
      case '!':
      case '@':
      case '#':
      case '$':
      case ':':
      case ';':
      case '-':
      case '<':
      case '>':
      case '[':
      case ']':
      case '(':
      case ')':
      case '{':
      case '}':
      case '=':
      case '/':
      case '+':
      case '%':
      case '&':
      case '^':
      case '*':
      case '\'':
      case '"':
      case '`':
      case '~':
      case '|':
      case '\\':
         return true;

      default:
         break;
   }

   return false;
}

void QTextEngine::setPreeditArea(int position, const QString &preeditText)
{
   if (preeditText.isEmpty()) {
      if (!specialData) {
         return;
      }
      if (specialData->formats.isEmpty()) {
         delete specialData;
         specialData = 0;
      } else {
         specialData->preeditText = QString();
         specialData->preeditPosition = -1;
      }

   } else {
      if (!specialData) {
         specialData = new SpecialData;
      }
      specialData->preeditPosition = position;
      specialData->preeditText = preeditText;
   }
   invalidate();
   clearLineData();
}

void QTextEngine::setFormats(const QVector<QTextLayout::FormatRange> &formats)
{
   if (formats.isEmpty()) {
      if (!specialData) {
         return;
      }
      if (specialData->preeditText.isEmpty()) {
         delete specialData;
         specialData = 0;
      } else {
         specialData->formats.clear();
      }
   } else {
      if (!specialData) {
         specialData = new SpecialData;
         specialData->preeditPosition = -1;
      }
      specialData->formats = formats;
      indexFormats();
   }
   invalidate();
   clearLineData();
}

void QTextEngine::indexFormats()
{
   QTextFormatCollection *collection = formatCollection();
   if (!collection) {
      Q_ASSERT(!block.docHandle());
      specialData->formatCollection.reset(new QTextFormatCollection);
      collection = specialData->formatCollection.data();
   }

   // replace with shared copies
   for (int i = 0; i < specialData->formats.size(); ++i) {
      QTextCharFormat &format = specialData->formats[i].format;
      format = collection->charFormat(collection->indexForFormat(format));
   }
}

/* These two helper functions are used to determine whether we need to insert a ZWJ character
   between the text that gets truncated and the ellipsis. This is important to get
   correctly shaped results for arabic text.
*/
static inline bool nextCharJoins(const QString &string, int pos)
{
   int strSize = string.size();

   while (pos < strSize && string.at(pos).category() == QChar::Mark_NonSpacing) {
      ++pos;
   }

   if (pos == strSize) {
      return false;
   }

   QChar::JoiningType joining = string.at(pos).joiningType();

   return joining != QChar::Joining_None && joining != QChar::Joining_Transparent;
}

static inline bool prevCharJoins(const QString &string, int pos)
{
   while (pos > 0 && string.at(pos - 1).category() == QChar::Mark_NonSpacing) {
      --pos;
   }

   if (pos == 0) {
      return false;
   }

   QChar::JoiningType joining = string.at(pos - 1).joiningType();

   return joining == QChar::Joining_Dual || joining == QChar::Joining_Causing;
}

static inline bool isRetainableControlCode(QChar c)
{
   return (c.unicode() >= 0x202a && c.unicode() <= 0x202e) // LRE, RLE, PDF, LRO, RLO
      || (c.unicode() >= 0x200e && c.unicode() <= 0x200f) // LRM, RLM
      || (c.unicode() >= 0x2066 && c.unicode() <= 0x2069); // LRM, RLM
}

static QString stringMidRetainingBidiCC(const QString &string, const QString &ellidePrefix,
   const QString &ellideSuffix, int subStringFrom, int subStringTo,
   int midStart, int midLength)
{
   QString prefix;

   for (int i = subStringFrom; i < midStart; ++i) {
      QChar c = string.at(i);
      if (isRetainableControlCode(c)) {
         prefix += c;
      }
   }

   QString suffix;
   for (int i = midStart + midLength; i < subStringTo; ++i) {
      QChar c = string.at(i);

      if (isRetainableControlCode(c)) {
         suffix += c;
      }
   }
   return prefix + ellidePrefix + string.mid(midStart, midLength) + ellideSuffix + suffix;
}

QString QTextEngine::elidedText(Qt::TextElideMode mode, const QFixed &width, int flags, int from, int count) const
{
   if (flags & Qt::TextShowMnemonic) {
      itemize();
      QCharAttributes *attributes = const_cast<QCharAttributes *>(this->attributes());

      if (! attributes) {
         return QString();
      }

      for (int i = 0; i < layoutData->items.size(); ++i) {
         QScriptItem &si = layoutData->items[i];
         if (! si.num_glyphs) {
            shape(i);
         }

         unsigned short *logClusters = this->logClusters(&si);
         QGlyphLayout glyphs = shapedGlyphs(&si);

         const int end = si.position + length(&si);

         for (int i = si.position; i < end - 1; ++i) {
            if (layoutData->string.at(i) == QLatin1Char('&')
               && !attributes[i + 1].whiteSpace && attributes[i + 1].graphemeBoundary) {

               const int gp = logClusters[i - si.position];
               glyphs.attributes[gp].dontPrint = true;

               attributes[i] = attributes[i + 1];

               memset(attributes + i + 1, 0, sizeof(QCharAttributes));

               if (layoutData->string.at(i + 1) == '&') {
                  ++i;
               }
            }
         }
      }
   }

   validate();

   int strSize = layoutData->string.size();

   const int to = count >= 0 && count <= strSize - from ? from + count  : strSize;
   if (mode == Qt::ElideNone || this->width(from, strSize) <= width || to - from <= 1)  {
      return layoutData->string.mid(from, from - to);
   }

   QFixed ellipsisWidth;
   QString ellipsisText;
   {
      QChar ellipsisChar(0x2026);
      QFontEngine *fe = fnt.d->engineForScript(QChar::Script_Common);
      glyph_t glyph  = fe->glyphIndex(ellipsisChar.unicode());

      QGlyphLayout glyphs;
      glyphs.numGlyphs = 1;
      glyphs.glyphs    = &glyph;
      glyphs.advances  = &ellipsisWidth;

      if (glyph != 0) {
         fe->recalcAdvances(&glyphs, 0);
         ellipsisText = ellipsisChar;

      } else {
         glyph = fe->glyphIndex('.');
         if (glyph != 0) {
            fe->recalcAdvances(&glyphs, 0);

            ellipsisWidth *= 3;
            ellipsisText = "...";
         }
      }
   }

   const QFixed availableWidth = width - ellipsisWidth;
   if (availableWidth < 0) {
      return QString();
   }

   const QCharAttributes *attributes = this->attributes();
   if (! attributes) {
      return QString();
   }

   if (mode == Qt::ElideRight) {
      QFixed currentWidth;
      int pos;
      int nextBreak = from;

      do {
         pos = nextBreak;

         ++nextBreak;
         while (nextBreak < strSize && !attributes[nextBreak].graphemeBoundary) {
            ++nextBreak;
         }

         currentWidth += this->width(pos, nextBreak - pos);

      } while (nextBreak < to && currentWidth < availableWidth);

      if (nextCharJoins(layoutData->string, pos)) {
         ellipsisText.prepend(QChar(0x200d) /* ZWJ */);
      }

      return stringMidRetainingBidiCC(layoutData->string, QString(), ellipsisText,
            from, to, from, pos - from);

   } else if (mode == Qt::ElideLeft) {
      QFixed currentWidth;
      int pos;
      int nextBreak = to;

      do {
         pos = nextBreak;
         --nextBreak;

         while (nextBreak > 0 && ! attributes[nextBreak].graphemeBoundary) {
            --nextBreak;
         }

         currentWidth += this->width(nextBreak, pos - nextBreak);

      } while (nextBreak > from && currentWidth < availableWidth);

      if (prevCharJoins(layoutData->string, pos)) {
         ellipsisText.append(QChar(0x200d) /* ZWJ */);
      }

      return stringMidRetainingBidiCC(layoutData->string, ellipsisText, QString(),
            from, to, pos, to - pos);

   } else if (mode == Qt::ElideMiddle) {
      QFixed leftWidth;
      QFixed rightWidth;

      int leftPos = from;
      int nextLeftBreak = from;

      int rightPos       = to;
      int nextRightBreak = to;

      do {
         leftPos  = nextLeftBreak;
         rightPos = nextRightBreak;

         ++nextLeftBreak;
         while (nextLeftBreak < strSize && ! attributes[nextLeftBreak].graphemeBoundary) {
            ++nextLeftBreak;
         }

         --nextRightBreak;
         while (nextRightBreak > from && !attributes[nextRightBreak].graphemeBoundary) {
            --nextRightBreak;
         }

         leftWidth += this->width(leftPos, nextLeftBreak - leftPos);
         rightWidth += this->width(nextRightBreak, rightPos - nextRightBreak);

      } while (nextLeftBreak < to && nextRightBreak > from && leftWidth + rightWidth < availableWidth);

      if (nextCharJoins(layoutData->string, leftPos)) {
         ellipsisText.prepend(QChar(0x200d) /* ZWJ */);
      }

      if (prevCharJoins(layoutData->string, rightPos)) {
         ellipsisText.append(QChar(0x200d) /* ZWJ */);
      }

      return layoutData->string.mid(from, leftPos - from) + ellipsisText + layoutData->string.mid(rightPos, to - rightPos);
   }

   return layoutData->string.mid(from, to - from);
}

void QTextEngine::setBoundary(int strPos) const
{
   const int item = findItem(strPos);

   if (item < 0) {
      return;
   }

   QScriptItem newItem = layoutData->items.at(item);

   if (newItem.position != strPos) {
      newItem.position = strPos;
      layoutData->items.insert(item + 1, newItem);
   }
}

QFixed QTextEngine::calculateTabWidth(int item, QFixed x) const
{
   const QScriptItem &si = layoutData->items[item];

   QFixed dpiScale = 1;
   if (block.docHandle() && block.docHandle()->layout()) {
      QPaintDevice *pdev = block.docHandle()->layout()->paintDevice();

      if (pdev) {
         dpiScale = QFixed::fromReal(pdev->logicalDpiY() / qreal(qt_defaultDpiY()));
      }

   } else {
      dpiScale = QFixed::fromReal(fnt.d->dpi / qreal(qt_defaultDpiY()));
   }

   QList<QTextOption::Tab> tabArray = option.tabs();
   if (! tabArray.isEmpty()) {

      if (isRightToLeft()) {
         // rebase the tabArray positions.
         QList<QTextOption::Tab> newTabs;
         QList<QTextOption::Tab>::iterator iter = tabArray.begin();

         while (iter != tabArray.end()) {
            QTextOption::Tab tab = *iter;

            if (tab.type == QTextOption::LeftTab) {
               tab.type = QTextOption::RightTab;
            } else if (tab.type == QTextOption::RightTab) {
               tab.type = QTextOption::LeftTab;
            }

            newTabs << tab;
            ++iter;
         }
         tabArray = newTabs;
      }

      for (int i = 0; i < tabArray.size(); ++i) {
         QFixed tab = QFixed::fromReal(tabArray[i].position) * dpiScale;

         if (tab > x) {  // this is the tab we need.
            QTextOption::Tab tabSpec = tabArray[i];
            int tabSectionEnd = layoutData->string.count();

            if (tabSpec.type == QTextOption::RightTab || tabSpec.type == QTextOption::CenterTab) {
               // find next tab to calculate the width required.
               tab = QFixed::fromReal(tabSpec.position);

               for (int i = item + 1; i < layoutData->items.count(); i++) {
                  const QScriptItem &item = layoutData->items[i];
                  if (item.analysis.flags == QScriptAnalysis::TabOrObject) {
                     // found it
                     tabSectionEnd = item.position;
                     break;
                  }
               }

            } else if (tabSpec.type == QTextOption::DelimiterTab) {
               // find delimitor character to calculate the width required
               tabSectionEnd = qMax(si.position, layoutData->string.indexOf(tabSpec.delimiter, si.position) + 1);
            }

            if (tabSectionEnd > si.position) {
               QFixed length;

               // Calculate the length of text between this tab and the tabSectionEnd
               for (int i = item; i < layoutData->items.count(); i++) {
                  QScriptItem &item = layoutData->items[i];

                  if (item.position > tabSectionEnd || item.position <= si.position) {
                     continue;
                  }

                  shape(i); // first, lets make sure relevant text is already shaped

                  if (item.analysis.flags == QScriptAnalysis::Object) {
                     length += item.width;
                     continue;
                  }

                  QGlyphLayout glyphs = this->shapedGlyphs(&item);
                  const int end = qMin(item.position + item.num_glyphs, tabSectionEnd) - item.position;
                  for (int i = 0; i < end; i++) {
                     length += glyphs.advances[i] * !glyphs.attributes[i].dontPrint;
                  }

                  if (end + item.position == tabSectionEnd && tabSpec.type == QTextOption::DelimiterTab) {
                     // remove half of matching char
                     length -= glyphs.advances[end] / 2 * !glyphs.attributes[end].dontPrint;
                  }
               }

               switch (tabSpec.type) {
                  case QTextOption::CenterTab:
                     length /= 2;
                  // fall through

                  case QTextOption::DelimiterTab:
                  // fall through

                  case QTextOption::RightTab:
                     tab = QFixed::fromReal(tabSpec.position) * dpiScale - length;

                     if (tab < x) {
                        // default to tab taking no space
                        return QFixed();
                     }
                     break;

                  case QTextOption::LeftTab:
                     break;
               }
            }
            return tab - x;
         }
      }
   }

   QFixed tab = QFixed::fromReal(option.tabStop());

   if (tab <= 0) {
      tab = 80;   // default
   }

   tab *= dpiScale;
   QFixed nextTabPos = ((x / tab).truncate() + 1) * tab;
   QFixed tabWidth = nextTabPos - x;

   return tabWidth;
}

namespace {
class FormatRangeComparatorByStart
{
 public:
   FormatRangeComparatorByStart(const QVector<QTextLayout::FormatRange> &list) : list(list) { }
   bool operator()(int a, int b) {
      return list.at(a).start < list.at(b).start;
   }

 private:
   const QVector<QTextLayout::FormatRange> &list;
};

class FormatRangeComparatorByEnd
{
 public:
   FormatRangeComparatorByEnd(const QVector<QTextLayout::FormatRange> &list) : list(list) { }
   bool operator()(int a, int b) {
      return list.at(a).start + list.at(a).length < list.at(b).start + list.at(b).length;
   }
 private:
   const QVector<QTextLayout::FormatRange> &list;

};

} // namespace

void QTextEngine::resolveFormats() const
{
   if (!specialData || specialData->formats.isEmpty()) {
      return;
   }
   Q_ASSERT(specialData->resolvedFormats.isEmpty());

   QTextFormatCollection *collection = formatCollection();

   QVector<QTextCharFormat> resolvedFormats(layoutData->items.count());

   QVarLengthArray<int, 64> formatsSortedByStart;
   formatsSortedByStart.reserve(specialData->formats.size());
   for (int i = 0; i < specialData->formats.size(); ++i) {
      if (specialData->formats.at(i).length >= 0) {
         formatsSortedByStart.append(i);
      }
   }

   QVarLengthArray<int, 64> formatsSortedByEnd = formatsSortedByStart;
   std::sort(formatsSortedByStart.begin(), formatsSortedByStart.end(),
      FormatRangeComparatorByStart(specialData->formats));

   std::sort(formatsSortedByEnd.begin(), formatsSortedByEnd.end(),
      FormatRangeComparatorByEnd(specialData->formats));

   QVarLengthArray<int, 16>  currentFormats;
   const int *startIt = formatsSortedByStart.constBegin();
   const int *endIt = formatsSortedByEnd.constBegin();

   for (int i = 0; i < layoutData->items.count(); ++i) {
      const QScriptItem *si = &layoutData->items.at(i);
      int end = si->position + length(si);

      while (startIt != formatsSortedByStart.constEnd() &&
         specialData->formats.at(*startIt).start <= si->position) {
         currentFormats.insert(std::upper_bound(currentFormats.begin(), currentFormats.end(), *startIt),
            *startIt);
         ++startIt;
      }
      while (endIt != formatsSortedByEnd.constEnd() &&
         specialData->formats.at(*endIt).start + specialData->formats.at(*endIt).length < end) {
         int *currentFormatIterator = std::lower_bound(currentFormats.begin(), currentFormats.end(), *endIt);
         if (*endIt < *currentFormatIterator) {
            currentFormatIterator = currentFormats.end();
         }

         currentFormats.remove(currentFormatIterator - currentFormats.begin());
         ++endIt;
      }

      QTextCharFormat &format = resolvedFormats[i];
      if (block.docHandle()) {
         // when we have a docHandle, formatIndex might still return a valid index based
         // on the preeditPosition. for all other cases, we cleared the resolved format indices
         format = collection->charFormat(formatIndex(si));
      }

      if (!currentFormats.isEmpty()) {
         for (int cur : currentFormats) {
            const QTextLayout::FormatRange &range = specialData->formats.at(cur);
            Q_ASSERT(range.start <= si->position && range.start + range.length >= end);

            format.merge(range.format);
         }
         format = collection->charFormat(collection->indexForFormat(format)); // get shared copy
      }
   }

   specialData->resolvedFormats = resolvedFormats;
}

QFixed QTextEngine::leadingSpaceWidth(const QScriptLine &line)
{
   if (! line.hasTrailingSpaces || (option.flags() & QTextOption::IncludeTrailingSpaces) || ! isRightToLeft()) {
      return QFixed();
   }

   return width(line.from + line.length, line.trailingSpaces);
}

QFixed QTextEngine::alignLine(const QScriptLine &line)
{
   QFixed x = 0;
   justify(line);

   // if width is QFIXED_MAX that means we used setNumColumns() and that implicitly makes this line left aligned.
   if (! line.justified && line.width != QFIXED_MAX) {
      int align = option.alignment();

      if (align & Qt::AlignJustify && isRightToLeft()) {
         align = Qt::AlignRight;
      }

      if (align & Qt::AlignRight) {
         x = line.width - (line.textAdvance);

      } else if (align & Qt::AlignHCenter) {
         x = (line.width - line.textAdvance) / 2;
      }
   }

   return x;
}

QFixed QTextEngine::offsetInLigature(const QScriptItem *si, int pos, int max, int glyph_pos)
{
   unsigned short *logClusters = this->logClusters(si);
   const QGlyphLayout &glyphs = shapedGlyphs(si);

   int offsetInCluster = 0;
   for (int i = pos - 1; i >= 0; i--) {
      if (logClusters[i] == glyph_pos) {
         offsetInCluster++;
      } else {
         break;
      }
   }

   // in the case that the offset is inside a (multi-character) glyph,
   // interpolate the position.
   if (offsetInCluster > 0) {
      int clusterLength = 0;
      for (int i = pos - offsetInCluster; i < max; i++) {
         if (logClusters[i] == glyph_pos) {
            clusterLength++;
         } else {
            break;
         }
      }

      if (clusterLength) {
         return glyphs.advances[glyph_pos] * offsetInCluster / clusterLength;
      }
   }

   return 0;
}

// Scan in logClusters[from..to-1] for glyph_pos
int QTextEngine::getClusterLength(unsigned short *logClusters, const QCharAttributes *attributes,
   int from, int to, int glyph_pos, int *start)
{
   int clusterLength = 0;

   for (int i = from; i < to; i++) {
      if (logClusters[i] == glyph_pos && attributes[i].graphemeBoundary) {
         if (*start < 0) {
            *start = i;
         }

         clusterLength++;
      } else if (clusterLength) {
         break;
      }
   }

   return clusterLength;
}

int QTextEngine::positionInLigature(const QScriptItem *si, int end,
   QFixed x, QFixed edge, int glyph_pos, bool cursorOnCharacter)
{
   unsigned short *logClusters = this->logClusters(si);
   int clusterStart = -1;
   int clusterLength = 0;

   if (si->analysis.script != QChar::Script_Common && si->analysis.script != QChar::Script_Greek) {
      if (glyph_pos == -1) {
         return si->position + end;
      } else {
         int i;
         for (i = 0; i < end; i++)
            if (logClusters[i] == glyph_pos) {
               break;
            }
         return si->position + i;
      }
   }

   if (glyph_pos == -1 && end > 0) {
      glyph_pos = logClusters[end - 1];

   } else {
      if (x <= edge) {
         glyph_pos--;
      }
   }

   const QCharAttributes *attrs = attributes() + si->position;
   logClusters = this->logClusters(si);
   clusterLength = getClusterLength(logClusters, attrs, 0, end, glyph_pos, &clusterStart);

   if (clusterLength) {
      const QGlyphLayout &glyphs = shapedGlyphs(si);
      QFixed glyphWidth = glyphs.effectiveAdvance(glyph_pos);
      // the approximate width of each individual element of the ligature
      QFixed perItemWidth = glyphWidth / clusterLength;

      if (perItemWidth <= 0) {
         return si->position + clusterStart;
      }

      QFixed left = x > edge ? edge : edge - glyphWidth;
      int n = ((x - left) / perItemWidth).floor().toInt();
      QFixed dist = x - left - n * perItemWidth;
      int closestItem = dist > (perItemWidth / 2) ? n + 1 : n;
      if (cursorOnCharacter && closestItem > 0) {
         closestItem--;
      }

      int pos = clusterStart + closestItem;
      // Jump to the next charStop
      while (pos < end && !attrs[pos].graphemeBoundary) {
         pos++;
      }

      return si->position + pos;
   }

   return si->position + end;
}

int QTextEngine::previousLogicalPosition(int oldPos) const
{
   const QCharAttributes *attrs = attributes();

   int len = block.isValid() ? block.length() - 1 : layoutData->string.length();
   Q_ASSERT(len <= layoutData->string.length());

   if (!attrs || oldPos <= 0 || oldPos > len) {
      return oldPos;
   }


   oldPos--;
   while (oldPos && !attrs[oldPos].graphemeBoundary) {
      oldPos--;
   }
   return oldPos;
}

int QTextEngine::nextLogicalPosition(int oldPos) const
{
   const QCharAttributes *attrs = attributes();

   int len = block.isValid() ? block.length() - 1 : layoutData->string.size();

   Q_ASSERT(len <= layoutData->string.size());

   if (! attrs || oldPos < 0 || oldPos >= len) {
      return oldPos;
   }

   oldPos++;
   while (oldPos < len && !attrs[oldPos].graphemeBoundary) {
      oldPos++;
   }
   return oldPos;
}

int QTextEngine::lineNumberForTextPosition(int pos)
{
   if (! layoutData) {
      itemize();
   }

   if (pos == layoutData->string.size() && lines.size()) {
      return lines.size() - 1;
   }

   for (int i = 0; i < lines.size(); ++i) {
      const QScriptLine &line = lines[i];
      if (line.from + line.length + line.trailingSpaces > pos) {
         return i;
      }
   }
   return -1;
}

void QTextEngine::insertionPointsForLine(int lineNum, QVector<int> &insertionPoints)
{
   QTextLineItemIterator iterator(this, lineNum);

   insertionPoints.reserve(iterator.line.length);
   bool lastLine = lineNum >= lines.size() - 1;

   while (! iterator.atEnd()) {
      const QScriptItem &si = iterator.next();

      int end = iterator.itemEnd;
      if (lastLine && iterator.item == iterator.lastItem) {
         ++end;   // the last item in the last line -> insert eol position
      }
      if (si.analysis.bidiLevel % 2) {
         for (int i = end - 1; i >= iterator.itemStart; --i) {
            insertionPoints.push_back(i);
         }

      } else {
         for (int i = iterator.itemStart; i < end; ++i) {
            insertionPoints.push_back(i);
         }
      }
   }
}

int QTextEngine::endOfLine(int lineNum)
{
   QVector<int> insertionPoints;
   insertionPointsForLine(lineNum, insertionPoints);

   if (insertionPoints.size() > 0) {
      return insertionPoints.last();
   }
   return 0;
}

int QTextEngine::beginningOfLine(int lineNum)
{
   QVector<int> insertionPoints;
   insertionPointsForLine(lineNum, insertionPoints);

   if (insertionPoints.size() > 0) {
      return insertionPoints.first();
   }
   return 0;
}

int QTextEngine::positionAfterVisualMovement(int pos, QTextCursor::MoveOperation op)
{
   itemize();

   bool moveRight = (op == QTextCursor::Right);
   bool alignRight = isRightToLeft();

   if (!layoutData->hasBidi) {
      return moveRight ^ alignRight ? nextLogicalPosition(pos) : previousLogicalPosition(pos);
   }

   int lineNum = lineNumberForTextPosition(pos);
   if (lineNum < 0) {
      return pos;
   }

   QVector<int> insertionPoints;
   insertionPointsForLine(lineNum, insertionPoints);

   int i, max = insertionPoints.size();

   for (i = 0; i < max; i++) {

      if (pos == insertionPoints[i]) {
         if (moveRight) {
            if (i + 1 < max) {
               return insertionPoints[i + 1];
            }

         } else {
            if (i > 0) {
               return insertionPoints[i - 1];
            }
         }

         if (moveRight ^ alignRight) {
            if (lineNum + 1 < lines.size()) {
               return alignRight ? endOfLine(lineNum + 1) : beginningOfLine(lineNum + 1);
            }

         } else {
            if (lineNum > 0) {
               return alignRight ? beginningOfLine(lineNum - 1) : endOfLine(lineNum - 1);
            }
         }

         break;
      }
   }

   return pos;
}

void QTextEngine::addItemDecoration(QPainter *painter, const QLineF &line, ItemDecorationList *decorationList)
{
   if (delayDecorations) {
      decorationList->append(ItemDecoration(line.x1(), line.x2(), line.y1(), painter->pen()));
   } else {
      painter->drawLine(line);
   }
}

void QTextEngine::addUnderline(QPainter *painter, const QLineF &line)
{
   addItemDecoration(painter, line, &underlineList);
}

void QTextEngine::addStrikeOut(QPainter *painter, const QLineF &line)
{
   addItemDecoration(painter, line, &strikeOutList);
}

void QTextEngine::addOverline(QPainter *painter, const QLineF &line)
{
   addItemDecoration(painter, line, &overlineList);
}

void QTextEngine::drawItemDecorationList(QPainter *painter, const ItemDecorationList &decorationList)
{
   if (decorationList.isEmpty()) {
      return;
   }

   for (const ItemDecoration &decoration : decorationList) {
      painter->setPen(decoration.pen);
      painter->drawLine(QLineF(decoration.x1, decoration.y, decoration.x2, decoration.y));
   }
}

void QTextEngine::drawDecorations(QPainter *painter)
{
   QPen oldPen = painter->pen();

   adjustUnderlines();
   drawItemDecorationList(painter, underlineList);
   drawItemDecorationList(painter, strikeOutList);
   drawItemDecorationList(painter, overlineList);

   clearDecorations();

   painter->setPen(oldPen);
}

void QTextEngine::clearDecorations()
{
   underlineList.clear();
   strikeOutList.clear();
   overlineList.clear();
}

void QTextEngine::adjustUnderlines()
{

   if (underlineList.isEmpty()) {
      return;
   }

   ItemDecorationList::iterator start = underlineList.begin();
   ItemDecorationList::iterator end   = underlineList.end();
   ItemDecorationList::iterator it = start;
   qreal underlinePos = start->y;
   qreal penWidth = start->pen.widthF();
   qreal lastLineEnd = start->x1;

   while (it != end) {
      if (qFuzzyCompare(lastLineEnd, it->x1)) { // no gap between underlines
         underlinePos = qMax(underlinePos, it->y);
         penWidth = qMax(penWidth, it->pen.widthF());
      } else { // gap between this and the last underline
         adjustUnderlines(start, it, underlinePos, penWidth);
         start = it;
         underlinePos = start->y;
         penWidth = start->pen.widthF();
      }
      lastLineEnd = it->x2;
      ++it;
   }

   adjustUnderlines(start, end, underlinePos, penWidth);
}

void QTextEngine::adjustUnderlines(ItemDecorationList::iterator start,
   ItemDecorationList::iterator end, qreal underlinePos, qreal penWidth)
{
   for (ItemDecorationList::iterator it = start; it != end; ++it) {
      it->y = underlinePos;
      it->pen.setWidthF(penWidth);
   }
}
QStackTextEngine::QStackTextEngine(const QString &string, const QFont &f)
   : QTextEngine(string, f), _layoutData(string, _memory, MemSize)
{
   stackEngine = true;
   layoutData  = &_layoutData;
}

QTextItemInt::QTextItemInt(const QScriptItem &si, QFont *font, const QTextCharFormat &format)
   : justified(false), underlineStyle(QTextCharFormat::NoUnderline), charFormat(format),
     logClusters(0), f(0), fontEngine(0)
{
   f = font;
   fontEngine = f->d->engineForScript(si.analysis.script);
   Q_ASSERT(fontEngine);

   initWithScriptItem(si);
}

QTextItemInt::QTextItemInt(const QGlyphLayout &g, QFont *font, QString::const_iterator begin, const QString::const_iterator end,
   QFontEngine *fe, const QTextCharFormat &format)

   : flags(0), justified(false), underlineStyle(QTextCharFormat::NoUnderline), charFormat(format),
     m_iter(begin), m_end(end), logClusters(0), f(font),  glyphs(g), fontEngine(fe)
{
}

// Fix up flags and underlineStyle with given info
void QTextItemInt::initWithScriptItem(const QScriptItem &si)
{
   // explicitly initialize flags so that initFontAttributes can be called
   // multiple times on the same TextItem
   flags = 0;

   if (si.analysis.bidiLevel % 2) {
      flags |= QTextItem::RightToLeft;
   }

   ascent = si.ascent;
   descent = si.descent;

   if (charFormat.hasProperty(QTextFormat::TextUnderlineStyle)) {
      underlineStyle = charFormat.underlineStyle();

   } else if (charFormat.boolProperty(QTextFormat::FontUnderline) || f->d->underline) {
      underlineStyle = QTextCharFormat::SingleUnderline;

   }

   // compat
   if (underlineStyle == QTextCharFormat::SingleUnderline) {
      flags |= QTextItem::Underline;
   }

   if (f->d->overline || charFormat.fontOverline()) {
      flags |= QTextItem::Overline;
   }

   if (f->d->strikeOut || charFormat.fontStrikeOut()) {
      flags |= QTextItem::StrikeOut;
   }
}

QTextItemInt QTextItemInt::midItem(QFontEngine *fontEngine, int firstGlyphIndex, int len) const
{
   QTextItemInt ti = *this;
   const int end   = firstGlyphIndex + len;

   ti.glyphs     = glyphs.mid(firstGlyphIndex, len);
   ti.fontEngine = fontEngine;

   if (logClusters && (m_iter != m_end)) {
      const int logClusterOffset = logClusters[0];

      int index = 0;

      while (logClusters[index] - logClusterOffset < firstGlyphIndex) {
         ++ti.m_iter;
         ++index;
      }

      ti.logClusters += index;

      index = 0;
      ti.m_end = ti.m_iter;

      while (ti.m_end != m_end && ti.logClusters[index] - logClusterOffset < end) {
         ++ti.m_end;
         ++index;
      }
   }

   return ti;
}

QTransform qt_true_matrix(qreal w, qreal h, QTransform x)
{
   QRectF rect = x.mapRect(QRectF(0, 0, w, h));
   return x * QTransform::fromTranslate(-rect.x(), -rect.y());
}

glyph_metrics_t glyph_metrics_t::transformed(const QTransform &matrix) const
{
   if (matrix.type() < QTransform::TxTranslate) {
      return *this;
   }

   glyph_metrics_t m = *this;

   qreal w = width.toReal();
   qreal h = height.toReal();
   QTransform xform = qt_true_matrix(w, h, matrix);

   QRectF rect(0, 0, w, h);
   rect = xform.mapRect(rect);
   m.width = QFixed::fromReal(rect.width());
   m.height = QFixed::fromReal(rect.height());

   QLineF l = xform.map(QLineF(x.toReal(), y.toReal(), xoff.toReal(), yoff.toReal()));

   m.x = QFixed::fromReal(l.x1());
   m.y = QFixed::fromReal(l.y1());

   // The offset is relative to the baseline which is why we use dx/dy of the line
   m.xoff = QFixed::fromReal(l.dx());
   m.yoff = QFixed::fromReal(l.dy());

   return m;
}

QTextLineItemIterator::QTextLineItemIterator(QTextEngine *_eng, int _lineNum, const QPointF &pos,
   const QTextLayout::FormatRange *_selection)
   : eng(_eng), line(eng->lines[_lineNum]), si(nullptr), lineNum(_lineNum), lineEnd(line.from + line.length),
     firstItem(eng->findItem(line.from)), lastItem(eng->findItem(lineEnd - 1, firstItem)),
     nItems((firstItem >= 0 && lastItem >= firstItem) ? (lastItem - firstItem + 1) : 0),
     logicalItem(-1), item(-1), visualOrder(nItems), selection(_selection)
{
   x = QFixed::fromReal(pos.x());
   x += line.x;
   x += eng->alignLine(line);

   QVarLengthArray<uchar> levels(nItems);

   for (int i = 0; i < nItems; ++i) {
      levels[i] = eng->layoutData->items[i + firstItem].analysis.bidiLevel;
   }

   QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

   eng->shapeLine(line);
}

QScriptItem &QTextLineItemIterator::next()
{
   x += itemWidth;

   ++logicalItem;
   item       = visualOrder[logicalItem] + firstItem;
   itemLength = eng->length(item);

   si = &eng->layoutData->items[item];

   if (! si->num_glyphs) {
      eng->shape(item);
   }

   itemStart = qMax(line.from, si->position);
   itemEnd   = qMin(lineEnd, si->position + itemLength);

   if (si->analysis.flags >= QScriptAnalysis::TabOrObject) {
      glyphsStart = 0;
      glyphsEnd   = 1;
      itemWidth   = si->width;

      return *si;
   }

   unsigned short *logClusters = eng->logClusters(si);
   QGlyphLayout glyphs = eng->shapedGlyphs(si);

   glyphsStart = logClusters[itemStart - si->position];
   glyphsEnd   = (itemEnd == si->position + itemLength) ? si->num_glyphs : logClusters[itemEnd - si->position];

   // show soft-hyphen at line-break
   if (si->position + itemLength >= lineEnd
         && eng->layoutData->string.at(lineEnd - 1) == QChar::SoftHyphen) {
      glyphs.attributes[glyphsEnd - 1].dontPrint = false;
   }

   itemWidth = 0;
   for (int g = glyphsStart; g < glyphsEnd; ++g) {
      itemWidth += glyphs.effectiveAdvance(g);
   }

   return *si;
}

bool QTextLineItemIterator::getSelectionBounds(QFixed *selectionX, QFixed *selectionWidth) const
{
   *selectionX = *selectionWidth = 0;

   if (! selection) {
      return false;
   }

   if (si->analysis.flags >= QScriptAnalysis::TabOrObject) {
      if (si->position >= selection->start + selection->length
         || si->position + itemLength <= selection->start) {
         return false;
      }

      *selectionX = x;
      *selectionWidth = itemWidth;

   } else {
      unsigned short *logClusters = eng->logClusters(si);
      QGlyphLayout glyphs = eng->shapedGlyphs(si);

      int from = qMax(itemStart, selection->start) - si->position;
      int to = qMin(itemEnd, selection->start + selection->length) - si->position;
      if (from >= to) {
         return false;
      }

      int start_glyph = logClusters[from];
      int end_glyph = (to == itemLength) ? si->num_glyphs : logClusters[to];
      QFixed soff;
      QFixed swidth;

      if (si->analysis.bidiLevel % 2) {
         for (int g = glyphsEnd - 1; g >= end_glyph; --g) {
            soff += glyphs.effectiveAdvance(g);
         }
         for (int g = end_glyph - 1; g >= start_glyph; --g) {
            swidth += glyphs.effectiveAdvance(g);
         }
      } else {
         for (int g = glyphsStart; g < start_glyph; ++g) {
            soff += glyphs.effectiveAdvance(g);
         }
         for (int g = start_glyph; g < end_glyph; ++g) {
            swidth += glyphs.effectiveAdvance(g);
         }
      }

      // If the starting character is in the middle of a ligature,
      // selection should only contain the right part of that ligature
      // glyph, so we need to get the width of the left part here and
      // add it to *selectionX

      QFixed leftOffsetInLigature = eng->offsetInLigature(si, from, to, start_glyph);
      *selectionX = x + soff + leftOffsetInLigature;
      *selectionWidth = swidth - leftOffsetInLigature;

      // If the ending character is also part of a ligature, swidth does
      // not contain that part yet, we also need to find out the width of
      // that left part

      *selectionWidth += eng->offsetInLigature(si, to, itemLength, end_glyph);
   }

   return true;
}
