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

#ifndef QDECLARATIVEPATH_P_H
#define QDECLARATIVEPATH_P_H

#include <qdeclarativeitem.h>
#include <qdeclarative.h>
#include <QtCore/QObject>
#include <QtGui/QPainterPath>

QT_BEGIN_NAMESPACE

class QDeclarativePathElement : public QObject
{
   DECL_CS_OBJECT(QDeclarativePathElement)

 public:
   QDeclarativePathElement(QObject *parent = 0) : QObject(parent) {}

   CS_SIGNAL_1(Public, void changed())
   CS_SIGNAL_2(changed)
};

class QDeclarativePathAttribute : public QDeclarativePathElement
{
   DECL_CS_OBJECT(QDeclarativePathAttribute)

   CS_PROPERTY_READ(name, name)
   CS_PROPERTY_WRITE(name, setName)
   CS_PROPERTY_NOTIFY(name, nameChanged)
   CS_PROPERTY_READ(value, value)
   CS_PROPERTY_WRITE(value, setValue)
   CS_PROPERTY_NOTIFY(value, valueChanged)
 public:
   QDeclarativePathAttribute(QObject *parent = 0) : QDeclarativePathElement(parent), _value(0) {}


   QString name() const;
   void setName(const QString &name);

   qreal value() const;
   void setValue(qreal value);

   CS_SIGNAL_1(Public, void nameChanged())
   CS_SIGNAL_2(nameChanged)
   CS_SIGNAL_1(Public, void valueChanged())
   CS_SIGNAL_2(valueChanged)

 private:
   QString _name;
   qreal _value;
};

class QDeclarativeCurve : public QDeclarativePathElement
{
   DECL_CS_OBJECT(QDeclarativeCurve)

   CS_PROPERTY_READ(x, x)
   CS_PROPERTY_WRITE(x, setX)
   CS_PROPERTY_NOTIFY(x, xChanged)
   CS_PROPERTY_READ(y, y)
   CS_PROPERTY_WRITE(y, setY)
   CS_PROPERTY_NOTIFY(y, yChanged)
 public:
   QDeclarativeCurve(QObject *parent = 0) : QDeclarativePathElement(parent), _x(0), _y(0) {}

   qreal x() const;
   void setX(qreal x);

   qreal y() const;
   void setY(qreal y);

   virtual void addToPath(QPainterPath &) {}

   CS_SIGNAL_1(Public, void xChanged())
   CS_SIGNAL_2(xChanged)
   CS_SIGNAL_1(Public, void yChanged())
   CS_SIGNAL_2(yChanged)

 private:
   qreal _x;
   qreal _y;
};

class QDeclarativePathLine : public QDeclarativeCurve
{
   DECL_CS_OBJECT(QDeclarativePathLine)

 public:
   QDeclarativePathLine(QObject *parent = 0) : QDeclarativeCurve(parent) {}

   void addToPath(QPainterPath &path);
};

class QDeclarativePathQuad : public QDeclarativeCurve
{
   DECL_CS_OBJECT(QDeclarativePathQuad)

   CS_PROPERTY_READ(controlX, controlX)
   CS_PROPERTY_WRITE(controlX, setControlX)
   CS_PROPERTY_NOTIFY(controlX, controlXChanged)
   CS_PROPERTY_READ(controlY, controlY)
   CS_PROPERTY_WRITE(controlY, setControlY)
   CS_PROPERTY_NOTIFY(controlY, controlYChanged)
 public:
   QDeclarativePathQuad(QObject *parent = 0) : QDeclarativeCurve(parent), _controlX(0), _controlY(0) {}

   qreal controlX() const;
   void setControlX(qreal x);

   qreal controlY() const;
   void setControlY(qreal y);

   void addToPath(QPainterPath &path);

   CS_SIGNAL_1(Public, void controlXChanged())
   CS_SIGNAL_2(controlXChanged)
   CS_SIGNAL_1(Public, void controlYChanged())
   CS_SIGNAL_2(controlYChanged)

 private:
   qreal _controlX;
   qreal _controlY;
};

class QDeclarativePathCubic : public QDeclarativeCurve
{
   DECL_CS_OBJECT(QDeclarativePathCubic)

   CS_PROPERTY_READ(control1X, control1X)
   CS_PROPERTY_WRITE(control1X, setControl1X)
   CS_PROPERTY_NOTIFY(control1X, control1XChanged)
   CS_PROPERTY_READ(control1Y, control1Y)
   CS_PROPERTY_WRITE(control1Y, setControl1Y)
   CS_PROPERTY_NOTIFY(control1Y, control1YChanged)
   CS_PROPERTY_READ(control2X, control2X)
   CS_PROPERTY_WRITE(control2X, setControl2X)
   CS_PROPERTY_NOTIFY(control2X, control2XChanged)
   CS_PROPERTY_READ(control2Y, control2Y)
   CS_PROPERTY_WRITE(control2Y, setControl2Y)
   CS_PROPERTY_NOTIFY(control2Y, control2YChanged)

