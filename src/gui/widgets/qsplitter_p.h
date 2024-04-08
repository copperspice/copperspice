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

#ifndef QSPLITTER_P_H
#define QSPLITTER_P_H

#include <qframe_p.h>
#include <qrubberband.h>

static constexpr const uint Default = 2;

class QSplitterLayoutStruct
{
 public:
   QSplitterLayoutStruct()
      : sizer(-1), collapsed(false), collapsible(Default), widget(nullptr), handle(nullptr)
   {
   }

   ~QSplitterLayoutStruct()
   {
      delete handle;
   }

   int getWidgetSize(Qt::Orientation orient);
   int getHandleSize(Qt::Orientation orient);

   int pick(const QSize &size, Qt::Orientation orient) {
      return (orient == Qt::Horizontal) ? size.width() : size.height();
   }

   QRect rect;
   int sizer;
   uint collapsed : 1;
   uint collapsible : 2;
   QWidget *widget;
   QSplitterHandle *handle;
};

class QSplitterPrivate : public QFramePrivate
{
   Q_DECLARE_PUBLIC(QSplitter)

 public:
   QSplitterPrivate()
      : rubberBand(nullptr), opaque(true), firstShow(true), childrenCollapsible(true),
        compatMode(false), handleWidth(-1), blockChildAdd(false), opaqueResizeSet(false)
   {
   }

   ~QSplitterPrivate();

   QPointer<QRubberBand> rubberBand;
   mutable QList<QSplitterLayoutStruct *> list;
   Qt::Orientation orient;
   bool opaque : 8;
   bool firstShow : 8;
   bool childrenCollapsible : 8;
   bool compatMode : 8;
   int handleWidth;
   bool blockChildAdd;
   bool opaqueResizeSet;

   int pick(const QPoint &pos) const {
      return orient == Qt::Horizontal ? pos.x() : pos.y();
   }

   int pick(const QSize &s) const {
      return orient == Qt::Horizontal ? s.width() : s.height();
   }

   int trans(const QPoint &pos) const {
      return orient == Qt::Vertical ? pos.x() : pos.y();
   }

   int trans(const QSize &s) const {
      return orient == Qt::Vertical ? s.width() : s.height();
   }

   void init();
   void recalc(bool update = false);
   void doResize();
   void storeSizes();
   void getRange(int index, int *, int *, int *, int *) const;
   void addContribution(int, int *, int *, bool) const;
   int adjustPos(int, int, int *, int *, int *, int *) const;
   bool collapsible(QSplitterLayoutStruct *) const;

   bool collapsible(int index) const {
      return (index < 0 || index >= list.size()) ? true : collapsible(list.at(index));
   }

   QSplitterLayoutStruct *findWidget(QWidget *) const;
   void insertWidget_helper(int index, QWidget *widget, bool show);
   QSplitterLayoutStruct *insertWidget(int index, QWidget *);

   void doMove(bool backwards, int pos, int index, int delta, bool mayCollapse, int *positions, int *widths);
   void setGeo(QSplitterLayoutStruct *s, int pos, int size, bool allowCollapse);
   int findWidgetJustBeforeOrJustAfter(int index, int delta, int &collapsibleSize) const;
   void updateHandles();
   void setSizes_helper(const QList<int> &sizes, bool clampNegativeSize = false);
};

class QSplitterHandlePrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QSplitterHandle)

 public:
   QSplitterHandlePrivate()
      : s(nullptr), orient(Qt::Horizontal), mouseOffset(0), opaq(false), hover(false), pressed(false)
   {
   }

   int pick(const QPoint &pos) const {
      return orient == Qt::Horizontal ? pos.x() : pos.y();
   }

   QSplitter *s;
   Qt::Orientation orient;
   int mouseOffset;
   bool opaq    : 1;
   bool hover   : 1;
   bool pressed : 1;
};

#endif
