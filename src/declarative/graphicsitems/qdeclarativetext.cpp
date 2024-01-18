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

#include "private/qdeclarativetext_p.h"
#include "private/qdeclarativetext_p_p.h"
#include <qdeclarativestyledtext_p.h>
#include <qdeclarativeinfo.h>
#include <qdeclarativepixmapcache_p.h>

#include <QSet>
#include <QTextLayout>
#include <QTextLine>
#include <QTextDocument>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <qmath.h>
#include <limits.h>

QT_BEGIN_NAMESPACE

extern Q_GUI_EXPORT bool qt_applefontsmoothing_enabled;

class QTextDocumentWithImageResources : public QTextDocument
{
   DECL_CS_OBJECT(QTextDocumentWithImageResources)

 public:
   QTextDocumentWithImageResources(QDeclarativeText *parent);
   virtual ~QTextDocumentWithImageResources();

   void setText(const QString &);
   int resourcesLoading() const {
      return outstanding;
   }

 protected:
   QVariant loadResource(int type, const QUrl &name);

 private:
   DECL_CS_SLOT_1(Private, void requestFinished())
   DECL_CS_SLOT_2(requestFinished)

   QHash<QUrl, QDeclarativePixmap *> m_resources;

   int outstanding;
   static QSet<QUrl> errors;
};

DEFINE_BOOL_CONFIG_OPTION(enableImageCache, QML_ENABLE_TEXT_IMAGE_CACHE);

QString QDeclarativeTextPrivate::elideChar = QString(0x2026);

QDeclarativeTextPrivate::QDeclarativeTextPrivate()
   : color((QRgb)0), style(QDeclarativeText::Normal), hAlign(QDeclarativeText::AlignLeft),
     vAlign(QDeclarativeText::AlignTop), elideMode(QDeclarativeText::ElideNone),
     format(QDeclarativeText::AutoText), wrapMode(QDeclarativeText::NoWrap), lineHeight(1),
     lineHeightMode(QDeclarativeText::ProportionalHeight), lineCount(1), truncated(false), maximumLineCount(INT_MAX),
     maximumLineCountValid(false), imageCacheDirty(true), updateOnComponentComplete(true), richText(false),
     singleline(false),
     cacheAllTextAsImage(true), internalWidthUpdate(false), requireImplicitWidth(false),  hAlignImplicit(true),
     rightToLeftText(false), layoutTextElided(false), naturalWidth(0), doc(0)
{
   cacheAllTextAsImage = enableImageCache();
   QGraphicsItemPrivate::acceptedMouseButtons = Qt::LeftButton;
   QGraphicsItemPrivate::flags = QGraphicsItemPrivate::flags & ~QGraphicsItem::ItemHasNoContents;
}

QTextDocumentWithImageResources::QTextDocumentWithImageResources(QDeclarativeText *parent)
   : QTextDocument(parent), outstanding(0)
{
}

QTextDocumentWithImageResources::~QTextDocumentWithImageResources()
{
   if (!m_resources.isEmpty()) {
      qDeleteAll(m_resources);
   }
}

QVariant QTextDocumentWithImageResources::loadResource(int type, const QUrl &name)
{
   QDeclarativeContext *context = qmlContext(parent());
   QUrl url = context->resolvedUrl(name);

   if (type == QTextDocument::ImageResource) {
      QHash<QUrl, QDeclarativePixmap *>::Iterator iter = m_resources.find(url);

      if (iter == m_resources.end()) {
         QDeclarativePixmap *p = new QDeclarativePixmap(context->engine(), url);
         iter = m_resources.insert(name, p);

         if (p->isLoading()) {
            p->connectFinished(this, SLOT(requestFinished()));
            outstanding++;
         }
      }

      QDeclarativePixmap *p = *iter;
      if (p->isReady()) {
         return p->pixmap();
      } else if (p->isError()) {
         if (!errors.contains(url)) {
            errors.insert(url);
            qmlInfo(parent()) << p->error();
         }
      }
   }

   return QTextDocument::loadResource(type, url); // The *resolved* URL
}

void QTextDocumentWithImageResources::requestFinished()
{
   outstanding--;
   if (outstanding == 0) {
      QDeclarativeText *textItem = static_cast<QDeclarativeText *>(parent());
      QString text = textItem->text();
#ifndef QT_NO_TEXTHTMLPARSER
      setHtml(text);
#else
      setPlainText(text);
#endif
      QDeclarativeTextPrivate *d = QDeclarativeTextPrivate::get(textItem);
      d->updateLayout();
   }
}

void QTextDocumentWithImageResources::setText(const QString &text)
{
   if (!m_resources.isEmpty()) {
      qDeleteAll(m_resources);
      m_resources.clear();
      outstanding = 0;
   }

#ifndef QT_NO_TEXTHTMLPARSER
   setHtml(text);
#else
   setPlainText(text);
#endif
}

QSet<QUrl> QTextDocumentWithImageResources::errors;

QDeclarativeTextPrivate::~QDeclarativeTextPrivate()
{
}

qreal QDeclarativeTextPrivate::implicitWidth() const
{
   if (!requireImplicitWidth) {
      // We don't calculate implicitWidth unless it is required.
      // We need to force a size update now to ensure implicitWidth is calculated
      QDeclarativeTextPrivate *me = const_cast<QDeclarativeTextPrivate *>(this);
      me->requireImplicitWidth = true;
      me->updateSize();
   }
   return mImplicitWidth;
}

