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

#ifndef QTEXTHTMLPARSER_P_H
#define QTEXTHTMLPARSER_P_H

#include <qbrush.h>
#include <qcolor.h>
#include <qfont.h>
#include <qtextcursor.h>
#include <qtextdocument.h>
#include <qvector.h>

#include <qcssparser_p.h>
#include <qtextdocument_p.h>
#include <qtextformat_p.h>

#ifndef QT_NO_TEXTHTMLPARSER

enum QTextHTMLElements {
   Html_unknown = -1,
   Html_qt = 0,
   Html_body,

   Html_a,
   Html_em,
   Html_i,
   Html_big,
   Html_small,
   Html_strong,
   Html_b,
   Html_cite,
   Html_address,
   Html_var,
   Html_dfn,

   Html_h1,
   Html_h2,
   Html_h3,
   Html_h4,
   Html_h5,
   Html_h6,
   Html_p,
   Html_center,

   Html_font,

   Html_ul,
   Html_ol,
   Html_li,

   Html_code,
   Html_tt,
   Html_kbd,
   Html_samp,

   Html_img,
   Html_br,
   Html_hr,

   Html_sub,
   Html_sup,

   Html_pre,
   Html_blockquote,
   Html_head,
   Html_div,
   Html_span,
   Html_dl,
   Html_dt,
   Html_dd,
   Html_u,
   Html_s,
   Html_nobr,

   // tables
   Html_table,
   Html_tr,
   Html_td,
   Html_th,
   Html_thead,
   Html_tbody,
   Html_tfoot,
   Html_caption,

   // misc...
   Html_html,
   Html_style,
   Html_title,
   Html_meta,
   Html_link,
   Html_script,

   Html_NumElements
};

struct QTextHtmlElement {
   const char name[11];
   QTextHTMLElements id;
   enum DisplayMode { DisplayBlock, DisplayInline, DisplayTable, DisplayNone } displayMode;
};

class QTextHtmlParser;

struct QTextHtmlParserNode {
   enum WhiteSpaceMode {
      WhiteSpaceNormal,
      WhiteSpacePre,
      WhiteSpaceNoWrap,
      WhiteSpacePreWrap,
      WhiteSpaceModeUndefined = -1
   };

   QTextHtmlParserNode();
   QString tag;
   QString text;
   QStringList attributes;

   int m_parserNodeParent;

   QVector<int> children;
   QTextHTMLElements id;
   QTextCharFormat charFormat;
   QTextBlockFormat blockFormat;
   uint cssFloat : 2;
   uint hasOwnListStyle : 1;
   uint hasCssListIndent : 1;
   uint isEmptyParagraph : 1;
   uint isTextFrame : 1;
   uint isRootFrame : 1;
   uint displayMode : 3; // QTextHtmlElement::DisplayMode
   uint hasHref : 1;
   QTextListFormat::Style listStyle;
   QString textListNumberPrefix;
   QString textListNumberSuffix;
   QString imageName;
   qreal imageWidth;
   qreal imageHeight;
   QTextLength width;
   QTextLength height;
   qreal tableBorder;
   int tableCellRowSpan;
   int tableCellColSpan;
   qreal tableCellSpacing;
   qreal tableCellPadding;
   QBrush borderBrush;
   QTextFrameFormat::BorderStyle borderStyle;
   int userState;

   int cssListIndent;

   WhiteSpaceMode wsm;

   bool isListStart() const {
      return id == Html_ol || id == Html_ul;
   }

   bool isTableCell() const {
      return id == Html_td || id == Html_th;
   }

   bool isBlock() const {
      return displayMode == QTextHtmlElement::DisplayBlock;
   }

   bool isNotSelfNesting() const {
      return id == Html_p || id == Html_li;
   }

