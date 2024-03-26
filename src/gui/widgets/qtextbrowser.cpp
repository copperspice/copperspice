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

#include <qtextbrowser.h>
#include <qtextedit_p.h>

#ifndef QT_NO_TEXTBROWSER

#include <qstack.h>
#include <qapplication.h>
#include <qevent.h>
#include <qdesktopwidget.h>
#include <qdebug.h>
#include <qabstracttextdocumentlayout.h>
#include <qtextcodec.h>
#include <qpainter.h>
#include <qdir.h>
#include <qwhatsthis.h>
#include <qtextobject.h>
#include <qdesktopservices.h>

#include <qtextdocumentlayout_p.h>

class QTextBrowserPrivate : public QTextEditPrivate
{
   Q_DECLARE_PUBLIC(QTextBrowser)

 public:
   inline QTextBrowserPrivate()
      : textOrSourceChanged(false), forceLoadOnSourceChange(false), openExternalLinks(false),
        openLinks(true)
#ifdef QT_KEYPAD_NAVIGATION
      , lastKeypadScrollValue(-1)
#endif
   {}

   void init();

   struct HistoryEntry {
      inline HistoryEntry()
         : hpos(0), vpos(0), focusIndicatorPosition(-1), focusIndicatorAnchor(-1)
      { }

      QUrl url;
      QString title;
      int hpos;
      int vpos;
      int focusIndicatorPosition, focusIndicatorAnchor;
   };

   HistoryEntry history(int i) const {

      if (i <= 0) {

         if (-i < stack.count()) {
            return stack[stack.count() + i - 1];
         } else {
            return HistoryEntry();
         }

      } else if (i <= forwardStack.count()) {
         return forwardStack[forwardStack.count() - i];

      } else {
         return HistoryEntry();
      }
   }

   HistoryEntry createHistoryEntry() const;
   void restoreHistoryEntry(const HistoryEntry entry);

   QStack<HistoryEntry> stack;
   QStack<HistoryEntry> forwardStack;
   QUrl home;
   QUrl currentURL;

   QStringList searchPaths;

   /*flag necessary to give the linkClicked() signal some meaningful
     semantics when somebody connected to it calls setText() or setSource() */
   bool textOrSourceChanged;
   bool forceLoadOnSourceChange;

   bool openExternalLinks;
   bool openLinks;

#ifndef QT_NO_CURSOR
   QCursor oldCursor;
#endif

   QString findFile(const QUrl &name) const;

   void _q_documentModified() {
      textOrSourceChanged = true;
      forceLoadOnSourceChange = !currentURL.path().isEmpty();
   }

   void _q_activateAnchor(const QString &anchor);
   void _q_highlightLink(const QString &anchor);

   void setSource(const QUrl &url);

   // re-imlemented from QTextEditPrivate
   QUrl resolveUrl(const QUrl &url) const override;

   QUrl resolveUrl(const QString &url) const {
      return resolveUrl(QUrl::fromEncoded(url.toUtf8()));
   }

#ifdef QT_KEYPAD_NAVIGATION
   void keypadMove(bool next);
   QTextCursor prevFocus;
   int lastKeypadScrollValue;
#endif

};

QString QTextBrowserPrivate::findFile(const QUrl &name) const
{
   QString fileName;

   if (name.scheme() == "qrc") {
      fileName = ":/" + name.path();

   } else if (name.scheme().isEmpty()) {
      fileName = name.path();

   } else {

#if defined(Q_OS_ANDROID)
      if (name.scheme() == QLatin1String("assets")) {
         fileName = QLatin1String("assets:") + name.path();
      } else
#endif

         fileName = name.toLocalFile();
   }

   if (QFileInfo(fileName).isAbsolute()) {
      return fileName;
   }

   for (QString path : searchPaths) {

      if (! path.endsWith(QLatin1Char('/'))) {
         path.append(QLatin1Char('/'));
      }

      path.append(fileName);

      if (QFileInfo(path).isReadable()) {
         return path;
      }
   }

   return fileName;
}

