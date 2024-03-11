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

#include <qwhatsthis.h>

#ifndef QT_NO_WHATSTHIS
#include <qpointer.h>
#include <qapplication.h>
#include <qtoolbutton.h>
#include <qdebug.h>
#include <qdesktopwidget.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qhash.h>
#include <qaction.h>
#include <qcursor.h>
#include <qbitmap.h>
#include <qtextdocument.h>
#include <qplatform_theme.h>

#include <qguiapplication_p.h>
#include <qtextdocumentlayout_p.h>

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible.h>
#endif


class QWhatsThat : public QWidget
{
   GUI_CS_OBJECT(QWhatsThat)

 public:
   QWhatsThat(const QString &txt, QWidget *parent, QWidget *showTextFor);
   ~QWhatsThat() ;

   static QWhatsThat *instance;

 protected:
   void showEvent(QShowEvent *e) override;
   void mousePressEvent(QMouseEvent *) override;
   void mouseReleaseEvent(QMouseEvent *) override;
   void mouseMoveEvent(QMouseEvent *) override;
   void keyPressEvent(QKeyEvent *) override;
   void paintEvent(QPaintEvent *) override;

 private:
   QPointer<QWidget>widget;
   bool pressed;
   QString text;
   QTextDocument *doc;
   QString anchor;
   QPixmap background;
};

QWhatsThat *QWhatsThat::instance = nullptr;

// shadowWidth not const, for XP drop-shadow-fu turns it to 0
static int shadowWidth  = 6;                 // also used as '5' and '6' and even '8' below
static constexpr const int vMargin = 8;
static constexpr const int hMargin = 12;

static inline bool dropShadow()
{
   if (const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme()) {
      return theme->themeHint(QPlatformTheme::DropShadow).toBool();
   }
   return false;
}
QWhatsThat::QWhatsThat(const QString &txt, QWidget *parent, QWidget *showTextFor)
   : QWidget(parent, Qt::Popup),
     widget(showTextFor), pressed(false), text(txt)
{
   delete instance;
   instance = this;
   setAttribute(Qt::WA_DeleteOnClose, true);
   setAttribute(Qt::WA_NoSystemBackground, true);
   if (parent) {
      setPalette(parent->palette());
   }
   setMouseTracking(true);
   setFocusPolicy(Qt::StrongFocus);

#ifndef QT_NO_CURSOR
   setCursor(Qt::ArrowCursor);
#endif

   QRect r;
   doc = nullptr;
   ensurePolished(); // Ensures style sheet font before size calc
   if (Qt::mightBeRichText(text)) {
      doc = new QTextDocument();
      doc->setUndoRedoEnabled(false);
      doc->setDefaultFont(QApplication::font(this));

#ifdef QT_NO_TEXTHTMLPARSER
      doc->setPlainText(text);
#else
      doc->setHtml(text);
#endif

      doc->setUndoRedoEnabled(false);
      doc->adjustSize();
      r.setTop(0);
      r.setLeft(0);
      r.setSize(doc->size().toSize());
   } else {
      int sw = QApplication::desktop()->width() / 3;
      if (sw < 200) {
         sw = 200;
      } else if (sw > 300) {
         sw = 300;
      }

      r = fontMetrics().boundingRect(0, 0, sw, 1000,
            Qt::AlignLeft + Qt::AlignTop
            + Qt::TextWordWrap + Qt::TextExpandTabs,
            text);
   }
   shadowWidth = dropShadow() ? 0 : 6;

   resize(r.width() + 2 * hMargin + shadowWidth, r.height() + 2 * vMargin + shadowWidth);
}

QWhatsThat::~QWhatsThat()
{
   instance = nullptr;
   if (doc) {
      delete doc;
   }
}

void QWhatsThat::showEvent(QShowEvent *)
{
   background = QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->internalWinId(),
         x(), y(), width(), height());
}

void QWhatsThat::mousePressEvent(QMouseEvent *e)
{
   pressed = true;
   if (e->button() == Qt::LeftButton && rect().contains(e->pos())) {
      if (doc) {
         anchor = doc->documentLayout()->anchorAt(e->pos() -  QPoint(hMargin, vMargin));
      }
      return;
   }
   close();
}

