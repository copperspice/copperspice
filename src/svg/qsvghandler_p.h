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

#ifndef QSVGHANDLER_P_H
#define QSVGHANDLER_P_H

#include "QtXml/qxmlstream.h"

#ifndef QT_NO_SVG

#include "QtCore/qhash.h"
#include "QtCore/qstack.h"
#include "qsvgstyle_p.h"
#include "qcssparser_p.h"
#include "qsvggraphics_p.h"

QT_BEGIN_NAMESPACE

class QSvgNode;
class QSvgTinyDocument;
class QSvgHandler;
class QColor;
class QSvgStyleSelector;
class QXmlStreamReader;

struct QSvgCssAttribute {
   QXmlStreamStringRef name;
   QXmlStreamStringRef value;
};

class QSvgHandler
{
 public:
   enum LengthType {
      LT_PERCENT,
      LT_PX,
      LT_PC,
      LT_PT,
      LT_MM,
      LT_CM,
      LT_IN,
      LT_OTHER
   };

 public:
   QSvgHandler(QIODevice *device);
   QSvgHandler(const QByteArray &data);
   QSvgHandler(QXmlStreamReader *const data);
   ~QSvgHandler();

   QSvgTinyDocument *document() const;

   inline bool ok() const {
      return document() != 0 && !xml->hasError();
   }

   inline QString errorString() const {
      return xml->errorString();
   }
   inline int lineNumber() const {
      return xml->lineNumber();
   }

   void setDefaultCoordinateSystem(LengthType type);
   LengthType defaultCoordinateSystem() const;

   void pushColor(const QColor &color);
   void pushColorCopy();
   void popColor();
   QColor currentColor() const;

   void setInStyle(bool b);
   bool inStyle() const;

   QSvgStyleSelector *selector() const;

   void setAnimPeriod(int start, int end);
   int animationDuration() const;

   void parseCSStoXMLAttrs(QString css, QVector<QSvgCssAttribute> *attributes);

   inline QPen defaultPen() const {
      return m_defaultPen;
   }

 public:
   bool startElement(const QString &localName, const QXmlStreamAttributes &attributes);
   bool endElement(const QStringRef &localName);
   bool characters(const QStringRef &str);
   bool processingInstruction(const QString &target, const QString &data);

 private:
   void init();

   QSvgTinyDocument *m_doc;
   QStack<QSvgNode *> m_nodes;

   QList<QSvgNode *>  m_resolveNodes;

   enum CurrentNode {
      Unknown,
      Graphics,
      Style
   };
   QStack<CurrentNode> m_skipNodes;

   QStack<QSvgText::WhitespaceMode>  m_whitespaceMode;
   QSvgRefCounter<QSvgStyleProperty> m_style;

   LengthType m_defaultCoords;

   QStack<QColor> m_colorStack;
   QStack<int>    m_colorTagCount;

   bool m_inStyle;

   QSvgStyleSelector *m_selector;

   int m_animEnd;

   QXmlStreamReader *const xml;
   QCss::Parser m_cssParser;
   void parse();
   void resolveGradients(QSvgNode *node);

   QPen m_defaultPen;
   /**
    * Whether we own the variable xml, and hence whether
    * we need to delete it.
    */
   const bool m_ownsReader;
};

QT_END_NAMESPACE

#endif // QT_NO_SVG
#endif // QSVGHANDLER_P_H