QUrl QTextBrowserPrivate::resolveUrl(const QUrl &url) const
{
   if (! url.isRelative()) {
      return url;
   }

   // For the second case QUrl can merge "#someanchor" with "foo.html"
   // correctly to "foo.html#someanchor"
   if (!(currentURL.isRelative()
         || (currentURL.scheme() == QLatin1String("file")
            && !QFileInfo(currentURL.toLocalFile()).isAbsolute()))
      || (url.hasFragment() && url.path().isEmpty())) {
      return currentURL.resolved(url);
   }

   // this is our last resort when current url and new url are both relative
   // we try to resolve against the current working directory in the local
   // file system.
   QFileInfo fi(currentURL.toLocalFile());
   if (fi.exists()) {
      return QUrl::fromLocalFile(fi.absolutePath() + QDir::separator()).resolved(url);
   }

   return url;
}

void QTextBrowserPrivate::_q_activateAnchor(const QString &anchor)
{
   if (anchor.isEmpty()) {
      return;
   }

   Q_Q(QTextBrowser);

#ifndef QT_NO_CURSOR
   viewport->setCursor(oldCursor);
#endif

   const QUrl url = resolveUrl(anchor);

   if (! openLinks) {
      emit q->anchorClicked(url);
      return;
   }

   textOrSourceChanged = false;

#ifndef QT_NO_DESKTOPSERVICES

   bool isFileScheme = url.scheme() == "file" || url.scheme() == "qrc";

   if ((openExternalLinks && ! isFileScheme && ! url.isRelative())
      || (url.isRelative() && ! currentURL.isRelative() && ! isFileScheme)) {

      QDesktopServices::openUrl(url);
      return;
   }

#endif

   emit q->anchorClicked(url);

   if (textOrSourceChanged) {
      return;
   }

   q->setSource(url);
}

void QTextBrowserPrivate::_q_highlightLink(const QString &anchor)
{
   Q_Q(QTextBrowser);

   if (anchor.isEmpty()) {

#ifndef QT_NO_CURSOR
      if (viewport->cursor().shape() != Qt::PointingHandCursor) {
         oldCursor = viewport->cursor();
      }
      viewport->setCursor(oldCursor);
#endif

      emit q->highlighted(QUrl());
      emit q->highlighted(QString());

   } else {
#ifndef QT_NO_CURSOR
      viewport->setCursor(Qt::PointingHandCursor);
#endif

      const QUrl url = resolveUrl(anchor);
      emit q->highlighted(url);

      // convenience to ease connecting to QStatusBar::showMessage(const QString &)
      emit q->highlighted(url.toString());
   }
}

