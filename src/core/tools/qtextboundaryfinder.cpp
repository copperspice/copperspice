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

#include <qtextboundaryfinder.h>
#include <qvarlengtharray.h>
#include <qunicodetables_p.h>
#include <qdebug.h>
#include <qharfbuzz_p.h>

class QTextBoundaryFinderPrivate
{
 public:
   HB_CharAttributes attributes[1];
};

static void init(QTextBoundaryFinder::BoundaryType type, const QString &str, HB_CharAttributes *attributes)
{
   QVarLengthArray<HB_ScriptItem> scriptItems;

   // correctly assign script, isTab, and isObject to the script analysis
   QString::const_iterator uc    = str.begin();
   QString::const_iterator start = uc;

   QString::const_iterator end = str.end();

   QChar::Script script     = QChar::Script_Common;
   QChar::Script lastScript = QChar::Script_Common;

   while (uc < end) {
      QChar::Script s = uc->script();

      if (s != QChar::Script_Inherited) {
         script = s;
      }

      if (*uc == QChar::ObjectReplacementCharacter || *uc == QChar::LineSeparator || *uc == QChar::Tabulation) {
         script = QChar::Script_Common;
      }

      if (script != lastScript) {

         if (uc != start) {
            HB_ScriptItem item;

            item.pos       = start - str.begin();
            item.length    = uc - start;
            item.script    = (HB_Script)lastScript;
            item.bidiLevel = 0;                       // ### what's the proper value?

            scriptItems.append(item);
            start = uc;
         }

         lastScript = script;
      }
      ++uc;
   }

   if (uc != start) {
      HB_ScriptItem item;

      item.pos       = start - str.begin();
      item.length    = uc - start;
      item.script    = (HB_Script)lastScript;
      item.bidiLevel = 0;                             // ### what's the proper value?

      scriptItems.append(item);
   }

   QString16 tmp = str.toUtf16();
   const HB_UChar16 *utf16_string = reinterpret_cast<const HB_UChar16 *>(tmp.constData());

   qGetCharAttributes(utf16_string, tmp.size_storage(), scriptItems.data(), scriptItems.count(), attributes);

   if (type == QTextBoundaryFinder::Word) {
      HB_GetWordBoundaries(utf16_string, tmp.size_storage(), scriptItems.data(), scriptItems.count(), attributes);

   } else if (type == QTextBoundaryFinder::Sentence) {
      HB_GetSentenceBoundaries(utf16_string, tmp.size_storage(), scriptItems.data(), scriptItems.count(), attributes);
   }
}

QTextBoundaryFinder::QTextBoundaryFinder()
   : t(Grapheme), freePrivate(true), m_valid(false), d(nullptr)
{
}

QTextBoundaryFinder::QTextBoundaryFinder(const QTextBoundaryFinder &other)
   : t(other.t), m_str(other.m_str), iter_pos(other.iter_pos), m_valid(other.m_valid), freePrivate(true)
{
   d = (QTextBoundaryFinderPrivate *) malloc(m_str.size() * sizeof(HB_CharAttributes));
   Q_CHECK_PTR(d);

   memcpy(d, other.d, m_str.size() * sizeof(HB_CharAttributes));
}

QTextBoundaryFinder &QTextBoundaryFinder::operator=(const QTextBoundaryFinder &other)
{
   if (&other == this) {
      return *this;
   }

   t        = other.t;
   m_str    = other.m_str;
   iter_pos = other.iter_pos;
   m_valid  = other.m_valid;

   QTextBoundaryFinderPrivate *newD = (QTextBoundaryFinderPrivate *)
                                      realloc(freePrivate ? d : nullptr, m_str.size() * sizeof(HB_CharAttributes));
   Q_CHECK_PTR(newD);
   freePrivate = true;
   d           = newD;

   memcpy(d, other.d, m_str.size() * sizeof(HB_CharAttributes));

   return *this;
}

QTextBoundaryFinder::~QTextBoundaryFinder()
{
   if (freePrivate) {
      free(d);
   }
}

QTextBoundaryFinder::QTextBoundaryFinder(BoundaryType type, const QString &str)
   : t(type), m_str(str), iter_pos(m_str.begin()), freePrivate(true)
{
   d = (QTextBoundaryFinderPrivate *) malloc(m_str.size() * sizeof(HB_CharAttributes));
   Q_CHECK_PTR(d);

   init(t, m_str, d->attributes);
}

