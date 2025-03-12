/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QRENDERRULE_P_H
#define QRENDERRULE_P_H

#include <qpainter.h>
#include <qrect.h>
#include <qsharedpointer.h>
#include <qsize.h>
#include <qvector.h>
#include <qwidget.h>

#include <qcssparser_p.h>

struct QStyleSheetBorderImageData : public QSharedData
{
   QStyleSheetBorderImageData()
      : horizStretch(QCss::TileMode_Unknown), vertStretch(QCss::TileMode_Unknown)
   {
      for (int i = 0; i < 4; i++) {
         cuts[i] = -1;
      }
   }

   int cuts[4];
   QPixmap pixmap;
   QImage image;
   QCss::TileMode horizStretch, vertStretch;
};

struct QStyleSheetBackgroundData : public QSharedData {
   QStyleSheetBackgroundData(const QBrush &b, const QPixmap &p, QCss::Repeat r,
         Qt::Alignment a, QCss::Origin o, QCss::Attachment t, QCss::Origin c)
      : brush(b), pixmap(p), repeat(r), position(a), origin(o), attachment(t), clip(c) { }

   bool isTransparent() const {
      if (brush.style() != Qt::NoBrush) {
         return ! brush.isOpaque();
      }
      return pixmap.isNull() ? false : pixmap.hasAlpha();
   }

   QBrush brush;
   QPixmap pixmap;
   QCss::Repeat repeat;
   Qt::Alignment position;
   QCss::Origin origin;
   QCss::Attachment attachment;
   QCss::Origin clip;
};

struct QStyleSheetBorderData : public QSharedData {
   QStyleSheetBorderData()
      : bi(nullptr)
   {
      for (int i = 0; i < 4; i++) {
         borders[i] = 0;
         styles[i] = QCss::BorderStyle_None;
      }
   }

   QStyleSheetBorderData(int *b, QBrush *c, QCss::BorderStyle *s, QSize *r)
      : bi(nullptr)
   {
      for (int i = 0; i < 4; i++) {
         borders[i] = b[i];
         styles[i] = s[i];
         colors[i] = c[i];
         radii[i] = r[i];
      }
   }

   int borders[4];
   QBrush colors[4];
   QCss::BorderStyle styles[4];
   QSize radii[4]; // topleft, topright, bottomleft, bottomright

   const QStyleSheetBorderImageData *borderImage() const {
      return bi;
   }

   bool hasBorderImage() const {
      return bi != nullptr;
   }

   QSharedDataPointer<QStyleSheetBorderImageData> bi;

   bool isOpaque() const {
      for (int i = 0; i < 4; i++) {
         if (styles[i] == QCss::BorderStyle_Native || styles[i] == QCss::BorderStyle_None) {
            continue;
         }

         if (styles[i] >= QCss::BorderStyle_Dotted && styles[i] <= QCss::BorderStyle_DotDotDash
            && styles[i] != QCss::BorderStyle_Solid) {
            return false;
         }

         if (!colors[i].isOpaque()) {
            return false;
         }

         if (!radii[i].isEmpty()) {
            return false;
         }
      }

      if (bi != nullptr && bi->pixmap.hasAlpha()) {
         return false;
      }

      return true;
   }
};


struct QStyleSheetOutlineData : public QStyleSheetBorderData {
   QStyleSheetOutlineData() {
      for (int i = 0; i < 4; i++) {
         offsets[i] = 0;
      }
   }

   QStyleSheetOutlineData(int *b, QBrush *c, QCss::BorderStyle *s, QSize *r, int *o)
      : QStyleSheetBorderData(b, c, s, r) {
      for (int i = 0; i < 4; i++) {
         offsets[i] = o[i];
      }
   }

   int offsets[4];
};

struct QStyleSheetBoxData : public QSharedData {
   QStyleSheetBoxData(int *m, int *p, int s) : spacing(s) {
      for (int i = 0; i < 4; i++) {
         margins[i] = m[i];
         paddings[i] = p[i];
      }
   }

   int margins[4];
   int paddings[4];

   int spacing;
};

struct QStyleSheetPaletteData : public QSharedData {
   QStyleSheetPaletteData(const QBrush &fg, const QBrush &sfg, const QBrush &sbg, const QBrush &abg)
      : foreground(fg), selectionForeground(sfg), selectionBackground(sbg), alternateBackground(abg) { }