   bool allowedInContext(int parentId) const {
      switch (id) {
         case Html_dd:
         case Html_dt:
            return (parentId == Html_dl);
         case Html_tr:
            return (parentId == Html_table
                  || parentId == Html_thead
                  || parentId == Html_tbody
                  || parentId == Html_tfoot
               );
         case Html_th:
         case Html_td:
            return (parentId == Html_tr);
         case Html_thead:
         case Html_tbody:
         case Html_tfoot:
            return (parentId == Html_table);
         case Html_caption:
            return (parentId == Html_table);
         case Html_body:
            return parentId != Html_head;
         default:
            break;
      }
      return true;
   }

   bool mayNotHaveChildren() const {
      return id == Html_img || id == Html_hr || id == Html_br || id == Html_meta;
   }

   void initializeProperties(const QTextHtmlParserNode *parent, const QTextHtmlParser *parser);

   int uncollapsedMargin(int mar) const {
      return margin[mar];
   }

   bool isNestedList(const QTextHtmlParser *parser) const;

   void parseStyleAttribute(const QString &value, const QTextDocument *resourceProvider);

#ifndef QT_NO_CSSPARSER
   void applyCssDeclarations(const QVector<QCss::Declaration> &declarations, const QTextDocument *resourceProvider);

   void setListStyle(const QVector<QCss::Value> &cssValues);
#endif

   void applyBackgroundImage(const QString &url, const QTextDocument *resourceProvider);

   bool hasOnlyWhitespace() const;

   int margin[4];
   int padding[4];

   friend class QTextHtmlParser;
};

class QTextHtmlParser
{
 public:
   enum Margin {
      MarginTop,
      MarginRight,
      MarginBottom,
      MarginLeft
   };

   const QTextHtmlParserNode &at(int i) const {
      return nodes.at(i);
   }

   QTextHtmlParserNode &operator[](int i) {
      return nodes[i];
   }

   int count() const {
      return nodes.count();
   }

   int last() const {
      return nodes.count() - 1;
   }

   int depth(int i) const;
   int topMargin(int i) const;
   int bottomMargin(int i) const;

   int leftMargin(int i) const {
      return margin(i, MarginLeft);
   }

   int rightMargin(int i) const {
      return margin(i, MarginRight);
   }

   int topPadding(int i) const {
      return at(i).padding[MarginTop];
   }

   int bottomPadding(int i) const {
      return at(i).padding[MarginBottom];
   }

   int leftPadding(int i) const {
      return at(i).padding[MarginLeft];
   }

   int rightPadding(int i) const {
      return at(i).padding[MarginRight];
   }

   void parse(const QString &text, const QTextDocument *resourceProvider);

   static int lookupElement(const QString &element);

 protected:
   void parse();
   void parseTag();
   void parseCloseTag();
   void parseExclamationTag();

   QString parseEntity();
   QString parseWord();

   QTextHtmlParserNode *resolveParent();
   void resolveNode();

   QStringList parseAttributes();
   void applyAttributes(const QStringList &attributes);
   void eatSpace();

   bool hasPrefix(QChar c, int lookahead = 0) const {
      return pos + lookahead < len && txt.at(pos) == c;
   }
   int margin(int i, int mar) const;

   bool nodeIsChildOf(int i, QTextHTMLElements id) const;

   QTextHtmlParserNode *newNode(int parent);
   QVector<QTextHtmlParserNode> nodes;

   QString txt;

   int pos;
   int len;

   bool m_textEditMode;

   const QTextDocument *m_resourceProvider;

#ifndef QT_NO_CSSPARSER
   QVector<QCss::Declaration> declarationsForNode(int node) const;
   void resolveStyleSheetImports(const QCss::StyleSheet &sheet);
   void importStyleSheet(const QString &href);

   struct ExternalStyleSheet {
      ExternalStyleSheet()
      { }

      ExternalStyleSheet(const QString &_url, const QCss::StyleSheet &_sheet)
         : url(_url), sheet(_sheet)
      { }

      QString url;
      QCss::StyleSheet sheet;
   };

   QVector<ExternalStyleSheet> externalStyleSheets;
   QVector<QCss::StyleSheet> inlineStyleSheets;
#endif

};

#endif // QT_NO_TEXTHTMLPARSER

#endif // QTEXTHTMLPARSER_P_H
