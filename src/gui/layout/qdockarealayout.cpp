/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qdockarealayout_p.h>

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qdockwidget.h>
#include <qmainwindow.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qtabbar.h>
#include <qvariant.h>
#include <qwidget.h>

#include <qdockwidget_p.h>
#include <qlayoutengine_p.h>
#include <qmainwindowlayout_p.h>
#include <qwidgetanimator_p.h>

#ifndef QT_NO_DOCKWIDGET

// qmainwindow.cpp
extern QMainWindowLayout *qt_mainwindow_layout(const QMainWindow *window);

static constexpr const int StateFlagVisible  = 1;
static constexpr const int StateFlagFloating = 2;

QPlaceHolderItem::QPlaceHolderItem(QWidget *w)
{
   objectName = w->objectName();
   hidden = w->isHidden();
   window = w->isWindow();

   if (window) {
      topLevelRect = w->geometry();
   }
}

QDockAreaLayoutItem::QDockAreaLayoutItem(QLayoutItem *_widgetItem)
   : widgetItem(_widgetItem), subinfo(nullptr), placeHolderItem(nullptr), pos(0), size(-1), flags(NoFlags)
{
}

QDockAreaLayoutItem::QDockAreaLayoutItem(QDockAreaLayoutInfo *_subinfo)
   : widgetItem(nullptr), subinfo(_subinfo), placeHolderItem(nullptr), pos(0), size(-1), flags(NoFlags)
{
}

QDockAreaLayoutItem::QDockAreaLayoutItem(QPlaceHolderItem *_placeHolderItem)
   : widgetItem(nullptr), subinfo(nullptr), placeHolderItem(_placeHolderItem), pos(0), size(-1), flags(NoFlags)
{
}

QDockAreaLayoutItem::QDockAreaLayoutItem(const QDockAreaLayoutItem &other)
   : widgetItem(other.widgetItem), subinfo(nullptr), placeHolderItem(nullptr), pos(other.pos),
     size(other.size), flags(other.flags)
{
   if (other.subinfo != nullptr) {
      subinfo = new QDockAreaLayoutInfo(*other.subinfo);

   } else if (other.placeHolderItem != nullptr) {
      placeHolderItem = new QPlaceHolderItem(*other.placeHolderItem);
   }
}

QDockAreaLayoutItem::~QDockAreaLayoutItem()
{
   delete subinfo;
   delete placeHolderItem;
}

bool QDockAreaLayoutItem::skip() const
{
   if (placeHolderItem != nullptr) {
      return true;
   }

   if (flags & GapItem) {
      return false;
   }

   if (widgetItem != nullptr) {
      return widgetItem->isEmpty();
   }

   if (subinfo != nullptr) {
      for (int i = 0; i < subinfo->item_list.count(); ++i) {
         if (!subinfo->item_list.at(i).skip()) {
            return false;
         }
      }
   }

   return true;
}

QSize QDockAreaLayoutItem::minimumSize() const
{
   if (widgetItem != nullptr) {
      int left, top, right, bottom;
      widgetItem->widget()->getContentsMargins(&left, &top, &right, &bottom);
      return widgetItem->minimumSize() + QSize(left + right, top + bottom);
   }

   if (subinfo != nullptr) {
      return subinfo->minimumSize();
   }
   return QSize(0, 0);
}

