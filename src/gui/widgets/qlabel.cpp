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

#include <qlabel_p.h>

#include <qpainter.h>
#include <qevent.h>
#include <qdrawutil.h>
#include <qapplication.h>
#include <qabstractbutton.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qaction.h>
#include <qclipboard.h>
#include <qdebug.h>
#include <qurl.h>
#include <qmath.h>

#include <qstylesheetstyle_p.h>
#include <qtextengine_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif

QLabelPrivate::QLabelPrivate()
   : QFramePrivate(),

#ifndef QT_NO_PICTURE
     picture(nullptr),
#endif

#ifndef QT_NO_MOVIE
     movie(),
#endif

#ifndef QT_NO_CURSOR
     cursor(),
#endif

#ifndef QT_NO_SHORTCUT
     buddy(),
     shortcutId(0),
#endif

     textformat(Qt::AutoText),
     textInteractionFlags(Qt::LinksAccessibleByMouse),
     sizePolicy(),
     margin(0),
     align(Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs),
     indent(-1),
     valid_hints(false),
     scaledcontents(false),
     textLayoutDirty(false),
     textDirty(false), isRichText(false), isTextLabel(false),
     hasShortcut(),
     control(nullptr),
     shortcutCursor(), sh(), msh(), text(),
     pixmap(nullptr), scaledpixmap(nullptr), cachedimage(nullptr),

#ifndef QT_NO_CURSOR
     validCursor(false),
     onAnchor(false),
#endif

     openExternalLinks(false)
{}

QLabelPrivate::~QLabelPrivate()
{}

#ifndef QT_NO_PICTURE

const QPicture *QLabel::picture() const
{
   Q_D(const QLabel);
   return d->picture;
}

#endif

QLabel::QLabel(QWidget *parent, Qt::WindowFlags flags)
   : QFrame(*new QLabelPrivate(), parent, flags)
{
   Q_D(QLabel);
   d->init();
}

QLabel::QLabel(const QString &text, QWidget *parent, Qt::WindowFlags flags)
   : QFrame(*new QLabelPrivate(), parent, flags)
{
   Q_D(QLabel);
   d->init();
   setText(text);
}

QLabel::~QLabel()
{
   Q_D(QLabel);
   d->clearContents();
}

void QLabelPrivate::init()
{
   Q_Q(QLabel);

   q->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred,
         QSizePolicy::Label));

   setLayoutItemMargins(QStyle::SE_LabelLayoutItem);
}

void QLabel::setText(const QString &text)
{
   Q_D(QLabel);

   if (d->text == text) {
      return;
   }

   QTextControl *oldControl = d->control;
   d->control = nullptr;

   d->clearContents();
   d->text        = text;
   d->isTextLabel = true;
   d->textDirty   = true;
   d->isRichText  = d->textformat == Qt::RichText || (d->textformat == Qt::AutoText && Qt::mightBeRichText(d->text));

   d->control = oldControl;

   if (d->needTextControl()) {
      d->ensureTextControl();
   } else {
      delete d->control;
      d->control = nullptr;
   }

   if (d->isRichText) {
      setMouseTracking(true);
   } else {
      // Note: mouse tracking not disabled intentionally
   }

#ifndef QT_NO_SHORTCUT
   if (d->buddy) {
      d->updateShortcut();
   }
#endif

   d->updateLabel();

#ifndef QT_NO_ACCESSIBILITY
   if (accessibleName().isEmpty()) {
      QAccessibleEvent event(this, QAccessible::NameChanged);
      QAccessible::updateAccessibility(&event);
   }
#endif
}

QString QLabel::text() const
{
   Q_D(const QLabel);
   return d->text;
}

void QLabel::clear()
{
   Q_D(QLabel);
   d->clearContents();
   d->updateLabel();
}

void QLabel::setPixmap(const QPixmap &pixmap)
{
   Q_D(QLabel);
   if (!d->pixmap || d->pixmap->cacheKey() != pixmap.cacheKey()) {
      d->clearContents();
      d->pixmap = new QPixmap(pixmap);
   }

   if (d->pixmap->depth() == 1 && !d->pixmap->mask()) {
      d->pixmap->setMask(*((QBitmap *)d->pixmap));
   }

   d->updateLabel();
}

const QPixmap *QLabel::pixmap() const
{
   Q_D(const QLabel);
   return d->pixmap;
}