void QTextBoundaryFinder::toStart()
{
   iter_pos = m_str.begin();
   m_valid  = true;
}

void QTextBoundaryFinder::toEnd()
{
   iter_pos = m_str.end();
   m_valid  = true;
}

int QTextBoundaryFinder::position() const
{
   return iter_pos - m_str.begin();
}

void QTextBoundaryFinder::setPosition(int position)
{
   int index = qBound(0, position, m_str.size());
   iter_pos  = m_str.begin() + index;
   m_valid   = true;
}

QString QTextBoundaryFinder::string() const
{
   return m_str;
}

int QTextBoundaryFinder::toNextBoundary()
{
   if (! d) {
      m_valid = false;
      return -1;
   }

   if (! m_valid || iter_pos == m_str.end()) {
      m_valid = false;
      return -1;
   }

   ++iter_pos;
   int index = iter_pos - m_str.begin();
   int max   = m_str.size();

   if (index == max) {
      return index;
   }

   switch (t) {
      case Grapheme:
         while (index < max && ! d->attributes[index].charStop) {
            ++index;
            ++iter_pos;
         }
         break;

      case Word:
         while (index < max && ! d->attributes[index].wordBoundary) {
            ++index;
            ++iter_pos;
         }
         break;

      case Sentence:
         while (index < max && !d->attributes[index].sentenceBoundary) {
            ++index;
            ++iter_pos;
         }
         break;

      case Line:
         while (index < max && d->attributes[index - 1].lineBreakType < HB_Break) {
            ++index;
            ++iter_pos;
         }
         break;
   }

   return index;
}

int QTextBoundaryFinder::toPreviousBoundary()
{
   if (! d) {
      m_valid = false;
      return -1;
   }

   if (! m_valid || iter_pos == m_str.begin() || iter_pos == m_str.end()) {
      m_valid = false;
      return -1;
   }

   --iter_pos;
   int index = iter_pos - m_str.begin();

   if (index == 0) {
      return 0;
   }

   switch (t) {
      case Grapheme:
         while (index > 0 && ! d->attributes[index].charStop) {
            --index;
            --iter_pos;
         }
         break;

      case Word:
         while (index > 0 && ! d->attributes[index].wordBoundary) {
            --index;
            --iter_pos;
         }
         break;

      case Sentence:
         while (index > 0 && ! d->attributes[index].sentenceBoundary) {
            --index;
            --iter_pos;
         }
         break;

      case Line:
         while (index > 0 && d->attributes[index - 1].lineBreakType < HB_Break) {
            --index;
            --iter_pos;
         }
         break;
   }

   return index;
}

bool QTextBoundaryFinder::isAtBoundary() const
{
   if (! d  || ! m_valid) {
      return false;
   }

   if (iter_pos == m_str.end()) {
      return true;
   }

   int index = iter_pos - m_str.begin();

   switch (t) {
      case Grapheme:
         return d->attributes[index].charStop;

      case Word:
         return d->attributes[index].wordBoundary;

      case Line:
         return (index > 0) ? d->attributes[index - 1].lineBreakType >= HB_Break : true;

      case Sentence:
         return d->attributes[index].sentenceBoundary;
   }

   return false;
}

QTextBoundaryFinder::BoundaryReasons QTextBoundaryFinder::boundaryReasons() const
{
   if (! d) {
      return NotAtBoundary;
   }

   if (! isAtBoundary()) {
      return NotAtBoundary;
   }

   if (iter_pos == m_str.begin()) {
      if (d->attributes[0].whiteSpace) {
         return NotAtBoundary;
      }

      return StartWord;
   }

   if (iter_pos == m_str.end()) {
      if (d->attributes[m_str.size() - 1].whiteSpace) {
         return NotAtBoundary;
      }

      return EndWord;
   }

   int index = iter_pos - m_str.begin();

   const bool nextIsSpace = d->attributes[index].whiteSpace;
   const bool prevIsSpace = d->attributes[index - 1].whiteSpace;

   if (prevIsSpace && !nextIsSpace) {
      return StartWord;

   } else if (! prevIsSpace && nextIsSpace) {
      return EndWord;

   } else if (! prevIsSpace && ! nextIsSpace) {
      return BoundaryReasons(StartWord | EndWord);

   } else {
      return NotAtBoundary;
   }
}