QSize QDockAreaLayoutItem::maximumSize() const
{
   if (widgetItem != nullptr) {
      int left, top, right, bottom;
      widgetItem->widget()->getContentsMargins(&left, &top, &right, &bottom);
      return widgetItem->maximumSize() + QSize(left + right, top + bottom);
   }
   if (subinfo != nullptr) {
      return subinfo->maximumSize();
   }
   return QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

bool QDockAreaLayoutItem::hasFixedSize(Qt::Orientation o) const
{
   return perp(o, minimumSize()) == perp(o, maximumSize());
}

bool QDockAreaLayoutItem::expansive(Qt::Orientation o) const
{
   if ((flags & GapItem) || placeHolderItem != nullptr) {
      return false;
   }

   if (widgetItem != nullptr) {
      return ((widgetItem->expandingDirections() & o) == o);
   }

   if (subinfo != nullptr) {
      return subinfo->expansive(o);
   }

   return false;
}

QSize QDockAreaLayoutItem::sizeHint() const
{
   if (placeHolderItem != nullptr) {
      return QSize(0, 0);
   }

   if (widgetItem != nullptr) {
      int left, top, right, bottom;
      widgetItem->widget()->getContentsMargins(&left, &top, &right, &bottom);
      return widgetItem->sizeHint() + QSize(left + right, top + bottom);
   }

   if (subinfo != nullptr) {
      return subinfo->sizeHint();
   }
   return QSize(-1, -1);
}

QDockAreaLayoutItem &QDockAreaLayoutItem::operator = (const QDockAreaLayoutItem &other)
{
   widgetItem = other.widgetItem;
   if (other.subinfo == nullptr) {
      subinfo = nullptr;
   } else {
      subinfo = new QDockAreaLayoutInfo(*other.subinfo);
   }

   delete placeHolderItem;
   if (other.placeHolderItem == nullptr) {
      placeHolderItem = nullptr;
   } else {
      placeHolderItem = new QPlaceHolderItem(*other.placeHolderItem);
   }

   pos   = other.pos;
   size  = other.size;
   flags = other.flags;

   return *this;
}

#ifndef QT_NO_TABBAR
static quintptr tabId(const QDockAreaLayoutItem &item)
{
   if (item.widgetItem == nullptr) {
      return 0;
   }

   return reinterpret_cast<quintptr>(item.widgetItem->widget());
}
#endif

static constexpr const int zero = 0;

QDockAreaLayoutInfo::QDockAreaLayoutInfo()
   : m_dockAreaSep(&zero), m_dockPos(QInternal::LeftDock), m_dockAreaOrientation(Qt::Horizontal), mainWindow(nullptr)

#ifndef QT_NO_TABBAR
   , tabbed(false), tabBar(nullptr), tabBarShape(QTabBar::RoundedSouth)
#endif

{
}

QDockAreaLayoutInfo::QDockAreaLayoutInfo(const int *sep, QInternal::DockPosition dockPos,
      Qt::Orientation orientation, int tbshape, QMainWindow *window)
   : m_dockAreaSep(sep), m_dockPos(dockPos), m_dockAreaOrientation(orientation), mainWindow(window)

#ifndef QT_NO_TABBAR
   , tabbed(false), tabBar(nullptr), tabBarShape(static_cast<QTabBar::Shape>(tbshape))
#endif

{
#ifdef QT_NO_TABBAR
   (void) tbshape;
#endif
}

QSize QDockAreaLayoutInfo::size() const
{
   return isEmpty() ? QSize(0, 0) : m_dockAreaInfoRect.size();
}

void QDockAreaLayoutInfo::clear()
{
   item_list.clear();
   m_dockAreaInfoRect = QRect();

#ifndef QT_NO_TABBAR
   tabbed = false;
   tabBar = nullptr;
#endif
}

bool QDockAreaLayoutInfo::isEmpty() const
{
   return next(-1) == -1;
}

QSize QDockAreaLayoutInfo::minimumSize() const
{
   if (isEmpty()) {
      return QSize(0, 0);
   }

   int a = 0;
   int b = 0;
   bool first = true;

   for (int i = 0; i < item_list.size(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);

      if (item.skip()) {
         continue;
      }

      QSize min_size = item.minimumSize();

#ifndef QT_NO_TABBAR
      if (tabbed) {
         a = qMax(a, pick(m_dockAreaOrientation, min_size));
      } else
#endif

      {
         if (! first) {
            a += *m_dockAreaSep;
         }

         a += pick(m_dockAreaOrientation, min_size);
      }

      b = qMax(b, perp(m_dockAreaOrientation, min_size));

      first = false;
   }

   QSize result;
   rpick(m_dockAreaOrientation, result) = a;
   rperp(m_dockAreaOrientation, result) = b;

#ifndef QT_NO_TABBAR
   QSize tbm = tabBarMinimumSize();

   if (!tbm.isNull()) {
      switch (tabBarShape) {
         case QTabBar::RoundedNorth:
         case QTabBar::RoundedSouth:
         case QTabBar::TriangularNorth:
         case QTabBar::TriangularSouth:
            result.rheight() += tbm.height();
            result.rwidth() = qMax(tbm.width(), result.width());
            break;

         case QTabBar::RoundedEast:
         case QTabBar::RoundedWest:
         case QTabBar::TriangularEast:
         case QTabBar::TriangularWest:
            result.rheight() = qMax(tbm.height(), result.height());
            result.rwidth() += tbm.width();
            break;

         default:
            break;
      }
   }
#endif

   return result;
}

QSize QDockAreaLayoutInfo::maximumSize() const
{
   if (isEmpty()) {
      return QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
   }

   int a = 0;
   int b = QWIDGETSIZE_MAX;

#ifndef QT_NO_TABBAR
   if (tabbed) {
      a = QWIDGETSIZE_MAX;
   }
#endif

   int min_perp = 0;

   bool first = true;
   for (int i = 0; i < item_list.size(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);

      if (item.skip()) {
         continue;
      }

      QSize max_size = item.maximumSize();
      min_perp = qMax(min_perp, perp(m_dockAreaOrientation, item.minimumSize()));

#ifndef QT_NO_TABBAR
      if (tabbed) {
         a = qMin(a, pick(m_dockAreaOrientation, max_size));
      } else
#endif

      {
         if (! first) {
            a += *m_dockAreaSep;
         }
         a += pick(m_dockAreaOrientation, max_size);
      }

      b = qMin(b, perp(m_dockAreaOrientation, max_size));

      a = qMin(a, int(QWIDGETSIZE_MAX));
      b = qMin(b, int(QWIDGETSIZE_MAX));

      first = false;
   }

   b = qMax(b, min_perp);

   QSize result;
   rpick(m_dockAreaOrientation, result) = a;
   rperp(m_dockAreaOrientation, result) = b;

#ifndef QT_NO_TABBAR
   QSize tbh = tabBarSizeHint();

   if (!tbh.isNull()) {
      switch (tabBarShape) {
         case QTabBar::RoundedNorth:
         case QTabBar::RoundedSouth:
            result.rheight() += tbh.height();
            break;

         case QTabBar::RoundedEast:
         case QTabBar::RoundedWest:
            result.rwidth() += tbh.width();
            break;

         default:
            break;
      }
   }
#endif

   return result;
}

QSize QDockAreaLayoutInfo::sizeHint() const
{
   if (isEmpty()) {
      return QSize(0, 0);
   }

   int a = 0, b = 0;
   int min_perp = 0;
   int max_perp = QWIDGETSIZE_MAX;

   const QDockAreaLayoutItem *previous = nullptr;

   for (int i = 0; i < item_list.size(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);

      if (item.skip()) {
         continue;
      }

      bool gap = item.flags & QDockAreaLayoutItem::GapItem;

      QSize size_hint = item.sizeHint();
      min_perp = qMax(min_perp, perp(m_dockAreaOrientation, item.minimumSize()));
      max_perp = qMin(max_perp, perp(m_dockAreaOrientation, item.maximumSize()));

#ifndef QT_NO_TABBAR
      if (tabbed) {
         a = qMax(a, gap ? item.size : pick(m_dockAreaOrientation, size_hint));
      } else
#endif

      {
         if (previous && !gap && !(previous->flags &  QDockAreaLayoutItem::GapItem)
               && ! previous->hasFixedSize(m_dockAreaOrientation)) {
            a += *m_dockAreaSep;
         }
         a += gap ? item.size : pick(m_dockAreaOrientation, size_hint);
      }

      b = qMax(b, perp(m_dockAreaOrientation, size_hint));

      previous = &item;
   }

   max_perp = qMax(max_perp, min_perp);
   b = qMax(b, min_perp);
   b = qMin(b, max_perp);

   QSize result;
   rpick(m_dockAreaOrientation, result) = a;
   rperp(m_dockAreaOrientation, result) = b;

#ifndef QT_NO_TABBAR
   if (tabbed) {
      QSize tbh = tabBarSizeHint();

      switch (tabBarShape) {
         case QTabBar::RoundedNorth:
         case QTabBar::RoundedSouth:
         case QTabBar::TriangularNorth:
         case QTabBar::TriangularSouth:
            result.rheight() += tbh.height();
            result.rwidth() = qMax(tbh.width(), result.width());
            break;

         case QTabBar::RoundedEast:
         case QTabBar::RoundedWest:
         case QTabBar::TriangularEast:
         case QTabBar::TriangularWest:
            result.rheight() = qMax(tbh.height(), result.height());
            result.rwidth() += tbh.width();
            break;

         default:
            break;
      }
   }
#endif

   return result;
}

bool QDockAreaLayoutInfo::expansive(Qt::Orientation newOrientation) const
{
   for (int i = 0; i < item_list.size(); ++i) {
      if (item_list.at(i).expansive(newOrientation)) {
         return true;
      }
   }

   return false;
}

static int realMinSize(const QDockAreaLayoutInfo &info)
{
   int result = 0;
   bool first = true;

   for (int i = 0; i < info.item_list.size(); ++i) {
      const QDockAreaLayoutItem &item = info.item_list.at(i);
      if (item.skip()) {
         continue;
      }

      int min = 0;
      if ((item.flags & QDockAreaLayoutItem::KeepSize) && item.size != -1) {
         min = item.size;
      } else {
         min = pick(info.m_dockAreaOrientation, item.minimumSize());
      }

      if (!first) {
         result += *info.m_dockAreaSep;
      }
      result += min;

      first = false;
   }

   return result;
}

static int realMaxSize(const QDockAreaLayoutInfo &info)
{
   int result = 0;
   bool first = true;

   for (int i = 0; i < info.item_list.size(); ++i) {
      const QDockAreaLayoutItem &item = info.item_list.at(i);
      if (item.skip()) {
         continue;
      }

      int max = 0;
      if ((item.flags & QDockAreaLayoutItem::KeepSize) && item.size != -1) {
         max = item.size;
      } else {
         max = pick(info.m_dockAreaOrientation, item.maximumSize());
      }

      if (!first) {
         result += *info.m_dockAreaSep;
      }
      result += max;

      if (result >= QWIDGETSIZE_MAX) {
         return QWIDGETSIZE_MAX;
      }

      first = false;
   }

   return result;
}

void QDockAreaLayoutInfo::fitItems()
{
#ifndef QT_NO_TABBAR
   if (tabbed) {
      return;
   }
#endif

   QVector<QLayoutStruct> layout_struct_list(item_list.size() * 2);
   int j = 0;

   int size = pick(m_dockAreaOrientation, m_dockAreaInfoRect.size());

   int min_size = realMinSize(*this);
   int max_size = realMaxSize(*this);
   int last_index = -1;

   const QDockAreaLayoutItem *previous = nullptr;
   for (int i = 0; i < item_list.size(); ++i) {
      QDockAreaLayoutItem &item = item_list[i];

      if (item.skip()) {
         continue;
      }

      bool gap = item.flags & QDockAreaLayoutItem::GapItem;
      if (previous && !gap) {
         if (! (previous->flags & QDockAreaLayoutItem::GapItem)) {
            QLayoutStruct &ls = layout_struct_list[j++];
            ls.init();
            ls.minimumSize = ls.maximumSize = ls.sizeHint = previous->hasFixedSize(m_dockAreaOrientation) ? 0 : *m_dockAreaSep;
            ls.empty = false;
         }
      }

      if (item.flags & QDockAreaLayoutItem::KeepSize) {
         // Check if the item can keep its size, without violating size constraints
         // of other items.

         if (size < min_size) {
            // There is too little space to keep this widget's size
            item.flags &= ~QDockAreaLayoutItem::KeepSize;
            min_size -= item.size;
            min_size += pick(m_dockAreaOrientation, item.minimumSize());
            min_size = qMax(0, min_size);

         } else if (size > max_size) {
            // There is too much space to keep this widget's size
            item.flags &= ~QDockAreaLayoutItem::KeepSize;
            max_size -= item.size;
            max_size += pick(m_dockAreaOrientation, item.maximumSize());
            max_size = qMin(QWIDGETSIZE_MAX, max_size);
         }
      }

      last_index = j;
      QLayoutStruct &ls = layout_struct_list[j++];
      ls.init();
      ls.empty = false;

      if (item.flags & QDockAreaLayoutItem::KeepSize) {
         ls.minimumSize = ls.maximumSize = ls.sizeHint = item.size;
         ls.expansive = false;
         ls.stretch = 0;
      } else {
         ls.maximumSize = pick(m_dockAreaOrientation, item.maximumSize());
         ls.expansive = item.expansive(m_dockAreaOrientation);
         ls.minimumSize = pick(m_dockAreaOrientation, item.minimumSize());
         ls.sizeHint = item.size == -1 ? pick(m_dockAreaOrientation, item.sizeHint()) : item.size;
         ls.stretch = ls.expansive ? ls.sizeHint : 0;
      }

      item.flags &= ~QDockAreaLayoutItem::KeepSize;
      previous = &item;
   }

   layout_struct_list.resize(j);

   // If there is more space than the widgets can take (due to maximum size constraints),
   // we detect it here and stretch the last widget to take up the rest of the space.
   if (size > max_size && last_index != -1) {
      layout_struct_list[last_index].maximumSize = QWIDGETSIZE_MAX;
      layout_struct_list[last_index].expansive = true;
   }

   qGeomCalc(layout_struct_list, 0, j, pick(m_dockAreaOrientation, m_dockAreaInfoRect.topLeft()), size, 0);

   j = 0;

   bool prev_gap = false;
   bool first    = true;

   for (int i = 0; i < item_list.size(); ++i) {
      QDockAreaLayoutItem &item = item_list[i];
      if (item.skip()) {
         continue;
      }

      bool gap = item.flags & QDockAreaLayoutItem::GapItem;
      if (!first && !gap && !prev_gap) {
         ++j;
      }

      const QLayoutStruct &ls = layout_struct_list.at(j++);
      item.size = ls.size;
      item.pos = ls.pos;

      if (item.subinfo != nullptr) {
         item.subinfo->m_dockAreaInfoRect = itemRect(i);
         item.subinfo->fitItems();
      }

      prev_gap = gap;
      first = false;
   }
}

static QInternal::DockPosition dockPosHelper(const QRect &rect, const QPoint &_pos,
   Qt::Orientation o,
   bool nestingEnabled,
   QDockAreaLayoutInfo::TabMode tabMode)
{
   if (tabMode == QDockAreaLayoutInfo::ForceTabs) {
      return QInternal::DockCount;
   }

   QPoint pos = _pos - rect.topLeft();

   int x = pos.x();
   int y = pos.y();
   int w = rect.width();
   int h = rect.height();

   if (tabMode != QDockAreaLayoutInfo::NoTabs) {
      // is it in the center?
      if (nestingEnabled) {
         /*             2/3
                 +--------------+
                 |              |
                 |   CCCCCCCC   |
            2/3  |   CCCCCCCC   |
                 |   CCCCCCCC   |
                 |              |
                 +--------------+     */

         QRect center(w / 6, h / 6, 2 * w / 3, 2 * h / 3);
         if (center.contains(pos)) {
            return QInternal::DockCount;
         }
      } else if (o == Qt::Horizontal) {
         /*             2/3
                 +--------------+
                 |   CCCCCCCC   |
                 |   CCCCCCCC   |
                 |   CCCCCCCC   |
                 |   CCCCCCCC   |
                 |   CCCCCCCC   |
                 +--------------+     */

         if (x > w / 6 && x < w * 5 / 6) {
            return QInternal::DockCount;
         }
      } else {
         /*
                 +--------------+
                 |              |
            2/3  |CCCCCCCCCCCCCC|
                 |CCCCCCCCCCCCCC|
                 |              |
                 +--------------+     */
         if (y > h / 6 && y < 5 * h / 6) {
            return QInternal::DockCount;
         }
      }
   }

   // not in the center. which edge?
   if (nestingEnabled) {
      if (o == Qt::Horizontal) {
         /*       1/3  1/3 1/3
                 +------------+     (we've already ruled out the center)
                 |LLLLTTTTRRRR|
                 |LLLLTTTTRRRR|
                 |LLLLBBBBRRRR|
                 |LLLLBBBBRRRR|
                 +------------+    */

         if (x < w / 3) {
            return QInternal::LeftDock;
         }
         if (x > 2 * w / 3) {
            return QInternal::RightDock;
         }
         if (y < h / 2) {
            return QInternal::TopDock;
         }
         return QInternal::BottomDock;
      } else {
         /*      +------------+     (we've already ruled out the center)
             1/3 |TTTTTTTTTTTT|
                 |LLLLLLRRRRRR|
             1/3 |LLLLLLRRRRRR|
             1/3 |BBBBBBBBBBBB|
                 +------------+    */

         if (y < h / 3) {
            return QInternal::TopDock;
         }
         if (y > 2 * h / 3) {
            return QInternal::BottomDock;
         }
         if (x < w / 2) {
            return QInternal::LeftDock;
         }
         return QInternal::RightDock;
      }
   } else {
      if (o == Qt::Horizontal) {
         return x < w / 2 ? QInternal::LeftDock : QInternal::RightDock;

      } else {
         return y < h / 2 ? QInternal::TopDock : QInternal::BottomDock;
      }
   }
}

QList<int> QDockAreaLayoutInfo::gapIndex(const QPoint &_pos, bool nestingEnabled, TabMode tabMode) const
{
   QList<int> result;
   QRect item_rect;
   int item_index = 0;

#ifndef QT_NO_TABBAR
   if (tabbed) {
      item_rect = tabContentRect();
   } else
#endif

   {
      int pos = pick(m_dockAreaOrientation, _pos);

      int last = -1;
      for (int i = 0; i < item_list.size(); ++i) {
         const QDockAreaLayoutItem &item = item_list.at(i);
         if (item.skip()) {
            continue;
         }

         last = i;

         if (item.pos + item.size < pos) {
            continue;
         }

         if (item.subinfo != nullptr

#ifndef QT_NO_TABBAR
            && ! item.subinfo->tabbed
#endif

         ) {
            result = item.subinfo->gapIndex(_pos, nestingEnabled, tabMode);
            result.prepend(i);
            return result;
         }

         item_rect = itemRect(i);
         item_index = i;
         break;
      }

      if (item_rect.isNull()) {
         result.append(last + 1);
         return result;
      }
   }

   Q_ASSERT(! item_rect.isNull());

   QInternal::DockPosition dock_pos = dockPosHelper(item_rect, _pos, m_dockAreaOrientation, nestingEnabled, tabMode);

   switch (dock_pos) {
      case QInternal::LeftDock:
         if (m_dockAreaOrientation == Qt::Horizontal) {
            result << item_index;
         } else {
            result << item_index << 0;   // this subinfo doesn't exist yet, but insertGap()
         }

         // handles this by inserting it
         break;

      case QInternal::RightDock:
         if (m_dockAreaOrientation == Qt::Horizontal) {
            result << item_index + 1;
         } else {
            result << item_index << 1;
         }
         break;

      case QInternal::TopDock:
         if (m_dockAreaOrientation == Qt::Horizontal) {
            result << item_index << 0;
         } else {
            result << item_index;
         }
         break;

      case QInternal::BottomDock:
         if (m_dockAreaOrientation == Qt::Horizontal) {
            result << item_index << 1;
         } else {
            result << item_index + 1;
         }
         break;

      case  QInternal::DockCount:
         result << (-item_index - 1) << 0;   // negative item_index means "on top of"
         // -item_index - 1, insertGap()
         // will insert a tabbed subinfo
         break;

      default:
         break;
   }

   return result;
}

static inline int shrink(QLayoutStruct &ls, int delta)
{
   if (ls.empty) {
      return 0;
   }

   int old_size = ls.size;
   ls.size = qMax(ls.size - delta, ls.minimumSize);

   return old_size - ls.size;
}

static inline int grow(QLayoutStruct &ls, int delta)
{
   if (ls.empty) {
      return 0;
   }

   int old_size = ls.size;
   ls.size = qMin(ls.size + delta, ls.maximumSize);

   return ls.size - old_size;
}

static int separatorMoveHelper(QVector<QLayoutStruct> &list, int index, int delta, int sep)
{
   // adjust sizes
   int pos = -1;
   for (int i = 0; i < list.size(); ++i) {
      const QLayoutStruct &ls = list.at(i);
      if (!ls.empty) {
         pos = ls.pos;
         break;
      }
   }
   if (pos == -1) {
      return 0;
   }

   if (delta > 0) {
      int growlimit = 0;
      for (int i = 0; i <= index; ++i) {
         const QLayoutStruct &ls = list.at(i);
         if (ls.empty) {
            continue;
         }
         if (ls.maximumSize == QLAYOUTSIZE_MAX) {
            growlimit = QLAYOUTSIZE_MAX;
            break;
         }
         growlimit += ls.maximumSize - ls.size;
      }
      if (delta > growlimit) {
         delta = growlimit;
      }

      int d = 0;
      for (int i = index + 1; d < delta && i < list.count(); ++i) {
         d += shrink(list[i], delta - d);
      }
      delta = d;
      d = 0;
      for (int i = index; d < delta && i >= 0; --i) {
         d += grow(list[i], delta - d);
      }
   } else if (delta < 0) {
      int growlimit = 0;
      for (int i = index + 1; i < list.count(); ++i) {
         const QLayoutStruct &ls = list.at(i);
         if (ls.empty) {
            continue;
         }
         if (ls.maximumSize == QLAYOUTSIZE_MAX) {
            growlimit = QLAYOUTSIZE_MAX;
            break;
         }
         growlimit += ls.maximumSize - ls.size;
      }
      if (-delta > growlimit) {
         delta = -growlimit;
      }

      int d = 0;
      for (int i = index; d < -delta && i >= 0; --i) {
         d += shrink(list[i], -delta - d);
      }
      delta = -d;
      d = 0;
      for (int i = index + 1; d < -delta && i < list.count(); ++i) {
         d += grow(list[i], -delta - d);
      }
   }

   // adjust positions
   bool first = true;
   for (int i = 0; i < list.size(); ++i) {
      QLayoutStruct &ls = list[i];
      if (ls.empty) {
         ls.pos = pos + (first ? 0 : sep);
         continue;
      }
      if (!first) {
         pos += sep;
      }
      ls.pos = pos;
      pos += ls.size;
      first = false;
   }

   return delta;
}

int QDockAreaLayoutInfo::separatorMove(int index, int delta)
{
#ifndef QT_NO_TABBAR
   Q_ASSERT(!tabbed);
#endif

   QVector<QLayoutStruct> list(item_list.size());
   for (int i = 0; i < list.size(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);
      QLayoutStruct &ls = list[i];

      Q_ASSERT(!(item.flags & QDockAreaLayoutItem::GapItem));

      if (item.skip()) {
         ls.empty = true;
      } else {
         const int separatorSpace = item.hasFixedSize(m_dockAreaOrientation) ? 0 : *m_dockAreaSep;
         ls.empty = false;
         ls.pos   = item.pos;
         ls.size  = item.size + separatorSpace;

         ls.minimumSize = pick(m_dockAreaOrientation, item.minimumSize()) + separatorSpace;
         ls.maximumSize = pick(m_dockAreaOrientation, item.maximumSize()) + separatorSpace;

      }
   }

   //the separator space has been added to the size, so we pass 0 as a parameter
   delta = separatorMoveHelper(list, index, delta, 0);

   for (int i = 0; i < list.size(); ++i) {
      QDockAreaLayoutItem &item = item_list[i];

      if (item.skip()) {
         continue;
      }

      QLayoutStruct &ls = list[i];
      const int separatorSpace = item.hasFixedSize(m_dockAreaOrientation) ? 0 : *m_dockAreaSep;
      item.size = ls.size - separatorSpace;
      item.pos  = ls.pos;

      if (item.subinfo != nullptr) {
         item.subinfo->m_dockAreaInfoRect = itemRect(i);
         item.subinfo->fitItems();
      }
   }

   return delta;
}

void QDockAreaLayoutInfo::unnest(int index)
{
   QDockAreaLayoutItem &item = item_list[index];
   if (item.subinfo == nullptr) {
      return;
   }
   if (item.subinfo->item_list.count() > 1) {
      return;
   }

   if (item.subinfo->item_list.count() == 0) {
      item_list.removeAt(index);

   } else if (item.subinfo->item_list.count() == 1) {
      QDockAreaLayoutItem &child = item.subinfo->item_list.first();

      if (child.widgetItem != nullptr) {
         item.widgetItem = child.widgetItem;
         delete item.subinfo;
         item.subinfo = nullptr;

      } else if (child.subinfo != nullptr) {
         QDockAreaLayoutInfo *tmp = item.subinfo;
         item.subinfo = child.subinfo;
         child.subinfo = nullptr;
         tmp->item_list.clear();
         delete tmp;
      }
   }
}

void QDockAreaLayoutInfo::remove(const QList<int> &path)
{
   Q_ASSERT(!path.isEmpty());

   if (path.count() > 1) {
      const int index = path.first();
      QDockAreaLayoutItem &item = item_list[index];
      Q_ASSERT(item.subinfo != nullptr);
      item.subinfo->remove(path.mid(1));
      unnest(index);

   } else {
      int index = path.first();
      item_list.removeAt(index);
   }
}

QLayoutItem *QDockAreaLayoutInfo::plug(const QList<int> &path)
{
   Q_ASSERT(!path.isEmpty());

   int index = path.first();
   if (index < 0) {
      index = -index - 1;
   }

   if (path.count() > 1) {
      const QDockAreaLayoutItem &item = item_list.at(index);
      Q_ASSERT(item.subinfo != nullptr);

      return item.subinfo->plug(path.mid(1));
   }

   QDockAreaLayoutItem &item = item_list[index];

   Q_ASSERT(item.widgetItem != nullptr);
   Q_ASSERT(item.flags & QDockAreaLayoutItem::GapItem);
   item.flags &= ~QDockAreaLayoutItem::GapItem;

   QRect result;

#ifndef QT_NO_TABBAR
   if (tabbed) {
   } else
#endif

   {
      int prev = this->prev(index);
      int next = this->next(index);

      if (prev != -1 && ! (item_list.at(prev).flags & QDockAreaLayoutItem::GapItem)) {
         item.pos  += *m_dockAreaSep;
         item.size -= *m_dockAreaSep;
      }

      if (next != -1 && !(item_list.at(next).flags & QDockAreaLayoutItem::GapItem)) {
         item.size -= *m_dockAreaSep;
      }

      QPoint pos;
      rpick(m_dockAreaOrientation, pos) = item.pos;
      rperp(m_dockAreaOrientation, pos) = perp(m_dockAreaOrientation, m_dockAreaInfoRect.topLeft());

      QSize s;
      rpick(m_dockAreaOrientation, s) = item.size;
      rperp(m_dockAreaOrientation, s) = perp(m_dockAreaOrientation, m_dockAreaInfoRect.size());

      result = QRect(pos, s);
   }

   return item.widgetItem;
}

QLayoutItem *QDockAreaLayoutInfo::unplug(const QList<int> &path)
{
   Q_ASSERT(! path.isEmpty());

   const int index = path.first();

   if (path.count() > 1) {
      const QDockAreaLayoutItem &item = item_list.at(index);
      Q_ASSERT(item.subinfo != nullptr);
      return item.subinfo->unplug(path.mid(1));
   }

   QDockAreaLayoutItem &item = item_list[index];
   int prev = this->prev(index);
   int next = this->next(index);

   Q_ASSERT(!(item.flags & QDockAreaLayoutItem::GapItem));
   item.flags |= QDockAreaLayoutItem::GapItem;

#ifndef QT_NO_TABBAR
   if (tabbed) {
   } else
#endif

   {
      if (prev != -1 && ! (item_list.at(prev).flags & QDockAreaLayoutItem::GapItem)) {
         item.pos  -= *m_dockAreaSep;
         item.size += *m_dockAreaSep;
      }

      if (next != -1 && ! (item_list.at(next).flags & QDockAreaLayoutItem::GapItem)) {
         item.size += *m_dockAreaSep;
      }
   }

   return item.widgetItem;
}

#ifndef QT_NO_TABBAR
quintptr QDockAreaLayoutInfo::currentTabId() const
{
   if (! tabbed || tabBar == nullptr) {
      return 0;
   }

   int index = tabBar->currentIndex();
   if (index == -1) {
      return 0;
   }

   return (tabBar->tabData(index)).value<quintptr>();
}

void QDockAreaLayoutInfo::setCurrentTab(QWidget *widget)
{
   setCurrentTabId(reinterpret_cast<quintptr>(widget));
}

void QDockAreaLayoutInfo::setCurrentTabId(quintptr id)
{
   if (! tabbed || tabBar == nullptr) {
      return;
   }

   for (int i = 0; i < tabBar->count(); ++i) {
      QVariant variant = tabBar->tabData(i);

      if (variant.value<quintptr>() == id) {
         tabBar->setCurrentIndex(i);
         return;
      }
   }
}
#endif

static QRect dockedGeometry(QWidget *widget)
{
   int titleHeight = 0;

   QDockWidgetLayout *layout = qobject_cast<QDockWidgetLayout *>(widget->layout());

   if (layout != nullptr && layout->nativeWindowDeco()) {
      titleHeight = layout->titleHeight();
   }

   QRect result = widget->geometry();
   result.adjust(0, -titleHeight, 0, 0);
   return result;
}

bool QDockAreaLayoutInfo::insertGap(const QList<int> &path, QLayoutItem *dockWidgetItem)
{
   Q_ASSERT(!path.isEmpty());

   bool insert_tabbed = false;
   int index = path.first();

   if (index < 0) {
      insert_tabbed = true;
      index = -index - 1;
   }

   if (path.count() > 1) {
      QDockAreaLayoutItem &item = item_list[index];

#ifdef QT_NO_TABBAR
      if (item.subinfo == nullptr) {
#else
      if (item.subinfo == nullptr || (item.subinfo->tabbed && ! insert_tabbed)) {
#endif

         // this is not yet a nested layout - make it

         QDockAreaLayoutInfo *subinfo = item.subinfo;
         QLayoutItem *widgetItem      = item.widgetItem;
         QPlaceHolderItem *placeHolderItem = item.placeHolderItem;

         QRect r;

         if (subinfo == nullptr) {

            if (widgetItem != nullptr) {
               r = dockedGeometry(widgetItem->widget());

            } else {
               r = placeHolderItem->topLevelRect;
            }

         } else {
            r = subinfo->m_dockAreaInfoRect;

         }

         Qt::Orientation opposite = (m_dockAreaOrientation == Qt::Horizontal) ? Qt::Vertical : Qt::Horizontal;

#ifdef QT_NO_TABBAR
         const int tabBarShape = 0;
#endif

         QDockAreaLayoutInfo *new_info = new QDockAreaLayoutInfo(m_dockAreaSep, m_dockPos, opposite, tabBarShape, mainWindow);

         //item become a new top-level
         item.subinfo         = new_info;
         item.widgetItem      = nullptr;
         item.placeHolderItem = nullptr;

         QDockAreaLayoutItem new_item = (widgetItem == nullptr)
               ? QDockAreaLayoutItem(subinfo) : widgetItem ? QDockAreaLayoutItem(widgetItem) : QDockAreaLayoutItem(placeHolderItem);

         new_item.size = pick(opposite, r.size());
         new_item.pos = pick(opposite, r.topLeft());
         new_info->item_list.append(new_item);

#ifndef QT_NO_TABBAR
         if (insert_tabbed) {
            new_info->tabbed = true;
         }
#endif

      }

      return item.subinfo->insertGap(path.mid(1), dockWidgetItem);
   }

   // create the gap item
   QDockAreaLayoutItem gap_item;
   gap_item.flags |= QDockAreaLayoutItem::GapItem;
   gap_item.widgetItem = dockWidgetItem;   // so minimumSize(), maximumSize() and sizeHint() will work

#ifndef QT_NO_TABBAR
   if (!tabbed)
#endif

   {
      int prev = this->prev(index);
      int next = this->next(index - 1);
      // find out how much space we have in the layout
      int space = 0;

      if (isEmpty()) {
         // I am an empty dock area, therefore I am a top-level dock area.
         switch (m_dockPos) {
            case QInternal::LeftDock:
            case QInternal::RightDock:
               if (m_dockAreaOrientation == Qt::Vertical) {
                  // the "size" is the height of the dock area (remember we are empty)
                  space = pick(Qt::Vertical, m_dockAreaInfoRect.size());
               } else {
                  space = pick(Qt::Horizontal, dockWidgetItem->widget()->size());
               }
               break;

            case QInternal::TopDock:
            case QInternal::BottomDock:
            default:
               if (m_dockAreaOrientation == Qt::Horizontal) {
                  // the "size" is width of the dock area
                  space = pick(Qt::Horizontal, m_dockAreaInfoRect.size());
               } else {
                  space = pick(Qt::Vertical, dockWidgetItem->widget()->size());
               }
               break;
         }

      } else {
         for (int i = 0; i < item_list.count(); ++i) {
            const QDockAreaLayoutItem &item = item_list.at(i);
            if (item.skip()) {
               continue;
            }

            Q_ASSERT(!(item.flags & QDockAreaLayoutItem::GapItem));

            space += item.size - pick(m_dockAreaOrientation, item.minimumSize());
         }
      }

      // find the actual size of the gap
      int gap_size = 0;
      int sep_size = 0;

      if (isEmpty()) {
         gap_size = space;
         sep_size = 0;

      } else {
         QRect r = dockedGeometry(dockWidgetItem->widget());
         gap_size = pick(m_dockAreaOrientation, r.size());

         if (prev != -1 && !(item_list.at(prev).flags & QDockAreaLayoutItem::GapItem)) {
            sep_size += *m_dockAreaSep;
         }
         if (next != -1 && !(item_list.at(next).flags & QDockAreaLayoutItem::GapItem)) {
            sep_size += *m_dockAreaSep;
         }
      }

      if (gap_size + sep_size > space) {
         gap_size = pick(m_dockAreaOrientation, gap_item.minimumSize());
      }

      gap_item.size = gap_size + sep_size;
   }

   // finally, insert the gap
   item_list.insert(index, gap_item);

   return true;
}

QDockAreaLayoutInfo *QDockAreaLayoutInfo::info(QWidget *widget)
{
   for (int i = 0; i < item_list.count(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);
      if (item.skip()) {
         continue;
      }

#ifndef QT_NO_TABBAR
      if (tabbed && widget == tabBar) {
         return this;
      }
#endif

      if (item.widgetItem != nullptr && item.widgetItem->widget() == widget) {
         return this;
      }

      if (item.subinfo != nullptr) {
         if (QDockAreaLayoutInfo *result = item.subinfo->info(widget)) {
            return result;
         }
      }
   }

   return nullptr;
}

QDockAreaLayoutInfo *QDockAreaLayoutInfo::info(const QList<int> &path)
{
   int index = path.first();

   if (index < 0) {
      index = -index - 1;
   }

   if (index >= item_list.count()) {
      return this;
   }

   if (path.count() == 1 || item_list[index].subinfo == nullptr) {
      return this;
   }

   return item_list[index].subinfo->info(path.mid(1));
}

QRect QDockAreaLayoutInfo::itemRect(int index) const
{
   const QDockAreaLayoutItem &item = item_list.at(index);

   if (item.skip()) {
      return QRect();
   }

   QRect result;

#ifndef QT_NO_TABBAR
   if (tabbed) {
      if (tabId(item) == currentTabId()) {
         result = tabContentRect();
      }

   } else
#endif

   {
      QPoint pos;
      rpick(m_dockAreaOrientation, pos) = item.pos;
      rperp(m_dockAreaOrientation, pos) = perp(m_dockAreaOrientation, m_dockAreaInfoRect.topLeft());

      QSize s;
      rpick(m_dockAreaOrientation, s) = item.size;
      rperp(m_dockAreaOrientation, s) = perp(m_dockAreaOrientation, m_dockAreaInfoRect.size());
      result = QRect(pos, s);
   }

   return result;
}

QRect QDockAreaLayoutInfo::itemRect(const QList<int> &path) const
{
   Q_ASSERT(!path.isEmpty());

   const int index = path.first();
   if (path.count() > 1) {
      const QDockAreaLayoutItem &item = item_list.at(index);
      Q_ASSERT(item.subinfo != nullptr);
      return item.subinfo->itemRect(path.mid(1));
   }

   return itemRect(index);
}

QRect QDockAreaLayoutInfo::separatorRect(int index) const
{
#ifndef QT_NO_TABBAR
   if (tabbed) {
      return QRect();
   }
#endif

   const QDockAreaLayoutItem &item = item_list.at(index);
   if (item.skip()) {
      return QRect();
   }

   QPoint pos = m_dockAreaInfoRect.topLeft();
   rpick(m_dockAreaOrientation, pos) = item.pos + item.size;

   QSize s = m_dockAreaInfoRect.size();
   rpick(m_dockAreaOrientation, s) = *m_dockAreaSep;

   return QRect(pos, s);
}

QRect QDockAreaLayoutInfo::separatorRect(const QList<int> &path) const
{
   Q_ASSERT(!path.isEmpty());

   const int index = path.first();

   if (path.count() > 1) {
      const QDockAreaLayoutItem &item = item_list.at(index);
      Q_ASSERT(item.subinfo != nullptr);

      return item.subinfo->separatorRect(path.mid(1));
   }
   return separatorRect(index);
}

QList<int> QDockAreaLayoutInfo::findSeparator(const QPoint &newPos) const
{
#ifndef QT_NO_TABBAR
   if (tabbed) {
      return QList<int>();
   }
#endif

   int pos = pick(m_dockAreaOrientation, newPos);

   for (int i = 0; i < item_list.size(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);

      if (item.skip() || (item.flags & QDockAreaLayoutItem::GapItem)) {
         continue;
      }

      if (item.pos + item.size > pos) {
         if (item.subinfo != nullptr) {
            QList<int> result = item.subinfo->findSeparator(newPos);

            if (! result.isEmpty()) {
               result.prepend(i);
               return result;
            } else {
               return QList<int>();
            }
         }
      }

      int next = this->next(i);
      if (next == -1 || (item_list.at(next).flags & QDockAreaLayoutItem::GapItem)) {
         continue;
      }

      QRect sepRect = separatorRect(i);
      if (!sepRect.isNull() && *m_dockAreaSep == 1) {
         sepRect.adjust(-2, -2, 2, 2);
      }

      // we also make sure we don't find a separator that's not there
      if (sepRect.contains(newPos) && !item.hasFixedSize(m_dockAreaOrientation)) {
         return QList<int>() << i;
      }
   }

   return QList<int>();
}

QList<int> QDockAreaLayoutInfo::indexOfPlaceHolder(const QString &objectName) const
{
   for (int i = 0; i < item_list.size(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);

      if (item.subinfo != nullptr) {
         QList<int> result = item.subinfo->indexOfPlaceHolder(objectName);
         if (!result.isEmpty()) {
            result.prepend(i);
            return result;
         }
         continue;
      }

      if (item.placeHolderItem != nullptr && item.placeHolderItem->objectName == objectName) {
         QList<int> result;
         result << i;
         return result;
      }
   }

   return QList<int>();
}

QList<int> QDockAreaLayoutInfo::indexOf(QWidget *widget) const
{
   for (int i = 0; i < item_list.size(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);

      if (item.placeHolderItem != nullptr) {
         continue;
      }

      if (item.subinfo != nullptr) {
         QList<int> result = item.subinfo->indexOf(widget);
         if (!result.isEmpty()) {
            result.prepend(i);
            return result;
         }
         continue;
      }

      if (!(item.flags & QDockAreaLayoutItem::GapItem) && item.widgetItem->widget() == widget) {
         QList<int> result;
         result << i;
         return result;
      }
   }

   return QList<int>();
}

QMainWindowLayout *QDockAreaLayoutInfo::mainWindowLayout() const
{
   QMainWindowLayout *result = qt_mainwindow_layout(mainWindow);
   Q_ASSERT(result != nullptr);
   return result;
}

bool QDockAreaLayoutInfo::hasFixedSize() const
{
   return perp(m_dockAreaOrientation, minimumSize()) == perp(m_dockAreaOrientation, maximumSize());
}

void QDockAreaLayoutInfo::apply(bool animate)
{
   QWidgetAnimator &widgetAnimator = mainWindowLayout()->widgetAnimator;

#ifndef QT_NO_TABBAR
   if (tabbed) {
      QRect tab_rect;
      QSize tbh = tabBarSizeHint();

      if (!tbh.isNull()) {
         switch (tabBarShape) {
            case QTabBar::RoundedNorth:
            case QTabBar::TriangularNorth:
               tab_rect = QRect(m_dockAreaInfoRect.left(), m_dockAreaInfoRect.top(), m_dockAreaInfoRect.width(), tbh.height());
               break;

            case QTabBar::RoundedSouth:
            case QTabBar::TriangularSouth:
               tab_rect = QRect(m_dockAreaInfoRect.left(), m_dockAreaInfoRect.bottom() - tbh.height() + 1, m_dockAreaInfoRect.width(), tbh.height());
               break;

            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
               tab_rect = QRect(m_dockAreaInfoRect.right() - tbh.width() + 1, m_dockAreaInfoRect.top(), tbh.width(), m_dockAreaInfoRect.height());
               break;

            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
               tab_rect = QRect(m_dockAreaInfoRect.left(), m_dockAreaInfoRect.top(), tbh.width(), m_dockAreaInfoRect.height());
               break;

            default:
               break;
         }
      }

      widgetAnimator.animate(tabBar, tab_rect, animate);
   }
#endif

   for (int i = 0; i < item_list.size(); ++i) {
      QDockAreaLayoutItem &item = item_list[i];

      if (item.flags & QDockAreaLayoutItem::GapItem) {
         continue;
      }

      if (item.subinfo != nullptr) {
         item.subinfo->apply(animate);
         continue;
      }

      if (item.skip()) {
         continue;
      }

      Q_ASSERT(item.widgetItem);
      QRect r = itemRect(i);
      QWidget *w = item.widgetItem->widget();

      QRect geo = w->geometry();
      widgetAnimator.animate(w, r, animate);

      if (!w->isHidden() && w->window()->isVisible()) {
         QDockWidget *dw = qobject_cast<QDockWidget *>(w);

         if (!r.isValid() && geo.right() >= 0 && geo.bottom() >= 0) {
            dw->lower();
            emit dw->visibilityChanged(false);

         } else if (r.isValid() && (geo.right() < 0 || geo.bottom() < 0)) {
            emit dw->visibilityChanged(true);
         }
      }
   }

#ifndef QT_NO_TABBAR
   if (*m_dockAreaSep == 1) {
      updateSeparatorWidgets();
   }
#endif
}

static void paintSep(QPainter *p, QWidget *w, const QRect &r, Qt::Orientation o, bool mouse_over)
{
   QStyleOption opt(0);
   opt.state = QStyle::State_None;

   if (w->isEnabled()) {
      opt.state |= QStyle::State_Enabled;
   }

   if (o != Qt::Horizontal) {
      opt.state |= QStyle::State_Horizontal;
   }

   if (mouse_over) {
      opt.state |= QStyle::State_MouseOver;
   }

   opt.rect = r;
   opt.palette = w->palette();

   w->style()->drawPrimitive(QStyle::PE_IndicatorDockWidgetResizeHandle, &opt, p, w);
}

QRegion QDockAreaLayoutInfo::separatorRegion() const
{
   QRegion result;

   if (isEmpty()) {
      return result;
   }
#ifndef QT_NO_TABBAR
   if (tabbed) {
      return result;
   }
#endif

   for (int i = 0; i < item_list.count(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);

      if (item.skip()) {
         continue;
      }

      int next = this->next(i);

      if (item.subinfo) {
         result |= item.subinfo->separatorRegion();
      }

      if (next == -1) {
         break;
      }
      result |= separatorRect(i);
   }

   return result;
}

void QDockAreaLayoutInfo::paintSeparators(QPainter *p, QWidget *widget, const QRegion &clip, const QPoint &mouse) const
{
   if (isEmpty()) {
      return;
   }

#ifndef QT_NO_TABBAR
   if (tabbed) {
      return;
   }
#endif

   for (int i = 0; i < item_list.count(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);

      if (item.skip()) {
         continue;
      }

      int next = this->next(i);
      if ((item.flags & QDockAreaLayoutItem::GapItem)
         || (next != -1 && (item_list.at(next).flags & QDockAreaLayoutItem::GapItem))) {
         continue;
      }

      if (item.subinfo) {
         if (clip.contains(item.subinfo->m_dockAreaInfoRect)) {
            item.subinfo->paintSeparators(p, widget, clip, mouse);
         }
      }

      if (next == -1) {
         break;
      }

      QRect r = separatorRect(i);
      if (clip.contains(r) && !item.hasFixedSize(m_dockAreaOrientation)) {
         paintSep(p, widget, r, m_dockAreaOrientation, r.contains(mouse));
      }
   }
}

int QDockAreaLayoutInfo::next(int index) const
{
   for (int i = index + 1; i < item_list.size(); ++i) {
      if (!item_list.at(i).skip()) {
         return i;
      }
   }
   return -1;
}

int QDockAreaLayoutInfo::prev(int index) const
{
   for (int i = index - 1; i >= 0; --i) {
      if (!item_list.at(i).skip()) {
         return i;
      }
   }

   return -1;
}

void QDockAreaLayoutInfo::tab(int index, QLayoutItem *dockWidgetItem)
{
#ifdef QT_NO_TABBAR
   (void) index;
   (void) dockWidgetItem;
#else
   if (tabbed) {
      item_list.append(QDockAreaLayoutItem(dockWidgetItem));
      updateTabBar();
      setCurrentTab(dockWidgetItem->widget());

   } else {
      QDockAreaLayoutInfo *new_info = new QDockAreaLayoutInfo(m_dockAreaSep, m_dockPos, m_dockAreaOrientation, tabBarShape, mainWindow);
      item_list[index].subinfo = new_info;
      new_info->item_list.append(item_list.at(index).widgetItem);
      item_list[index].widgetItem = nullptr;

      new_info->item_list.append(dockWidgetItem);
      new_info->tabbed = true;
      new_info->updateTabBar();
      new_info->setCurrentTab(dockWidgetItem->widget());
   }
#endif
}

void QDockAreaLayoutInfo::split(int index, Qt::Orientation orientation, QLayoutItem *dockWidgetItem)
{
   if (orientation == m_dockAreaOrientation) {
      item_list.insert(index + 1, QDockAreaLayoutItem(dockWidgetItem));

   } else {

#ifdef QT_NO_TABBAR
      const int tabBarShape = 0;
#endif

      QDockAreaLayoutInfo *new_info = new QDockAreaLayoutInfo(m_dockAreaSep, m_dockPos, orientation, tabBarShape, mainWindow);

      item_list[index].subinfo = new_info;
      new_info->item_list.append(item_list.at(index).widgetItem);
      item_list[index].widgetItem = nullptr;
      new_info->item_list.append(dockWidgetItem);
   }
}

QDockAreaLayoutItem &QDockAreaLayoutInfo::item(const QList<int> &path)
{
   Q_ASSERT(! path.isEmpty());

   const int index = path.first();

   if (path.count() > 1) {
      const QDockAreaLayoutItem &item = item_list[index];
      Q_ASSERT(item.subinfo != nullptr);

      return item.subinfo->item(path.mid(1));
   }

   return item_list[index];
}

QLayoutItem *QDockAreaLayoutInfo::itemAt(int *x, int index) const
{
   for (int i = 0; i < item_list.count(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);
      if (item.placeHolderItem != nullptr) {
         continue;
      }

      if (item.subinfo) {
         if (QLayoutItem *ret = item.subinfo->itemAt(x, index)) {
            return ret;
         }

      } else if (item.widgetItem) {
         if ((*x)++ == index) {
            return item.widgetItem;
         }
      }
   }
   return nullptr;
}

QLayoutItem *QDockAreaLayoutInfo::takeAt(int *x, int index)
{
   for (int i = 0; i < item_list.count(); ++i) {
      QDockAreaLayoutItem &item = item_list[i];

      if (item.placeHolderItem != nullptr) {
         continue;

      } else if (item.subinfo) {
         if (QLayoutItem *ret = item.subinfo->takeAt(x, index)) {
            unnest(i);
            return ret;
         }

      } else if (item.widgetItem) {
         if ((*x)++ == index) {
            item.placeHolderItem = new QPlaceHolderItem(item.widgetItem->widget());
            QLayoutItem *ret = item.widgetItem;
            item.widgetItem = nullptr;

            if (item.size != -1) {
               item.flags |= QDockAreaLayoutItem::KeepSize;
            }
            return ret;
         }
      }
   }

   return nullptr;
}

void QDockAreaLayoutInfo::deleteAllLayoutItems()
{
   for (int i = 0; i < item_list.count(); ++i) {
      QDockAreaLayoutItem &item = item_list[i];

      if (item.subinfo) {
         item.subinfo->deleteAllLayoutItems();
      } else {
         delete item.widgetItem;
         item.widgetItem = nullptr;
      }
   }
}

void QDockAreaLayoutInfo::saveState(QDataStream &stream) const
{
#ifndef QT_NO_TABBAR
   if (tabbed) {
      stream << (uchar) TabMarker;

      // write the index in item_list of the widget that's currently on top.
      quintptr id = currentTabId();
      int index   = -1;

      for (int i = 0; i < item_list.count(); ++i) {
         if (tabId(item_list.at(i)) == id) {
            index = i;
            break;
         }
      }

      stream << index;

   } else
#endif

   {
      stream << (uchar) SequenceMarker;
   }

   stream << (uchar) m_dockAreaOrientation << item_list.count();

   for (int i = 0; i < item_list.count(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);

      if (item.widgetItem != nullptr) {
         stream << (uchar) WidgetMarker;

         QWidget *w = item.widgetItem->widget();
         QString name = w->objectName();

         if (name.isEmpty()) {
            qWarning("QDockAreaLayoutInfo::saveState() Object name was not set for this QDockWidget %p %s",
               static_cast<void *>(w), csPrintable(w->windowTitle()));
         }
         stream << name;

         uchar flags = 0;
         if (! w->isHidden()) {
            flags |= StateFlagVisible;
         }

         if (w->isWindow()) {
            flags |= StateFlagFloating;
         }

         stream << flags;

         if (w->isWindow()) {
            const QRect geometry = w->geometry();
            stream << geometry.x() << geometry.y() << geometry.width() << geometry.height();

         } else {
            stream << item.pos << item.size
                   << pick(m_dockAreaOrientation, item.minimumSize())
                   << pick(m_dockAreaOrientation, item.maximumSize());
         }

      } else if (item.placeHolderItem != nullptr) {
         stream << (uchar) WidgetMarker;
         stream << item.placeHolderItem->objectName;

         uchar flags = 0;

         if (! item.placeHolderItem->hidden) {
            flags |= StateFlagVisible;
         }

         if (item.placeHolderItem->window) {
            flags |= StateFlagFloating;
         }
         stream << flags;

         if (item.placeHolderItem->window) {
            QRect r = item.placeHolderItem->topLevelRect;
            stream << r.x() << r.y() << r.width() << r.height();

         } else {
            stream << item.pos << item.size << (int)0 << (int)0;
         }

      } else if (item.subinfo != nullptr) {
         stream << (uchar) SequenceMarker << item.pos << item.size << pick(m_dockAreaOrientation, item.minimumSize())
               << pick(m_dockAreaOrientation, item.maximumSize());

         item.subinfo->saveState(stream);
      }
   }
}

static Qt::DockWidgetArea toDockWidgetArea(QInternal::DockPosition pos)
{
   switch (pos) {
      case QInternal::LeftDock:
         return Qt::LeftDockWidgetArea;

      case QInternal::RightDock:
         return Qt::RightDockWidgetArea;

      case QInternal::TopDock:
         return Qt::TopDockWidgetArea;

      case QInternal::BottomDock:
         return Qt::BottomDockWidgetArea;

      default:
         break;
   }

   return Qt::NoDockWidgetArea;
}

bool QDockAreaLayoutInfo::restoreState(QDataStream &stream, QList<QDockWidget *> &widgets, bool testing)
{
   uchar marker;
   stream >> marker;

   if (marker != TabMarker && marker != SequenceMarker) {
      return false;
   }

#ifndef QT_NO_TABBAR
   tabbed = (marker == TabMarker);

   int index = -1;

   if (tabbed) {
      stream >> index;
   }
#endif

   uchar orientation;
   stream >> orientation;

   m_dockAreaOrientation = static_cast<Qt::Orientation>(orientation);

   decltype(item_list)::size_type cnt;
   stream >> cnt;

   for (int i = 0; i < cnt; ++i) {
      uchar nextMarker;
      stream >> nextMarker;

      if (nextMarker == WidgetMarker) {
         QString name;
         uchar flags;

         stream >> name
                >> flags;

         if (name.isEmpty()) {
            int dummy;
            stream >> dummy >> dummy >> dummy >> dummy;
            continue;
         }

         QDockWidget *widget = nullptr;
         for (int j = 0; j < widgets.count(); ++j) {
            if (widgets.at(j)->objectName() == name) {
               widget = widgets.takeAt(j);
               break;
            }
         }

         if (widget == nullptr) {
            QPlaceHolderItem *placeHolder = new QPlaceHolderItem;
            QDockAreaLayoutItem item(placeHolder);

            placeHolder->objectName = name;
            placeHolder->window = flags & StateFlagFloating;
            placeHolder->hidden = !(flags & StateFlagVisible);

            if (placeHolder->window) {
               int x, y, w, h;
               stream >> x >> y >> w >> h;

               placeHolder->topLevelRect = QRect(x, y, w, h);

            } else {
               int dummy;
               stream >> item.pos >> item.size >> dummy >> dummy;
            }

            if (item.size != -1) {
               item.flags |= QDockAreaLayoutItem::KeepSize;
            }

            if (! testing) {
               item_list.append(item);
            }

         } else {
            QDockAreaLayoutItem item(new QDockWidgetItem(widget));

            if (flags & StateFlagFloating) {
               bool drawer = false;

               if (!testing) {
                  widget->hide();
                  if (! drawer) {
                     widget->setFloating(true);
                  }
               }

               int x, y, w, h;
               stream >> x >> y >> w >> h;

               if (! testing) {
                  widget->setGeometry(QDockAreaLayout::constrainedRect(QRect(x, y, w, h), widget));
               }

               if (! testing) {
                  widget->setVisible(flags & StateFlagVisible);
                  item_list.append(item);
               }

            } else {
               int dummy;
               stream >> item.pos >> item.size >> dummy >> dummy;

               if (! testing) {
                  item_list.append(item);

                  widget->setFloating(false);
                  widget->setVisible(flags & StateFlagVisible);

                  emit widget->dockLocationChanged(toDockWidgetArea(m_dockPos));
               }
            }

            if (testing) {
               // was it is not really added to the layout, we need to delete the object here
               delete item.widgetItem;
            }
         }

      } else if (nextMarker == SequenceMarker) {
         int dummy;

#ifdef QT_NO_TABBAR
         const int tabBarShape = 0;
#endif

         QDockAreaLayoutItem item(new QDockAreaLayoutInfo(m_dockAreaSep, m_dockPos, m_dockAreaOrientation, tabBarShape, mainWindow));

         stream >> item.pos >> item.size >> dummy >> dummy;

         //we need to make sure the element is in the list so the dock widget can eventually be docked correctly
         if (! testing) {
            item_list.append(item);
         }

         //here we need to make sure we change the item in the item_list
         QDockAreaLayoutItem &lastItem = testing ? item : item_list.last();

         if (!lastItem.subinfo->restoreState(stream, widgets, testing)) {
            return false;
         }

      } else {
         return false;
      }
   }

#ifndef QT_NO_TABBAR
   if (!testing && tabbed && index >= 0 && index < item_list.count()) {
      updateTabBar();
      setCurrentTabId(tabId(item_list.at(index)));
   }

   if (!testing && *m_dockAreaSep == 1) {
      updateSeparatorWidgets();
   }
#endif

   return true;
}

#ifndef QT_NO_TABBAR
void QDockAreaLayoutInfo::updateSeparatorWidgets() const
{
   if (tabbed) {
      separatorWidgets.clear();
      return;
   }

   int j = 0;
   for (int i = 0; i < item_list.count(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);

      if (item.skip()) {
         continue;
      }

      int next = this->next(i);
      if ((item.flags & QDockAreaLayoutItem::GapItem)
         || (next != -1 && (item_list.at(next).flags & QDockAreaLayoutItem::GapItem))) {
         continue;
      }

      if (item.subinfo) {
         item.subinfo->updateSeparatorWidgets();
      }

      if (next == -1) {
         break;
      }

      QWidget *sepWidget;
      if (j < separatorWidgets.size() && separatorWidgets.at(j)) {
         sepWidget = separatorWidgets.at(j);
      } else {
         sepWidget = mainWindowLayout()->getSeparatorWidget();
         separatorWidgets.append(sepWidget);
      }
      j++;

      sepWidget->raise();

      QRect sepRect = separatorRect(i).adjusted(-2, -2, 2, 2);
      sepWidget->setGeometry(sepRect);
      sepWidget->setMask( QRegion(separatorRect(i).translated( - sepRect.topLeft())));
      sepWidget->show();
   }

   for (int k = j; k < separatorWidgets.size(); ++k) {
      separatorWidgets[k]->hide();
   }
   separatorWidgets.resize(j);
   Q_ASSERT(separatorWidgets.size() == j);
}
#endif

#ifndef QT_NO_TABBAR
void QDockAreaLayoutInfo::reparentWidgets(QWidget *parent)
{
   if (tabBar) {
      tabBar->setParent(parent);
   }

   for (int i = 0; i < item_list.count(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);

      if (item.flags & QDockAreaLayoutItem::GapItem) {
         continue;
      }

      if (item.subinfo) {
         item.subinfo->reparentWidgets(parent);
      }

      if (item.widgetItem) {
         QWidget *w = item.widgetItem->widget();
         if (qobject_cast<QDockWidgetGroupWindow *>(w)) {
            continue;
         }
         if (w->parent() != parent) {
            bool hidden = w->isHidden();
            w->setParent(parent, w->windowFlags());

            if (!hidden) {
               w->show();
            }
         }
      }
   }
}

bool QDockAreaLayoutInfo::updateTabBar() const
{
   if (!tabbed) {
      return false;
   }

   QDockAreaLayoutInfo *self = const_cast<QDockAreaLayoutInfo *>(this);

   if (self->tabBar == nullptr) {
      self->tabBar = mainWindowLayout()->getTabBar();
      self->tabBar->setShape(static_cast<QTabBar::Shape>(tabBarShape));
      self->tabBar->setDrawBase(true);
   }

   bool blocked = tabBar->blockSignals(true);
   bool gap = false;

   const quintptr oldCurrentId = currentTabId();
   int tab_idx = 0;

   for (int i = 0; i < item_list.count(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);

      if (item.skip()) {
         continue;
      }

      if (item.flags & QDockAreaLayoutItem::GapItem) {
         gap = true;
         continue;
      }

      if (item.widgetItem == nullptr) {
         continue;
      }

      QDockWidget *dw = qobject_cast<QDockWidget *>(item.widgetItem->widget());
      QString title = dw->d_func()->fixedWindowTitle;
      quintptr id   = tabId(item);

      if (tab_idx == tabBar->count()) {
         tabBar->insertTab(tab_idx, title);

#ifndef QT_NO_TOOLTIP
         tabBar->setTabToolTip(tab_idx, title);
#endif

         tabBar->setTabData(tab_idx, id);

      } else if (tabBar->tabData(tab_idx).value<quintptr>() != id ) {
         bool doMore = true;

         if (tab_idx + 1 < tabBar->count()) {
            QVariant variant = tabBar->tabData(tab_idx + 1);

            if (variant.value<quintptr>() == id) {
               tabBar->removeTab(tab_idx);
               doMore = false;
            }
         }

         if (doMore) {
            tabBar->insertTab(tab_idx, title);

#ifndef QT_NO_TOOLTIP
            tabBar->setTabToolTip(tab_idx, title);
#endif
            tabBar->setTabData(tab_idx, id);
         }
      }

      if (title != tabBar->tabText(tab_idx)) {
         tabBar->setTabText(tab_idx, title);

#ifndef QT_NO_TOOLTIP
         tabBar->setTabToolTip(tab_idx, title);
#endif

      }

      ++tab_idx;
   }

   while (tab_idx < tabBar->count()) {
      tabBar->removeTab(tab_idx);
   }

   if (oldCurrentId > 0 && currentTabId() != oldCurrentId) {
      self->setCurrentTabId(oldCurrentId);
   }

   if (QDockWidgetGroupWindow *dwgw = qobject_cast<QDockWidgetGroupWindow *>(tabBar->parent())) {
      dwgw->adjustFlags();
   }

   tabBar->blockSignals(blocked);

   //returns if the tabbar is visible or not
   return ( (gap ? 1 : 0) + tabBar->count()) > 1;
}

void QDockAreaLayoutInfo::setTabBarShape(int shape)
{
   if (shape == tabBarShape) {
      return;
   }

   tabBarShape = shape;
   if (tabBar != nullptr) {
      tabBar->setShape(static_cast<QTabBar::Shape>(shape));
   }

   for (int i = 0; i < item_list.count(); ++i) {
      QDockAreaLayoutItem &item = item_list[i];
      if (item.subinfo != nullptr) {
         item.subinfo->setTabBarShape(shape);
      }
   }
}

QSize QDockAreaLayoutInfo::tabBarMinimumSize() const
{
   if (!updateTabBar()) {
      return QSize(0, 0);
   }

   return tabBar->minimumSizeHint();
}

QSize QDockAreaLayoutInfo::tabBarSizeHint() const
{
   if (!updateTabBar()) {
      return QSize(0, 0);
   }

   return tabBar->sizeHint();
}

QSet<QTabBar *> QDockAreaLayoutInfo::usedTabBars() const
{
   QSet<QTabBar *> result;

   if (tabbed) {
      updateTabBar();
      result.insert(tabBar);
   }

   for (int i = 0; i < item_list.count(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);
      if (item.subinfo != nullptr) {
         result += item.subinfo->usedTabBars();
      }
   }

   return result;
}

// returns a set of all used separator widgets for this dockarelayout info
// and all subinfos
QSet<QWidget *> QDockAreaLayoutInfo::usedSeparatorWidgets() const
{
   QSet<QWidget *> result;

   for (int i = 0; i < separatorWidgets.count(); ++i) {
      result << separatorWidgets.at(i);
   }

   for (int i = 0; i < item_list.count(); ++i) {
      const QDockAreaLayoutItem &item = item_list.at(i);
      if (item.subinfo != nullptr) {
         result += item.subinfo->usedSeparatorWidgets();
      }
   }

   return result;
}

QRect QDockAreaLayoutInfo::tabContentRect() const
{
   if (! tabbed) {
      return QRect();
   }

   QRect result = m_dockAreaInfoRect;
   QSize tbh = tabBarSizeHint();

   if (!tbh.isNull()) {
      switch (tabBarShape) {
         case QTabBar::RoundedNorth:
         case QTabBar::TriangularNorth:
            result.adjust(0, tbh.height(), 0, 0);
            break;

         case QTabBar::RoundedSouth:
         case QTabBar::TriangularSouth:
            result.adjust(0, 0, 0, -tbh.height());
            break;

         case QTabBar::RoundedEast:
         case QTabBar::TriangularEast:
            result.adjust(0, 0, -tbh.width(), 0);
            break;

         case QTabBar::RoundedWest:
         case QTabBar::TriangularWest:
            result.adjust(tbh.width(), 0, 0, 0);
            break;

         default:
            break;
      }
   }

   return result;
}

int QDockAreaLayoutInfo::tabIndexToListIndex(int tabIndex) const
{
   Q_ASSERT(tabbed && tabBar);

   quintptr data = (tabBar->tabData(tabIndex)).value<quintptr>();

   for (int i = 0; i < item_list.count(); ++i) {
      if (tabId(item_list.at(i)) == data) {
         return i;
      }
   }
   return -1;
}

void QDockAreaLayoutInfo::moveTab(int from, int to)
{
   item_list.move(tabIndexToListIndex(from), tabIndexToListIndex(to));
}
#endif // QT_NO_TABBAR


QDockAreaLayout::QDockAreaLayout(QMainWindow *win)
   : fallbackToSizeHints(true)
{
   mainWindow = win;
   sep = win->style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent, nullptr, win);

#ifndef QT_NO_TABBAR
   const int tabShape = QTabBar::RoundedSouth;
#else
   const int tabShape = 0;
#endif

   m_docks[QInternal::LeftDock]   = QDockAreaLayoutInfo(&sep, QInternal::LeftDock,   Qt::Vertical,   tabShape, win);
   m_docks[QInternal::RightDock]  = QDockAreaLayoutInfo(&sep, QInternal::RightDock,  Qt::Vertical,   tabShape, win);
   m_docks[QInternal::TopDock]    = QDockAreaLayoutInfo(&sep, QInternal::TopDock,    Qt::Horizontal, tabShape, win);
   m_docks[QInternal::BottomDock] = QDockAreaLayoutInfo(&sep, QInternal::BottomDock, Qt::Horizontal, tabShape, win);

   centralWidgetItem = nullptr;

   corners[Qt::TopLeftCorner]     = Qt::TopDockWidgetArea;
   corners[Qt::TopRightCorner]    = Qt::TopDockWidgetArea;
   corners[Qt::BottomLeftCorner]  = Qt::BottomDockWidgetArea;
   corners[Qt::BottomRightCorner] = Qt::BottomDockWidgetArea;
}

bool QDockAreaLayout::isValid() const
{
   return m_dockAreaRect.isValid();
}

void QDockAreaLayout::saveState(QDataStream &stream) const
{
   stream << (uchar) DockWidgetStateMarker;
   int cnt = 0;

   for (int i = 0; i < QInternal::DockCount; ++i) {
      if (! m_docks[i].item_list.isEmpty()) {
         ++cnt;
      }
   }

   stream << cnt;

   for (int i = 0; i < QInternal::DockCount; ++i) {
      if (m_docks[i].item_list.isEmpty()) {
         continue;
      }

      stream << i << m_docks[i].m_dockAreaInfoRect.size();
      m_docks[i].saveState(stream);
   }

   stream << centralWidgetRect.size();

   for (int i = 0; i < 4; ++i) {
      stream << static_cast<int>(corners[i]);
   }
}

bool QDockAreaLayout::restoreState(QDataStream &stream, const QList<QDockWidget *> &_dockwidgets, bool testing)
{
   QList<QDockWidget *> dockwidgets = _dockwidgets;

   // DockWidgetStateMarker has been consumed already

   int cnt;
   stream >> cnt;

   for (int i = 0; i < cnt; ++i) {
      int pos;
      stream >> pos;

      QSize size;
      stream >> size;

      if (! testing) {
         m_docks[pos].m_dockAreaInfoRect = QRect(QPoint(0, 0), size);
      }

      if (! m_docks[pos].restoreState(stream, dockwidgets, testing)) {
         stream.setStatus(QDataStream::ReadCorruptData);
         return false;
      }
   }

   QSize size;
   stream >> size;
   centralWidgetRect = QRect(QPoint(0, 0), size);

   bool ok = (stream.status() == QDataStream::Ok);

   if (ok) {
      int cornerData[4];
      for (int i = 0; i < 4; ++i) {
         stream >> cornerData[i];
      }

      if (stream.status() == QDataStream::Ok) {
         for (int i = 0; i < 4; ++i) {
            corners[i] = static_cast<Qt::DockWidgetArea>(cornerData[i]);
         }
      }

      if (! testing) {
         fallbackToSizeHints = false;
      }
   }

   return ok;
}

QList<int> QDockAreaLayout::indexOfPlaceHolder(const QString &objectName) const
{
   for (int i = 0; i < QInternal::DockCount; ++i) {
      QList<int> result = m_docks[i].indexOfPlaceHolder(objectName);

      if (! result.isEmpty()) {
         result.prepend(i);
         return result;
      }
   }

   return QList<int>();
}

QList<int> QDockAreaLayout::indexOf(QWidget *dockWidget) const
{
   for (int i = 0; i < QInternal::DockCount; ++i) {
      QList<int> result = m_docks[i].indexOf(dockWidget);

      if (! result.isEmpty()) {
         result.prepend(i);
         return result;
      }
   }

   return QList<int>();
}

QList<int> QDockAreaLayout::gapIndex(const QPoint &pos) const
{
   QMainWindow::DockOptions opts = mainWindow->dockOptions();
   bool nestingEnabled = opts & QMainWindow::AllowNestedDocks;

   QDockAreaLayoutInfo::TabMode tabMode = QDockAreaLayoutInfo::NoTabs;

#ifndef QT_NO_TABBAR
   if (opts & QMainWindow::AllowTabbedDocks || opts & QMainWindow::VerticalTabs) {
      tabMode = QDockAreaLayoutInfo::AllowTabs;
   }

   if (opts & QMainWindow::ForceTabbedDocks) {
      tabMode = QDockAreaLayoutInfo::ForceTabs;
   }

   if (tabMode == QDockAreaLayoutInfo::ForceTabs) {
      nestingEnabled = false;
   }
#endif

   for (int i = 0; i < QInternal::DockCount; ++i) {
      const QDockAreaLayoutInfo &info = m_docks[i];

      if (!info.isEmpty() && info.m_dockAreaInfoRect.contains(pos)) {
         QList<int> result = m_docks[i].gapIndex(pos, nestingEnabled, tabMode);

         if (! result.isEmpty()) {
            result.prepend(i);
         }

         return result;
      }
   }

   for (int i = 0; i < QInternal::DockCount; ++i) {
      const QDockAreaLayoutInfo &info = m_docks[i];

      if (info.isEmpty()) {
         QRect r;

         switch (i) {
            case QInternal::LeftDock:
               r = QRect(m_dockAreaRect.left(), m_dockAreaRect.top(), EmptyDropAreaSize, m_dockAreaRect.height());
               break;

            case QInternal::RightDock:
               r = QRect(m_dockAreaRect.right() - EmptyDropAreaSize, m_dockAreaRect.top(), EmptyDropAreaSize, m_dockAreaRect.height());
               break;

            case QInternal::TopDock:
               r = QRect(m_dockAreaRect.left(), m_dockAreaRect.top(), m_dockAreaRect.width(), EmptyDropAreaSize);
               break;

            case QInternal::BottomDock:
               r = QRect(m_dockAreaRect.left(), m_dockAreaRect.bottom() - EmptyDropAreaSize, m_dockAreaRect.width(), EmptyDropAreaSize);
               break;
         }

         if (r.contains(pos)) {
            if (opts & QMainWindow::ForceTabbedDocks && !info.item_list.isEmpty()) {
               //in case of ForceTabbedDocks, we pass -1 in order to force the gap to be tabbed
               //it mustn't be completely empty otherwise it won't work
               return QList<int>() << i << -1 << 0;

            } else {
               return QList<int>() << i << 0;
            }
         }
      }
   }

   return QList<int>();
}

QList<int> QDockAreaLayout::findSeparator(const QPoint &pos) const
{
   QList<int> result;

   for (int i = 0; i < QInternal::DockCount; ++i) {
      const QDockAreaLayoutInfo &info = m_docks[i];

      if (info.isEmpty()) {
         continue;
      }

      QRect rect = separatorRect(i);

      if (! rect.isNull() && sep == 1) {
         rect.adjust(-2, -2, 2, 2);
      }

      if (rect.contains(pos) && ! info.hasFixedSize()) {
         result << i;
         break;

      } else if (info.m_dockAreaInfoRect.contains(pos)) {
         result = m_docks[i].findSeparator(pos);

         if (! result.isEmpty()) {
            result.prepend(i);
            break;
         }
      }
   }

   return result;
}

QDockAreaLayoutInfo *QDockAreaLayout::info(QWidget *widget)
{
   for (int i = 0; i < QInternal::DockCount; ++i) {
      if (QDockAreaLayoutInfo *result = m_docks[i].info(widget)) {
         return result;
      }
   }

   return nullptr;
}

QDockAreaLayoutInfo *QDockAreaLayout::info(const QList<int> &path)
{
   Q_ASSERT(!path.isEmpty());
   const int index = path.first();

   Q_ASSERT(index >= 0 && index < QInternal::DockCount);

   if (path.count() == 1) {
      return &m_docks[index];
   }

   return m_docks[index].info(path.mid(1));
}

const QDockAreaLayoutInfo *QDockAreaLayout::info(const QList<int> &path) const
{
   return const_cast<QDockAreaLayout *>(this)->info(path);
}

QDockAreaLayoutItem &QDockAreaLayout::item(const QList<int> &path)
{
   Q_ASSERT(! path.isEmpty());

   const int index = path.first();
   Q_ASSERT(index >= 0 && index < QInternal::DockCount);

   return m_docks[index].item(path.mid(1));
}

QRect QDockAreaLayout::itemRect(const QList<int> &path) const
{
   Q_ASSERT(!path.isEmpty());

   const int index = path.first();
   Q_ASSERT(index >= 0 && index < QInternal::DockCount);

   return m_docks[index].itemRect(path.mid(1));
}

QRect QDockAreaLayout::separatorRect(int index) const
{
   const QDockAreaLayoutInfo &dock = m_docks[index];

   if (dock.isEmpty()) {
      return QRect();
   }

   QRect r = dock.m_dockAreaInfoRect;

   switch (index) {
      case QInternal::LeftDock:
         return QRect(r.right() + 1, r.top(), sep, r.height());

      case QInternal::RightDock:
         return QRect(r.left() - sep, r.top(), sep, r.height());

      case QInternal::TopDock:
         return QRect(r.left(), r.bottom() + 1, r.width(), sep);

      case QInternal::BottomDock:
         return QRect(r.left(), r.top() - sep, r.width(), sep);

      default:
         break;
   }
   return QRect();
}

QRect QDockAreaLayout::separatorRect(const QList<int> &path) const
{
   Q_ASSERT(!path.isEmpty());

   const int index = path.first();
   Q_ASSERT(index >= 0 && index < QInternal::DockCount);

   if (path.count() == 1) {
      return separatorRect(index);
   } else {
      return m_docks[index].separatorRect(path.mid(1));
   }
}

bool QDockAreaLayout::insertGap(const QList<int> &path, QLayoutItem *dockWidgetItem)
{
   Q_ASSERT(!path.isEmpty());
   const int index = path.first();

   Q_ASSERT(index >= 0 && index < QInternal::DockCount);

   return m_docks[index].insertGap(path.mid(1), dockWidgetItem);
}

QLayoutItem *QDockAreaLayout::plug(const QList<int> &path)
{
   Q_ASSERT(!path.isEmpty());
   const int index = path.first();

   Q_ASSERT(index >= 0 && index < QInternal::DockCount);

   QLayoutItem *item = m_docks[index].plug(path.mid(1));
   m_docks[index].reparentWidgets(mainWindow);

   return item;
}

QLayoutItem *QDockAreaLayout::unplug(const QList<int> &path)
{
   Q_ASSERT(!path.isEmpty());
   const int index = path.first();

   Q_ASSERT(index >= 0 && index < QInternal::DockCount);

   return m_docks[index].unplug(path.mid(1));
}

void QDockAreaLayout::remove(const QList<int> &path)
{
   Q_ASSERT(!path.isEmpty());
   const int index = path.first();

   Q_ASSERT(index >= 0 && index < QInternal::DockCount);

   m_docks[index].remove(path.mid(1));
}

void QDockAreaLayout::removePlaceHolder(const QString &name)
{
   QList<int> index = indexOfPlaceHolder(name);

   if (! index.isEmpty()) {
      remove(index);
   }

   auto children = mainWindow->findChildren<QDockWidgetGroupWindow *>(QString(), Qt::FindDirectChildrenOnly);

   for (QDockWidgetGroupWindow *dwgw : children) {
      index = dwgw->layoutInfo()->indexOfPlaceHolder(name);

      if (! index.isEmpty()) {
         dwgw->layoutInfo()->remove(index);
         dwgw->destroyOrHideIfEmpty();
      }
   }
}

static inline int qMax(int i1, int i2, int i3)
{
   return qMax(i1, qMax(i2, i3));
}

void QDockAreaLayout::getGrid(QVector<QLayoutStruct> *_ver_struct_list, QVector<QLayoutStruct> *_hor_struct_list)
{
   QSize center_hint(0, 0);
   QSize center_min(0, 0);
   QSize center_max(0, 0);

   const bool have_central = (centralWidgetItem != nullptr) && !centralWidgetItem->isEmpty();

   if (have_central) {
      center_hint = centralWidgetRect.size();
      if (!center_hint.isValid()) {
         center_hint = centralWidgetItem->sizeHint();
      }

      center_min = centralWidgetItem->minimumSize();
      center_max = centralWidgetItem->maximumSize();
   }

   QRect center_rect = m_dockAreaRect;

   if (! m_docks[QInternal::LeftDock].isEmpty()) {
      center_rect.setLeft(m_dockAreaRect.left() + m_docks[QInternal::LeftDock].m_dockAreaInfoRect.width() + sep);
   }

   if (! m_docks[QInternal::TopDock].isEmpty()) {
      center_rect.setTop(m_dockAreaRect.top() + m_docks[QInternal::TopDock].m_dockAreaInfoRect.height() + sep);
   }

   if (! m_docks[QInternal::RightDock].isEmpty()) {
      center_rect.setRight(m_dockAreaRect.right() - m_docks[QInternal::RightDock].m_dockAreaInfoRect.width() - sep);
   }

   if (! m_docks[QInternal::BottomDock].isEmpty()) {
      center_rect.setBottom(m_dockAreaRect.bottom() - m_docks[QInternal::BottomDock].m_dockAreaInfoRect.height() - sep);
   }

   QSize left_hint = m_docks[QInternal::LeftDock].size();
   if (left_hint.isNull() || fallbackToSizeHints) {
      left_hint = m_docks[QInternal::LeftDock].sizeHint();
   }

   QSize left_min = m_docks[QInternal::LeftDock].minimumSize();
   QSize left_max = m_docks[QInternal::LeftDock].maximumSize();
   left_hint = left_hint.boundedTo(left_max).expandedTo(left_min);

   QSize right_hint = m_docks[QInternal::RightDock].size();
   if (right_hint.isNull() || fallbackToSizeHints) {
      right_hint = m_docks[QInternal::RightDock].sizeHint();
   }

   QSize right_min = m_docks[QInternal::RightDock].minimumSize();
   QSize right_max = m_docks[QInternal::RightDock].maximumSize();
   right_hint = right_hint.boundedTo(right_max).expandedTo(right_min);

   QSize top_hint = m_docks[QInternal::TopDock].size();
   if (top_hint.isNull() || fallbackToSizeHints) {
      top_hint = m_docks[QInternal::TopDock].sizeHint();
   }

   QSize top_min = m_docks[QInternal::TopDock].minimumSize();
   QSize top_max = m_docks[QInternal::TopDock].maximumSize();
   top_hint = top_hint.boundedTo(top_max).expandedTo(top_min);

   QSize bottom_hint = m_docks[QInternal::BottomDock].size();
   if (bottom_hint.isNull() || fallbackToSizeHints) {
      bottom_hint = m_docks[QInternal::BottomDock].sizeHint();
   }

   QSize bottom_min = m_docks[QInternal::BottomDock].minimumSize();
   QSize bottom_max = m_docks[QInternal::BottomDock].maximumSize();
   bottom_hint = bottom_hint.boundedTo(bottom_max).expandedTo(bottom_min);

   if (_ver_struct_list != nullptr) {
      QVector<QLayoutStruct> &ver_struct_list = *_ver_struct_list;
      ver_struct_list.resize(3);

      // top --------------------------------------------------
      ver_struct_list[0].init();
      ver_struct_list[0].stretch = 0;
      ver_struct_list[0].sizeHint = top_hint.height();
      ver_struct_list[0].minimumSize = top_min.height();
      ver_struct_list[0].maximumSize = top_max.height();
      ver_struct_list[0].expansive = false;
      ver_struct_list[0].empty = m_docks[QInternal::TopDock].isEmpty();
      ver_struct_list[0].pos   = m_docks[QInternal::TopDock].m_dockAreaInfoRect.top();
      ver_struct_list[0].size  = m_docks[QInternal::TopDock].m_dockAreaInfoRect.height();

      // center --------------------------------------------------
      ver_struct_list[1].init();
      ver_struct_list[1].stretch = center_hint.height();

      bool tl_significant = corners[Qt::TopLeftCorner] == Qt::TopDockWidgetArea
         || m_docks[QInternal::TopDock].isEmpty();

      bool bl_significant = corners[Qt::BottomLeftCorner] == Qt::BottomDockWidgetArea
         || m_docks[QInternal::BottomDock].isEmpty();

      bool tr_significant = corners[Qt::TopRightCorner] == Qt::TopDockWidgetArea
         || m_docks[QInternal::TopDock].isEmpty();

      bool br_significant = corners[Qt::BottomRightCorner] == Qt::BottomDockWidgetArea
         || m_docks[QInternal::BottomDock].isEmpty();

      int left  = (tl_significant && bl_significant) ? left_hint.height() : 0;
      int right = (tr_significant && br_significant) ? right_hint.height() : 0;
      ver_struct_list[1].sizeHint = qMax(left, center_hint.height(), right);

      left  = (tl_significant && bl_significant) ? left_min.height() : 0;
      right = (tr_significant && br_significant) ? right_min.height() : 0;
      ver_struct_list[1].minimumSize = qMax(left, center_min.height(), right);
      ver_struct_list[1].maximumSize = center_max.height();

      ver_struct_list[1].expansive = have_central;
      ver_struct_list[1].empty = m_docks[QInternal::LeftDock].isEmpty()
            && ! have_central && m_docks[QInternal::RightDock].isEmpty();

      ver_struct_list[1].pos  = center_rect.top();
      ver_struct_list[1].size = center_rect.height();

      // bottom --------------------------------------------------
      ver_struct_list[2].init();
      ver_struct_list[2].stretch = 0;
      ver_struct_list[2].sizeHint = bottom_hint.height();
      ver_struct_list[2].minimumSize = bottom_min.height();
      ver_struct_list[2].maximumSize = bottom_max.height();
      ver_struct_list[2].expansive = false;
      ver_struct_list[2].empty = m_docks[QInternal::BottomDock].isEmpty();
      ver_struct_list[2].pos   = m_docks[QInternal::BottomDock].m_dockAreaInfoRect.top();
      ver_struct_list[2].size  = m_docks[QInternal::BottomDock].m_dockAreaInfoRect.height();

      for (int i = 0; i < 3; ++i) {
         ver_struct_list[i].sizeHint
            = qMax(ver_struct_list[i].sizeHint, ver_struct_list[i].minimumSize);
      }

      if (have_central && ver_struct_list[0].empty && ver_struct_list[2].empty) {
         ver_struct_list[1].maximumSize = QWIDGETSIZE_MAX;
      }
   }

   if (_hor_struct_list != nullptr) {
      QVector<QLayoutStruct> &hor_struct_list = *_hor_struct_list;
      hor_struct_list.resize(3);

      // left --------------------------------------------------
      hor_struct_list[0].init();
      hor_struct_list[0].stretch = 0;
      hor_struct_list[0].sizeHint = left_hint.width();
      hor_struct_list[0].minimumSize = left_min.width();
      hor_struct_list[0].maximumSize = left_max.width();
      hor_struct_list[0].expansive = false;
      hor_struct_list[0].empty = m_docks[QInternal::LeftDock].isEmpty();
      hor_struct_list[0].pos   = m_docks[QInternal::LeftDock].m_dockAreaInfoRect.left();
      hor_struct_list[0].size  = m_docks[QInternal::LeftDock].m_dockAreaInfoRect.width();

      // center --------------------------------------------------
      hor_struct_list[1].init();
      hor_struct_list[1].stretch = center_hint.width();

      bool tl_significant = corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea
         || m_docks[QInternal::LeftDock].isEmpty();
      bool tr_significant = corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea
         || m_docks[QInternal::RightDock].isEmpty();
      bool bl_significant = corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea
         || m_docks[QInternal::LeftDock].isEmpty();
      bool br_significant = corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea
         || m_docks[QInternal::RightDock].isEmpty();

      int top = (tl_significant && tr_significant) ? top_hint.width() : 0;
      int bottom = (bl_significant && br_significant) ? bottom_hint.width() : 0;
      hor_struct_list[1].sizeHint = qMax(top, center_hint.width(), bottom);

      top = (tl_significant && tr_significant) ? top_min.width() : 0;
      bottom = (bl_significant && br_significant) ? bottom_min.width() : 0;
      hor_struct_list[1].minimumSize = qMax(top, center_min.width(), bottom);

      hor_struct_list[1].maximumSize = center_max.width();
      hor_struct_list[1].expansive = have_central;
      hor_struct_list[1].empty = ! have_central;
      hor_struct_list[1].pos   = center_rect.left();
      hor_struct_list[1].size  = center_rect.width();

      // right --------------------------------------------------
      hor_struct_list[2].init();
      hor_struct_list[2].stretch = 0;
      hor_struct_list[2].sizeHint = right_hint.width();
      hor_struct_list[2].minimumSize = right_min.width();
      hor_struct_list[2].maximumSize = right_max.width();
      hor_struct_list[2].expansive = false;
      hor_struct_list[2].empty = m_docks[QInternal::RightDock].isEmpty();
      hor_struct_list[2].pos   = m_docks[QInternal::RightDock].m_dockAreaInfoRect.left();
      hor_struct_list[2].size  = m_docks[QInternal::RightDock].m_dockAreaInfoRect.width();

      for (int i = 0; i < 3; ++i) {
         hor_struct_list[i].sizeHint = qMax(hor_struct_list[i].sizeHint, hor_struct_list[i].minimumSize);
      }

      if (have_central && hor_struct_list[0].empty && hor_struct_list[2].empty) {
         hor_struct_list[1].maximumSize = QWIDGETSIZE_MAX;
      }
   }
}

void QDockAreaLayout::setGrid(QVector<QLayoutStruct> *ver_struct_list, QVector<QLayoutStruct> *hor_struct_list)
{
   // top ---------------------------------------------------

   if (! m_docks[QInternal::TopDock].isEmpty()) {
      QRect r = m_docks[QInternal::TopDock].m_dockAreaInfoRect;

      if (hor_struct_list != nullptr) {
         r.setLeft(corners[Qt::TopLeftCorner] == Qt::TopDockWidgetArea
            || m_docks[QInternal::LeftDock].isEmpty() ? m_dockAreaRect.left() : hor_struct_list->at(1).pos);

         r.setRight(corners[Qt::TopRightCorner] == Qt::TopDockWidgetArea
            || m_docks[QInternal::RightDock].isEmpty() ? m_dockAreaRect.right() : hor_struct_list->at(2).pos - sep - 1);
      }

      if (ver_struct_list != nullptr) {
         r.setTop(m_dockAreaRect.top());
         r.setBottom(ver_struct_list->at(1).pos - sep - 1);
      }

      m_docks[QInternal::TopDock].m_dockAreaInfoRect = r;
      m_docks[QInternal::TopDock].fitItems();
   }

   // bottom ---------------------------------------------------

   if (! m_docks[QInternal::BottomDock].isEmpty()) {
      QRect r = m_docks[QInternal::BottomDock].m_dockAreaInfoRect;

      if (hor_struct_list != nullptr) {
         r.setLeft(corners[Qt::BottomLeftCorner] == Qt::BottomDockWidgetArea
            || m_docks[QInternal::LeftDock].isEmpty() ? m_dockAreaRect.left() : hor_struct_list->at(1).pos);

         r.setRight(corners[Qt::BottomRightCorner] == Qt::BottomDockWidgetArea
            || m_docks[QInternal::RightDock].isEmpty() ? m_dockAreaRect.right() : hor_struct_list->at(2).pos - sep - 1);
      }

      if (ver_struct_list != nullptr) {
         r.setTop(ver_struct_list->at(2).pos);
         r.setBottom(m_dockAreaRect.bottom());
      }

      m_docks[QInternal::BottomDock].m_dockAreaInfoRect = r;
      m_docks[QInternal::BottomDock].fitItems();
   }

   // left ---------------------------------------------------

   if (! m_docks[QInternal::LeftDock].isEmpty()) {
      QRect r = m_docks[QInternal::LeftDock].m_dockAreaInfoRect;

      if (hor_struct_list != nullptr) {
         r.setLeft(m_dockAreaRect.left());
         r.setRight(hor_struct_list->at(1).pos - sep - 1);
      }

      if (ver_struct_list != nullptr) {
         r.setTop(corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea
            || m_docks[QInternal::TopDock].isEmpty() ? m_dockAreaRect.top() : ver_struct_list->at(1).pos);

         r.setBottom(corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea
            || m_docks[QInternal::BottomDock].isEmpty() ? m_dockAreaRect.bottom() : ver_struct_list->at(2).pos - sep - 1);
      }

      m_docks[QInternal::LeftDock].m_dockAreaInfoRect = r;
      m_docks[QInternal::LeftDock].fitItems();
   }

   // right ---------------------------------------------------

   if (! m_docks[QInternal::RightDock].isEmpty()) {
      QRect r = m_docks[QInternal::RightDock].m_dockAreaInfoRect;

      if (hor_struct_list != nullptr) {
         r.setLeft(hor_struct_list->at(2).pos);
         r.setRight(m_dockAreaRect.right());
      }

      if (ver_struct_list != nullptr) {
         r.setTop(corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea
            || m_docks[QInternal::TopDock].isEmpty() ? m_dockAreaRect.top() : ver_struct_list->at(1).pos);

         r.setBottom(corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea
            || m_docks[QInternal::BottomDock].isEmpty() ? m_dockAreaRect.bottom() : ver_struct_list->at(2).pos - sep - 1);
      }

      m_docks[QInternal::RightDock].m_dockAreaInfoRect = r;
      m_docks[QInternal::RightDock].fitItems();
   }

   // center ---------------------------------------------------

   if (hor_struct_list != nullptr) {
      centralWidgetRect.setLeft(hor_struct_list->at(1).pos);
      centralWidgetRect.setWidth(hor_struct_list->at(1).size);
   }

   if (ver_struct_list != nullptr) {
      centralWidgetRect.setTop(ver_struct_list->at(1).pos);
      centralWidgetRect.setHeight(ver_struct_list->at(1).size);
   }
}

void QDockAreaLayout::fitLayout()
{
   QVector<QLayoutStruct> ver_struct_list(3);
   QVector<QLayoutStruct> hor_struct_list(3);
   getGrid(&ver_struct_list, &hor_struct_list);

   qGeomCalc(ver_struct_list, 0, 3, m_dockAreaRect.top(), m_dockAreaRect.height(), sep);
   qGeomCalc(hor_struct_list, 0, 3, m_dockAreaRect.left(), m_dockAreaRect.width(), sep);

   setGrid(&ver_struct_list, &hor_struct_list);
}

void QDockAreaLayout::clear()
{
   for (int i = 0; i < QInternal::DockCount; ++i) {
      m_docks[i].clear();
   }

   m_dockAreaRect = QRect();
   centralWidgetRect = QRect();
}

QSize QDockAreaLayout::sizeHint() const
{
   int left_sep   = 0;
   int right_sep  = 0;
   int top_sep    = 0;
   int bottom_dockAreaSep = 0;

   if (centralWidgetItem != nullptr) {
      left_sep   = m_docks[QInternal::LeftDock].isEmpty() ? 0 : sep;
      right_sep  = m_docks[QInternal::RightDock].isEmpty() ? 0 : sep;
      top_sep    = m_docks[QInternal::TopDock].isEmpty() ? 0 : sep;
      bottom_dockAreaSep = m_docks[QInternal::BottomDock].isEmpty() ? 0 : sep;
   }

   QSize left    = m_docks[QInternal::LeftDock].sizeHint() + QSize(left_sep, 0);
   QSize right   = m_docks[QInternal::RightDock].sizeHint() + QSize(right_sep, 0);
   QSize top     = m_docks[QInternal::TopDock].sizeHint() + QSize(0, top_sep);
   QSize bottom  = m_docks[QInternal::BottomDock].sizeHint() + QSize(0, bottom_dockAreaSep);
   QSize center  = (centralWidgetItem == nullptr) ? QSize(0, 0) : centralWidgetItem->sizeHint();

   int row1 = top.width();
   int row2 = left.width() + center.width() + right.width();
   int row3 = bottom.width();
   int col1 = left.height();
   int col2 = top.height() + center.height() + bottom.height();
   int col3 = right.height();

   if (corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea) {
      row1 += left.width();
   } else {
      col1 += top.height();
   }

   if (corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea) {
      row1 += right.width();
   } else {
      col3 += top.height();
   }

   if (corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea) {
      row3 += left.width();
   } else {
      col1 += bottom.height();
   }

   if (corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea) {
      row3 += right.width();
   } else {
      col3 += bottom.height();
   }

   return QSize(qMax(row1, row2, row3), qMax(col1, col2, col3));
}

QSize QDockAreaLayout::minimumSize() const
{
   int left_sep   = 0;
   int right_sep  = 0;
   int top_sep    = 0;
   int bottom_dockAreaSep = 0;

   if (centralWidgetItem != nullptr) {
      left_sep   = m_docks[QInternal::LeftDock].isEmpty() ? 0 : sep;
      right_sep  = m_docks[QInternal::RightDock].isEmpty() ? 0 : sep;
      top_sep    = m_docks[QInternal::TopDock].isEmpty() ? 0 : sep;
      bottom_dockAreaSep = m_docks[QInternal::BottomDock].isEmpty() ? 0 : sep;
   }

   QSize left    = m_docks[QInternal::LeftDock].minimumSize() + QSize(left_sep, 0);
   QSize right   = m_docks[QInternal::RightDock].minimumSize() + QSize(right_sep, 0);
   QSize top     = m_docks[QInternal::TopDock].minimumSize() + QSize(0, top_sep);
   QSize bottom  = m_docks[QInternal::BottomDock].minimumSize() + QSize(0, bottom_dockAreaSep);
   QSize center  = (centralWidgetItem == nullptr) ? QSize(0, 0) : centralWidgetItem->minimumSize();

   int row1 = top.width();
   int row2 = left.width() + center.width() + right.width();
   int row3 = bottom.width();
   int col1 = left.height();
   int col2 = top.height() + center.height() + bottom.height();
   int col3 = right.height();

   if (corners[Qt::TopLeftCorner] == Qt::LeftDockWidgetArea) {
      row1 += left.width();
   } else {
      col1 += top.height();
   }

   if (corners[Qt::TopRightCorner] == Qt::RightDockWidgetArea) {
      row1 += right.width();
   } else {
      col3 += top.height();
   }

   if (corners[Qt::BottomLeftCorner] == Qt::LeftDockWidgetArea) {
      row3 += left.width();
   } else {
      col1 += bottom.height();
   }

   if (corners[Qt::BottomRightCorner] == Qt::RightDockWidgetArea) {
      row3 += right.width();
   } else {
      col3 += bottom.height();
   }

   return QSize(qMax(row1, row2, row3), qMax(col1, col2, col3));
}

QRect QDockAreaLayout::constrainedRect(QRect rect, QWidget *widget)
{
   QRect desktop;
   QDesktopWidget *desktopW = QApplication::desktop();
   if (desktopW->isVirtualDesktop()) {
      desktop = desktopW->screenGeometry(rect.topLeft());
   } else {
      desktop = desktopW->screenGeometry(widget);
   }

   if (desktop.isValid()) {
      rect.setWidth(qMin(rect.width(), desktop.width()));
      rect.setHeight(qMin(rect.height(), desktop.height()));
      rect.moveLeft(qMax(rect.left(), desktop.left()));
      rect.moveTop(qMax(rect.top(), desktop.top()));
      rect.moveRight(qMin(rect.right(), desktop.right()));
      rect.moveBottom(qMin(rect.bottom(), desktop.bottom()));
   }

   return rect;
}

bool QDockAreaLayout::restoreDockWidget(QDockWidget *dockWidget)
{
   QDockAreaLayoutItem *item = nullptr;

   auto children = mainWindow->findChildren<QDockWidgetGroupWindow *>(QString(), Qt::FindDirectChildrenOnly);

   for (QDockWidgetGroupWindow *dwgw : children) {
      QList<int> index = dwgw->layoutInfo()->indexOfPlaceHolder(dockWidget->objectName());

      if (! index.isEmpty()) {
         dockWidget->setParent(dwgw);
         item = const_cast<QDockAreaLayoutItem *>(&dwgw->layoutInfo()->item(index));
         break;
      }
   }

   if (! item) {
      QList<int> index = indexOfPlaceHolder(dockWidget->objectName());
      if (index.isEmpty()) {
         return false;
      }
      item = const_cast<QDockAreaLayoutItem *>(&this->item(index));
   }

   QPlaceHolderItem *placeHolder = item->placeHolderItem;
   Q_ASSERT(placeHolder != nullptr);

   item->widgetItem = new QDockWidgetItem(dockWidget);

   if (placeHolder->window) {

      const QRect r = constrainedRect(placeHolder->topLevelRect, dockWidget);
      dockWidget->d_func()->setWindowState(true, true, r);
   }
   dockWidget->setVisible(!placeHolder->hidden);

   item->placeHolderItem = nullptr;
   delete placeHolder;

   return true;
}

void QDockAreaLayout::addDockWidget(QInternal::DockPosition pos, QDockWidget *dockWidget,
   Qt::Orientation orientation)
{
   QLayoutItem *dockWidgetItem = new QDockWidgetItem(dockWidget);
   QDockAreaLayoutInfo &info = m_docks[pos];

   if (orientation == info.m_dockAreaOrientation  || info.item_list.count() <= 1) {
      // empty dock areas, or dock areas containing exactly one widget can have their orientation switched
      info.m_dockAreaOrientation = orientation;

      QDockAreaLayoutItem new_item(dockWidgetItem);
      info.item_list.append(new_item);

#ifndef QT_NO_TABBAR
      if (info.tabbed && !new_item.skip()) {
         info.updateTabBar();
         info.setCurrentTabId(tabId(new_item));
      }
#endif

   } else {

#ifndef QT_NO_TABBAR
      int tbshape = info.tabBarShape;
#else
      int tbshape = 0;
#endif

      QDockAreaLayoutInfo new_info(&sep, pos, orientation, tbshape, mainWindow);
      new_info.item_list.append(new QDockAreaLayoutInfo(info));
      new_info.item_list.append(dockWidgetItem);
      info = new_info;
   }

   removePlaceHolder(dockWidget->objectName());
}

void QDockAreaLayout::tabifyDockWidget(QDockWidget *first, QDockWidget *second)
{
   QList<int> path = indexOf(first);
   if (path.isEmpty()) {
      return;
   }

   QDockAreaLayoutInfo *info = this->info(path);
   Q_ASSERT(info != nullptr);
   info->tab(path.last(), new QDockWidgetItem(second));

   removePlaceHolder(second->objectName());
}

void QDockAreaLayout::resizeDocks(const QList<QDockWidget *> &docks, const QList<int> &sizes, Qt::Orientation newOrientation)
{
   if (docks.count() != sizes.count()) {
      qWarning("QDockAreaLayout::resizeDocks() Sizes are not the same");
      return;
   }

   int count = docks.count();
   fallbackToSizeHints = false;

   for (int i = 0; i < count; ++i) {
      QList<int> path = indexOf(docks[i]);

      if (path.isEmpty()) {
         qWarning("QDockAreaLayout::resizeDocks() QDockWidget is not part of this layout");
         continue;
      }

      int size = sizes[i];

      if (size <= 0) {
         qWarning("QDockAreaLayout::resizeDocks() Sizes need to be larger than 0");
         size = 1;
      }

      while (path.size() > 1) {
         QDockAreaLayoutInfo *info = this->info(path);

         if (! info->tabbed && info->m_dockAreaOrientation  == newOrientation) {
            info->item_list[path.last()].size = size;
            int totalSize = 0;

            for (const QDockAreaLayoutItem &item : info->item_list) {
               if (! item.skip()) {
                  if (totalSize != 0) {
                     totalSize += sep;
                  }

                  totalSize += item.size == -1 ? pick(newOrientation, item.sizeHint()) : item.size;
               }
            }
            size = totalSize;
         }

         path.removeLast();
      }

      const int dockNum = path.first();
      Q_ASSERT(dockNum < QInternal::DockCount);

      QRect &r = m_docks[dockNum].m_dockAreaInfoRect;

      QSize s = r.size();
      rpick(newOrientation, s) = size;
      r.setSize(s);
   }
}

void QDockAreaLayout::splitDockWidget(QDockWidget *after, QDockWidget *dockWidget, Qt::Orientation orientation)
{
   QList<int> path = indexOf(after);
   if (path.isEmpty()) {
      return;
   }

   QDockAreaLayoutInfo *info = this->info(path);
   Q_ASSERT(info != nullptr);
   info->split(path.last(), orientation, new QDockWidgetItem(dockWidget));

   removePlaceHolder(dockWidget->objectName());
}

void QDockAreaLayout::apply(bool animate)
{
   QWidgetAnimator &widgetAnimator = qt_mainwindow_layout(mainWindow)->widgetAnimator;

   for (int i = 0; i < QInternal::DockCount; ++i) {
      m_docks[i].apply(animate);
   }

   if (centralWidgetItem != nullptr && !centralWidgetItem->isEmpty()) {
      widgetAnimator.animate(centralWidgetItem->widget(), centralWidgetRect, animate);
   }

#ifndef QT_NO_TABBAR
   if (sep == 1) {
      updateSeparatorWidgets();
   }
#endif
}

void QDockAreaLayout::paintSeparators(QPainter *p, QWidget *widget, const QRegion &clip, const QPoint &mouse) const
{
   for (int i = 0; i < QInternal::DockCount; ++i) {
      const QDockAreaLayoutInfo &dock = m_docks[i];

      if (dock.isEmpty()) {
         continue;
      }

      QRect r = separatorRect(i);

      if (clip.contains(r) && !dock.hasFixedSize()) {
         Qt::Orientation opposite = dock.m_dockAreaOrientation  == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal;
         paintSep(p, widget, r, opposite, r.contains(mouse));
      }

      if (clip.contains(dock.m_dockAreaInfoRect)) {
         dock.paintSeparators(p, widget, clip, mouse);
      }
   }
}

QRegion QDockAreaLayout::separatorRegion() const
{
   QRegion result;

   for (int i = 0; i < QInternal::DockCount; ++i) {
      const QDockAreaLayoutInfo &dock = m_docks[i];

      if (dock.isEmpty()) {
         continue;
      }

      result |= separatorRect(i);
      result |= dock.separatorRegion();
   }

   return result;
}

int QDockAreaLayout::separatorMove(const QList<int> &separator, const QPoint &origin, const QPoint &dest)
{
   int delta = 0;
   int index = separator.last();

   if (separator.count() > 1) {
      QDockAreaLayoutInfo *info = this->info(separator);
      delta = pick(info->m_dockAreaOrientation, dest - origin);

      if (delta != 0) {
         delta = info->separatorMove(index, delta);
      }

      info->apply(false);

      return delta;
   }

   QVector<QLayoutStruct> list;

   if (index == QInternal::LeftDock || index == QInternal::RightDock) {
      getGrid(nullptr, &list);
   } else {
      getGrid(&list, nullptr);
   }

   int sep_index = (index == QInternal::LeftDock || index == QInternal::TopDock) ? 0 : 1;

   Qt::Orientation newOrientation = (index == QInternal::LeftDock || index == QInternal::RightDock)
         ? Qt::Horizontal : Qt::Vertical;

   delta = pick(newOrientation, dest - origin);
   delta = separatorMoveHelper(list, sep_index, delta, sep);

   if (index == QInternal::LeftDock || index == QInternal::RightDock) {
      setGrid(nullptr, &list);
   } else {
      setGrid(&list, nullptr);
   }

   apply(false);

   return delta;
}

#ifndef QT_NO_TABBAR

// Sets the correct positions for the separator widgets
// Allocates new sepearator widgets with getSeparatorWidget

void QDockAreaLayout::updateSeparatorWidgets() const
{
   int j = 0;

   for (int i = 0; i < QInternal::DockCount; ++i) {
      const QDockAreaLayoutInfo &dock = m_docks[i];
      if (dock.isEmpty()) {
         continue;
      }

      QWidget *sepWidget;
      if (j < separatorWidgets.size()) {
         sepWidget = separatorWidgets.at(j);
      } else {
         sepWidget = qt_mainwindow_layout(mainWindow)->getSeparatorWidget();
         separatorWidgets.append(sepWidget);
      }

      j++;

      sepWidget->raise();

      QRect sepRect = separatorRect(i).adjusted(-2, -2, 2, 2);
      sepWidget->setGeometry(sepRect);
      sepWidget->setMask( QRegion(separatorRect(i).translated( - sepRect.topLeft())));
      sepWidget->show();
   }

   for (int i = j; i < separatorWidgets.size(); ++i) {
      separatorWidgets.at(i)->hide();
   }

   separatorWidgets.resize(j);
}
#endif

QLayoutItem *QDockAreaLayout::itemAt(int *x, int index) const
{
   Q_ASSERT(x != nullptr);

   for (int i = 0; i < QInternal::DockCount; ++i) {
      const QDockAreaLayoutInfo &dock = m_docks[i];
      if (QLayoutItem *ret = dock.itemAt(x, index)) {
         return ret;
      }
   }

   if (centralWidgetItem && (*x)++ == index) {
      return centralWidgetItem;
   }

   return nullptr;
}

QLayoutItem *QDockAreaLayout::takeAt(int *x, int index)
{
   Q_ASSERT(x != nullptr);

   for (int i = 0; i < QInternal::DockCount; ++i) {
      QDockAreaLayoutInfo &dock = m_docks[i];
      if (QLayoutItem *ret = dock.takeAt(x, index)) {
         return ret;
      }
   }

   if (centralWidgetItem && (*x)++ == index) {
      QLayoutItem *ret  = centralWidgetItem;
      centralWidgetItem = nullptr;
      return ret;
   }

   return nullptr;
}

void QDockAreaLayout::deleteAllLayoutItems()
{
   for (int i = 0; i < QInternal::DockCount; ++i) {
      m_docks[i].deleteAllLayoutItems();
   }
}

#ifndef QT_NO_TABBAR
QSet<QTabBar *> QDockAreaLayout::usedTabBars() const
{
   QSet<QTabBar *> result;

   for (int i = 0; i < QInternal::DockCount; ++i) {
      const QDockAreaLayoutInfo &dock = m_docks[i];
      result += dock.usedTabBars();
   }

   return result;
}

// Returns the set of all used separator widgets
QSet<QWidget *> QDockAreaLayout::usedSeparatorWidgets() const
{
   QSet<QWidget *> result;

   for (int i = 0; i < separatorWidgets.count(); ++i) {
      result << separatorWidgets.at(i);
   }

   for (int i = 0; i < QInternal::DockCount; ++i) {
      const QDockAreaLayoutInfo &dock = m_docks[i];
      result += dock.usedSeparatorWidgets();
   }

   return result;
}
#endif

QRect QDockAreaLayout::gapRect(const QList<int> &path) const
{
   const QDockAreaLayoutInfo *info = this->info(path);
   if (info == nullptr) {
      return QRect();
   }

   const QList<QDockAreaLayoutItem> &item_list = info->item_list;

   Qt::Orientation newOrientation = info->m_dockAreaOrientation;

   int index = path.last();

   if (index < 0 || index >= item_list.count()) {
      return QRect();
   }

   const QDockAreaLayoutItem &item = item_list.at(index);

   if (! (item.flags & QDockAreaLayoutItem::GapItem)) {
      return QRect();
   }

   QRect result;

#ifndef QT_NO_TABBAR
   if (info->tabbed) {
      result = info->tabContentRect();
   } else
#endif

   {
      int pos = item.pos;
      int size = item.size;

      int prev = info->prev(index);
      int next = info->next(index);

      if (prev != -1 && !(item_list.at(prev).flags & QDockAreaLayoutItem::GapItem)) {
         pos += sep;
         size -= sep;
      }
      if (next != -1 && !(item_list.at(next).flags & QDockAreaLayoutItem::GapItem)) {
         size -= sep;
      }

      QPoint p;
      rpick(newOrientation, p) = pos;
      rperp(newOrientation, p) = perp(newOrientation, info->m_dockAreaInfoRect.topLeft());

      QSize s;
      rpick(newOrientation, s) = size;
      rperp(newOrientation, s) = perp(newOrientation, info->m_dockAreaInfoRect.size());

      result = QRect(p, s);
   }

   return result;
}

void QDockAreaLayout::keepSize(QDockWidget *w)
{
   QList<int> path = indexOf(w);

   if (path.isEmpty()) {
      return;
   }

   QDockAreaLayoutItem &item = this->item(path);
   if (item.size != -1) {
      item.flags |= QDockAreaLayoutItem::KeepSize;
   }
}

void QDockAreaLayout::styleChangedEvent()
{
   sep = mainWindow->style()->pixelMetric(QStyle::PM_DockWidgetSeparatorExtent, nullptr, mainWindow);
   if (isValid()) {
      fitLayout();
   }
}

#endif // QT_NO_DOCKWIDGET
