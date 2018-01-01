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

#include "qplatformdefs.h"
#include "qsvghandler_p.h"

#ifndef QT_NO_SVG

#include "qsvgtinydocument_p.h"
#include "qsvgstructure_p.h"
#include "qsvggraphics_p.h"
#include "qsvgnode_p.h"
#include "qsvgfont_p.h"

#include "qapplication.h"
#include "qwidget.h"
#include "qpen.h"
#include "qpainterpath.h"
#include "qbrush.h"
#include "qcolor.h"
#include "qtextformat.h"
#include "qvector.h"
#include "qfileinfo.h"
#include "qfile.h"
#include "qdebug.h"
#include "qmath.h"
#include "qnumeric.h"
#include "qvarlengtharray.h"
#include "qmath_p.h"

#include "float.h"

QT_BEGIN_NAMESPACE

static const char *qt_inherit_text = "inherit";
#define QT_INHERIT QLatin1String(qt_inherit_text)

Q_CORE_EXPORT double qstrtod(const char *s00, char const **se, bool *ok);

// ======== duplicated from qcolor_p

static inline int qsvg_h2i(char hex)
{
   if (hex >= '0' && hex <= '9') {
      return hex - '0';
   }
   if (hex >= 'a' && hex <= 'f') {
      return hex - 'a' + 10;
   }
   if (hex >= 'A' && hex <= 'F') {
      return hex - 'A' + 10;
   }
   return -1;
}

static inline int qsvg_hex2int(const char *s)
{
   return (qsvg_h2i(s[0]) << 4) | qsvg_h2i(s[1]);
}

static inline int qsvg_hex2int(char s)
{
   int h = qsvg_h2i(s);
   return (h << 4) | h;
}

bool qsvg_get_hex_rgb(const char *name, QRgb *rgb)
{
   if (name[0] != '#') {
      return false;
   }
   name++;
   int len = qstrlen(name);
   int r, g, b;
   if (len == 12) {
      r = qsvg_hex2int(name);
      g = qsvg_hex2int(name + 4);
      b = qsvg_hex2int(name + 8);
   } else if (len == 9) {
      r = qsvg_hex2int(name);
      g = qsvg_hex2int(name + 3);
      b = qsvg_hex2int(name + 6);
   } else if (len == 6) {
      r = qsvg_hex2int(name);
      g = qsvg_hex2int(name + 2);
      b = qsvg_hex2int(name + 4);
   } else if (len == 3) {
      r = qsvg_hex2int(name[0]);
      g = qsvg_hex2int(name[1]);
      b = qsvg_hex2int(name[2]);
   } else {
      r = g = b = -1;
   }
   if ((uint)r > 255 || (uint)g > 255 || (uint)b > 255) {
      *rgb = 0;
      return false;
   }
   *rgb = qRgb(r, g , b);
   return true;
}

bool qsvg_get_hex_rgb(const QChar *str, int len, QRgb *rgb)
{
   if (len > 13) {
      return false;
   }
   char tmp[16];
   for (int i = 0; i < len; ++i) {
      tmp[i] = str[i].toLatin1();
   }
   tmp[len] = 0;
   return qsvg_get_hex_rgb(tmp, rgb);
}

// ======== end of qcolor_p duplicate

static bool parsePathDataFast(const QStringRef &data, QPainterPath &path);

static inline QString someId(const QXmlStreamAttributes &attributes)
{
   QString id = attributes.value(QLatin1String("id")).toString();
   if (id.isEmpty()) {
      id = attributes.value(QLatin1String("xml:id")).toString();
   }
   return id;
}

struct QSvgAttributes {
   QSvgAttributes(const QXmlStreamAttributes &xmlAttributes, QSvgHandler *handler);

   QString id;

   QStringRef color;
   QStringRef colorOpacity;
   QStringRef fill;
   QStringRef fillRule;
   QStringRef fillOpacity;
   QStringRef stroke;
   QStringRef strokeDashArray;
   QStringRef strokeDashOffset;
   QStringRef strokeLineCap;
   QStringRef strokeLineJoin;
   QStringRef strokeMiterLimit;
   QStringRef strokeOpacity;
   QStringRef strokeWidth;
   QStringRef vectorEffect;
   QStringRef fontFamily;
   QStringRef fontSize;
   QStringRef fontStyle;
   QStringRef fontWeight;
   QStringRef fontVariant;
   QStringRef textAnchor;
   QStringRef transform;
   QStringRef visibility;
   QStringRef opacity;
   QStringRef compOp;
   QStringRef display;
   QStringRef offset;
   QStringRef stopColor;
   QStringRef stopOpacity;

   QVector<QSvgCssAttribute> m_cssAttributes;
};

QSvgAttributes::QSvgAttributes(const QXmlStreamAttributes &xmlAttributes, QSvgHandler *handler)
{
   QStringRef style = xmlAttributes.value(QLatin1String("style"));
   if (!style.isEmpty()) {
      handler->parseCSStoXMLAttrs(style.toString(), &m_cssAttributes);
      for (int j = 0; j < m_cssAttributes.count(); ++j) {
         const QSvgCssAttribute &attribute = m_cssAttributes.at(j);
         QStringRef name = attribute.name;
         QStringRef value = attribute.value;
         if (name.isEmpty()) {
            continue;
         }

         switch (name.at(0).unicode()) {

            case 'c':
               if (name == QLatin1String("color")) {
                  color = value;
               } else if (name == QLatin1String("color-opacity")) {
                  colorOpacity = value;
               } else if (name == QLatin1String("comp-op")) {
                  compOp = value;
               }
               break;

            case 'd':
               if (name == QLatin1String("display")) {
                  display = value;
               }
               break;

            case 'f':
               if (name == QLatin1String("fill")) {
                  fill = value;
               } else if (name == QLatin1String("fill-rule")) {
                  fillRule = value;
               } else if (name == QLatin1String("fill-opacity")) {
                  fillOpacity = value;
               } else if (name == QLatin1String("font-family")) {
                  fontFamily = value;
               } else if (name == QLatin1String("font-size")) {
                  fontSize = value;
               } else if (name == QLatin1String("font-style")) {
                  fontStyle = value;
               } else if (name == QLatin1String("font-weight")) {
                  fontWeight = value;
               } else if (name == QLatin1String("font-variant")) {
                  fontVariant = value;
               }
               break;

            case 'o':
               if (name == QLatin1String("opacity")) {
                  opacity = value;
               } else if (name == QLatin1String("offset")) {
                  offset = value;
               }
               break;

            case 's':
               if (name.length() > 5 && QStringRef(name.string(), name.position() + 1, 5) == QLatin1String("troke")) {
                  QStringRef strokeRef(name.string(), name.position() + 6, name.length() - 6);
                  if (strokeRef.isEmpty()) {
                     stroke = value;
                  } else if (strokeRef == QLatin1String("-dasharray")) {
                     strokeDashArray = value;
                  } else if (strokeRef == QLatin1String("-dashoffset")) {
                     strokeDashOffset = value;
                  } else if (strokeRef == QLatin1String("-linecap")) {
                     strokeLineCap = value;
                  } else if (strokeRef == QLatin1String("-linejoin")) {
                     strokeLineJoin = value;
                  } else if (strokeRef == QLatin1String("-miterlimit")) {
                     strokeMiterLimit = value;
                  } else if (strokeRef == QLatin1String("-opacity")) {
                     strokeOpacity = value;
                  } else if (strokeRef == QLatin1String("-width")) {
                     strokeWidth = value;
                  }
               } else if (name == QLatin1String("stop-color")) {
                  stopColor = value;
               } else if (name == QLatin1String("stop-opacity")) {
                  stopOpacity = value;
               }
               break;

            case 't':
               if (name == QLatin1String("text-anchor")) {
                  textAnchor = value;
               } else if (name == QLatin1String("transform")) {
                  transform = value;
               }
               break;

            case 'v':
               if (name == QLatin1String("vector-effect")) {
                  vectorEffect = value;
               } else if (name == QLatin1String("visibility")) {
                  visibility = value;
               }
               break;

            default:
               break;
         }
      }
   }

   for (int i = 0; i < xmlAttributes.count(); ++i) {
      const QXmlStreamAttribute &attribute = xmlAttributes.at(i);
      QStringRef name = attribute.qualifiedName();
      if (name.isEmpty()) {
         continue;
      }
      QStringRef value = attribute.value();

      switch (name.at(0).unicode()) {

         case 'c':
            if (name == QLatin1String("color")) {
               color = value;
            } else if (name == QLatin1String("color-opacity")) {
               colorOpacity = value;
            } else if (name == QLatin1String("comp-op")) {
               compOp = value;
            }
            break;

         case 'd':
            if (name == QLatin1String("display")) {
               display = value;
            }
            break;

         case 'f':
            if (name == QLatin1String("fill")) {
               fill = value;
            } else if (name == QLatin1String("fill-rule")) {
               fillRule = value;
            } else if (name == QLatin1String("fill-opacity")) {
               fillOpacity = value;
            } else if (name == QLatin1String("font-family")) {
               fontFamily = value;
            } else if (name == QLatin1String("font-size")) {
               fontSize = value;
            } else if (name == QLatin1String("font-style")) {
               fontStyle = value;
            } else if (name == QLatin1String("font-weight")) {
               fontWeight = value;
            } else if (name == QLatin1String("font-variant")) {
               fontVariant = value;
            }
            break;

         case 'i':
            if (name == QLatin1String("id")) {
               id = value.toString();
            }
            break;

         case 'o':
            if (name == QLatin1String("opacity")) {
               opacity = value;
            }
            if (name == QLatin1String("offset")) {
               offset = value;
            }
            break;

         case 's':
            if (name.length() > 5 && QStringRef(name.string(), name.position() + 1, 5) == QLatin1String("troke")) {
               QStringRef strokeRef(name.string(), name.position() + 6, name.length() - 6);
               if (strokeRef.isEmpty()) {
                  stroke = value;
               } else if (strokeRef == QLatin1String("-dasharray")) {
                  strokeDashArray = value;
               } else if (strokeRef == QLatin1String("-dashoffset")) {
                  strokeDashOffset = value;
               } else if (strokeRef == QLatin1String("-linecap")) {
                  strokeLineCap = value;
               } else if (strokeRef == QLatin1String("-linejoin")) {
                  strokeLineJoin = value;
               } else if (strokeRef == QLatin1String("-miterlimit")) {
                  strokeMiterLimit = value;
               } else if (strokeRef == QLatin1String("-opacity")) {
                  strokeOpacity = value;
               } else if (strokeRef == QLatin1String("-width")) {
                  strokeWidth = value;
               }
            } else if (name == QLatin1String("stop-color")) {
               stopColor = value;
            } else if (name == QLatin1String("stop-opacity")) {
               stopOpacity = value;
            }
            break;

         case 't':
            if (name == QLatin1String("text-anchor")) {
               textAnchor = value;
            } else if (name == QLatin1String("transform")) {
               transform = value;
            }
            break;

         case 'v':
            if (name == QLatin1String("vector-effect")) {
               vectorEffect = value;
            } else if (name == QLatin1String("visibility")) {
               visibility = value;
            }
            break;

         case 'x':
            if (name == QLatin1String("xml:id") && id.isEmpty()) {
               id = value.toString();
            }
            break;

         default:
            break;
      }
   }

}

static const char *QSvgStyleSelector_nodeString[] = {
   "svg",
   "g",
   "defs",
   "switch",
   "animation",
   "arc",
   "circle",
   "ellipse",
   "image",
   "line",
   "path",
   "polygon",
   "polyline",
   "rect",
   "text",
   "textarea",
   "use",
   "video"
};

class QSvgStyleSelector : public QCss::StyleSelector
{
 public:
   QSvgStyleSelector() {
      nameCaseSensitivity = Qt::CaseInsensitive;
   }
   virtual ~QSvgStyleSelector() {
   }

   inline QString nodeToName(QSvgNode *node) const {
      return QLatin1String(QSvgStyleSelector_nodeString[node->type()]);
   }

   inline QSvgNode *svgNode(NodePtr node) const {
      return (QSvgNode *)node.ptr;
   }
   inline QSvgStructureNode *nodeToStructure(QSvgNode *n) const {
      if (n &&
            (n->type() == QSvgNode::DOC ||
             n->type() == QSvgNode::G ||
             n->type() == QSvgNode::DEFS ||
             n->type() == QSvgNode::SWITCH)) {
         return (QSvgStructureNode *)n;
      }
      return 0;
   }

   inline QSvgStructureNode *svgStructure(NodePtr node) const {
      QSvgNode *n = svgNode(node);
      QSvgStructureNode *st = nodeToStructure(n);
      return st;
   }

   bool nodeNameEquals(NodePtr node, const QString &nodeName) const  override{
      QSvgNode *n = svgNode(node);
      if (!n) {
         return false;
      }
      QString name = nodeToName(n);
      return QString::compare(name, nodeName, Qt::CaseInsensitive) == 0;
   }

   QString attribute(NodePtr node, const QString &name) const override {
      QSvgNode *n = svgNode(node);
      if ((!n->nodeId().isEmpty() && (name == QLatin1String("id") ||
                                      name == QLatin1String("xml:id")))) {
         return n->nodeId();
      }
      if (!n->xmlClass().isEmpty() && name == QLatin1String("class")) {
         return n->xmlClass();
      }
      return QString();
   }

   bool hasAttributes(NodePtr node) const override {
      QSvgNode *n = svgNode(node);
      return (n &&
              (!n->nodeId().isEmpty() || !n->xmlClass().isEmpty()));
   }

   QStringList nodeIds(NodePtr node) const override {
      QSvgNode *n = svgNode(node);
      QString nid;
      if (n) {
         nid = n->nodeId();
      }
      QStringList lst;
      lst.append(nid);
      return lst;
   }

   QStringList nodeNames(NodePtr node) const override {
      QSvgNode *n = svgNode(node);
      if (n) {
         return QStringList(nodeToName(n));
      }
      return QStringList();
   }

   bool isNullNode(NodePtr node) const override {
      return !node.ptr;
   }

   NodePtr parentNode(NodePtr node) const override {
      QSvgNode *n = svgNode(node);
      NodePtr newNode;
      newNode.ptr = 0;
      newNode.id = 0;
      if (n) {
         QSvgNode *svgParent = n->parent();
         if (svgParent) {
            newNode.ptr = svgParent;
         }
      }
      return newNode;
   }

   NodePtr previousSiblingNode(NodePtr node) const override {
      NodePtr newNode;
      newNode.ptr = 0;
      newNode.id = 0;

      QSvgNode *n = svgNode(node);
      if (!n) {
         return newNode;
      }
      QSvgStructureNode *svgParent = nodeToStructure(n->parent());

      if (svgParent) {
         newNode.ptr = svgParent->previousSiblingNode(n);
      }
      return newNode;
   }

   NodePtr duplicateNode(NodePtr node) const override {
      NodePtr n;
      n.ptr = node.ptr;
      n.id  = node.id;
      return n;
   }

   void freeNode(NodePtr node) const override {
      Q_UNUSED(node);
   }
};

// '0' is 0x30 and '9' is 0x39
static inline bool isDigit(ushort ch)
{
   static quint16 magic = 0x3ff;
   return ((ch >> 4) == 3) && (magic >> (ch & 15));
}