void QTextBrowserPrivate::setSource(const QUrl &url)
{
   Q_Q(QTextBrowser);

#ifndef QT_NO_CURSOR
   if (q->isVisible()) {
      QApplication::setOverrideCursor(Qt::WaitCursor);
   }
#endif

   textOrSourceChanged = true;

   QString txt;

   bool doSetText = false;

   QUrl currentUrlWithoutFragment = currentURL;
   currentUrlWithoutFragment.setFragment(QString());
   QUrl newUrlWithoutFragment = currentURL.resolved(url);
   newUrlWithoutFragment.setFragment(QString());

   if (url.isValid() && (newUrlWithoutFragment != currentUrlWithoutFragment || forceLoadOnSourceChange)) {
      QVariant data = q->loadResource(QTextDocument::HtmlResource, resolveUrl(url));

      if (data.type() == QVariant::String) {
         txt = data.toString();

      } else if (data.type() == QVariant::ByteArray) {

#ifndef QT_NO_TEXTCODEC
         QByteArray ba = data.toByteArray();
         QTextCodec *codec = Qt::codecForHtml(ba);
         txt = codec->toUnicode(ba);
#else
         txt = data.toString();
#endif
      }

      if (txt.isEmpty()) {
         qWarning("QTextBrowser:setSource() No document for %s", csPrintable(url.toString()) );
      }

      if (q->isVisible()) {
         QString firstTag = txt.left(txt.indexOf('>') + 1);

         if (firstTag.startsWith("<qt") && firstTag.contains("type") && firstTag.contains("detail")) {
#ifndef QT_NO_CURSOR
            QApplication::restoreOverrideCursor();
#endif

#ifndef QT_NO_WHATSTHIS
            QWhatsThis::showText(QCursor::pos(), txt, q);
#endif
            return;
         }
      }

      currentURL = resolveUrl(url);
      doSetText = true;
   }

   if (!home.isValid()) {
      home = url;
   }

   if (doSetText) {
#ifndef QT_NO_TEXTHTMLPARSER
      q->QTextEdit::setHtml(txt);
      q->document()->setMetaInformation(QTextDocument::DocumentUrl, currentURL.toString());
#else
      q->QTextEdit::setPlainText(txt);
#endif

#ifdef QT_KEYPAD_NAVIGATION
      prevFocus.movePosition(QTextCursor::Start);
#endif
   }

   forceLoadOnSourceChange = false;

   if (!url.fragment().isEmpty()) {
      q->scrollToAnchor(url.fragment());
   } else {
      hbar->setValue(0);
      vbar->setValue(0);
   }
#ifdef QT_KEYPAD_NAVIGATION
   lastKeypadScrollValue = vbar->value();
   emit q->highlighted(QUrl());
   emit q->highlighted(QString());
#endif

#ifndef QT_NO_CURSOR
   if (q->isVisible()) {
      QApplication::restoreOverrideCursor();
   }
#endif
   emit q->sourceChanged(url);
}

