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

#ifndef QDECLARATIVEPINCHAREA_H
#define QDECLARATIVEPINCHAREA_H

#include <qdeclarativeitem.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE


class QDeclarativePinch : public QObject
{
    CS_OBJECT(QDeclarativePinch)

    CS_ENUM(Axis)
    CS_PROPERTY_READ(*target, target)
    CS_PROPERTY_WRITE(*target, setTarget)
    CS_PROPERTY_RESET(*target, resetTarget)
    CS_PROPERTY_READ(minimumScale, minimumScale)
    CS_PROPERTY_WRITE(minimumScale, setMinimumScale)
    CS_PROPERTY_NOTIFY(minimumScale, minimumScaleChanged)
    CS_PROPERTY_READ(maximumScale, maximumScale)
    CS_PROPERTY_WRITE(maximumScale, setMaximumScale)
    CS_PROPERTY_NOTIFY(maximumScale, maximumScaleChanged)
    CS_PROPERTY_READ(minimumRotation, minimumRotation)
    CS_PROPERTY_WRITE(minimumRotation, setMinimumRotation)
    CS_PROPERTY_NOTIFY(minimumRotation, minimumRotationChanged)
    CS_PROPERTY_READ(maximumRotation, maximumRotation)
    CS_PROPERTY_WRITE(maximumRotation, setMaximumRotation)
    CS_PROPERTY_NOTIFY(maximumRotation, maximumRotationChanged)
    CS_PROPERTY_READ(dragAxis, axis)
    CS_PROPERTY_WRITE(dragAxis, setAxis)
    CS_PROPERTY_NOTIFY(dragAxis, dragAxisChanged)
    CS_PROPERTY_READ(minimumX, xmin)
    CS_PROPERTY_WRITE(minimumX, setXmin)
    CS_PROPERTY_NOTIFY(minimumX, minimumXChanged)
    CS_PROPERTY_READ(maximumX, xmax)
    CS_PROPERTY_WRITE(maximumX, setXmax)
    CS_PROPERTY_NOTIFY(maximumX, maximumXChanged)
    CS_PROPERTY_READ(minimumY, ymin)
    CS_PROPERTY_WRITE(minimumY, setYmin)
    CS_PROPERTY_NOTIFY(minimumY, minimumYChanged)
    CS_PROPERTY_READ(maximumY, ymax)
    CS_PROPERTY_WRITE(maximumY, setYmax)
    CS_PROPERTY_NOTIFY(maximumY, maximumYChanged)
    CS_PROPERTY_READ(active, active)
    CS_PROPERTY_NOTIFY(active, activeChanged)

public:
    QDeclarativePinch();

    QGraphicsObject *target() const { return m_target; }
    void setTarget(QGraphicsObject *target) {
        if (target == m_target)
            return;
        m_target = target;
        emit targetChanged();
    }
    void resetTarget() {
        if (!m_target)
            return;
        m_target = 0;
        emit targetChanged();
    }

    qreal minimumScale() const { return m_minScale; }
    void setMinimumScale(qreal s) {
        if (s == m_minScale)
            return;
        m_minScale = s;
        emit minimumScaleChanged();
    }
    qreal maximumScale() const { return m_maxScale; }
    void setMaximumScale(qreal s) {
        if (s == m_maxScale)
            return;
        m_maxScale = s;
        emit maximumScaleChanged();
    }

    qreal minimumRotation() const { return m_minRotation; }
    void setMinimumRotation(qreal r) {
        if (r == m_minRotation)
            return;
        m_minRotation = r;
        emit minimumRotationChanged();
    }
    qreal maximumRotation() const { return m_maxRotation; }
    void setMaximumRotation(qreal r) {
        if (r == m_maxRotation)
            return;
        m_maxRotation = r;
        emit maximumRotationChanged();
    }

    enum Axis { NoDrag=0x00, XAxis=0x01, YAxis=0x02, XandYAxis=0x03 };
    Axis axis() const { return m_axis; }
    void setAxis(Axis a) {
        if (a == m_axis)
            return;
        m_axis = a;
        emit dragAxisChanged();
    }

