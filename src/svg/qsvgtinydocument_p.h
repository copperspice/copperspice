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

#ifndef QSVGTINYDOCUMENT_P_H
#define QSVGTINYDOCUMENT_P_H

#include <qsvgstructure_p.h>

#include <qrect.h>
#include <qlist.h>
#include <qhash.h>
#include <qdatetime.h>
#include <qxmlstream.h>
#include <qsvgstyle_p.h>
#include <qsvgfont_p.h>

class QPainter;
class QByteArray;
class QSvgFont;

class Q_SVG_EXPORT QSvgTinyDocument : public QSvgStructureNode
{
 public:
   static QSvgTinyDocument *load(const QString &file);
   static QSvgTinyDocument *load(const QByteArray &contents);
   static QSvgTinyDocument *load(QXmlStreamReader *contents);
 public:
   QSvgTinyDocument();
   ~QSvgTinyDocument();

   Type type() const override;

   QSize size() const;
   void setWidth(int len, bool percent);
   void setHeight(int len, bool percent);
   int width() const;
   int height() const;
   bool widthPercent() const;
   bool heightPercent() const;

   bool preserveAspectRatio() const;

   QRectF viewBox() const;
   void setViewBox(const QRectF &rect);

   void draw(QPainter *p, QSvgExtraStates &) override;      // from the QSvgNode

   void draw(QPainter *p);
   void draw(QPainter *p, const QRectF &bounds);
   void draw(QPainter *p, const QString &id, const QRectF &bounds = QRectF());

   QMatrix matrixForElement(const QString &id) const;
   QRectF boundsOnElement(const QString &id) const;
   bool   elementExists(const QString &id) const;

   void addSvgFont(QSvgFont *);
   QSvgFont *svgFont(const QString &family) const;
   void addNamedNode(const QString &id, QSvgNode *node);
   QSvgNode *namedNode(const QString &id) const;
   void addNamedStyle(const QString &id, QSvgFillStyleProperty *style);
   QSvgFillStyleProperty *namedStyle(const QString &id) const;

   void restartAnimation();
   int currentElapsed() const;
   bool animated() const;
   void setAnimated(bool a);
   int animationDuration() const;
   int currentFrame() const;
   void setCurrentFrame(int);
   void setFramesPerSecond(int num);

 private:
   void mapSourceToTarget(QPainter *p, const QRectF &targetRect, const QRectF &sourceRect = QRectF());

   QSize  m_size;
   bool   m_widthPercent;
   bool   m_heightPercent;

   mutable QRectF m_viewBox;

   QHash<QString, QSvgRefCounter<QSvgFont> > m_fonts;
   QHash<QString, QSvgNode *> m_namedNodes;
   QHash<QString, QSvgRefCounter<QSvgFillStyleProperty> > m_namedStyles;

   QTime m_time;
   bool  m_animated;
   int   m_animationDuration;
   int   m_fps;

   QSvgExtraStates m_states;
};

inline QSize QSvgTinyDocument::size() const
{
   if (m_size.isEmpty()) {
      return viewBox().size().toSize();
   } else {
      return m_size;
   }
}

inline int QSvgTinyDocument::width() const
{
   return size().width();
}

inline int QSvgTinyDocument::height() const
{
   return size().height();
}

inline bool QSvgTinyDocument::widthPercent() const
{
   return m_widthPercent;
}

inline bool QSvgTinyDocument::heightPercent() const
{
   return m_heightPercent;
}

inline QRectF QSvgTinyDocument::viewBox() const
{
   if (m_viewBox.isNull()) {
      m_viewBox = transformedBounds();
   }

   return m_viewBox;
}

inline bool QSvgTinyDocument::preserveAspectRatio() const
{
   return false;
}

inline int QSvgTinyDocument::currentElapsed() const
{
   return m_time.elapsed();
}

inline int QSvgTinyDocument::animationDuration() const
{
   return m_animationDuration;
}


#endif
