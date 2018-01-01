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

#ifndef QVGFONTGLYPHCACHE_H
#define QVGFONTGLYPHCACHE_H

#include <qvarlengtharray.h>
#include <qfontengine_p.h>
#include <qvg_p.h>

QT_BEGIN_NAMESPACE

class QVGPaintEnginePrivate;

#ifndef QVG_NO_DRAW_GLYPHS

class QVGFontGlyphCache
{
public:
    QVGFontGlyphCache();
    virtual ~QVGFontGlyphCache();

    virtual void cacheGlyphs(QVGPaintEnginePrivate *d,
                             QFontEngine *fontEngine,
                             const glyph_t *g, int count);
    void setScaleFromText(const QFont &font, QFontEngine *fontEngine);

    VGFont font;
    VGfloat scaleX;
    VGfloat scaleY;
    bool invertedGlyphs;
    uint cachedGlyphsMask[256 / 32];
    QSet<glyph_t> cachedGlyphs;
};

#endif

QT_END_NAMESPACE

#endif // QVGFONTGLYPHCACHE_H