    qreal xmin() const { return m_xmin; }
    void setXmin(qreal x) {
        if (x == m_xmin)
            return;
        m_xmin = x;
        emit minimumXChanged();
    }
    qreal xmax() const { return m_xmax; }
    void setXmax(qreal x) {
        if (x == m_xmax)
            return;
        m_xmax = x;
        emit maximumXChanged();
    }
    qreal ymin() const { return m_ymin; }
    void setYmin(qreal y) {
        if (y == m_ymin)
            return;
        m_ymin = y;
        emit minimumYChanged();
    }
    qreal ymax() const { return m_ymax; }
    void setYmax(qreal y) {
        if (y == m_ymax)
            return;
        m_ymax = y;
        emit maximumYChanged();
    }

    bool active() const { return m_active; }
    void setActive(bool a) {
        if (a == m_active)
            return;
        m_active = a;
        emit activeChanged();
    }

public:
    CS_SIGNAL_1(Public, void targetChanged())
    CS_SIGNAL_2(targetChanged) 
    CS_SIGNAL_1(Public, void minimumScaleChanged())
    CS_SIGNAL_2(minimumScaleChanged) 
    CS_SIGNAL_1(Public, void maximumScaleChanged())
    CS_SIGNAL_2(maximumScaleChanged) 
    CS_SIGNAL_1(Public, void minimumRotationChanged())
    CS_SIGNAL_2(minimumRotationChanged) 
    CS_SIGNAL_1(Public, void maximumRotationChanged())
    CS_SIGNAL_2(maximumRotationChanged) 
    CS_SIGNAL_1(Public, void dragAxisChanged())
    CS_SIGNAL_2(dragAxisChanged) 
    CS_SIGNAL_1(Public, void minimumXChanged())
    CS_SIGNAL_2(minimumXChanged) 
    CS_SIGNAL_1(Public, void maximumXChanged())
    CS_SIGNAL_2(maximumXChanged) 
    CS_SIGNAL_1(Public, void minimumYChanged())
    CS_SIGNAL_2(minimumYChanged) 
    CS_SIGNAL_1(Public, void maximumYChanged())
    CS_SIGNAL_2(maximumYChanged) 
    CS_SIGNAL_1(Public, void activeChanged())
    CS_SIGNAL_2(activeChanged) 

private:
    QGraphicsObject *m_target;
    qreal m_minScale;
    qreal m_maxScale;
    qreal m_minRotation;
    qreal m_maxRotation;
    Axis m_axis;
    qreal m_xmin;
    qreal m_xmax;
    qreal m_ymin;
    qreal m_ymax;
    bool m_active;
};

class QDeclarativePinchEvent : public QObject
{
    CS_OBJECT(QDeclarativePinchEvent)

    CS_PROPERTY_READ(center, center)
    CS_PROPERTY_READ(startCenter, startCenter)
    CS_PROPERTY_READ(previousCenter, previousCenter)
    CS_PROPERTY_READ(scale, scale)
    CS_PROPERTY_READ(previousScale, previousScale)
    CS_PROPERTY_READ(angle, angle)
    CS_PROPERTY_READ(previousAngle, previousAngle)
    CS_PROPERTY_READ(rotation, rotation)
    CS_PROPERTY_READ(point1, point1)
    CS_PROPERTY_READ(startPoint1, startPoint1)
    CS_PROPERTY_READ(point2, point2)
    CS_PROPERTY_READ(startPoint2, startPoint2)
    CS_PROPERTY_READ(pointCount, pointCount)
    CS_PROPERTY_READ(accepted, accepted)
    CS_PROPERTY_WRITE(accepted, setAccepted)

public:
    QDeclarativePinchEvent(QPointF c, qreal s, qreal a, qreal r)
        : QObject(), m_center(c), m_scale(s), m_angle(a), m_rotation(r)
        , m_pointCount(0), m_accepted(true) {}

