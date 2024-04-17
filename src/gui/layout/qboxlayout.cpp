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

#include <qboxlayout.h>

#include <qapplication.h>
#include <qlist.h>
#include <qsizepolicy.h>
#include <qvector.h>
#include <qwidget.h>

#include <qlayoutengine_p.h>
#include <qlayout_p.h>

struct QBoxLayoutItem {
   QBoxLayoutItem(QLayoutItem *it, int stretchX = 0)
      : item(it), stretch(stretchX), magic(false)
   { }

   ~QBoxLayoutItem() {
      delete item;
   }

   int hfw(int w) {
      if (item->hasHeightForWidth()) {
         return item->heightForWidth(w);
      } else {
         return item->sizeHint().height();
      }
   }

   int mhfw(int w) {
      if (item->hasHeightForWidth()) {
         return item->heightForWidth(w);
      } else {
         return item->minimumSize().height();
      }
   }

   int hStretch() {
      if (stretch == 0 && item->widget()) {
         return item->widget()->sizePolicy().horizontalStretch();
      } else {
         return stretch;
      }
   }

   int vStretch() {
      if (stretch == 0 && item->widget()) {
         return item->widget()->sizePolicy().verticalStretch();
      } else {
         return stretch;
      }
   }

   QLayoutItem *item;
   int stretch;
   bool magic;
};

class QBoxLayoutPrivate : public QLayoutPrivate
{
   Q_DECLARE_PUBLIC(QBoxLayout)

 public:
   QBoxLayoutPrivate() : hfwWidth(-1), dirty(true), spacing(-1) { }
   ~QBoxLayoutPrivate();

   void setDirty() {
      geomArray.clear();
      hfwWidth = -1;
      hfwHeight = -1;
      dirty = true;
   }

   QList<QBoxLayoutItem *> list;
   QVector<QLayoutStruct> geomArray;
   int hfwWidth;
   int hfwHeight;
   int hfwMinHeight;
   QSize sizeHint;
   QSize minSize;
   QSize maxSize;
   int leftMargin, topMargin, rightMargin, bottomMargin;
   Qt::Orientations expanding;
   uint hasHfw : 1;
   uint dirty : 1;
   QBoxLayout::Direction dir;
   int spacing;

   void deleteAll() {
      while (! list.isEmpty()) {
         delete list.takeFirst();
      }
   }

   void setupGeom();
   void calcHfw(int);

   void effectiveMargins(int *left, int *top, int *right, int *bottom) const;
   QLayoutItem *replaceAt(int index, QLayoutItem *) override;
};

QBoxLayoutPrivate::~QBoxLayoutPrivate()
{
}

static inline bool horz(QBoxLayout::Direction dir)
{
   return dir == QBoxLayout::RightToLeft || dir == QBoxLayout::LeftToRight;
}