#ifndef QT_NO_PICTURE
void QLabel::setPicture(const QPicture &picture)
{
   Q_D(QLabel);
   d->clearContents();
   d->picture = new QPicture(picture);

   d->updateLabel();
}
#endif

void QLabel::setNum(int num)
{
   setText(QString::number(num));
}

void QLabel::setNum(double num)
{
   setText(QString::number(num));
}

void QLabel::setAlignment(Qt::Alignment alignment)
{
   Q_D(QLabel);

   if (alignment == (d->align & (Qt::AlignVertical_Mask | Qt::AlignHorizontal_Mask))) {
      return;
   }

   d->align = (d->align & ~(Qt::AlignVertical_Mask | Qt::AlignHorizontal_Mask))
      | (alignment & (Qt::AlignVertical_Mask | Qt::AlignHorizontal_Mask));

   d->updateLabel();
}

Qt::Alignment QLabel::alignment() const
{
   Q_D(const QLabel);
   return QFlag(d->align & (Qt::AlignVertical_Mask | Qt::AlignHorizontal_Mask));
}

void QLabel::setWordWrap(bool on)
{
   Q_D(QLabel);

   if (on) {
      d->align |= Qt::TextWordWrap;
   } else {
      d->align &= ~Qt::TextWordWrap;
   }

   d->updateLabel();
}

bool QLabel::wordWrap() const
{
   Q_D(const QLabel);
   return d->align & Qt::TextWordWrap;
}

void QLabel::setIndent(int indent)
{
   Q_D(QLabel);
   d->indent = indent;
   d->updateLabel();
}

int QLabel::indent() const
{
   Q_D(const QLabel);
   return d->indent;
}

int QLabel::margin() const
{
   Q_D(const QLabel);
   return d->margin;
}

void QLabel::setMargin(int margin)
{
   Q_D(QLabel);
   if (d->margin == margin) {
      return;
   }
   d->margin = margin;
   d->updateLabel();
}

QSize QLabelPrivate::sizeForWidth(int w) const
{
   Q_Q(const QLabel);
   if (q->minimumWidth() > 0) {
      w = qMax(w, q->minimumWidth());
   }
   QSize contentsMargin(leftmargin + rightmargin, topmargin + bottommargin);

   QRect br;

   int hextra = 2 * margin;
   int vextra = hextra;
   QFontMetrics fm = q->fontMetrics();

   if (pixmap && !pixmap->isNull()) {
      br = pixmap->rect();
      br.setSize(br.size() / pixmap->devicePixelRatio());

#ifndef QT_NO_PICTURE
   } else if (picture && !picture->isNull()) {
      br = picture->boundingRect();

#endif

#ifndef QT_NO_MOVIE
   } else if (movie && !movie->currentPixmap().isNull()) {
      br = movie->currentPixmap().rect();
      br.setSize(br.size() / movie->currentPixmap().devicePixelRatio());
#endif

   } else if (isTextLabel) {
      int align = QStyle::visualAlignment(textDirection(), QFlag(this->align));
      // Add indentation
      int m = indent;

      if (m < 0 && q->frameWidth()) {
         // no indent, but we do have a frame
         m = fm.width('x') - margin * 2;
      }

      if (m > 0) {
         if ((align & Qt::AlignLeft) || (align & Qt::AlignRight)) {
            hextra += m;
         }
         if ((align & Qt::AlignTop) || (align & Qt::AlignBottom)) {
            vextra += m;
         }
      }

      if (control) {
         ensureTextLayouted();

         const qreal oldTextWidth = control->textWidth();
         // Calculate the length of document if w is the width

         if (align & Qt::TextWordWrap) {
            if (w >= 0) {
               w = qMax(w - hextra - contentsMargin.width(), 0); // strip margin and indent
               control->setTextWidth(w);
            } else {
               control->adjustSize();
            }

         } else {
            control->setTextWidth(-1);
         }

         QSizeF controlSize = control->size();
         br = QRect(QPoint(0, 0), QSize(qCeil(controlSize.width()), qCeil(controlSize.height())));

         // restore state
         control->setTextWidth(oldTextWidth);

      } else {
         // Turn off center alignment in order to avoid rounding errors for centering,
         // since centering involves a division by 2. At the end, all we want is the size.
         int flags = align & ~(Qt::AlignVCenter | Qt::AlignHCenter);
         if (hasShortcut) {
            flags |= Qt::TextShowMnemonic;
            QStyleOption opt;
            opt.initFrom(q);
            if (!q->style()->styleHint(QStyle::SH_UnderlineShortcut, &opt, q)) {
               flags |= Qt::TextHideMnemonic;
            }
         }

         bool tryWidth = (w < 0) && (align & Qt::TextWordWrap);

         if (tryWidth) {
            w = qMin(fm.averageCharWidth() * 80, q->maximumSize().width());
         }

         else if (w < 0) {
            w = 2000;
         }

         w -= (hextra + contentsMargin.width());
         br = fm.boundingRect(0, 0, w, 2000, flags, text);
         if (tryWidth && br.height() < 4 * fm.lineSpacing() && br.width() > w / 2) {
            br = fm.boundingRect(0, 0, w / 2, 2000, flags, text);
         }
         if (tryWidth && br.height() < 2 * fm.lineSpacing() && br.width() > w / 4) {
            br = fm.boundingRect(0, 0, w / 4, 2000, flags, text);
         }
      }
   } else {
      br = QRect(QPoint(0, 0), QSize(fm.averageCharWidth(), fm.lineSpacing()));
   }

   const QSize contentsSize(br.width() + hextra, br.height() + vextra);
   return (contentsSize + contentsMargin).expandedTo(q->minimumSize());
}