#ifdef QT_KEYPAD_NAVIGATION
void QTextBrowserPrivate::keypadMove(bool next)
{
   Q_Q(QTextBrowser);

   const int height = viewport->height();
   const int overlap = qBound(20, height / 5, 40); // XXX arbitrary, but a good balance
   const int visibleLinkAmount = overlap; // consistent, but maybe not the best choice (?)
   int yOffset = vbar->value();
   int scrollYOffset = qBound(0, next ? yOffset + height - overlap : yOffset - height + overlap, vbar->maximum());

   bool foundNextAnchor = false;
   bool focusIt = false;
   int focusedPos = -1;

   QTextCursor anchorToFocus;

   QRectF viewRect = QRectF(0, yOffset, control->size().width(), height);
   QRectF newViewRect = QRectF(0, scrollYOffset, control->size().width(), height);
   QRectF bothViewRects = viewRect.united(newViewRect);

   // If we don't have a previous anchor, pretend that we had the first/last character
   // on the screen selected.
   if (prevFocus.isNull()) {
      if (next) {
         prevFocus = control->cursorForPosition(QPointF(0, yOffset));
      } else {
         prevFocus = control->cursorForPosition(QPointF(control->size().width(), yOffset + height));
      }
   }

   // First, check to see if someone has moved the scroll bars independently
   if (lastKeypadScrollValue != yOffset) {
      // Someone (user or programmatically) has moved us, so we might
      // need to start looking from the current position instead of prevFocus

      bool findOnScreen = true;

      // If prevFocus is on screen at all, we just use it.
      if (prevFocus.hasSelection()) {
         QRectF prevRect = control->selectionRect(prevFocus);
         if (viewRect.intersects(prevRect)) {
            findOnScreen = false;
         }
      }

      // Otherwise, we find a new anchor that's on screen.
      // Basically, create a cursor with the last/first character
      // on screen
      if (findOnScreen) {
         if (next) {
            prevFocus = control->cursorForPosition(QPointF(0, yOffset));
         } else {
            prevFocus = control->cursorForPosition(QPointF(control->size().width(), yOffset + height));
         }
      }
      foundNextAnchor = control->findNextPrevAnchor(prevFocus, next, anchorToFocus);
   } else if (prevFocus.hasSelection()) {
      // Check the pathological case that the current anchor is higher
      // than the screen, and just scroll through it in that case
      QRectF prevRect = control->selectionRect(prevFocus);
      if ((next && prevRect.bottom() > (yOffset + height)) ||
         (!next && prevRect.top() < yOffset)) {
         anchorToFocus = prevFocus;
         focusedPos = scrollYOffset;
         focusIt = true;
      } else {
         // This is the "normal" case - no scroll bar adjustments, no large anchors,
         // and no wrapping.
         foundNextAnchor = control->findNextPrevAnchor(prevFocus, next, anchorToFocus);
      }
   }

   // If not found yet, see if we need to wrap
   if (!focusIt && !foundNextAnchor) {
      if (next) {
         if (yOffset == vbar->maximum()) {
            prevFocus.movePosition(QTextCursor::Start);
            yOffset = scrollYOffset = 0;

            // Refresh the rectangles
            viewRect = QRectF(0, yOffset, control->size().width(), height);
            newViewRect = QRectF(0, scrollYOffset, control->size().width(), height);
            bothViewRects = viewRect.united(newViewRect);
         }
      } else {
         if (yOffset == 0) {
            prevFocus.movePosition(QTextCursor::End);
            yOffset = scrollYOffset = vbar->maximum();

            // Refresh the rectangles
            viewRect = QRectF(0, yOffset, control->size().width(), height);
            newViewRect = QRectF(0, scrollYOffset, control->size().width(), height);
            bothViewRects = viewRect.united(newViewRect);
         }
      }

      // Try looking now
      foundNextAnchor = control->findNextPrevAnchor(prevFocus, next, anchorToFocus);
   }

   // If we did actually find an anchor to use...
   if (foundNextAnchor) {
      QRectF desiredRect = control->selectionRect(anchorToFocus);

      // XXX This is an arbitrary heuristic
      // Decide to focus an anchor if it will be at least be
      // in the middle region of the screen after a scroll.
      // This can result in partial anchors with focus, but
      // insisting on links being completely visible before
      // selecting them causes disparities between links that
      // take up 90% of the screen height and those that take
      // up e.g. 110%
      // Obviously if a link is entirely visible, we still
      // focus it.
      if (bothViewRects.contains(desiredRect)
         || bothViewRects.adjusted(0, visibleLinkAmount, 0, -visibleLinkAmount).intersects(desiredRect)) {
         focusIt = true;

         // We aim to put the new link in the middle of the screen,
         // unless the link is larger than the screen (we just move to
         // display the first page of the link)
         if (desiredRect.height() > height) {
            if (next) {
               focusedPos = (int) desiredRect.top();
            } else {
               focusedPos = (int) desiredRect.bottom() - height;
            }
         } else {
            focusedPos = (int) ((desiredRect.top() + desiredRect.bottom()) / 2 - (height / 2));
         }

         // and clamp it to make sure we don't skip content.
         if (next) {
            focusedPos = qBound(yOffset, focusedPos, scrollYOffset);
         } else {
            focusedPos = qBound(scrollYOffset, focusedPos, yOffset);
         }
      }
   }

   // If we didn't get a new anchor, check if the old one is still on screen when we scroll
   // Note that big (larger than screen height) anchors also have some handling at the
   // start of this function.
   if (!focusIt && prevFocus.hasSelection()) {
      QRectF desiredRect = control->selectionRect(prevFocus);
      // XXX this may be better off also using the visibleLinkAmount value
      if (newViewRect.intersects(desiredRect)) {
         focusedPos = scrollYOffset;
         focusIt = true;
         anchorToFocus = prevFocus;
      }
   }

   // setTextCursor ensures that the cursor is visible. save & restore
   // the scroll bar values therefore
   const int savedXOffset = hbar->value();

   // Now actually process our decision
   if (focusIt && control->setFocusToAnchor(anchorToFocus)) {
      // Save the focus for next time
      prevFocus = control->textCursor();

      // Scroll
      vbar->setValue(focusedPos);
      lastKeypadScrollValue = focusedPos;
      hbar->setValue(savedXOffset);

      // Ensure that the new selection is highlighted.
      const QString href = control->anchorAtCursor();
      QUrl url = resolveUrl(href);

      emit q->highlighted(url);
      emit q->highlighted(url.toString());

   } else {
      // Scroll
      vbar->setValue(scrollYOffset);
      lastKeypadScrollValue = scrollYOffset;

      // now make sure we don't have a focused anchor
      QTextCursor cursor = control->textCursor();
      cursor.clearSelection();

      control->setTextCursor(cursor);

      hbar->setValue(savedXOffset);
      vbar->setValue(scrollYOffset);

      emit q->highlighted(QUrl());
      emit q->highlighted(QString());
   }
}
#endif