    QPointF center() const { return m_center; }
    QPointF startCenter() const { return m_startCenter; }
    void setStartCenter(QPointF c) { m_startCenter = c; }
    QPointF previousCenter() const { return m_lastCenter; }
    void setPreviousCenter(QPointF c) { m_lastCenter = c; }
    qreal scale() const { return m_scale; }
    qreal previousScale() const { return m_lastScale; }
    void setPreviousScale(qreal s) { m_lastScale = s; }
    qreal angle() const { return m_angle; }
    qreal previousAngle() const { return m_lastAngle; }
    void setPreviousAngle(qreal a) { m_lastAngle = a; }
    qreal rotation() const { return m_rotation; }
    QPointF point1() const { return m_point1; }
    void setPoint1(QPointF p) { m_point1 = p; }
    QPointF startPoint1() const { return m_startPoint1; }
    void setStartPoint1(QPointF p) { m_startPoint1 = p; }
    QPointF point2() const { return m_point2; }
    void setPoint2(QPointF p) { m_point2 = p; }
    QPointF startPoint2() const { return m_startPoint2; }
    void setStartPoint2(QPointF p) { m_startPoint2 = p; }
    int pointCount() const { return m_pointCount; }
    void setPointCount(int count) { m_pointCount = count; }

    bool accepted() const { return m_accepted; }
    void setAccepted(bool a) { m_accepted = a; }

private:
    QPointF m_center;
    QPointF m_startCenter;
    QPointF m_lastCenter;
    qreal m_scale;
    qreal m_lastScale;
    qreal m_angle;
    qreal m_lastAngle;
    qreal m_rotation;
    QPointF m_point1;
    QPointF m_point2;
    QPointF m_startPoint1;
    QPointF m_startPoint2;
    int m_pointCount;
    bool m_accepted;
};


class QDeclarativeMouseEvent;
class QDeclarativePinchAreaPrivate;
class QDeclarativePinchArea : public QDeclarativeItem
{
    CS_OBJECT(QDeclarativePinchArea)

    CS_PROPERTY_READ(enabled, isEnabled)
    CS_PROPERTY_WRITE(enabled, setEnabled)
    CS_PROPERTY_NOTIFY(enabled, enabledChanged)
    CS_PROPERTY_READ(*pinch, pinch)
    CS_PROPERTY_CONSTANT(*pinch)

public:
    QDeclarativePinchArea(QDeclarativeItem *parent=0);
    ~QDeclarativePinchArea();

    bool isEnabled() const;
    void setEnabled(bool);

    QDeclarativePinch *pinch();

public:
    CS_SIGNAL_1(Public, void enabledChanged())
    CS_SIGNAL_2(enabledChanged) 
    CS_SIGNAL_1(Public, void pinchStarted(QDeclarativePinchEvent * pinch))
    CS_SIGNAL_2(pinchStarted,pinch) 
    CS_SIGNAL_1(Public, void pinchUpdated(QDeclarativePinchEvent * pinch))
    CS_SIGNAL_2(pinchUpdated,pinch) 
    CS_SIGNAL_1(Public, void pinchFinished(QDeclarativePinchEvent * pinch))
    CS_SIGNAL_2(pinchFinished,pinch) 

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    bool sceneEvent(QEvent *);
    bool sendMouseEvent(QGraphicsSceneMouseEvent *event);
    bool sceneEventFilter(QGraphicsItem *i, QEvent *e);
    bool event(QEvent *);

    virtual void geometryChanged(const QRectF &newGeometry,
                                 const QRectF &oldGeometry);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

private:
    void updatePinch();
    void handlePress();
    void handleRelease();

private:
    Q_DISABLE_COPY(QDeclarativePinchArea)
    Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativePinchArea)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativePinch)
QML_DECLARE_TYPE(QDeclarativePinchEvent)
QML_DECLARE_TYPE(QDeclarativePinchArea)

QT_END_HEADER

#endif // QDECLARATIVEPINCHAREA_H