static qreal toDouble(const QChar *&str)
{
   const int maxLen = 255;//technically doubles can go til 308+ but whatever
   char temp[maxLen + 1];
   int pos = 0;

   if (*str == QLatin1Char('-')) {
      temp[pos++] = '-';
      ++str;
   } else if (*str == QLatin1Char('+')) {
      ++str;
   }
   while (isDigit(str->unicode()) && pos < maxLen) {
      temp[pos++] = str->toLatin1();
      ++str;
   }
   if (*str == QLatin1Char('.') && pos < maxLen) {
      temp[pos++] = '.';
      ++str;
   }
   while (isDigit(str->unicode()) && pos < maxLen) {
      temp[pos++] = str->toLatin1();
      ++str;
   }
   bool exponent = false;
   if ((*str == QLatin1Char('e') || *str == QLatin1Char('E')) && pos < maxLen) {
      exponent = true;
      temp[pos++] = 'e';
      ++str;
      if ((*str == QLatin1Char('-') || *str == QLatin1Char('+')) && pos < maxLen) {
         temp[pos++] = str->toLatin1();
         ++str;
      }
      while (isDigit(str->unicode()) && pos < maxLen) {
         temp[pos++] = str->toLatin1();
         ++str;
      }
   }

   temp[pos] = '\0';

   qreal val;
   if (!exponent && pos < 10) {
      int ival = 0;
      const char *t = temp;
      bool neg = false;
      if (*t == '-') {
         neg = true;
         ++t;
      }
      while (*t && *t != '.') {
         ival *= 10;
         ival += (*t) - '0';
         ++t;
      }
      if (*t == '.') {
         ++t;
         int div = 1;
         while (*t) {
            ival *= 10;
            ival += (*t) - '0';
            div *= 10;
            ++t;
         }
         val = ((qreal)ival) / ((qreal)div);
      } else {
         val = ival;
      }
      if (neg) {
         val = -val;
      }
   } else {
#if defined(Q_WS_QWS)
      if (sizeof(qreal) == sizeof(float)) {
         val = strtof(temp, 0);
      } else
#endif
      {
         bool ok = false;
         val = qstrtod(temp, 0, &ok);
      }
   }
   return val;

}
static qreal toDouble(const QString &str, bool *ok = NULL)
{
   const QChar *c = str.constData();
   qreal res = toDouble(c);
   if (ok) {
      *ok = ((*c) == QLatin1Char('\0'));
   }
   return res;
}

static qreal toDouble(const QStringRef &str, bool *ok = NULL)
{
   const QChar *c = str.constData();
   qreal res = toDouble(c);
   if (ok) {
      *ok = (c == (str.constData() + str.length()));
   }
   return res;
}

static QVector<qreal> parseNumbersList(const QChar *&str)
{
   QVector<qreal> points;
   if (!str) {
      return points;
   }
   points.reserve(32);

   while (str->isSpace()) {
      ++str;
   }
   while (isDigit(str->unicode()) ||
          *str == QLatin1Char('-') || *str == QLatin1Char('+') ||
          *str == QLatin1Char('.')) {

      points.append(toDouble(str));

      while (str->isSpace()) {
         ++str;
      }
      if (*str == QLatin1Char(',')) {
         ++str;
      }

      //eat the rest of space
      while (str->isSpace()) {
         ++str;
      }
   }

   return points;
}

static inline void parseNumbersArray(const QChar *&str, QVarLengthArray<qreal, 8> &points)
{
   while (str->isSpace()) {
      ++str;
   }
   while (isDigit(str->unicode()) ||
          *str == QLatin1Char('-') || *str == QLatin1Char('+') ||
          *str == QLatin1Char('.')) {

      points.append(toDouble(str));

      while (str->isSpace()) {
         ++str;
      }
      if (*str == QLatin1Char(',')) {
         ++str;
      }

      //eat the rest of space
      while (str->isSpace()) {
         ++str;
      }
   }
}

static QVector<qreal> parsePercentageList(const QChar *&str)
{
   QVector<qreal> points;
   if (!str) {
      return points;
   }

   while (str->isSpace()) {
      ++str;
   }
   while ((*str >= QLatin1Char('0') && *str <= QLatin1Char('9')) ||
          *str == QLatin1Char('-') || *str == QLatin1Char('+') ||
          *str == QLatin1Char('.')) {

      points.append(toDouble(str));

      while (str->isSpace()) {
         ++str;
      }
      if (*str == QLatin1Char('%')) {
         ++str;
      }
      while (str->isSpace()) {
         ++str;
      }
      if (*str == QLatin1Char(',')) {
         ++str;
      }

      //eat the rest of space
      while (str->isSpace()) {
         ++str;
      }
   }

   return points;
}

static QString idFromUrl(const QString &url)
{
   QString::const_iterator itr = url.constBegin();
   while ((*itr).isSpace()) {
      ++itr;
   }
   if ((*itr) == QLatin1Char('(')) {
      ++itr;
   }
   while ((*itr).isSpace()) {
      ++itr;
   }
   if ((*itr) == QLatin1Char('#')) {
      ++itr;
   }
   QString id;
   while ((*itr) != QLatin1Char(')')) {
      id += *itr;
      ++itr;
   }
   return id;
}

static inline QStringRef trimRef(const QStringRef &str)
{
   if (str.isEmpty()) {
      return QStringRef();
   }
   const QChar *s = str.string()->constData() + str.position();
   int end = str.length() - 1;
   if (!s[0].isSpace() && !s[end].isSpace()) {
      return str;
   }

   int start = 0;
   while (start <= end && s[start].isSpace()) { // skip white space from start
      start++;
   }
   if (start <= end) {                          // only white space
      while (s[end].isSpace()) {         // skip white space from end
         end--;
      }
   }
   int l = end - start + 1;
   if (l <= 0) {
      return QStringRef();
   }
   return QStringRef(str.string(), str.position() + start, l);
}

/**
 * returns true when successfuly set the color. false signifies
 * that the color should be inherited
 */
static bool resolveColor(const QStringRef &colorStr, QColor &color, QSvgHandler *handler)
{
   QStringRef colorStrTr = trimRef(colorStr);
   if (colorStrTr.isEmpty()) {
      return false;
   }

   switch (colorStrTr.at(0).unicode()) {

      case '#': {
         // #rrggbb is very very common, so let's tackle it here
         // rather than falling back to QColor
         QRgb rgb;
         bool ok = qsvg_get_hex_rgb(colorStrTr.unicode(), colorStrTr.length(), &rgb);
         if (ok) {
            color.setRgb(rgb);
         }
         return ok;
      }
      break;

      case 'r': {
         // starts with "rgb(", ends with ")" and consists of at least 7 characters "rgb(,,)"
         if (colorStrTr.length() >= 7 && colorStrTr.at(colorStrTr.length() - 1) == QLatin1Char(')')
               && QStringRef(colorStrTr.string(), colorStrTr.position(), 4) == QLatin1String("rgb(")) {
            const QChar *s = colorStrTr.constData() + 4;
            QVector<qreal> compo = parseNumbersList(s);
            //1 means that it failed after reaching non-parsable
            //character which is going to be "%"
            if (compo.size() == 1) {
               s = colorStrTr.constData() + 4;
               compo = parsePercentageList(s);
               for (int i = 0; i < compo.size(); ++i) {
                  compo[i] *= (qreal)2.55;
               }
            }

            if (compo.size() == 3) {
               color = QColor(int(compo[0]),
                              int(compo[1]),
                              int(compo[2]));
               return true;
            }
            return false;
         }
      }
      break;

      case 'c':
         if (colorStrTr == QLatin1String("currentColor")) {
            color = handler->currentColor();
            return true;
         }
         break;
      case 'i':
         if (colorStrTr == QT_INHERIT) {
            return false;
         }
         break;
      default:
         break;
   }

   color = QColor(colorStrTr.toString());
   return color.isValid();
}

static bool constructColor(const QStringRef &colorStr, const QStringRef &opacity,
                           QColor &color, QSvgHandler *handler)
{
   if (!resolveColor(colorStr, color, handler)) {
      return false;
   }
   if (!opacity.isEmpty()) {
      bool ok = true;
      qreal op = qMin(qreal(1.0), qMax(qreal(0.0), toDouble(opacity, &ok)));
      if (!ok) {
         op = 1.0;
      }
      color.setAlphaF(op);
   }
   return true;
}

static qreal parseLength(const QString &str, QSvgHandler::LengthType &type,
                         QSvgHandler *handler, bool *ok = NULL)
{
   QString numStr = str.trimmed();

   if (numStr.endsWith(QLatin1Char('%'))) {
      numStr.chop(1);
      type = QSvgHandler::LT_PERCENT;
   } else if (numStr.endsWith(QLatin1String("px"))) {
      numStr.chop(2);
      type = QSvgHandler::LT_PX;
   } else if (numStr.endsWith(QLatin1String("pc"))) {
      numStr.chop(2);
      type = QSvgHandler::LT_PC;
   } else if (numStr.endsWith(QLatin1String("pt"))) {
      numStr.chop(2);
      type = QSvgHandler::LT_PT;
   } else if (numStr.endsWith(QLatin1String("mm"))) {
      numStr.chop(2);
      type = QSvgHandler::LT_MM;
   } else if (numStr.endsWith(QLatin1String("cm"))) {
      numStr.chop(2);
      type = QSvgHandler::LT_CM;
   } else if (numStr.endsWith(QLatin1String("in"))) {
      numStr.chop(2);
      type = QSvgHandler::LT_IN;
   } else {
      type = handler->defaultCoordinateSystem();
      //type = QSvgHandler::LT_OTHER;
   }
   qreal len = toDouble(numStr, ok);
   //qDebug()<<"len is "<<len<<", from '"<<numStr << "'";
   return len;
}

static inline qreal convertToNumber(const QString &str, QSvgHandler *handler, bool *ok = NULL)
{
   QSvgHandler::LengthType type;
   qreal num = parseLength(str, type, handler, ok);
   if (type == QSvgHandler::LT_PERCENT) {
      num = num / 100.0;
   }
   return num;
}

static bool createSvgGlyph(QSvgFont *font, const QXmlStreamAttributes &attributes)
{
   QStringRef uncStr = attributes.value(QLatin1String("unicode"));
   QStringRef havStr = attributes.value(QLatin1String("horiz-adv-x"));
   QStringRef pathStr = attributes.value(QLatin1String("d"));

   QChar unicode = (uncStr.isEmpty()) ? 0 : uncStr.at(0);
   qreal havx = (havStr.isEmpty()) ? -1 : toDouble(havStr);
   QPainterPath path;
   path.setFillRule(Qt::WindingFill);
   parsePathDataFast(pathStr, path);

   font->addGlyph(unicode, path, havx);

   return true;
}

// this should really be called convertToDefaultCoordinateSystem
// and convert when type != QSvgHandler::defaultCoordinateSystem
static qreal convertToPixels(qreal len, bool , QSvgHandler::LengthType type)
{

   switch (type) {
      case QSvgHandler::LT_PERCENT:
         break;
      case QSvgHandler::LT_PX:
         break;
      case QSvgHandler::LT_PC:
         break;
      case QSvgHandler::LT_PT:
         return len * 1.25;
         break;
      case QSvgHandler::LT_MM:
         return len * 3.543307;
         break;
      case QSvgHandler::LT_CM:
         return len * 35.43307;
         break;
      case QSvgHandler::LT_IN:
         return len * 90;
         break;
      case QSvgHandler::LT_OTHER:
         break;
      default:
         break;
   }
   return len;
}

static void parseColor(QSvgNode *,
                       const QSvgAttributes &attributes,
                       QSvgHandler *handler)
{
   QColor color;
   if (constructColor(attributes.color, attributes.colorOpacity, color, handler)) {
      handler->popColor();
      handler->pushColor(color);
   }
}

static QSvgStyleProperty *styleFromUrl(QSvgNode *node, const QString &url)
{
   return node ? node->styleProperty(idFromUrl(url)) : 0;
}

static void parseBrush(QSvgNode *node,
                       const QSvgAttributes &attributes,
                       QSvgHandler *handler)
{
   if (!attributes.fill.isEmpty() || !attributes.fillRule.isEmpty() || !attributes.fillOpacity.isEmpty()) {
      QSvgFillStyle *prop = new QSvgFillStyle;

      //fill-rule attribute handling
      if (!attributes.fillRule.isEmpty() && attributes.fillRule != QT_INHERIT) {
         if (attributes.fillRule == QLatin1String("evenodd")) {
            prop->setFillRule(Qt::OddEvenFill);
         } else if (attributes.fillRule == QLatin1String("nonzero")) {
            prop->setFillRule(Qt::WindingFill);
         }
      }

      //fill-opacity atttribute handling
      if (!attributes.fillOpacity.isEmpty() && attributes.fillOpacity != QT_INHERIT) {
         prop->setFillOpacity(qMin(qreal(1.0), qMax(qreal(0.0), toDouble(attributes.fillOpacity))));
      }

      //fill attribute handling
      if ((!attributes.fill.isEmpty()) && (attributes.fill != QT_INHERIT) ) {
         if (attributes.fill.length() > 3 &&
               QStringRef(attributes.fill.string(), attributes.fill.position(), 3) == QLatin1String("url")) {
            QStringRef urlRef(attributes.fill.string(), attributes.fill.position() + 3, attributes.fill.length() - 3);
            QString value = urlRef.toString();
            QSvgStyleProperty *style = styleFromUrl(node, value);
            if (style) {
               if (style->type() == QSvgStyleProperty::SOLID_COLOR || style->type() == QSvgStyleProperty::GRADIENT) {
                  prop->setFillStyle(reinterpret_cast<QSvgFillStyleProperty *>(style));
               }
            } else {
               QString id = idFromUrl(value);
               prop->setGradientId(id);
               prop->setGradientResolved(false);
            }
         } else if (attributes.fill != QLatin1String("none")) {
            QColor color;
            if (resolveColor(attributes.fill, color, handler)) {
               prop->setBrush(QBrush(color));
            }
         } else {
            prop->setBrush(QBrush(Qt::NoBrush));
         }
      }
      node->appendStyleProperty(prop, attributes.id);
   }
}



static QMatrix parseTransformationMatrix(const QStringRef &value)
{
   if (value.isEmpty()) {
      return QMatrix();
   }

   QMatrix matrix;
   const QChar *str = value.constData();
   const QChar *end = str + value.length();

   while (str < end) {
      if (str->isSpace() || *str == QLatin1Char(',')) {
         ++str;
         continue;
      }
      enum State {
         Matrix,
         Translate,
         Rotate,
         Scale,
         SkewX,
         SkewY
      };
      State state = Matrix;
      if (*str == QLatin1Char('m')) {  //matrix
         const char *ident = "atrix";
         for (int i = 0; i < 5; ++i)
            if (*(++str) != QLatin1Char(ident[i])) {
               goto error;
            }
         ++str;
         state = Matrix;
      } else if (*str == QLatin1Char('t')) { //translate
         const char *ident = "ranslate";
         for (int i = 0; i < 8; ++i)
            if (*(++str) != QLatin1Char(ident[i])) {
               goto error;
            }
         ++str;
         state = Translate;
      } else if (*str == QLatin1Char('r')) { //rotate
         const char *ident = "otate";
         for (int i = 0; i < 5; ++i)
            if (*(++str) != QLatin1Char(ident[i])) {
               goto error;
            }
         ++str;
         state = Rotate;
      } else if (*str == QLatin1Char('s')) { //scale, skewX, skewY
         ++str;
         if (*str == QLatin1Char('c')) {
            const char *ident = "ale";
            for (int i = 0; i < 3; ++i)
               if (*(++str) != QLatin1Char(ident[i])) {
                  goto error;
               }
            ++str;
            state = Scale;
         } else if (*str == QLatin1Char('k')) {
            if (*(++str) != QLatin1Char('e')) {
               goto error;
            }
            if (*(++str) != QLatin1Char('w')) {
               goto error;
            }
            ++str;
            if (*str == QLatin1Char('X')) {
               state = SkewX;
            } else if (*str == QLatin1Char('Y')) {
               state = SkewY;
            } else {
               goto error;
            }
            ++str;
         } else {
            goto error;
         }
      } else {
         goto error;
      }


      while (str < end && str->isSpace()) {
         ++str;
      }
      if (*str != QLatin1Char('(')) {
         goto error;
      }
      ++str;
      QVarLengthArray<qreal, 8> points;
      parseNumbersArray(str, points);
      if (*str != QLatin1Char(')')) {
         goto error;
      }
      ++str;

      if (state == Matrix) {
         if (points.count() != 6) {
            goto error;
         }
         matrix = matrix * QMatrix(points[0], points[1],
                                   points[2], points[3],
                                   points[4], points[5]);
      } else if (state == Translate) {
         if (points.count() == 1) {
            matrix.translate(points[0], 0);
         } else if (points.count() == 2) {
            matrix.translate(points[0], points[1]);
         } else {
            goto error;
         }
      } else if (state == Rotate) {
         if (points.count() == 1) {
            matrix.rotate(points[0]);
         } else if (points.count() == 3) {
            matrix.translate(points[1], points[2]);
            matrix.rotate(points[0]);
            matrix.translate(-points[1], -points[2]);
         } else {
            goto error;
         }
      } else if (state == Scale) {
         if (points.count() < 1 || points.count() > 2) {
            goto error;
         }
         qreal sx = points[0];
         qreal sy = sx;
         if (points.count() == 2) {
            sy = points[1];
         }
         matrix.scale(sx, sy);
      } else if (state == SkewX) {
         if (points.count() != 1) {
            goto error;
         }
         const qreal deg2rad = qreal(0.017453292519943295769);
         matrix.shear(qTan(points[0]*deg2rad), 0);
      } else if (state == SkewY) {
         if (points.count() != 1) {
            goto error;
         }
         const qreal deg2rad = qreal(0.017453292519943295769);
         matrix.shear(0, qTan(points[0]*deg2rad));
      }
   }
error:
   return matrix;
}