int QLabel::heightForWidth(int w) const
{
   Q_D(const QLabel);
   if (d->isTextLabel) {
      return d->sizeForWidth(w).height();
   }
   return QWidget::heightForWidth(w);
}

bool QLabel::openExternalLinks() const
{
   Q_D(const QLabel);
   return d->openExternalLinks;
}

void QLabel::setOpenExternalLinks(bool open)
{
   Q_D(QLabel);
   d->openExternalLinks = open;
   if (d->control) {
      d->control->setOpenExternalLinks(open);
   }
}

void QLabel::setTextInteractionFlags(Qt::TextInteractionFlags flags)
{
   Q_D(QLabel);

   if (d->textInteractionFlags == flags) {
      return;
   }

   d->textInteractionFlags = flags;
   if (flags & Qt::LinksAccessibleByKeyboard) {
      setFocusPolicy(Qt::StrongFocus);
   } else if (flags & (Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse)) {
      setFocusPolicy(Qt::ClickFocus);
   } else {
      setFocusPolicy(Qt::NoFocus);
   }

   if (d->needTextControl()) {
      d->ensureTextControl();
   } else {
      delete d->control;
      d->control = nullptr;
   }

   if (d->control) {
      d->control->setTextInteractionFlags(d->textInteractionFlags);
   }
}

Qt::TextInteractionFlags QLabel::textInteractionFlags() const
{
   Q_D(const QLabel);
   return d->textInteractionFlags;
}

void QLabel::setSelection(int start, int length)
{
   Q_D(QLabel);
   if (d->control) {
      d->ensureTextPopulated();
      QTextCursor cursor = d->control->textCursor();
      cursor.setPosition(start);
      cursor.setPosition(start + length, QTextCursor::KeepAnchor);
      d->control->setTextCursor(cursor);
   }
}

bool QLabel::hasSelectedText() const
{
   Q_D(const QLabel);
   if (d->control) {
      return d->control->textCursor().hasSelection();
   }
   return false;
}

QString QLabel::selectedText() const
{
   Q_D(const QLabel);
   if (d->control) {
      return d->control->textCursor().selectedText();
   }
   return QString();
}

int QLabel::selectionStart() const
{
   Q_D(const QLabel);
   if (d->control && d->control->textCursor().hasSelection()) {
      return d->control->textCursor().selectionStart();
   }
   return -1;
}

QSize QLabel::sizeHint() const
{
   Q_D(const QLabel);
   if (!d->valid_hints) {
      (void) QLabel::minimumSizeHint();
   }
   return d->sh;
}

QSize QLabel::minimumSizeHint() const
{
   Q_D(const QLabel);
   if (d->valid_hints) {
      if (d->sizePolicy == sizePolicy()) {
         return d->msh;
      }
   }

   ensurePolished();
   d->valid_hints = true;
   d->sh = d->sizeForWidth(-1); // wrap ? golden ratio : min doc size
   QSize msh(-1, -1);

   if (!d->isTextLabel) {
      msh = d->sh;
   } else {
      msh.rheight() = d->sizeForWidth(QWIDGETSIZE_MAX).height(); // height for one line
      msh.rwidth() = d->sizeForWidth(0).width(); // wrap ? size of biggest word : min doc size
      if (d->sh.height() < msh.height()) {
         msh.rheight() = d->sh.height();
      }
   }

   d->msh = msh;
   d->sizePolicy = sizePolicy();
   return msh;
}

