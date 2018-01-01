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

#include <QtCore/qtextboundaryfinder.h>
#include <QtCore/qvarlengtharray.h>
#include <qunicodetables_p.h>
#include <qdebug.h>
#include <qharfbuzz_p.h>

QT_BEGIN_NAMESPACE

class QTextBoundaryFinderPrivate
{
 public:
   HB_CharAttributes attributes[1];
};

static void init(QTextBoundaryFinder::BoundaryType type, const QChar *chars, int length, HB_CharAttributes *attributes)
{
   QVarLengthArray<HB_ScriptItem> scriptItems;

   const ushort *string  = reinterpret_cast<const ushort *>(chars);
   const ushort *unicode = string;

   // correctly assign script, isTab and isObject to the script analysis
   const ushort *uc = unicode;
   const ushort *e  = uc + length;

   int script     = QChar::Script_Common;
   int lastScript = QChar::Script_Common;

   const ushort *start = uc;

   while (uc < e) {
      int s = QChar::script(*uc);
      if (s != QChar::Script_Inherited) {
         script = s;
      }

      if (*uc == QChar::ObjectReplacementCharacter || *uc == QChar::LineSeparator || *uc == 9) {
         script = QChar::Script_Common;
      }

      if (script != lastScript) {
         if (uc != start) {
            HB_ScriptItem item;
            item.pos       = start - string;
            item.length    = uc - start;
            item.script    = (HB_Script)lastScript;
            item.bidiLevel = 0;                      // ### what's the proper value?

            scriptItems.append(item);
            start = uc;
         }
         lastScript = script;
      }
      ++uc;
   }

   if (uc != start) {
      HB_ScriptItem item;
      item.pos       = start - string;
      item.length    = uc - start;
      item.script    = (HB_Script)lastScript;
      item.bidiLevel = 0; // ### what's the proper value?

      scriptItems.append(item);
   }

   qGetCharAttributes(string, length, scriptItems.data(), scriptItems.count(), attributes);

   if (type == QTextBoundaryFinder::Word) {
      HB_GetWordBoundaries(string, length, scriptItems.data(), scriptItems.count(), attributes);

   } else if (type == QTextBoundaryFinder::Sentence) {
      HB_GetSentenceBoundaries(string, length, scriptItems.data(), scriptItems.count(), attributes);
   }
}

QTextBoundaryFinder::QTextBoundaryFinder()
   : t(Grapheme), chars(0), length(0), freePrivate(true), d(0)
{
}

/*!
  Copies the QTextBoundaryFinder object, \a other.
*/
QTextBoundaryFinder::QTextBoundaryFinder(const QTextBoundaryFinder &other)
   : t(other.t)
   , s(other.s)
   , chars(other.chars)
   , length(other.length)
   , pos(other.pos)
   , freePrivate(true)
{
   d = (QTextBoundaryFinderPrivate *) malloc(length * sizeof(HB_CharAttributes));
   Q_CHECK_PTR(d);
   memcpy(d, other.d, length * sizeof(HB_CharAttributes));
}

/*!
  Assigns the object, \a other, to another QTextBoundaryFinder object.
*/
QTextBoundaryFinder &QTextBoundaryFinder::operator=(const QTextBoundaryFinder &other)
{
   if (&other == this) {
      return *this;
   }

   t = other.t;
   s = other.s;
   chars = other.chars;
   length = other.length;
   pos = other.pos;

   QTextBoundaryFinderPrivate *newD = (QTextBoundaryFinderPrivate *)
                                      realloc(freePrivate ? d : 0, length * sizeof(HB_CharAttributes));
   Q_CHECK_PTR(newD);
   freePrivate = true;
   d = newD;
   memcpy(d, other.d, length * sizeof(HB_CharAttributes));

   return *this;
}

/*!
  Destructs the QTextBoundaryFinder object.
*/
QTextBoundaryFinder::~QTextBoundaryFinder()
{
   if (freePrivate) {
      free(d);
   }
}

/*!
  Creates a QTextBoundaryFinder object of \a type operating on \a string.
*/
QTextBoundaryFinder::QTextBoundaryFinder(BoundaryType type, const QString &string)
   : t(type)
   , s(string)
   , chars(string.unicode())
   , length(string.length())
   , pos(0)
   , freePrivate(true)
{
   d = (QTextBoundaryFinderPrivate *) malloc(length * sizeof(HB_CharAttributes));
   Q_CHECK_PTR(d);
   init(t, chars, length, d->attributes);
}

/*!
  Creates a QTextBoundaryFinder object of \a type operating on \a chars
  with \a length.

  \a buffer is an optional working buffer of size \a bufferSize you can pass to
  the QTextBoundaryFinder. If the buffer is large enough to hold the working
  data required, it will use this instead of allocating its own buffer.

  \warning QTextBoundaryFinder does not create a copy of \a chars. It is the
  application programmer's responsibility to ensure the array is allocated for
  as long as the QTextBoundaryFinder object stays alive. The same applies to
  \a buffer.
*/
QTextBoundaryFinder::QTextBoundaryFinder(BoundaryType type, const QChar *chars, int length, unsigned char *buffer,
      int bufferSize)
   : t(type)
   , chars(chars)
   , length(length)
   , pos(0)
{
   if (buffer && (uint)bufferSize >= length * sizeof(HB_CharAttributes)) {
      d = (QTextBoundaryFinderPrivate *)buffer;
      freePrivate = false;
   } else {
      d = (QTextBoundaryFinderPrivate *) malloc(length * sizeof(HB_CharAttributes));
      Q_CHECK_PTR(d);
      freePrivate = true;
   }
   init(t, chars, length, d->attributes);
}