void QDeclarativeTextPrivate::updateLayout()
{
   Q_Q(QDeclarativeText);
   if (!q->isComponentComplete()) {
      updateOnComponentComplete = true;
      return;
   }

   layoutTextElided = false;
   // Setup instance of QTextLayout for all cases other than richtext
   if (!richText) {
      layout.clearLayout();
      layout.setFont(font);
      if (format != QDeclarativeText::StyledText) {
         QString tmp = text;
         tmp.replace(QLatin1Char('\n'), QChar::LineSeparator);
         singleline = !tmp.contains(QChar::LineSeparator);
         if (singleline && !maximumLineCountValid && elideMode != QDeclarativeText::ElideNone && q->widthValid()) {
            QFontMetrics fm(font);
            tmp = fm.elidedText(tmp, (Qt::TextElideMode)elideMode, q->width());
            if (tmp != text) {
               layoutTextElided = true;
               if (!truncated) {
                  truncated = true;
                  emit q->truncatedChanged();
               }
            }
         }
         layout.setText(tmp);
      } else {
         singleline = false;
         QDeclarativeStyledText::parse(text, layout);
      }
   }

   updateSize();
}

void QDeclarativeTextPrivate::updateSize()
{
   Q_Q(QDeclarativeText);

   if (!q->isComponentComplete()) {
      updateOnComponentComplete = true;
      return;
   }

   if (!requireImplicitWidth) {
      emit q->implicitWidthChanged();
      // if the implicitWidth is used, then updateSize() has already been called (recursively)
      if (requireImplicitWidth) {
         return;
      }
   }

   invalidateImageCache();

   QFontMetrics fm(font);
   if (text.isEmpty()) {
      q->setImplicitWidth(0);
      q->setImplicitHeight(fm.height());
      paintedSize = QSize(0, fm.height());
      emit q->paintedSizeChanged();
      q->update();
      return;
   }

   int dy = q->height();
   QSize size(0, 0);

   //setup instance of QTextLayout for all cases other than richtext
   if (!richText) {
      QRect textRect = setupTextLayout();
      if (layedOutTextRect.size() != textRect.size()) {
         q->prepareGeometryChange();
      }
      layedOutTextRect = textRect;
      size = textRect.size();
      dy -= size.height();
   } else {
      singleline = false; // richtext can't elide or be optimized for single-line case
      ensureDoc();
      doc->setDefaultFont(font);

      QDeclarativeText::HAlignment horizontalAlignment = q->effectiveHAlign();
      if (rightToLeftText) {
         if (horizontalAlignment == QDeclarativeText::AlignLeft) {
            horizontalAlignment = QDeclarativeText::AlignRight;
         } else if (horizontalAlignment == QDeclarativeText::AlignRight) {
            horizontalAlignment = QDeclarativeText::AlignLeft;
         }
      }
      QTextOption option;
      option.setAlignment((Qt::Alignment)int(horizontalAlignment | vAlign));
      option.setWrapMode(QTextOption::WrapMode(wrapMode));
      doc->setDefaultTextOption(option);
      if (requireImplicitWidth && q->widthValid()) {
         doc->setTextWidth(-1);
         naturalWidth = doc->idealWidth();
      }
      if (q->widthValid()) {
         doc->setTextWidth(q->width());
      } else {
         doc->setTextWidth(doc->idealWidth());   // ### Text does not align if width is not set (QTextDoc bug)
      }
      dy -= (int)doc->size().height();
      QSize dsize = doc->size().toSize();
      if (dsize != layedOutTextRect.size()) {
         q->prepareGeometryChange();
         layedOutTextRect = QRect(QPoint(0, 0), dsize);
      }
      size = QSize(int(doc->idealWidth()), dsize.height());
   }
   int yoff = 0;

   if (q->heightValid()) {
      if (vAlign == QDeclarativeText::AlignBottom) {
         yoff = dy;
      } else if (vAlign == QDeclarativeText::AlignVCenter) {
         yoff = dy / 2;
      }
   }
   q->setBaselineOffset(fm.ascent() + yoff);

   //### need to comfirm cost of always setting these for richText
   internalWidthUpdate = true;
   if (!q->widthValid()) {
      q->setImplicitWidth(size.width());
   } else if (requireImplicitWidth) {
      q->setImplicitWidth(naturalWidth);
   }
   internalWidthUpdate = false;
   q->setImplicitHeight(size.height());
   if (paintedSize != size) {
      paintedSize = size;
      emit q->paintedSizeChanged();
   }
   q->update();
}

