/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QDECLARATIVEGESTUREAREA_H
#define QDECLARATIVEGESTUREAREA_H

#include <qdeclarativeitem.h>
#include <qdeclarativescriptstring.h>
#include <qdeclarativecustomparser_p.h>

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtGui/qgesture.h>

#ifndef QT_NO_GESTURES

QT_BEGIN_NAMESPACE

class QDeclarativeBoundSignal;
class QDeclarativeContext;
class QDeclarativeGestureAreaPrivate;
class QDeclarativeGestureArea : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QGesture *gesture READ gesture)

public:
    QDeclarativeGestureArea(QDeclarativeItem *parent=0);
    ~QDeclarativeGestureArea();

    QGesture *gesture() const;

protected:
    bool sceneEvent(QEvent *event);

private:
    void connectSignals();
    void componentComplete();
    friend class QDeclarativeGestureAreaParser;

    Q_DISABLE_COPY(QDeclarativeGestureArea)
    Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeGestureArea)
};

class QDeclarativeGestureAreaParser : public QDeclarativeCustomParser
{
public:
    virtual QByteArray compile(const QList<QDeclarativeCustomParserProperty> &);
    virtual void setCustomData(QObject *, const QByteArray &);
};


QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeGestureArea)

#endif // QT_NO_GESTURES

#endif