void QWhatsThat::mouseReleaseEvent(QMouseEvent *e)
{
   if (! pressed) {
      return;
   }

   if (widget && e->button() == Qt::LeftButton && doc && rect().contains(e->pos())) {
      QString a = doc->documentLayout()->anchorAt(e->pos() -  QPoint(hMargin, vMargin));
      QString href;

      if (anchor == a) {
         href = a;
      }

      anchor.clear();

      if (!href.isEmpty()) {
         QWhatsThisClickedEvent e(href);
         if (QApplication::sendEvent(widget, &e)) {
            return;
         }
      }
   }
   close();
}

void QWhatsThat::mouseMoveEvent(QMouseEvent *e)
{
#ifdef QT_NO_CURSOR
   (void) e;
#else
   if (! doc) {
      return;
   }

   QString a = doc->documentLayout()->anchorAt(e->pos() -  QPoint(hMargin, vMargin));

   if (! a.isEmpty()) {
      setCursor(Qt::PointingHandCursor);
   } else {
      setCursor(Qt::ArrowCursor);
   }
#endif
}

void QWhatsThat::keyPressEvent(QKeyEvent *)
{
   close();
}

void QWhatsThat::paintEvent(QPaintEvent *)
{
   const bool drawShadow = dropShadow();


   QRect r = rect();
   r.adjust(0, 0, -1, -1);
   if (drawShadow) {
      r.adjust(0, 0, -shadowWidth, -shadowWidth);
   }
   QPainter p(this);
   p.drawPixmap(0, 0, background);
   p.setPen(QPen(palette().toolTipText(), 0));
   p.setBrush(palette().toolTipBase());
   p.drawRect(r);
   int w = r.width();
   int h = r.height();
   p.setPen(palette().brush(QPalette::Dark).color());
   p.drawRect(1, 1, w - 2, h - 2);
   if (drawShadow) {
      p.setPen(palette().shadow().color());
      p.drawPoint(w + 5, 6);
      p.drawLine(w + 3, 6, w + 5, 8);
      p.drawLine(w + 1, 6, w + 5, 10);
      int i;
      for (i = 7; i < h; i += 2) {
         p.drawLine(w, i, w + 5, i + 5);
      }
      for (i = w - i + h; i > 6; i -= 2) {
         p.drawLine(i, h, i + 5, h + 5);
      }
      for (; i > 0 ; i -= 2) {
         p.drawLine(6, h + 6 - i, i + 5, h + 5);
      }
   }
   r.adjust(0, 0, 1, 1);
   p.setPen(palette().toolTipText().color());
   r.adjust(hMargin, vMargin, -hMargin, -vMargin);

   if (doc) {
      p.translate(r.x(), r.y());
      QRect rect = r;
      rect.translate(-r.x(), -r.y());
      p.setClipRect(rect);
      QAbstractTextDocumentLayout::PaintContext context;
      context.palette.setBrush(QPalette::Text, context.palette.toolTipText());
      doc->documentLayout()->draw(&p, context);
   } else {
      p.drawText(r, Qt::AlignLeft + Qt::AlignTop + Qt::TextWordWrap + Qt::TextExpandTabs, text);
   }
}

static const char *const button_image[] = {
   "16 16 3 1",
   "         c None",
   "o        c #000000",
   "a        c #000080",
   "o        aaaaa  ",
   "oo      aaa aaa ",
   "ooo    aaa   aaa",
   "oooo   aa     aa",
   "ooooo  aa     aa",
   "oooooo  a    aaa",
   "ooooooo     aaa ",
   "oooooooo   aaa  ",
   "ooooooooo aaa   ",
   "ooooo     aaa   ",
   "oo ooo          ",
   "o  ooo    aaa   ",
   "    ooo   aaa   ",
   "    ooo         ",
   "     ooo        ",
   "     ooo        "
};

class QWhatsThisPrivate : public QObject
{
 public:
   QWhatsThisPrivate();
   ~QWhatsThisPrivate();
   static QWhatsThisPrivate *instance;

   bool eventFilter(QObject *, QEvent *) override;
   QPointer<QAction> action;

   static void say(QWidget *, const QString &, int x = 0, int y = 0);
   static void notifyToplevels(QEvent *e);
   bool leaveOnMouseRelease;
};

