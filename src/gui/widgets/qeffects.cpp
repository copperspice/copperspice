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

#ifndef QT_NO_EFFECTS

#include <qdesktopwidget.h>
#include <qevent.h>
#include <qimage.h>
#include <qpainter.h>
#include <qscreen.h>
#include <qpixmap.h>
#include <qpointer.h>
#include <qtimer.h>
#include <qelapsedtimer.h>
#include <qdebug.h>

#include <qeffects_p.h>

class QAlphaWidget: public QWidget, private QEffects
{
   GUI_CS_OBJECT(QAlphaWidget)

 public:
   QAlphaWidget(QWidget *w, Qt::WindowFlags flags = Qt::EmptyFlag);
   ~QAlphaWidget();

   void run(int time);

 protected:
   void paintEvent(QPaintEvent *e)  override;
   void closeEvent(QCloseEvent *)  override;
   void alphaBlend();
   bool eventFilter(QObject *, QEvent *)  override;

   GUI_CS_SLOT_1(Protected, void render())
   GUI_CS_SLOT_2(render)

 private:
   QPixmap pm;
   double alpha;
   QImage backImage;
   QImage frontImage;
   QImage mixedImage;
   QPointer<QWidget> widget;
   int duration;
   int elapsed;
   bool showWidget;
   QTimer anim;
   QElapsedTimer checkTime;
};

static QAlphaWidget *q_blend = nullptr;

QAlphaWidget::QAlphaWidget(QWidget *w, Qt::WindowFlags flags)
   : QWidget(QApplication::desktop()->screen(QApplication::desktop()->screenNumber(w)), flags)
{
#ifndef Q_OS_WIN
   setEnabled(false);
#endif

   setAttribute(Qt::WA_NoSystemBackground, true);
   widget = w;
   alpha = 0;
}

QAlphaWidget::~QAlphaWidget()
{
#if defined(Q_OS_WIN)
   // Restore user-defined opacity value
   if (widget) {
      widget->setWindowOpacity(1);
   }
#endif
}

void QAlphaWidget::paintEvent(QPaintEvent *)
{
   QPainter p(this);
   p.drawPixmap(0, 0, pm);
}

void QAlphaWidget::run(int time)
{
   duration = time;

   if (duration < 0) {
      duration = 150;
   }

   if (!widget) {
      return;
   }

   elapsed = 0;
   checkTime.start();

   showWidget = true;

#if defined(Q_OS_WIN)
   qApp->installEventFilter(this);
   widget->setWindowOpacity(0.0);
   widget->show();
   connect(&anim, &QTimer::timeout, this, &QAlphaWidget::render);
   anim.start(1);

#else
   //This is roughly equivalent to calling setVisible(true) without actually showing the widget
   widget->setAttribute(Qt::WA_WState_ExplicitShowHide, true);
   widget->setAttribute(Qt::WA_WState_Hidden, false);

   qApp->installEventFilter(this);

   move(widget->geometry().x(), widget->geometry().y());
   resize(widget->size().width(), widget->size().height());

   frontImage = widget->grab().toImage();
   backImage = QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId(),
         widget->geometry().x(), widget->geometry().y(),
         widget->geometry().width(), widget->geometry().height()).toImage();

   if (!backImage.isNull() && checkTime.elapsed() < duration / 2) {
      mixedImage = backImage.copy();
      pm = QPixmap::fromImage(mixedImage);
      show();
      setEnabled(false);

      connect(&anim, &QTimer::timeout, this, &QAlphaWidget::render);
      anim.start(1);

   } else {
      duration = 0;
      render();
   }
#endif
}

bool QAlphaWidget::eventFilter(QObject *o, QEvent *e)
{
   switch (e->type()) {
      case QEvent::Move:
         if (o != widget) {
            break;
         }
         move(widget->geometry().x(), widget->geometry().y());
         update();
         break;

      case QEvent::Hide:
      case QEvent::Close:
         if (o != widget) {
            break;
         }
         [[fallthrough]];

      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonDblClick:
         showWidget = false;
         render();
         break;

      case QEvent::KeyPress: {
         QKeyEvent *ke = (QKeyEvent *)e;
         if (ke->matches(QKeySequence::Cancel)) {
            showWidget = false;
         } else {
            duration = 0;
         }
         render();
         break;

      }

      default:
         break;
   }
   return QWidget::eventFilter(o, e);
}

void QAlphaWidget::closeEvent(QCloseEvent *e)
{
   e->accept();
   if (!q_blend) {
      return;
   }

   showWidget = false;
   render();

   QWidget::closeEvent(e);
}

