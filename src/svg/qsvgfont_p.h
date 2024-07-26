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

#ifndef QSVGFONT_P_H
#define QSVGFONT_P_H

#include <qhash.h>
#include <qpainterpath.h>
#include <qstring.h>

#include <qsvgstyle_p.h>

class QSvgGlyph
{
 public:
   QSvgGlyph(QChar unicode, const QPainterPath &path, qreal horizAdvX);
   QSvgGlyph() : m_unicode(0), m_horizAdvX(0) {}

   QChar m_unicode;
   QPainterPath m_path;
   qreal m_horizAdvX;
};

class QSvgFont : public QSvgRefCounted
{
 public:
   QSvgFont(qreal horizAdvX);

   void setFamilyName(const QString &name);
   QString familyName() const;

   void setUnitsPerEm(qreal upem);
   void addGlyph(QChar unicode, const QPainterPath &path, qreal horizAdvX = -1);
   void draw(QPainter *p, const QPointF &point, const QString &str, qreal pixelSize, Qt::Alignment alignment) const;

 public:
   QString m_familyName;
   qreal m_unitsPerEm;
   qreal m_ascent;
   qreal m_descent;
   qreal m_horizAdvX;
   QHash<QChar, QSvgGlyph> m_glyphs;
};

#endif
