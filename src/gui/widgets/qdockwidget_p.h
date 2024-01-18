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

#ifndef QDockWidget_P_H
#define QDockWidget_P_H

#include <qstyleoption.h>
#include <qwidget_p.h>
#include <qboxlayout.h>
#include <qdockwidget.h>

#ifndef QT_NO_DOCKWIDGET

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
      : QWidgetPrivate(), state(nullptr),
        features(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable),
        allowedAreas(Qt::AllDockWidgetAreas), resizer(nullptr)
   { }

   void init();
   void _q_toggleView(bool);
   void _q_toggleTopLevel();

   void updateButtons();
   DragState *state;

   QDockWidget::DockWidgetFeatures features;
   Qt::DockWidgetAreas allowedAreas;

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
   void startDrag(bool group = true);
   void endDrag(bool abort = false);
   void moveEvent(QMoveEvent *event);

   void unplug(const QRect &rect);
   void plug(const QRect &rect);
   void setResizerActive(bool active);

   bool isAnimating() const;
 private:
   QWidgetResizeHandler *resizer;
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

   static bool wmSupportsNativeWindowDeco();

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
   return nullptr;
}

inline QDockWidgetLayout *QDockWidgetItem::dockWidgetLayout() const
{
   QWidget *w = const_cast<QDockWidgetItem *>(this)->widget();
   if (w != nullptr) {
      return qobject_cast<QDockWidgetLayout *>(w->layout());
   }

   return nullptr;
}

#endif // QT_NO_DOCKWIDGET

#endif // QDYNAMICDOCKWIDGET_P_H
