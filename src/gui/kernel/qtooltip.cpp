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

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qevent.h>

#include <qlabel.h>
#include <qpointer.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qstylepainter.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qeffects_p.h>
#include <qtextdocument.h>
#include <qdebug.h>
#include <qstylesheetstyle_p.h>

#ifndef QT_NO_TOOLTIP

class QTipLabel : public QLabel
{
   GUI_CS_OBJECT(QTipLabel)

 public:
   QTipLabel(const QString &text, QWidget *w, int msecDisplayTime);
   ~QTipLabel();
   static QTipLabel *instance;

   bool eventFilter(QObject *, QEvent *) override;

   QBasicTimer hideTimer, expireTimer;

   bool fadingOut;

   void reuseTip(const QString &text, int msecDisplayTime);
   void hideTip();
   void hideTipImmediately();
   void setTipRect(QWidget *w, const QRect &r);
   void restartExpireTimer(int msecDisplayTime);
   bool tipChanged(const QPoint &pos, const QString &text, QObject *o);
   void placeTip(const QPoint &pos, QWidget *w);

   static int getTipScreen(const QPoint &pos, QWidget *w);

 protected:
   void timerEvent(QTimerEvent *e) override;
   void paintEvent(QPaintEvent *e) override;
   void mouseMoveEvent(QMouseEvent *e) override;
   void resizeEvent(QResizeEvent *e) override;

 private:
   QWidget *widget;
   QRect rect;

#ifndef QT_NO_STYLE_STYLESHEET
 public:
   // internal, Cleanup the _q_stylesheet_parent propery
   GUI_CS_SLOT_1(Public, void styleSheetParentDestroyed())
   GUI_CS_SLOT_2(styleSheetParentDestroyed)

 private:
   QWidget *styleSheetParent;
#endif

};

QTipLabel *QTipLabel::instance = nullptr;

QTipLabel::QTipLabel(const QString &text, QWidget *w, int msecDisplayTime)

#ifndef QT_NO_STYLE_STYLESHEET
   : QLabel(w, Qt::ToolTip | Qt::BypassGraphicsProxyWidget), widget(nullptr), styleSheetParent(nullptr)
#else
   : QLabel(w, Qt::ToolTip | Qt::BypassGraphicsProxyWidget), widget(nullptr)
#endif
{
   delete instance;
   instance = this;
   setForegroundRole(QPalette::ToolTipText);
   setBackgroundRole(QPalette::ToolTipBase);
   setPalette(QToolTip::palette());
   ensurePolished();
   setMargin(1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, nullptr, this));
   setFrameStyle(QFrame::NoFrame);
   setAlignment(Qt::AlignLeft);
   setIndent(1);
   qApp->installEventFilter(this);
   setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, nullptr, this) / qreal(255.0));
   setMouseTracking(true);
   fadingOut = false;
   reuseTip(text, msecDisplayTime);
}

void QTipLabel::restartExpireTimer(int msecDisplayTime)
{
   int time = 10000 + 40 * qMax(0, text().length() - 100);
   if (msecDisplayTime > 0) {
      time = msecDisplayTime;
   }
   expireTimer.start(time, this);
   hideTimer.stop();
}

void QTipLabel::reuseTip(const QString &text, int msecDisplayTime)
{
#ifndef QT_NO_STYLE_STYLESHEET
   if (styleSheetParent) {
      disconnect(styleSheetParent, &QWidget::destroyed, QTipLabel::instance, &QTipLabel::styleSheetParentDestroyed);
      styleSheetParent = nullptr;
   }
#endif

   setWordWrap(Qt::mightBeRichText(text));
   setText(text);
   QFontMetrics fm(font());
   QSize extra(1, 0);

   // Make it look good with the default ToolTip font on Mac, which has a small descent.
   if (fm.descent() == 2 && fm.ascent() >= 11) {
      ++extra.rheight();
   }
   resize(sizeHint() + extra);
   restartExpireTimer(msecDisplayTime);
}