static void parsePen(QSvgNode *node,
                     const QSvgAttributes &attributes,
                     QSvgHandler *handler)
{
   //qDebug()<<"Node "<<node->type()<<", attrs are "<<value<<width;

   if (!attributes.stroke.isEmpty() || !attributes.strokeDashArray.isEmpty() || !attributes.strokeDashOffset.isEmpty() ||
         !attributes.strokeLineCap.isEmpty()
         || !attributes.strokeLineJoin.isEmpty() || !attributes.strokeMiterLimit.isEmpty() ||
         !attributes.strokeOpacity.isEmpty() || !attributes.strokeWidth.isEmpty()
         || !attributes.vectorEffect.isEmpty()) {

      QSvgStrokeStyle *prop = new QSvgStrokeStyle;

      //stroke attribute handling
      if ((!attributes.stroke.isEmpty()) && (attributes.stroke != QT_INHERIT) ) {
         if (attributes.stroke.length() > 3 &&
               QStringRef(attributes.stroke.string(), attributes.stroke.position(), 3) == QLatin1String("url")) {
            QStringRef urlRef(attributes.stroke.string(), attributes.stroke.position() + 3, attributes.stroke.length() - 3);
            QString value = urlRef.toString();
            QSvgStyleProperty *style = styleFromUrl(node, value);
            if (style) {
               if (style->type() == QSvgStyleProperty::SOLID_COLOR || style->type() == QSvgStyleProperty::GRADIENT) {
                  prop->setStyle(reinterpret_cast<QSvgFillStyleProperty *>(style));
               }
            } else {
               QString id = idFromUrl(value);
               prop->setGradientId(id);
               prop->setGradientResolved(false);
            }
         } else if (attributes.stroke != QLatin1String("none")) {
            QColor color;
            if (resolveColor(attributes.stroke, color, handler)) {
               prop->setStroke(QBrush(color));
            }
         } else {
            prop->setStroke(QBrush(Qt::NoBrush));
         }
      }

      //stroke-width handling
      if (!attributes.strokeWidth.isEmpty() && attributes.strokeWidth != QT_INHERIT) {
         QSvgHandler::LengthType lt;
         prop->setWidth(parseLength(attributes.strokeWidth.toString(), lt, handler));
      }

      //stroke-dasharray
      if (!attributes.strokeDashArray.isEmpty() && attributes.strokeDashArray != QT_INHERIT) {
         if (attributes.strokeDashArray == QLatin1String("none")) {
            prop->setDashArrayNone();
         } else {
            QString dashArray  = attributes.strokeDashArray.toString();
            const QChar *s = dashArray.constData();
            QVector<qreal> dashes = parseNumbersList(s);
            // if the dash count is odd the dashes should be duplicated
            if ((dashes.size() & 1) != 0) {
               dashes << QVector<qreal>(dashes);
            }
            prop->setDashArray(dashes);
         }
      }

      //stroke-linejoin attribute handling
      if (!attributes.strokeLineJoin.isEmpty()) {
         if (attributes.strokeLineJoin == QLatin1String("miter")) {
            prop->setLineJoin(Qt::SvgMiterJoin);
         } else if (attributes.strokeLineJoin == QLatin1String("round")) {
            prop->setLineJoin(Qt::RoundJoin);
         } else if (attributes.strokeLineJoin == QLatin1String("bevel")) {
            prop->setLineJoin(Qt::BevelJoin);
         }
      }

      //stroke-linecap attribute handling
      if (!attributes.strokeLineCap.isEmpty()) {
         if (attributes.strokeLineCap == QLatin1String("butt")) {
            prop->setLineCap(Qt::FlatCap);
         } else if (attributes.strokeLineCap == QLatin1String("round")) {
            prop->setLineCap(Qt::RoundCap);
         } else if (attributes.strokeLineCap == QLatin1String("square")) {
            prop->setLineCap(Qt::SquareCap);
         }
      }

      //stroke-dashoffset attribute handling
      if (!attributes.strokeDashOffset.isEmpty() && attributes.strokeDashOffset != QT_INHERIT) {
         prop->setDashOffset(toDouble(attributes.strokeDashOffset));
      }

      //vector-effect attribute handling
      if (!attributes.vectorEffect.isEmpty()) {
         if (attributes.vectorEffect == QLatin1String("non-scaling-stroke")) {
            prop->setVectorEffect(true);
         } else if (attributes.vectorEffect == QLatin1String("none")) {
            prop->setVectorEffect(false);
         }
      }

      //stroke-miterlimit
      if (!attributes.strokeMiterLimit.isEmpty() && attributes.strokeMiterLimit != QT_INHERIT) {
         prop->setMiterLimit(toDouble(attributes.strokeMiterLimit));
      }

      //stroke-opacity atttribute handling
      if (!attributes.strokeOpacity.isEmpty() && attributes.strokeOpacity != QT_INHERIT) {
         prop->setOpacity(qMin(qreal(1.0), qMax(qreal(0.0), toDouble(attributes.strokeOpacity))));
      }

      node->appendStyleProperty(prop, attributes.id);
   }
}

static void parseFont(QSvgNode *node,
                      const QSvgAttributes &attributes,
                      QSvgHandler *handler)
{
   if (attributes.fontFamily.isEmpty() && attributes.fontSize.isEmpty() && attributes.fontStyle.isEmpty() &&
         attributes.fontWeight.isEmpty() && attributes.fontVariant.isEmpty() && attributes.textAnchor.isEmpty()) {
      return;
   }

   QSvgTinyDocument *doc = node->document();
   QSvgFontStyle *fontStyle = 0;
   if (!attributes.fontFamily.isEmpty()) {
      QSvgFont *svgFont = doc->svgFont(attributes.fontFamily.toString());
      if (svgFont) {
         fontStyle = new QSvgFontStyle(svgFont, doc);
      }
   }
   if (!fontStyle) {
      fontStyle = new QSvgFontStyle;
   }

   if (!attributes.fontFamily.isEmpty() && attributes.fontFamily != QT_INHERIT) {
      fontStyle->setFamily(attributes.fontFamily.toString().trimmed());
   }

   if (!attributes.fontSize.isEmpty() && attributes.fontSize != QT_INHERIT) {
      // TODO: Support relative sizes 'larger' and 'smaller'.
      QSvgHandler::LengthType dummy; // should always be pixel size
      qreal size = 0;
      static const qreal sizeTable[] = { qreal(6.9), qreal(8.3), qreal(10.0), qreal(12.0), qreal(14.4), qreal(17.3), qreal(20.7) };
      enum AbsFontSize { XXSmall, XSmall, Small, Medium, Large, XLarge, XXLarge };
      switch (attributes.fontSize.at(0).unicode()) {
         case 'x':
            if (attributes.fontSize == QLatin1String("xx-small")) {
               size = sizeTable[XXSmall];
            } else if (attributes.fontSize == QLatin1String("x-small")) {
               size = sizeTable[XSmall];
            } else if (attributes.fontSize == QLatin1String("x-large")) {
               size = sizeTable[XLarge];
            } else if (attributes.fontSize == QLatin1String("xx-large")) {
               size = sizeTable[XXLarge];
            }
            break;
         case 's':
            if (attributes.fontSize == QLatin1String("small")) {
               size = sizeTable[Small];
            }
            break;
         case 'm':
            if (attributes.fontSize == QLatin1String("medium")) {
               size = sizeTable[Medium];
            }
            break;
         case 'l':
            if (attributes.fontSize == QLatin1String("large")) {
               size = sizeTable[Large];
            }
            break;
         default:
            size = parseLength(attributes.fontSize.toString(), dummy, handler);
            break;
      }
      fontStyle->setSize(size);
   }

   if (!attributes.fontStyle.isEmpty() && attributes.fontStyle != QT_INHERIT) {
      if (attributes.fontStyle == QLatin1String("normal")) {
         fontStyle->setStyle(QFont::StyleNormal);
      } else if (attributes.fontStyle == QLatin1String("italic")) {
         fontStyle->setStyle(QFont::StyleItalic);
      } else if (attributes.fontStyle == QLatin1String("oblique")) {
         fontStyle->setStyle(QFont::StyleOblique);
      }
   }

   if (!attributes.fontWeight.isEmpty() && attributes.fontWeight != QT_INHERIT) {
      bool ok = false;
      int weightNum = attributes.fontWeight.toString().toInt(&ok);
      if (ok) {
         fontStyle->setWeight(weightNum);
      } else {
         if (attributes.fontWeight == QLatin1String("normal")) {
            fontStyle->setWeight(400);
         } else if (attributes.fontWeight == QLatin1String("bold")) {
            fontStyle->setWeight(700);
         } else if (attributes.fontWeight == QLatin1String("bolder")) {
            fontStyle->setWeight(QSvgFontStyle::BOLDER);
         } else if (attributes.fontWeight == QLatin1String("lighter")) {
            fontStyle->setWeight(QSvgFontStyle::LIGHTER);
         }
      }
   }

   if (!attributes.fontVariant.isEmpty() && attributes.fontVariant != QT_INHERIT) {
      if (attributes.fontVariant == QLatin1String("normal")) {
         fontStyle->setVariant(QFont::MixedCase);
      } else if (attributes.fontVariant == QLatin1String("small-caps")) {
         fontStyle->setVariant(QFont::SmallCaps);
      }
   }

   if (!attributes.textAnchor.isEmpty() && attributes.textAnchor != QT_INHERIT) {
      if (attributes.textAnchor == QLatin1String("start")) {
         fontStyle->setTextAnchor(Qt::AlignLeft);
      }
      if (attributes.textAnchor == QLatin1String("middle")) {
         fontStyle->setTextAnchor(Qt::AlignHCenter);
      } else if (attributes.textAnchor == QLatin1String("end")) {
         fontStyle->setTextAnchor(Qt::AlignRight);
      }
   }

   node->appendStyleProperty(fontStyle, attributes.id);
}

static void parseTransform(QSvgNode *node,
                           const QSvgAttributes &attributes,
                           QSvgHandler *)
{
   if (attributes.transform.isEmpty()) {
      return;
   }
   QMatrix matrix = parseTransformationMatrix(trimRef(attributes.transform));

   if (!matrix.isIdentity()) {
      node->appendStyleProperty(new QSvgTransformStyle(QTransform(matrix)), attributes.id);
   }

}

static void parseVisibility(QSvgNode *node,
                            const QSvgAttributes &attributes,
                            QSvgHandler *)
{
   QSvgNode *parent = node->parent();

   if (parent && (attributes.visibility.isEmpty() || attributes.visibility == QT_INHERIT)) {
      node->setVisible(parent->isVisible());
   } else if (attributes.visibility == QLatin1String("hidden") || attributes.visibility == QLatin1String("collapse")) {
      node->setVisible(false);
   } else {
      node->setVisible(true);
   }
}

static void pathArcSegment(QPainterPath &path,
                           qreal xc, qreal yc,
                           qreal th0, qreal th1,
                           qreal rx, qreal ry, qreal xAxisRotation)
{
   qreal sinTh, cosTh;
   qreal a00, a01, a10, a11;
   qreal x1, y1, x2, y2, x3, y3;
   qreal t;
   qreal thHalf;

   sinTh = qSin(xAxisRotation * (Q_PI / 180.0));
   cosTh = qCos(xAxisRotation * (Q_PI / 180.0));

   a00 =  cosTh * rx;
   a01 = -sinTh * ry;
   a10 =  sinTh * rx;
   a11 =  cosTh * ry;

   thHalf = 0.5 * (th1 - th0);
   t = (8.0 / 3.0) * qSin(thHalf * 0.5) * qSin(thHalf * 0.5) / qSin(thHalf);
   x1 = xc + qCos(th0) - t * qSin(th0);
   y1 = yc + qSin(th0) + t * qCos(th0);
   x3 = xc + qCos(th1);
   y3 = yc + qSin(th1);
   x2 = x3 + t * qSin(th1);
   y2 = y3 - t * qCos(th1);

   path.cubicTo(a00 * x1 + a01 * y1, a10 * x1 + a11 * y1,
                a00 * x2 + a01 * y2, a10 * x2 + a11 * y2,
                a00 * x3 + a01 * y3, a10 * x3 + a11 * y3);
}

// the arc handling code underneath is from XSVG (BSD license)
/*
 * Copyright  2002 USC/Information Sciences Institute
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Information Sciences Institute not be used in advertising or
 * publicity pertaining to distribution of the software without
 * specific, written prior permission.  Information Sciences Institute
 * makes no representations about the suitability of this software for
 * any purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * INFORMATION SCIENCES INSTITUTE DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL INFORMATION SCIENCES
 * INSTITUTE BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */
static void pathArc(QPainterPath &path,
                    qreal               rx,
                    qreal               ry,
                    qreal               x_axis_rotation,
                    int         large_arc_flag,
                    int         sweep_flag,
                    qreal               x,
                    qreal               y,
                    qreal curx, qreal cury)
{
   qreal sin_th, cos_th;
   qreal a00, a01, a10, a11;
   qreal x0, y0, x1, y1, xc, yc;
   qreal d, sfactor, sfactor_sq;
   qreal th0, th1, th_arc;
   int i, n_segs;
   qreal dx, dy, dx1, dy1, Pr1, Pr2, Px, Py, check;

   rx = qAbs(rx);
   ry = qAbs(ry);

   sin_th = qSin(x_axis_rotation * (Q_PI / 180.0));
   cos_th = qCos(x_axis_rotation * (Q_PI / 180.0));

   dx = (curx - x) / 2.0;
   dy = (cury - y) / 2.0;
   dx1 =  cos_th * dx + sin_th * dy;
   dy1 = -sin_th * dx + cos_th * dy;
   Pr1 = rx * rx;
   Pr2 = ry * ry;
   Px = dx1 * dx1;
   Py = dy1 * dy1;
   /* Spec : check if radii are large enough */
   check = Px / Pr1 + Py / Pr2;
   if (check > 1) {
      rx = rx * qSqrt(check);
      ry = ry * qSqrt(check);
   }

   a00 =  cos_th / rx;
   a01 =  sin_th / rx;
   a10 = -sin_th / ry;
   a11 =  cos_th / ry;
   x0 = a00 * curx + a01 * cury;
   y0 = a10 * curx + a11 * cury;
   x1 = a00 * x + a01 * y;
   y1 = a10 * x + a11 * y;
   /* (x0, y0) is current point in transformed coordinate space.
      (x1, y1) is new point in transformed coordinate space.

      The arc fits a unit-radius circle in this space.
   */
   d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
   sfactor_sq = 1.0 / d - 0.25;
   if (sfactor_sq < 0) {
      sfactor_sq = 0;
   }
   sfactor = qSqrt(sfactor_sq);
   if (sweep_flag == large_arc_flag) {
      sfactor = -sfactor;
   }
   xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
   yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);
   /* (xc, yc) is center of the circle. */

   th0 = qAtan2(y0 - yc, x0 - xc);
   th1 = qAtan2(y1 - yc, x1 - xc);

   th_arc = th1 - th0;
   if (th_arc < 0 && sweep_flag) {
      th_arc += 2 * Q_PI;
   } else if (th_arc > 0 && !sweep_flag) {
      th_arc -= 2 * Q_PI;
   }

   n_segs = qCeil(qAbs(th_arc / (Q_PI * 0.5 + 0.001)));

   for (i = 0; i < n_segs; i++) {
      pathArcSegment(path, xc, yc,
                     th0 + i * th_arc / n_segs,
                     th0 + (i + 1) * th_arc / n_segs,
                     rx, ry, x_axis_rotation);
   }
}

