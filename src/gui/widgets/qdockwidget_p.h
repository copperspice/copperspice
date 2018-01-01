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

#ifndef QDockWidget_P_H
#define QDockWidget_P_H

#include <QtGui/qstyleoption.h>
#include <qwidget_p.h>
#include <QtGui/qboxlayout.h>
#include <QtGui/qdockwidget.h>

#ifndef QT_NO_DOCKWIDGET

QT_BEGIN_NAMESPACE

class QGridLayout;
class QWidgetResizeHandler;
class QRubberBand;
class QDockWidgetTitleButton;
class QSpacerItem;
class QDockWidgetItem;

class QDockWidgetPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QDockWidget)

   struct DragState {
      QPoint pressPos;
      bool dragging;
      QLayoutItem *widgetItem;
      bool ownWidgetItem;
      bool nca;
      bool ctrlDrag;
   };

 public:
   inline QDockWidgetPrivate()
      : QWidgetPrivate(), state(0),
        features(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable 
                 | QDockWidget::DockWidgetFloatable),
        allowedAreas(Qt::AllDockWidgetAreas) {
   }

   void init();
   void _q_toggleView(bool);
   void _q_toggleTopLevel();

   void updateButtons();
   DragState *state;

   QDockWidget::DockWidgetFeatures features;
   Qt::DockWidgetAreas allowedAreas;

   QWidgetResizeHandler *resizer;

#ifndef QT_NO_ACTION
   QAction *toggleViewAction;
#endif

   //    QMainWindow *findMainWindow(QWidget *widget) const;
   QRect undockedGeometry;
   QString fixedWindowTitle;

   bool mousePressEvent(QMouseEvent *event);
   bool mouseDoubleClickEvent(QMouseEvent *event);
   bool mouseMoveEvent(QMouseEvent *event);
   bool mouseReleaseEvent(QMouseEvent *event);
   void setWindowState(bool floating, bool unplug = false, const QRect &rect = QRect());
   void nonClientAreaMouseEvent(QMouseEvent *event);
   void initDrag(const QPoint &pos, bool nca);
   void startDrag();
   void endDrag(bool abort = false);
   void moveEvent(QMoveEvent *event);

   void unplug(const QRect &rect);
   void plug(const QRect &rect);

   bool isAnimating() const;
};

class Q_GUI_EXPORT QDockWidgetLayout : public QLayout
{
   GUI_CS_OBJECT(QDockWidgetLayout)

 public:
   QDockWidgetLayout(QWidget *parent = nullptr);
   ~QDockWidgetLayout();

   void addItem(QLayoutItem *item)override;
   QLayoutItem *itemAt(int index) const override;
   QLayoutItem *takeAt(int index) override;
   int count() const override;

   QSize maximumSize() const override;
   QSize minimumSize() const override;
   QSize sizeHint() const override;

   QSize sizeFromContent(const QSize &content, bool floating) const;

   void setGeometry(const QRect &r) override;

   enum Role { Content, CloseButton, FloatButton, TitleBar, RoleCount };
   QWidget *widgetForRole(Role r) const;
   void setWidgetForRole(Role r, QWidget *w);
   QLayoutItem *itemForRole(Role r) const;

   QRect titleArea() const {
      return _titleArea;
   }

   int minimumTitleWidth() const;
   int titleHeight() const;
   void updateMaxSize();
   bool nativeWindowDeco() const;
   bool nativeWindowDeco(bool floating) const;

   void setVerticalTitleBar(bool b);

   bool verticalTitleBar;

 private:
   QVector<QLayoutItem *> item_list;
   QRect _titleArea;
};

/* The size hints of a QDockWidget will depend on whether it is docked or not.
   This layout item always returns the size hints as if the dock widget was docked. */

class QDockWidgetItem : public QWidgetItem
{
 public:
   QDockWidgetItem(QDockWidget *dockWidget);

   QSize minimumSize() const override;
   QSize maximumSize() const override;
   QSize sizeHint() const override;

 private:
   inline QLayoutItem *dockWidgetChildItem() const;
   inline QDockWidgetLayout *dockWidgetLayout() const;
};

inline QLayoutItem *QDockWidgetItem::dockWidgetChildItem() const
{
   if (QDockWidgetLayout *layout = dockWidgetLayout()) {
      return layout->itemForRole(QDockWidgetLayout::Content);
   }
   return 0;
}

inline QDockWidgetLayout *QDockWidgetItem::dockWidgetLayout() const
{
   QWidget *w = const_cast<QDockWidgetItem *>(this)->widget();
   if (w != 0) {
      return qobject_cast<QDockWidgetLayout *>(w->layout());
   }
   return 0;
}

QT_END_NAMESPACE

#endif // QT_NO_DOCKWIDGET

#endif // QDYNAMICDOCKWIDGET_P_H
