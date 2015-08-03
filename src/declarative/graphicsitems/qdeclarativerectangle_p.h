/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QDeclarativeRectangle_P_H
#define QDeclarativeRectangle_P_H

#include <qdeclarativeitem.h>
#include <QtGui/qbrush.h>
#include <qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativePen : public QObject
{
   DECL_CS_OBJECT(QDeclarativePen)

   CS_PROPERTY_READ(width, width)
   CS_PROPERTY_WRITE(width, setWidth)
   CS_PROPERTY_NOTIFY(width, penChanged)
   CS_PROPERTY_READ(color, color)
   CS_PROPERTY_WRITE(color, setColor)
   CS_PROPERTY_NOTIFY(color, penChanged)

 public:
   QDeclarativePen(QObject *parent = 0)
      : QObject(parent), _width(1), _color("#000000"), _valid(false) {
   }

   int width() const {
      return _width;
   }
   void setWidth(int w);

   QColor color() const {
      return _color;
   }
   void setColor(const QColor &c);

   bool isValid() {
      return _valid;
   }

 public:
   CS_SIGNAL_1(Public, void penChanged())
   CS_SIGNAL_2(penChanged)

 private:
   int _width;
   QColor _color;
   bool _valid;
};

class QDeclarativeGradientStop : public QObject
{
   DECL_CS_OBJECT(QDeclarativeGradientStop)

   CS_PROPERTY_READ(position, position)
   CS_PROPERTY_WRITE(position, setPosition)
   CS_PROPERTY_READ(color, color)
   CS_PROPERTY_WRITE(color, setColor)

 public:
   QDeclarativeGradientStop(QObject *parent = 0) : QObject(parent) {}

   qreal position() const {
      return m_position;
   }
   void setPosition(qreal position) {
      m_position = position;
      updateGradient();
   }

   QColor color() const {
      return m_color;
   }
   void setColor(const QColor &color) {
      m_color = color;
      updateGradient();
   }

 private:
   void updateGradient();

 private:
   qreal m_position;
   QColor m_color;
};

class QDeclarativeGradient : public QObject
{
   DECL_CS_OBJECT(QDeclarativeGradient)

   CS_PROPERTY_READ(stops, stops)
   DECL_CS_CLASSINFO("DefaultProperty", "stops")

 public:
   QDeclarativeGradient(QObject *parent = 0) : QObject(parent), m_gradient(0) {}
   ~QDeclarativeGradient() {
      delete m_gradient;
   }

   QDeclarativeListProperty<QDeclarativeGradientStop> stops() {
      return QDeclarativeListProperty<QDeclarativeGradientStop>(this, m_stops);
   }

   const QGradient *gradient() const;

 public:
   CS_SIGNAL_1(Public, void updated())
   CS_SIGNAL_2(updated)

 private:
   void doUpdate();

 private:
   QList<QDeclarativeGradientStop *> m_stops;
   mutable QGradient *m_gradient;
   friend class QDeclarativeGradientStop;
};

class QDeclarativeRectanglePrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeRectangle : public QDeclarativeItem
{
   DECL_CS_OBJECT(QDeclarativeRectangle)

   CS_PROPERTY_READ(color, color)
   CS_PROPERTY_WRITE(color, setColor)
   CS_PROPERTY_NOTIFY(color, colorChanged)
   CS_PROPERTY_READ(*gradient, gradient)
   CS_PROPERTY_WRITE(*gradient, setGradient)
   CS_PROPERTY_READ(*, border)
   CS_PROPERTY_CONSTANT(*)
   CS_PROPERTY_READ(radius, radius)
   CS_PROPERTY_WRITE(radius, setRadius)
   CS_PROPERTY_NOTIFY(radius, radiusChanged)
 public:
   QDeclarativeRectangle(QDeclarativeItem *parent = 0);

   QColor color() const;
   void setColor(const QColor &);

   QDeclarativePen *border();

   QDeclarativeGradient *gradient() const;
   void setGradient(QDeclarativeGradient *gradient);

   qreal radius() const;
   void setRadius(qreal radius);

   QRectF boundingRect() const;

   void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

 public:
   CS_SIGNAL_1(Public, void colorChanged())
   CS_SIGNAL_2(colorChanged)
   CS_SIGNAL_1(Public, void radiusChanged())
   CS_SIGNAL_2(radiusChanged)

 private :
   CS_SLOT_1(Private, void doUpdate())
   CS_SLOT_2(doUpdate)

   void generateRoundedRect();
   void generateBorderedRect();
   void drawRect(QPainter &painter);

   Q_DISABLE_COPY(QDeclarativeRectangle)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeRectangle)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativePen)
QML_DECLARE_TYPE(QDeclarativeGradientStop)
QML_DECLARE_TYPE(QDeclarativeGradient)
QML_DECLARE_TYPE(QDeclarativeRectangle)


#endif // QDECLARATIVERECT_H