/*!
    Lays out the QDeclarativeTextPrivate::layout QTextLayout in the constraints of the QDeclarativeText.

    Returns the size of the final text.  This can be used to position the text vertically (the text is
    already absolutely positioned horizontally).
*/
QRect QDeclarativeTextPrivate::setupTextLayout()
{
   // ### text layout handling should be profiled and optimized as needed
   // what about QStackTextEngine engine(tmp, d->font.font()); QTextLayout textLayout(&engine);
   Q_Q(QDeclarativeText);
   layout.setCacheEnabled(true);

   qreal lineWidth = 0;
   int visibleCount = 0;

   //set manual width
   if (q->widthValid()) {
      lineWidth = q->width();
   }

   QTextOption textOption = layout.textOption();
   textOption.setAlignment(Qt::Alignment(q->effectiveHAlign()));
   textOption.setWrapMode(QTextOption::WrapMode(wrapMode));
   layout.setTextOption(textOption);

   bool elideText = false;
   bool truncate = false;

   QFontMetrics fm(layout.font());
   elidePos = QPointF();

   if (requireImplicitWidth && q->widthValid()) {
      // requires an extra layout
      QString elidedText;
      if (layoutTextElided) {
         // We have provided elided text to the layout, but we must calculate unelided width.
         elidedText = layout.text();
         layout.setText(text);
      }
      layout.beginLayout();
      forever {
         QTextLine line = layout.createLine();
         if (!line.isValid())
         {
            break;
         }
      }
      layout.endLayout();
      QRectF br;
      for (int i = 0; i < layout.lineCount(); ++i) {
         QTextLine line = layout.lineAt(i);
         br = br.united(line.naturalTextRect());
      }
      naturalWidth = br.width();
      if (layoutTextElided) {
         layout.setText(elidedText);
      }
   }

   if (maximumLineCountValid) {
      layout.beginLayout();
      if (!lineWidth) {
         lineWidth = INT_MAX;
      }
      int linesLeft = maximumLineCount;
      int visibleTextLength = 0;
      while (linesLeft > 0) {
         QTextLine line = layout.createLine();
         if (!line.isValid()) {
            break;
         }

         visibleCount++;
         if (lineWidth) {
            line.setLineWidth(lineWidth);
         }
         visibleTextLength += line.textLength();

         if (--linesLeft == 0) {
            if (visibleTextLength < text.length()) {
               truncate = true;
               if (elideMode == QDeclarativeText::ElideRight && q->widthValid()) {
                  qreal elideWidth = fm.width(elideChar);
                  // Need to correct for alignment
                  line.setLineWidth(lineWidth - elideWidth);
                  if (layout.text().mid(line.textStart(), line.textLength()).isRightToLeft()) {
                     line.setPosition(QPointF(line.position().x() + elideWidth, line.position().y()));
                     elidePos.setX(line.naturalTextRect().left() - elideWidth);
                  } else {
                     elidePos.setX(line.naturalTextRect().right());
                  }
                  elideText = true;
               }
            }
         }
      }
      layout.endLayout();

      //Update truncated
      if (truncated != truncate) {
         truncated = truncate;
         emit q->truncatedChanged();
      }
   } else {
      layout.beginLayout();
      forever {
         QTextLine line = layout.createLine();
         if (!line.isValid())
         {
            break;
         }
         visibleCount++;
         if (lineWidth)
         {
            line.setLineWidth(lineWidth);
         }
      }
      layout.endLayout();
   }

   qreal height = 0;
   QRectF br;
   for (int i = 0; i < layout.lineCount(); ++i) {
      QTextLine line = layout.lineAt(i);
      // set line spacing
      line.setPosition(QPointF(line.position().x(), height));
      if (elideText && i == layout.lineCount() - 1) {
         elidePos.setY(height + fm.ascent());
         br = br.united(QRectF(elidePos, QSizeF(fm.width(elideChar), fm.ascent())));
      }
      br = br.united(line.naturalTextRect());
      height += (lineHeightMode == QDeclarativeText::FixedHeight) ? lineHeight : line.height() * lineHeight;
   }
   br.setHeight(height);

   if (!q->widthValid()) {
      naturalWidth = br.width();
   }

   //Update the number of visible lines
   if (lineCount != visibleCount) {
      lineCount = visibleCount;
      emit q->lineCountChanged();
   }

   return QRect(qRound(br.x()), qRound(br.y()), qCeil(br.width()), qCeil(br.height()));
}

/*!
    Returns a painted version of the QDeclarativeTextPrivate::layout QTextLayout.
    If \a drawStyle is true, the style color overrides all colors in the document.
*/
QPixmap QDeclarativeTextPrivate::textLayoutImage(bool drawStyle)
{
   //do layout
   QSize size = layedOutTextRect.size();
   //paint text
   QPixmap img(size);
   if (!size.isEmpty()) {
      img.fill(Qt::transparent);
#ifdef Q_OS_DARWIN
      bool oldSmooth = qt_applefontsmoothing_enabled;
      qt_applefontsmoothing_enabled = false;
#endif
      QPainter p(&img);
#ifdef Q_OS_DARWIN
      qt_applefontsmoothing_enabled = oldSmooth;
#endif
      drawTextLayout(&p, QPointF(-layedOutTextRect.x(), 0), drawStyle);
   }
   return img;
}

/*!
    Paints the QDeclarativeTextPrivate::layout QTextLayout into \a painter at \a pos.  If
    \a drawStyle is true, the style color overrides all colors in the document.
*/
void QDeclarativeTextPrivate::drawTextLayout(QPainter *painter, const QPointF &pos, bool drawStyle)
{
   if (drawStyle) {
      painter->setPen(styleColor);
   } else {
      painter->setPen(color);
   }
   painter->setFont(font);
   layout.draw(painter, pos);
   if (!elidePos.isNull()) {
      painter->drawText(pos + elidePos, elideChar);
   }
}

