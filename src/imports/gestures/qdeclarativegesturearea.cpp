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

#include <qdeclarativegesturearea_p.h>
#include <qdeclarativeexpression.h>
#include <qdeclarativecontext.h>
#include <qdeclarativeinfo.h>
#include <qdeclarativeproperty_p.h>
#include <qdeclarativeitem_p.h>
#include <QtCore/qdebug.h>
#include <QtCore/qstringlist.h>
#include <QtGui/qevent.h>
#include <qobject_p.h>

#ifndef QT_NO_GESTURES

QT_BEGIN_NAMESPACE

class QDeclarativeGestureAreaPrivate : public QDeclarativeItemPrivate
{
    Q_DECLARE_PUBLIC(QDeclarativeGestureArea)
public:
    QDeclarativeGestureAreaPrivate() : componentcomplete(false), gesture(0) {}

    typedef QMap<Qt::GestureType,QDeclarativeExpression*> Bindings;
    Bindings bindings;

    bool componentcomplete;

    QByteArray data;

    QGesture *gesture;

    bool gestureEvent(QGestureEvent *event);
};

/*!
    \qmlclass GestureArea QDeclarativeGestureArea
    \ingroup qml-basic-interaction-elements

    \brief The GestureArea item enables simple gesture handling.
    \inherits Item

    A GestureArea is like a MouseArea, but it has signals for gesture events.

    \warning Elements in the Qt.labs module are not guaranteed to remain compatible
    in future versions.

    \warning GestureArea is an experimental element whose development has
    been discontinued.  PinchArea is available in QtQuick 1.1 and handles
    two finger gesture input.

    \note This element is only functional on devices with touch input.

    \qml
    import Qt.labs.gestures 1.0

    GestureArea {
        anchors.fill: parent
     // onPan:        ... gesture.acceleration ...
     // onPinch:      ... gesture.rotationAngle ...
     // onSwipe:      ...
     // onTapAndHold: ...
     // onTap:        ...
     // onGesture:    ...
    }
    \endqml

    Each signal has a \e gesture parameter that has the
    properties of the gesture.

    \table
    \header \o Signal \o Type \o Property \o Description
    \row \o onTap \o point \o position \o the position of the tap
    \row \o onTapAndHold \o point \o position \o the position of the tap
    \row \o onPan \o real \o acceleration \o the acceleration of the pan
    \row \o onPan \o point \o delta \o the offset from the previous input position to the current input
    \row \o onPan \o point \o offset \o the total offset from the first input position to the current input position
    \row \o onPan \o point \o lastOffset \o the previous value of offset
    \row \o onPinch \o point \o centerPoint \o the midpoint between the two input points
    \row \o onPinch \o point \o lastCenterPoint \o the previous value of centerPoint
    \row \o onPinch \o point \o startCenterPoint \o the first value of centerPoint
    \row \o onPinch \o real \o rotationAngle \o the angle covered by the gesture motion
    \row \o onPinch \o real \o lastRotationAngle \o the previous value of rotationAngle
    \row \o onPinch \o real \o totalRotationAngle \o the complete angle covered by the gesture
    \row \o onPinch \o real \o scaleFactor \o the change in distance between the two input points
    \row \o onPinch \o real \o lastScaleFactor \o the previous value of scaleFactor
    \row \o onPinch \o real \o totalScaleFactor \o the complete scale factor of the gesture
    \row \o onSwipe \o real \o swipeAngle \o the angle of the swipe
    \endtable

    Custom gestures, handled by onGesture, will have custom properties.

    GestureArea is an invisible item: it is never painted.

    \sa MouseArea
*/

/*!
    \internal
    \class QDeclarativeGestureArea
    \brief The QDeclarativeGestureArea class provides simple gesture handling.

*/
QDeclarativeGestureArea::QDeclarativeGestureArea(QDeclarativeItem *parent) :
    QDeclarativeItem(*(new QDeclarativeGestureAreaPrivate), parent)
{
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptTouchEvents(true);
}

QDeclarativeGestureArea::~QDeclarativeGestureArea()
{
}