/*!
  Moves the finder to the start of the string. This is equivalent to setPosition(0).

  \sa setPosition(), position()
*/
void QTextBoundaryFinder::toStart()
{
   pos = 0;
}

/*!
  Moves the finder to the end of the string. This is equivalent to setPosition(string.length()).

  \sa setPosition(), position()
*/
void QTextBoundaryFinder::toEnd()
{
   pos = length;
}

/*!
  Returns the current position of the QTextBoundaryFinder.

  The range is from 0 (the beginning of the string) to the length of
  the string inclusive.

  \sa setPosition()
*/
int QTextBoundaryFinder::position() const
{
   return pos;
}

/*!
  Sets the current position of the QTextBoundaryFinder to \a position.

  If \a position is out of bounds, it will be bound to only valid
  positions. In this case, valid positions are from 0 to the length of
  the string inclusive.

  \sa position()
*/
void QTextBoundaryFinder::setPosition(int position)
{
   pos = qBound(0, position, length);
}

/*! \fn QTextBoundaryFinder::BoundaryType QTextBoundaryFinder::type() const

  Returns the type of the QTextBoundaryFinder.
*/

/*! \fn bool QTextBoundaryFinder::isValid() const

   Returns true if the text boundary finder is valid; otherwise returns false.
   A default QTextBoundaryFinder is invalid.
*/

/*!
  Returns the string  the QTextBoundaryFinder object operates on.
*/
QString QTextBoundaryFinder::string() const
{
   if (chars == s.unicode() && length == s.length()) {
      return s;
   }
   return QString(chars, length);
}


/*!
  Moves the QTextBoundaryFinder to the next boundary position and returns that position.

  Returns -1 if there is no next boundary.
*/
int QTextBoundaryFinder::toNextBoundary()
{
   if (!d) {
      pos = -1;
      return pos;
   }

   if (pos < 0 || pos >= length) {
      pos = -1;
      return pos;
   }
   ++pos;
   if (pos == length) {
      return pos;
   }

   switch (t) {
      case Grapheme:
         while (pos < length && !d->attributes[pos].charStop) {
            ++pos;
         }
         break;
      case Word:
         while (pos < length && !d->attributes[pos].wordBoundary) {
            ++pos;
         }
         break;
      case Sentence:
         while (pos < length && !d->attributes[pos].sentenceBoundary) {
            ++pos;
         }
         break;
      case Line:
         Q_ASSERT(pos);
         while (pos < length && d->attributes[pos - 1].lineBreakType < HB_Break) {
            ++pos;
         }
         break;
   }

   return pos;
}

/*!
  Moves the QTextBoundaryFinder to the previous boundary position and returns that position.

  Returns -1 if there is no previous boundary.
*/
int QTextBoundaryFinder::toPreviousBoundary()
{
   if (!d) {
      pos = -1;
      return pos;
   }

   if (pos <= 0 || pos > length) {
      pos = -1;
      return pos;
   }
   --pos;
   if (pos == 0) {
      return pos;
   }

   switch (t) {
      case Grapheme:
         while (pos > 0 && !d->attributes[pos].charStop) {
            --pos;
         }
         break;
      case Word:
         while (pos > 0 && !d->attributes[pos].wordBoundary) {
            --pos;
         }
         break;
      case Sentence:
         while (pos > 0 && !d->attributes[pos].sentenceBoundary) {
            --pos;
         }
         break;
      case Line:
         while (pos > 0 && d->attributes[pos - 1].lineBreakType < HB_Break) {
            --pos;
         }
         break;
   }

   return pos;
}

/*!
  Returns true if the object's position() is currently at a valid text boundary.
*/
bool QTextBoundaryFinder::isAtBoundary() const
{
   if (!d || pos < 0) {
      return false;
   }

   if (pos == length) {
      return true;
   }

   switch (t) {
      case Grapheme:
         return d->attributes[pos].charStop;
      case Word:
         return d->attributes[pos].wordBoundary;
      case Line:
         return (pos > 0) ? d->attributes[pos - 1].lineBreakType >= HB_Break : true;
      case Sentence:
         return d->attributes[pos].sentenceBoundary;
   }
   return false;
}

/*!
  Returns the reasons for the boundary finder to have chosen the current position as a boundary.
*/
QTextBoundaryFinder::BoundaryReasons QTextBoundaryFinder::boundaryReasons() const
{
   if (!d) {
      return NotAtBoundary;
   }
   if (! isAtBoundary()) {
      return NotAtBoundary;
   }
   if (pos == 0) {
      if (d->attributes[pos].whiteSpace) {
         return NotAtBoundary;
      }
      return StartWord;
   }
   if (pos == length) {
      if (d->attributes[length - 1].whiteSpace) {
         return NotAtBoundary;
      }
      return EndWord;
   }

   const bool nextIsSpace = d->attributes[pos].whiteSpace;
   const bool prevIsSpace = d->attributes[pos - 1].whiteSpace;

   if (prevIsSpace && !nextIsSpace) {
      return StartWord;
   } else if (!prevIsSpace && nextIsSpace) {
      return EndWord;
   } else if (!prevIsSpace && !nextIsSpace) {
      return BoundaryReasons(StartWord | EndWord);
   } else {
      return NotAtBoundary;
   }
}

QT_END_NAMESPACE
