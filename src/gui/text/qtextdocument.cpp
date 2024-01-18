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

#include <qtextdocument.h>
#include <qtextformat.h>

#include <qtextdocumentfragment.h>
#include <qtexttable.h>
#include <qtextlist.h>
#include <qdebug.h>
#include <qregularexpression.h>
#include <qvarlengtharray.h>
#include <qtextcodec.h>
#include <qthread.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qtextedit.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

#include <qapplication.h>
#include <qtexthtmlparser_p.h>
#include <qtextcontrol_p.h>
#include <qfont_p.h>
#include <qtextedit_p.h>
#include <qdataurl_p.h>
#include <qtextdocument_p.h>
#include <qprinter_p.h>
#include <qtextdocumentfragment_p.h>
#include <qabstracttextdocumentlayout_p.h>
#include <qtextcursor_p.h>
#include <qtextdocumentlayout_p.h>

#include <limits.h>

Q_CORE_EXPORT unsigned int qt_int_sqrt(unsigned int n);

QTextDocument::QTextDocument(QObject *parent)
   : QObject(parent), d_ptr(new QTextDocumentPrivate)
{
   d_ptr->q_ptr = this;

   Q_D(QTextDocument);
   d->init();
}

QTextDocument::QTextDocument(const QString &text, QObject *parent)
   : QObject(parent), d_ptr(new QTextDocumentPrivate)
{
   d_ptr->q_ptr = this;

   Q_D(QTextDocument);

   d->init();
   QTextCursor(this).insertText(text);
}

// internal
QTextDocument::QTextDocument(QTextDocumentPrivate &dd, QObject *parent)
   : QObject(parent), d_ptr(&dd)
{
   d_ptr->q_ptr = this;

   Q_D(QTextDocument);
   d->init();
}

QTextDocument::~QTextDocument()
{
}

QTextDocument *QTextDocument::clone(QObject *parent) const
{
   Q_D(const QTextDocument);

   QTextDocument *doc = new QTextDocument(parent);
   QTextCursor(doc).insertFragment(QTextDocumentFragment(this));
   doc->rootFrame()->setFrameFormat(rootFrame()->frameFormat());
   QTextDocumentPrivate *priv = doc->d_func();
   priv->title = d->title;
   priv->url = d->url;
   priv->pageSize = d->pageSize;
   priv->indentWidth = d->indentWidth;
   priv->defaultTextOption = d->defaultTextOption;
   priv->setDefaultFont(d->defaultFont());
   priv->resources = d->resources;
   priv->cachedResources.clear();

#ifndef QT_NO_CSSPARSER
   priv->defaultStyleSheet = d->defaultStyleSheet;
   priv->parsedDefaultStyleSheet = d->parsedDefaultStyleSheet;
#endif

   return doc;
}

/*!
    Returns true if the document is empty; otherwise returns false.
*/
bool QTextDocument::isEmpty() const
{
   Q_D(const QTextDocument);
   /* because if we're empty we still have one single paragraph as
    * one single fragment */
   return d->length() <= 1;
}

/*!
  Clears the document.
*/
void QTextDocument::clear()
{
   Q_D(QTextDocument);
   d->clear();
   d->resources.clear();
}

void QTextDocument::undo(QTextCursor *cursor)
{
   Q_D(QTextDocument);
   const int pos = d->undoRedo(true);
   if (cursor && pos >= 0) {
      *cursor = QTextCursor(this);
      cursor->setPosition(pos);
   }
}

void QTextDocument::redo(QTextCursor *cursor)
{
   Q_D(QTextDocument);
   const int pos = d->undoRedo(false);
   if (cursor && pos >= 0) {
      *cursor = QTextCursor(this);
      cursor->setPosition(pos);
   }
}

void QTextDocument::clearUndoRedoStacks(Stacks stacksToClear)
{
   Q_D(QTextDocument);
   d->clearUndoRedoStacks(stacksToClear, true);
}


void QTextDocument::undo()
{
   Q_D(QTextDocument);
   d->undoRedo(true);
}


void QTextDocument::redo()
{
   Q_D(QTextDocument);
   d->undoRedo(false);
}


void QTextDocument::appendUndoItem(QAbstractUndoItem *item)
{
   Q_D(QTextDocument);
   d->appendUndoItem(item);
}

/*!
    \property QTextDocument::undoRedoEnabled
    \brief whether undo/redo are enabled for this document

    This defaults to true. If disabled, the undo stack is cleared and
    no items will be added to it.
*/
void QTextDocument::setUndoRedoEnabled(bool enable)
{
   Q_D(QTextDocument);
   d->enableUndoRedo(enable);
}

bool QTextDocument::isUndoRedoEnabled() const
{
   Q_D(const QTextDocument);
   return d->isUndoRedoEnabled();
}

int QTextDocument::maximumBlockCount() const
{
   Q_D(const QTextDocument);
   return d->maximumBlockCount;
}

void QTextDocument::setMaximumBlockCount(int maximum)
{
   Q_D(QTextDocument);
   d->maximumBlockCount = maximum;
   d->ensureMaximumBlockCount();
   setUndoRedoEnabled(false);
}

QTextOption QTextDocument::defaultTextOption() const
{
   Q_D(const QTextDocument);
   return d->defaultTextOption;
}


void QTextDocument::setDefaultTextOption(const QTextOption &option)
{
   Q_D(QTextDocument);
   d->defaultTextOption = option;

   if (d->lout) {
      d->lout->documentChanged(0, 0, d->length());
   }
}


QUrl QTextDocument::baseUrl() const
{
   Q_D(const QTextDocument);
   return d->baseUrl;
}

void QTextDocument::setBaseUrl(const QUrl &url)
{
   Q_D(QTextDocument);
   if (d->baseUrl != url) {
      d->baseUrl = url;
      if (d->lout) {
         d->lout->documentChanged(0, 0, d->length());
      }
      emit baseUrlChanged(url);
   }
}


Qt::CursorMoveStyle QTextDocument::defaultCursorMoveStyle() const
{
   Q_D(const QTextDocument);
   return d->defaultCursorMoveStyle;
}

void QTextDocument::setDefaultCursorMoveStyle(Qt::CursorMoveStyle style)
{
   Q_D(QTextDocument);
   d->defaultCursorMoveStyle = style;
}

void QTextDocument::markContentsDirty(int from, int length)
{
   Q_D(QTextDocument);
   d->documentChange(from, length);
   if (!d->inContentsChange) {
      if (d->lout) {
         d->lout->documentChanged(d->docChangeFrom, d->docChangeOldLength, d->docChangeLength);
         d->docChangeFrom = -1;
      }
   }
}

void QTextDocument::setUseDesignMetrics(bool b)
{
   Q_D(QTextDocument);
   if (b == d->defaultTextOption.useDesignMetrics()) {
      return;
   }

   d->defaultTextOption.setUseDesignMetrics(b);
   if (d->lout) {
      d->lout->documentChanged(0, 0, d->length());
   }
}

bool QTextDocument::useDesignMetrics() const
{
   Q_D(const QTextDocument);
   return d->defaultTextOption.useDesignMetrics();
}


void QTextDocument::drawContents(QPainter *p, const QRectF &rect)
{
   p->save();
   QAbstractTextDocumentLayout::PaintContext ctx;
   if (rect.isValid()) {
      p->setClipRect(rect);
      ctx.clip = rect;
   }
   documentLayout()->draw(p, ctx);
   p->restore();
}

void QTextDocument::setTextWidth(qreal width)
{
   Q_D(QTextDocument);
   QSizeF sz = d->pageSize;
   sz.setWidth(width);
   sz.setHeight(-1);
   setPageSize(sz);
}

qreal QTextDocument::textWidth() const
{
   Q_D(const QTextDocument);
   return d->pageSize.width();
}

qreal QTextDocument::idealWidth() const
{
   if (QTextDocumentLayout *lout = qobject_cast<QTextDocumentLayout *>(documentLayout())) {
      return lout->idealWidth();
   }
   return textWidth();
}

qreal QTextDocument::documentMargin() const
{
   Q_D(const QTextDocument);
   return d->documentMargin;
}