void QTipLabel::paintEvent(QPaintEvent *ev)
{
   QStylePainter p(this);
   QStyleOptionFrame opt;
   opt.initFrom(this);

   p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
   p.end();

   QLabel::paintEvent(ev);
}

void QTipLabel::resizeEvent(QResizeEvent *e)
{
   QStyleHintReturnMask frameMask;
   QStyleOption option;
   option.initFrom(this);

   if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask)) {
      setMask(frameMask.region);
   }

   QLabel::resizeEvent(e);
}

void QTipLabel::mouseMoveEvent(QMouseEvent *e)
{
   if (rect.isNull()) {
      return;
   }
   QPoint pos = e->globalPos();
   if (widget) {
      pos = widget->mapFromGlobal(pos);
   }
   if (!rect.contains(pos)) {
      hideTip();
   }
   QLabel::mouseMoveEvent(e);
}

QTipLabel::~QTipLabel()
{
   instance = nullptr;
}

void QTipLabel::hideTip()
{
   if (! hideTimer.isActive()) {
      hideTimer.start(300, this);
   }
}

void QTipLabel::hideTipImmediately()
{
   close();          // to trigger QEvent::Close which stops the animation
   deleteLater();
}

void QTipLabel::setTipRect(QWidget *w, const QRect &r)
{
   if (! r.isNull() && ! w) {
      qWarning("QToolTip::setTipRect() Current widget must be valid when a rectangle is specified");

   } else {
      widget = w;
      rect   = r;
   }
}

void QTipLabel::timerEvent(QTimerEvent *e)
{
   if (e->timerId() == hideTimer.timerId()
      || e->timerId() == expireTimer.timerId()) {
      hideTimer.stop();
      expireTimer.stop();

      hideTipImmediately();
   }
}

bool QTipLabel::eventFilter(QObject *o, QEvent *e)
{
   switch (e->type()) {
      case QEvent::Leave:
         hideTip();
         break;
      case QEvent::WindowActivate:
      case QEvent::WindowDeactivate:
      case QEvent::FocusIn:
      case QEvent::FocusOut:
      case QEvent::Close:
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:

      case QEvent::Wheel:
         hideTipImmediately();
         break;

      case QEvent::MouseMove:
         if (o == widget && !rect.isNull() && !rect.contains(static_cast<QMouseEvent *>(e)->pos())) {
            hideTip();
         }
      default:
         break;
   }
   return false;
}

int QTipLabel::getTipScreen(const QPoint &pos, QWidget *w)
{
   if (QApplication::desktop()->isVirtualDesktop()) {
      return QApplication::desktop()->screenNumber(pos);
   } else {
      return QApplication::desktop()->screenNumber(w);
   }
}

void QTipLabel::placeTip(const QPoint &pos, QWidget *w)
{
#ifndef QT_NO_STYLE_STYLESHEET
   if (testAttribute(Qt::WA_StyleSheet) || (w && qobject_cast<QStyleSheetStyle *>(w->style()))) {
      //the stylesheet need to know the real parent
      QTipLabel::instance->setProperty("_q_stylesheet_parent", QVariant::fromValue(w));

      //we force the style to be the QStyleSheetStyle, and force to clear the cache as well.
      QTipLabel::instance->setStyleSheet(QString("/* */"));

      // Set up for cleaning up this later...
      QTipLabel::instance->styleSheetParent = w;

      if (w) {
         connect(w, &QWidget::destroyed, QTipLabel::instance, &QTipLabel::styleSheetParentDestroyed);
      }
   }
#endif

   QRect screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, w));

   QPoint p = pos;
   p += QPoint(2, 16);

   if (p.x() + this->width() > screen.x() + screen.width()) {
      p.rx() -= 4 + this->width();
   }
   if (p.y() + this->height() > screen.y() + screen.height()) {
      p.ry() -= 24 + this->height();
   }
   if (p.y() < screen.y()) {
      p.setY(screen.y());
   }
   if (p.x() + this->width() > screen.x() + screen.width()) {
      p.setX(screen.x() + screen.width() - this->width());
   }
   if (p.x() < screen.x()) {
      p.setX(screen.x());
   }
   if (p.y() + this->height() > screen.y() + screen.height()) {
      p.setY(screen.y() + screen.height() - this->height());
   }
   this->move(p);
}