/*!
    Returns a painted version of the QDeclarativeTextPrivate::doc QTextDocument.
    If \a drawStyle is true, the style color overrides all colors in the document.
*/
QPixmap QDeclarativeTextPrivate::textDocumentImage(bool drawStyle)
{
   QSize size = doc->size().toSize();

   //paint text
   QPixmap img(size);
   img.fill(Qt::transparent);
#ifdef Q_OS_DARWIN
   bool oldSmooth = qt_applefontsmoothing_enabled;
   qt_applefontsmoothing_enabled = false;
#endif
   QPainter p(&img);
#ifdef Q_OS_DARWIN
   qt_applefontsmoothing_enabled = oldSmooth;
#endif

   QAbstractTextDocumentLayout::PaintContext context;

   QTextOption oldOption(doc->defaultTextOption());
   if (drawStyle) {
      context.palette.setColor(QPalette::Text, styleColor);
      QTextOption colorOption(doc->defaultTextOption());
      colorOption.setFlags(QTextOption::SuppressColors);
      doc->setDefaultTextOption(colorOption);
   } else {
      context.palette.setColor(QPalette::Text, color);
   }
   doc->documentLayout()->draw(&p, context);
   if (drawStyle) {
      doc->setDefaultTextOption(oldOption);
   }
   return img;
}

/*!
    Mark the image cache as dirty.
*/
void QDeclarativeTextPrivate::invalidateImageCache()
{
   Q_Q(QDeclarativeText);

   if (cacheAllTextAsImage || style != QDeclarativeText::Normal) { //If actually using the image cache
      if (imageCacheDirty) {
         return;
      }

      imageCacheDirty = true;
      imageCache = QPixmap();
   }
   if (q->isComponentComplete()) {
      q->update();
   }
}

/*!
    Tests if the image cache is dirty, and repaints it if it is.
*/
void QDeclarativeTextPrivate::checkImageCache()
{
   if (!imageCacheDirty) {
      return;
   }

   if (text.isEmpty()) {

      imageCache = QPixmap();

   } else {

      QPixmap textImage;
      QPixmap styledImage;

      if (richText) {
         textImage = textDocumentImage(false);
         if (style != QDeclarativeText::Normal) {
            styledImage = textDocumentImage(true);   //### should use styleColor
         }
      } else {
         textImage = textLayoutImage(false);
         if (style != QDeclarativeText::Normal) {
            styledImage = textLayoutImage(true);   //### should use styleColor
         }
      }

      switch (style) {
         case QDeclarativeText::Outline:
            imageCache = drawOutline(textImage, styledImage);
            break;
         case QDeclarativeText::Sunken:
            imageCache = drawOutline(textImage, styledImage, -1);
            break;
         case QDeclarativeText::Raised:
            imageCache = drawOutline(textImage, styledImage, 1);
            break;
         default:
            imageCache = textImage;
            break;
      }

   }

   imageCacheDirty = false;
}

/*!
    Ensures the QDeclarativeTextPrivate::doc variable is set to a valid text document
*/
void QDeclarativeTextPrivate::ensureDoc()
{
   if (!doc) {
      Q_Q(QDeclarativeText);
      doc = new QTextDocumentWithImageResources(q);
      doc->setDocumentMargin(0);
   }
}

/*!
    Draw \a styleSource as an outline around \a source and return the new image.
*/
QPixmap QDeclarativeTextPrivate::drawOutline(const QPixmap &source, const QPixmap &styleSource)
{
   QPixmap img = QPixmap(styleSource.width() + 2, styleSource.height() + 2);
   img.fill(Qt::transparent);

   QPainter ppm(&img);

   QPoint pos(0, 0);
   pos += QPoint(-1, 0);
   ppm.drawPixmap(pos, styleSource);
   pos += QPoint(2, 0);
   ppm.drawPixmap(pos, styleSource);
   pos += QPoint(-1, -1);
   ppm.drawPixmap(pos, styleSource);
   pos += QPoint(0, 2);
   ppm.drawPixmap(pos, styleSource);

   pos += QPoint(0, -1);
   ppm.drawPixmap(pos, source);
   ppm.end();

   return img;
}

/*!
    Draw \a styleSource below \a source at \a yOffset and return the new image.
*/
QPixmap QDeclarativeTextPrivate::drawOutline(const QPixmap &source, const QPixmap &styleSource, int yOffset)
{
   QPixmap img = QPixmap(styleSource.width() + 2, styleSource.height() + 2);
   img.fill(Qt::transparent);

   QPainter ppm(&img);

   ppm.drawPixmap(QPoint(0, yOffset), styleSource);
   ppm.drawPixmap(0, 0, source);

   ppm.end();

   return img;
}

/*!
    \qmlclass Text QDeclarativeText
    \ingroup qml-basic-visual-elements
    \since 4.7
    \brief The Text item allows you to add formatted text to a scene.
    \inherits Item

    Text items can display both plain and rich text. For example, red text with
    a specific font and size can be defined like this:

    \qml
    Text {
        text: "Hello World!"
        font.family: "Helvetica"
        font.pointSize: 24
        color: "red"
    }
    \endqml

    Rich text is defined using HTML-style markup:

    \qml
    Text {
        text: "<b>Hello</b> <i>World!</i>"
    }
    \endqml

    \image declarative-text.png

    If height and width are not explicitly set, Text will attempt to determine how
    much room is needed and set it accordingly. Unless \l wrapMode is set, it will always
    prefer width to height (all text will be placed on a single line).

    The \l elide property can alternatively be used to fit a single line of
    plain text to a set width.

    Note that the \l{Supported HTML Subset} is limited. Also, if the text contains
    HTML img tags that load remote images, the text is reloaded.

    Text provides read-only text. For editable text, see \l TextEdit.

    \sa {declarative/text/fonts}{Fonts example}
*/
QDeclarativeText::QDeclarativeText(QDeclarativeItem *parent)
   : QDeclarativeImplicitSizeItem(*(new QDeclarativeTextPrivate), parent)
{
}

QDeclarativeText::~QDeclarativeText()
{
}