void QTextDocument::setDocumentMargin(qreal margin)
{
   Q_D(QTextDocument);
   if (d->documentMargin != margin) {
      d->documentMargin = margin;

      QTextFrame *root = rootFrame();
      QTextFrameFormat format = root->frameFormat();
      format.setMargin(margin);
      root->setFrameFormat(format);

      if (d->lout) {
         d->lout->documentChanged(0, 0, d->length());
      }
   }
}

qreal QTextDocument::indentWidth() const
{
   Q_D(const QTextDocument);
   return d->indentWidth;
}



void QTextDocument::setIndentWidth(qreal width)
{
   Q_D(QTextDocument);
   if (d->indentWidth != width) {
      d->indentWidth = width;
      if (d->lout) {
         d->lout->documentChanged(0, 0, d->length());
      }
   }
}

void QTextDocument::adjustSize()
{
   // Pull this private function in from qglobal.cpp
   QFont f = defaultFont();

   QFontMetrics fm(f);
   int mw =  fm.width('x') * 80;
   int w = mw;
   setTextWidth(w);

   QSizeF size = documentLayout()->documentSize();

   if (size.width() != 0) {
      w = qt_int_sqrt((uint)(5 * size.height() * size.width() / 3));
      setTextWidth(qMin(w, mw));

      size = documentLayout()->documentSize();
      if (w * 3 < 5 * size.height()) {
         w = qt_int_sqrt((uint)(2 * size.height() * size.width()));
         setTextWidth(qMin(w, mw));
      }
   }
   setTextWidth(idealWidth());
}

QSizeF QTextDocument::size() const
{
   return documentLayout()->documentSize();
}

int QTextDocument::blockCount() const
{
   Q_D(const QTextDocument);
   return d->blockMap().numNodes();
}

int QTextDocument::lineCount() const
{
   Q_D(const QTextDocument);
   return d->blockMap().length(2);
}

int QTextDocument::characterCount() const
{
   Q_D(const QTextDocument);
   return d->length();
}


QChar QTextDocument::characterAt(int pos) const
{
   Q_D(const QTextDocument);

   if (pos < 0 || pos >= d->length()) {
      return QChar();
   }

   QTextDocumentPrivate::FragmentIterator fragIt = d->find(pos);

   const QTextFragmentData *const frag = fragIt.value();
   const int offsetInFragment = qMax(0, pos - fragIt.position());

   return d->text.at(frag->stringPosition + offsetInFragment);
}

#ifndef QT_NO_CSSPARSER
void QTextDocument::setDefaultStyleSheet(const QString &sheet)
{
   Q_D(QTextDocument);
   d->defaultStyleSheet = sheet;
   QCss::Parser parser(sheet);
   d->parsedDefaultStyleSheet = QCss::StyleSheet();
   d->parsedDefaultStyleSheet.origin = QCss::StyleSheetOrigin_UserAgent;
   parser.parse(&d->parsedDefaultStyleSheet);
}

QString QTextDocument::defaultStyleSheet() const
{
   Q_D(const QTextDocument);
   return d->defaultStyleSheet;
}
#endif // QT_NO_CSSPARSER

bool QTextDocument::isUndoAvailable() const
{
   Q_D(const QTextDocument);
   return d->isUndoAvailable();
}


bool QTextDocument::isRedoAvailable() const
{
   Q_D(const QTextDocument);
   return d->isRedoAvailable();
}


int QTextDocument::availableUndoSteps() const
{
   Q_D(const QTextDocument);
   return d->availableUndoSteps();
}


int QTextDocument::availableRedoSteps() const
{
   Q_D(const QTextDocument);
   return d->availableRedoSteps();
}


int QTextDocument::revision() const
{
   Q_D(const QTextDocument);
   return d->revision;
}

void QTextDocument::setDocumentLayout(QAbstractTextDocumentLayout *layout)
{
   Q_D(QTextDocument);
   d->setLayout(layout);
}


QAbstractTextDocumentLayout *QTextDocument::documentLayout() const
{
   Q_D(const QTextDocument);
   if (!d->lout) {
      QTextDocument *that = const_cast<QTextDocument *>(this);
      that->d_func()->setLayout(new QTextDocumentLayout(that));
   }
   return d->lout;
}



QString QTextDocument::metaInformation(MetaInformation info) const
{
   Q_D(const QTextDocument);
   switch (info) {
      case DocumentTitle:
         return d->title;
      case DocumentUrl:
         return d->url;
   }
   return QString();
}

void QTextDocument::setMetaInformation(MetaInformation info, const QString &string)
{
   Q_D(QTextDocument);
   switch (info) {
      case DocumentTitle:
         d->title = string;
         break;
      case DocumentUrl:
         d->url = string;
         break;
   }
}

QString QTextDocument::toPlainText() const
{
   Q_D(const QTextDocument);

   QString retval;

   for (QChar c : d->plainText()) {

      switch (c.unicode()) {

         case 0xfdd0:    // QTextBeginningOfFrame
         case 0xfdd1:    // QTextEndOfFrame
         case QChar::ParagraphSeparator:
         case QChar::LineSeparator:
            retval.append('\n');
            break;

         case QChar::Nbsp:
            retval.append(' ');
            break;

         default:
            retval.append(c);
      }
   }

   return retval;
}

void QTextDocument::setPlainText(const QString &text)
{
   Q_D(QTextDocument);

   bool previousState = d->isUndoRedoEnabled();
   d->enableUndoRedo(false);
   d->beginEditBlock();
   d->clear();

   QTextCursor(this).insertText(text);
   d->endEditBlock();
   d->enableUndoRedo(previousState);
}


#ifndef QT_NO_TEXTHTMLPARSER

void QTextDocument::setHtml(const QString &html)
{
   Q_D(QTextDocument);

   bool previousState = d->isUndoRedoEnabled();
   d->enableUndoRedo(false);
   d->beginEditBlock();
   d->clear();
   QTextHtmlImporter(this, html, QTextHtmlImporter::ImportToDocument).import();
   d->endEditBlock();
   d->enableUndoRedo(previousState);
}

#endif

QTextCursor QTextDocument::find(const QString &subString, int from, FindFlags options) const
{
   QPatternOptionFlags flags;

   if (options & QTextDocument::FindCaseSensitively) {
      flags = QPatternOption::NoPatternOption;
   } else {
      flags = QPatternOption::CaseInsensitiveOption;
   }

   QRegularExpression expr(QRegularExpression::escape(subString), flags);

   return find(expr, from, options);
}

QTextCursor QTextDocument::find(const QString &subString, const QTextCursor &from, FindFlags options) const
{
   int pos = 0;

   if (! from.isNull()) {
      if (options & QTextDocument::FindBackward) {
         pos = from.selectionStart();
      } else {
         pos = from.selectionEnd();
      }
   }

   QPatternOptionFlags flags;

   if (options & QTextDocument::FindCaseSensitively) {
      flags = QPatternOption::NoPatternOption;
   } else {
      flags = QPatternOption::CaseInsensitiveOption;
   }

   QRegularExpression expr(QRegularExpression::escape(subString), flags);

   return find(expr, pos, options);
}

static bool findInBlock(const QTextBlock &block, const QRegularExpression &expression, int offset,
   QTextDocument::FindFlags options, QTextCursor &cursor)
{
   if (offset < 0) {
      return false;
   }

   QRegularExpression expr(expression);

   if (options & QTextDocument::FindWholeWords) {
      QString pattern = expr.pattern();

      pattern = "(?<!\\w)" + pattern + "(?!\\w)";
      expr.setPattern(pattern);
   }

   QString text = block.text();
   text.replace(QChar::Nbsp, ' ');

   bool goBackwards;

   if (options & QTextDocument::FindBackward) {
      goBackwards = true;
   } else {
      goBackwards = false;
   }

   if (offset <= text.length()) {

      QString::const_iterator iter = text.begin() + offset;
      QRegularExpressionMatch match;
      QRegularExpressionMatch match_hold;

      if (goBackwards) {
         // simulates an rmatch()
         QString::const_iterator iter_i = text.begin();

         while (true)  {
            match = expr.match(text, iter_i);

            if (match.hasMatch() && match.capturedEnd() <= iter) {

               // save the match in case we need it
               match_hold = match;

               // handles overlapping matches
               iter_i = match.capturedStart() + 1;

               if (iter_i > iter) {
                  break;
               }

            } else {
               break;
            }
         }

         match = match_hold;

      } else {
         match = expr.match(text, iter);

      }

      if (! match.hasMatch()) {
         return false;
      }

      // we have a hit, return the cursor for it
      cursor = QTextCursor(block.docHandle(), block.position() + (match.capturedStart() - text.begin()));
      cursor.setPosition(cursor.position() + match.capturedLength(0), QTextCursor::KeepAnchor);

      return true;
   }

   return false;
}