QByteArray
QDeclarativeGestureAreaParser::compile(const QList<QDeclarativeCustomParserProperty> &props)
{
    QByteArray rv;
    QDataStream ds(&rv, QIODevice::WriteOnly);

    for(int ii = 0; ii < props.count(); ++ii)
    {
        QString propName = QString::fromUtf8(props.at(ii).name());
        Qt::GestureType type;

        if (propName == QLatin1String("onTap")) {
            type = Qt::TapGesture;
        } else if (propName == QLatin1String("onTapAndHold")) {
            type = Qt::TapAndHoldGesture;
        } else if (propName == QLatin1String("onPan")) {
            type = Qt::PanGesture;
        } else if (propName == QLatin1String("onPinch")) {
            type = Qt::PinchGesture;
        } else if (propName == QLatin1String("onSwipe")) {
            type = Qt::SwipeGesture;
        } else if (propName == QLatin1String("onGesture")) {
            type = Qt::CustomGesture;
        } else {
            error(props.at(ii), QDeclarativeGestureArea::tr("Cannot assign to non-existent property \"%1\"").arg(propName));
            return QByteArray();
        }

        QList<QVariant> values = props.at(ii).assignedValues();

        for (int i = 0; i < values.count(); ++i) {
            const QVariant &value = values.at(i);

            if (value.userType() == qMetaTypeId<QDeclarativeCustomParserNode>()) {
                error(props.at(ii), QDeclarativeGestureArea::tr("GestureArea: nested objects not allowed"));
                return QByteArray();
            } else if (value.userType() == qMetaTypeId<QDeclarativeCustomParserProperty>()) {
                error(props.at(ii), QDeclarativeGestureArea::tr("GestureArea: syntax error"));
                return QByteArray();
            } else {
                QDeclarativeParser::Variant v = qvariant_cast<QDeclarativeParser::Variant>(value);
                if (v.isScript()) {
                    ds << propName;
                    ds << int(type);
                    ds << v.asScript();
                } else {
                    error(props.at(ii), QDeclarativeGestureArea::tr("GestureArea: script expected"));
                    return QByteArray();
                }
            }
        }
    }

    return rv;
}

void QDeclarativeGestureAreaParser::setCustomData(QObject *object,
                                            const QByteArray &data)
{
    QDeclarativeGestureArea *ga = static_cast<QDeclarativeGestureArea*>(object);
    ga->d_func()->data = data;
}


void QDeclarativeGestureArea::connectSignals()
{
    Q_D(QDeclarativeGestureArea);
    if (!d->componentcomplete)
        return;

    QDataStream ds(d->data);
    while (!ds.atEnd()) {
        QString propName;
        ds >> propName;
        int gesturetype;
        ds >> gesturetype;
        QString script;
        ds >> script;
        QDeclarativeExpression *exp = new QDeclarativeExpression(qmlContext(this), this, script);
        d->bindings.insert(Qt::GestureType(gesturetype),exp);
        grabGesture(Qt::GestureType(gesturetype));
    }
}

void QDeclarativeGestureArea::componentComplete()
{
    QDeclarativeItem::componentComplete();
    Q_D(QDeclarativeGestureArea);
    d->componentcomplete=true;
    connectSignals();
}

QGesture *QDeclarativeGestureArea::gesture() const
{
    Q_D(const QDeclarativeGestureArea);
    return d->gesture;
}

bool QDeclarativeGestureArea::sceneEvent(QEvent *event)
{
    Q_D(QDeclarativeGestureArea);
    if (event->type() == QEvent::Gesture)
        return d->gestureEvent(static_cast<QGestureEvent*>(event));
    return QDeclarativeItem::sceneEvent(event);
}

bool QDeclarativeGestureAreaPrivate::gestureEvent(QGestureEvent *event)
{
    bool accept = true;
    for (Bindings::Iterator it = bindings.begin(); it != bindings.end(); ++it) {
        if ((gesture = event->gesture(it.key()))) {
            QDeclarativeExpression *expr = it.value();
            expr->evaluate();
            if (expr->hasError())
                qmlInfo(q_func()) << expr->error();
            event->setAccepted(true); // XXX only if value returns true?
        }
    }
    return accept;
}

QT_END_NAMESPACE

#endif // QT_NO_GESTURES