QTextBrowserPrivate::HistoryEntry QTextBrowserPrivate::createHistoryEntry() const
{
   HistoryEntry entry;
   entry.url = q_func()->source();
   entry.title = q_func()->documentTitle();
   entry.hpos = hbar->value();
   entry.vpos = vbar->value();

   const QTextCursor cursor = control->textCursor();
   if (control->cursorIsFocusIndicator()
      && cursor.hasSelection()) {

      entry.focusIndicatorPosition = cursor.position();
      entry.focusIndicatorAnchor = cursor.anchor();
   }
   return entry;
}

void QTextBrowserPrivate::restoreHistoryEntry(const HistoryEntry entry)
{
   setSource(entry.url);
   hbar->setValue(entry.hpos);
   vbar->setValue(entry.vpos);

   if (entry.focusIndicatorAnchor != -1 && entry.focusIndicatorPosition != -1) {
      QTextCursor cursor(control->document());
      cursor.setPosition(entry.focusIndicatorAnchor);
      cursor.setPosition(entry.focusIndicatorPosition, QTextCursor::KeepAnchor);
      control->setTextCursor(cursor);
      control->setCursorIsFocusIndicator(true);
   }

#ifdef QT_KEYPAD_NAVIGATION
   lastKeypadScrollValue = vbar->value();
   prevFocus = control->textCursor();

   Q_Q(QTextBrowser);

   const QString href = prevFocus.charFormat().anchorHref();
   QUrl url = resolveUrl(href);

   emit q->highlighted(url);
   emit q->highlighted(url.toString());
#endif
}

void QTextBrowserPrivate::init()
{
   Q_Q(QTextBrowser);
   control->setTextInteractionFlags(Qt::TextBrowserInteraction);

#ifndef QT_NO_CURSOR
   viewport->setCursor(oldCursor);
#endif

   q->setUndoRedoEnabled(false);
   viewport->setMouseTracking(true);

   QObject::connect(q->document(), &QTextDocument::contentsChanged, q, &QTextBrowser::_q_documentModified);
   QObject::connect(control,       &QTextControl::linkActivated,    q, &QTextBrowser::_q_activateAnchor);
   QObject::connect(control,       &QTextControl::linkHovered,      q, &QTextBrowser::_q_highlightLink);
}

QTextBrowser::QTextBrowser(QWidget *parent)
   : QTextEdit(*new QTextBrowserPrivate, parent)
{
   Q_D(QTextBrowser);
   d->init();
}

QTextBrowser::~QTextBrowser()
{
}

QUrl QTextBrowser::source() const
{
   Q_D(const QTextBrowser);

   if (d->stack.isEmpty()) {
      return QUrl();
   } else {
      return d->stack.top().url;
   }
}

QStringList QTextBrowser::searchPaths() const
{
   Q_D(const QTextBrowser);
   return d->searchPaths;
}

void QTextBrowser::setSearchPaths(const QStringList &paths)
{
   Q_D(QTextBrowser);
   d->searchPaths = paths;
}

void QTextBrowser::reload()
{
   Q_D(QTextBrowser);
   QUrl s = d->currentURL;
   d->currentURL = QUrl();
   setSource(s);
}