QTextCursor QTextDocument::find(const QRegularExpression &expr, int from, FindFlags options) const
{
   Q_D(const QTextDocument);

   if (expr.pattern().isEmpty()) {
      return QTextCursor();
   }

   int pos = from;

   // the cursor is positioned between characters, for a backward search do not
   // include the character given in the position

   if (options & FindBackward) {
      --pos ;

      if (pos < 0) {
         return QTextCursor();
      }
   }

   QTextCursor cursor;
   QTextBlock block = d->blocksFind(pos);

   if (! (options & FindBackward)) {
      int blockOffset = qMax(0, pos - block.position());

      while (block.isValid()) {
         if (findInBlock(block, expr, blockOffset, options, cursor)) {
            return cursor;
         }

         blockOffset = 0;
         block = block.next();
      }

   } else {
      int blockOffset = pos - block.position();

      while (block.isValid()) {
         if (findInBlock(block, expr, blockOffset, options, cursor)) {
            return cursor;
         }

         block = block.previous();
         blockOffset = block.length() - 1;
      }
   }

   return QTextCursor();
}

QTextCursor QTextDocument::find(const QRegularExpression &expr, const QTextCursor &from, FindFlags options) const
{
   int pos = 0;
   if (!from.isNull()) {
      if (options & QTextDocument::FindBackward) {
         pos = from.selectionStart();
      } else {
         pos = from.selectionEnd();
      }
   }
   return find(expr, pos, options);
}

QTextObject *QTextDocument::createObject(const QTextFormat &f)
{
   QTextObject *obj = nullptr;
   if (f.isListFormat()) {
      obj = new QTextList(this);
   } else if (f.isTableFormat()) {
      obj = new QTextTable(this);
   } else if (f.isFrameFormat()) {
      obj = new QTextFrame(this);
   }

   return obj;
}

/*!
    \internal

    Returns the frame that contains the text cursor position \a pos.
*/
QTextFrame *QTextDocument::frameAt(int pos) const
{
   Q_D(const QTextDocument);
   return d->frameAt(pos);
}

/*!
    Returns the document's root frame.
*/
QTextFrame *QTextDocument::rootFrame() const
{
   Q_D(const QTextDocument);
   return d->rootFrame();
}

/*!
    Returns the text object associated with the given \a objectIndex.
*/
QTextObject *QTextDocument::object(int objectIndex) const
{
   Q_D(const QTextDocument);
   return d->objectForIndex(objectIndex);
}

QTextObject *QTextDocument::objectForFormat(const QTextFormat &f) const
{
   Q_D(const QTextDocument);
   return d->objectForFormat(f);
}

QTextBlock QTextDocument::findBlock(int pos) const
{
   Q_D(const QTextDocument);
   return QTextBlock(docHandle(), d->blockMap().findNode(pos));
}


QTextBlock QTextDocument::findBlockByNumber(int blockNumber) const
{
   Q_D(const QTextDocument);
   return QTextBlock(docHandle(), d->blockMap().findNode(blockNumber, 1));
}

QTextBlock QTextDocument::findBlockByLineNumber(int lineNumber) const
{
   Q_D(const QTextDocument);
   return QTextBlock(docHandle(), d->blockMap().findNode(lineNumber, 2));
}

QTextBlock QTextDocument::begin() const
{
   Q_D(const QTextDocument);
   return QTextBlock(docHandle(), d->blockMap().begin().n);
}

QTextBlock QTextDocument::end() const
{
   return QTextBlock(docHandle(), 0);
}

QTextBlock QTextDocument::firstBlock() const
{
   Q_D(const QTextDocument);
   return QTextBlock(docHandle(), d->blockMap().begin().n);
}

QTextBlock QTextDocument::lastBlock() const
{
   Q_D(const QTextDocument);
   return QTextBlock(docHandle(), d->blockMap().last().n);
}

/*!
    \property QTextDocument::pageSize
    \brief the page size that should be used for laying out the document

    By default, for a newly-created, empty document, this property contains
    an undefined size.

    \sa modificationChanged()
*/

void QTextDocument::setPageSize(const QSizeF &size)
{
   Q_D(QTextDocument);
   d->pageSize = size;
   if (d->lout) {
      d->lout->documentChanged(0, 0, d->length());
   }
}

QSizeF QTextDocument::pageSize() const
{
   Q_D(const QTextDocument);
   return d->pageSize;
}

/*!
  returns the number of pages in this document.
*/
int QTextDocument::pageCount() const
{
   return documentLayout()->pageCount();
}

/*!
    Sets the default \a font to use in the document layout.
*/
void QTextDocument::setDefaultFont(const QFont &font)
{
   Q_D(QTextDocument);
   d->setDefaultFont(font);
   if (d->lout) {
      d->lout->documentChanged(0, 0, d->length());
   }
}

/*!
    Returns the default font to be used in the document layout.
*/
QFont QTextDocument::defaultFont() const
{
   Q_D(const QTextDocument);
   return d->defaultFont();
}

bool QTextDocument::isModified() const
{
   return docHandle()->isModified();
}

void QTextDocument::setModified(bool m)
{
   docHandle()->setModified(m);
}

#ifndef QT_NO_PRINTER
static void printPage(int index, QPainter *painter, const QTextDocument *doc, const QRectF &body,
   const QPointF &pageNumberPos)
{
   painter->save();
   painter->translate(body.left(), body.top() - (index - 1) * body.height());
   QRectF view(0, (index - 1) * body.height(), body.width(), body.height());

   QAbstractTextDocumentLayout *layout = doc->documentLayout();
   QAbstractTextDocumentLayout::PaintContext ctx;

   painter->setClipRect(view);
   ctx.clip = view;

   // don't use the system palette text as default text color, on HP/UX
   // for example that's white, and white text on white paper doesn't
   // look that nice
   ctx.palette.setColor(QPalette::Text, Qt::black);

   layout->draw(painter, ctx);

   if (!pageNumberPos.isNull()) {
      painter->setClipping(false);
      painter->setFont(QFont(doc->defaultFont()));
      const QString pageString = QString::number(index);

      painter->drawText(qRound(pageNumberPos.x() - painter->fontMetrics().width(pageString)),
         qRound(pageNumberPos.y() + view.top()),
         pageString);
   }

   painter->restore();
}


