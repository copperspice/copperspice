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

#include <qtextboundaryfinder.h>

#include <qunicodetools_p.h>

class QTextBoundaryFinderPrivate
{
 public:
   QCharAttributes attributes[1];
};

static void init(QTextBoundaryFinder::BoundaryType type, const QString &str, QCharAttributes *attributes)
{
   QVector<QUnicodeTools::ScriptItem> scriptItems;

   {
      auto length = str.size();

      QVector<QChar::Script> scriptIds(length);
      QUnicodeTools::initScripts(str, scriptIds);

      int start = 0;

      for (int k = start + 1; k <= length; ++k) {
         if (k == length || scriptIds[k] != scriptIds[start]) {

            QUnicodeTools::ScriptItem item;
            item.position = start;
            item.script   = scriptIds[start];

            scriptItems.append(item);
            start = k;
         }
      }
   }

   QUnicodeTools::CharAttributeOptions options = Qt::EmptyFlag;

   switch (type) {
      case QTextBoundaryFinder::Grapheme:
         options |= QUnicodeTools::GraphemeBreaks;
         break;

      case QTextBoundaryFinder::Word:
         options |= QUnicodeTools::WordBreaks;
         break;

      case QTextBoundaryFinder::Sentence:
         options |= QUnicodeTools::SentenceBreaks;
         break;

      case QTextBoundaryFinder::Line:
         options |= QUnicodeTools::LineBreaks;
         break;

      default:
         break;
   }

   QUnicodeTools::initCharAttributes(str, scriptItems, attributes, options);
}

QTextBoundaryFinder::QTextBoundaryFinder()
   : m_type(Grapheme), iter_pos(m_str.cbegin()), m_valid(false), freePrivate(true), d(nullptr)
{
}

QTextBoundaryFinder::QTextBoundaryFinder(BoundaryType type, const QString &str)
   : m_type(type), m_str(str), iter_pos(m_str.cbegin()), m_valid(true), freePrivate(true), d(nullptr)
{
   auto length = m_str.size();

   if (length > 0) {
      d = (QTextBoundaryFinderPrivate *) malloc((length + 1) * sizeof(QCharAttributes));
      Q_CHECK_PTR(d);

      init(m_type, m_str, d->attributes);
   }
}

QTextBoundaryFinder::QTextBoundaryFinder(const QTextBoundaryFinder &other)
   : m_type(other.m_type), m_str(other.m_str), m_valid(other.m_valid), freePrivate(true), d(nullptr)
{
   iter_pos = m_str.cbegin() + (other.iter_pos - other.m_str.cbegin());

   if (other.d != nullptr) {
      auto length = m_str.size();

      Q_ASSERT(length > 0);
      d = (QTextBoundaryFinderPrivate *) malloc((length + 1) * sizeof(QCharAttributes));

      Q_CHECK_PTR(d);
      memcpy(d, other.d, (length + 1) * sizeof(QCharAttributes));
   }
}

QTextBoundaryFinder::QTextBoundaryFinder(QTextBoundaryFinder &&other)
{
   *this = std::move(other);
}

QTextBoundaryFinder &QTextBoundaryFinder::operator=(const QTextBoundaryFinder &other)
{
   if (&other == this) {
      return *this;
   }

   m_type   = other.m_type;
   m_str    = other.m_str;
   iter_pos = m_str.cbegin() + (other.iter_pos - other.m_str.cbegin());
   m_valid  = other.m_valid;

   auto length = other.m_str.size();

   if (other.d == nullptr) {

      if (freePrivate) {
         free(d);
      }

      d = nullptr;

   } else {
      Q_ASSERT(length > 0);

      uint newCapacity = (length + 1) * sizeof(QCharAttributes);
      QTextBoundaryFinderPrivate *newD = (QTextBoundaryFinderPrivate *) realloc(freePrivate ? d : nullptr, newCapacity);

      Q_CHECK_PTR(newD);
      freePrivate = true;
      d           = newD;

      memcpy(d, other.d, (length + 1) * sizeof(QCharAttributes));
   }

   return *this;
}

QTextBoundaryFinder &QTextBoundaryFinder::operator=(QTextBoundaryFinder &&other)
{
   m_type      = other.m_type;
   m_str       = std::move(other.m_str);
   iter_pos    = std::move(other.iter_pos);
   m_valid     = other.m_valid;

   if (freePrivate) {
      free(d);
   }

   freePrivate       = other.freePrivate;
   other.freePrivate = false;

   d       = other.d;
   other.d = nullptr;

   return *this;
}

QTextBoundaryFinder::~QTextBoundaryFinder()
{
   if (freePrivate) {
      free(d);
   }
}

void QTextBoundaryFinder::toStart()
{
   iter_pos = m_str.cbegin();
   m_valid  = true;
}

