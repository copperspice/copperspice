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

#include <qframe.h>
#include <qbitmap.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qapplication.h>
#include <qframe_p.h>

QT_BEGIN_NAMESPACE

QFramePrivate::QFramePrivate()
   : frect(QRect(0, 0, 0, 0)),
     frameStyle(QFrame::NoFrame | QFrame::Plain),
     lineWidth(1),
     midLineWidth(0),
     frameWidth(0),
     leftFrameWidth(0), rightFrameWidth(0),
     topFrameWidth(0), bottomFrameWidth(0)
{
}

inline void QFramePrivate::init()
{
   setLayoutItemMargins(QStyle::SE_FrameLayoutItem);
}

QFrame::QFrame(QWidget *parent, Qt::WindowFlags f)
   : QWidget(*new QFramePrivate, parent, f)
{
   Q_D(QFrame);
   d->init();
}

QFrame::QFrame(QFramePrivate &dd, QWidget *parent, Qt::WindowFlags f)
   : QWidget(dd, parent, f)
{
   Q_D(QFrame);
   d->init();
}

QFrame::~QFrame()
{
}

int QFrame::frameStyle() const
{
   Q_D(const QFrame);
   return d->frameStyle;
}

QFrame::Shape QFrame::frameShape() const
{
   Q_D(const QFrame);
   return (Shape) (d->frameStyle & Shape_Mask);
}

void QFrame::setFrameShape(QFrame::Shape s)
{
   Q_D(QFrame);
   setFrameStyle((d->frameStyle & Shadow_Mask) | s);
}


/*!
    \property QFrame::frameShadow
    \brief the frame shadow value from the frame style

    \sa frameStyle(), frameShape()
*/
QFrame::Shadow QFrame::frameShadow() const
{
   Q_D(const QFrame);
   return (Shadow) (d->frameStyle & Shadow_Mask);
}

void QFrame::setFrameShadow(QFrame::Shadow s)
{
   Q_D(QFrame);
   setFrameStyle((d->frameStyle & Shape_Mask) | s);
}

/*!
    Sets the frame style to \a style.

    The \a style is the bitwise OR between a frame shape and a frame
    shadow style. See the picture of the frames in the main class
    documentation.

    The frame shapes are given in \l{QFrame::Shape} and the shadow
    styles in \l{QFrame::Shadow}.

    If a mid-line width greater than 0 is specified, an additional
    line is drawn for \l Raised or \l Sunken \l Box, \l HLine, and \l
    VLine frames. The mid-color of the current color group is used for
    drawing middle lines.

    \sa frameStyle()
*/

void QFrame::setFrameStyle(int style)
{
   Q_D(QFrame);
   if (!testAttribute(Qt::WA_WState_OwnSizePolicy)) {
      QSizePolicy sp;

      switch (style & Shape_Mask) {
         case HLine:
            sp = QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed, QSizePolicy::Line);
            break;
         case VLine:
            sp = QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum, QSizePolicy::Line);
            break;
         default:
            sp = QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred, QSizePolicy::Frame);
      }
      setSizePolicy(sp);
      setAttribute(Qt::WA_WState_OwnSizePolicy, false);
   }
   d->frameStyle = (short)style;
   update();
   d->updateFrameWidth();
}

/*!
    \property QFrame::lineWidth
    \brief the line width

    Note that the \e total line width for frames used as separators
    (\l HLine and \l VLine) is specified by \l frameWidth.

    The default value is 1.

    \sa midLineWidth, frameWidth
*/

void QFrame::setLineWidth(int w)
{
   Q_D(QFrame);
   if (short(w) == d->lineWidth) {
      return;
   }
   d->lineWidth = short(w);
   d->updateFrameWidth();
}

int QFrame::lineWidth() const
{
   Q_D(const QFrame);
   return d->lineWidth;
}

/*!
    \property QFrame::midLineWidth
    \brief the width of the mid-line

    The default value is 0.

    \sa lineWidth, frameWidth
*/

void QFrame::setMidLineWidth(int w)
{
   Q_D(QFrame);
   if (short(w) == d->midLineWidth) {
      return;
   }
   d->midLineWidth = short(w);
   d->updateFrameWidth();
}

int QFrame::midLineWidth() const
{
   Q_D(const QFrame);
   return d->midLineWidth;
}

/*!
  \internal
  Updates the frame widths from the style.
*/
void QFramePrivate::updateStyledFrameWidths()
{
   Q_Q(const QFrame);
   QStyleOptionFrameV3 opt;
   opt.initFrom(q);
   opt.lineWidth = lineWidth;
   opt.midLineWidth = midLineWidth;
   opt.frameShape = QFrame::Shape(frameStyle & QFrame::Shape_Mask);

   QRect cr = q->style()->subElementRect(QStyle::SE_ShapedFrameContents, &opt, q);
   leftFrameWidth = cr.left() - opt.rect.left();
   topFrameWidth = cr.top() - opt.rect.top();
   rightFrameWidth = opt.rect.right() - cr.right(),
   bottomFrameWidth = opt.rect.bottom() - cr.bottom();
   frameWidth = qMax(qMax(leftFrameWidth, rightFrameWidth),
                     qMax(topFrameWidth, bottomFrameWidth));
}