QFont QDeclarativeText::font() const
{
   Q_D(const QDeclarativeText);
   return d->sourceFont;
}

void QDeclarativeText::setFont(const QFont &font)
{
   Q_D(QDeclarativeText);
   if (d->sourceFont == font) {
      return;
   }

   d->sourceFont = font;
   QFont oldFont = d->font;
   d->font = font;
   if (d->font.pointSizeF() != -1) {
      // 0.5pt resolution
      qreal size = qRound(d->font.pointSizeF() * 2.0);
      d->font.setPointSizeF(size / 2.0);
   }

   if (oldFont != d->font) {
      d->updateLayout();
   }

   emit fontChanged(d->sourceFont);
}

/*!
    \qmlproperty string Text::text

    The text to display. Text supports both plain and rich text strings.

    The item will try to automatically determine whether the text should
    be treated as rich text. This determination is made using Qt::mightBeRichText().
*/
QString QDeclarativeText::text() const
{
   Q_D(const QDeclarativeText);
   return d->text;
}

void QDeclarativeText::setText(const QString &n)
{
   Q_D(QDeclarativeText);
   if (d->text == n) {
      return;
   }

   d->richText = d->format == RichText || (d->format == AutoText && Qt::mightBeRichText(n));
   d->text = n;
   if (isComponentComplete()) {
      if (d->richText) {
         d->ensureDoc();
         d->doc->setText(n);
         d->rightToLeftText = d->doc->toPlainText().isRightToLeft();
      } else {
         d->rightToLeftText = d->text.isRightToLeft();
      }
      d->determineHorizontalAlignment();
   }
   d->updateLayout();
   emit textChanged(d->text);
}


/*!
    \qmlproperty color Text::color

    The text color.

    An example of green text defined using hexadecimal notation:
    \qml
    Text {
        color: "#00FF00"
        text: "green text"
    }
    \endqml

    An example of steel blue text defined using an SVG color name:
    \qml
    Text {
        color: "steelblue"
        text: "blue text"
    }
    \endqml
*/
QColor QDeclarativeText::color() const
{
   Q_D(const QDeclarativeText);
   return d->color;
}

void QDeclarativeText::setColor(const QColor &color)
{
   Q_D(QDeclarativeText);
   if (d->color == color) {
      return;
   }

   d->color = color;
   d->invalidateImageCache();
   emit colorChanged(d->color);
}

/*!
    \qmlproperty enumeration Text::style

    Set an additional text style.

    Supported text styles are:
    \list
    \o Text.Normal - the default
    \o Text.Outline
    \o Text.Raised
    \o Text.Sunken
    \endlist

    \qml
    Row {
        Text { font.pointSize: 24; text: "Normal" }
        Text { font.pointSize: 24; text: "Raised"; style: Text.Raised; styleColor: "#AAAAAA" }
        Text { font.pointSize: 24; text: "Outline";style: Text.Outline; styleColor: "red" }
        Text { font.pointSize: 24; text: "Sunken"; style: Text.Sunken; styleColor: "#AAAAAA" }
    }
    \endqml

    \image declarative-textstyle.png
*/
QDeclarativeText::TextStyle QDeclarativeText::style() const
{
   Q_D(const QDeclarativeText);
   return d->style;
}

void QDeclarativeText::setStyle(QDeclarativeText::TextStyle style)
{
   Q_D(QDeclarativeText);
   if (d->style == style) {
      return;
   }

   // changing to/from Normal requires the boundingRect() to change
   if (isComponentComplete() && (d->style == Normal || style == Normal)) {
      prepareGeometryChange();
   }
   d->style = style;
   d->invalidateImageCache();
   emit styleChanged(d->style);
}

/*!
    \qmlproperty color Text::styleColor

    Defines the secondary color used by text styles.

    \c styleColor is used as the outline color for outlined text, and as the
    shadow color for raised or sunken text. If no style has been set, it is not
    used at all.

    \qml
    Text { font.pointSize: 18; text: "hello"; style: Text.Raised; styleColor: "gray" }
    \endqml

    \sa style
 */
QColor QDeclarativeText::styleColor() const
{
   Q_D(const QDeclarativeText);
   return d->styleColor;
}

void QDeclarativeText::setStyleColor(const QColor &color)
{
   Q_D(QDeclarativeText);
   if (d->styleColor == color) {
      return;
   }

   d->styleColor = color;
   d->invalidateImageCache();
   emit styleColorChanged(d->styleColor);
}


/*!
    \qmlproperty enumeration Text::horizontalAlignment
    \qmlproperty enumeration Text::verticalAlignment

    Sets the horizontal and vertical alignment of the text within the Text items
    width and height. By default, the text is vertically aligned to the top. Horizontal
    alignment follows the natural alignment of the text, for example text that is read
    from left to right will be aligned to the left.

    The valid values for \c horizontalAlignment are \c Text.AlignLeft, \c Text.AlignRight, \c Text.AlignHCenter and
    \c Text.AlignJustify.  The valid values for \c verticalAlignment are \c Text.AlignTop, \c Text.AlignBottom
    and \c Text.AlignVCenter.

    Note that for a single line of text, the size of the text is the area of the text. In this common case,
    all alignments are equivalent. If you want the text to be, say, centered in its parent, then you will
    need to either modify the Item::anchors, or set horizontalAlignment to Text.AlignHCenter and bind the width to
    that of the parent.

    When using the attached property \l {LayoutMirroring::enabled} to mirror application
    layouts, the horizontal alignment of text will also be mirrored. However, the property
    \c horizontalAlignment will remain unchanged. To query the effective horizontal alignment
    of Text, use the property \l {LayoutMirroring::enabled}.
*/
QDeclarativeText::HAlignment QDeclarativeText::hAlign() const
{
   Q_D(const QDeclarativeText);
   return d->hAlign;
}