   QBrush foreground;
   QBrush selectionForeground;
   QBrush selectionBackground;
   QBrush alternateBackground;
};

struct QStyleSheetGeometryData : public QSharedData {
   QStyleSheetGeometryData(int w, int h, int minw, int minh, int maxw, int maxh)
      : minWidth(minw), minHeight(minh), width(w), height(h), maxWidth(maxw), maxHeight(maxh) { }

   int minWidth, minHeight, width, height, maxWidth, maxHeight;
};

struct QStyleSheetPositionData : public QSharedData {
   QStyleSheetPositionData(int l, int t, int r, int b, QCss::Origin o, Qt::Alignment p,
      QCss::PositionMode m, Qt::Alignment a = Qt::EmptyFlag)
      : left(l), top(t), bottom(b), right(r), origin(o), position(p), mode(m), textAlignment(a) { }

   int left, top, bottom, right;
   QCss::Origin origin;
   Qt::Alignment position;
   QCss::PositionMode mode;
   Qt::Alignment textAlignment;
};

struct QStyleSheetImageData : public QSharedData {
   QStyleSheetImageData(const QIcon &i, Qt::Alignment a, const QSize &sz)
      : icon(i), alignment(a), size(sz) { }

   QIcon icon;
   Qt::Alignment alignment;
   QSize size;
};

class QRenderRule
{
 public:
   enum RenderArea {
      Margin  = 1,
      Border  = 2,
      Padding = 4,
      All     = Margin | Border | Padding
   };

   QRenderRule()
      : clipset(0), features(0), hasFont(false),
        m_renderPalette(nullptr), m_renderBox(nullptr), m_renderBg(nullptr), m_renderBorder(nullptr),
        m_renderOutline(nullptr), m_renderGeometry(nullptr), m_renderPos(nullptr), m_renderImage(nullptr)
   {
   }

   QRenderRule(const QVector<QCss::Declaration> &, const QObject *);

   QRect borderRect(const QRect &r) const;
   QRect outlineRect(const QRect &r) const;
   QRect paddingRect(const QRect &r) const;
   QRect contentsRect(const QRect &r) const;

   QRect boxRect(const QRect &r, int flags = All) const;
   QSize boxSize(const QSize &s, int flags = All) const;
   QRect originRect(const QRect &rect, QCss::Origin origin) const;

   QPainterPath borderClip(QRect rect);
   void drawBorder(QPainter *, const QRect &);
   void drawOutline(QPainter *, const QRect &);
   void drawBorderImage(QPainter *, const QRect &);
   void drawBackground(QPainter *, const QRect &, const QPoint & = QPoint(0, 0));
   void drawBackgroundImage(QPainter *, const QRect &, QPoint = QPoint(0, 0));
   void drawFrame(QPainter *, const QRect &);
   void drawImage(QPainter *p, const QRect &rect);
   void drawRule(QPainter *, const QRect &);
   void configurePalette(QPalette *, QPalette::ColorGroup, const QWidget *, bool);
   void configurePalette(QPalette *p, QPalette::ColorRole fr, QPalette::ColorRole br);

   const QStyleSheetPaletteData *palette() const {
      return m_renderPalette;
   }

   const QStyleSheetBoxData *box() const {
      return m_renderBox;
   }

   const QStyleSheetBackgroundData *background() const {
      return m_renderBg;
   }

   const QStyleSheetBorderData *border() const {
      return m_renderBorder;
   }

   const QStyleSheetOutlineData *outline() const {
      return m_renderOutline;
   }

   const QStyleSheetGeometryData *geometry() const {
      return m_renderGeometry;
   }

   const QStyleSheetPositionData *position() const {
      return m_renderPos;
   }

   bool hasPalette() const {
      return m_renderPalette != nullptr;
   }

   bool hasBackground() const {
      return m_renderBg != nullptr && (! m_renderBg->pixmap.isNull() || m_renderBg->brush.style() != Qt::NoBrush);
   }

   bool hasGradientBackground() const {
      return m_renderBg && m_renderBg->brush.style() >= Qt::LinearGradientPattern &&
            m_renderBg->brush.style() <= Qt::ConicalGradientPattern;
   }

   bool hasNativeBorder() const {
      return m_renderBorder == nullptr ||
            (! m_renderBorder->hasBorderImage() && m_renderBorder->styles[0] == QCss::BorderStyle_Native);
   }

