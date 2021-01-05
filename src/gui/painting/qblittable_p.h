/***********************************************************************
*
* Copyright (c) 2012-2021 Barbara Geller
* Copyright (c) 2012-2021 Ansel Sermersheim
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

#ifndef QBLITTABLE_P_H
#define QBLITTABLE_P_H

#include <qsize.h>
#include <qpixmap_blitter_p.h>

#ifndef QT_NO_BLITTABLE


class QImage;
class QBlittablePrivate;

class Q_GUI_EXPORT QBlittable
{
   Q_DECLARE_PRIVATE(QBlittable);

 public:
   enum Capability {

      SolidRectCapability              = 0x0001,
      SourcePixmapCapability           = 0x0002,
      SourceOverPixmapCapability       = 0x0004,
      SourceOverScaledPixmapCapability = 0x0008,
      AlphaFillRectCapability          = 0x0010,
      OpacityPixmapCapability          = 0x0020,
      DrawScaledCachedGlyphsCapability = 0x0040,
      SubPixelGlyphsCapability         = 0x0080,
      ComplexClipCapability            = 0x0100,

      // Internal ones
      OutlineCapability                = 0x0001000,
   };
   using Capabilities = QFlags<Capability>;

   QBlittable (const QSize &size, Capabilities caps);
   virtual ~QBlittable();

   Capabilities capabilities() const;
   QSize size() const;

   virtual void fillRect(const QRectF &rect, const QColor &color) = 0;
   virtual void drawPixmap(const QRectF &rect, const QPixmap &pixmap, const QRectF &subrect) = 0;
   virtual void alphaFillRect(const QRectF &rect, const QColor &color, QPainter::CompositionMode cmode) {
      Q_UNUSED(rect);
      Q_UNUSED(color);
      Q_UNUSED(cmode);
      qWarning("Please implement alphaFillRect function in your platform or remove AlphaFillRectCapability from it");
   }

   virtual void drawPixmapOpacity(const QRectF &rect, const QPixmap &pixmap, const QRectF &subrect,
      QPainter::CompositionMode cmode, qreal opacity) {
      Q_UNUSED(rect);
      Q_UNUSED(pixmap);
      Q_UNUSED(subrect);
      Q_UNUSED(cmode);
      Q_UNUSED(opacity);
      qWarning("Please implement drawPixmapOpacity function in your platform or remove OpacityPixmapCapability from it");
   }

   virtual bool drawCachedGlyphs(const QPaintEngineState *state, QFontEngine::GlyphFormat glyphFormat, int numGlyphs,
      const glyph_t *glyphs, const QFixedPoint *positions, QFontEngine *fontEngine) {
      Q_UNUSED(state);
      Q_UNUSED(glyphFormat);
      Q_UNUSED(numGlyphs);
      Q_UNUSED(glyphs);
      Q_UNUSED(positions);
      Q_UNUSED(fontEngine);
      qWarning("Please implement drawCachedGlyphs function in your platform or remove DrawCachedGlyphsCapability from it");
      return true;
   }
   QImage *lock();
   void unlock();

   bool isLocked() const;
 protected:
   virtual QImage *doLock() = 0;
   virtual void doUnlock() = 0;
   QBlittablePrivate *d_ptr;
};



#endif //QT_NO_BLITTABLE
#endif //QBLITTABLE_P_H