void QWhatsThisPrivate::notifyToplevels(QEvent *e)
{
   QWidgetList toplevels = QApplication::topLevelWidgets();
   for (int i = 0; i < toplevels.count(); ++i) {
      QWidget *w = toplevels.at(i);
      QApplication::sendEvent(w, e);
   }
}

QWhatsThisPrivate *QWhatsThisPrivate::instance = nullptr;

QWhatsThisPrivate::QWhatsThisPrivate()
   : leaveOnMouseRelease(false)
{
   instance = this;
   qApp->installEventFilter(this);

   QPoint pos = QCursor::pos();

   if (QWidget *w = QApplication::widgetAt(pos)) {
      QHelpEvent e(QEvent::QueryWhatsThis, w->mapFromGlobal(pos), pos);
      bool sentEvent = QApplication::sendEvent(w, &e);

#ifdef QT_NO_CURSOR
      (void) sentEvent;
#else
      QApplication::setOverrideCursor((! sentEvent || !e.isAccepted()) ?
         Qt::ForbiddenCursor : Qt::WhatsThisCursor);
   } else {
      QApplication::setOverrideCursor(Qt::WhatsThisCursor);
#endif

   }

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleEvent event(this, QAccessible::ContextHelpStart);
   QAccessible::updateAccessibility(&event);
#endif
}

QWhatsThisPrivate::~QWhatsThisPrivate()
{
   if (action) {
      action->setChecked(false);
   }

#ifndef QT_NO_CURSOR
   QApplication::restoreOverrideCursor();
#endif

#ifndef QT_NO_ACCESSIBILITY
   QAccessibleEvent event(this, QAccessible::ContextHelpEnd);
   QAccessible::updateAccessibility(&event);
#endif

   instance = nullptr;
}

bool QWhatsThisPrivate::eventFilter(QObject *o, QEvent *e)
{
   if (!o->isWidgetType()) {
      return false;
   }
   QWidget *w = static_cast<QWidget *>(o);
   bool customWhatsThis = w->testAttribute(Qt::WA_CustomWhatsThis);

   switch (e->type()) {
      case QEvent::MouseButtonPress: {
         QMouseEvent *me = static_cast<QMouseEvent *>(e);
         if (me->button() == Qt::RightButton || customWhatsThis) {
            return false;
         }
         QHelpEvent e(QEvent::WhatsThis, me->pos(), me->globalPos());
         if (!QApplication::sendEvent(w, &e) || ! e.isAccepted()) {
            leaveOnMouseRelease = true;
         }

      }
      break;

      case QEvent::MouseMove: {
         QMouseEvent *me = static_cast<QMouseEvent *>(e);
         QHelpEvent e(QEvent::QueryWhatsThis, me->pos(), me->globalPos());
         bool sentEvent = QApplication::sendEvent(w, &e);

#ifdef QT_NO_CURSOR
         (void) sentEvent;
#else
         QApplication::changeOverrideCursor((!sentEvent || ! e.isAccepted()) ?
            Qt::ForbiddenCursor : Qt::WhatsThisCursor);
#endif
      }
      [[fallthrough]];

      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:
         if (leaveOnMouseRelease && e->type() == QEvent::MouseButtonRelease) {
            QWhatsThis::leaveWhatsThisMode();
         }
         if (static_cast<QMouseEvent *>(e)->button() == Qt::RightButton || customWhatsThis) {
            return false;   // ignore RMB release
         }
         break;

      case QEvent::KeyPress: {
         QKeyEvent *kev = (QKeyEvent *)e;

         if (kev->matches(QKeySequence::Cancel)) {
            QWhatsThis::leaveWhatsThisMode();
            return true;
         } else if (customWhatsThis) {
            return false;
         } else if (kev->key() == Qt::Key_Menu ||
            (kev->key() == Qt::Key_F10 &&
               kev->modifiers() == Qt::ShiftModifier)) {
            // we don't react to these keys, they are used for context menus
            return false;
         } else if (kev->key() != Qt::Key_Shift && kev->key() != Qt::Key_Alt // not a modifier key
            && kev->key() != Qt::Key_Control && kev->key() != Qt::Key_Meta) {
            QWhatsThis::leaveWhatsThisMode();
         }
      }
      break;
      default:
         return false;
   }
   return true;
}