bool QTipLabel::tipChanged(const QPoint &pos, const QString &text, QObject *o)
{
   if (QTipLabel::instance->text() != text) {
      return true;
   }

   if (o != widget) {
      return true;
   }

   if (!rect.isNull()) {
      return !rect.contains(pos);
   } else {
      return false;
   }
}

void QToolTip::showText(const QPoint &pos, const QString &text, QWidget *w, const QRect &rect)
{
   showText(pos, text, w, rect, -1);
}
void QToolTip::showText(const QPoint &pos, const QString &text, QWidget *w, const QRect &rect, int msecDisplayTime)
{
   if (QTipLabel::instance && QTipLabel::instance->isVisible()) { // a tip does already exist
      if (text.isEmpty()) { // empty text means hide current tip
         QTipLabel::instance->hideTip();
         return;
      } else if (!QTipLabel::instance->fadingOut) {
         // If the tip has changed, reuse the one
         // that is showing (removes flickering)
         QPoint localPos = pos;
         if (w) {
            localPos = w->mapFromGlobal(pos);
         }

         if (QTipLabel::instance->tipChanged(localPos, text, w)) {
            QTipLabel::instance->reuseTip(text, msecDisplayTime);
            QTipLabel::instance->setTipRect(w, rect);
            QTipLabel::instance->placeTip(pos, w);
         }
         return;
      }
   }

   if (! text.isEmpty()) { // no tip can be reused, create new tip:

      new QTipLabel(text, w, msecDisplayTime); // sets QTipLabel::instance to itself

      QTipLabel::instance->setTipRect(w, rect);
      QTipLabel::instance->placeTip(pos, w);
      QTipLabel::instance->setObjectName(QLatin1String("qtooltip_label"));

#if ! defined(QT_NO_EFFECTS)
      if (QApplication::isEffectEnabled(Qt::UI_FadeTooltip)) {
         qFadeEffect(QTipLabel::instance);
      } else if (QApplication::isEffectEnabled(Qt::UI_AnimateTooltip)) {
         qScrollEffect(QTipLabel::instance);
      } else {
         QTipLabel::instance->showNormal();
      }

#else
      QTipLabel::instance->showNormal();
#endif

   }
}

void QToolTip::showText(const QPoint &pos, const QString &text, QWidget *w)
{
   QToolTip::showText(pos, text, w, QRect());
}

bool QToolTip::isVisible()
{
   return (QTipLabel::instance != nullptr && QTipLabel::instance->isVisible());
}

QString QToolTip::text()
{
   if (QTipLabel::instance) {
      return QTipLabel::instance->text();
   }
   return QString();
}

static QPalette *tooltip_palette()
{
   static QPalette retval;
   return &retval;
}

QPalette QToolTip::palette()
{
   return *tooltip_palette();
}

QFont QToolTip::font()
{
   return QApplication::font("QTipLabel");
}

void QToolTip::setPalette(const QPalette &palette)
{
   *tooltip_palette() = palette;

   if (QTipLabel::instance) {
      QTipLabel::instance->setPalette(palette);
   }
}

void QToolTip::setFont(const QFont &font)
{
   QApplication::setFont(font, "QTipLabel");
}

#ifndef QT_NO_STYLE_STYLESHEET
void QTipLabel::styleSheetParentDestroyed() {
   setProperty("_q_stylesheet_parent", QVariant());
   styleSheetParent = nullptr;
}
#endif

#endif // QT_NO_TOOLTIP
