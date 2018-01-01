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

#include <qglobal.h>

#if !defined(QT_NO_RAWFONT)

#include <qglyphrun.h>
#include <qglyphrun_p.h>

QT_BEGIN_NAMESPACE


QGlyphRun::QGlyphRun() : d(new QGlyphRunPrivate)
{
}

QGlyphRun::QGlyphRun(const QGlyphRun &other)
{
   d = other.d;
}

/*!
    Destroys the QGlyphRun.
*/
QGlyphRun::~QGlyphRun()
{
   // Required for QExplicitlySharedDataPointer
}

/*!
    \internal
*/
void QGlyphRun::detach()
{
   if (d->ref.load() != 1) {
      d.detach();
   }
}

/*!
    Assigns \a other to this QGlyphRun object.
*/
QGlyphRun &QGlyphRun::operator=(const QGlyphRun &other)
{
   d = other.d;
   return *this;
}

/*!
    Compares \a other to this QGlyphRun object. Returns true if the list of glyph indexes,
    the list of positions and the font are all equal, otherwise returns false.
*/
bool QGlyphRun::operator==(const QGlyphRun &other) const
{
   if (d == other.d) {
      return true;
   }

   if ((d->glyphIndexDataSize != other.d->glyphIndexDataSize)
         || (d->glyphPositionDataSize != other.d->glyphPositionDataSize)) {
      return false;
   }

   if (d->glyphIndexData != other.d->glyphIndexData) {
      for (int i = 0; i < d->glyphIndexDataSize; ++i) {
         if (d->glyphIndexData[i] != other.d->glyphIndexData[i]) {
            return false;
         }
      }
   }
   if (d->glyphPositionData != other.d->glyphPositionData) {
      for (int i = 0; i < d->glyphPositionDataSize; ++i) {
         if (d->glyphPositionData[i] != other.d->glyphPositionData[i]) {
            return false;
         }
      }
   }

   return (d->overline == other.d->overline
           && d->underline == other.d->underline
           && d->strikeOut == other.d->strikeOut
           && d->rawFont == other.d->rawFont);
}

/*!
    \fn bool QGlyphRun::operator!=(const QGlyphRun &other) const

    Compares \a other to this QGlyphRun object. Returns true if any of the list of glyph
    indexes, the list of positions or the font are different, otherwise returns false.
*/

/*!
    Returns the font selected for this QGlyphRun object.

    \sa setRawFont()
*/
QRawFont QGlyphRun::rawFont() const
{
   return d->rawFont;
}

/*!
    Sets the font specified by \a rawFont to be the font used to look up the
    glyph indexes.

    \sa rawFont(), setGlyphIndexes()
*/
void QGlyphRun::setRawFont(const QRawFont &rawFont)
{
   detach();
   d->rawFont = rawFont;
}

/*!
    Returns the glyph indexes for this QGlyphRun object.

    \sa setGlyphIndexes(), setPositions()
*/
QVector<quint32> QGlyphRun::glyphIndexes() const
{
   if (d->glyphIndexes.constData() == d->glyphIndexData) {
      return d->glyphIndexes;
   } else {
      QVector<quint32> indexes(d->glyphIndexDataSize);
      memcpy(indexes.data(), d->glyphIndexData, d->glyphIndexDataSize * sizeof(quint32));
      return indexes;
   }
}

/*!
    Set the glyph indexes for this QGlyphRun object to \a glyphIndexes. The glyph indexes must
    be valid for the selected font.
*/
void QGlyphRun::setGlyphIndexes(const QVector<quint32> &glyphIndexes)
{
   detach();
   d->glyphIndexes = glyphIndexes; // Keep a reference to the QVector to avoid copying
   d->glyphIndexData = glyphIndexes.constData();
   d->glyphIndexDataSize = glyphIndexes.size();
}

/*!
    Returns the position of the edge of the baseline for each glyph in this set of glyph indexes.
*/
QVector<QPointF> QGlyphRun::positions() const
{
   if (d->glyphPositions.constData() == d->glyphPositionData) {
      return d->glyphPositions;
   } else {
      QVector<QPointF> glyphPositions(d->glyphPositionDataSize);
      memcpy(glyphPositions.data(), d->glyphPositionData,
             d->glyphPositionDataSize * sizeof(QPointF));
      return glyphPositions;
   }
}

/*!
    Sets the positions of the edge of the baseline for each glyph in this set of glyph indexes to
    \a positions.
*/
void QGlyphRun::setPositions(const QVector<QPointF> &positions)
{
   detach();
   d->glyphPositions = positions; // Keep a reference to the vector to avoid copying
   d->glyphPositionData = positions.constData();
   d->glyphPositionDataSize = positions.size();
}

/*!
    Clears all data in the QGlyphRun object.
*/
void QGlyphRun::clear()
{
   detach();
   d->rawFont = QRawFont();
   d->strikeOut = false;
   d->overline = false;
   d->underline = false;

   setPositions(QVector<QPointF>());
   setGlyphIndexes(QVector<quint32>());
}

/*!
    Sets the glyph indexes and positions of this QGlyphRun to use the first \a size
    elements in the arrays \a glyphIndexArray and \a glyphPositionArray. The data is
    \e not copied. The caller must guarantee that the arrays are not deleted as long
    as this QGlyphRun and any copies of it exists.

    \sa setGlyphIndexes(), setPositions()
*/
void QGlyphRun::setRawData(const quint32 *glyphIndexArray, const QPointF *glyphPositionArray,
                           int size)
{
   detach();
   d->glyphIndexes.clear();
   d->glyphPositions.clear();

   d->glyphIndexData = glyphIndexArray;
   d->glyphPositionData = glyphPositionArray;
   d->glyphIndexDataSize = d->glyphPositionDataSize = size;
}

/*!
   Returns true if this QGlyphRun should be painted with an overline decoration.

   \sa setOverline()
*/
bool QGlyphRun::overline() const
{
   return d->overline;
}

/*!
  Indicates that this QGlyphRun should be painted with an overline decoration if \a overline is true.
  Otherwise the QGlyphRun should be painted with no overline decoration.

  \sa overline()
*/
void QGlyphRun::setOverline(bool overline)
{
   if (d->overline == overline) {
      return;
   }

   detach();
   d->overline = overline;
}

/*!
   Returns true if this QGlyphRun should be painted with an underline decoration.

   \sa setUnderline()
*/
bool QGlyphRun::underline() const
{
   return d->underline;
}

/*!
  Indicates that this QGlyphRun should be painted with an underline decoration if \a underline is
  true. Otherwise the QGlyphRun should be painted with no underline decoration.

  \sa underline()
*/
void QGlyphRun::setUnderline(bool underline)
{
   if (d->underline == underline) {
      return;
   }

   detach();
   d->underline = underline;
}

/*!
   Returns true if this QGlyphRun should be painted with a strike out decoration.

   \sa setStrikeOut()
*/
bool QGlyphRun::strikeOut() const
{
   return d->strikeOut;
}

/*!
  Indicates that this QGlyphRun should be painted with an strike out decoration if \a strikeOut is
  true. Otherwise the QGlyphRun should be painted with no strike out decoration.

  \sa strikeOut()
*/
void QGlyphRun::setStrikeOut(bool strikeOut)
{
   if (d->strikeOut == strikeOut) {
      return;
   }

   detach();
   d->strikeOut = strikeOut;
}

QT_END_NAMESPACE

#endif // QT_NO_RAWFONT
