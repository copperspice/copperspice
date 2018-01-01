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

#ifndef QRAWFONT_H
#define QRAWFONT_H

#include <QtCore/qstring.h>
#include <QtCore/qiodevice.h>
#include <QtCore/qglobal.h>
#include <QtCore/qobject.h>
#include <QtCore/qpoint.h>
#include <QtGui/qfont.h>
#include <QtGui/qtransform.h>
#include <QtGui/qfontdatabase.h>

#if !defined(QT_NO_RAWFONT)

QT_BEGIN_NAMESPACE

class QRawFontPrivate;

class Q_GUI_EXPORT QRawFont
{

 public:
   enum AntialiasingType {
      PixelAntialiasing,
      SubPixelAntialiasing
   };

   QRawFont();
   QRawFont(const QString &fileName, qreal pixelSize, QFont::HintingPreference hintingPreference = QFont::PreferDefaultHinting);
   QRawFont(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference = QFont::PreferDefaultHinting);
   QRawFont(const QRawFont &other);
   ~QRawFont();

   bool isValid() const;

   QRawFont &operator=(const QRawFont &other);

   bool operator==(const QRawFont &other) const;
   inline bool operator!=(const QRawFont &other) const {
      return !operator==(other);
   }

   QString familyName() const;
   QString styleName() const;

   QFont::Style style() const;
   int weight() const;

   QVector<quint32> glyphIndexesForString(const QString &text) const;
   QVector<QPointF> advancesForGlyphIndexes(const QVector<quint32> &glyphIndexes) const;
   bool glyphIndexesForChars(const QChar *chars, int numChars, quint32 *glyphIndexes, int *numGlyphs) const;
   bool advancesForGlyphIndexes(const quint32 *glyphIndexes, QPointF *advances, int numGlyphs) const;

   QImage alphaMapForGlyph(quint32 glyphIndex, AntialiasingType antialiasingType = SubPixelAntialiasing,
                           const QTransform &transform = QTransform()) const;

   QPainterPath pathForGlyph(quint32 glyphIndex) const;

   void setPixelSize(qreal pixelSize);
   qreal pixelSize() const;

   QFont::HintingPreference hintingPreference() const;

   qreal ascent() const;
   qreal descent() const;
   qreal leading() const;
   qreal xHeight() const;
   qreal averageCharWidth() const;
   qreal maxCharWidth() const;

   qreal unitsPerEm() const;

   void loadFromFile(const QString &fileName, qreal pixelSize, QFont::HintingPreference hintingPreference);

   void loadFromData(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference);

   bool supportsCharacter(quint32 ucs4) const;
   bool supportsCharacter(QChar character) const;
   QList<QFontDatabase::WritingSystem> supportedWritingSystems() const;

   QByteArray fontTable(const char *tagName) const;

   static QRawFont fromFont(const QFont &font, QFontDatabase::WritingSystem writingSystem = QFontDatabase::Any);

 private:
   friend class QRawFontPrivate;
   QExplicitlySharedDataPointer<QRawFontPrivate> d;
};

QT_END_NAMESPACE

#endif // QT_NO_RAWFONT

#endif // QRAWFONT_H
