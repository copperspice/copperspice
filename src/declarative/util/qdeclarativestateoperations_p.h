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

#ifndef QDECLARATIVESTATEOPERATIONS_H
#define QDECLARATIVESTATEOPERATIONS_H

#include "private/qdeclarativestate_p.h"

#include <qdeclarativeitem.h>
#include <private/qdeclarativeanchors_p.h>
#include <qdeclarativescriptstring.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QDeclarativeParentChangePrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeParentChange : public QDeclarativeStateOperation, public QDeclarativeActionEvent
{
    CS_OBJECT(QDeclarativeParentChange)
    Q_DECLARE_PRIVATE(QDeclarativeParentChange)

    CS_PROPERTY_READ(*target, object)
    CS_PROPERTY_WRITE(*target, setObject)
    CS_PROPERTY_READ(*parent, parent)
    CS_PROPERTY_WRITE(*parent, setParent)
    CS_PROPERTY_READ(x, x)
    CS_PROPERTY_WRITE(x, setX)
    CS_PROPERTY_READ(y, y)
    CS_PROPERTY_WRITE(y, setY)
    CS_PROPERTY_READ(width, width)
    CS_PROPERTY_WRITE(width, setWidth)
    CS_PROPERTY_READ(height, height)
    CS_PROPERTY_WRITE(height, setHeight)
    CS_PROPERTY_READ(scale, scale)
    CS_PROPERTY_WRITE(scale, setScale)
    CS_PROPERTY_READ(rotation, rotation)
    CS_PROPERTY_WRITE(rotation, setRotation)
public:
    QDeclarativeParentChange(QObject *parent=0);
    ~QDeclarativeParentChange();

    QDeclarativeItem *object() const;
    void setObject(QDeclarativeItem *);

    QDeclarativeItem *parent() const;
    void setParent(QDeclarativeItem *);

    QDeclarativeItem *originalParent() const;

    QDeclarativeScriptString x() const;
    void setX(QDeclarativeScriptString x);
    bool xIsSet() const;

    QDeclarativeScriptString y() const;
    void setY(QDeclarativeScriptString y);
    bool yIsSet() const;

    QDeclarativeScriptString width() const;
    void setWidth(QDeclarativeScriptString width);
    bool widthIsSet() const;

    QDeclarativeScriptString height() const;
    void setHeight(QDeclarativeScriptString height);
    bool heightIsSet() const;

    QDeclarativeScriptString scale() const;
    void setScale(QDeclarativeScriptString scale);
    bool scaleIsSet() const;

    QDeclarativeScriptString rotation() const;
    void setRotation(QDeclarativeScriptString rotation);
    bool rotationIsSet() const;

    virtual ActionList actions();

    virtual void saveOriginals();
    //virtual void copyOriginals(QDeclarativeActionEvent*);
    virtual void execute(Reason reason = ActualChange);
    virtual bool isReversable();
    virtual void reverse(Reason reason = ActualChange);
    virtual QString typeName() const;
    virtual bool override(QDeclarativeActionEvent*other);
    virtual void rewind();
    virtual void saveCurrentValues();
};

class QDeclarativeStateChangeScriptPrivate;
class QDeclarativeStateChangeScript : public QDeclarativeStateOperation, public QDeclarativeActionEvent
{
    CS_OBJECT(QDeclarativeStateChangeScript)
    Q_DECLARE_PRIVATE(QDeclarativeStateChangeScript)

    CS_PROPERTY_READ(script, script)
    CS_PROPERTY_WRITE(script, setScript)
    CS_PROPERTY_READ(name, name)
    CS_PROPERTY_WRITE(name, setName)

public:
    QDeclarativeStateChangeScript(QObject *parent=0);
    ~QDeclarativeStateChangeScript();

    virtual ActionList actions();

    virtual QString typeName() const;

    QDeclarativeScriptString script() const;
    void setScript(const QDeclarativeScriptString &);
    
    QString name() const;
    void setName(const QString &);

    virtual void execute(Reason reason = ActualChange);
};

class QDeclarativeAnchorChanges;
class QDeclarativeAnchorSetPrivate;
class QDeclarativeAnchorSet : public QObject
{
    CS_OBJECT(QDeclarativeAnchorSet)

    CS_PROPERTY_READ(left, left)
    CS_PROPERTY_WRITE(left, setLeft)
    CS_PROPERTY_RESET(left, resetLeft)
    CS_PROPERTY_READ(right, right)
    CS_PROPERTY_WRITE(right, setRight)
    CS_PROPERTY_RESET(right, resetRight)
    CS_PROPERTY_READ(horizontalCenter, horizontalCenter)
    CS_PROPERTY_WRITE(horizontalCenter, setHorizontalCenter)
    CS_PROPERTY_RESET(horizontalCenter, resetHorizontalCenter)
    CS_PROPERTY_READ(top, top)
    CS_PROPERTY_WRITE(top, setTop)
    CS_PROPERTY_RESET(top, resetTop)
    CS_PROPERTY_READ(bottom, bottom)
    CS_PROPERTY_WRITE(bottom, setBottom)
    CS_PROPERTY_RESET(bottom, resetBottom)
    CS_PROPERTY_READ(verticalCenter, verticalCenter)
    CS_PROPERTY_WRITE(verticalCenter, setVerticalCenter)
    CS_PROPERTY_RESET(verticalCenter, resetVerticalCenter)
    CS_PROPERTY_READ(baseline, baseline)
    CS_PROPERTY_WRITE(baseline, setBaseline)
    CS_PROPERTY_RESET(baseline, resetBaseline)
    //Q_PROPERTY(QDeclarativeItem *fill READ fill WRITE setFill RESET resetFill)
    //Q_PROPERTY(QDeclarativeItem *centerIn READ centerIn WRITE setCenterIn RESET resetCenterIn)