void QDeclarativeText::setHAlign(HAlignment align)
{
   Q_D(QDeclarativeText);
   bool forceAlign = d->hAlignImplicit && d->effectiveLayoutMirror;
   d->hAlignImplicit = false;
   if (d->setHAlign(align, forceAlign) && isComponentComplete()) {
      d->updateLayout();
   }
}

void QDeclarativeText::resetHAlign()
{
   Q_D(QDeclarativeText);
   d->hAlignImplicit = true;
   if (d->determineHorizontalAlignment() && isComponentComplete()) {
      d->updateLayout();
   }
}

QDeclarativeText::HAlignment QDeclarativeText::effectiveHAlign() const
{
   Q_D(const QDeclarativeText);
   QDeclarativeText::HAlignment effectiveAlignment = d->hAlign;
   if (!d->hAlignImplicit && d->effectiveLayoutMirror) {
      switch (d->hAlign) {
         case QDeclarativeText::AlignLeft:
            effectiveAlignment = QDeclarativeText::AlignRight;
            break;
         case QDeclarativeText::AlignRight:
            effectiveAlignment = QDeclarativeText::AlignLeft;
            break;
         default:
            break;
      }
   }
   return effectiveAlignment;
}

bool QDeclarativeTextPrivate::setHAlign(QDeclarativeText::HAlignment alignment, bool forceAlign)
{
   Q_Q(QDeclarativeText);
   if (hAlign != alignment || forceAlign) {
      hAlign = alignment;
      emit q->horizontalAlignmentChanged(hAlign);
      return true;
   }
   return false;
}

bool QDeclarativeTextPrivate::determineHorizontalAlignment()
{
   Q_Q(QDeclarativeText);
   if (hAlignImplicit && q->isComponentComplete()) {
      bool alignToRight = text.isEmpty() ? QApplication::keyboardInputDirection() == Qt::RightToLeft : rightToLeftText;
      return setHAlign(alignToRight ? QDeclarativeText::AlignRight : QDeclarativeText::AlignLeft);
   }
   return false;
}

void QDeclarativeTextPrivate::mirrorChange()
{
   Q_Q(QDeclarativeText);
   if (q->isComponentComplete()) {
      if (!hAlignImplicit && (hAlign == QDeclarativeText::AlignRight || hAlign == QDeclarativeText::AlignLeft)) {
         updateLayout();
      }
   }
}

QTextDocument *QDeclarativeTextPrivate::textDocument()
{
   return doc;
}

QDeclarativeText::VAlignment QDeclarativeText::vAlign() const
{
   Q_D(const QDeclarativeText);
   return d->vAlign;
}

void QDeclarativeText::setVAlign(VAlignment align)
{
   Q_D(QDeclarativeText);
   if (d->vAlign == align) {
      return;
   }

   if (isComponentComplete()) {
      prepareGeometryChange();
   }
   d->vAlign = align;
   emit verticalAlignmentChanged(align);
}

/*!
    \qmlproperty enumeration Text::wrapMode

    Set this property to wrap the text to the Text item's width.  The text will only
    wrap if an explicit width has been set.  wrapMode can be one of:

    \list
    \o Text.NoWrap (default) - no wrapping will be performed. If the text contains insufficient newlines, then \l paintedWidth will exceed a set width.
    \o Text.WordWrap - wrapping is done on word boundaries only. If a word is too long, \l paintedWidth will exceed a set width.
    \o Text.WrapAnywhere - wrapping is done at any point on a line, even if it occurs in the middle of a word.
    \o Text.Wrap - if possible, wrapping occurs at a word boundary; otherwise it will occur at the appropriate point on the line, even in the middle of a word.
    \endlist
*/
QDeclarativeText::WrapMode QDeclarativeText::wrapMode() const
{
   Q_D(const QDeclarativeText);
   return d->wrapMode;
}

void QDeclarativeText::setWrapMode(WrapMode mode)
{
   Q_D(QDeclarativeText);
   if (mode == d->wrapMode) {
      return;
   }

   d->wrapMode = mode;
   d->updateLayout();

   emit wrapModeChanged();
}

/*!
    \qmlproperty int Text::lineCount
    \since QtQuick 1.1

    Returns the number of lines visible in the text item.

    This property is not supported for rich text.

    \sa maximumLineCount
*/
int QDeclarativeText::lineCount() const
{
   Q_D(const QDeclarativeText);
   return d->lineCount;
}

/*!
    \qmlproperty bool Text::truncated
    \since QtQuick 1.1

    Returns true if the text has been truncated due to \l maximumLineCount
    or \l elide.

    This property is not supported for rich text.

    \sa maximumLineCount, elide
*/
bool QDeclarativeText::truncated() const
{
   Q_D(const QDeclarativeText);
   return d->truncated;
}

/*!
    \qmlproperty int Text::maximumLineCount
    \since QtQuick 1.1

    Set this property to limit the number of lines that the text item will show.
    If elide is set to Text.ElideRight, the text will be elided appropriately.
    By default, this is the value of the largest possible integer.

    This property is not supported for rich text.

    \sa lineCount, elide
*/
int QDeclarativeText::maximumLineCount() const
{
   Q_D(const QDeclarativeText);
   return d->maximumLineCount;
}

