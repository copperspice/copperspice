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

#ifndef QDECLARATIVETEXT_P_P_H
#define QDECLARATIVETEXT_P_P_H

#include <qdeclarativeitem.h>
#include <qdeclarativeimplicitsizeitem_p_p.h>
#include <qdeclarativetextlayout_p.h>
#include <qdeclarative.h>
#include <QtGui/qtextlayout.h>

QT_BEGIN_NAMESPACE

class QTextLayout;
class QTextDocumentWithImageResources;

class QDeclarativeTextPrivate : public QDeclarativeImplicitSizeItemPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeText)
 public:
   QDeclarativeTextPrivate();

   ~QDeclarativeTextPrivate();

   void updateSize();
   void updateLayout();
   bool determineHorizontalAlignment();
   bool setHAlign(QDeclarativeText::HAlignment, bool forceAlign = false);
   void mirrorChange();
   QTextDocument *textDocument();

   QString text;
   QFont font;
   QFont sourceFont;
   QColor  color;
   QDeclarativeText::TextStyle style;
   QColor  styleColor;
   QString activeLink;
   QDeclarativeText::HAlignment hAlign;
   QDeclarativeText::VAlignment vAlign;
   QDeclarativeText::TextElideMode elideMode;
   QDeclarativeText::TextFormat format;
   QDeclarativeText::WrapMode wrapMode;
   qreal lineHeight;
   QDeclarativeText::LineHeightMode lineHeightMode;
   int lineCount;
   bool truncated;
   int maximumLineCount;
   int maximumLineCountValid;
   QPointF elidePos;

   static QString elideChar;

   void invalidateImageCache();
   void checkImageCache();
   QPixmap imageCache;

   bool imageCacheDirty: 1;
   bool updateOnComponentComplete: 1;
   bool richText: 1;
   bool singleline: 1;
   bool cacheAllTextAsImage: 1;
   bool internalWidthUpdate: 1;
   bool requireImplicitWidth: 1;
   bool hAlignImplicit: 1;
   bool rightToLeftText: 1;
   bool layoutTextElided: 1;

   QRect layedOutTextRect;
   QSize paintedSize;
   qreal naturalWidth;
   virtual qreal implicitWidth() const;
   void ensureDoc();
   QPixmap textDocumentImage(bool drawStyle);
   QTextDocumentWithImageResources *doc;

   QRect setupTextLayout();
   QPixmap textLayoutImage(bool drawStyle);
   void drawTextLayout(QPainter *p, const QPointF &pos, bool drawStyle);
   QDeclarativeTextLayout layout;

   static QPixmap drawOutline(const QPixmap &source, const QPixmap &styleSource);
   static QPixmap drawOutline(const QPixmap &source, const QPixmap &styleSource, int yOffset);

   static inline QDeclarativeTextPrivate *get(QDeclarativeText *t) {
      return t->d_func();
   }
};

QT_END_NAMESPACE
#endif