void QLabel::mousePressEvent(QMouseEvent *ev)
{
   Q_D(QLabel);
   d->sendControlEvent(ev);
}

void QLabel::mouseMoveEvent(QMouseEvent *ev)
{
   Q_D(QLabel);
   d->sendControlEvent(ev);
}

void QLabel::mouseReleaseEvent(QMouseEvent *ev)
{
   Q_D(QLabel);
   d->sendControlEvent(ev);
}

void QLabel::contextMenuEvent(QContextMenuEvent *ev)
{
#ifdef QT_NO_CONTEXTMENU
   (void) ev;
#else
   Q_D(QLabel);
   if (!d->isTextLabel) {
      ev->ignore();
      return;
   }
   QMenu *menu = d->createStandardContextMenu(ev->pos());
   if (!menu) {
      ev->ignore();
      return;
   }
   ev->accept();
   menu->setAttribute(Qt::WA_DeleteOnClose);
   menu->popup(ev->globalPos());
#endif
}

void QLabel::focusInEvent(QFocusEvent *ev)
{
   Q_D(QLabel);
   if (d->isTextLabel) {
      d->ensureTextControl();
      d->sendControlEvent(ev);
   }
   QFrame::focusInEvent(ev);
}

void QLabel::focusOutEvent(QFocusEvent *ev)
{
   Q_D(QLabel);

   if (d->control) {
      d->sendControlEvent(ev);
      QTextCursor cursor = d->control->textCursor();
      Qt::FocusReason reason = ev->reason();

      if (reason != Qt::ActiveWindowFocusReason
         && reason != Qt::PopupFocusReason
         && cursor.hasSelection()) {
         cursor.clearSelection();
         d->control->setTextCursor(cursor);
      }
   }

   QFrame::focusOutEvent(ev);
}

bool QLabel::focusNextPrevChild(bool next)
{
   Q_D(QLabel);

   if (d->control && d->control->setFocusToNextOrPreviousAnchor(next)) {
      return true;
   }

   return QFrame::focusNextPrevChild(next);
}

void QLabel::keyPressEvent(QKeyEvent *ev)
{
   Q_D(QLabel);
   d->sendControlEvent(ev);
}

bool QLabel::event(QEvent *e)
{
   Q_D(QLabel);
   QEvent::Type type = e->type();

#ifndef QT_NO_SHORTCUT
   if (type == QEvent::Shortcut) {
      QShortcutEvent *se = static_cast<QShortcutEvent *>(e);

      if (se->shortcutId() == d->shortcutId) {
         QWidget *w = d->buddy;
         QAbstractButton *button = qobject_cast<QAbstractButton *>(w);

         if (w->focusPolicy() != Qt::NoFocus) {
            w->setFocus(Qt::ShortcutFocusReason);
         }

         if (button && !se->isAmbiguous()) {
            button->animateClick();
         } else {
            window()->setAttribute(Qt::WA_KeyboardFocusChange);
         }
         return true;
      }

   } else

#endif
      if (type == QEvent::Resize) {
         if (d->control) {
            d->textLayoutDirty = true;
         }

      } else if (e->type() == QEvent::StyleChange

#ifdef Q_OS_DARWIN
         || e->type() == QEvent::MacSizeChange
#endif

      ) {
         d->setLayoutItemMargins(QStyle::SE_LabelLayoutItem);
         d->updateLabel();
      }

   return QFrame::event(e);
}