void QTextBrowser::setSource(const QUrl &url)
{
   Q_D(QTextBrowser);

   const QTextBrowserPrivate::HistoryEntry historyEntry = d->createHistoryEntry();

   d->setSource(url);

   if (!url.isValid()) {
      return;
   }

   // the same url you are already watching?
   if (!d->stack.isEmpty() && d->stack.top().url == url) {
      return;
   }

   if (!d->stack.isEmpty()) {
      d->stack.top() = historyEntry;
   }

   QTextBrowserPrivate::HistoryEntry entry;
   entry.url = url;
   entry.title = documentTitle();
   entry.hpos = 0;
   entry.vpos = 0;
   d->stack.push(entry);

   emit backwardAvailable(d->stack.count() > 1);

   if (!d->forwardStack.isEmpty() && d->forwardStack.top().url == url) {
      d->forwardStack.pop();
      emit forwardAvailable(d->forwardStack.count() > 0);
   } else {
      d->forwardStack.clear();
      emit forwardAvailable(false);
   }

   emit historyChanged();
}

void QTextBrowser::backward()
{
   Q_D(QTextBrowser);
   if (d->stack.count() <= 1) {
      return;
   }

   // Update the history entry
   d->forwardStack.push(d->createHistoryEntry());
   d->stack.pop(); // throw away the old version of the current entry
   d->restoreHistoryEntry(d->stack.top()); // previous entry
   emit backwardAvailable(d->stack.count() > 1);
   emit forwardAvailable(true);
   emit historyChanged();
}

void QTextBrowser::forward()
{
   Q_D(QTextBrowser);
   if (d->forwardStack.isEmpty()) {
      return;
   }
   if (!d->stack.isEmpty()) {
      // Update the history entry
      d->stack.top() = d->createHistoryEntry();
   }
   d->stack.push(d->forwardStack.pop());
   d->restoreHistoryEntry(d->stack.top());
   emit backwardAvailable(true);
   emit forwardAvailable(!d->forwardStack.isEmpty());
   emit historyChanged();
}

void QTextBrowser::home()
{
   Q_D(QTextBrowser);
   if (d->home.isValid()) {
      setSource(d->home);
   }
}

void QTextBrowser::keyPressEvent(QKeyEvent *ev)
{
#ifdef QT_KEYPAD_NAVIGATION
   Q_D(QTextBrowser);

   switch (ev->key()) {
      case Qt::Key_Select:
         if (QApplication::keypadNavigationEnabled()) {
            if (!hasEditFocus()) {
               setEditFocus(true);
               return;

            } else {
               QTextCursor cursor = d->control->textCursor();
               QTextCharFormat charFmt = cursor.charFormat();

               if (!cursor.hasSelection() || charFmt.anchorHref().isEmpty()) {
                  ev->accept();
                  return;
               }
            }
         }
         break;

      case Qt::Key_Back:
         if (QApplication::keypadNavigationEnabled()) {
            if (hasEditFocus()) {
               setEditFocus(false);
               ev->accept();
               return;
            }
         }
         QTextEdit::keyPressEvent(ev);
         return;
      default:
         if (QApplication::keypadNavigationEnabled() && !hasEditFocus()) {
            ev->ignore();
            return;
         }
   }
#endif

   if (ev->modifiers() & Qt::AltModifier) {
      switch (ev->key()) {
         case Qt::Key_Right:
            forward();
            ev->accept();
            return;
         case Qt::Key_Left:
            backward();
            ev->accept();
            return;
         case Qt::Key_Up:
            home();
            ev->accept();
            return;
      }
   }
#ifdef QT_KEYPAD_NAVIGATION
   else {
      if (ev->key() == Qt::Key_Up) {
         d->keypadMove(false);
         return;
      } else if (ev->key() == Qt::Key_Down) {
         d->keypadMove(true);
         return;
      }
   }
#endif
   QTextEdit::keyPressEvent(ev);
}

void QTextBrowser::mouseMoveEvent(QMouseEvent *e)
{
   QTextEdit::mouseMoveEvent(e);
}

void QTextBrowser::mousePressEvent(QMouseEvent *e)
{
   QTextEdit::mousePressEvent(e);
}

void QTextBrowser::mouseReleaseEvent(QMouseEvent *e)
{
   QTextEdit::mouseReleaseEvent(e);
}