void QTextDocument::print(QPagedPaintDevice *printer) const
{
   Q_D(const QTextDocument);

   if (! printer) {
      return;
   }

   bool documentPaginated = d->pageSize.isValid() && !d->pageSize.isNull()
      && d->pageSize.height() != INT_MAX;

   // ### set page size to paginated size
   QMarginsF m = printer->margins();

   if (! documentPaginated && m.left() == 0.0 && m.right() == 0.0 && m.top() == 0.0 && m.bottom() == 0.0) {
      m.setLeft(2.0);
      m.setRight(2.0);
      m.setTop(2.0);
      m.setBottom(2.0);

      printer->setMargins(m);
   }

   QPainter p(printer);

   // Check if there is a valid device to print to
   if (! p.isActive()) {
      return;
   }

   const QTextDocument *doc = this;
   QScopedPointer<QTextDocument> clonedDoc;
   (void)doc->documentLayout(); // make sure that there is a layout

   QRectF body = QRectF(QPointF(0, 0), d->pageSize);
   QPointF pageNumberPos;

   if (documentPaginated) {
      qreal sourceDpiX = qt_defaultDpi();
      qreal sourceDpiY = sourceDpiX;

      QPaintDevice *dev = doc->documentLayout()->paintDevice();
      if (dev) {
         sourceDpiX = dev->logicalDpiX();
         sourceDpiY = dev->logicalDpiY();
      }

      const qreal dpiScaleX = qreal(printer->logicalDpiX()) / sourceDpiX;
      const qreal dpiScaleY = qreal(printer->logicalDpiY()) / sourceDpiY;

      // scale to dpi
      p.scale(dpiScaleX, dpiScaleY);

      QSizeF scaledPageSize = d->pageSize;
      scaledPageSize.rwidth() *= dpiScaleX;
      scaledPageSize.rheight() *= dpiScaleY;

      const QSizeF printerPageSize(printer->width(), printer->height());

      // scale to page
      p.scale(printerPageSize.width() / scaledPageSize.width(),
         printerPageSize.height() / scaledPageSize.height());

   } else {
      doc = clone(const_cast<QTextDocument *>(this));
      clonedDoc.reset(const_cast<QTextDocument *>(doc));

      for (QTextBlock srcBlock = firstBlock(), dstBlock = clonedDoc->firstBlock();
         srcBlock.isValid() && dstBlock.isValid();
         srcBlock = srcBlock.next(), dstBlock = dstBlock.next()) {
         dstBlock.layout()->setFormats(srcBlock.layout()->formats());
      }

      QAbstractTextDocumentLayout *layout = doc->documentLayout();
      layout->setPaintDevice(p.device());

      // copy the custom object handlers
      layout->d_func()->handlers = documentLayout()->d_func()->handlers;

      int dpiy = p.device()->logicalDpiY();

      int margin = (int) ((2 / 2.54) * dpiy); // 2 cm margins
      QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
      fmt.setMargin(margin);
      doc->rootFrame()->setFrameFormat(fmt);

      body = QRectF(0, 0, printer->width(), printer->height());

      pageNumberPos = QPointF(body.width() - margin, body.height() - margin
            + QFontMetrics(doc->defaultFont(), p.device()).ascent() + 5 * dpiy / 72.0);

      clonedDoc->setPageSize(body.size());
   }

   int fromPage   = printer->fromPage();
   int toPage     = printer->toPage();
   bool ascending = true;

   if (fromPage == 0 && toPage == 0) {
      fromPage = 1;
      toPage   = doc->pageCount();
   }

   // paranoia check
   fromPage = qMax(1, fromPage);
   toPage   = qMin(doc->pageCount(), toPage);

   if (toPage < fromPage) {
      // if the user entered a page range outside the actual number
      // of printable pages, just return
      return;
   }

   int page = fromPage;

   while (true) {
      printPage(page, &p, doc, body, pageNumberPos);

      if (page == toPage) {
         break;
      }

      if (ascending) {
         ++page;
      } else {
         --page;
      }

      if (! printer->newPage()) {
         return;
      }
   }
}
#endif

QVariant QTextDocument::resource(int type, const QUrl &name) const
{
   Q_D(const QTextDocument);

   const QUrl url = d->baseUrl.resolved(name);
   QVariant r = d->resources.value(url);

   if (!r.isValid()) {
      r = d->cachedResources.value(url);

      if (!r.isValid())  {
         r = const_cast<QTextDocument *>(this)->loadResource(type, url);
      }
   }
   return r;
}

void QTextDocument::addResource(int type, const QUrl &name, const QVariant &resource)
{
   (void) type;

   Q_D(QTextDocument);
   d->resources.insert(name, resource);
}

QVariant QTextDocument::loadResource(int type, const QUrl &name)
{
   Q_D(QTextDocument);
   QVariant r;

   QObject *p = parent();

   if (p) {
      const QMetaObject *me = p->metaObject();
      int index = me->indexOfMethod("loadResource(int, const QUrl &)");

      if (index >= 0) {
         QMetaMethod loader = me->method(index);
         loader.invoke(p, Q_RETURN_ARG(QVariant, r), Q_ARG(int, type), Q_ARG(const QUrl &, name));
      }
   }

   // handle data: URLs
   if (! r.isValid() && name.scheme().compare("data", Qt::CaseInsensitive) == 0) {
      r = qDecodeDataUrl(name).second;
   }

   // if resource was not loaded try to load it here
   if (! qobject_cast<QTextDocument *>(p) && ! r.isValid()) {
      QUrl resourceUrl = name;

      if (name.isRelative()) {
         QUrl currentURL = QUrl(d->url);

         // For the second case QUrl can merge "#someanchor" with "foo.html"
         // correctly to "foo.html#someanchor"
         if (! (currentURL.isRelative() || (currentURL.scheme() == "file" && ! QFileInfo(currentURL.toLocalFile()).isAbsolute()))
            || (name.hasFragment() && name.path().isEmpty())) {
            resourceUrl =  currentURL.resolved(name);

         } else {
            // this is our last resort when current url and new url are both relative
            // we try to resolve against the current working directory in the local  file system.
            QFileInfo fi(currentURL.toLocalFile());

            if (fi.exists()) {
               resourceUrl = QUrl::fromLocalFile(fi.absolutePath() + QDir::separator()).resolved(name);

            } else if (currentURL.isEmpty()) {
               resourceUrl.setScheme("file");
            }
         }
      }

      QString s = resourceUrl.toLocalFile();
      QFile f(s);
      if (!s.isEmpty() && f.open(QFile::ReadOnly)) {
         r = f.readAll();
         f.close();
      }
   }

   if (r.isValid()) {
      if (type == ImageResource && r.type() == QVariant::ByteArray) {

         if (qApp->thread() != QThread::currentThread()) {
            // must use images in non-GUI threads
            QImage image;
            image.loadFromData(r.toByteArray());

            if (!image.isNull()) {
               r = image;
            }

         } else {
            QPixmap pm;
            pm.loadFromData(r.toByteArray());
            if (!pm.isNull()) {
               r = pm;
            }
         }
      }

      d->cachedResources.insert(name, r);
   }

   return r;
}

static QTextFormat formatDifference(const QTextFormat &from, const QTextFormat &to)
{
   QTextFormat diff = to;

   const QMap<int, QVariant> props = to.properties();
   for (auto it = props.begin(), end = props.end(); it != end; ++it) {
      if (it.value() == from.property(it.key())) {
         diff.clearProperty(it.key());
      }

   }

   return diff;
}


static QString colorValue(QColor color)
{
   QString result;

   if (color.alpha() == 255) {
      result = color.name();

   } else if (color.alpha()) {

      QString alphaValue = QString::number(color.alphaF(), 'f', 6).remove(QRegularExpression("\\.?0*$"));

      result = QString("rgba(%1,%2,%3,%4)").formatArg(color.red())
         .formatArg(color.green()).formatArg(color.blue()).formatArg(alphaValue);
   } else {
      result = "transparent";
   }

   return result;
}


QTextHtmlExporter::QTextHtmlExporter(const QTextDocument *_doc)
   : doc(_doc), fragmentMarkers(false)
{
   const QFont defaultFont = doc->defaultFont();
   defaultCharFormat.setFont(defaultFont);

   // don't export those for the default font since we cannot turn them off with CSS
   defaultCharFormat.clearProperty(QTextFormat::FontUnderline);
   defaultCharFormat.clearProperty(QTextFormat::FontOverline);
   defaultCharFormat.clearProperty(QTextFormat::FontStrikeOut);
   defaultCharFormat.clearProperty(QTextFormat::TextUnderlineStyle);
}