void QLabel::paintEvent(QPaintEvent *)
{
   Q_D(QLabel);

   QStyle *style = QWidget::style();

   QPainter painter(this);
   drawFrame(&painter);

   QRect cr = contentsRect();
   cr.adjust(d->margin, d->margin, -d->margin, -d->margin);

   int align = QStyle::visualAlignment(d->isTextLabel ? d->textDirection()
         : layoutDirection(), QFlag(d->align));

#ifndef QT_NO_MOVIE
   if (d->movie) {
      if (d->scaledcontents) {
         style->drawItemPixmap(&painter, cr, align, d->movie->currentPixmap().scaled(cr.size()));
      } else {
         style->drawItemPixmap(&painter, cr, align, d->movie->currentPixmap());
      }

   } else
#endif

      if (d->isTextLabel) {
         QRectF lr = d->layoutRect().toAlignedRect();
         QStyleOption opt;
         opt.initFrom(this);

#ifndef QT_NO_STYLE_STYLESHEET
         if (QStyleSheetStyle *cssStyle = qobject_cast<QStyleSheetStyle *>(style)) {
            cssStyle->styleSheetPalette(this, &opt, &opt.palette);
         }
#endif

         if (d->control) {
#ifndef QT_NO_SHORTCUT
            const bool underline = (bool)style->styleHint(QStyle::SH_UnderlineShortcut, nullptr, this, nullptr);

            if (d->shortcutId != 0
               && underline != d->shortcutCursor.charFormat().fontUnderline()) {
               QTextCharFormat fmt;
               fmt.setFontUnderline(underline);
               d->shortcutCursor.mergeCharFormat(fmt);
            }
#endif
            d->ensureTextLayouted();

            QAbstractTextDocumentLayout::PaintContext context;

            // Adjust the palette
            context.palette = opt.palette;

            if (foregroundRole() != QPalette::Text && isEnabled()) {
               context.palette.setColor(QPalette::Text, context.palette.color(foregroundRole()));
            }

            painter.save();
            painter.translate(lr.topLeft());
            painter.setClipRect(lr.translated(-lr.x(), -lr.y()));
            d->control->setPalette(context.palette);
            d->control->drawContents(&painter, QRectF(), this);
            painter.restore();

         } else {
            int flags = align | (d->textDirection() == Qt::LeftToRight ? Qt::TextForceLeftToRight
                  : Qt::TextForceRightToLeft);

            if (d->hasShortcut) {
               flags |= Qt::TextShowMnemonic;
               if (!style->styleHint(QStyle::SH_UnderlineShortcut, &opt, this)) {
                  flags |= Qt::TextHideMnemonic;
               }
            }
            style->drawItemText(&painter, lr.toRect(), flags, opt.palette, isEnabled(), d->text, foregroundRole());
         }

      } else

#ifndef QT_NO_PICTURE
         if (d->picture) {
            QRect br = d->picture->boundingRect();
            int rw = br.width();
            int rh = br.height();
            if (d->scaledcontents) {
               painter.save();
               painter.translate(cr.x(), cr.y());
               painter.scale((double)cr.width() / rw, (double)cr.height() / rh);
               painter.drawPicture(-br.x(), -br.y(), *d->picture);
               painter.restore();
            } else {
               int xo = 0;
               int yo = 0;
               if (align & Qt::AlignVCenter) {
                  yo = (cr.height() - rh) / 2;
               } else if (align & Qt::AlignBottom) {
                  yo = cr.height() - rh;
               }

               if (align & Qt::AlignRight) {
                  xo = cr.width() - rw;
               } else if (align & Qt::AlignHCenter) {
                  xo = (cr.width() - rw) / 2;
               }

               painter.drawPicture(cr.x() + xo - br.x(), cr.y() + yo - br.y(), *d->picture);
            }

         } else

#endif
            if (d->pixmap && !d->pixmap->isNull()) {
               QPixmap pix;

               if (d->scaledcontents) {
                  QSize scaledSize = cr.size() * devicePixelRatioF();

                  if (!d->scaledpixmap || d->scaledpixmap->size() != scaledSize) {
                     if (!d->cachedimage) {
                        d->cachedimage = new QImage(d->pixmap->toImage());
                     }

                     delete d->scaledpixmap;
                     QImage scaledImage =
                        d->cachedimage->scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                     d->scaledpixmap = new QPixmap(QPixmap::fromImage(scaledImage));
                     d->scaledpixmap->setDevicePixelRatio(devicePixelRatioF());
                  }
                  pix = *d->scaledpixmap;

               } else {
                  pix = *d->pixmap;
               }

               QStyleOption opt;
               opt.initFrom(this);

               if (!isEnabled()) {
                  pix = style->generatedIconPixmap(QIcon::Disabled, pix, &opt);
               }
               style->drawItemPixmap(&painter, cr, align, pix);
            }
}

void QLabelPrivate::updateLabel()
{
   Q_Q(QLabel);

   valid_hints = false;

   if (isTextLabel) {
      QSizePolicy policy = q->sizePolicy();
      const bool wrap = align & Qt::TextWordWrap;
      policy.setHeightForWidth(wrap);

      if (policy != q->sizePolicy()) {
         // ### should be replaced by WA_WState_OwnSizePolicy idiom
         q->setSizePolicy(policy);
      }

      textLayoutDirty = true;
   }

   q->updateGeometry();
   q->update(q->contentsRect());
}

