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

#ifndef QSPLITTER_H
#define QSPLITTER_H

#include <QtGui/qframe.h>
#include <QtGui/qsizepolicy.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SPLITTER

class QTextStream;
class QSplitterPrivate;
class QSplitterHandlePrivate;

template <typename T> class QList;
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
   explicit QSplitter(Qt::Orientation, QWidget *parent = nullptr);
   ~QSplitter();

   void addWidget(QWidget *widget);
   void insertWidget(int index, QWidget *widget);

   void setOrientation(Qt::Orientation);
   Qt::Orientation orientation() const;

   void setChildrenCollapsible(bool);
   bool childrenCollapsible() const;

   void setCollapsible(int index, bool);
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
   void setHandleWidth(int);

   int indexOf(QWidget *w) const;
   QWidget *widget(int index) const;
   int count() const;

   void getRange(int index, int *, int *) const;
   QSplitterHandle *handle(int index) const;

   void setStretchFactor(int index, int stretch);

   GUI_CS_SIGNAL_1(Public, void splitterMoved(int pos, int index))
   GUI_CS_SIGNAL_2(splitterMoved, pos, index)

 protected:
   virtual QSplitterHandle *createHandle();

   void childEvent(QChildEvent *) override;

   bool event(QEvent *) override;
   void resizeEvent(QResizeEvent *) override;

   void changeEvent(QEvent *) override;
   void moveSplitter(int pos, int index);
   void setRubberBand(int position);
   int closestLegalPosition(int, int);

 private:
   Q_DISABLE_COPY(QSplitter)
   Q_DECLARE_PRIVATE(QSplitter)

   friend class QSplitterHandle;
};

Q_GUI_EXPORT QTextStream &operator<<(QTextStream &, const QSplitter &);
Q_GUI_EXPORT QTextStream &operator>>(QTextStream &, QSplitter &);

class Q_GUI_EXPORT QSplitterHandle : public QWidget
{
   GUI_CS_OBJECT(QSplitterHandle)

 public:
   QSplitterHandle(Qt::Orientation o, QSplitter *parent);
   void setOrientation(Qt::Orientation o);
   Qt::Orientation orientation() const;
   bool opaqueResize() const;
   QSplitter *splitter() const;

   QSize sizeHint() const override;

 protected:
   void paintEvent(QPaintEvent *) override;
   void mouseMoveEvent(QMouseEvent *) override;
   void mousePressEvent(QMouseEvent *) override;
   void mouseReleaseEvent(QMouseEvent *) override;
   void resizeEvent(QResizeEvent *) override;
   bool event(QEvent *) override;

   void moveSplitter(int p);
   int closestLegalPosition(int p);

 private:
   Q_DISABLE_COPY(QSplitterHandle)
   Q_DECLARE_PRIVATE(QSplitterHandle)
};

#endif // QT_NO_SPLITTER

QT_END_NAMESPACE

#endif // QSPLITTER_H