   bool hasNativeOutline() const {
      return (m_renderOutline == nullptr ||
            (! m_renderOutline->hasBorderImage() && m_renderOutline->styles[0] == QCss::BorderStyle_Native));
   }

   bool baseStyleCanDraw() const {
      if (! hasBackground() || (background()->brush.style() == Qt::NoBrush && m_renderBg->pixmap.isNull())) {
         return true;
      }

      if (m_renderBg && ! m_renderBg->pixmap.isNull()) {
         return false;
      }

      if (hasGradientBackground()) {
         return features & QCss::StyleFeature_BackgroundGradient;
      }

      return features & QCss::StyleFeature_BackgroundColor;
   }

   bool hasBox() const {
      return m_renderBox != nullptr;
   }

   bool hasBorder() const {
      return m_renderBorder !=nullptr;
   }

   bool hasOutline() const {
      return m_renderOutline != nullptr;
   }

   bool hasPosition() const {
      return m_renderPos != nullptr;
   }

   bool hasGeometry() const {
      return m_renderGeometry != nullptr;
   }

   bool hasDrawable() const {
      return ! hasNativeBorder() || hasBackground() || hasImage();
   }

   bool hasImage() const {
      return m_renderImage != nullptr;
   }

   QSize minimumContentsSize() const {
      return m_renderGeometry ? QSize(m_renderGeometry->minWidth, m_renderGeometry->minHeight) : QSize(0, 0);
   }

   QSize minimumSize() const {
      return boxSize(minimumContentsSize());
   }

   QSize contentsSize() const {
      return m_renderGeometry ? QSize(m_renderGeometry->width, m_renderGeometry->height) :
            ((m_renderImage && m_renderImage->size.isValid()) ? m_renderImage->size : QSize());
   }

   QSize contentsSize(const QSize &sz) const {
      QSize csz = contentsSize();

      if (csz.width() == -1) {
         csz.setWidth(sz.width());
      }

      if (csz.height() == -1) {
         csz.setHeight(sz.height());
      }

      return csz;
   }

   bool hasContentsSize() const {
      return (m_renderGeometry && (m_renderGeometry->width != -1 ||
            m_renderGeometry->height != -1)) || (m_renderImage && m_renderImage->size.isValid());
   }

   QSize size() const {
      return boxSize(contentsSize());
   }

   QSize size(const QSize &sz) const {
      return boxSize(contentsSize(sz));
   }

   QSize adjustSize(const QSize &sz) {
      if (! m_renderGeometry) {
         return sz;
      }

      QSize csz = contentsSize();

      if (csz.width() == -1) {
         csz.setWidth(sz.width());
      }

      if (csz.height() == -1) {
         csz.setHeight(sz.height());
      }

      if (m_renderGeometry->maxWidth != -1 && csz.width() > m_renderGeometry->maxWidth) {
         csz.setWidth(m_renderGeometry->maxWidth);
      }

      if (m_renderGeometry->maxHeight != -1 && csz.height() > m_renderGeometry->maxHeight) {
         csz.setHeight(m_renderGeometry->maxHeight);
      }

      csz = csz.expandedTo(QSize(m_renderGeometry->minWidth, m_renderGeometry->minHeight));

      return csz;
   }

   bool hasStyleHint(const QString &sh) const {
      return styleHints.contains(sh);
   }

   QVariant styleHint(const QString &sh) const {
      return styleHints.value(sh);
   }

   void fixupBorder(int);
   void setClip(QPainter *p, const QRect &rect);
   void unsetClip(QPainter *);

   int clipset;
   int features;

   bool hasFont;

   QBrush defaultBackground;
   QFont font;
   QPainterPath clipPath;

   QHash<QString, QVariant> styleHints;

   QSharedDataPointer<QStyleSheetPaletteData> m_renderPalette;
   QSharedDataPointer<QStyleSheetBoxData> m_renderBox;
   QSharedDataPointer<QStyleSheetBackgroundData> m_renderBg;
   QSharedDataPointer<QStyleSheetBorderData> m_renderBorder;
   QSharedDataPointer<QStyleSheetOutlineData> m_renderOutline;
   QSharedDataPointer<QStyleSheetGeometryData> m_renderGeometry;
   QSharedDataPointer<QStyleSheetPositionData> m_renderPos;
   QSharedDataPointer<QStyleSheetImageData> m_renderImage;
};
#endif