QString QTextHtmlExporter::toHtml(const QString &encoding, ExportMode mode)
{
   html = QString::fromLatin1("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
         "\"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
         "<html><head><meta name=\"qrichtext\" content=\"1\" />");

   fragmentMarkers = (mode == ExportFragment);

   if (! encoding.isEmpty()) {
      html += QString::fromLatin1("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%1\" />")
         .formatArg(encoding);
   }

   QString title  = doc->metaInformation(QTextDocument::DocumentTitle);
   if (! title.isEmpty()) {
      html += "<title>" + title + "</title>";
   }

   html += "<style type=\"text/css\">\n";
   html += "p, li { white-space: pre-wrap; }\n";
   html += "</style>";
   html += "</head><body";

   if (mode == ExportEntireDocument) {
      html += " style=\"";

      emitFontFamily(defaultCharFormat.fontFamily());

      if (defaultCharFormat.hasProperty(QTextFormat::FontPointSize)) {
         html += " font-size:";
         html += QString::number(defaultCharFormat.fontPointSize());
         html += "pt;";

      } else if (defaultCharFormat.hasProperty(QTextFormat::FontPixelSize)) {
         html += " font-size:";
         html += QString::number(defaultCharFormat.intProperty(QTextFormat::FontPixelSize));
         html += "px;";
      }

      html += " font-weight:";
      html += QString::number(defaultCharFormat.fontWeight() * 8);
      html += ';';

      html += " font-style:";
      html += (defaultCharFormat.fontItalic() ? QLatin1String("italic") : QLatin1String("normal"));
      html += ';';

      // do not set text-decoration on the default font since those values are /always/ propagated
      // and cannot be turned off with CSS

      html += '\"';

      const QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
      emitBackgroundAttribute(fmt);

   } else {
      defaultCharFormat = QTextCharFormat();
   }

   html += '>';

   QTextFrameFormat rootFmt = doc->rootFrame()->frameFormat();
   rootFmt.clearProperty(QTextFormat::BackgroundBrush);

   QTextFrameFormat defaultFmt;
   defaultFmt.setMargin(doc->documentMargin());

   if (rootFmt == defaultFmt) {
      emitFrame(doc->rootFrame()->begin());
   } else {
      emitTextFrame(doc->rootFrame());
   }

   html += "</body></html>";
   return html;
}

void QTextHtmlExporter::emitAttribute(const QString &attribute, const QString &value)
{
   html += ' ';
   html += attribute;
   html += "=\"";
   html += value.toHtmlEscaped();
   html += '"';
}

bool QTextHtmlExporter::emitCharFormatStyle(const QTextCharFormat &format)
{
   bool attributesEmitted = false;

   {
      const QString family = format.fontFamily();

      if (! family.isEmpty() && family != defaultCharFormat.fontFamily()) {
         emitFontFamily(family);
         attributesEmitted = true;
      }
   }

   if (format.hasProperty(QTextFormat::FontPointSize) && format.fontPointSize() != defaultCharFormat.fontPointSize()) {
      html += " font-size:";
      html += QString::number(format.fontPointSize());
      html += "pt;";
      attributesEmitted = true;

   } else if (format.hasProperty(QTextFormat::FontSizeAdjustment)) {
      static const char *const sizeNames[] = {
         "small", "medium", "large", "x-large", "xx-large"
      };

      const char *name = nullptr;
      const int idx = format.intProperty(QTextFormat::FontSizeAdjustment) + 1;

      if (idx >= 0 && idx <= 4) {
         name = sizeNames[idx];
      }

      if (name) {
         html += " font-size:";
         html += QString::fromLatin1(name);
         html += ';';
         attributesEmitted = true;
      }

   } else if (format.hasProperty(QTextFormat::FontPixelSize)) {
      html += " font-size:";
      html += QString::number(format.intProperty(QTextFormat::FontPixelSize));
      html += "px;";
      attributesEmitted = true;
   }

   if (format.hasProperty(QTextFormat::FontWeight) && format.fontWeight() != defaultCharFormat.fontWeight()) {
      html += " font-weight:";
      html += QString::number(format.fontWeight() * 8);
      html += ';';
      attributesEmitted = true;
   }

   if (format.hasProperty(QTextFormat::FontItalic) && format.fontItalic() != defaultCharFormat.fontItalic()) {
      html += " font-style:";
      html += (format.fontItalic() ? QString("italic") : QString("normal"));
      html += ';';
      attributesEmitted = true;
   }

   QString decorationTag(" text-decoration:");
   html += decorationTag;
   bool hasDecoration = false;
   bool atLeastOneDecorationSet = false;

   if ((format.hasProperty(QTextFormat::FontUnderline) || format.hasProperty(QTextFormat::TextUnderlineStyle))
      && format.fontUnderline() != defaultCharFormat.fontUnderline()) {

      hasDecoration = true;

      if (format.fontUnderline()) {
         html += " underline";
         atLeastOneDecorationSet = true;
      }
   }

   if (format.hasProperty(QTextFormat::FontOverline) && format.fontOverline() != defaultCharFormat.fontOverline()) {
      hasDecoration = true;

      if (format.fontOverline()) {
         html += " overline";
         atLeastOneDecorationSet = true;
      }
   }

   if (format.hasProperty(QTextFormat::FontStrikeOut) && format.fontStrikeOut() != defaultCharFormat.fontStrikeOut()) {
      hasDecoration = true;

      if (format.fontStrikeOut()) {
         html += " line-through";
         atLeastOneDecorationSet = true;
      }
   }

   if (hasDecoration) {
      if (! atLeastOneDecorationSet) {
         html += "none";
      }

      html += ';';
      attributesEmitted = true;

   } else {
      html.chop(decorationTag.length());
   }

   if (format.foreground() != defaultCharFormat.foreground() && format.foreground().style() != Qt::NoBrush) {
      html += " color:";
      html += colorValue(format.foreground().color());
      html += ';';
      attributesEmitted = true;
   }

   if (format.background() != defaultCharFormat.background() && format.background().style() == Qt::SolidPattern) {
      html += " background-color:";
      html += colorValue(format.background().color());
      html += ';';
      attributesEmitted = true;
   }

   if (format.verticalAlignment() != defaultCharFormat.verticalAlignment() &&
      format.verticalAlignment() != QTextCharFormat::AlignNormal) {
      html += " vertical-align:";

      QTextCharFormat::VerticalAlignment valign = format.verticalAlignment();

      if (valign == QTextCharFormat::AlignSubScript) {
         html += "sub";

      } else if (valign == QTextCharFormat::AlignSuperScript) {
         html += "super";

      } else if (valign == QTextCharFormat::AlignMiddle) {
         html += "middle";

      } else if (valign == QTextCharFormat::AlignTop) {
         html += "top";

      } else if (valign == QTextCharFormat::AlignBottom) {
         html += "bottom";
      }

      html += ';';
      attributesEmitted = true;
   }

   if (format.fontCapitalization() != QFont::MixedCase) {
      const QFont::Capitalization caps = format.fontCapitalization();
      if (caps == QFont::AllUppercase) {
         html += " text-transform:uppercase;";

      } else if (caps == QFont::AllLowercase) {
         html += " text-transform:lowercase;";

      } else if (caps == QFont::SmallCaps) {
         html += " font-variant:small-caps;";
      }

      attributesEmitted = true;
   }

   if (format.fontWordSpacing() != 0.0) {
      html += " word-spacing:";
      html += QString::number(format.fontWordSpacing());
      html += "px;";
      attributesEmitted = true;
   }

   return attributesEmitted;
}

void QTextHtmlExporter::emitTextLength(const QString &attribute, const QTextLength &length)
{
   if (length.type() == QTextLength::VariableLength) {
      // default
      return;
   }

   html += ' ';
   html += attribute;
   html += "=\"";
   html += QString::number(length.rawValue());

   if (length.type() == QTextLength::PercentageLength) {
      html += "%\"";
   } else {
      html += '\"';
   }
}

void QTextHtmlExporter::emitAlignment(Qt::Alignment align)
{
   if (align & Qt::AlignLeft) {
      return;

   } else if (align & Qt::AlignRight) {
      html += " align=\"right\"";

   } else if (align & Qt::AlignHCenter) {
      html += " align=\"center\"";

   } else if (align & Qt::AlignJustify) {
      html += " align=\"justify\"";
   }
}

