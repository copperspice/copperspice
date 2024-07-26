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

#ifndef QSVGNODE_P_H
#define QSVGNODE_P_H

#include <qhash.h>
#include <qstring.h>

#include <qsvgstyle_p.h>

class QPainter;
class QSvgTinyDocument;

class QSvgNode
{
 public:
   enum Type {
      DOC,
      G,
      DEFS,
      SWITCH,
      ANIMATION,
      ARC,
      CIRCLE,
      ELLIPSE,
      IMAGE,
      LINE,
      PATH,
      POLYGON,
      POLYLINE,
      RECT,
      TEXT,
      TEXTAREA,
      TSPAN,
      USE,
      VIDEO
   };

   enum DisplayMode {
      InlineMode,
      BlockMode,
      ListItemMode,
      RunInMode,
      CompactMode,
      MarkerMode,
      TableMode,
      InlineTableMode,
      TableRowGroupMode,
      TableHeaderGroupMode,
      TableFooterGroupMode,
      TableRowMode,
      TableColumnGroupMode,
      TableColumnMode,
      TableCellMode,
      TableCaptionMode,
      NoneMode,
      InheritMode
   };

 public:
   QSvgNode(QSvgNode *parent = nullptr);
   virtual ~QSvgNode();
   virtual void draw(QPainter *p, QSvgExtraStates &states) = 0;

   QSvgNode *parent() const;
   bool isDescendantOf(const QSvgNode *parent) const;

   void appendStyleProperty(QSvgStyleProperty *prop, const QString &id);
   void applyStyle(QPainter *p, QSvgExtraStates &states) const;
   void revertStyle(QPainter *p, QSvgExtraStates &states) const;
   QSvgStyleProperty *styleProperty(QSvgStyleProperty::Type type) const;
   QSvgFillStyleProperty *styleProperty(const QString &id) const;

   QSvgTinyDocument *document() const;

   virtual Type type() const = 0;
   virtual QRectF bounds(QPainter *p, QSvgExtraStates &states) const;
   virtual QRectF transformedBounds(QPainter *p, QSvgExtraStates &states) const;
   QRectF transformedBounds() const;

   void setRequiredFeatures(const QStringList &lst);
   const QStringList &requiredFeatures() const;

   void setRequiredExtensions(const QStringList &lst);
   const QStringList &requiredExtensions() const;

   void setRequiredLanguages(const QStringList &lst);
   const QStringList &requiredLanguages() const;

   void setRequiredFormats(const QStringList &lst);
   const QStringList &requiredFormats() const;

   void setRequiredFonts(const QStringList &lst);
   const QStringList &requiredFonts() const;

   void setVisible(bool visible);
   bool isVisible() const;

   void setDisplayMode(DisplayMode display);
   DisplayMode displayMode() const;

   QString nodeId() const;
   void setNodeId(const QString &i);

   QString xmlClass() const;
   void setXmlClass(const QString &str);

 protected:
   mutable QSvgStyle m_style;

   static qreal strokeWidth(QPainter *p);

 private:
   QSvgNode   *m_parent;

   QStringList m_requiredFeatures;
   QStringList m_requiredExtensions;
   QStringList m_requiredLanguages;
   QStringList m_requiredFormats;
   QStringList m_requiredFonts;

   bool m_visible;

   QString m_id;
   QString m_class;

   DisplayMode m_displayMode;
   mutable QRectF m_cachedBounds;

   friend class QSvgTinyDocument;
};

inline QSvgNode *QSvgNode::parent() const
{
   return m_parent;
}

inline bool QSvgNode::isVisible() const
{
   return m_visible;
}

inline QString QSvgNode::nodeId() const
{
   return m_id;
}

inline QString QSvgNode::xmlClass() const
{
   return m_class;
}

#endif