#ifndef QT_NO_SHORTCUT
void QLabel::setBuddy(QWidget *buddy)
{
   Q_D(QLabel);
   d->buddy = buddy;

   if (d->isTextLabel) {
      if (d->shortcutId) {
         releaseShortcut(d->shortcutId);
      }

      d->shortcutId = 0;
      d->textDirty = true;

      if (buddy) {
         d->updateShortcut();   // grab new shortcut
      }

      d->updateLabel();
   }
}

QWidget *QLabel::buddy() const
{
   Q_D(const QLabel);
   return d->buddy;
}

void QLabelPrivate::updateShortcut()
{
   Q_Q(QLabel);
   Q_ASSERT(shortcutId == 0);
   // Introduce an extra boolean to indicate the presence of a shortcut in the
   // text. We cannot use the shortcutId itself because on the mac mnemonics are
   // off by default, so QKeySequence::mnemonic always returns an empty sequence.
   // But then we do want to hide the ampersands, so we can't use shortcutId.
   hasShortcut = false;

   if (!text.contains(QLatin1Char('&'))) {
      return;
   }
   hasShortcut = true;
   shortcutId = q->grabShortcut(QKeySequence::mnemonic(text));
}
#endif // QT_NO_SHORTCUT

#ifndef QT_NO_MOVIE
void QLabelPrivate::_q_movieUpdated(const QRect &rect)
{
   Q_Q(QLabel);
   if (movie && movie->isValid()) {
      QRect r;
      if (scaledcontents) {
         QRect cr = q->contentsRect();
         QRect pixmapRect(cr.topLeft(), movie->currentPixmap().size());
         if (pixmapRect.isEmpty()) {
            return;
         }
         r.setRect(cr.left(), cr.top(),
            (rect.width() * cr.width()) / pixmapRect.width(),
            (rect.height() * cr.height()) / pixmapRect.height());
      } else {
         r = q->style()->itemPixmapRect(q->contentsRect(), align, movie->currentPixmap());
         r.translate(rect.x(), rect.y());
         r.setWidth(qMin(r.width(), rect.width()));
         r.setHeight(qMin(r.height(), rect.height()));
      }
      q->update(r);
   }
}

void QLabelPrivate::_q_movieResized(const QSize &size)
{
   Q_Q(QLabel);
   q->update(); //we need to refresh the whole background in case the new size is smaler
   valid_hints = false;
   _q_movieUpdated(QRect(QPoint(0, 0), size));
   q->updateGeometry();
}

void QLabel::setMovie(QMovie *movie)
{
   Q_D(QLabel);
   d->clearContents();

   if (! movie) {
      return;
   }

   d->movie = movie;
   connect(movie, &QMovie::resized, this, &QLabel::_q_movieResized);
   connect(movie, &QMovie::updated, this, &QLabel::_q_movieUpdated);

   // Assume that if the movie is running,
   // resize/update signals will come soon enough
   if (movie->state() != QMovie::Running) {
      d->updateLabel();
   }
}

#endif // QT_NO_MOVIE

void QLabelPrivate::clearContents()
{
   delete control;
   control = nullptr;
   isTextLabel = false;
   hasShortcut = false;

#ifndef QT_NO_PICTURE
   delete picture;
   picture = nullptr;
#endif

   delete scaledpixmap;
   scaledpixmap = nullptr;

   delete cachedimage;
   cachedimage = nullptr;

   delete pixmap;
   pixmap = nullptr;

   text.clear();

   Q_Q(QLabel);

#ifndef QT_NO_SHORTCUT
   if (shortcutId) {
      q->releaseShortcut(shortcutId);
   }
   shortcutId = 0;
#endif

#ifndef QT_NO_MOVIE
   if (movie) {
      QObject::disconnect(movie.data(), &QMovie::resized, q, &QLabel::_q_movieResized);
      QObject::disconnect(movie.data(), &QMovie::updated, q, &QLabel::_q_movieUpdated);
   }
   movie = nullptr;
#endif

#ifndef QT_NO_CURSOR
   if (onAnchor) {
      if (validCursor) {
         q->setCursor(cursor);
      } else {
         q->unsetCursor();
      }
   }
   validCursor = false;
   onAnchor = false;
#endif
}