class QWhatsThisAction: public QAction
{
   GUI_CS_OBJECT(QWhatsThisAction)

 public:
   explicit QWhatsThisAction(QObject *parent = nullptr);

 private:
   GUI_CS_SLOT_1(Private, void actionTriggered())
   GUI_CS_SLOT_2(actionTriggered)
};

QWhatsThisAction::QWhatsThisAction(QObject *parent) : QAction(tr("What's This?"), parent)
{
#ifndef QT_NO_IMAGEFORMAT_XPM
   QPixmap p(button_image);
   setIcon(p);
#endif

   setCheckable(true);
   connect(this, &QWhatsThisAction::triggered, this, &QWhatsThisAction::actionTriggered);

#ifndef QT_NO_SHORTCUT
   setShortcut(Qt::ShiftModifier + Qt::Key_F1);
#endif
}

void QWhatsThisAction::actionTriggered()
{
   if (isChecked()) {
      QWhatsThis::enterWhatsThisMode();
      QWhatsThisPrivate::instance->action = this;
   }
}

void QWhatsThis::enterWhatsThisMode()
{
   if (QWhatsThisPrivate::instance) {
      return;
   }
   (void) new QWhatsThisPrivate;
   QEvent e(QEvent::EnterWhatsThisMode);
   QWhatsThisPrivate::notifyToplevels(&e);
}

bool QWhatsThis::inWhatsThisMode()
{
   return (QWhatsThisPrivate::instance != nullptr);
}

void QWhatsThis::leaveWhatsThisMode()
{
   delete QWhatsThisPrivate::instance;
   QEvent e(QEvent::LeaveWhatsThisMode);
   QWhatsThisPrivate::notifyToplevels(&e);
}

void QWhatsThisPrivate::say(QWidget *widget, const QString &text, int x, int y)
{
   if (text.size() == 0) {
      return;
   }

   // make a fresh widget, and set it up
   QWhatsThat *whatsThat = new QWhatsThat(text, nullptr, widget);

   // find a suitable location
   int scr = (widget ?
         QApplication::desktop()->screenNumber(widget) : QApplication::desktop()->screenNumber(QPoint(x, y)));

   QRect screen = QApplication::desktop()->screenGeometry(scr);

   int w = whatsThat->width();
   int h = whatsThat->height();
   int sx = screen.x();
   int sy = screen.y();

   // first try locating the widget immediately above/below,
   // with nice alignment if possible.
   QPoint pos;
   if (widget) {
      pos = widget->mapToGlobal(QPoint(0, 0));
   }

   if (widget && w > widget->width() + 16) {
      x = pos.x() + widget->width() / 2 - w / 2;
   } else {
      x = x - w / 2;
   }

   // squeeze it in if that would result in part of what's this
   // being only partially visible
   if (x + w  + shadowWidth > sx + screen.width())
      x = (widget ? (qMin(screen.width(),
                  pos.x() + widget->width())
            ) : screen.width())
         - w;

   if (x < sx) {
      x = sx;
   }

   if (widget && h > widget->height() + 16) {
      y = pos.y() + widget->height() + 2; // below, two pixels spacing
      // what's this is above or below, wherever there's most space
      if (y + h + 10 > sy + screen.height()) {
         y = pos.y() + 2 - shadowWidth - h;   // above, overlap
      }
   }
   y = y + 2;

   // squeeze it in if that would result in part of what's this
   // being only partially visible
   if (y + h + shadowWidth > sy + screen.height())
      y = (widget ? (qMin(screen.height(),
                  pos.y() + widget->height())
            ) : screen.height())
         - h;
   if (y < sy) {
      y = sy;
   }

   whatsThat->move(x, y);
   whatsThat->show();
   whatsThat->grabKeyboard();
}

void QWhatsThis::showText(const QPoint &pos, const QString &text, QWidget *w)
{
   leaveWhatsThisMode();
   QWhatsThisPrivate::say(w, text, pos.x(), pos.y());
}

void QWhatsThis::hideText()
{
   delete QWhatsThat::instance;
}

QAction *QWhatsThis::createAction(QObject *parent)
{
   return new QWhatsThisAction(parent);
}

#endif