void QTextHtmlExporter::emitFloatStyle(QTextFrameFormat::Position pos, StyleMode mode)
{
   if (pos == QTextFrameFormat::InFlow) {
      return;
   }

   if (mode == EmitStyleTag) {
      html += " style=\"float:";
   } else {
      html += " float:";
   }

   if (pos == QTextFrameFormat::FloatLeft) {
      html += " left;";

   } else if (pos == QTextFrameFormat::FloatRight) {
      html += " right;";

   } else {
      Q_ASSERT_X(0, "QTextHtmlExporter::emitFloatStyle()", "pos should be a valid enum type");
   }

   if (mode == EmitStyleTag) {
      html += QLatin1Char('\"');
   }
}

void QTextHtmlExporter::emitBorderStyle(QTextFrameFormat::BorderStyle style)
{
   Q_ASSERT(style <= QTextFrameFormat::BorderStyle_Outset);

   html += " border-style:";

   switch (style) {
      case QTextFrameFormat::BorderStyle_None:
         html += "none";
         break;

      case QTextFrameFormat::BorderStyle_Dotted:
         html += "dotted";
         break;

      case QTextFrameFormat::BorderStyle_Dashed:
         html += "dashed";
         break;

      case QTextFrameFormat::BorderStyle_Solid:
         html += "solid";
         break;

      case QTextFrameFormat::BorderStyle_Double:
         html += "double";
         break;

      case QTextFrameFormat::BorderStyle_DotDash:
         html += "dot-dash";
         break;

      case QTextFrameFormat::BorderStyle_DotDotDash:
         html += "dot-dot-dash";
         break;

      case QTextFrameFormat::BorderStyle_Groove:
         html += "groove";
         break;

      case QTextFrameFormat::BorderStyle_Ridge:
         html += "ridge";
         break;

      case QTextFrameFormat::BorderStyle_Inset:
         html += "inset";
         break;

      case QTextFrameFormat::BorderStyle_Outset:
         html += "outset";
         break;

      default:
         Q_ASSERT(false);
         break;
   };

   html += ';';
}

void QTextHtmlExporter::emitPageBreakPolicy(QTextFormat::PageBreakFlags policy)
{
   if (policy & QTextFormat::PageBreak_AlwaysBefore) {
      html += " page-break-before:always;";
   }

   if (policy & QTextFormat::PageBreak_AlwaysAfter) {
      html += " page-break-after:always;";
   }
}

void QTextHtmlExporter::emitFontFamily(const QString &family)
{
   html += " font-family:";

   QString quote("\'");

   if (family.contains('\'')) {
      quote = "&quot;";
   }

   html += quote;
   html += family.toHtmlEscaped();
   html += quote;
   html += ';';
}

void QTextHtmlExporter::emitMargins(const QString &top, const QString &bottom, const QString &left,
   const QString &right)
{
   html += " margin-top:";
   html += top;
   html += "px;";

   html += " margin-bottom:";
   html += bottom;
   html += "px;";

   html += " margin-left:";
   html += left;
   html += "px;";

   html += " margin-right:";
   html += right;
   html += "px;";
}

void QTextHtmlExporter::emitFragment(const QTextFragment &fragment)
{
   const QTextCharFormat format = fragment.charFormat();

   bool closeAnchor = false;

   if (format.isAnchor()) {
      const QString name = format.anchorName();
      if (! name.isEmpty()) {
         html += "<a name=\"";
         html += name.toHtmlEscaped();
         html += "\"></a>";
      }

      const QString href = format.anchorHref();
      if (!href.isEmpty()) {
         html += "<a href=\"";
         html += href.toHtmlEscaped();
         html += "\">";
         closeAnchor = true;
      }
   }

   QString txt = fragment.text();

   const bool isObject = txt.contains(QChar(QChar::ObjectReplacementCharacter));
   const bool isImage  = isObject && format.isImageFormat();

   QString styleTag("<span style=\"");
   html += styleTag;

   bool attributesEmitted = false;
   if (! isImage) {
      attributesEmitted = emitCharFormatStyle(format);
   }

   if (attributesEmitted) {
      html += "\">";
   } else {
      html.chop(styleTag.length());
   }

   if (isObject) {
      for (int i = 0; isImage && i < txt.length(); ++i) {
         QTextImageFormat imgFmt = format.toImageFormat();

         html += "<img";

         if (imgFmt.hasProperty(QTextFormat::ImageName)) {
            emitAttribute("src", imgFmt.name());
         }

         if (imgFmt.hasProperty(QTextFormat::ImageWidth)) {
            emitAttribute("width", QString::number(imgFmt.width()));
         }

         if (imgFmt.hasProperty(QTextFormat::ImageHeight)) {
            emitAttribute("height", QString::number(imgFmt.height()));
         }

         if (imgFmt.verticalAlignment() == QTextCharFormat::AlignMiddle) {
            html += " style=\"vertical-align: middle;\"";

         } else if (imgFmt.verticalAlignment() == QTextCharFormat::AlignTop) {
            html += " style=\"vertical-align: top;\"";
         }

         if (QTextFrame *imageFrame = qobject_cast<QTextFrame *>(doc->objectForFormat(imgFmt))) {
            emitFloatStyle(imageFrame->frameFormat().position());
         }

         html += " />";
      }

   } else {
      Q_ASSERT(! txt.contains(QChar(QChar::ObjectReplacementCharacter)));

      txt = txt.toHtmlEscaped();

      // split for [\n{LineSeparator}]
      QString forcedLineBreakRegExp = "[\\n" + QString(QChar::LineSeparator) + "]";

      const QStringList lines = txt.split(QRegularExpression(forcedLineBreakRegExp));

      for (int i = 0; i < lines.count(); ++i) {
         if (i > 0) {
            html += "<br />";         // space on purpose for compatibility with Netscape, Lynx & Co.
         }
         html += lines.at(i);
      }
   }

   if (attributesEmitted) {
      html += "</span>";
   }

   if (closeAnchor) {
      html += "</a>";
   }
}

static bool isOrderedList(int style)
{
   return style == QTextListFormat::ListDecimal || style == QTextListFormat::ListLowerAlpha
      || style == QTextListFormat::ListUpperAlpha
      || style == QTextListFormat::ListUpperRoman
      || style == QTextListFormat::ListLowerRoman
      ;
}

void QTextHtmlExporter::emitBlockAttributes(const QTextBlock &block)
{
   QTextBlockFormat format = block.blockFormat();
   emitAlignment(format.alignment());

   // assume default to not bloat the html too much
   // html += " dir='ltr'";

   if (block.textDirection() == Qt::RightToLeft) {
      html += " dir='rtl'";
   }

   QLatin1String style(" style=\"");
   html += style;

   const bool emptyBlock = block.begin().atEnd();
   if (emptyBlock) {
      html += "-qt-paragraph-type:empty;";
   }

   emitMargins(QString::number(format.topMargin()),
      QString::number(format.bottomMargin()),
      QString::number(format.leftMargin()),
      QString::number(format.rightMargin()));

   html += " -qt-block-indent:";
   html += QString::number(format.indent());
   html += ';';

   html += " text-indent:";
   html += QString::number(format.textIndent());
   html += "px;";

   if (block.userState() != -1) {
      html += " -qt-user-state:";
      html += QString::number(block.userState());
      html += ';';
   }

   if (format.lineHeightType() != QTextBlockFormat::SingleHeight) {
      switch (format.lineHeightType()) {
         case QTextBlockFormat::ProportionalHeight:
         case QTextBlockFormat::FixedHeight:
            html += " line-height:";
            break;

         case QTextBlockFormat::MinimumHeight:
            html += " min-height:";
            break;

         case QTextBlockFormat::LineDistanceHeight:
            html += " line-spacing:";
            break;

         case QTextBlockFormat::SingleHeight:
         default:
            break; // Should never reach here
      }

      html += QString::number(format.lineHeight());
      if (format.lineHeightType() == QTextBlockFormat::ProportionalHeight) {
         html += "%;";
      } else {
         html += "px;";
      }
   }
   emitPageBreakPolicy(format.pageBreakPolicy());

   QTextCharFormat diff;
   if (emptyBlock) { // only print character properties when we don't expect them to be repeated by actual text in the parag
      const QTextCharFormat blockCharFmt = block.charFormat();
      diff = formatDifference(defaultCharFormat, blockCharFmt).toCharFormat();
   }

   diff.clearProperty(QTextFormat::BackgroundBrush);
   if (format.hasProperty(QTextFormat::BackgroundBrush)) {
      QBrush bg = format.background();
      if (bg.style() != Qt::NoBrush) {
         diff.setProperty(QTextFormat::BackgroundBrush, format.property(QTextFormat::BackgroundBrush));
      }
   }

   if (!diff.properties().isEmpty()) {
      emitCharFormatStyle(diff);
   }

   html += QLatin1Char('"');

}