void QAlphaWidget::render()
{
   int tempel = checkTime.elapsed();
   if (elapsed >= tempel) {
      elapsed++;
   } else {
      elapsed = tempel;
   }

   if (duration != 0) {
      alpha = tempel / double(duration);
   } else {
      alpha = 1;
   }

#if defined(Q_OS_WIN)
   if (alpha >= 1 || ! showWidget) {
      anim.stop();
      qApp->removeEventFilter(this);
      widget->setWindowOpacity(1);

      q_blend = nullptr;
      deleteLater();

   } else {
      widget->setWindowOpacity(alpha);
   }

#else
   if (alpha >= 1 || ! showWidget) {
      anim.stop();
      qApp->removeEventFilter(this);

      if (widget != nullptr) {

         if (! showWidget) {
            widget->hide();

         } else {
            // since we are faking the visibility of the widget
            // we need to unset the hidden state on it before calling show
            widget->setAttribute(Qt::WA_WState_Hidden, true);
            widget->show();
            lower();
         }
      }

      q_blend = nullptr;
      deleteLater();

   } else {
      alphaBlend();
      pm = QPixmap::fromImage(mixedImage);
      repaint();
   }
#endif
}

void QAlphaWidget::alphaBlend()
{
   const int a = qRound(alpha * 256);
   const int ia = 256 - a;

   const int sw = frontImage.width();
   const int sh = frontImage.height();
   const int bpl = frontImage.bytesPerLine();

   switch (frontImage.depth()) {
      case 32: {
         uchar *mixed_data = mixedImage.bits();
         const uchar *back_data = backImage.bits();
         const uchar *front_data = frontImage.bits();

         for (int sy = 0; sy < sh; sy++) {
            quint32 *mixed = (quint32 *)mixed_data;
            const quint32 *back = (const quint32 *)back_data;
            const quint32 *front = (const quint32 *)front_data;
            for (int sx = 0; sx < sw; sx++) {
               quint32 bp = back[sx];
               quint32 fp = front[sx];

               mixed[sx] =  qRgb((qRed(bp) * ia + qRed(fp) * a) >> 8,
                     (qGreen(bp) * ia + qGreen(fp) * a) >> 8,
                     (qBlue(bp) * ia + qBlue(fp) * a) >> 8);
            }
            mixed_data += bpl;
            back_data += bpl;
            front_data += bpl;
         }
      }
      default:
         break;
   }
}

class QRollEffect : public QWidget, private QEffects
{
   GUI_CS_OBJECT(QRollEffect)

 public:
   QRollEffect(QWidget *w, Qt::WindowFlags flags, DirFlags orient);

   void run(int time);

 protected:
   void paintEvent(QPaintEvent *)  override;
   void closeEvent(QCloseEvent *)  override;

 private:
   GUI_CS_SLOT_1(Private, void scroll())
   GUI_CS_SLOT_2(scroll)

   QPointer<QWidget> widget;

   int currentHeight;
   int currentWidth;
   int totalHeight;
   int totalWidth;

   int duration;
   int elapsed;
   bool done;
   bool showWidget;
   int orientation;

   QTimer anim;
   QElapsedTimer checkTime;

   QPixmap pm;
};

static QRollEffect *q_roll = nullptr;

QRollEffect::QRollEffect(QWidget *w, Qt::WindowFlags flags, DirFlags orient)
   : QWidget(nullptr, flags), orientation(orient)
{
#ifndef Q_OS_WIN
   setEnabled(false);
#endif

   widget = w;
   Q_ASSERT(widget);

   setAttribute(Qt::WA_NoSystemBackground, true);

   if (widget->testAttribute(Qt::WA_Resized)) {
      totalWidth = widget->width();
      totalHeight = widget->height();
   } else {
      totalWidth = widget->sizeHint().width();
      totalHeight = widget->sizeHint().height();
   }

   currentHeight = totalHeight;
   currentWidth = totalWidth;

   if (orientation & (RightScroll | LeftScroll)) {
      currentWidth = 0;
   }

   if (orientation & (DownScroll | UpScroll)) {
      currentHeight = 0;
   }

   pm = widget->grab();
}

void QRollEffect::paintEvent(QPaintEvent *)
{
   int x = orientation & RightScroll ? qMin(0, currentWidth - totalWidth) : 0;
   int y = orientation & DownScroll ? qMin(0, currentHeight - totalHeight) : 0;

   QPainter p(this);
   p.drawPixmap(x, y, pm);
}