void QBoxLayoutPrivate::effectiveMargins(int *left, int *top, int *right, int *bottom) const
{
   int l = leftMargin;
   int t = topMargin;
   int r = rightMargin;
   int b = bottomMargin;

#ifdef Q_OS_DARWIN
   Q_Q(const QBoxLayout);

   if (horz(dir)) {
      QBoxLayoutItem *leftBox  = nullptr;
      QBoxLayoutItem *rightBox = nullptr;

      if (left || right) {
         leftBox = list.value(0);
         rightBox = list.value(list.count() - 1);

         if (dir == QBoxLayout::RightToLeft) {
            qSwap(leftBox, rightBox);
         }

         int leftDelta = 0;
         int rightDelta = 0;
         if (leftBox) {
            QLayoutItem *itm = leftBox->item;
            if (QWidget *w = itm->widget()) {
               leftDelta = itm->geometry().left() - w->geometry().left();
            }
         }
         if (rightBox) {
            QLayoutItem *itm = rightBox->item;
            if (QWidget *w = itm->widget()) {
               rightDelta = w->geometry().right() - itm->geometry().right();
            }
         }
         QWidget *w = q->parentWidget();
         Qt::LayoutDirection layoutDirection = w ? w->layoutDirection() : QApplication::layoutDirection();
         if (layoutDirection == Qt::RightToLeft) {
            qSwap(leftDelta, rightDelta);
         }

         l = qMax(l, leftDelta);
         r = qMax(r, rightDelta);
      }

      int count = top || bottom ? list.count() : 0;
      for (int i = 0; i < count; ++i) {
         QBoxLayoutItem *box = list.at(i);
         QLayoutItem *itm = box->item;
         QWidget *w = itm->widget();
         if (w) {
            QRect lir = itm->geometry();
            QRect wr = w->geometry();
            if (top) {
               t = qMax(t, lir.top() - wr.top());
            }
            if (bottom) {
               b = qMax(b, wr.bottom() - lir.bottom());
            }
         }
      }

   } else {
      // vertical layout
      QBoxLayoutItem *topBox    = nullptr;
      QBoxLayoutItem *bottomBox = nullptr;

      if (top || bottom) {
         topBox = list.value(0);
         bottomBox = list.value(list.count() - 1);
         if (dir == QBoxLayout::BottomToTop) {
            qSwap(topBox, bottomBox);
         }

         if (top && topBox) {
            QLayoutItem *itm = topBox->item;
            QWidget *w = itm->widget();
            if (w) {
               t = qMax(t, itm->geometry().top() - w->geometry().top());
            }
         }

         if (bottom && bottomBox) {
            QLayoutItem *itm = bottomBox->item;
            QWidget *w = itm->widget();
            if (w) {
               b = qMax(b, w->geometry().bottom() - itm->geometry().bottom());
            }
         }
      }

      int count = left || right ? list.count() : 0;
      for (int i = 0; i < count; ++i) {
         QBoxLayoutItem *box = list.at(i);
         QLayoutItem *itm = box->item;
         QWidget *w = itm->widget();
         if (w) {
            QRect lir = itm->geometry();
            QRect wr = w->geometry();
            if (left) {
               l = qMax(l, lir.left() - wr.left());
            }
            if (right) {
               r = qMax(r, wr.right() - lir.right());
            }
         }
      }
   }
#endif
   if (left) {
      *left = l;
   }
   if (top) {
      *top = t;
   }
   if (right) {
      *right = r;
   }
   if (bottom) {
      *bottom = b;
   }
}

