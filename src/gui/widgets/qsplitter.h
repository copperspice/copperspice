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

#ifndef QSPLITTER_H
#define QSPLITTER_H

#include <qframe.h>
#include <qsizepolicy.h>
#include <qcontainerfwd.h>

#ifndef QT_NO_SPLITTER

class QTextStream;
class QSplitterPrivate;
class QSplitterHandlePrivate;
class QSplitterHandle;

class Q_GUI_EXPORT QSplitter : public QFrame
{
   GUI_CS_OBJECT(QSplitter)

   GUI_CS_PROPERTY_READ(orientation, orientation)
   GUI_CS_PROPERTY_WRITE(orientation, setOrientation)

   GUI_CS_PROPERTY_READ(opaqueResize, opaqueResize)
   GUI_CS_PROPERTY_WRITE(opaqueResize, setOpaqueResize)

   GUI_CS_PROPERTY_READ(handleWidth, handleWidth)
   GUI_CS_PROPERTY_WRITE(handleWidth, setHandleWidth)

   GUI_CS_PROPERTY_READ(childrenCollapsible, childrenCollapsible)
   GUI_CS_PROPERTY_WRITE(childrenCollapsible, setChildrenCollapsible)

 public:
   explicit QSplitter(QWidget *parent = nullptr);
   explicit QSplitter(Qt::Orientation orientation, QWidget *parent = nullptr);

   QSplitter(const QSplitter &) = delete;
   QSplitter &operator=(const QSplitter &) = delete;

   ~QSplitter();

   void addWidget(QWidget *widget);
   void insertWidget(int index, QWidget *widget);

   void setOrientation(Qt::Orientation value);
   Qt::Orientation orientation() const;

   void setChildrenCollapsible(bool value);
   bool childrenCollapsible() const;

   void setCollapsible(int index, bool collapse);
   bool isCollapsible(int index) const;
   void setOpaqueResize(bool opaque = true);
   bool opaqueResize() const;
   void refresh();

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

   QList<int> sizes() const;
   void setSizes(const QList<int> &list);

   QByteArray saveState() const;
   bool restoreState(const QByteArray &state);

   int handleWidth() const;
   void setHandleWidth(int width);

   int indexOf(QWidget *widget) const;
   QWidget *widget(int index) const;
   int count() const;

   void getRange(int index, int *min, int *max) const;
   QSplitterHandle *handle(int index) const;

   void setStretchFactor(int index, int stretch);

   GUI_CS_SIGNAL_1(Public, void splitterMoved(int pos, int index))
   GUI_CS_SIGNAL_2(splitterMoved, pos, index)

 protected:
   virtual QSplitterHandle *createHandle();

   void childEvent(QChildEvent *event) override;

   bool event(QEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;

   void changeEvent(QEvent *event) override;
   void moveSplitter(int pos, int index);
   void setRubberBand(int pos);
   int closestLegalPosition(int pos, int index);

 private:
   Q_DECLARE_PRIVATE(QSplitter)

   friend class QSplitterHandle;
};

Q_GUI_EXPORT QTextStream &operator<<(QTextStream &, const QSplitter &);
Q_GUI_EXPORT QTextStream &operator>>(QTextStream &, QSplitter &);

class Q_GUI_EXPORT QSplitterHandle : public QWidget
{
   GUI_CS_OBJECT(QSplitterHandle)

 public:
   explicit QSplitterHandle(Qt::Orientation orientation, QSplitter *parent);

   QSplitterHandle(const QSplitterHandle &) = delete;
   QSplitterHandle &operator=(const QSplitterHandle &) = delete;

   ~QSplitterHandle();

   void setOrientation(Qt::Orientation orientation);
   Qt::Orientation orientation() const;
   bool opaqueResize() const;
   QSplitter *splitter() const;

   QSize sizeHint() const override;

 protected:
   void paintEvent(QPaintEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   bool event(QEvent *event) override;

   void moveSplitter(int pos);
   int closestLegalPosition(int pos);

 private:
   Q_DECLARE_PRIVATE(QSplitterHandle)
};

#endif // QT_NO_SPLITTER

#endif