/*!
  \internal
  Updated the frameWidth parameter.
*/

void QFramePrivate::updateFrameWidth()
{
   Q_Q(QFrame);
   QRect fr = q->frameRect();
   updateStyledFrameWidths();
   q->setFrameRect(fr);
   setLayoutItemMargins(QStyle::SE_FrameLayoutItem);
}

/*!
    \property QFrame::frameWidth
    \brief the width of the frame that is drawn.

    Note that the frame width depends on the \l{QFrame::setFrameStyle()}{frame style},
    not only the line width and the mid-line width. For example, the style specified
    by \l NoFrame always has a frame width of 0, whereas the style \l Panel has a
    frame width equivalent to the line width.

    \sa lineWidth(), midLineWidth(), frameStyle()
*/
int QFrame::frameWidth() const
{
   Q_D(const QFrame);
   return d->frameWidth;
}


/*!
    \property QFrame::frameRect
    \brief the frame's rectangle

    The frame's rectangle is the rectangle the frame is drawn in. By
    default, this is the entire widget. Setting the rectangle does
    does \e not cause a widget update. The frame rectangle is
    automatically adjusted when the widget changes size.

    If you set the rectangle to a null rectangle (for example,
    QRect(0, 0, 0, 0)), then the resulting frame rectangle is
    equivalent to the \link QWidget::rect() widget rectangle\endlink.
*/

QRect QFrame::frameRect() const
{
   Q_D(const QFrame);
   QRect fr = contentsRect();
   fr.adjust(-d->leftFrameWidth, -d->topFrameWidth, d->rightFrameWidth, d->bottomFrameWidth);
   return fr;
}

void QFrame::setFrameRect(const QRect &r)
{
   Q_D(QFrame);
   QRect cr = r.isValid() ? r : rect();
   cr.adjust(d->leftFrameWidth, d->topFrameWidth, -d->rightFrameWidth, -d->bottomFrameWidth);
   setContentsMargins(cr.left(), cr.top(), rect().right() - cr.right(), rect().bottom() - cr.bottom());
}

/*!\reimp
*/
QSize QFrame::sizeHint() const
{
   Q_D(const QFrame);
   //   Returns a size hint for the frame - for HLine and VLine
   //   shapes, this is stretchable one way and 3 pixels wide the
   //   other.  For other shapes, QWidget::sizeHint() is used.
   switch (d->frameStyle & Shape_Mask) {
      case HLine:
         return QSize(-1, 3);
      case VLine:
         return QSize(3, -1);
      default:
         return QWidget::sizeHint();
   }
}

/*!\reimp
*/

void QFrame::paintEvent(QPaintEvent *)
{
   QPainter paint(this);
   drawFrame(&paint);
}

/*!
    \internal
*/
void QFrame::drawFrame(QPainter *p)
{
   Q_D(QFrame);
   QStyleOptionFrameV3 opt;
   opt.init(this);
   int frameShape  = d->frameStyle & QFrame::Shape_Mask;
   int frameShadow = d->frameStyle & QFrame::Shadow_Mask;
   opt.frameShape = Shape(int(opt.frameShape) | frameShape);
   opt.rect = frameRect();

   switch (frameShape) {
      case QFrame::Box:
      case QFrame::HLine:
      case QFrame::VLine:
      case QFrame::StyledPanel:
      case QFrame::Panel:
         opt.lineWidth = d->lineWidth;
         opt.midLineWidth = d->midLineWidth;
         break;
      default:
         // most frame styles do not handle customized line and midline widths
         // (see updateFrameWidth()).
         opt.lineWidth = d->frameWidth;
         break;
   }

   if (frameShadow == Sunken) {
      opt.state |= QStyle::State_Sunken;
   } else if (frameShadow == Raised) {
      opt.state |= QStyle::State_Raised;
   }

   style()->drawControl(QStyle::CE_ShapedFrame, &opt, p, this);
}


/*!\reimp
 */
void QFrame::changeEvent(QEvent *ev)
{
   Q_D(QFrame);

   if (ev->type() == QEvent::StyleChange
#ifdef Q_OS_MAC
         || ev->type() == QEvent::MacSizeChange
#endif
      ) {
      d->updateFrameWidth();
   }
   QWidget::changeEvent(ev);
}

/*! \reimp */
bool QFrame::event(QEvent *e)
{
   if (e->type() == QEvent::ParentChange) {
      d_func()->updateFrameWidth();
   }
   bool result = QWidget::event(e);
   //this has to be done after the widget has been polished
   if (e->type() == QEvent::Polish) {
      d_func()->updateFrameWidth();
   }
   return result;
}

QT_END_NAMESPACE