#ifndef QT_NO_MOVIE
QMovie *QLabel::movie() const
{
   Q_D(const QLabel);
   return d->movie;
}

#endif

Qt::TextFormat QLabel::textFormat() const
{
   Q_D(const QLabel);
   return d->textformat;
}

void QLabel::setTextFormat(Qt::TextFormat format)
{
   Q_D(QLabel);

   if (format != d->textformat) {
      d->textformat = format;
      QString t = d->text;

      if (! t.isEmpty()) {
         d->text.clear();
         setText(t);
      }
   }
}

void QLabel::changeEvent(QEvent *ev)
{
   Q_D(QLabel);

   if (ev->type() == QEvent::FontChange || ev->type() == QEvent::ApplicationFontChange) {
      if (d->isTextLabel) {
         if (d->control) {
            d->control->document()->setDefaultFont(font());
         }
         d->updateLabel();
      }

   } else if (ev->type() == QEvent::PaletteChange && d->control) {
      d->control->setPalette(palette());
   } else if (ev->type() == QEvent::ContentsRectChange) {
      d->updateLabel();
   }

   QFrame::changeEvent(ev);
}

bool QLabel::hasScaledContents() const
{
   Q_D(const QLabel);
   return d->scaledcontents;
}

void QLabel::setScaledContents(bool enable)
{
   Q_D(QLabel);

   if ((bool)d->scaledcontents == enable) {
      return;
   }

   d->scaledcontents = enable;

   if (!enable) {
      delete d->scaledpixmap;
      d->scaledpixmap = nullptr;

      delete d->cachedimage;
      d->cachedimage = nullptr;
   }
   update(contentsRect());
}

Qt::LayoutDirection QLabelPrivate::textDirection() const
{
   if (control) {
      QTextOption opt = control->document()->defaultTextOption();
      return opt.textDirection();
   }

   if (QTextEngine::isRightToLeft(text)) {
      return Qt::RightToLeft;
   } else {
      return Qt::LeftToRight;
   }
}

// Returns the rect that is available for us to draw the document
QRect QLabelPrivate::documentRect() const
{
   Q_Q(const QLabel);

   Q_ASSERT_X(isTextLabel, "documentRect():", "Called for label that is not a text label");
   QRect cr = q->contentsRect();

   cr.adjust(margin, margin, -margin, -margin);
   const int align = QStyle::visualAlignment(isTextLabel ? textDirection()
         : q->layoutDirection(), QFlag(this->align));

   int m = indent;
   if (m < 0 && q->frameWidth()) {
      // no indent, but we do have a frame
      m = q->fontMetrics().width('x') / 2 - margin;
   }
   if (m > 0) {
      if (align & Qt::AlignLeft) {
         cr.setLeft(cr.left() + m);
      }
      if (align & Qt::AlignRight) {
         cr.setRight(cr.right() - m);
      }
      if (align & Qt::AlignTop) {
         cr.setTop(cr.top() + m);
      }
      if (align & Qt::AlignBottom) {
         cr.setBottom(cr.bottom() - m);
      }
   }
   return cr;
}

void QLabelPrivate::ensureTextPopulated() const
{
   if (!textDirty) {
      return;
   }
   if (control) {
      QTextDocument *doc = control->document();
      if (textDirty) {
#ifndef QT_NO_TEXTHTMLPARSER
         if (isRichText) {
            doc->setHtml(text);
         } else {
            doc->setPlainText(text);
         }
#else
         doc->setPlainText(text);
#endif
         doc->setUndoRedoEnabled(false);

#ifndef QT_NO_SHORTCUT
         if (hasShortcut) {
            // Underline the first character that follows an ampersand (and remove the others ampersands)
            int from   = 0;
            bool found = false;
            QTextCursor cursor;

            while (!(cursor = control->document()->find(("&"), from)).isNull()) {
               cursor.deleteChar(); // remove the ampersand
               cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
               from = cursor.position();

               if (!found && cursor.selectedText() != "&") {
                  //not a second &
                  found = true;
                  shortcutCursor = cursor;
               }
            }
         }
#endif
      }
   }
   textDirty = false;
}