void QDeclarativeText::setMaximumLineCount(int lines)
{
   Q_D(QDeclarativeText);

   d->maximumLineCountValid = lines == INT_MAX ? false : true;
   if (d->maximumLineCount != lines) {
      d->maximumLineCount = lines;
      d->updateLayout();
      emit maximumLineCountChanged();
   }
}

void QDeclarativeText::resetMaximumLineCount()
{
   Q_D(QDeclarativeText);
   setMaximumLineCount(INT_MAX);
   d->elidePos = QPointF();
   if (d->truncated != false) {
      d->truncated = false;
      emit truncatedChanged();
   }
}

/*!
    \qmlproperty enumeration Text::textFormat

    The way the text property should be displayed.

    Supported text formats are:

    \list
    \o Text.AutoText (default)
    \o Text.PlainText
    \o Text.RichText
    \o Text.StyledText
    \endlist

    If the text format is \c Text.AutoText the text element
    will automatically determine whether the text should be treated as
    rich text.  This determination is made using Qt::mightBeRichText().

    Text.StyledText is an optimized format supporting some basic text
    styling markup, in the style of html 3.2:

    \code
    <font size="4" color="#ff0000">font size and color</font>
    <b>bold</b>
    <i>italic</i>
    <br>
    &gt; &lt; &amp;
    \endcode

    \c Text.StyledText parser is strict, requiring tags to be correctly nested.

    \table
    \row
    \o
    \qml
Column {
    Text {
        font.pointSize: 24
        text: "<b>Hello</b> <i>World!</i>"
    }
    Text {
        font.pointSize: 24
        textFormat: Text.RichText
        text: "<b>Hello</b> <i>World!</i>"
    }
    Text {
        font.pointSize: 24
        textFormat: Text.PlainText
        text: "<b>Hello</b> <i>World!</i>"
    }
}
    \endqml
    \o \image declarative-textformat.png
    \endtable
*/
QDeclarativeText::TextFormat QDeclarativeText::textFormat() const
{
   Q_D(const QDeclarativeText);
   return d->format;
}

void QDeclarativeText::setTextFormat(TextFormat format)
{
   Q_D(QDeclarativeText);
   if (format == d->format) {
      return;
   }
   d->format = format;
   bool wasRich = d->richText;
   d->richText = format == RichText || (format == AutoText && Qt::mightBeRichText(d->text));

   if (!wasRich && d->richText && isComponentComplete()) {
      d->ensureDoc();
      d->doc->setText(d->text);
   }

   d->updateLayout();

   emit textFormatChanged(d->format);
}

/*!
    \qmlproperty enumeration Text::elide

    Set this property to elide parts of the text fit to the Text item's width.
    The text will only elide if an explicit width has been set.

    This property cannot be used with rich text.

    Eliding can be:
    \list
    \o Text.ElideNone  - the default
    \o Text.ElideLeft
    \o Text.ElideMiddle
    \o Text.ElideRight
    \endlist

    If this property is set to Text.ElideRight, it can be used with multiline
    text. The text will only elide if maximumLineCount has been set.

    If the text is a multi-length string, and the mode is not \c Text.ElideNone,
    the first string that fits will be used, otherwise the last will be elided.

    Multi-length strings are ordered from longest to shortest, separated by the
    Unicode "String Terminator" character \c U009C (write this in QML with \c{"\u009C"} or \c{"\x9C"}).
*/
QDeclarativeText::TextElideMode QDeclarativeText::elideMode() const
{
   Q_D(const QDeclarativeText);
   return d->elideMode;
}

void QDeclarativeText::setElideMode(QDeclarativeText::TextElideMode mode)
{
   Q_D(QDeclarativeText);
   if (mode == d->elideMode) {
      return;
   }

   d->elideMode = mode;
   d->updateLayout();

   emit elideModeChanged(d->elideMode);
}

/*! \internal */
QRectF QDeclarativeText::boundingRect() const
{
   Q_D(const QDeclarativeText);

   QRect rect = d->layedOutTextRect;
   if (d->style != Normal) {
      rect.adjust(-1, 0, 1, 2);
   }

   // Could include font max left/right bearings to either side of rectangle.

   int h = height();
   switch (d->vAlign) {
      case AlignTop:
         break;
      case AlignBottom:
         rect.moveTop(h - rect.height());
         break;
      case AlignVCenter:
         rect.moveTop((h - rect.height()) / 2);
         break;
   }

   return QRectF(rect);
}

/*! \internal */
void QDeclarativeText::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
   Q_D(QDeclarativeText);
   if ((!d->internalWidthUpdate && newGeometry.width() != oldGeometry.width())
         && (d->wrapMode != QDeclarativeText::NoWrap
             || d->elideMode != QDeclarativeText::ElideNone
             || d->hAlign != QDeclarativeText::AlignLeft)) {
      if ((d->singleline || d->maximumLineCountValid) && d->elideMode != QDeclarativeText::ElideNone && widthValid()) {
         // We need to re-elide
         d->updateLayout();
      } else {
         // We just need to re-layout
         d->updateSize();
      }
   }

   QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
}

/*!
    \qmlproperty real Text::paintedWidth

    Returns the width of the text, including width past the width
    which is covered due to insufficient wrapping if WrapMode is set.
*/
qreal QDeclarativeText::paintedWidth() const
{
   Q_D(const QDeclarativeText);
   return d->paintedSize.width();
}