void QBoxLayoutPrivate::setupGeom()
{
   if (! dirty) {
      return;
   }

   Q_Q(QBoxLayout);
   int maxw  = horz(dir) ? 0 : QLAYOUTSIZE_MAX;
   int maxh  = horz(dir) ? QLAYOUTSIZE_MAX : 0;
   int minw  = 0;
   int minh  = 0;
   int hintw = 0;
   int hinth = 0;

   bool horexp = false;
   bool verexp = false;

   hasHfw = false;

   int n = list.count();
   geomArray.clear();

   QVector<QLayoutStruct> a(n);

   QSizePolicy::ControlTypes controlTypes1;
   QSizePolicy::ControlTypes controlTypes2;

   int fixedSpacing = q->spacing();
   int previousNonEmptyIndex = -1;

   QStyle *style = nullptr;
   if (fixedSpacing < 0) {
      if (QWidget *parentWidget = q->parentWidget()) {
         style = parentWidget->style();
      }
   }

   for (int i = 0; i < n; i++) {
      QBoxLayoutItem *box = list.at(i);
      QSize max  = box->item->maximumSize();
      QSize min  = box->item->minimumSize();
      QSize hint = box->item->sizeHint();
      Qt::Orientations exp = box->item->expandingDirections();

      bool empty  = box->item->isEmpty();
      int spacing = 0;

      if (! empty) {
         if (fixedSpacing >= 0) {
            spacing = (previousNonEmptyIndex >= 0) ? fixedSpacing : 0;

#ifdef Q_OS_DARWIN
            if (! horz(dir) && previousNonEmptyIndex >= 0) {
               QBoxLayoutItem *sibling = (dir == QBoxLayout::TopToBottom  ? box : list.at(previousNonEmptyIndex));

               if (sibling) {
                  QWidget *wid = sibling->item->widget();

                  if (wid) {
                     spacing = qMax(spacing, sibling->item->geometry().top() - wid->geometry().top());
                  }
               }
            }
#endif

         } else {
            controlTypes1 = controlTypes2;
            controlTypes2 = box->item->controlTypes();

            if (previousNonEmptyIndex >= 0) {
               QSizePolicy::ControlTypes actual1 = controlTypes1;
               QSizePolicy::ControlTypes actual2 = controlTypes2;

               if (dir == QBoxLayout::RightToLeft || dir == QBoxLayout::BottomToTop) {
                  qSwap(actual1, actual2);
               }

               if (style) {
                  spacing = style->combinedLayoutSpacing(actual1, actual2,
                        horz(dir) ? Qt::Horizontal : Qt::Vertical, nullptr, q->parentWidget());

                  if (spacing < 0) {
                     spacing = 0;
                  }
               }
            }
         }

         if (previousNonEmptyIndex >= 0) {
            a[previousNonEmptyIndex].spacing = spacing;
         }
         previousNonEmptyIndex = i;
      }

      bool ignore = empty && box->item->widget(); // ignore hidden widgets
      bool dummy  = true;

      if (horz(dir)) {
         bool expand = (exp & Qt::Horizontal || box->stretch > 0);
         horexp  = horexp || expand;
         maxw   += spacing + max.width();
         minw   += spacing + min.width();
         hintw  += spacing + hint.width();

         if (! ignore) {
            qMaxExpCalc(maxh, verexp, dummy, max.height(), exp & Qt::Vertical, box->item->isEmpty());
         }

         minh  = qMax(minh, min.height());
         hinth = qMax(hinth, hint.height());

         a[i].sizeHint    = hint.width();
         a[i].maximumSize = max.width();
         a[i].minimumSize = min.width();
         a[i].expansive   = expand;
         a[i].stretch     = box->stretch ? box->stretch : box->hStretch();

      } else {
         bool expand = (exp & Qt::Vertical || box->stretch > 0);
         verexp  = verexp || expand;
         maxh   += spacing + max.height();
         minh   += spacing + min.height();
         hinth  += spacing + hint.height();

         if (! ignore) {
            qMaxExpCalc(maxw, horexp, dummy, max.width(), exp & Qt::Horizontal, box->item->isEmpty());
         }

         minw  = qMax(minw, min.width());
         hintw = qMax(hintw, hint.width());

         a[i].sizeHint = hint.height();
         a[i].maximumSize = max.height();
         a[i].minimumSize = min.height();
         a[i].expansive = expand;
         a[i].stretch = box->stretch ? box->stretch : box->vStretch();
      }

      a[i].empty = empty;
      a[i].spacing = 0;   // might be be initialized with a non-zero value in a later iteration
      hasHfw = hasHfw || box->item->hasHeightForWidth();
   }

   geomArray = a;

   expanding = (Qt::Orientations) ((horexp ? Qt::Horizontal : 0) | (verexp ? Qt::Vertical : 0));

   minSize = QSize(minw, minh);
   maxSize = QSize(maxw, maxh).expandedTo(minSize);
   sizeHint = QSize(hintw, hinth).expandedTo(minSize).boundedTo(maxSize);

   q->getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

   int left, top, right, bottom;
   effectiveMargins(&left, &top, &right, &bottom);
   QSize extra(left + right, top + bottom);

   minSize  += extra;
   maxSize  += extra;
   sizeHint += extra;

   dirty = false;
}