static bool parsePathDataFast(const QStringRef &dataStr, QPainterPath &path)
{
   qreal x0 = 0, y0 = 0;              // starting point
   qreal x = 0, y = 0;                // current point
   char lastMode = 0;
   QPointF ctrlPt;
   const QChar *str = dataStr.constData();
   const QChar *end = str + dataStr.size();

   while (str != end) {
      while (str->isSpace()) {
         ++str;
      }
      QChar pathElem = *str;
      ++str;
      QChar endc = *end;
      *const_cast<QChar *>(end) = 0; // parseNumbersArray requires 0-termination that QStringRef cannot guarantee
      QVarLengthArray<qreal, 8> arg;
      parseNumbersArray(str, arg);
      *const_cast<QChar *>(end) = endc;
      if (pathElem == QLatin1Char('z') || pathElem == QLatin1Char('Z')) {
         arg.append(0);   //dummy
      }
      const qreal *num = arg.constData();
      int count = arg.count();
      while (count > 0) {
         qreal offsetX = x;        // correction offsets
         qreal offsetY = y;        // for relative commands
         switch (pathElem.unicode()) {
            case 'm': {
               if (count < 2) {
                  num++;
                  count--;
                  break;
               }
               x = x0 = num[0] + offsetX;
               y = y0 = num[1] + offsetY;
               num += 2;
               count -= 2;
               path.moveTo(x0, y0);

               // As per 1.2  spec 8.3.2 The "moveto" commands
               // If a 'moveto' is followed by multiple pairs of coordinates without explicit commands,
               // the subsequent pairs shall be treated as implicit 'lineto' commands.
               pathElem = QLatin1Char('l');
            }
            break;
            case 'M': {
               if (count < 2) {
                  num++;
                  count--;
                  break;
               }
               x = x0 = num[0];
               y = y0 = num[1];
               num += 2;
               count -= 2;
               path.moveTo(x0, y0);

               // As per 1.2  spec 8.3.2 The "moveto" commands
               // If a 'moveto' is followed by multiple pairs of coordinates without explicit commands,
               // the subsequent pairs shall be treated as implicit 'lineto' commands.
               pathElem = QLatin1Char('L');
            }
            break;
            case 'z':
            case 'Z': {
               x = x0;
               y = y0;
               count--; // skip dummy
               num++;
               path.closeSubpath();
            }
            break;
            case 'l': {
               if (count < 2) {
                  num++;
                  count--;
                  break;
               }
               x = num[0] + offsetX;
               y = num[1] + offsetY;
               num += 2;
               count -= 2;
               path.lineTo(x, y);

            }
            break;
            case 'L': {
               if (count < 2) {
                  num++;
                  count--;
                  break;
               }
               x = num[0];
               y = num[1];
               num += 2;
               count -= 2;
               path.lineTo(x, y);
            }
            break;
            case 'h': {
               x = num[0] + offsetX;
               num++;
               count--;
               path.lineTo(x, y);
            }
            break;
            case 'H': {
               x = num[0];
               num++;
               count--;
               path.lineTo(x, y);
            }
            break;
            case 'v': {
               y = num[0] + offsetY;
               num++;
               count--;
               path.lineTo(x, y);
            }
            break;
            case 'V': {
               y = num[0];
               num++;
               count--;
               path.lineTo(x, y);
            }
            break;
            case 'c': {
               if (count < 6) {
                  num += count;
                  count = 0;
                  break;
               }
               QPointF c1(num[0] + offsetX, num[1] + offsetY);
               QPointF c2(num[2] + offsetX, num[3] + offsetY);
               QPointF e(num[4] + offsetX, num[5] + offsetY);
               num += 6;
               count -= 6;
               path.cubicTo(c1, c2, e);
               ctrlPt = c2;
               x = e.x();
               y = e.y();
               break;
            }
            case 'C': {
               if (count < 6) {
                  num += count;
                  count = 0;
                  break;
               }
               QPointF c1(num[0], num[1]);
               QPointF c2(num[2], num[3]);
               QPointF e(num[4], num[5]);
               num += 6;
               count -= 6;
               path.cubicTo(c1, c2, e);
               ctrlPt = c2;
               x = e.x();
               y = e.y();
               break;
            }
            case 's': {
               if (count < 4) {
                  num += count;
                  count = 0;
                  break;
               }
               QPointF c1;
               if (lastMode == 'c' || lastMode == 'C' ||
                     lastMode == 's' || lastMode == 'S') {
                  c1 = QPointF(2 * x - ctrlPt.x(), 2 * y - ctrlPt.y());
               } else {
                  c1 = QPointF(x, y);
               }
               QPointF c2(num[0] + offsetX, num[1] + offsetY);
               QPointF e(num[2] + offsetX, num[3] + offsetY);
               num += 4;
               count -= 4;
               path.cubicTo(c1, c2, e);
               ctrlPt = c2;
               x = e.x();
               y = e.y();
               break;
            }
            case 'S': {
               if (count < 4) {
                  num += count;
                  count = 0;
                  break;
               }
               QPointF c1;
               if (lastMode == 'c' || lastMode == 'C' ||
                     lastMode == 's' || lastMode == 'S') {
                  c1 = QPointF(2 * x - ctrlPt.x(), 2 * y - ctrlPt.y());
               } else {
                  c1 = QPointF(x, y);
               }
               QPointF c2(num[0], num[1]);
               QPointF e(num[2], num[3]);
               num += 4;
               count -= 4;
               path.cubicTo(c1, c2, e);
               ctrlPt = c2;
               x = e.x();
               y = e.y();
               break;
            }
            case 'q': {
               if (count < 4) {
                  num += count;
                  count = 0;
                  break;
               }
               QPointF c(num[0] + offsetX, num[1] + offsetY);
               QPointF e(num[2] + offsetX, num[3] + offsetY);
               num += 4;
               count -= 4;
               path.quadTo(c, e);
               ctrlPt = c;
               x = e.x();
               y = e.y();
               break;
            }
            case 'Q': {
               if (count < 4) {
                  num += count;
                  count = 0;
                  break;
               }
               QPointF c(num[0], num[1]);
               QPointF e(num[2], num[3]);
               num += 4;
               count -= 4;
               path.quadTo(c, e);
               ctrlPt = c;
               x = e.x();
               y = e.y();
               break;
            }
            case 't': {
               if (count < 2) {
                  num += count;
                  count = 0;
                  break;
               }
               QPointF e(num[0] + offsetX, num[1] + offsetY);
               num += 2;
               count -= 2;
               QPointF c;
               if (lastMode == 'q' || lastMode == 'Q' ||
                     lastMode == 't' || lastMode == 'T') {
                  c = QPointF(2 * x - ctrlPt.x(), 2 * y - ctrlPt.y());
               } else {
                  c = QPointF(x, y);
               }
               path.quadTo(c, e);
               ctrlPt = c;
               x = e.x();
               y = e.y();
               break;
            }
            case 'T': {
               if (count < 2) {
                  num += count;
                  count = 0;
                  break;
               }
               QPointF e(num[0], num[1]);
               num += 2;
               count -= 2;
               QPointF c;
               if (lastMode == 'q' || lastMode == 'Q' ||
                     lastMode == 't' || lastMode == 'T') {
                  c = QPointF(2 * x - ctrlPt.x(), 2 * y - ctrlPt.y());
               } else {
                  c = QPointF(x, y);
               }
               path.quadTo(c, e);
               ctrlPt = c;
               x = e.x();
               y = e.y();
               break;
            }
            case 'a': {
               if (count < 7) {
                  num += count;
                  count = 0;
                  break;
               }
               qreal rx = (*num++);
               qreal ry = (*num++);
               qreal xAxisRotation = (*num++);
               qreal largeArcFlag  = (*num++);
               qreal sweepFlag = (*num++);
               qreal ex = (*num++) + offsetX;
               qreal ey = (*num++) + offsetY;
               count -= 7;
               qreal curx = x;
               qreal cury = y;
               pathArc(path, rx, ry, xAxisRotation, int(largeArcFlag),
                       int(sweepFlag), ex, ey, curx, cury);

               x = ex;
               y = ey;
            }
            break;
            case 'A': {
               if (count < 7) {
                  num += count;
                  count = 0;
                  break;
               }
               qreal rx = (*num++);
               qreal ry = (*num++);
               qreal xAxisRotation = (*num++);
               qreal largeArcFlag  = (*num++);
               qreal sweepFlag = (*num++);
               qreal ex = (*num++);
               qreal ey = (*num++);
               count -= 7;
               qreal curx = x;
               qreal cury = y;
               pathArc(path, rx, ry, xAxisRotation, int(largeArcFlag),
                       int(sweepFlag), ex, ey, curx, cury);

               x = ex;
               y = ey;
            }
            break;
            default:
               return false;
         }
         lastMode = pathElem.toLatin1();
      }
   }
   return true;
}

static bool parseStyle(QSvgNode *node,
                       const QXmlStreamAttributes &attributes,
                       QSvgHandler *);

static bool parseStyle(QSvgNode *node,
                       const QSvgAttributes &attributes,
                       QSvgHandler *);

static void parseCSStoXMLAttrs(const QVector<QCss::Declaration> &declarations,
                               QXmlStreamAttributes &attributes)
{
   for (int i = 0; i < declarations.count(); ++i) {
      const QCss::Declaration &decl = declarations.at(i);
      if (decl.d->property.isEmpty()) {
         continue;
      }
      QCss::Value val = decl.d->values.first();
      QString valueStr;
      if (decl.d->values.count() != 1) {
         for (int i = 0; i < decl.d->values.count(); ++i) {
            const QString &value = decl.d->values[i].toString();
            if (value.isEmpty()) {
               valueStr += QLatin1Char(',');
            } else {
               valueStr += value;
            }
         }
      } else {
         valueStr = val.toString();
      }
      if (val.type == QCss::Value::Uri) {
         valueStr.prepend(QLatin1String("url("));
         valueStr.append(QLatin1Char(')'));
      } else if (val.type == QCss::Value::Function) {
         QStringList lst = val.variant.toStringList();
         valueStr.append(lst.at(0));
         valueStr.append(QLatin1Char('('));
         for (int i = 1; i < lst.count(); ++i) {
            valueStr.append(lst.at(i));
            if ((i + 1) < lst.count()) {
               valueStr.append(QLatin1Char(','));
            }
         }
         valueStr.append(QLatin1Char(')'));
      } else if (val.type == QCss::Value::KnownIdentifier) {
         switch (val.variant.toInt()) {
            case QCss::Value_None:
               valueStr = QLatin1String("none");
               break;
            default:
               break;
         }
      }

      attributes.append(QString(), decl.d->property, valueStr);
   }
}

void QSvgHandler::parseCSStoXMLAttrs(QString css, QVector<QSvgCssAttribute> *attributes)
{
   // preprocess (for unicode escapes), tokenize and remove comments
   m_cssParser.init(css);
   QString key;

   attributes->reserve(10);

   while (m_cssParser.hasNext()) {
      m_cssParser.skipSpace();

      if (!m_cssParser.hasNext()) {
         break;
      }
      m_cssParser.next();

      QStringRef name;
      if (m_cssParser.hasEscapeSequences) {
         key = m_cssParser.lexem();
         name = QStringRef(&key, 0, key.length());
      } else {
         const QCss::Symbol &sym = m_cssParser.symbol();
         name = QStringRef(&sym.text, sym.start, sym.len);
      }

      m_cssParser.skipSpace();
      if (!m_cssParser.test(QCss::COLON)) {
         break;
      }

      m_cssParser.skipSpace();
      if (!m_cssParser.hasNext()) {
         break;
      }

      QSvgCssAttribute attribute;
      attribute.name = QXmlStreamStringRef(name);

      const int firstSymbol = m_cssParser.index;
      int symbolCount = 0;
      do {
         m_cssParser.next();
         ++symbolCount;
      } while (m_cssParser.hasNext() && !m_cssParser.test(QCss::SEMICOLON));

      bool canExtractValueByRef = !m_cssParser.hasEscapeSequences;
      if (canExtractValueByRef) {
         int len = m_cssParser.symbols.at(firstSymbol).len;
         for (int i = firstSymbol + 1; i < firstSymbol + symbolCount; ++i) {
            len += m_cssParser.symbols.at(i).len;

            if (m_cssParser.symbols.at(i - 1).start + m_cssParser.symbols.at(i - 1).len
                  != m_cssParser.symbols.at(i).start) {
               canExtractValueByRef = false;
               break;
            }
         }
         if (canExtractValueByRef) {
            const QCss::Symbol &sym = m_cssParser.symbols.at(firstSymbol);
            attribute.value = QXmlStreamStringRef(QStringRef(&sym.text, sym.start, len));
         }
      }
      if (!canExtractValueByRef) {
         QString value;
         for (int i = firstSymbol; i < m_cssParser.index - 1; ++i) {
            value += m_cssParser.symbols.at(i).lexem();
         }
         attribute.value = QXmlStreamStringRef(QStringRef(&value, 0, value.length()));
      }

      attributes->append(attribute);

      m_cssParser.skipSpace();
   }
}

static void cssStyleLookup(QSvgNode *node,
                           QSvgHandler *handler,
                           QSvgStyleSelector *selector)
{
   QCss::StyleSelector::NodePtr cssNode;
   cssNode.ptr = node;
   QVector<QCss::Declaration> decls = selector->declarationsForNode(cssNode);

   QXmlStreamAttributes attributes;
   parseCSStoXMLAttrs(decls, attributes);
   parseStyle(node, attributes, handler);
}

static inline QStringList stringToList(const QString &str)
{
   QStringList lst = str.split(QLatin1Char(','), QString::SkipEmptyParts);
   return lst;
}

static bool parseCoreNode(QSvgNode *node,
                          const QXmlStreamAttributes &attributes)
{
   QStringList features;
   QStringList extensions;
   QStringList languages;
   QStringList formats;
   QStringList fonts;
   QString xmlClassStr;

   for (int i = 0; i < attributes.count(); ++i) {
      const QXmlStreamAttribute &attribute = attributes.at(i);
      QStringRef name = attribute.qualifiedName();
      if (name.isEmpty()) {
         continue;
      }
      QStringRef value = attribute.value();
      switch (name.at(0).unicode()) {
         case 'c':
            if (name == QLatin1String("class")) {
               xmlClassStr = value.toString();
            }
            break;
         case 'r':
            if (name == QLatin1String("requiredFeatures")) {
               features = stringToList(value.toString());
            } else if (name == QLatin1String("requiredExtensions")) {
               extensions = stringToList(value.toString());
            } else if (name == QLatin1String("requiredFormats")) {
               formats = stringToList(value.toString());
            } else if (name == QLatin1String("requiredFonts")) {
               fonts = stringToList(value.toString());
            }
            break;
         case 's':
            if (name == QLatin1String("systemLanguage")) {
               languages = stringToList(value.toString());
            }
            break;
         default:
            break;
      }
   }

   node->setRequiredFeatures(features);
   node->setRequiredExtensions(extensions);
   node->setRequiredLanguages(languages);
   node->setRequiredFormats(formats);
   node->setRequiredFonts(fonts);
   node->setNodeId(someId(attributes));
   node->setXmlClass(xmlClassStr);

   return true;
}

