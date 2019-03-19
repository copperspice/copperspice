/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QABSTRACTFONTENGINE_P_H
#define QABSTRACTFONTENGINE_P_H

#include <qfontengine_p.h>
#include <qabstractfontengine_qws.h>

QT_BEGIN_NAMESPACE

class QCustomFontEngine;

class QProxyFontEngine : public QFontEngine
{
   GUI_CS_OBJECT(QProxyFontEngine)

 public:
   QProxyFontEngine(QAbstractFontEngine *engine, const QFontDef &def);
   virtual ~QProxyFontEngine();

   bool stringToCMap(QStringView str, QGlyphLayout *glyphs, int *nglyphs, QTextEngine::ShaperFlags flags) const override;

   virtual void recalcAdvances(QGlyphLayout *, QTextEngine::ShaperFlags) const;
   virtual QImage alphaMapForGlyph(glyph_t);

   virtual void addGlyphsToPath(glyph_t *glyphs, QFixedPoint *positions, int nglyphs, QPainterPath *path, QTextItem::RenderFlags flags);

   virtual glyph_metrics_t boundingBox(const QGlyphLayout &glyphs);
   virtual glyph_metrics_t boundingBox(glyph_t glyph);

   virtual QFixed ascent() const;
   virtual QFixed descent() const;
   virtual QFixed leading() const;
   virtual QFixed xHeight() const;
   virtual QFixed averageCharWidth() const;
   virtual QFixed lineThickness() const;
   virtual QFixed underlinePosition() const;
   virtual qreal maxCharWidth() const;
   virtual qreal minLeftBearing() const;
   virtual qreal minRightBearing() const;
   virtual int glyphCount() const;

   bool canRender(QStringView str) override;

   const QString &fontEngineName() const override {
      static QString retval("proxy engine");
      return retval;
   }

   Type type() const override {
      return Proxy;
   }

#if !defined(Q_WS_X11) && !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
   virtual void draw(QPaintEngine *, qreal, qreal, const QTextItemInt &);
#endif

   inline QAbstractFontEngine::Capabilities capabilities() const {
      return engineCapabilities;
   }

   bool drawAsOutline() const;

 private:
   QAbstractFontEngine *engine;
   QAbstractFontEngine::Capabilities engineCapabilities;
};

QT_END_NAMESPACE

#endif