void QTextBoundaryFinder::toEnd()
{
   iter_pos = m_str.cend();
   m_valid  = true;
}

int QTextBoundaryFinder::position() const
{
   return iter_pos - m_str.cbegin();
}

void QTextBoundaryFinder::setPosition(int position)
{
   int index = qBound(0, position, m_str.size());
   iter_pos  = m_str.cbegin() + index;
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

   if (! m_valid || iter_pos == m_str.cend()) {
      m_valid = false;
      return -1;
   }

   ++iter_pos;
   int index = iter_pos - m_str.cbegin();

   if (iter_pos == m_str.cend()) {
      return index;
   }

   switch (m_type) {
      case Grapheme:
         while (iter_pos != m_str.cend() && ! d->attributes[index].graphemeBoundary) {
            ++iter_pos;
            ++index;
         }

         break;

      case Word:
         while (iter_pos != m_str.cend() && ! d->attributes[index].wordBreak) {
            ++iter_pos;
            ++index;
         }

         break;

      case Sentence:
         while (iter_pos != m_str.cend() && ! d->attributes[index].sentenceBoundary) {
            ++iter_pos;
            ++index;
         }

         break;

      case Line:
         while (iter_pos != m_str.cend() && ! d->attributes[index].lineBreak) {
            ++iter_pos;
            ++index;
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

   if (! m_valid || iter_pos == m_str.cbegin() || iter_pos == m_str.cend()) {
      m_valid = false;
      return -1;
   }

   --iter_pos;
   int index = iter_pos - m_str.cbegin();

   if (iter_pos == m_str.cbegin()) {
      return index;
   }

   switch (m_type) {
      case Grapheme:
         while (iter_pos != m_str.cbegin() && ! d->attributes[index].graphemeBoundary) {
            --iter_pos;
            --index;
         }

         break;

      case Word:
         while (iter_pos != m_str.cbegin() && ! d->attributes[index].wordBreak) {
            --iter_pos;
            --index;
         }

         break;

      case Sentence:
         while (iter_pos != m_str.cbegin() && ! d->attributes[index].sentenceBoundary) {
            --iter_pos;
            --index;
         }

         break;

      case Line:
         while (iter_pos != m_str.cbegin() && !d ->attributes[index].lineBreak) {
            --iter_pos;
            --index;
         }

         break;
   }

   return index;
}

bool QTextBoundaryFinder::isAtBoundary() const
{
   if (! d || ! m_valid) {
      return false;
   }

   if (iter_pos == m_str.cend()) {
      return true;
   }

   int index = iter_pos - m_str.cbegin();

   switch (m_type) {
      case Grapheme:
         return d->attributes[index].graphemeBoundary;

      case Word:
         return d->attributes[index].wordBreak;

      case Sentence:
         return d->attributes[index].sentenceBoundary;

      case Line:
         return d->attributes[index].lineBreak || index == 0;
   }

   return false;
}

QTextBoundaryFinder::BoundaryReasons QTextBoundaryFinder::boundaryReasons() const
{
   BoundaryReasons reasons = NotAtBoundary;

   if (! m_valid) {
      return reasons;
   }

   int index = iter_pos - m_str.cbegin();

   const QCharAttributes attr = d->attributes[index];

   switch (m_type) {
      case Grapheme:
         if (attr.graphemeBoundary) {
            reasons |= BreakOpportunity | StartOfItem | EndOfItem;

            if (index == 0) {
               reasons &= (~EndOfItem);

            } else if (iter_pos == m_str.cend()) {
               reasons &= (~StartOfItem);

            }
         }

         break;

      case Word:
         if (attr.wordBreak) {
            reasons |= BreakOpportunity;

            if (attr.wordStart) {
               reasons |= StartOfItem;
            }

            if (attr.wordEnd) {
               reasons |= EndOfItem;
            }
         }

         break;

      case Sentence:
         if (attr.sentenceBoundary) {
            reasons |= BreakOpportunity | StartOfItem | EndOfItem;

            if (index == 0) {
               reasons &= (~EndOfItem);

            } else if (iter_pos == m_str.cend()) {
               reasons &= (~StartOfItem);
            }
         }

         break;

      case Line:
         if (attr.lineBreak || index == 0) {
            reasons |= BreakOpportunity;

            if (attr.mandatoryBreak || index == 0) {
               reasons |= MandatoryBreak | StartOfItem | EndOfItem;

               if (index == 0) {
                  reasons &= (~EndOfItem);

               } else if (iter_pos == m_str.cend()) {
                  reasons &= (~StartOfItem);
               }

            } else if (index > 0 && iter_pos[-1].unicode() == QChar::SoftHyphen) {
               reasons |= SoftHyphen;
            }
         }

         break;

      default:
         break;
   }

   return reasons;
}