void QRollEffect::closeEvent(QCloseEvent *e)
{
   e->accept();
   if (done) {
      return;
   }

   showWidget = false;
   done = true;
   scroll();

   QWidget::closeEvent(e);
}

void QRollEffect::run(int time)
{
   if (!widget) {
      return;
   }

   duration  = time;
   elapsed = 0;

   if (duration < 0) {
      int dist = 0;
      if (orientation & (RightScroll | LeftScroll)) {
         dist += totalWidth - currentWidth;
      }

      if (orientation & (DownScroll | UpScroll)) {
         dist += totalHeight - currentHeight;
      }

      duration = qMin(qMax(dist / 3, 50), 120);
   }

   connect(&anim, &QTimer::timeout, this, &QRollEffect::scroll);

   move(widget->geometry().x(), widget->geometry().y());
   resize(qMin(currentWidth, totalWidth), qMin(currentHeight, totalHeight));

   //This is roughly equivalent to calling setVisible(true) without actually showing the widget
   widget->setAttribute(Qt::WA_WState_ExplicitShowHide, true);
   widget->setAttribute(Qt::WA_WState_Hidden, false);

   show();
   setEnabled(false);

   showWidget = true;
   done = false;
   anim.start(1);
   checkTime.start();
}

void QRollEffect::scroll()
{
   if (!done && widget) {
      int tempel = checkTime.elapsed();
      if (elapsed >= tempel) {
         elapsed++;
      } else {
         elapsed = tempel;
      }

      if (currentWidth != totalWidth) {
         currentWidth = totalWidth * (elapsed / duration)
            + (2 * totalWidth * (elapsed % duration) + duration)
            / (2 * duration);
         // equiv. to int((totalWidth*elapsed) / duration + 0.5)
         done = (currentWidth >= totalWidth);
      }
      if (currentHeight != totalHeight) {
         currentHeight = totalHeight * (elapsed / duration)
            + (2 * totalHeight * (elapsed % duration) + duration)
            / (2 * duration);
         // equiv. to int((totalHeight*elapsed) / duration + 0.5)
         done = (currentHeight >= totalHeight);
      }
      done = (currentHeight >= totalHeight) &&
         (currentWidth >= totalWidth);

      int w = totalWidth;
      int h = totalHeight;
      int x = widget->geometry().x();
      int y = widget->geometry().y();

      if (orientation & RightScroll || orientation & LeftScroll) {
         w = qMin(currentWidth, totalWidth);
      }
      if (orientation & DownScroll || orientation & UpScroll) {
         h = qMin(currentHeight, totalHeight);
      }

      setUpdatesEnabled(false);
      if (orientation & UpScroll) {
         y = widget->geometry().y() + qMax(0, totalHeight - currentHeight);
      }
      if (orientation & LeftScroll) {
         x = widget->geometry().x() + qMax(0, totalWidth - currentWidth);
      }
      if (orientation & UpScroll || orientation & LeftScroll) {
         move(x, y);
      }

      resize(w, h);
      setUpdatesEnabled(true);
      repaint();
   }
   if (done || !widget) {
      anim.stop();

      if (widget) {
         if (! showWidget) {
#ifdef Q_OS_WIN
            setEnabled(true);
            setFocus();
#endif
            widget->hide();

         } else {
            //Since we are faking the visibility of the widget
            //we need to unset the hidden state on it before calling show
            widget->setAttribute(Qt::WA_WState_Hidden, true);
            widget->show();
            lower();
         }
      }

      q_roll = nullptr;
      deleteLater();
   }
}

void qScrollEffect(QWidget *w, QEffects::DirFlags orient, int time)
{
   if (q_roll) {
      q_roll->deleteLater();
      q_roll = nullptr;
   }

   if (! w) {
      return;
   }

   QApplication::sendPostedEvents(w, QEvent::Move);
   QApplication::sendPostedEvents(w, QEvent::Resize);
   Qt::WindowFlags flags = Qt::ToolTip;

   // those can be popups - they would steal the focus, but are disabled
   q_roll = new QRollEffect(w, flags, orient);
   q_roll->run(time);
}

void qFadeEffect(QWidget *w, int time)
{
   if (q_blend) {
      q_blend->deleteLater();
      q_blend = nullptr;
   }

   if (!w) {
      return;
   }

   QApplication::sendPostedEvents(w, QEvent::Move);
   QApplication::sendPostedEvents(w, QEvent::Resize);

   Qt::WindowFlags flags = Qt::ToolTip;

   // those can be popups - they would steal the focus, but are disabled
   q_blend = new QAlphaWidget(w, flags);

   q_blend->run(time);
}

#endif //QT_NO_EFFECTS