static void parseOpacity(QSvgNode *node,
                         const QSvgAttributes &attributes,
                         QSvgHandler *)
{
   if (attributes.opacity.isEmpty()) {
      return;
   }

   const QString value = attributes.opacity.toString().trimmed();

   bool ok = false;
   qreal op = value.toDouble(&ok);

   if (ok) {
      QSvgOpacityStyle *opacity = new QSvgOpacityStyle(qBound(qreal(0.0), op, qreal(1.0)));
      node->appendStyleProperty(opacity, attributes.id);
   }
}

static QPainter::CompositionMode svgToQtCompositionMode(const QString &op)
{
#define NOOP qDebug()<<"Operation: "<<op<<" is not implemented"
   if (op == QLatin1String("clear")) {
      return QPainter::CompositionMode_Clear;
   } else if (op == QLatin1String("src")) {
      return QPainter::CompositionMode_Source;
   } else if (op == QLatin1String("dst")) {
      return QPainter::CompositionMode_Destination;
   } else if (op == QLatin1String("src-over")) {
      return QPainter::CompositionMode_SourceOver;
   } else if (op == QLatin1String("dst-over")) {
      return QPainter::CompositionMode_DestinationOver;
   } else if (op == QLatin1String("src-in")) {
      return QPainter::CompositionMode_SourceIn;
   } else if (op == QLatin1String("dst-in")) {
      return QPainter::CompositionMode_DestinationIn;
   } else if (op == QLatin1String("src-out")) {
      return QPainter::CompositionMode_SourceOut;
   } else if (op == QLatin1String("dst-out")) {
      return QPainter::CompositionMode_DestinationOut;
   } else if (op == QLatin1String("src-atop")) {
      return QPainter::CompositionMode_SourceAtop;
   } else if (op == QLatin1String("dst-atop")) {
      return QPainter::CompositionMode_DestinationAtop;
   } else if (op == QLatin1String("xor")) {
      return QPainter::CompositionMode_Xor;
   } else if (op == QLatin1String("plus")) {
      return QPainter::CompositionMode_Plus;
   } else if (op == QLatin1String("multiply")) {
      return QPainter::CompositionMode_Multiply;
   } else if (op == QLatin1String("screen")) {
      return QPainter::CompositionMode_Screen;
   } else if (op == QLatin1String("overlay")) {
      return QPainter::CompositionMode_Overlay;
   } else if (op == QLatin1String("darken")) {
      return QPainter::CompositionMode_Darken;
   } else if (op == QLatin1String("lighten")) {
      return QPainter::CompositionMode_Lighten;
   } else if (op == QLatin1String("color-dodge")) {
      return QPainter::CompositionMode_ColorDodge;
   } else if (op == QLatin1String("color-burn")) {
      return QPainter::CompositionMode_ColorBurn;
   } else if (op == QLatin1String("hard-light")) {
      return QPainter::CompositionMode_HardLight;
   } else if (op == QLatin1String("soft-light")) {
      return QPainter::CompositionMode_SoftLight;
   } else if (op == QLatin1String("difference")) {
      return QPainter::CompositionMode_Difference;
   } else if (op == QLatin1String("exclusion")) {
      return QPainter::CompositionMode_Exclusion;
   } else {
      NOOP;
   }

   return QPainter::CompositionMode_SourceOver;
}

static void parseCompOp(QSvgNode *node,
                        const QSvgAttributes &attributes,
                        QSvgHandler *)
{
   if (attributes.compOp.isEmpty()) {
      return;
   }
   QString value = attributes.compOp.toString().trimmed();

   if (!value.isEmpty()) {
      QSvgCompOpStyle *compop = new QSvgCompOpStyle(svgToQtCompositionMode(value));
      node->appendStyleProperty(compop, attributes.id);
   }
}

static inline QSvgNode::DisplayMode displayStringToEnum(const QString &str)
{
   if (str == QLatin1String("inline")) {
      return QSvgNode::InlineMode;
   } else if (str == QLatin1String("block")) {
      return QSvgNode::BlockMode;
   } else if (str == QLatin1String("list-item")) {
      return QSvgNode::ListItemMode;
   } else if (str == QLatin1String("run-in")) {
      return QSvgNode::RunInMode;
   } else if (str == QLatin1String("compact")) {
      return QSvgNode::CompactMode;
   } else if (str == QLatin1String("marker")) {
      return QSvgNode::MarkerMode;
   } else if (str == QLatin1String("table")) {
      return QSvgNode::TableMode;
   } else if (str == QLatin1String("inline-table")) {
      return QSvgNode::InlineTableMode;
   } else if (str == QLatin1String("table-row")) {
      return QSvgNode::TableRowGroupMode;
   } else if (str == QLatin1String("table-header-group")) {
      return QSvgNode::TableHeaderGroupMode;
   } else if (str == QLatin1String("table-footer-group")) {
      return QSvgNode::TableFooterGroupMode;
   } else if (str == QLatin1String("table-row")) {
      return QSvgNode::TableRowMode;
   } else if (str == QLatin1String("table-column-group")) {
      return QSvgNode::TableColumnGroupMode;
   } else if (str == QLatin1String("table-column")) {
      return QSvgNode::TableColumnMode;
   } else if (str == QLatin1String("table-cell")) {
      return QSvgNode::TableCellMode;
   } else if (str == QLatin1String("table-caption")) {
      return QSvgNode::TableCaptionMode;
   } else if (str == QLatin1String("none")) {
      return QSvgNode::NoneMode;
   } else if (str == QT_INHERIT) {
      return QSvgNode::InheritMode;
   }
   return QSvgNode::BlockMode;
}

static void parseOthers(QSvgNode *node,
                        const QSvgAttributes &attributes,
                        QSvgHandler *)
{
   if (attributes.display.isEmpty()) {
      return;
   }
   QString displayStr = attributes.display.toString().trimmed();

   if (!displayStr.isEmpty()) {
      node->setDisplayMode(displayStringToEnum(displayStr));
   }
}

static bool parseStyle(QSvgNode *node,
                       const QSvgAttributes &attributes,
                       QSvgHandler *handler)
{
   parseColor(node, attributes, handler);
   parseBrush(node, attributes, handler);
   parsePen(node, attributes, handler);
   parseFont(node, attributes, handler);
   parseTransform(node, attributes, handler);
   parseVisibility(node, attributes, handler);
   parseOpacity(node, attributes, handler);
   parseCompOp(node, attributes, handler);
   parseOthers(node, attributes, handler);

   return true;
}

static bool parseStyle(QSvgNode *node,
                       const QXmlStreamAttributes &attrs,
                       QSvgHandler *handler)
{
   return parseStyle(node, QSvgAttributes(attrs, handler), handler);
}