void QTextBrowser::focusOutEvent(QFocusEvent *ev)
{
#ifndef QT_NO_CURSOR
   Q_D(QTextBrowser);
   d->viewport->setCursor((!(d->control->textInteractionFlags() & Qt::TextEditable)) ? d->oldCursor : Qt::IBeamCursor);
#endif
   QTextEdit::focusOutEvent(ev);
}

bool QTextBrowser::focusNextPrevChild(bool next)
{
   Q_D(QTextBrowser);

   if (d->control->setFocusToNextOrPreviousAnchor(next)) {
#ifdef QT_KEYPAD_NAVIGATION
      // Might need to synthesize a highlight event.
      if (d->prevFocus != d->control->textCursor() && d->control->textCursor().hasSelection()) {
         const QString href = d->control->anchorAtCursor();
         QUrl url = d->resolveUrl(href);

         emit highlighted(url);
         emit highlighted(url.toString());
      }

      d->prevFocus = d->control->textCursor();
#endif
      return true;

   } else {
#ifdef QT_KEYPAD_NAVIGATION
      // We assume we have no highlight now.
      emit highlighted(QUrl());
      emit highlighted(QString());
#endif

   }
   return QTextEdit::focusNextPrevChild(next);
}

void QTextBrowser::paintEvent(QPaintEvent *e)
{
   Q_D(QTextBrowser);
   QPainter p(d->viewport);
   d->paint(&p, e);
}

QVariant QTextBrowser::loadResource(int, const QUrl &name)
{
   Q_D(QTextBrowser);

   QByteArray data;
   QString fileName = d->findFile(d->resolveUrl(name));
   QFile f(fileName);
   if (f.open(QFile::ReadOnly)) {
      data = f.readAll();
      f.close();
   } else {
      return QVariant();
   }

   return data;
}


bool QTextBrowser::isBackwardAvailable() const
{
   Q_D(const QTextBrowser);
   return d->stack.count() > 1;
}

bool QTextBrowser::isForwardAvailable() const
{
   Q_D(const QTextBrowser);
   return !d->forwardStack.isEmpty();
}


void QTextBrowser::clearHistory()
{
   Q_D(QTextBrowser);
   d->forwardStack.clear();
   if (!d->stack.isEmpty()) {
      QTextBrowserPrivate::HistoryEntry historyEntry = d->stack.top();
      d->stack.resize(0);
      d->stack.push(historyEntry);
      d->home = historyEntry.url;
   }
   emit forwardAvailable(false);
   emit backwardAvailable(false);
   emit historyChanged();
}


QUrl QTextBrowser::historyUrl(int i) const
{
   Q_D(const QTextBrowser);
   return d->history(i).url;
}


QString QTextBrowser::historyTitle(int i) const
{
   Q_D(const QTextBrowser);
   return d->history(i).title;
}

int QTextBrowser::forwardHistoryCount() const
{
   Q_D(const QTextBrowser);
   return d->forwardStack.count();
}

int QTextBrowser::backwardHistoryCount() const
{
   Q_D(const QTextBrowser);
   return d->stack.count() - 1;
}

bool QTextBrowser::openExternalLinks() const
{
   Q_D(const QTextBrowser);
   return d->openExternalLinks;
}

void QTextBrowser::setOpenExternalLinks(bool open)
{
   Q_D(QTextBrowser);
   d->openExternalLinks = open;
}

bool QTextBrowser::openLinks() const
{
   Q_D(const QTextBrowser);
   return d->openLinks;
}

void QTextBrowser::setOpenLinks(bool open)
{
   Q_D(QTextBrowser);
   d->openLinks = open;
}

bool QTextBrowser::event(QEvent *e)
{
   return QTextEdit::event(e);
}

void QTextBrowser::_q_documentModified()
{
   Q_D(QTextBrowser);
   d->_q_documentModified();
}

void QTextBrowser::_q_activateAnchor(const QString &anchor)
{
   Q_D(QTextBrowser);
   d->_q_activateAnchor(anchor);
}

void QTextBrowser::_q_highlightLink(const QString &anchor)
{
   Q_D(QTextBrowser);
   d->_q_highlightLink(anchor);
}

#endif // QT_NO_TEXTBROWSER