void QBoxLayoutPrivate::calcHfw(int w)
{
   QVector<QLayoutStruct> &a = geomArray;

   int n  = a.count();
   int h  = 0;
   int mh = 0;

   Q_ASSERT(n == list.size());

   if (horz(dir)) {
      qGeomCalc(a, 0, n, 0, w);
      for (int i = 0; i < n; i++) {
         QBoxLayoutItem *box = list.at(i);
         h = qMax(h, box->hfw(a.at(i).size));
         mh = qMax(mh, box->mhfw(a.at(i).size));
      }
   } else {
      for (int i = 0; i < n; ++i) {
         QBoxLayoutItem *box = list.at(i);
         int spacing = a.at(i).spacing;
         h += box->hfw(w);
         mh += box->mhfw(w);
         h += spacing;
         mh += spacing;
      }
   }
   hfwWidth = w;
   hfwHeight = h;
   hfwMinHeight = mh;
}

QLayoutItem *QBoxLayoutPrivate::replaceAt(int index, QLayoutItem *item)
{
   Q_Q(QBoxLayout);

   if (!item) {
      return nullptr;
   }

   QBoxLayoutItem *b = list.value(index);
   if (!b) {
      return nullptr;
   }

   QLayoutItem *r = b->item;

   b->item = item;
   q->invalidate();
   return r;
}

QBoxLayout::QBoxLayout(Direction dir, QWidget *parent)
   : QLayout(*new QBoxLayoutPrivate, nullptr, parent)
{
   Q_D(QBoxLayout);
   d->dir = dir;
}

QBoxLayout::~QBoxLayout()
{
   Q_D(QBoxLayout);
   d->deleteAll(); // must do it before QObject deletes children, so can't be in ~QBoxLayoutPrivate
}

int QBoxLayout::spacing() const
{
   Q_D(const QBoxLayout);

   if (d->spacing >= 0) {
      return d->spacing;

   } else {
      return qSmartSpacing(this, d->dir == LeftToRight || d->dir == RightToLeft
            ? QStyle::PM_LayoutHorizontalSpacing
            : QStyle::PM_LayoutVerticalSpacing);
   }
}

void QBoxLayout::setSpacing(int spacing)
{
   Q_D(QBoxLayout);
   d->spacing = spacing;
   invalidate();
}

QSize QBoxLayout::sizeHint() const
{
   Q_D(const QBoxLayout);
   if (d->dirty) {
      const_cast<QBoxLayout *>(this)->d_func()->setupGeom();
   }
   return d->sizeHint;
}

QSize QBoxLayout::minimumSize() const
{
   Q_D(const QBoxLayout);
   if (d->dirty) {
      const_cast<QBoxLayout *>(this)->d_func()->setupGeom();
   }
   return d->minSize;
}

QSize QBoxLayout::maximumSize() const
{
   Q_D(const QBoxLayout);
   if (d->dirty) {
      const_cast<QBoxLayout *>(this)->d_func()->setupGeom();
   }

   QSize s = d->maxSize.boundedTo(QSize(QLAYOUTSIZE_MAX, QLAYOUTSIZE_MAX));

   if (alignment() & Qt::AlignHorizontal_Mask) {
      s.setWidth(QLAYOUTSIZE_MAX);
   }

   if (alignment() & Qt::AlignVertical_Mask) {
      s.setHeight(QLAYOUTSIZE_MAX);
   }
   return s;
}

bool QBoxLayout::hasHeightForWidth() const
{
   Q_D(const QBoxLayout);
   if (d->dirty) {
      const_cast<QBoxLayout *>(this)->d_func()->setupGeom();
   }
   return d->hasHfw;
}

int QBoxLayout::heightForWidth(int w) const
{
   Q_D(const QBoxLayout);
   if (!hasHeightForWidth()) {
      return -1;
   }

   int left, top, right, bottom;
   d->effectiveMargins(&left, &top, &right, &bottom);

   w -= left + right;
   if (w != d->hfwWidth) {
      const_cast<QBoxLayout *>(this)->d_func()->calcHfw(w);
   }

   return d->hfwHeight + top + bottom;
}

int QBoxLayout::minimumHeightForWidth(int w) const
{
   Q_D(const QBoxLayout);

   // return value is unused
   (void) heightForWidth(w);

   int top, bottom;
   d->effectiveMargins(nullptr, &top, nullptr, &bottom);

   return d->hasHfw ? (d->hfwMinHeight + top + bottom) : -1;
}