void QLabelPrivate::ensureTextLayouted() const
{
   if (!textLayoutDirty) {
      return;
   }
   ensureTextPopulated();
   if (control) {
      QTextDocument *doc = control->document();
      QTextOption opt = doc->defaultTextOption();

      opt.setAlignment(QFlag(this->align));

      if (this->align & Qt::TextWordWrap) {
         opt.setWrapMode(QTextOption::WordWrap);
      } else {
         opt.setWrapMode(QTextOption::ManualWrap);
      }

      doc->setDefaultTextOption(opt);

      QTextFrameFormat fmt = doc->rootFrame()->frameFormat();
      fmt.setMargin(0);
      doc->rootFrame()->setFrameFormat(fmt);
      doc->setTextWidth(documentRect().width());
   }
   textLayoutDirty = false;
}

void QLabelPrivate::ensureTextControl() const
{
   Q_Q(const QLabel);

   if (! isTextLabel) {
      return;
   }

   if (! control) {
      control = new QTextControl(const_cast<QLabel *>(q));
      control->document()->setUndoRedoEnabled(false);
      control->document()->setDefaultFont(q->font());
      control->setTextInteractionFlags(textInteractionFlags);
      control->setOpenExternalLinks(openExternalLinks);
      control->setPalette(q->palette());
      control->setFocus(q->hasFocus());

      QObject::connect(control, &QTextControl::updateRequest,  q, cs_mp_cast<>(&QLabel::update));
      QObject::connect(control, &QTextControl::linkHovered,    q, &QLabel::_q_linkHovered);
      QObject::connect(control, &QTextControl::linkActivated,  q, &QLabel::linkActivated);

      textLayoutDirty = true;
      textDirty = true;
   }
}

void QLabelPrivate::sendControlEvent(QEvent *e)
{
   Q_Q(QLabel);

   if (! isTextLabel || !control || textInteractionFlags == Qt::NoTextInteraction) {
      e->ignore();
      return;
   }

   control->processEvent(e, -layoutRect().topLeft(), q);
}

void QLabelPrivate::_q_linkHovered(const QString &anchor)
{
   Q_Q(QLabel);

#ifndef QT_NO_CURSOR
   if (anchor.isEmpty()) {
      // restore cursor

      if (validCursor) {
         q->setCursor(cursor);
      } else {
         q->unsetCursor();
      }
      onAnchor = false;

   } else if (!onAnchor) {
      validCursor = q->testAttribute(Qt::WA_SetCursor);
      if (validCursor) {
         cursor = q->cursor();
      }
      q->setCursor(Qt::PointingHandCursor);
      onAnchor = true;
   }
#endif

   emit q->linkHovered(anchor);
}

// Return the layout rect - this is the rect that is given to the layout painting code
// This may be different from the document rect since vertical alignment is not
// done by the text layout code
QRectF QLabelPrivate::layoutRect() const
{
   QRectF cr = documentRect();
   if (!control) {
      return cr;
   }
   ensureTextLayouted();
   // Caculate y position manually
   qreal rh = control->document()->documentLayout()->documentSize().height();
   qreal yo = 0;
   if (align & Qt::AlignVCenter) {
      yo = qMax((cr.height() - rh) / 2, qreal(0));
   } else if (align & Qt::AlignBottom) {
      yo = qMax(cr.height() - rh, qreal(0));
   }
   return QRectF(cr.x(), yo + cr.y(), cr.width(), cr.height());
}

// Returns the point in the document rect adjusted with p
QPoint QLabelPrivate::layoutPoint(const QPoint &p) const
{
   QRect lr = layoutRect().toRect();
   return p - lr.topLeft();
}

#ifndef QT_NO_CONTEXTMENU
QMenu *QLabelPrivate::createStandardContextMenu(const QPoint &pos)
{
   QString linkToCopy;
   QPoint p;

   if (control && isRichText) {
      p = layoutPoint(pos);
      linkToCopy = control->document()->documentLayout()->anchorAt(p);
   }

   if (linkToCopy.isEmpty() && !control) {
      return nullptr;
   }

   return control->createStandardContextMenu(p, q_func());
}
#endif

#ifndef QT_NO_MOVIE
void QLabel::_q_movieUpdated(const QRect &rect)
{
   Q_D(QLabel);
   d->_q_movieUpdated(rect);
}

void QLabel::_q_movieResized(const QSize &size)
{
   Q_D(QLabel);
   d->_q_movieResized(size);
}
#endif

void QLabel::_q_linkHovered(const QString &anchor)
{
   Q_D(QLabel);
   d->_q_linkHovered(anchor);
}
