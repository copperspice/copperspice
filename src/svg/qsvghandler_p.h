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

#ifndef QSVGHANDLER_P_H
#define QSVGHANDLER_P_H

#include <qxmlstream.h>

#include <qhash.h>
#include <qstack.h>
#include <qsvgstyle_p.h>
#include <qcssparser_p.h>
#include <qsvggraphics_p.h>

class QSvgNode;
class QSvgTinyDocument;
class QSvgHandler;
class QColor;
class QSvgStyleSelector;
class QXmlStreamReader;

#ifndef QT_NO_CSSPARSER
struct QSvgCssAttribute {
   QString name;
   QString value;
};
#endif

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
      return document() != nullptr && !xml->hasError();
   }

   QString errorString() const {
      return xml->errorString();
   }

   int lineNumber() const {
      return xml->lineNumber();
   }

   void setDefaultCoordinateSystem(LengthType type);
   LengthType defaultCoordinateSystem() const;

   void pushColor(const QColor &color);
   void pushColorCopy();
   void popColor();
   QColor currentColor() const;

#ifndef QT_NO_CSSPARSER
   void setInStyle(bool b);
   bool inStyle() const;

   QSvgStyleSelector *selector() const;
#endif

   void setAnimPeriod(int start, int end);
   int animationDuration() const;

#ifndef QT_NO_CSSPARSER
   void parseCSStoXMLAttrs(QString css, QVector<QSvgCssAttribute> *attributes);
#endif

   inline QPen defaultPen() const {
      return m_defaultPen;
   }

 public:
   bool startElement(const QString &localName, const QXmlStreamAttributes &attributes);
   bool endElement(QStringView localName);
   bool characters(QStringView str);
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

#ifndef QT_NO_CSSPARSER
   bool m_inStyle;
   QSvgStyleSelector *m_selector;
   QCss::Parser m_cssParser;
#endif

   int m_animEnd;

   QXmlStreamReader *const xml;

   void parse();
   void resolveGradients(QSvgNode *node);

   QPen m_defaultPen;

   // Whether we own the variable xml, and hence whether we need to delete it.
   const bool m_ownsReader;
};


#endif