void QTextHtmlExporter::emitBlock(const QTextBlock &block)
{
   if (block.begin().atEnd()) {
      // ### HACK, remove once QTextFrame::iterator is fixed
      int p = block.position();
      if (p > 0) {
         --p;
      }
      QTextDocumentPrivate::FragmentIterator frag = doc->docHandle()->find(p);
      QChar ch = doc->docHandle()->buffer().at(frag->stringPosition);
      if (ch == QTextBeginningOfFrame
         || ch == QTextEndOfFrame) {
         return;
      }
   }

   html += QLatin1Char('\n');

   // save and later restore, in case we 'change' the default format by
   // emitting block char format information
   QTextCharFormat oldDefaultCharFormat = defaultCharFormat;

   QTextList *list = block.textList();
   if (list) {
      if (list->itemNumber(block) == 0) { // first item? emit <ul> or appropriate
         const QTextListFormat format = list->format();
         const int style = format.style();
         switch (style) {
            case QTextListFormat::ListDecimal:
               html += "<ol";
               break;

            case QTextListFormat::ListDisc:
               html += "<ul";
               break;

            case QTextListFormat::ListCircle:
               html += "<ul type=\"circle\"";
               break;

            case QTextListFormat::ListSquare:
               html += "<ul type=\"square\"";
               break;

            case QTextListFormat::ListLowerAlpha:
               html += "<ol type=\"a\"";
               break;

            case QTextListFormat::ListUpperAlpha:
               html += "<ol type=\"A\"";
               break;

            case QTextListFormat::ListLowerRoman:
               html += "<ol type=\"i\"";
               break;

            case QTextListFormat::ListUpperRoman:
               html += "<ol type=\"I\"";
               break;

            default:
               html += "<ul";       // should not happen
         }

         QString styleString = QString::fromLatin1("margin-top: 0px; margin-bottom: 0px; margin-left: 0px; margin-right: 0px;");

         if (format.hasProperty(QTextFormat::ListIndent)) {
            styleString += " -qt-list-indent: ";
            styleString += QString::number(format.indent());
            styleString += ';';
         }

         if (format.hasProperty(QTextFormat::ListNumberPrefix)) {
            QString numberPrefix = format.numberPrefix();
            numberPrefix.replace('"', "\\22");

            // FIXME: problem in the CSS parser the prevents this from being correctly restored
            numberPrefix.replace('\'', "\\27");

            styleString += " -qt-list-number-prefix: ";
            styleString += '\'';
            styleString += numberPrefix;
            styleString += '\'';
            styleString += ';';
         }

         if (format.hasProperty(QTextFormat::ListNumberSuffix)) {
            if (format.numberSuffix() != ".") {
               // this is our default

               QString numberSuffix = format.numberSuffix();
               numberSuffix.replace('"',  "\\22");
               numberSuffix.replace('\'', "\\27");    // see above

               styleString += " -qt-list-number-suffix: ";
               styleString += '\'';
               styleString += numberSuffix;
               styleString += '\'';
               styleString += ';';
            }
         }

         html += " style=\"";
         html += styleString;
         html += "\">";
      }

      html += "<li";

      const QTextCharFormat blockFmt = formatDifference(defaultCharFormat, block.charFormat()).toCharFormat();
      if (!blockFmt.properties().isEmpty()) {
         html += QLatin1String(" style=\"");
         emitCharFormatStyle(blockFmt);
         html += QLatin1Char('\"');

         defaultCharFormat.merge(block.charFormat());
      }
   }

   const QTextBlockFormat blockFormat = block.blockFormat();
   if (blockFormat.hasProperty(QTextFormat::BlockTrailingHorizontalRulerWidth)) {
      html += QLatin1String("<hr");

      QTextLength width = blockFormat.lengthProperty(QTextFormat::BlockTrailingHorizontalRulerWidth);
      if (width.type() != QTextLength::VariableLength) {
         emitTextLength("width", width);
      } else {
         html += QLatin1Char(' ');
      }

      html += QLatin1String("/>");
      return;
   }

   const bool pre = blockFormat.nonBreakableLines();
   if (pre) {
      if (list) {
         html += QLatin1Char('>');
      }
      html += QLatin1String("<pre");
   } else if (!list) {
      html += QLatin1String("<p");
   }

   emitBlockAttributes(block);

   html += QLatin1Char('>');
   if (block.begin().atEnd()) {
      html += QLatin1String("<br />");
   }

   QTextBlock::iterator it = block.begin();
   if (fragmentMarkers && !it.atEnd() && block == doc->begin()) {
      html += QLatin1String("<!--StartFragment-->");
   }

   for (; !it.atEnd(); ++it) {
      emitFragment(it.fragment());
   }

   if (fragmentMarkers && block.position() + block.length() == doc->docHandle()->length()) {
      html += QLatin1String("<!--EndFragment-->");
   }

   if (pre) {
      html += QLatin1String("</pre>");
   } else if (list) {
      html += QLatin1String("</li>");
   } else {
      html += QLatin1String("</p>");
   }

   if (list) {
      if (list->itemNumber(block) == list->count() - 1) { // last item? close list
         if (isOrderedList(list->format().style())) {
            html += QLatin1String("</ol>");
         } else {
            html += QLatin1String("</ul>");
         }
      }
   }

   defaultCharFormat = oldDefaultCharFormat;
}

extern bool qHasPixmapTexture(const QBrush &brush);

QString QTextHtmlExporter::findUrlForImage(const QTextDocument *doc, qint64 cacheKey, bool isPixmap)
{
   QString url;
   if (! doc) {
      return url;
   }

   if (QTextDocument *parent = qobject_cast<QTextDocument *>(doc->parent())) {
      return findUrlForImage(parent, cacheKey, isPixmap);
   }

   if (doc && doc->docHandle()) {
      QTextDocumentPrivate *priv = doc->docHandle();
      QMap<QUrl, QVariant>::const_iterator it = priv->cachedResources.constBegin();

      for (; it != priv->cachedResources.constEnd(); ++it) {
         const QVariant &v = it.value();

         if (v.type() == QVariant::Image && ! isPixmap) {
            if (v.value<QImage>().cacheKey() == cacheKey) {
               break;
            }
         }

         if (v.type() == QVariant::Pixmap && isPixmap) {
            if (v.value<QPixmap>().cacheKey() == cacheKey) {
               break;
            }
         }
      }

      if (it != priv->cachedResources.constEnd()) {
         url = it.key().toString();
      }
   }

   return url;
}

void QTextDocumentPrivate::mergeCachedResources(const QTextDocumentPrivate *priv)
{
   if (!priv) {
      return;
   }

   cachedResources.unite(priv->cachedResources);
}

void QTextHtmlExporter::emitBackgroundAttribute(const QTextFormat &format)
{
   if (format.hasProperty(QTextFormat::BackgroundImageUrl)) {
      QString url = format.property(QTextFormat::BackgroundImageUrl).toString();
      emitAttribute("background", url);
   } else {
      const QBrush &brush = format.background();
      if (brush.style() == Qt::SolidPattern) {
         emitAttribute("bgcolor", colorValue(brush.color()));
      } else if (brush.style() == Qt::TexturePattern) {
         const bool isPixmap = qHasPixmapTexture(brush);
         const qint64 cacheKey = isPixmap ? brush.texture().cacheKey() : brush.textureImage().cacheKey();

         const QString url = findUrlForImage(doc, cacheKey, isPixmap);

         if (!url.isEmpty()) {
            emitAttribute("background", url);
         }
      }
   }
}

