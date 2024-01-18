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

#ifndef QRAWFONT_H
#define QRAWFONT_H

#include <qstring.h>
#include <qiodevice.h>
#include <qglobal.h>
#include <qobject.h>
#include <qpoint.h>
#include <qfont.h>
#include <qtransform.h>
#include <qfontdatabase.h>

class QRawFontPrivate;

class Q_GUI_EXPORT QRawFont
{
 public:
   enum AntialiasingType {
      PixelAntialiasing,
      SubPixelAntialiasing
   };

   enum LayoutFlag {
      SeparateAdvances = 0,
      KernedAdvances   = 1,
      UseDesignMetrics = 2
   };

   using LayoutFlags = QFlags<LayoutFlag>;

   QRawFont();

   QRawFont(const QString &fileName, qreal pixelSize,
      QFont::HintingPreference hintingPreference = QFont::PreferDefaultHinting);

   QRawFont(const QByteArray &fontData, qreal pixelSize,
      QFont::HintingPreference hintingPreference = QFont::PreferDefaultHinting);

   QRawFont(const QRawFont &other);

   ~QRawFont();

   bool isValid() const;

   QRawFont &operator=(const QRawFont &other);

   QRawFont &operator=(QRawFont &&other) {
      swap(other);
      return *this;
   }

   bool operator==(const QRawFont &other) const;
   inline bool operator!=(const QRawFont &other) const {
      return ! operator==(other);
   }

   QString familyName() const;
   QString styleName() const;

   QFont::Style style() const;
   int weight() const;

   QVector<quint32> glyphIndexesForString(const QString &text) const;

   inline QVector<QPointF> advancesForGlyphIndexes(const QVector<quint32> &glyphIndexes) const;
   inline QVector<QPointF> advancesForGlyphIndexes(const QVector<quint32> &glyphIndexes, LayoutFlags layoutFlags) const;

   bool glyphIndexesForChars(QStringView str, quint32 *glyphIndexes, int *numGlyphs) const;
   bool advancesForGlyphIndexes(const quint32 *glyphIndexes, QPointF *advances, int numGlyphs) const;
   bool advancesForGlyphIndexes(const quint32 *glyphIndexes, QPointF *advances, int numGlyphs, LayoutFlags layoutFlags) const;

   QImage alphaMapForGlyph(quint32 glyphIndex, AntialiasingType antialiasingType = SubPixelAntialiasing,
      const QTransform &transform = QTransform()) const;

   QPainterPath pathForGlyph(quint32 glyphIndex) const;
   QRectF boundingRect(quint32 glyphIndex) const;

   void setPixelSize(qreal pixelSize);
   qreal pixelSize() const;

   QFont::HintingPreference hintingPreference() const;

   qreal ascent() const;
   qreal descent() const;
   qreal leading() const;
   qreal xHeight() const;
   qreal averageCharWidth() const;
   qreal maxCharWidth() const;
   qreal lineThickness() const;
   qreal underlinePosition() const;

   qreal unitsPerEm() const;

   void loadFromFile(const QString &fileName, qreal pixelSize, QFont::HintingPreference hintingPreference);
   void loadFromData(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference);

   void swap(QRawFont &other) {
      qSwap(m_fontPrivate, other.m_fontPrivate);
   }

   bool supportsCharacter(quint32 ucs4) const;
   bool supportsCharacter(QChar character) const;
   QList<QFontDatabase::WritingSystem> supportedWritingSystems() const;

   QByteArray fontTable(const char *tagName) const;

   static QRawFont fromFont(const QFont &font, QFontDatabase::WritingSystem writingSystem = QFontDatabase::Any);

 private:
   friend class QRawFontPrivate;
   friend class QTextLayout;
   friend class QTextEngine;

   std::shared_ptr<QRawFontPrivate> m_fontPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QRawFont::LayoutFlags)

inline QVector<QPointF> QRawFont::advancesForGlyphIndexes(const QVector<quint32> &glyphIndexes,
   QRawFont::LayoutFlags layoutFlags) const
{
   QVector<QPointF> advances(glyphIndexes.size());

   if (advancesForGlyphIndexes(glyphIndexes.constData(), advances.data(), glyphIndexes.size(), layoutFlags)) {
      return advances;
   }

   return QVector<QPointF>();
}

inline QVector<QPointF> QRawFont::advancesForGlyphIndexes(const QVector<quint32> &glyphIndexes) const
{
   return advancesForGlyphIndexes(glyphIndexes, QRawFont::SeparateAdvances);
}

#endif
