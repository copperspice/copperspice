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

#ifndef QFONTENGINE_QPF2_P_H
#define QFONTENGINE_QPF2_P_H

#include <qglobal.h>
#include <qendian.h>
#include <qbuffer.h>
#include <qfile.h>

#include <qfontengine_p.h>

class QFontEngine;
class QFreetypeFace;
class QBuffer;

class Q_GUI_EXPORT QFontEngineQPF2 : public QFontEngine
{
 public:
   // if you add new tags please make sure to update the tables in
   // qpfutil.cpp and tools/makeqpf/qpf2.cpp
   enum HeaderTag {
      Tag_FontName,          // 0 string
      Tag_FileName,          // 1 string
      Tag_FileIndex,         // 2 quint32
      Tag_FontRevision,      // 3 quint32
      Tag_FreeText,          // 4 string
      Tag_Ascent,            // 5 QFixed
      Tag_Descent,           // 6 QFixed
      Tag_Leading,           // 7 QFixed
      Tag_XHeight,           // 8 QFixed
      Tag_AverageCharWidth,  // 9 QFixed
      Tag_MaxCharWidth,      // 10 QFixed
      Tag_LineThickness,     // 11 QFixed
      Tag_MinLeftBearing,    // 12 QFixed
      Tag_MinRightBearing,   // 13 QFixed
      Tag_UnderlinePosition, // 14 QFixed
      Tag_GlyphFormat,       // 15 quint8
      Tag_PixelSize,         // 16 quint8
      Tag_Weight,            // 17 quint8
      Tag_Style,             // 18 quint8
      Tag_EndOfHeader,       // 19 string
      Tag_WritingSystems,    // 20 bitfield

      NumTags
   };

   enum TagType {
      StringType,
      FixedType,
      UInt8Type,
      UInt32Type,
      BitFieldType
   };

   struct Tag {
      quint16 tag;
      quint16 size;
   };

   enum GlyphFormat {
      BitmapGlyphs = 1,
      AlphamapGlyphs = 8
   };

   enum {
      CurrentMajorVersion = 2,
      CurrentMinorVersion = 0
   };

   // The CMap is identical to the TrueType CMap table format
   // The GMap table is a normal array with the total number of
   // covered glyphs in the TrueType font
   enum BlockTag {
      CMapBlock,
      GMapBlock,
      GlyphBlock
   };

   struct Header {
      char magic[4]; // 'QPF2'
      quint32 lock;  // values: 0 = unlocked, 0xffffffff = read-only, otherwise qws client id of locking process
      quint8 majorVersion;
      quint8 minorVersion;
      quint16 dataSize;
   };

   struct Block {
      quint16 tag;
      quint16 pad;
      quint32 dataSize;
   };

   struct Glyph {
      quint8 width;
      quint8 height;
      quint8 bytesPerLine;
      qint8 x;
      qint8 y;
      qint8 advance;
   };

   QFontEngineQPF2(const QFontDef &def, const QByteArray &data);
   ~QFontEngineQPF2();

   FaceId faceId() const override {
      return face_id;
   }
   bool getSfntTableData(uint tag, uchar *buffer, uint *length) const override;

   glyph_t glyphIndex(char32_t ch) const override;
   bool stringToCMap(QStringView str, QGlyphLayout *glyphs, int *nglyphs, ShaperFlags flags) const override;
   void recalcAdvances(QGlyphLayout *, ShaperFlags) const override;

   void addOutlineToPath(qreal x, qreal y, const QGlyphLayout &glyphs, QPainterPath *path, QTextItem::RenderFlags flags) override;
   QImage alphaMapForGlyph(glyph_t t) override;

   glyph_metrics_t boundingBox(const QGlyphLayout &glyphs) override;
   glyph_metrics_t boundingBox(glyph_t glyph) override;

   const QString &fontEngineName() const override {
      static QString retval = "qpf2";
      return retval;
   }

   QFixed ascent() const override;
   QFixed descent() const override;
   QFixed leading() const override;
   qreal maxCharWidth() const override;
   qreal minLeftBearing() const override;
   qreal minRightBearing() const override;
   QFixed underlinePosition() const override;
   QFixed lineThickness() const override;

   int glyphCount() const override {
      return glyphMapEntries;
   }

   bool isValid() const;

   const Glyph *findGlyph(glyph_t g) const;

   static bool verifyHeader(const uchar *data, int size);
   static QVariant extractHeaderField(const uchar *data, HeaderTag tag);

 private:
   const uchar *fontData;
   int dataSize;
   const uchar *cmap;
   quint32 cmapOffset;
   int cmapSize;
   quint32 glyphMapOffset;
   quint32 glyphMapEntries;
   quint32 glyphDataOffset;
   quint32 glyphDataSize;
   QString internalFileName;
   QString encodedFileName;
   bool readOnly;

   FaceId face_id;
   QByteArray freetypeCMapTable;
   mutable bool kerning_pairs_loaded;
};

struct QPF2Generator {
   QPF2Generator(QBuffer *device, QFontEngine *engine)
      : dev(device), fe(engine) {}

   void generate();
   void writeHeader();
   void writeGMap();
   void writeBlock(QFontEngineQPF2::BlockTag tag, const QByteArray &data);

   void writeTaggedString(QFontEngineQPF2::HeaderTag tag, const QString &string);

   void writeTaggedUInt32(QFontEngineQPF2::HeaderTag tag, quint32 value);
   void writeTaggedUInt8(QFontEngineQPF2::HeaderTag tag,  quint8 value);
   void writeTaggedQFixed(QFontEngineQPF2::HeaderTag tag, QFixed value);

   void writeUInt16(quint16 value) {
      value = qToBigEndian(value);
      dev->write((const char *)&value, sizeof(value));
   }
   void writeUInt32(quint32 value) {
      value = qToBigEndian(value);
      dev->write((const char *)&value, sizeof(value));
   }
   void writeUInt8(quint8 value)   {
      dev->write((const char *)&value, sizeof(value));
   }
   void writeInt8(qint8 value)     {
      dev->write((const char *)&value, sizeof(value));
   }

   void align4() {
      while (dev->pos() & 3) {
         dev->putChar('\0');
      }
   }

   QBuffer *dev;
   QFontEngine *fe;
};

#endif