    /*Q_PROPERTY(qreal margins READ margins WRITE setMargins NOTIFY marginsChanged)
    Q_PROPERTY(qreal leftMargin READ leftMargin WRITE setLeftMargin NOTIFY leftMarginChanged)
    Q_PROPERTY(qreal rightMargin READ rightMargin WRITE setRightMargin NOTIFY rightMarginChanged)
    Q_PROPERTY(qreal horizontalCenterOffset READ horizontalCenterOffset WRITE setHorizontalCenterOffset NOTIFY horizontalCenterOffsetChanged())
    Q_PROPERTY(qreal topMargin READ topMargin WRITE setTopMargin NOTIFY topMarginChanged)
    Q_PROPERTY(qreal bottomMargin READ bottomMargin WRITE setBottomMargin NOTIFY bottomMarginChanged)
    Q_PROPERTY(qreal verticalCenterOffset READ verticalCenterOffset WRITE setVerticalCenterOffset NOTIFY verticalCenterOffsetChanged())
    Q_PROPERTY(qreal baselineOffset READ baselineOffset WRITE setBaselineOffset NOTIFY baselineOffsetChanged())*/

public:
    QDeclarativeAnchorSet(QObject *parent=0);
    virtual ~QDeclarativeAnchorSet();

    QDeclarativeScriptString left() const;
    void setLeft(const QDeclarativeScriptString &edge);
    void resetLeft();

    QDeclarativeScriptString right() const;
    void setRight(const QDeclarativeScriptString &edge);
    void resetRight();

    QDeclarativeScriptString horizontalCenter() const;
    void setHorizontalCenter(const QDeclarativeScriptString &edge);
    void resetHorizontalCenter();

    QDeclarativeScriptString top() const;
    void setTop(const QDeclarativeScriptString &edge);
    void resetTop();

    QDeclarativeScriptString bottom() const;
    void setBottom(const QDeclarativeScriptString &edge);
    void resetBottom();

    QDeclarativeScriptString verticalCenter() const;
    void setVerticalCenter(const QDeclarativeScriptString &edge);
    void resetVerticalCenter();

    QDeclarativeScriptString baseline() const;
    void setBaseline(const QDeclarativeScriptString &edge);
    void resetBaseline();

    QDeclarativeItem *fill() const;
    void setFill(QDeclarativeItem *);
    void resetFill();

    QDeclarativeItem *centerIn() const;
    void setCenterIn(QDeclarativeItem *);
    void resetCenterIn();

    /*qreal leftMargin() const;
    void setLeftMargin(qreal);

    qreal rightMargin() const;
    void setRightMargin(qreal);

    qreal horizontalCenterOffset() const;
    void setHorizontalCenterOffset(qreal);

    qreal topMargin() const;
    void setTopMargin(qreal);

    qreal bottomMargin() const;
    void setBottomMargin(qreal);

    qreal margins() const;
    void setMargins(qreal);

    qreal verticalCenterOffset() const;
    void setVerticalCenterOffset(qreal);

    qreal baselineOffset() const;
    void setBaselineOffset(qreal);*/

    QDeclarativeAnchors::Anchors usedAnchors() const;

/*Q_SIGNALS:
    void leftMarginChanged();
    void rightMarginChanged();
    void topMarginChanged();
    void bottomMarginChanged();
    void marginsChanged();
    void verticalCenterOffsetChanged();
    void horizontalCenterOffsetChanged();
    void baselineOffsetChanged();*/

private:
    friend class QDeclarativeAnchorChanges;
    Q_DISABLE_COPY(QDeclarativeAnchorSet)
    Q_DECLARE_PRIVATE(QDeclarativeAnchorSet)
};

class QDeclarativeAnchorChangesPrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeAnchorChanges : public QDeclarativeStateOperation, public QDeclarativeActionEvent
{
    CS_OBJECT(QDeclarativeAnchorChanges)
    Q_DECLARE_PRIVATE(QDeclarativeAnchorChanges)

    CS_PROPERTY_READ(*target, object)
    CS_PROPERTY_WRITE(*target, setObject)
    CS_PROPERTY_READ(*anchors, anchors)
    CS_PROPERTY_CONSTANT(*anchors)

public:
    QDeclarativeAnchorChanges(QObject *parent=0);
    ~QDeclarativeAnchorChanges();

    virtual ActionList actions();

    QDeclarativeAnchorSet *anchors();

    QDeclarativeItem *object() const;
    void setObject(QDeclarativeItem *);

    virtual void execute(Reason reason = ActualChange);
    virtual bool isReversable();
    virtual void reverse(Reason reason = ActualChange);
    virtual QString typeName() const;
    virtual bool override(QDeclarativeActionEvent*other);
    virtual bool changesBindings();
    virtual void saveOriginals();
    virtual bool needsCopy() { return true; }
    virtual void copyOriginals(QDeclarativeActionEvent*);
    virtual void clearBindings();
    virtual void rewind();
    virtual void saveCurrentValues();

    QList<QDeclarativeAction> additionalActions();
    virtual void saveTargetValues();
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeParentChange)
QML_DECLARE_TYPE(QDeclarativeStateChangeScript)
QML_DECLARE_TYPE(QDeclarativeAnchorSet)
QML_DECLARE_TYPE(QDeclarativeAnchorChanges)

QT_END_HEADER

#endif // QDECLARATIVESTATEOPERATIONS_H