void QBoxLayout::invalidate()
{
   Q_D(QBoxLayout);
   d->setDirty();
   QLayout::invalidate();
}

int QBoxLayout::count() const
{
   Q_D(const QBoxLayout);
   return d->list.count();
}

QLayoutItem *QBoxLayout::itemAt(int index) const
{
   Q_D(const QBoxLayout);
   return index >= 0 && index < d->list.count() ? d->list.at(index)->item : nullptr;
}

QLayoutItem *QBoxLayout::takeAt(int index)
{
   Q_D(QBoxLayout);
   if (index < 0 || index >= d->list.count()) {
      return nullptr;
   }

   QBoxLayoutItem *b = d->list.takeAt(index);
   QLayoutItem *item = b->item;

   b->item = nullptr;
   delete b;

   if (QLayout *l = item->layout()) {
      // sanity check in case the user passed something weird to QObject::setParent()
      if (l->parent() == this) {
         l->setParent(nullptr);
      }
   }

   invalidate();
   return item;
}

Qt::Orientations QBoxLayout::expandingDirections() const
{
   Q_D(const QBoxLayout);
   if (d->dirty) {
      const_cast<QBoxLayout *>(this)->d_func()->setupGeom();
   }
   return d->expanding;
}

void QBoxLayout::setGeometry(const QRect &r)
{
   Q_D(QBoxLayout);

   if (d->dirty || r != geometry()) {
      QRect oldRect = geometry();

      QLayout::setGeometry(r);
      if (d->dirty) {
         d->setupGeom();
      }

      QRect cr = alignment() ? alignmentRect(r) : r;

      int left, top, right, bottom;
      d->effectiveMargins(&left, &top, &right, &bottom);

      QRect s(cr.x() + left, cr.y() + top, cr.width() - (left + right), cr.height() - (top + bottom));

      QVector<QLayoutStruct> a = d->geomArray;

      int pos   = horz(d->dir) ? s.x() : s.y();
      int space = horz(d->dir) ? s.width() : s.height();
      int n = a.count();

      if (d->hasHfw && ! horz(d->dir)) {

         for (int i = 0; i < n; i++) {
            QBoxLayoutItem *box = d->list.at(i);

            if (box->item->hasHeightForWidth()) {
               int width = qBound(box->item->minimumSize().width(), s.width(), box->item->maximumSize().width());
               a[i].sizeHint = a[i].minimumSize = box->item->heightForWidth(width);
            }
         }
      }

      Direction visualDir = d->dir;
      QWidget *parent = parentWidget();

      if (parent && parent->isRightToLeft()) {
         if (d->dir == LeftToRight) {
            visualDir = RightToLeft;

         } else if (d->dir == RightToLeft) {
            visualDir = LeftToRight;
         }
      }

      qGeomCalc(a, 0, n, pos, space);

      bool reverse = (horz(visualDir) ? ((r.right() > oldRect.right()) != (visualDir == RightToLeft))
            : r.bottom() > oldRect.bottom());

      for (int j = 0; j < n; j++) {

         int i = reverse ? n - j - 1 : j;
         QBoxLayoutItem *box = d->list.at(i);

         switch (visualDir) {
            case LeftToRight:
               box->item->setGeometry(QRect(a.at(i).pos, s.y(), a.at(i).size, s.height()));
               break;

            case RightToLeft:
               box->item->setGeometry(QRect(s.left() + s.right() - a.at(i).pos - a.at(i).size + 1,
                     s.y(), a.at(i).size, s.height()));
               break;

            case TopToBottom:
               box->item->setGeometry(QRect(s.x(), a.at(i).pos, s.width(), a.at(i).size));
               break;


            case BottomToTop:
               box->item->setGeometry(QRect(s.x(), s.top() + s.bottom() - a.at(i).pos - a.at(i).size + 1,
                     s.width(), a.at(i).size));
         }
      }
   }
}

