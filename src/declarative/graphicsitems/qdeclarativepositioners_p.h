/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDECLARATIVEPOSITIONERS_P_H
#define QDECLARATIVEPOSITIONERS_P_H

#include <qdeclarativeimplicitsizeitem_p.h>
#include <qdeclarativestate_p.h>
#include <qpodvector_p.h>
#include <QtCore/QObject>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QDeclarativeBasePositionerPrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeBasePositioner : public QDeclarativeImplicitSizeItem
{
   CS_OBJECT(QDeclarativeBasePositioner)

   CS_PROPERTY_READ(spacing, spacing)
   CS_PROPERTY_WRITE(spacing, setSpacing)
   CS_PROPERTY_NOTIFY(spacing, spacingChanged)
   CS_PROPERTY_READ(*move, move)
   CS_PROPERTY_WRITE(*move, setMove)
   CS_PROPERTY_NOTIFY(*move, moveChanged)
   CS_PROPERTY_READ(*add, add)
   CS_PROPERTY_WRITE(*add, setAdd)
   CS_PROPERTY_NOTIFY(*add, addChanged)

 public:
   enum PositionerType { None = 0x0, Horizontal = 0x1, Vertical = 0x2, Both = 0x3 };
   QDeclarativeBasePositioner(PositionerType, QDeclarativeItem *parent);
   ~QDeclarativeBasePositioner();

   int spacing() const;
   void setSpacing(int);

   QDeclarativeTransition *move() const;
   void setMove(QDeclarativeTransition *);

   QDeclarativeTransition *add() const;
   void setAdd(QDeclarativeTransition *);

   CS_SIGNAL_1(Public, void spacingChanged())
   CS_SIGNAL_2(spacingChanged)
   CS_SIGNAL_1(Public, void moveChanged())
   CS_SIGNAL_2(moveChanged)
   CS_SIGNAL_1(Public, void addChanged())
   CS_SIGNAL_2(addChanged)

 protected:
   QDeclarativeBasePositioner(QDeclarativeBasePositionerPrivate &dd, PositionerType at, QDeclarativeItem *parent);
   virtual void componentComplete();
   virtual QVariant itemChange(GraphicsItemChange, const QVariant &);
   void finishApplyTransitions();

   CS_SLOT_1(Protected, void prePositioning())
   CS_SLOT_2(prePositioning)
   CS_SLOT_1(Protected, void graphicsWidgetGeometryChanged())
   CS_SLOT_2(graphicsWidgetGeometryChanged)

   virtual void doPositioning(QSizeF *contentSize) = 0;
   virtual void reportConflictingAnchors() = 0;

   class PositionedItem
   {
    public :
      PositionedItem(QGraphicsObject *i) : item(i), isNew(false), isVisible(true) {}
      bool operator==(const PositionedItem &other) const {
         return other.item == item;
      }
      QGraphicsObject *item;
      bool isNew;
      bool isVisible;
   };

   QPODVector<PositionedItem, 8> positionedItems;
   void positionX(int, const PositionedItem &target);
   void positionY(int, const PositionedItem &target);

 private:
   Q_DISABLE_COPY(QDeclarativeBasePositioner)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeBasePositioner)
};

class QDeclarativeColumn : public QDeclarativeBasePositioner
{
   CS_OBJECT(QDeclarativeColumn)

 public:
   QDeclarativeColumn(QDeclarativeItem *parent = 0);

 protected:
   virtual void doPositioning(QSizeF *contentSize);
   virtual void reportConflictingAnchors();

 private:
   Q_DISABLE_COPY(QDeclarativeColumn)
};

class QDeclarativeRow: public QDeclarativeBasePositioner
{
   CS_OBJECT(QDeclarativeRow)

   CS_PROPERTY_READ(layoutDirection, layoutDirection)
   CS_PROPERTY_WRITE(layoutDirection, setLayoutDirection)
   CS_PROPERTY_NOTIFY(layoutDirection, layoutDirectionChanged)
   CS_PROPERTY_REVISION(layoutDirection, 1)

 public:
   QDeclarativeRow(QDeclarativeItem *parent = 0);

   Qt::LayoutDirection layoutDirection() const;
   void setLayoutDirection (Qt::LayoutDirection);
   Qt::LayoutDirection effectiveLayoutDirection() const;

   CS_SIGNAL_1(Public, void layoutDirectionChanged())
   CS_SIGNAL_2(layoutDirectionChanged)
   CS_REVISION(layoutDirectionChanged, 1)