void QTextHtmlExporter::emitTable(const QTextTable *table)
{
   QTextTableFormat format = table->format();

   html += QLatin1String("\n<table");

   if (format.hasProperty(QTextFormat::FrameBorder)) {
      emitAttribute("border", QString::number(format.border()));
   }

   emitFrameStyle(format, TableFrame);

   emitAlignment(format.alignment());
   emitTextLength("width", format.width());

   if (format.hasProperty(QTextFormat::TableCellSpacing)) {
      emitAttribute("cellspacing", QString::number(format.cellSpacing()));
   }
   if (format.hasProperty(QTextFormat::TableCellPadding)) {
      emitAttribute("cellpadding", QString::number(format.cellPadding()));
   }

   emitBackgroundAttribute(format);

   html += QLatin1Char('>');

   const int rows = table->rows();
   const int columns = table->columns();

   QVector<QTextLength> columnWidths = format.columnWidthConstraints();
   if (columnWidths.isEmpty()) {
      columnWidths.resize(columns);
      columnWidths.fill(QTextLength());
   }
   Q_ASSERT(columnWidths.count() == columns);

   QVarLengthArray<bool> widthEmittedForColumn(columns);
   for (int i = 0; i < columns; ++i) {
      widthEmittedForColumn[i] = false;
   }

   const int headerRowCount = qMin(format.headerRowCount(), rows);
   if (headerRowCount > 0) {
      html += QLatin1String("<thead>");
   }

   for (int row = 0; row < rows; ++row) {
      html += QLatin1String("\n<tr>");

      for (int col = 0; col < columns; ++col) {
         const QTextTableCell cell = table->cellAt(row, col);

         // for col/rowspans
         if (cell.row() != row) {
            continue;
         }

         if (cell.column() != col) {
            continue;
         }

         html += QLatin1String("\n<td");

         if (!widthEmittedForColumn[col] && cell.columnSpan() == 1) {
            emitTextLength("width", columnWidths.at(col));
            widthEmittedForColumn[col] = true;
         }

         if (cell.columnSpan() > 1) {
            emitAttribute("colspan", QString::number(cell.columnSpan()));
         }

         if (cell.rowSpan() > 1) {
            emitAttribute("rowspan", QString::number(cell.rowSpan()));
         }

         const QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
         emitBackgroundAttribute(cellFormat);

         QTextCharFormat oldDefaultCharFormat = defaultCharFormat;

         QTextCharFormat::VerticalAlignment valign = cellFormat.verticalAlignment();

         QString styleString;
         if (valign >= QTextCharFormat::AlignMiddle && valign <= QTextCharFormat::AlignBottom) {
            styleString += QLatin1String(" vertical-align:");
            switch (valign) {
               case QTextCharFormat::AlignMiddle:
                  styleString += QLatin1String("middle");
                  break;
               case QTextCharFormat::AlignTop:
                  styleString += QLatin1String("top");
                  break;
               case QTextCharFormat::AlignBottom:
                  styleString += QLatin1String("bottom");
                  break;
               default:
                  break;
            }
            styleString += QLatin1Char(';');

            QTextCharFormat temp;
            temp.setVerticalAlignment(valign);
            defaultCharFormat.merge(temp);
         }

         if (cellFormat.hasProperty(QTextFormat::TableCellLeftPadding)) {
            styleString += QLatin1String(" padding-left:") + QString::number(cellFormat.leftPadding()) + QLatin1Char(';');
         }
         if (cellFormat.hasProperty(QTextFormat::TableCellRightPadding)) {
            styleString += QLatin1String(" padding-right:") + QString::number(cellFormat.rightPadding()) + QLatin1Char(';');
         }
         if (cellFormat.hasProperty(QTextFormat::TableCellTopPadding)) {
            styleString += QLatin1String(" padding-top:") + QString::number(cellFormat.topPadding()) + QLatin1Char(';');
         }
         if (cellFormat.hasProperty(QTextFormat::TableCellBottomPadding)) {
            styleString += QLatin1String(" padding-bottom:") + QString::number(cellFormat.bottomPadding()) + QLatin1Char(';');
         }

         if (!styleString.isEmpty()) {
            html += QLatin1String(" style=\"") + styleString + QLatin1Char('\"');
         }

         html += QLatin1Char('>');

         emitFrame(cell.begin());

         html += QLatin1String("</td>");

         defaultCharFormat = oldDefaultCharFormat;
      }

      html += QLatin1String("</tr>");
      if (headerRowCount > 0 && row == headerRowCount - 1) {
         html += QLatin1String("</thead>");
      }
   }

   html += QLatin1String("</table>");
}

void QTextHtmlExporter::emitFrame(QTextFrame::iterator frameIt)
{
   if (!frameIt.atEnd()) {
      QTextFrame::iterator next = frameIt;
      ++next;
      if (next.atEnd()
         && frameIt.currentFrame() == nullptr
         && frameIt.parentFrame() != doc->rootFrame()
         && frameIt.currentBlock().begin().atEnd()) {
         return;
      }
   }

   for (QTextFrame::iterator it = frameIt;
      !it.atEnd(); ++it) {
      if (QTextFrame *f = it.currentFrame()) {
         if (QTextTable *table = qobject_cast<QTextTable *>(f)) {
            emitTable(table);
         } else {
            emitTextFrame(f);
         }
      } else if (it.currentBlock().isValid()) {
         emitBlock(it.currentBlock());
      }
   }
}

void QTextHtmlExporter::emitTextFrame(const QTextFrame *f)
{
   FrameType frameType = f->parentFrame() ? TextFrame : RootFrame;

   html += QLatin1String("\n<table");
   QTextFrameFormat format = f->frameFormat();

   if (format.hasProperty(QTextFormat::FrameBorder)) {
      emitAttribute("border", QString::number(format.border()));
   }

   emitFrameStyle(format, frameType);

   emitTextLength("width", format.width());
   emitTextLength("height", format.height());

   // root frame's bcolor goes in the <body> tag
   if (frameType != RootFrame) {
      emitBackgroundAttribute(format);
   }

   html += QLatin1Char('>');
   html += QLatin1String("\n<tr>\n<td style=\"border: none;\">");
   emitFrame(f->begin());
   html += QLatin1String("</td></tr></table>");
}

void QTextHtmlExporter::emitFrameStyle(const QTextFrameFormat &format, FrameType frameType)
{
   QString styleAttribute(" style=\"");

   html += styleAttribute;
   const int originalHtmlLength = html.length();

   if (frameType == TextFrame) {
      html += "-qt-table-type: frame;";
   } else if (frameType == RootFrame) {
      html += "-qt-table-type: root;";
   }

   const QTextFrameFormat defaultFormat;

   emitFloatStyle(format.position(), OmitStyleTag);
   emitPageBreakPolicy(format.pageBreakPolicy());

   if (format.borderBrush() != defaultFormat.borderBrush()) {
      html += " border-color:";
      html += colorValue(format.borderBrush().color());
      html += ';';
   }

   if (format.borderStyle() != defaultFormat.borderStyle()) {
      emitBorderStyle(format.borderStyle());
   }

   if (format.hasProperty(QTextFormat::FrameMargin)
      || format.hasProperty(QTextFormat::FrameLeftMargin)
      || format.hasProperty(QTextFormat::FrameRightMargin)
      || format.hasProperty(QTextFormat::FrameTopMargin)
      || format.hasProperty(QTextFormat::FrameBottomMargin))
      emitMargins(QString::number(format.topMargin()),
         QString::number(format.bottomMargin()),
         QString::number(format.leftMargin()),
         QString::number(format.rightMargin()));

   if (html.length() == originalHtmlLength) {
      html.chop(styleAttribute.length());

   } else {
      html += QLatin1Char('\"');
   }
}

#ifndef QT_NO_TEXTHTMLPARSER
QString QTextDocument::toHtml(const QString &encoding) const
{
   return QTextHtmlExporter(this).toHtml(encoding);
}
#endif

QVector<QTextFormat> QTextDocument::allFormats() const
{
   Q_D(const QTextDocument);
   return d->formatCollection()->formats;
}

QTextDocumentPrivate *QTextDocument::docHandle() const
{
   Q_D(const QTextDocument);
   return const_cast<QTextDocumentPrivate *>(d);
}