static bool parseAnchorNode(QSvgNode *parent,
                            const QXmlStreamAttributes &attributes,
                            QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static bool parseAnimateNode(QSvgNode *parent,
                             const QXmlStreamAttributes &attributes,
                             QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static bool parseAnimateColorNode(QSvgNode *parent,
                                  const QXmlStreamAttributes &attributes,
                                  QSvgHandler *handler)
{
   QString typeStr    = attributes.value(QLatin1String("type")).toString();
   QStringRef fromStr    = attributes.value(QLatin1String("from"));
   QStringRef toStr      = attributes.value(QLatin1String("to"));
   QString valuesStr  = attributes.value(QLatin1String("values")).toString();
   QString beginStr   = attributes.value(QLatin1String("begin")).toString();
   QString durStr     = attributes.value(QLatin1String("dur")).toString();
   QString targetStr  = attributes.value(QLatin1String("attributeName")).toString();
   QString repeatStr  = attributes.value(QLatin1String("repeatCount")).toString();
   QString fillStr    = attributes.value(QLatin1String("fill")).toString();

   QList<QColor> colors;
   if (valuesStr.isEmpty()) {
      QColor startColor, endColor;
      resolveColor(fromStr, startColor, handler);
      resolveColor(toStr, endColor, handler);
      colors.append(startColor);
      colors.append(endColor);
   } else {
      QStringList str = valuesStr.split(QLatin1Char(';'));
      QStringList::const_iterator itr;
      for (itr = str.constBegin(); itr != str.constEnd(); ++itr) {
         QColor color;
         QString str = *itr;
         resolveColor(QStringRef(&str), color, handler);
         colors.append(color);
      }
   }

   int ms = 1000;
   beginStr = beginStr.trimmed();
   if (beginStr.endsWith(QLatin1String("ms"))) {
      beginStr.chop(2);
      ms = 1;
   } else if (beginStr.endsWith(QLatin1String("s"))) {
      beginStr.chop(1);
   }
   durStr = durStr.trimmed();
   if (durStr.endsWith(QLatin1String("ms"))) {
      durStr.chop(2);
      ms = 1;
   } else if (durStr.endsWith(QLatin1String("s"))) {
      durStr.chop(1);
   }
   int begin = static_cast<int>(toDouble(beginStr) * ms);
   int end   = static_cast<int>((toDouble(durStr) + begin) * ms);

   QSvgAnimateColor *anim = new QSvgAnimateColor(begin, end, 0);
   anim->setArgs((targetStr == QLatin1String("fill")), colors);
   anim->setFreeze(fillStr == QLatin1String("freeze"));
   anim->setRepeatCount(
      (repeatStr == QLatin1String("indefinite")) ? -1 :
      (repeatStr == QLatin1String("")) ? 1 : toDouble(repeatStr));

   parent->appendStyleProperty(anim, someId(attributes));
   parent->document()->setAnimated(true);
   handler->setAnimPeriod(begin, end);
   return true;
}

static bool parseAimateMotionNode(QSvgNode *parent,
                                  const QXmlStreamAttributes &attributes,
                                  QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static void parseNumberTriplet(QVector<qreal> &values, const QChar *&s)
{
   QVector<qreal> list = parseNumbersList(s);
   values << list;
   for (int i = 3 - list.size(); i > 0; --i) {
      values.append(0.0);
   }
}

static bool parseAnimateTransformNode(QSvgNode *parent,
                                      const QXmlStreamAttributes &attributes,
                                      QSvgHandler *handler)
{
   QString typeStr    = attributes.value(QLatin1String("type")).toString();
   QString values     = attributes.value(QLatin1String("values")).toString();
   QString beginStr   = attributes.value(QLatin1String("begin")).toString();
   QString durStr     = attributes.value(QLatin1String("dur")).toString();
   QString targetStr  = attributes.value(QLatin1String("attributeName")).toString();
   QString repeatStr  = attributes.value(QLatin1String("repeatCount")).toString();
   QString fillStr    = attributes.value(QLatin1String("fill")).toString();
   QString fromStr    = attributes.value(QLatin1String("from")).toString();
   QString toStr      = attributes.value(QLatin1String("to")).toString();
   QString byStr      = attributes.value(QLatin1String("by")).toString();
   QString addtv      = attributes.value(QLatin1String("additive")).toString();

   QSvgAnimateTransform::Additive additive = QSvgAnimateTransform::Replace;
   if (addtv == QLatin1String("sum")) {
      additive = QSvgAnimateTransform::Sum;
   }

   QVector<qreal> vals;
   if (values.isEmpty()) {
      const QChar *s;
      if (fromStr.isEmpty()) {
         if (!byStr.isEmpty()) {
            // By-animation.
            additive = QSvgAnimateTransform::Sum;
            vals.append(0.0);
            vals.append(0.0);
            vals.append(0.0);
            parseNumberTriplet(vals, s = byStr.constData());
         } else {
            // To-animation not defined.
            return false;
         }
      } else {
         if (!toStr.isEmpty()) {
            // From-to-animation.
            parseNumberTriplet(vals, s = fromStr.constData());
            parseNumberTriplet(vals, s = toStr.constData());
         } else if (!byStr.isEmpty()) {
            // From-by-animation.
            parseNumberTriplet(vals, s = fromStr.constData());
            parseNumberTriplet(vals, s = byStr.constData());
            for (int i = vals.size() - 3; i < vals.size(); ++i) {
               vals[i] += vals[i - 3];
            }
         } else {
            return false;
         }
      }
   } else {
      const QChar *s = values.constData();
      while (s && *s != QLatin1Char(0)) {
         parseNumberTriplet(vals, s);
         if (*s == QLatin1Char(0)) {
            break;
         }
         ++s;
      }
   }

   int ms = 1000;
   beginStr = beginStr.trimmed();
   if (beginStr.endsWith(QLatin1String("ms"))) {
      beginStr.chop(2);
      ms = 1;
   } else if (beginStr.endsWith(QLatin1String("s"))) {
      beginStr.chop(1);
   }
   int begin = static_cast<int>(toDouble(beginStr) * ms);
   durStr = durStr.trimmed();
   if (durStr.endsWith(QLatin1String("ms"))) {
      durStr.chop(2);
      ms = 1;
   } else if (durStr.endsWith(QLatin1String("s"))) {
      durStr.chop(1);
      ms = 1000;
   }
   int end = static_cast<int>(toDouble(durStr) * ms) + begin;

   QSvgAnimateTransform::TransformType type = QSvgAnimateTransform::Empty;
   if (typeStr == QLatin1String("translate")) {
      type = QSvgAnimateTransform::Translate;
   } else if (typeStr == QLatin1String("scale")) {
      type = QSvgAnimateTransform::Scale;
   } else if (typeStr == QLatin1String("rotate")) {
      type = QSvgAnimateTransform::Rotate;
   } else if (typeStr == QLatin1String("skewX")) {
      type = QSvgAnimateTransform::SkewX;
   } else if (typeStr == QLatin1String("skewY")) {
      type = QSvgAnimateTransform::SkewY;
   } else {
      return false;
   }

   QSvgAnimateTransform *anim = new QSvgAnimateTransform(begin, end, 0);
   anim->setArgs(type, additive, vals);
   anim->setFreeze(fillStr == QLatin1String("freeze"));
   anim->setRepeatCount(
      (repeatStr == QLatin1String("indefinite")) ? -1 :
      (repeatStr == QLatin1String("")) ? 1 : toDouble(repeatStr));

   parent->appendStyleProperty(anim, someId(attributes));
   parent->document()->setAnimated(true);
   handler->setAnimPeriod(begin, end);
   return true;
}

static QSvgNode *createAnimationNode(QSvgNode *parent,
                                     const QXmlStreamAttributes &attributes,
                                     QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return 0;
}

static bool parseAudioNode(QSvgNode *parent,
                           const QXmlStreamAttributes &attributes,
                           QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static QSvgNode *createCircleNode(QSvgNode *parent,
                                  const QXmlStreamAttributes &attributes,
                                  QSvgHandler *)
{
   QString cx      = attributes.value(QLatin1String("cx")).toString();
   QString cy      = attributes.value(QLatin1String("cy")).toString();
   QString r       = attributes.value(QLatin1String("r")).toString();
   qreal ncx = toDouble(cx);
   qreal ncy = toDouble(cy);
   qreal nr  = toDouble(r);

   QRectF rect(ncx - nr, ncy - nr, nr * 2, nr * 2);
   QSvgNode *circle = new QSvgCircle(parent, rect);
   return circle;
}

static QSvgNode *createDefsNode(QSvgNode *parent,
                                const QXmlStreamAttributes &attributes,
                                QSvgHandler *)
{
   Q_UNUSED(attributes);
   QSvgDefs *defs = new QSvgDefs(parent);
   return defs;
}

static bool parseDescNode(QSvgNode *parent,
                          const QXmlStreamAttributes &attributes,
                          QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static bool parseDiscardNode(QSvgNode *parent,
                             const QXmlStreamAttributes &attributes,
                             QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static QSvgNode *createEllipseNode(QSvgNode *parent,
                                   const QXmlStreamAttributes &attributes,
                                   QSvgHandler *)
{
   QString cx      = attributes.value(QLatin1String("cx")).toString();
   QString cy      = attributes.value(QLatin1String("cy")).toString();
   QString rx      = attributes.value(QLatin1String("rx")).toString();
   QString ry      = attributes.value(QLatin1String("ry")).toString();
   qreal ncx = toDouble(cx);
   qreal ncy = toDouble(cy);
   qreal nrx = toDouble(rx);
   qreal nry = toDouble(ry);

   QRectF rect(ncx - nrx, ncy - nry, nrx * 2, nry * 2);
   QSvgNode *ellipse = new QSvgEllipse(parent, rect);
   return ellipse;
}

static QSvgStyleProperty *createFontNode(QSvgNode *parent,
      const QXmlStreamAttributes &attributes,
      QSvgHandler *)
{
   QString hax      = attributes.value(QLatin1String("horiz-adv-x")).toString();
   QString myId     = someId(attributes);

   qreal horizAdvX = toDouble(hax);

   while (parent && parent->type() != QSvgNode::DOC) {
      parent = parent->parent();
   }

   if (parent) {
      QSvgTinyDocument *doc = static_cast<QSvgTinyDocument *>(parent);
      QSvgFont *font = new QSvgFont(horizAdvX);
      font->setFamilyName(myId);
      if (!font->familyName().isEmpty()) {
         if (!doc->svgFont(font->familyName())) {
            doc->addSvgFont(font);
         }
      }
      return new QSvgFontStyle(font, doc);
   }
   return 0;
}

static bool parseFontFaceNode(QSvgStyleProperty *parent,
                              const QXmlStreamAttributes &attributes,
                              QSvgHandler *)
{
   if (parent->type() != QSvgStyleProperty::FONT) {
      return false;
   }

   QSvgFontStyle *style = static_cast<QSvgFontStyle *>(parent);
   QSvgFont *font = style->svgFont();
   QString name   = attributes.value(QLatin1String("font-family")).toString();
   QString unitsPerEmStr   = attributes.value(QLatin1String("units-per-em")).toString();

   qreal unitsPerEm = toDouble(unitsPerEmStr);
   if (!unitsPerEm) {
      unitsPerEm = 1000;
   }

   if (!name.isEmpty()) {
      font->setFamilyName(name);
   }
   font->setUnitsPerEm(unitsPerEm);

   if (!font->familyName().isEmpty())
      if (!style->doc()->svgFont(font->familyName())) {
         style->doc()->addSvgFont(font);
      }

   return true;
}

static bool parseFontFaceNameNode(QSvgStyleProperty *parent,
                                  const QXmlStreamAttributes &attributes,
                                  QSvgHandler *)
{
   if (parent->type() != QSvgStyleProperty::FONT) {
      return false;
   }

   QSvgFontStyle *style = static_cast<QSvgFontStyle *>(parent);
   QSvgFont *font = style->svgFont();
   QString name   = attributes.value(QLatin1String("name")).toString();

   if (!name.isEmpty()) {
      font->setFamilyName(name);
   }

   if (!font->familyName().isEmpty())
      if (!style->doc()->svgFont(font->familyName())) {
         style->doc()->addSvgFont(font);
      }

   return true;
}

static bool parseFontFaceSrcNode(QSvgStyleProperty *parent,
                                 const QXmlStreamAttributes &attributes,
                                 QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static bool parseFontFaceUriNode(QSvgStyleProperty *parent,
                                 const QXmlStreamAttributes &attributes,
                                 QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static bool parseForeignObjectNode(QSvgNode *parent,
                                   const QXmlStreamAttributes &attributes,
                                   QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static QSvgNode *createGNode(QSvgNode *parent,
                             const QXmlStreamAttributes &attributes,
                             QSvgHandler *)
{
   Q_UNUSED(attributes);
   QSvgG *node = new QSvgG(parent);
   return node;
}

static bool parseGlyphNode(QSvgStyleProperty *parent,
                           const QXmlStreamAttributes &attributes,
                           QSvgHandler *)
{
   if (parent->type() != QSvgStyleProperty::FONT) {
      return false;
   }

   QSvgFontStyle *style = static_cast<QSvgFontStyle *>(parent);
   QSvgFont *font = style->svgFont();
   createSvgGlyph(font, attributes);
   return true;
}

static bool parseHandlerNode(QSvgNode *parent,
                             const QXmlStreamAttributes &attributes,
                             QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static bool parseHkernNode(QSvgNode *parent,
                           const QXmlStreamAttributes &attributes,
                           QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static QSvgNode *createImageNode(QSvgNode *parent,
                                 const QXmlStreamAttributes &attributes,
                                 QSvgHandler *handler)
{
   QString x = attributes.value(QLatin1String("x")).toString();
   QString y = attributes.value(QLatin1String("y")).toString();
   QString width  = attributes.value(QLatin1String("width")).toString();
   QString height = attributes.value(QLatin1String("height")).toString();
   QString filename = attributes.value(QLatin1String("xlink:href")).toString();
   qreal nx = toDouble(x);
   qreal ny = toDouble(y);
   QSvgHandler::LengthType type;
   qreal nwidth = parseLength(width, type, handler);
   nwidth = convertToPixels(nwidth, true, type);

   qreal nheight = parseLength(height, type, handler);
   nheight = convertToPixels(nheight, false, type);


   filename = filename.trimmed();
   QImage image;
   if (filename.startsWith(QLatin1String("data"))) {
      int idx = filename.lastIndexOf(QLatin1String("base64,"));
      if (idx != -1) {
         idx += 7;
         QString dataStr = filename.mid(idx);
         QByteArray data = QByteArray::fromBase64(dataStr.toLatin1());
         image = QImage::fromData(data);
      } else {
         qDebug() << "QSvgHandler::createImageNode: Unrecognized inline image format!";
      }
   } else {
      image = QImage(filename);
   }

   if (image.isNull()) {
      qDebug() << "couldn't create image from " << filename;
      return 0;
   }

   if (image.format() == QImage::Format_ARGB32) {
      image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
   }

   QSvgNode *img = new QSvgImage(parent,
                                 image,
                                 QRect(int(nx),
                                       int(ny),
                                       int(nwidth),
                                       int(nheight)));
   return img;
}

static QSvgNode *createLineNode(QSvgNode *parent,
                                const QXmlStreamAttributes &attributes,
                                QSvgHandler *)
{
   QString x1 = attributes.value(QLatin1String("x1")).toString();
   QString y1 = attributes.value(QLatin1String("y1")).toString();
   QString x2 = attributes.value(QLatin1String("x2")).toString();
   QString y2 = attributes.value(QLatin1String("y2")).toString();
   qreal nx1 = toDouble(x1);
   qreal ny1 = toDouble(y1);
   qreal nx2 = toDouble(x2);
   qreal ny2 = toDouble(y2);

   QLineF lineBounds(nx1, ny1, nx2, ny2);
   QSvgNode *line = new QSvgLine(parent, lineBounds);
   return line;
}


static void parseBaseGradient(QSvgNode *node,
                              const QXmlStreamAttributes &attributes,
                              QSvgGradientStyle *gradProp,
                              QSvgHandler *handler)
{
   QString link   = attributes.value(QLatin1String("xlink:href")).toString();
   QStringRef trans  = attributes.value(QLatin1String("gradientTransform"));
   QString spread = attributes.value(QLatin1String("spreadMethod")).toString();
   QString units = attributes.value(QLatin1String("gradientUnits")).toString();
   QStringRef colorStr = attributes.value(QLatin1String("color"));
   QStringRef colorOpacityStr = attributes.value(QLatin1String("color-opacity"));

   QColor color;
   if (constructColor(colorStr, colorOpacityStr, color, handler)) {
      handler->popColor();
      handler->pushColor(color);
   }

   QMatrix matrix;
   QGradient *grad = gradProp->qgradient();
   if (!link.isEmpty()) {
      QSvgStyleProperty *prop = node->styleProperty(link);
      //qDebug()<<"inherited "<<prop<<" ("<<link<<")";
      if (prop && prop->type() == QSvgStyleProperty::GRADIENT) {
         QSvgGradientStyle *inherited =
            static_cast<QSvgGradientStyle *>(prop);
         if (!inherited->stopLink().isEmpty()) {
            gradProp->setStopLink(inherited->stopLink(), handler->document());
         } else {
            grad->setStops(inherited->qgradient()->stops());
            gradProp->setGradientStopsSet(inherited->gradientStopsSet());
         }

         matrix = inherited->qmatrix();
      } else {
         gradProp->setStopLink(link, handler->document());
      }
   }

   if (!trans.isEmpty()) {
      matrix = parseTransformationMatrix(trans);
      gradProp->setMatrix(matrix);
   } else if (!matrix.isIdentity()) {
      gradProp->setMatrix(matrix);
   }

   if (!spread.isEmpty()) {
      if (spread == QLatin1String("pad")) {
         grad->setSpread(QGradient::PadSpread);
      } else if (spread == QLatin1String("reflect")) {
         grad->setSpread(QGradient::ReflectSpread);
      } else if (spread == QLatin1String("repeat")) {
         grad->setSpread(QGradient::RepeatSpread);
      }
   }

   if (units.isEmpty() || units == QLatin1String("objectBoundingBox")) {
      grad->setCoordinateMode(QGradient::ObjectBoundingMode);
   }
}

static QSvgStyleProperty *createLinearGradientNode(QSvgNode *node,
      const QXmlStreamAttributes &attributes,
      QSvgHandler *handler)
{
   QString x1 = attributes.value(QLatin1String("x1")).toString();
   QString y1 = attributes.value(QLatin1String("y1")).toString();
   QString x2 = attributes.value(QLatin1String("x2")).toString();
   QString y2 = attributes.value(QLatin1String("y2")).toString();

   qreal nx1 = 0.0;
   qreal ny1 = 0.0;
   qreal nx2 = 1.0;
   qreal ny2 = 0.0;

   if (!x1.isEmpty()) {
      nx1 =  convertToNumber(x1, handler);
   }
   if (!y1.isEmpty()) {
      ny1 =  convertToNumber(y1, handler);
   }
   if (!x2.isEmpty()) {
      nx2 =  convertToNumber(x2, handler);
   }
   if (!y2.isEmpty()) {
      ny2 =  convertToNumber(y2, handler);
   }

   QSvgNode *itr = node;
   while (itr && itr->type() != QSvgNode::DOC) {
      itr = itr->parent();
   }

   QLinearGradient *grad = new QLinearGradient(nx1, ny1, nx2, ny2);
   grad->setInterpolationMode(QGradient::ComponentInterpolation);
   QSvgGradientStyle *prop = new QSvgGradientStyle(grad);
   parseBaseGradient(node, attributes, prop, handler);

   return prop;
}

static bool parseMetadataNode(QSvgNode *parent,
                              const QXmlStreamAttributes &attributes,
                              QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static bool parseMissingGlyphNode(QSvgStyleProperty *parent,
                                  const QXmlStreamAttributes &attributes,
                                  QSvgHandler *)
{
   if (parent->type() != QSvgStyleProperty::FONT) {
      return false;
   }

   QSvgFontStyle *style = static_cast<QSvgFontStyle *>(parent);
   QSvgFont *font = style->svgFont();
   createSvgGlyph(font, attributes);
   return true;
}

static bool parseMpathNode(QSvgNode *parent,
                           const QXmlStreamAttributes &attributes,
                           QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static QSvgNode *createPathNode(QSvgNode *parent,
                                const QXmlStreamAttributes &attributes,
                                QSvgHandler *)
{
   QStringRef data      = attributes.value(QLatin1String("d"));

   QPainterPath qpath;
   qpath.setFillRule(Qt::WindingFill);
   //XXX do error handling
   parsePathDataFast(data, qpath);

   QSvgNode *path = new QSvgPath(parent, qpath);
   return path;
}

static QSvgNode *createPolygonNode(QSvgNode *parent,
                                   const QXmlStreamAttributes &attributes,
                                   QSvgHandler *)
{
   QString pointsStr  = attributes.value(QLatin1String("points")).toString();

   //same QPolygon parsing is in createPolylineNode
   const QChar *s = pointsStr.constData();
   QVector<qreal> points = parseNumbersList(s);
   QPolygonF poly(points.count() / 2);
   for (int i = 0; i < poly.size(); ++i) {
      poly[i] = QPointF(points.at(2 * i), points.at(2 * i + 1));
   }
   QSvgNode *polygon = new QSvgPolygon(parent, poly);
   return polygon;
}

static QSvgNode *createPolylineNode(QSvgNode *parent,
                                    const QXmlStreamAttributes &attributes,
                                    QSvgHandler *)
{
   QString pointsStr  = attributes.value(QLatin1String("points")).toString();

   //same QPolygon parsing is in createPolygonNode
   const QChar *s = pointsStr.constData();
   QVector<qreal> points = parseNumbersList(s);
   QPolygonF poly(points.count() / 2);
   for (int i = 0; i < poly.size(); ++i) {
      poly[i] = QPointF(points.at(2 * i), points.at(2 * i + 1));
   }

   QSvgNode *line = new QSvgPolyline(parent, poly);
   return line;
}

static bool parsePrefetchNode(QSvgNode *parent,
                              const QXmlStreamAttributes &attributes,
                              QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static QSvgStyleProperty *createRadialGradientNode(QSvgNode *node,
      const QXmlStreamAttributes &attributes,
      QSvgHandler *handler)
{
   QString cx = attributes.value(QLatin1String("cx")).toString();
   QString cy = attributes.value(QLatin1String("cy")).toString();
   QString r  = attributes.value(QLatin1String("r")).toString();
   QString fx = attributes.value(QLatin1String("fx")).toString();
   QString fy = attributes.value(QLatin1String("fy")).toString();

   qreal ncx = 0.5;
   qreal ncy = 0.5;
   qreal nr  = 0.5;
   if (!cx.isEmpty()) {
      ncx = toDouble(cx);
   }
   if (!cy.isEmpty()) {
      ncy = toDouble(cy);
   }
   if (!r.isEmpty()) {
      nr = toDouble(r);
   }

   qreal nfx = ncx;
   if (!fx.isEmpty()) {
      nfx = toDouble(fx);
   }
   qreal nfy = ncy;
   if (!fy.isEmpty()) {
      nfy = toDouble(fy);
   }

   QRadialGradient *grad = new QRadialGradient(ncx, ncy, nr, nfx, nfy);
   grad->setInterpolationMode(QGradient::ComponentInterpolation);

   QSvgGradientStyle *prop = new QSvgGradientStyle(grad);
   parseBaseGradient(node, attributes, prop, handler);

   return prop;
}

static QSvgNode *createRectNode(QSvgNode *parent,
                                const QXmlStreamAttributes &attributes,
                                QSvgHandler *handler)
{
   QString x      = attributes.value(QLatin1String("x")).toString();
   QString y      = attributes.value(QLatin1String("y")).toString();
   QString width  = attributes.value(QLatin1String("width")).toString();
   QString height = attributes.value(QLatin1String("height")).toString();
   QString rx      = attributes.value(QLatin1String("rx")).toString();
   QString ry      = attributes.value(QLatin1String("ry")).toString();

   QSvgHandler::LengthType type;
   qreal nwidth = parseLength(width, type, handler);
   nwidth = convertToPixels(nwidth, true, type);

   qreal nheight = parseLength(height, type, handler);
   nheight = convertToPixels(nheight, true, type);
   qreal nrx = toDouble(rx);
   qreal nry = toDouble(ry);

   QRectF bounds(toDouble(x), toDouble(y),
                 nwidth, nheight);

   //9.2 The 'rect'  element clearly specifies it
   // but the case might in fact be handled because
   // we draw rounded rectangles differently
   if (nrx > bounds.width() / 2) {
      nrx = bounds.width() / 2;
   }
   if (nry > bounds.height() / 2) {
      nry = bounds.height() / 2;
   }

   if (!rx.isEmpty() && ry.isEmpty()) {
      nry = nrx;
   } else if (!ry.isEmpty() && rx.isEmpty()) {
      nrx = nry;
   }

   //we draw rounded rect from 0...99
   //svg from 0...bounds.width()/2 so we're adjusting the
   //coordinates
   nrx *= (100 / (bounds.width() / 2));
   nry *= (100 / (bounds.height() / 2));

   QSvgNode *rect = new QSvgRect(parent, bounds,
                                 int(nrx),
                                 int(nry));
   return rect;
}

static bool parseScriptNode(QSvgNode *parent,
                            const QXmlStreamAttributes &attributes,
                            QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static bool parseSetNode(QSvgNode *parent,
                         const QXmlStreamAttributes &attributes,
                         QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static QSvgStyleProperty *createSolidColorNode(QSvgNode *parent,
      const QXmlStreamAttributes &attributes,
      QSvgHandler *handler)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   QStringRef solidColorStr = attributes.value(QLatin1String("solid-color"));
   QStringRef solidOpacityStr = attributes.value(QLatin1String("solid-opacity"));

   if (solidOpacityStr.isEmpty()) {
      solidOpacityStr = attributes.value(QLatin1String("opacity"));
   }

   QColor color;
   if (!constructColor(solidColorStr, solidOpacityStr, color, handler)) {
      return 0;
   }
   QSvgSolidColorStyle *style = new QSvgSolidColorStyle(color);
   return style;
}

static bool parseStopNode(QSvgStyleProperty *parent,
                          const QXmlStreamAttributes &attributes,
                          QSvgHandler *handler)
{
   if (parent->type() != QSvgStyleProperty::GRADIENT) {
      return false;
   }
   QString nodeIdStr     = someId(attributes);
   QString xmlClassStr   = attributes.value(QLatin1String("class")).toString();

   //### nasty hack because stop gradients are not in the rendering tree
   //    we force a dummy node with the same id and class into a rendering
   //    tree to figure out whether the selector has a style for it
   //    QSvgStyleSelector should be coded in a way that could avoid it
   QSvgAnimation anim;
   anim.setNodeId(nodeIdStr);
   anim.setXmlClass(xmlClassStr);

   QCss::StyleSelector::NodePtr cssNode;
   cssNode.ptr = &anim;
   QVector<QCss::Declaration> decls = handler->selector()->declarationsForNode(cssNode);

   QXmlStreamAttributes xmlAttr = attributes;
   for (int i = 0; i < decls.count(); ++i) {
      const QCss::Declaration &decl = decls.at(i);

      if (decl.d->property.isEmpty()) {
         continue;
      }
      if (decl.d->values.count() != 1) {
         continue;
      }
      QCss::Value val = decl.d->values.first();
      QString valueStr = val.toString();
      if (val.type == QCss::Value::Uri) {
         valueStr.prepend(QLatin1String("url("));
         valueStr.append(QLatin1Char(')'));
      }
      xmlAttr.append(QString(), decl.d->property, valueStr);
   }
   QSvgAttributes attrs(xmlAttr, handler);

   QSvgGradientStyle *style =
      static_cast<QSvgGradientStyle *>(parent);
   QString offsetStr   = attrs.offset.toString();
   QStringRef colorStr    = attrs.stopColor;
   QColor color;

   bool ok = true;
   qreal offset = convertToNumber(offsetStr, handler, &ok);
   if (!ok) {
      offset = 0.0;
   }
   QString black = QString::fromLatin1("#000000");
   if (colorStr.isEmpty()) {
      colorStr = QStringRef(&black);
   }

   constructColor(colorStr, attrs.stopOpacity, color, handler);

   QGradient *grad = style->qgradient();

   offset = qMin(qreal(1), qMax(qreal(0), offset)); // Clamp to range [0, 1]
   QGradientStops stops;
   if (style->gradientStopsSet()) {
      stops = grad->stops();
      // If the stop offset equals the one previously added, add an epsilon to make it greater.
      if (offset <= stops.back().first) {
         offset = stops.back().first + FLT_EPSILON;
      }
   }

   // If offset is greater than one, it must be clamped to one.
   if (offset > 1.0) {
      if ((stops.size() == 1) || (stops.at(stops.size() - 2).first < 1.0 - FLT_EPSILON)) {
         stops.back().first = 1.0 - FLT_EPSILON;
         grad->setStops(stops);
      }
      offset = 1.0;
   }

   grad->setColorAt(offset, color);
   style->setGradientStopsSet(true);
   return true;
}

static bool parseStyleNode(QSvgNode *parent,
                           const QXmlStreamAttributes &attributes,
                           QSvgHandler *handler)
{
   Q_UNUSED(parent);
   QString type = attributes.value(QLatin1String("type")).toString();
   type = type.toLower();

   if (type == QLatin1String("text/css")) {
      handler->setInStyle(true);
   }

   return true;
}

static QSvgNode *createSvgNode(QSvgNode *parent,
                               const QXmlStreamAttributes &attributes,
                               QSvgHandler *handler)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);

   QString baseProfile = attributes.value(QLatin1String("baseProfile")).toString();

   QSvgTinyDocument *node = new QSvgTinyDocument();
   QString widthStr  = attributes.value(QLatin1String("width")).toString();
   QString heightStr = attributes.value(QLatin1String("height")).toString();
   QString viewBoxStr = attributes.value(QLatin1String("viewBox")).toString();

   QSvgHandler::LengthType type = QSvgHandler::LT_PX; // FIXME: is the default correct?
   qreal width = 0;
   if (!widthStr.isEmpty()) {
      width = parseLength(widthStr, type, handler);
      if (type != QSvgHandler::LT_PT) {
         width = convertToPixels(width, true, type);
      }
      node->setWidth(int(width), type == QSvgHandler::LT_PERCENT);
   }
   qreal height = 0;
   if (!heightStr.isEmpty()) {
      height = parseLength(heightStr, type, handler);
      if (type != QSvgHandler::LT_PT) {
         height = convertToPixels(height, false, type);
      }
      node->setHeight(int(height), type == QSvgHandler::LT_PERCENT);
   }

   QStringList viewBoxValues;
   if (!viewBoxStr.isEmpty()) {
      viewBoxStr = viewBoxStr.replace(QLatin1Char(' '), QLatin1Char(','));
      viewBoxStr = viewBoxStr.replace(QLatin1Char('\r'), QLatin1Char(','));
      viewBoxStr = viewBoxStr.replace(QLatin1Char('\n'), QLatin1Char(','));
      viewBoxStr = viewBoxStr.replace(QLatin1Char('\t'), QLatin1Char(','));
      viewBoxValues = viewBoxStr.split(QLatin1Char(','), QString::SkipEmptyParts);
   }
   if (viewBoxValues.count() == 4) {
      QString xStr      = viewBoxValues.at(0).trimmed();
      QString yStr      = viewBoxValues.at(1).trimmed();
      QString widthStr  = viewBoxValues.at(2).trimmed();
      QString heightStr = viewBoxValues.at(3).trimmed();

      QSvgHandler::LengthType lt;
      qreal x = parseLength(xStr, lt, handler);
      qreal y = parseLength(yStr, lt, handler);
      qreal w = parseLength(widthStr, lt, handler);
      qreal h = parseLength(heightStr, lt, handler);

      node->setViewBox(QRectF(x, y, w, h));

   } else if (width && height) {
      if (type == QSvgHandler::LT_PT) {
         width = convertToPixels(width, false, type);
         height = convertToPixels(height, false, type);
      }
      node->setViewBox(QRectF(0, 0, width, height));
   }
   handler->setDefaultCoordinateSystem(QSvgHandler::LT_PX);

   return node;
}

static QSvgNode *createSwitchNode(QSvgNode *parent,
                                  const QXmlStreamAttributes &attributes,
                                  QSvgHandler *)
{
   Q_UNUSED(attributes);
   QSvgSwitch *node = new QSvgSwitch(parent);
   return node;
}

static bool parseTbreakNode(QSvgNode *parent,
                            const QXmlStreamAttributes &,
                            QSvgHandler *)
{
   if (parent->type() != QSvgNode::TEXTAREA) {
      return false;
   }
   static_cast<QSvgText *>(parent)->addLineBreak();
   return true;
}

static QSvgNode *createTextNode(QSvgNode *parent,
                                const QXmlStreamAttributes &attributes,
                                QSvgHandler *handler)
{
   QString x = attributes.value(QLatin1String("x")).toString();
   QString y = attributes.value(QLatin1String("y")).toString();
   //### editable and rotate not handled
   QSvgHandler::LengthType type;
   qreal nx = parseLength(x, type, handler);
   qreal ny = parseLength(y, type, handler);

   QSvgNode *text = new QSvgText(parent, QPointF(nx, ny));
   return text;
}

static QSvgNode *createTextAreaNode(QSvgNode *parent,
                                    const QXmlStreamAttributes &attributes,
                                    QSvgHandler *handler)
{
   QSvgText *node = static_cast<QSvgText *>(createTextNode(parent, attributes, handler));
   if (node) {
      QSvgHandler::LengthType type;
      qreal width = parseLength(attributes.value(QLatin1String("width")).toString(), type, handler);
      qreal height = parseLength(attributes.value(QLatin1String("height")).toString(), type, handler);
      node->setTextArea(QSizeF(width, height));
   }
   return node;
}

static QSvgNode *createTspanNode(QSvgNode *parent,
                                 const QXmlStreamAttributes &,
                                 QSvgHandler *)
{
   return new QSvgTspan(parent);
}

static bool parseTitleNode(QSvgNode *parent,
                           const QXmlStreamAttributes &attributes,
                           QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return true;
}

static QSvgNode *createUseNode(QSvgNode *parent,
                               const QXmlStreamAttributes &attributes,
                               QSvgHandler *handler)
{
   QString linkId = attributes.value(QLatin1String("xlink:href")).toString().remove(0, 1);
   QString xStr = attributes.value(QLatin1String("x")).toString();
   QString yStr = attributes.value(QLatin1String("y")).toString();
   QSvgStructureNode *group = 0;

   if (linkId.isEmpty()) {
      linkId = attributes.value(QLatin1String("href")).toString().remove(0, 1);
   }
   switch (parent->type()) {
      case QSvgNode::DOC:
      case QSvgNode::DEFS:
      case QSvgNode::G:
      case QSvgNode::SWITCH:
         group = static_cast<QSvgStructureNode *>(parent);
         break;
      default:
         break;
   }

   if (group) {
      QSvgNode *link = group->scopeNode(linkId);
      if (link) {
         QPointF pt;
         if (!xStr.isNull() || !yStr.isNull()) {
            QSvgHandler::LengthType type;
            qreal nx = parseLength(xStr, type, handler);
            nx = convertToPixels(nx, true, type);

            qreal ny = parseLength(yStr, type, handler);
            ny = convertToPixels(ny, true, type);
            pt = QPointF(nx, ny);
         }

         //delay link resolving till the first draw call on
         //use nodes, link 2might have not been created yet
         QSvgUse *node = new QSvgUse(pt, parent, link);
         return node;
      }
   }

   qWarning("link %s hasn't been detected!", qPrintable(linkId));
   return 0;
}

static QSvgNode *createVideoNode(QSvgNode *parent,
                                 const QXmlStreamAttributes &attributes,
                                 QSvgHandler *)
{
   Q_UNUSED(parent);
   Q_UNUSED(attributes);
   return 0;
}

typedef QSvgNode *(*FactoryMethod)(QSvgNode *, const QXmlStreamAttributes &, QSvgHandler *);

static FactoryMethod findGroupFactory(const QString &name)
{
   if (name.isEmpty()) {
      return 0;
   }

   QStringRef ref(&name, 1, name.length() - 1);
   switch (name.at(0).unicode()) {
      case 'd':
         if (ref == QLatin1String("efs")) {
            return createDefsNode;
         }
         break;
      case 'g':
         if (ref.isEmpty()) {
            return createGNode;
         }
         break;
      case 's':
         if (ref == QLatin1String("vg")) {
            return createSvgNode;
         }
         if (ref == QLatin1String("witch")) {
            return createSwitchNode;
         }
         break;
      default:
         break;
   }
   return 0;
}

static FactoryMethod findGraphicsFactory(const QString &name)
{
   if (name.isEmpty()) {
      return 0;
   }

   QStringRef ref(&name, 1, name.length() - 1);
   switch (name.at(0).unicode()) {
      case 'a':
         if (ref == QLatin1String("nimation")) {
            return createAnimationNode;
         }
         break;
      case 'c':
         if (ref == QLatin1String("ircle")) {
            return createCircleNode;
         }
         break;
      case 'e':
         if (ref == QLatin1String("llipse")) {
            return createEllipseNode;
         }
         break;
      case 'i':
         if (ref == QLatin1String("mage")) {
            return createImageNode;
         }
         break;
      case 'l':
         if (ref == QLatin1String("ine")) {
            return createLineNode;
         }
         break;
      case 'p':
         if (ref == QLatin1String("ath")) {
            return createPathNode;
         }
         if (ref == QLatin1String("olygon")) {
            return createPolygonNode;
         }
         if (ref == QLatin1String("olyline")) {
            return createPolylineNode;
         }
         break;
      case 'r':
         if (ref == QLatin1String("ect")) {
            return createRectNode;
         }
         break;
      case 't':
         if (ref == QLatin1String("ext")) {
            return createTextNode;
         }
         if (ref == QLatin1String("extArea")) {
            return createTextAreaNode;
         }
         if (ref == QLatin1String("span")) {
            return createTspanNode;
         }
         break;
      case 'u':
         if (ref == QLatin1String("se")) {
            return createUseNode;
         }
         break;
      case 'v':
         if (ref == QLatin1String("ideo")) {
            return createVideoNode;
         }
         break;
      default:
         break;
   }
   return 0;
}

typedef bool (*ParseMethod)(QSvgNode *, const QXmlStreamAttributes &, QSvgHandler *);

static ParseMethod findUtilFactory(const QString &name)
{
   if (name.isEmpty()) {
      return 0;
   }

   QStringRef ref(&name, 1, name.length() - 1);
   switch (name.at(0).unicode()) {
      case 'a':
         if (ref.isEmpty()) {
            return parseAnchorNode;
         }
         if (ref == QLatin1String("nimate")) {
            return parseAnimateNode;
         }
         if (ref == QLatin1String("nimateColor")) {
            return parseAnimateColorNode;
         }
         if (ref == QLatin1String("nimateMotion")) {
            return parseAimateMotionNode;
         }
         if (ref == QLatin1String("nimateTransform")) {
            return parseAnimateTransformNode;
         }
         if (ref == QLatin1String("udio")) {
            return parseAudioNode;
         }
         break;
      case 'd':
         if (ref == QLatin1String("esc")) {
            return parseDescNode;
         }
         if (ref == QLatin1String("iscard")) {
            return parseDiscardNode;
         }
         break;
      case 'f':
         if (ref == QLatin1String("oreignObject")) {
            return parseForeignObjectNode;
         }
         break;
      case 'h':
         if (ref == QLatin1String("andler")) {
            return parseHandlerNode;
         }
         if (ref == QLatin1String("kern")) {
            return parseHkernNode;
         }
         break;
      case 'm':
         if (ref == QLatin1String("etadata")) {
            return parseMetadataNode;
         }
         if (ref == QLatin1String("path")) {
            return parseMpathNode;
         }
         break;
      case 'p':
         if (ref == QLatin1String("refetch")) {
            return parsePrefetchNode;
         }
         break;
      case 's':
         if (ref == QLatin1String("cript")) {
            return parseScriptNode;
         }
         if (ref == QLatin1String("et")) {
            return parseSetNode;
         }
         if (ref == QLatin1String("tyle")) {
            return parseStyleNode;
         }
         break;
      case 't':
         if (ref == QLatin1String("break")) {
            return parseTbreakNode;
         }
         if (ref == QLatin1String("itle")) {
            return parseTitleNode;
         }
         break;
      default:
         break;
   }
   return 0;
}

typedef QSvgStyleProperty *(*StyleFactoryMethod)(QSvgNode *,
      const QXmlStreamAttributes &,
      QSvgHandler *);

static StyleFactoryMethod findStyleFactoryMethod(const QString &name)
{
   if (name.isEmpty()) {
      return 0;
   }

   QStringRef ref(&name, 1, name.length() - 1);
   switch (name.at(0).unicode()) {
      case 'f':
         if (ref == QLatin1String("ont")) {
            return createFontNode;
         }
         break;
      case 'l':
         if (ref == QLatin1String("inearGradient")) {
            return createLinearGradientNode;
         }
         break;
      case 'r':
         if (ref == QLatin1String("adialGradient")) {
            return createRadialGradientNode;
         }
         break;
      case 's':
         if (ref == QLatin1String("olidColor")) {
            return createSolidColorNode;
         }
         break;
      default:
         break;
   }
   return 0;
}

typedef bool (*StyleParseMethod)(QSvgStyleProperty *,
                                 const QXmlStreamAttributes &,
                                 QSvgHandler *);

static StyleParseMethod findStyleUtilFactoryMethod(const QString &name)
{
   if (name.isEmpty()) {
      return 0;
   }

   QStringRef ref(&name, 1, name.length() - 1);
   switch (name.at(0).unicode()) {
      case 'f':
         if (ref == QLatin1String("ont-face")) {
            return parseFontFaceNode;
         }
         if (ref == QLatin1String("ont-face-name")) {
            return parseFontFaceNameNode;
         }
         if (ref == QLatin1String("ont-face-src")) {
            return parseFontFaceSrcNode;
         }
         if (ref == QLatin1String("ont-face-uri")) {
            return parseFontFaceUriNode;
         }
         break;
      case 'g':
         if (ref == QLatin1String("lyph")) {
            return parseGlyphNode;
         }
         break;
      case 'm':
         if (ref == QLatin1String("issing-glyph")) {
            return parseMissingGlyphNode;
         }
         break;
      case 's':
         if (ref == QLatin1String("top")) {
            return parseStopNode;
         }
         break;
      default:
         break;
   }
   return 0;
}

QSvgHandler::QSvgHandler(QIODevice *device) : xml(new QXmlStreamReader(device))
   , m_ownsReader(true)
{
   init();
}

QSvgHandler::QSvgHandler(const QByteArray &data) : xml(new QXmlStreamReader(data))
   , m_ownsReader(true)
{
   init();
}

QSvgHandler::QSvgHandler(QXmlStreamReader *const reader) : xml(reader)
   , m_ownsReader(false)
{
   init();
}

void QSvgHandler::init()
{
   m_doc = 0;
   m_style = 0;
   m_animEnd = 0;
   m_defaultCoords = LT_PX;
   m_defaultPen = QPen(Qt::black, 1, Qt::SolidLine, Qt::FlatCap, Qt::SvgMiterJoin);
   m_defaultPen.setMiterLimit(4);
   parse();
}

void QSvgHandler::parse()
{
   xml->setNamespaceProcessing(false);
   m_selector = new QSvgStyleSelector;
   m_inStyle = false;
   bool done = false;
   while (!xml->atEnd() && !done) {
      switch (xml->readNext()) {
         case QXmlStreamReader::StartElement:
            // he we could/should verify the namespaces, and simply
            // call m_skipNodes(Unknown) if we don't know the
            // namespace.  We do support http://www.w3.org/2000/svg
            // but also http://www.w3.org/2000/svg-20000303-stylable
            // And if the document uses an external dtd, the reported
            // namespaceUri is empty. The only possible strategy at
            // this point is to do what everyone else seems to do and
            // ignore the reported namespaceUri completely.
            if (!startElement(xml->name().toString(), xml->attributes())) {
               delete m_doc;
               m_doc = 0;
               return;
            }
            break;
         case QXmlStreamReader::EndElement:
            endElement(xml->name());
            // if we are using somebody else's qxmlstreamreader
            // we should not read until the end of the stream
            done = !m_ownsReader && (xml->name() == QLatin1String("svg"));
            break;
         case QXmlStreamReader::Characters:
            characters(xml->text());
            break;
         case QXmlStreamReader::ProcessingInstruction:
            processingInstruction(xml->processingInstructionTarget().toString(), xml->processingInstructionData().toString());
            break;
         default:
            break;
      }
   }
   resolveGradients(m_doc);
}

bool QSvgHandler::startElement(const QString &localName,
                               const QXmlStreamAttributes &attributes)
{
   QSvgNode *node = 0;

   pushColorCopy();

   /* The xml:space attribute may appear on any element. We do
    * a lookup by the qualified name here, but this is namespace aware, since
    * the XML namespace can only be bound to prefix "xml." */
   const QStringRef xmlSpace(attributes.value(QLatin1String("xml:space")));
   if (xmlSpace.isNull()) {
      // This element has no xml:space attribute.
      m_whitespaceMode.push(m_whitespaceMode.isEmpty() ? QSvgText::Default : m_whitespaceMode.top());
   } else if (xmlSpace == QLatin1String("preserve")) {
      m_whitespaceMode.push(QSvgText::Preserve);
   } else if (xmlSpace == QLatin1String("default")) {
      m_whitespaceMode.push(QSvgText::Default);
   } else {
      qWarning() << QString::fromLatin1("\"%1\" is an invalid value for attribute xml:space. "
                                        "Valid values are \"preserve\" and \"default\".").arg(xmlSpace.toString());
      m_whitespaceMode.push(QSvgText::Default);
   }

   if (!m_doc && localName != QLatin1String("svg")) {
      return false;
   }

   if (FactoryMethod method = findGroupFactory(localName)) {
      //group
      node = method(m_doc ? m_nodes.top() : 0, attributes, this);
      Q_ASSERT(node);
      if (!m_doc) {
         Q_ASSERT(node->type() == QSvgNode::DOC);
         m_doc = static_cast<QSvgTinyDocument *>(node);
      } else {
         switch (m_nodes.top()->type()) {
            case QSvgNode::DOC:
            case QSvgNode::G:
            case QSvgNode::DEFS:
            case QSvgNode::SWITCH: {
               QSvgStructureNode *group =
                  static_cast<QSvgStructureNode *>(m_nodes.top());
               group->addChild(node, someId(attributes));
            }
            break;
            default:
               break;
         }
      }
      parseCoreNode(node, attributes);
      cssStyleLookup(node, this, m_selector);
      parseStyle(node, attributes, this);
   } else if (FactoryMethod method = findGraphicsFactory(localName)) {
      //rendering element
      Q_ASSERT(!m_nodes.isEmpty());
      node = method(m_nodes.top(), attributes, this);
      if (node) {
         switch (m_nodes.top()->type()) {
            case QSvgNode::DOC:
            case QSvgNode::G:
            case QSvgNode::DEFS:
            case QSvgNode::SWITCH: {
               QSvgStructureNode *group =
                  static_cast<QSvgStructureNode *>(m_nodes.top());
               group->addChild(node, someId(attributes));
            }
            break;
            case QSvgNode::TEXT:
            case QSvgNode::TEXTAREA:
               if (node->type() == QSvgNode::TSPAN) {
                  static_cast<QSvgText *>(m_nodes.top())->addTspan(static_cast<QSvgTspan *>(node));
               } else {
                  qWarning("\'text\' or \'textArea\' element contains invalid element type.");
                  delete node;
                  node = 0;
               }
               break;
            default:
               qWarning("Could not add child element to parent element because the types are incorrect.");
               delete node;
               node = 0;
               break;
         }

         if (node) {
            parseCoreNode(node, attributes);
            cssStyleLookup(node, this, m_selector);
            parseStyle(node, attributes, this);
            if (node->type() == QSvgNode::TEXT || node->type() == QSvgNode::TEXTAREA) {
               static_cast<QSvgText *>(node)->setWhitespaceMode(m_whitespaceMode.top());
            } else if (node->type() == QSvgNode::TSPAN) {
               static_cast<QSvgTspan *>(node)->setWhitespaceMode(m_whitespaceMode.top());
            }
         }
      }
   } else if (ParseMethod method = findUtilFactory(localName)) {
      Q_ASSERT(!m_nodes.isEmpty());
      if (!method(m_nodes.top(), attributes, this)) {
         qWarning("Problem parsing %s", qPrintable(localName));
      }
   } else if (StyleFactoryMethod method = findStyleFactoryMethod(localName)) {
      QSvgStyleProperty *prop = method(m_nodes.top(), attributes, this);
      if (prop) {
         m_style = prop;
         m_nodes.top()->appendStyleProperty(prop, someId(attributes));
      } else {
         qWarning("Could not parse node: %s", qPrintable(localName));
      }
   } else if (StyleParseMethod method = findStyleUtilFactoryMethod(localName)) {
      if (m_style) {
         if (!method(m_style, attributes, this)) {
            qWarning("Problem parsing %s", qPrintable(localName));
         }
      }
   } else {
      //qWarning()<<"Skipping unknown element!"<<namespaceURI<<"::"<<localName;
      m_skipNodes.push(Unknown);
      return true;
   }

   if (node) {
      m_nodes.push(node);
      m_skipNodes.push(Graphics);
   } else {
      //qDebug()<<"Skipping "<<localName;
      m_skipNodes.push(Style);
   }
   return true;
}

bool QSvgHandler::endElement(const QStringRef &localName)
{
   CurrentNode node = m_skipNodes.top();
   m_skipNodes.pop();
   m_whitespaceMode.pop();

   popColor();

   if (node == Unknown) {
      return true;
   }

   if (m_inStyle && localName == QLatin1String("style")) {
      m_inStyle = false;
   }

   if (node == Graphics) {
      m_nodes.pop();
   } else if (m_style && !m_skipNodes.isEmpty() && m_skipNodes.top() != Style) {
      m_style = 0;
   }

   return true;
}

void QSvgHandler::resolveGradients(QSvgNode *node)
{
   if (!node || (node->type() != QSvgNode::DOC && node->type() != QSvgNode::G
                 && node->type() != QSvgNode::DEFS && node->type() != QSvgNode::SWITCH)) {
      return;
   }
   QSvgStructureNode *structureNode = static_cast<QSvgStructureNode *>(node);

   QList<QSvgNode *> ren = structureNode->renderers();
   for (QList<QSvgNode *>::iterator it = ren.begin(); it != ren.end(); ++it) {
      QSvgFillStyle *fill = static_cast<QSvgFillStyle *>((*it)->styleProperty(QSvgStyleProperty::FILL));
      if (fill && !fill->isGradientResolved()) {
         QString id = fill->gradientId();
         QSvgFillStyleProperty *style = structureNode->styleProperty(id);
         if (style) {
            fill->setFillStyle(style);
         } else {
            qWarning("Could not resolve property : %s", qPrintable(id));
            fill->setBrush(Qt::NoBrush);
         }
      }

      QSvgStrokeStyle *stroke = static_cast<QSvgStrokeStyle *>((*it)->styleProperty(QSvgStyleProperty::STROKE));
      if (stroke && !stroke->isGradientResolved()) {
         QString id = stroke->gradientId();
         QSvgFillStyleProperty *style = structureNode->styleProperty(id);
         if (style) {
            stroke->setStyle(style);
         } else {
            qWarning("Could not resolve property : %s", qPrintable(id));
            stroke->setStroke(Qt::NoBrush);
         }
      }

      resolveGradients(*it);
   }
}

bool QSvgHandler::characters(const QStringRef &str)
{
   if (m_inStyle) {
      QString css = str.toString();
      QCss::StyleSheet sheet;
      QCss::Parser(css).parse(&sheet);
      m_selector->styleSheets.append(sheet);
      return true;
   } else if (m_skipNodes.isEmpty() || m_skipNodes.top() == Unknown || m_nodes.isEmpty()) {
      return true;
   }

   if (m_nodes.top()->type() == QSvgNode::TEXT || m_nodes.top()->type() == QSvgNode::TEXTAREA) {
      static_cast<QSvgText *>(m_nodes.top())->addText(str.toString());
   } else if (m_nodes.top()->type() == QSvgNode::TSPAN) {
      static_cast<QSvgTspan *>(m_nodes.top())->addText(str.toString());
   }

   return true;
}

QSvgTinyDocument *QSvgHandler::document() const
{
   return m_doc;
}

QSvgHandler::LengthType QSvgHandler::defaultCoordinateSystem() const
{
   return m_defaultCoords;
}

void QSvgHandler::setDefaultCoordinateSystem(LengthType type)
{
   m_defaultCoords = type;
}

void QSvgHandler::pushColor(const QColor &color)
{
   m_colorStack.push(color);
   m_colorTagCount.push(1);
}

void QSvgHandler::pushColorCopy()
{
   if (m_colorTagCount.count()) {
      ++m_colorTagCount.top();
   } else {
      pushColor(Qt::black);
   }
}

void QSvgHandler::popColor()
{
   if (m_colorTagCount.count()) {
      if (!--m_colorTagCount.top()) {
         m_colorStack.pop();
         m_colorTagCount.pop();
      }
   }
}

QColor QSvgHandler::currentColor() const
{
   if (!m_colorStack.isEmpty()) {
      return m_colorStack.top();
   } else {
      return QColor(0, 0, 0);
   }
}

void QSvgHandler::setInStyle(bool b)
{
   m_inStyle = b;
}

bool QSvgHandler::inStyle() const
{
   return m_inStyle;
}

QSvgStyleSelector *QSvgHandler::selector() const
{
   return m_selector;
}

bool QSvgHandler::processingInstruction(const QString &target, const QString &data)
{
   if (target == QLatin1String("xml-stylesheet")) {
      QRegExp rx(QLatin1String("type=\\\"(.+)\\\""));
      rx.setMinimal(true);
      bool isCss = false;
      int pos = 0;
      while ((pos = rx.indexIn(data, pos)) != -1) {
         QString type =  rx.cap(1);
         if (type.toLower() == QLatin1String("text/css")) {
            isCss = true;
         }
         pos += rx.matchedLength();
      }

      if (isCss) {
         QRegExp rx(QLatin1String("href=\\\"(.+)\\\""));
         rx.setMinimal(true);
         pos = 0;
         pos = rx.indexIn(data, pos);
         QString addr = rx.cap(1);
         QFileInfo fi(addr);
         //qDebug()<<"External CSS file "<<fi.absoluteFilePath()<<fi.exists();
         if (fi.exists()) {
            QFile file(fi.absoluteFilePath());
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
               return true;
            }
            QByteArray cssData = file.readAll();
            QString css = QString::fromUtf8(cssData);

            QCss::StyleSheet sheet;
            QCss::Parser(css).parse(&sheet);
            m_selector->styleSheets.append(sheet);
         }

      }
   }

   return true;
}

void QSvgHandler::setAnimPeriod(int start, int end)
{
   Q_UNUSED(start);
   m_animEnd   = qMax(end, m_animEnd);
}

int QSvgHandler::animationDuration() const
{
   return m_animEnd;
}

QSvgHandler::~QSvgHandler()
{
   delete m_selector;
   m_selector = 0;

   if (m_ownsReader) {
      delete xml;
   }
}

QT_END_NAMESPACE

#endif // QT_NO_SVG