 protected:
   virtual void doPositioning(QSizeF *contentSize);
   virtual void reportConflictingAnchors();

 private:
   Q_DISABLE_COPY(QDeclarativeRow)
};

class QDeclarativeGrid : public QDeclarativeBasePositioner
{
   CS_OBJECT(QDeclarativeGrid)
   CS_PROPERTY_READ(rows, rows)
   CS_PROPERTY_WRITE(rows, setRows)
   CS_PROPERTY_NOTIFY(rows, rowsChanged)
   CS_PROPERTY_READ(columns, columns)
   CS_PROPERTY_WRITE(columns, setColumns)
   CS_PROPERTY_NOTIFY(columns, columnsChanged)
   CS_PROPERTY_READ(flow, flow)
   CS_PROPERTY_WRITE(flow, setFlow)
   CS_PROPERTY_NOTIFY(flow, flowChanged)
   CS_PROPERTY_READ(layoutDirection, layoutDirection)
   CS_PROPERTY_WRITE(layoutDirection, setLayoutDirection)
   CS_PROPERTY_NOTIFY(layoutDirection, layoutDirectionChanged)
   CS_PROPERTY_REVISION(layoutDirection, 1)

 public:
   QDeclarativeGrid(QDeclarativeItem *parent = 0);

   int rows() const {
      return m_rows;
   }
   void setRows(const int rows);

   int columns() const {
      return m_columns;
   }
   void setColumns(const int columns);

   CS_ENUM(Flow)
   enum Flow { LeftToRight, TopToBottom };
   Flow flow() const;
   void setFlow(Flow);

   Qt::LayoutDirection layoutDirection() const;
   void setLayoutDirection (Qt::LayoutDirection);
   Qt::LayoutDirection effectiveLayoutDirection() const;

   CS_SIGNAL_1(Public, void rowsChanged())
   CS_SIGNAL_2(rowsChanged)
   CS_SIGNAL_1(Public, void columnsChanged())
   CS_SIGNAL_2(columnsChanged)
   CS_SIGNAL_1(Public, void flowChanged())
   CS_SIGNAL_2(flowChanged)

   CS_SIGNAL_1(Public, void layoutDirectionChanged())
   CS_SIGNAL_2(layoutDirectionChanged)
   CS_REVISION(layoutDirectionChanged, 1)

 protected:
   virtual void doPositioning(QSizeF *contentSize);
   virtual void reportConflictingAnchors();

 private:
   int m_rows;
   int m_columns;
   Flow m_flow;
   Q_DISABLE_COPY(QDeclarativeGrid)
};

class QDeclarativeFlowPrivate;
class QDeclarativeFlow: public QDeclarativeBasePositioner
{
   CS_OBJECT(QDeclarativeFlow)

   CS_PROPERTY_READ(flow, flow)
   CS_PROPERTY_WRITE(flow, setFlow)
   CS_PROPERTY_NOTIFY(flow, flowChanged)
   CS_PROPERTY_READ(layoutDirection, layoutDirection)
   CS_PROPERTY_WRITE(layoutDirection, setLayoutDirection)
   CS_PROPERTY_NOTIFY(layoutDirection, layoutDirectionChanged)
   CS_PROPERTY_REVISION(layoutDirection, 1)

 public:
   QDeclarativeFlow(QDeclarativeItem *parent = 0);

   CS_ENUM(Flow)
   enum Flow { LeftToRight, TopToBottom };
   Flow flow() const;
   void setFlow(Flow);

   Qt::LayoutDirection layoutDirection() const;
   void setLayoutDirection (Qt::LayoutDirection);
   Qt::LayoutDirection effectiveLayoutDirection() const;

   CS_SIGNAL_1(Public, void flowChanged())
   CS_SIGNAL_2(flowChanged)

   CS_SIGNAL_1(Public, void layoutDirectionChanged())
   CS_SIGNAL_2(layoutDirectionChanged)
   CS_REVISION(layoutDirectionChanged, 1)

 protected:
   virtual void doPositioning(QSizeF *contentSize);
   virtual void reportConflictingAnchors();

   QDeclarativeFlow(QDeclarativeFlowPrivate &dd, QDeclarativeItem *parent);

 private:
   Q_DISABLE_COPY(QDeclarativeFlow)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeFlow)
};


QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeColumn)
QML_DECLARE_TYPE(QDeclarativeRow)
QML_DECLARE_TYPE(QDeclarativeGrid)
QML_DECLARE_TYPE(QDeclarativeFlow)

#endif