void QBoxLayout::addItem(QLayoutItem *item)
{
   Q_D(QBoxLayout);
   QBoxLayoutItem *it = new QBoxLayoutItem(item);
   d->list.append(it);
   invalidate();
}

void QBoxLayout::insertItem(int index, QLayoutItem *item)
{
   Q_D(QBoxLayout);

   // append
   if (index < 0) {
      index = d->list.count();
   }

   QBoxLayoutItem *it = new QBoxLayoutItem(item);
   d->list.insert(index, it);
   invalidate();
}

void QBoxLayout::insertSpacing(int index, int size)
{
   Q_D(QBoxLayout);

   // append
   if (index < 0) {
      index = d->list.count();
   }

   QLayoutItem *b;
   if (horz(d->dir)) {
      b = QLayoutPrivate::createSpacerItem(this, size, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
   } else {
      b = QLayoutPrivate::createSpacerItem(this, 0, size, QSizePolicy::Minimum, QSizePolicy::Fixed);
   }

   try {
      QBoxLayoutItem *it = new QBoxLayoutItem(b);
      it->magic = true;
      d->list.insert(index, it);

   } catch (...) {
      delete b;
      throw;
   }

   invalidate();
}

void QBoxLayout::insertStretch(int index, int stretch)
{
   Q_D(QBoxLayout);

   // append
   if (index < 0) {
      index = d->list.count();
   }

   QLayoutItem *b;
   if (horz(d->dir)) {
      b = QLayoutPrivate::createSpacerItem(this, 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
   } else {
      b = QLayoutPrivate::createSpacerItem(this, 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
   }

   QBoxLayoutItem *it = new QBoxLayoutItem(b, stretch);
   it->magic = true;
   d->list.insert(index, it);
   invalidate();
}

void QBoxLayout::insertSpacerItem(int index, QSpacerItem *spacerItem)
{
   Q_D(QBoxLayout);
   if (index < 0) {                              // append
      index = d->list.count();
   }

   QBoxLayoutItem *it = new QBoxLayoutItem(spacerItem);
   it->magic = true;
   d->list.insert(index, it);
   invalidate();
}

void QBoxLayout::insertLayout(int index, QLayout *layout, int stretch)
{
   Q_D(QBoxLayout);

   if (! d->checkLayout(layout)) {
      return;
   }

   if (! adoptLayout(layout)) {
      return;
   }

   // append
   if (index < 0) {
      index = d->list.count();
   }

   QBoxLayoutItem *it = new QBoxLayoutItem(layout, stretch);
   d->list.insert(index, it);
   invalidate();
}

void QBoxLayout::insertWidget(int index, QWidget *widget, int stretch,
   Qt::Alignment alignment)
{
   Q_D(QBoxLayout);

   if (! d->checkWidget(widget)) {
      return;
   }

   addChildWidget(widget);

   // append
   if (index < 0) {
      index = d->list.count();
   }
   QWidgetItem *b = QLayoutPrivate::createWidgetItem(this, widget);
   b->setAlignment(alignment);

   QBoxLayoutItem *it;
   try {
      it = new QBoxLayoutItem(b, stretch);
   } catch (...) {
      delete b;
      throw;
   }

   try {
      d->list.insert(index, it);
   } catch (...) {
      delete it;
      throw;
   }
   invalidate();
}

void QBoxLayout::addSpacing(int size)
{
   insertSpacing(-1, size);
}

void QBoxLayout::addStretch(int stretch)
{
   insertStretch(-1, stretch);
}

void QBoxLayout::addSpacerItem(QSpacerItem *spacerItem)
{
   insertSpacerItem(-1, spacerItem);
}

void QBoxLayout::addWidget(QWidget *widget, int stretch, Qt::Alignment alignment)
{
   insertWidget(-1, widget, stretch, alignment);
}

void QBoxLayout::addLayout(QLayout *layout, int stretch)
{
   insertLayout(-1, layout, stretch);
}

void QBoxLayout::addStrut(int size)
{
   Q_D(QBoxLayout);
   QLayoutItem *b;
   if (horz(d->dir)) {
      b = QLayoutPrivate::createSpacerItem(this, 0, size, QSizePolicy::Fixed, QSizePolicy::Minimum);
   } else {
      b = QLayoutPrivate::createSpacerItem(this, size, 0, QSizePolicy::Minimum, QSizePolicy::Fixed);
   }

   QBoxLayoutItem *it = new QBoxLayoutItem(b);
   it->magic = true;
   d->list.append(it);
   invalidate();
}

bool QBoxLayout::setStretchFactor(QWidget *widget, int stretch)
{
   Q_D(QBoxLayout);
   if (!widget) {
      return false;
   }
   for (int i = 0; i < d->list.size(); ++i) {
      QBoxLayoutItem *box = d->list.at(i);
      if (box->item->widget() == widget) {
         box->stretch = stretch;
         invalidate();
         return true;
      }
   }
   return false;
}

bool QBoxLayout::setStretchFactor(QLayout *layout, int stretch)
{
   Q_D(QBoxLayout);
   for (int i = 0; i < d->list.size(); ++i) {
      QBoxLayoutItem *box = d->list.at(i);
      if (box->item->layout() == layout) {
         if (box->stretch != stretch) {
            box->stretch = stretch;
            invalidate();
         }
         return true;
      }
   }
   return false;
}

void QBoxLayout::setStretch(int index, int stretch)
{
   Q_D(QBoxLayout);
   if (index >= 0 && index < d->list.size()) {
      QBoxLayoutItem *box = d->list.at(index);
      if (box->stretch != stretch) {
         box->stretch = stretch;
         invalidate();
      }
   }
}

int QBoxLayout::stretch(int index) const
{
   Q_D(const QBoxLayout);
   if (index >= 0 && index < d->list.size()) {
      return d->list.at(index)->stretch;
   }
   return -1;
}

void QBoxLayout::setDirection(Direction direction)
{
   Q_D(QBoxLayout);

   if (d->dir == direction) {
      return;
   }

   if (horz(d->dir) != horz(direction)) {
      // swap around the spacers (the "magic" bits)
      // might be better to add access functions to spacerItem or even a QSpacerItem::flip()

      for (int i = 0; i < d->list.size(); ++i) {
         QBoxLayoutItem *box = d->list.at(i);

         if (box->magic) {
            QSpacerItem *sp = box->item->spacerItem();

            if (sp) {
               if (sp->expandingDirections() == Qt::Orientations(Qt::EmptyFlag)) {
                  //spacing or strut
                  QSize s = sp->sizeHint();
                  sp->changeSize(s.height(), s.width(),
                     horz(direction) ? QSizePolicy::Fixed : QSizePolicy::Minimum,
                     horz(direction) ? QSizePolicy::Minimum : QSizePolicy::Fixed);

               } else {
                  //stretch
                  if (horz(direction))
                     sp->changeSize(0, 0, QSizePolicy::Expanding,
                        QSizePolicy::Minimum);
                  else
                     sp->changeSize(0, 0, QSizePolicy::Minimum,
                        QSizePolicy::Expanding);
               }
            }
         }
      }
   }

   d->dir = direction;
   invalidate();
}

QBoxLayout::Direction QBoxLayout::direction() const
{
   Q_D(const QBoxLayout);
   return d->dir;
}

QHBoxLayout::QHBoxLayout(QWidget *parent)
   : QBoxLayout(LeftToRight, parent)
{
}

QHBoxLayout::QHBoxLayout()
   : QBoxLayout(LeftToRight)
{
}

QHBoxLayout::~QHBoxLayout()
{
}

QVBoxLayout::QVBoxLayout(QWidget *parent)
   : QBoxLayout(TopToBottom, parent)
{
}

QVBoxLayout::QVBoxLayout()
   : QBoxLayout(TopToBottom)
{
}

QVBoxLayout::~QVBoxLayout()
{
}