/*!
    \qmlproperty real Text::paintedHeight

    Returns the height of the text, including height past the height
    which is covered due to there being more text than fits in the set height.
*/
qreal QDeclarativeText::paintedHeight() const
{
   Q_D(const QDeclarativeText);
   return d->paintedSize.height();
}

/*!
    \qmlproperty real Text::lineHeight
    \since QtQuick 1.1

    Sets the line height for the text.
    The value can be in pixels or a multiplier depending on lineHeightMode.

    The default value is a multiplier of 1.0.
    The line height must be a positive value.
*/
qreal QDeclarativeText::lineHeight() const
{
   Q_D(const QDeclarativeText);
   return d->lineHeight;
}

void QDeclarativeText::setLineHeight(qreal lineHeight)
{
   Q_D(QDeclarativeText);

   if ((d->lineHeight == lineHeight) || (lineHeight < 0.0)) {
      return;
   }

   d->lineHeight = lineHeight;
   d->updateLayout();
   emit lineHeightChanged(lineHeight);
}

/*!
    \qmlproperty enumeration Text::lineHeightMode

    This property determines how the line height is specified.
    The possible values are:

    \list
    \o Text.ProportionalHeight (default) - this sets the spacing proportional to the
       line (as a multiplier). For example, set to 2 for double spacing.
    \o Text.FixedHeight - this sets the line height to a fixed line height (in pixels).
    \endlist
*/
QDeclarativeText::LineHeightMode QDeclarativeText::lineHeightMode() const
{
   Q_D(const QDeclarativeText);
   return d->lineHeightMode;
}

void QDeclarativeText::setLineHeightMode(LineHeightMode mode)
{
   Q_D(QDeclarativeText);
   if (mode == d->lineHeightMode) {
      return;
   }

   d->lineHeightMode = mode;
   d->updateLayout();

   emit lineHeightModeChanged(mode);
}

/*!
    Returns the number of resources (images) that are being loaded asynchronously.
*/
int QDeclarativeText::resourcesLoading() const
{
   Q_D(const QDeclarativeText);
   return d->doc ? d->doc->resourcesLoading() : 0;
}

/*! \internal */
void QDeclarativeText::paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
{
   Q_D(QDeclarativeText);

   if (d->cacheAllTextAsImage || d->style != Normal) {
      d->checkImageCache();
      if (d->imageCache.isNull()) {
         return;
      }

      bool oldAA = p->testRenderHint(QPainter::Antialiasing);
      bool oldSmooth = p->testRenderHint(QPainter::SmoothPixmapTransform);
      if (d->smooth) {
         p->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, d->smooth);
      }

      QRect br = boundingRect().toRect();

      bool needClip = clip() && (d->imageCache.width() > width() ||
                                 d->imageCache.height() > height());

      if (needClip) {
         p->drawPixmap(0, 0, width(), height(), d->imageCache, -br.x(), -br.y(), width(), height());
      } else {
         p->drawPixmap(br.x(), br.y(), d->imageCache);
      }

      if (d->smooth) {
         p->setRenderHint(QPainter::Antialiasing, oldAA);
         p->setRenderHint(QPainter::SmoothPixmapTransform, oldSmooth);
      }
   } else {
      QRectF bounds = boundingRect();

      bool needClip = clip() && (d->layedOutTextRect.width() > width() ||
                                 d->layedOutTextRect.height() > height());

      if (needClip) {
         p->save();
         p->setClipRect(0, 0, width(), height(), Qt::IntersectClip);
      }
      if (d->richText) {
         QAbstractTextDocumentLayout::PaintContext context;
         context.palette.setColor(QPalette::Text, d->color);
         p->translate(bounds.x(), bounds.y());
         d->doc->documentLayout()->draw(p, context);
         p->translate(-bounds.x(), -bounds.y());
      } else {
         d->drawTextLayout(p, QPointF(0, bounds.y()), false);
      }

      if (needClip) {
         p->restore();
      }
   }
}

/*! \internal */
void QDeclarativeText::componentComplete()
{
   Q_D(QDeclarativeText);
   QDeclarativeItem::componentComplete();
   if (d->updateOnComponentComplete) {
      d->updateOnComponentComplete = false;
      if (d->richText) {
         d->ensureDoc();
         d->doc->setText(d->text);
         d->rightToLeftText = d->doc->toPlainText().isRightToLeft();
      } else {
         d->rightToLeftText = d->text.isRightToLeft();
      }
      d->determineHorizontalAlignment();
      d->updateLayout();
   }
}

/*!  \internal */
void QDeclarativeText::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativeText);

   if (!d->richText || !d->doc || d->doc->documentLayout()->anchorAt(event->pos()).isEmpty()) {
      event->setAccepted(false);
      d->activeLink.clear();
   } else {
      d->activeLink = d->doc->documentLayout()->anchorAt(event->pos());
   }

   // ### may malfunction if two of the same links are clicked & dragged onto each other)

   if (!event->isAccepted()) {
      QDeclarativeItem::mousePressEvent(event);
   }

}

/*! \internal */
void QDeclarativeText::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
   Q_D(QDeclarativeText);

   // ### confirm the link, and send a signal out
   if (d->richText && d->doc && d->activeLink == d->doc->documentLayout()->anchorAt(event->pos())) {
      emit linkActivated(d->activeLink);
   } else {
      event->setAccepted(false);
   }

   if (!event->isAccepted()) {
      QDeclarativeItem::mouseReleaseEvent(event);
   }
}

QT_END_NAMESPACE
