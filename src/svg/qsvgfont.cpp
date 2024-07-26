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

#include <qsvgfont_p.h>

#include <qdebug.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpicture.h>

QSvgGlyph::QSvgGlyph(QChar unicode, const QPainterPath &path, qreal horizAdvX)
   : m_unicode(unicode), m_path(path), m_horizAdvX(horizAdvX)
{
}

QSvgFont::QSvgFont(qreal horizAdvX)
   : m_horizAdvX(horizAdvX)
{
}

QString QSvgFont::familyName() const
{
   return m_familyName;
}

void QSvgFont::addGlyph(QChar unicode, const QPainterPath &path, qreal horizAdvX )
{
   m_glyphs.insert(unicode, QSvgGlyph(unicode, path,
                                      (horizAdvX == -1) ? m_horizAdvX : horizAdvX));
}


void QSvgFont::draw(QPainter *p, const QPointF &point, const QString &str, qreal pixelSize,
                    Qt::Alignment alignment) const
{
   p->save();
   p->translate(point);
   p->scale(pixelSize / m_unitsPerEm, -pixelSize / m_unitsPerEm);

   // Calculate the text width to be used for alignment
   int textWidth = 0;

   for (QChar unicode : str) {

      if (! m_glyphs.contains(unicode)) {
         unicode = 0;

         if (! m_glyphs.contains(unicode)) {
            continue;
         }
      }

      textWidth += static_cast<int>(m_glyphs[unicode].m_horizAdvX);
   }

   QPoint alignmentOffset(0, 0);
   if (alignment == Qt::AlignHCenter) {
      alignmentOffset.setX(-textWidth / 2);

   } else if (alignment == Qt::AlignRight) {
      alignmentOffset.setX(-textWidth);
   }

   p->translate(alignmentOffset);

   // since in SVG the embedded font is not really a path
   // the outline must stay untransformed

   qreal penWidth = p->pen().widthF();
   penWidth /= (pixelSize / m_unitsPerEm);

   QPen pen = p->pen();
   pen.setWidthF(penWidth);
   p->setPen(pen);

   for (QChar unicode : str) {

      if (! m_glyphs.contains(unicode)) {
         unicode = 0;

         if (! m_glyphs.contains(unicode)) {
            continue;
         }
      }
      p->drawPath(m_glyphs[unicode].m_path);
      p->translate(m_glyphs[unicode].m_horizAdvX, 0);
   }

   p->restore();
}

void QSvgFont::setFamilyName(const QString &name)
{
   m_familyName = name;
}

void QSvgFont::setUnitsPerEm(qreal upem)
{
   m_unitsPerEm = upem;
}