   QDeclarativePathCubic(QObject *parent = 0) : QDeclarativeCurve(parent), _control1X(0), _control1Y(0), _control2X(0),
      _control2Y(0) {}

   qreal control1X() const;
   void setControl1X(qreal x);

   qreal control1Y() const;
   void setControl1Y(qreal y);

   qreal control2X() const;
   void setControl2X(qreal x);

   qreal control2Y() const;
   void setControl2Y(qreal y);

   void addToPath(QPainterPath &path);

 public:
   CS_SIGNAL_1(Public, void control1XChanged())
   CS_SIGNAL_2(control1XChanged)
   CS_SIGNAL_1(Public, void control1YChanged())
   CS_SIGNAL_2(control1YChanged)
   CS_SIGNAL_1(Public, void control2XChanged())
   CS_SIGNAL_2(control2XChanged)
   CS_SIGNAL_1(Public, void control2YChanged())
   CS_SIGNAL_2(control2YChanged)

 private:
   qreal _control1X;
   qreal _control1Y;
   qreal _control2X;
   qreal _control2Y;
};

class QDeclarativePathPercent : public QDeclarativePathElement
{
   DECL_CS_OBJECT(QDeclarativePathPercent)
   CS_PROPERTY_READ(value, value)
   CS_PROPERTY_WRITE(value, setValue)
   CS_PROPERTY_NOTIFY(value, valueChanged)

 public:
   QDeclarativePathPercent(QObject *parent = 0) : QDeclarativePathElement(parent) {}

   qreal value() const;
   void setValue(qreal value);

   CS_SIGNAL_1(Public, void valueChanged())
   CS_SIGNAL_2(valueChanged)

 private:
   qreal _value;
};

class QDeclarativePathPrivate;
class QDeclarativePath : public QObject, public QDeclarativeParserStatus
{
   DECL_CS_OBJECT(QDeclarativePath)

   CS_INTERFACES(QDeclarativeParserStatus, QDeclarativeParserStatus)

   CS_PROPERTY_READ(pathElements, pathElements)
   CS_PROPERTY_READ(startX, startX)
   CS_PROPERTY_WRITE(startX, setStartX)
   CS_PROPERTY_NOTIFY(startX, startXChanged)
   CS_PROPERTY_READ(startY, startY)
   CS_PROPERTY_WRITE(startY, setStartY)
   CS_PROPERTY_NOTIFY(startY, startYChanged)
   CS_PROPERTY_READ(closed, isClosed)
   CS_PROPERTY_NOTIFY(closed, changed)
   DECL_CS_CLASSINFO("DefaultProperty", "pathElements")

 public:
   QDeclarativePath(QObject *parent = 0);
   ~QDeclarativePath();

   QDeclarativeListProperty<QDeclarativePathElement> pathElements();

   qreal startX() const;
   void setStartX(qreal x);

   qreal startY() const;
   void setStartY(qreal y);

   bool isClosed() const;

   QPainterPath path() const;
   QStringList attributes() const;
   qreal attributeAt(const QString &, qreal) const;
   QPointF pointAt(qreal) const;

   CS_SIGNAL_1(Public, void changed())
   CS_SIGNAL_2(changed)
   CS_SIGNAL_1(Public, void startXChanged())
   CS_SIGNAL_2(startXChanged)
   CS_SIGNAL_1(Public, void startYChanged())
   CS_SIGNAL_2(startYChanged)

 protected:
   virtual void componentComplete();
   virtual void classBegin();

 private :
   CS_SLOT_1(Private, void processPath())
   CS_SLOT_2(processPath)

   struct AttributePoint {
      AttributePoint() : percent(0), scale(1), origpercent(0) {}
      AttributePoint(const AttributePoint &other)
         : percent(other.percent), scale(other.scale), origpercent(other.origpercent), values(other.values) {}
      AttributePoint &operator=(const AttributePoint &other) {
         percent = other.percent;
         scale = other.scale;
         origpercent = other.origpercent;
         values = other.values;
         return *this;
      }
      qreal percent;      //massaged percent along the painter path
      qreal scale;
      qreal origpercent;  //'real' percent along the painter path
      QHash<QString, qreal> values;
   };

   void interpolate(int idx, const QString &name, qreal value);
   void endpoint(const QString &name);
   void createPointCache() const;

   Q_DISABLE_COPY(QDeclarativePath)
   Q_DECLARE_PRIVATE(QDeclarativePath)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativePathElement)
QML_DECLARE_TYPE(QDeclarativePathAttribute)
QML_DECLARE_TYPE(QDeclarativeCurve)
QML_DECLARE_TYPE(QDeclarativePathLine)
QML_DECLARE_TYPE(QDeclarativePathQuad)
QML_DECLARE_TYPE(QDeclarativePathCubic)
QML_DECLARE_TYPE(QDeclarativePathPercent)
QML_DECLARE_TYPE(QDeclarativePath)

#endif // QDECLARATIVEPATH_H
