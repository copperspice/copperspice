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

#ifndef QDECLARATIVEANIMATION_H
#define QDECLARATIVEANIMATION_H

#include "private/qdeclarativetransition_p.h"
#include "private/qdeclarativestate_p.h"
#include <QtGui/qvector3d.h>

#include <qdeclarativepropertyvaluesource.h>
#include <qdeclarative.h>
#include <qdeclarativescriptstring.h>

#include <QtCore/qvariant.h>
#include <QtCore/qeasingcurve.h>
#include <QtCore/QAbstractAnimation>
#include <QtGui/qcolor.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QDeclarativeAbstractAnimationPrivate;
class QDeclarativeAnimationGroup;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeAbstractAnimation 
      : public QObject, public QDeclarativePropertyValueSource, public QDeclarativeParserStatus
{
    CS_OBJECT(QDeclarativeAbstractAnimation)
    Q_DECLARE_PRIVATE(QDeclarativeAbstractAnimation)

    CS_INTERFACES(QDeclarativeParserStatus, QDeclarativePropertyValueSource)

    CS_ENUM(Loops)
    CS_PROPERTY_READ(running, isRunning)
    CS_PROPERTY_WRITE(running, setRunning)
    CS_PROPERTY_NOTIFY(running, runningChanged)
    CS_PROPERTY_READ(paused, isPaused)
    CS_PROPERTY_WRITE(paused, setPaused)
    CS_PROPERTY_NOTIFY(paused, pausedChanged)
    CS_PROPERTY_READ(alwaysRunToEnd, alwaysRunToEnd)
    CS_PROPERTY_WRITE(alwaysRunToEnd, setAlwaysRunToEnd)
    CS_PROPERTY_NOTIFY(alwaysRunToEnd, alwaysRunToEndChanged)
    CS_PROPERTY_READ(loops, loops)
    CS_PROPERTY_WRITE(loops, setLoops)
    CS_PROPERTY_NOTIFY(loops, loopCountChanged)

    CS_CLASSINFO("DefaultMethod", "start()")

public:
    QDeclarativeAbstractAnimation(QObject *parent=0);
    virtual ~QDeclarativeAbstractAnimation();

    enum Loops { Infinite = -2 };

    bool isRunning() const;
    void setRunning(bool);
    bool isPaused() const;
    void setPaused(bool);
    bool alwaysRunToEnd() const;
    void setAlwaysRunToEnd(bool);

    int loops() const;
    void setLoops(int);

    int currentTime();
    void setCurrentTime(int);

    QDeclarativeAnimationGroup *group() const;
    void setGroup(QDeclarativeAnimationGroup *);

    void setDefaultTarget(const QDeclarativeProperty &);
    void setDisableUserControl();

    void classBegin();
    void componentComplete();

public:
    CS_SIGNAL_1(Public, void started())
    CS_SIGNAL_2(started) 
    CS_SIGNAL_1(Public, void completed())
    CS_SIGNAL_2(completed) 
    CS_SIGNAL_1(Public, void runningChanged(bool un_named_arg1))
    CS_SIGNAL_2(runningChanged,un_named_arg1) 
    CS_SIGNAL_1(Public, void pausedChanged(bool un_named_arg1))
    CS_SIGNAL_2(pausedChanged,un_named_arg1) 
    CS_SIGNAL_1(Public, void alwaysRunToEndChanged(bool un_named_arg1))
    CS_SIGNAL_2(alwaysRunToEndChanged,un_named_arg1) 
    CS_SIGNAL_1(Public, void loopCountChanged(int un_named_arg1))
    CS_SIGNAL_2(loopCountChanged,un_named_arg1) 

public :
    CS_SLOT_1(Public, void restart())
    CS_SLOT_2(restart) 
    CS_SLOT_1(Public, void start())
    CS_SLOT_2(start) 
    CS_SLOT_1(Public, void pause())
    CS_SLOT_2(pause) 
    CS_SLOT_1(Public, void resume())
    CS_SLOT_2(resume) 
    CS_SLOT_1(Public, void stop())
    CS_SLOT_2(stop) 
    CS_SLOT_1(Public, void complete())
    CS_SLOT_2(complete) 

protected:
    QDeclarativeAbstractAnimation(QDeclarativeAbstractAnimationPrivate &dd, QObject *parent);

public:
    enum TransitionDirection { Forward, Backward };
    virtual void transition(QDeclarativeStateActions &actions,
                            QDeclarativeProperties &modified,
                            TransitionDirection direction);
    virtual QAbstractAnimation *qtAnimation() = 0;

private :
    CS_SLOT_1(Private, void timelineComplete())
    CS_SLOT_2(timelineComplete) 
    CS_SLOT_1(Private, void componentFinalized())
    CS_SLOT_2(componentFinalized) 
private:
    virtual void setTarget(const QDeclarativeProperty &);
    void notifyRunningChanged(bool running);
    friend class QDeclarativeBehavior;


};

class QDeclarativePauseAnimationPrivate;
class QDeclarativePauseAnimation : public QDeclarativeAbstractAnimation
{
    CS_OBJECT(QDeclarativePauseAnimation)
    Q_DECLARE_PRIVATE(QDeclarativePauseAnimation)

    CS_PROPERTY_READ(duration, duration)
    CS_PROPERTY_WRITE(duration, setDuration)
    CS_PROPERTY_NOTIFY(duration, durationChanged)

public:
    QDeclarativePauseAnimation(QObject *parent=0);
    virtual ~QDeclarativePauseAnimation();

    int duration() const;
    void setDuration(int);

public:
    CS_SIGNAL_1(Public, void durationChanged(int un_named_arg1))
    CS_SIGNAL_2(durationChanged,un_named_arg1) 

protected:
    virtual QAbstractAnimation *qtAnimation();
};

class QDeclarativeScriptActionPrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeScriptAction : public QDeclarativeAbstractAnimation
{
    CS_OBJECT(QDeclarativeScriptAction)
    Q_DECLARE_PRIVATE(QDeclarativeScriptAction)

    CS_PROPERTY_READ(script, script)
    CS_PROPERTY_WRITE(script, setScript)
    CS_PROPERTY_READ(scriptName, stateChangeScriptName)
    CS_PROPERTY_WRITE(scriptName, setStateChangeScriptName)

public:
    QDeclarativeScriptAction(QObject *parent=0);
    virtual ~QDeclarativeScriptAction();

    QDeclarativeScriptString script() const;
    void setScript(const QDeclarativeScriptString &);

    QString stateChangeScriptName() const;
    void setStateChangeScriptName(const QString &);

protected:
    virtual void transition(QDeclarativeStateActions &actions,
                            QDeclarativeProperties &modified,
                            TransitionDirection direction);
    virtual QAbstractAnimation *qtAnimation();
};

class QDeclarativePropertyActionPrivate;
class QDeclarativePropertyAction : public QDeclarativeAbstractAnimation
{
    CS_OBJECT(QDeclarativePropertyAction)
    Q_DECLARE_PRIVATE(QDeclarativePropertyAction)

    CS_PROPERTY_READ(*target, target)
    CS_PROPERTY_WRITE(*target, setTarget)
    CS_PROPERTY_NOTIFY(*target, targetChanged)
    CS_PROPERTY_READ(property, property)
    CS_PROPERTY_WRITE(property, setProperty)
    CS_PROPERTY_NOTIFY(property, propertyChanged)
    CS_PROPERTY_READ(properties, properties)
    CS_PROPERTY_WRITE(properties, setProperties)
    CS_PROPERTY_NOTIFY(properties, propertiesChanged)
    CS_PROPERTY_READ(targets, targets)
    CS_PROPERTY_READ(exclude, exclude)
    CS_PROPERTY_READ(value, value)
    CS_PROPERTY_WRITE(value, setValue)
    CS_PROPERTY_NOTIFY(value, valueChanged)

public:
    QDeclarativePropertyAction(QObject *parent=0);
    virtual ~QDeclarativePropertyAction();

    QObject *target() const;
    void setTarget(QObject *);

    QString property() const;
    void setProperty(const QString &);

    QString properties() const;
    void setProperties(const QString &);

    QDeclarativeListProperty<QObject> targets();
    QDeclarativeListProperty<QObject> exclude();

    QVariant value() const;
    void setValue(const QVariant &);

public:
    CS_SIGNAL_1(Public, void valueChanged(const QVariant & un_named_arg1))
    CS_SIGNAL_2(valueChanged,un_named_arg1) 
    CS_SIGNAL_1(Public, void propertiesChanged(const QString & un_named_arg1))
    CS_SIGNAL_2(propertiesChanged,un_named_arg1) 
    CS_SIGNAL_1(Public, void targetChanged())
    CS_SIGNAL_2(targetChanged) 
    CS_SIGNAL_1(Public, void propertyChanged())
    CS_SIGNAL_2(propertyChanged) 

protected:
    virtual void transition(QDeclarativeStateActions &actions,
                            QDeclarativeProperties &modified,
                            TransitionDirection direction);
    virtual QAbstractAnimation *qtAnimation();
};

class QDeclarativeItem;
class QDeclarativePropertyAnimationPrivate;
class QDeclarativePropertyAnimation : public QDeclarativeAbstractAnimation
{
    CS_OBJECT(QDeclarativePropertyAnimation)
    Q_DECLARE_PRIVATE(QDeclarativePropertyAnimation)

    CS_PROPERTY_READ(duration, duration)
    CS_PROPERTY_WRITE(duration, setDuration)
    CS_PROPERTY_NOTIFY(duration, durationChanged)
    CS_PROPERTY_READ(from, from)
    CS_PROPERTY_WRITE(from, setFrom)
    CS_PROPERTY_NOTIFY(from, fromChanged)
    CS_PROPERTY_READ(to, to)
    CS_PROPERTY_WRITE(to, setTo)
    CS_PROPERTY_NOTIFY(to, toChanged)
    CS_PROPERTY_READ(easing, easing)
    CS_PROPERTY_WRITE(easing, setEasing)
    CS_PROPERTY_NOTIFY(easing, easingChanged)
    CS_PROPERTY_READ(*target, target)
    CS_PROPERTY_WRITE(*target, setTarget)
    CS_PROPERTY_NOTIFY(*target, targetChanged)
    CS_PROPERTY_READ(property, property)
    CS_PROPERTY_WRITE(property, setProperty)
    CS_PROPERTY_NOTIFY(property, propertyChanged)
    CS_PROPERTY_READ(properties, properties)
    CS_PROPERTY_WRITE(properties, setProperties)
    CS_PROPERTY_NOTIFY(properties, propertiesChanged)
    CS_PROPERTY_READ(targets, targets)
    CS_PROPERTY_READ(exclude, exclude)

public:
    QDeclarativePropertyAnimation(QObject *parent=0);
    virtual ~QDeclarativePropertyAnimation();

    virtual int duration() const;
    virtual void setDuration(int);

    QVariant from() const;
    void setFrom(const QVariant &);

    QVariant to() const;
    void setTo(const QVariant &);

    QEasingCurve easing() const;
    void setEasing(const QEasingCurve &);

    QObject *target() const;
    void setTarget(QObject *);

    QString property() const;
    void setProperty(const QString &);

    QString properties() const;
    void setProperties(const QString &);

    QDeclarativeListProperty<QObject> targets();
    QDeclarativeListProperty<QObject> exclude();

protected:
    QDeclarativePropertyAnimation(QDeclarativePropertyAnimationPrivate &dd, QObject *parent);
    virtual void transition(QDeclarativeStateActions &actions,
                            QDeclarativeProperties &modified,
                            TransitionDirection direction);
    virtual QAbstractAnimation *qtAnimation();

public:
    CS_SIGNAL_1(Public, void durationChanged(int un_named_arg1))
    CS_SIGNAL_2(durationChanged,un_named_arg1) 
    CS_SIGNAL_1(Public, void fromChanged(QVariant un_named_arg1))
    CS_SIGNAL_2(fromChanged,un_named_arg1) 
    CS_SIGNAL_1(Public, void toChanged(QVariant un_named_arg1))
    CS_SIGNAL_2(toChanged,un_named_arg1) 
    CS_SIGNAL_1(Public, void easingChanged(const QEasingCurve & un_named_arg1))
    CS_SIGNAL_2(easingChanged,un_named_arg1) 
    CS_SIGNAL_1(Public, void propertiesChanged(const QString & un_named_arg1))
    CS_SIGNAL_2(propertiesChanged,un_named_arg1) 
    CS_SIGNAL_1(Public, void targetChanged())
    CS_SIGNAL_2(targetChanged) 
    CS_SIGNAL_1(Public, void propertyChanged())
    CS_SIGNAL_2(propertyChanged) 
};

class QDeclarativeColorAnimation : public QDeclarativePropertyAnimation
{
    CS_OBJECT(QDeclarativeColorAnimation)
    Q_DECLARE_PRIVATE(QDeclarativePropertyAnimation)
    CS_PROPERTY_READ(from, from)
    CS_PROPERTY_WRITE(from, setFrom)
    CS_PROPERTY_READ(to, to)
    CS_PROPERTY_WRITE(to, setTo)

public:
    QDeclarativeColorAnimation(QObject *parent=0);
    virtual ~QDeclarativeColorAnimation();

    QColor from() const;
    void setFrom(const QColor &);

    QColor to() const;
    void setTo(const QColor &);
};

class QDeclarativeNumberAnimation : public QDeclarativePropertyAnimation
{
    CS_OBJECT(QDeclarativeNumberAnimation)
    Q_DECLARE_PRIVATE(QDeclarativePropertyAnimation)

    CS_PROPERTY_READ(from, from)
    CS_PROPERTY_WRITE(from, setFrom)
    CS_PROPERTY_READ(to, to)
    CS_PROPERTY_WRITE(to, setTo)

public:
    QDeclarativeNumberAnimation(QObject *parent=0);
    virtual ~QDeclarativeNumberAnimation();

    qreal from() const;
    void setFrom(qreal);

    qreal to() const;
    void setTo(qreal);

protected:
    QDeclarativeNumberAnimation(QDeclarativePropertyAnimationPrivate &dd, QObject *parent);

private:
    void init();
};

class QDeclarativeVector3dAnimation : public QDeclarativePropertyAnimation
{
    CS_OBJECT(QDeclarativeVector3dAnimation)
    Q_DECLARE_PRIVATE(QDeclarativePropertyAnimation)

    CS_PROPERTY_READ(from, from)
    CS_PROPERTY_WRITE(from, setFrom)
    CS_PROPERTY_READ(to, to)
    CS_PROPERTY_WRITE(to, setTo)

public:
    QDeclarativeVector3dAnimation(QObject *parent=0);
    virtual ~QDeclarativeVector3dAnimation();

    QVector3D from() const;
    void setFrom(QVector3D);

    QVector3D to() const;
    void setTo(QVector3D);
};

class QDeclarativeRotationAnimationPrivate;
class QDeclarativeRotationAnimation : public QDeclarativePropertyAnimation
{
    CS_OBJECT(QDeclarativeRotationAnimation)
    Q_DECLARE_PRIVATE(QDeclarativeRotationAnimation)
    CS_ENUM(RotationDirection)

    CS_PROPERTY_READ(from, from)
    CS_PROPERTY_WRITE(from, setFrom)
    CS_PROPERTY_READ(to, to)
    CS_PROPERTY_WRITE(to, setTo)
    CS_PROPERTY_READ(direction, direction)
    CS_PROPERTY_WRITE(direction, setDirection)
    CS_PROPERTY_NOTIFY(direction, directionChanged)

public:
    QDeclarativeRotationAnimation(QObject *parent=0);
    virtual ~QDeclarativeRotationAnimation();

    qreal from() const;
    void setFrom(qreal);

    qreal to() const;
    void setTo(qreal);

    enum RotationDirection { Numerical, Shortest, Clockwise, Counterclockwise };
    RotationDirection direction() const;
    void setDirection(RotationDirection direction);

public:
    CS_SIGNAL_1(Public, void directionChanged())
    CS_SIGNAL_2(directionChanged) 
};

class QDeclarativeAnimationGroupPrivate;
class QDeclarativeAnimationGroup : public QDeclarativeAbstractAnimation
{
    CS_OBJECT(QDeclarativeAnimationGroup)
    Q_DECLARE_PRIVATE(QDeclarativeAnimationGroup)

    CS_CLASSINFO("DefaultProperty", "animations")
    CS_PROPERTY_READ(animations, animations)

public:
    QDeclarativeAnimationGroup(QObject *parent);
    virtual ~QDeclarativeAnimationGroup();

    QDeclarativeListProperty<QDeclarativeAbstractAnimation> animations();
    friend class QDeclarativeAbstractAnimation;

protected:
    QDeclarativeAnimationGroup(QDeclarativeAnimationGroupPrivate &dd, QObject *parent);
};

class QDeclarativeSequentialAnimation : public QDeclarativeAnimationGroup
{
    CS_OBJECT(QDeclarativeSequentialAnimation)
    Q_DECLARE_PRIVATE(QDeclarativeAnimationGroup)

public:
    QDeclarativeSequentialAnimation(QObject *parent=0);
    virtual ~QDeclarativeSequentialAnimation();

protected:
    virtual void transition(QDeclarativeStateActions &actions,
                            QDeclarativeProperties &modified,
                            TransitionDirection direction);
    virtual QAbstractAnimation *qtAnimation();
};

class QDeclarativeParallelAnimation : public QDeclarativeAnimationGroup
{
    CS_OBJECT(QDeclarativeParallelAnimation)
    Q_DECLARE_PRIVATE(QDeclarativeAnimationGroup)

public:
    QDeclarativeParallelAnimation(QObject *parent=0);
    virtual ~QDeclarativeParallelAnimation();

protected:
    virtual void transition(QDeclarativeStateActions &actions,
                            QDeclarativeProperties &modified,
                            TransitionDirection direction);
    virtual QAbstractAnimation *qtAnimation();
};

class QDeclarativeParentAnimationPrivate;
class QDeclarativeParentAnimation : public QDeclarativeAnimationGroup
{
    CS_OBJECT(QDeclarativeParentAnimation)
    Q_DECLARE_PRIVATE(QDeclarativeParentAnimation)

    CS_PROPERTY_READ(*target, target)
    CS_PROPERTY_WRITE(*target, setTarget)
    CS_PROPERTY_NOTIFY(*target, targetChanged)
    CS_PROPERTY_READ(*newParent, newParent)
    CS_PROPERTY_WRITE(*newParent, setNewParent)
    CS_PROPERTY_NOTIFY(*newParent, newParentChanged)
    CS_PROPERTY_READ(*via, via)
    CS_PROPERTY_WRITE(*via, setVia)
    CS_PROPERTY_NOTIFY(*via, viaChanged)

public:
    QDeclarativeParentAnimation(QObject *parent=0);
    virtual ~QDeclarativeParentAnimation();

    QDeclarativeItem *target() const;
    void setTarget(QDeclarativeItem *);

    QDeclarativeItem *newParent() const;
    void setNewParent(QDeclarativeItem *);

    QDeclarativeItem *via() const;
    void setVia(QDeclarativeItem *);

public:
    CS_SIGNAL_1(Public, void targetChanged())
    CS_SIGNAL_2(targetChanged) 
    CS_SIGNAL_1(Public, void newParentChanged())
    CS_SIGNAL_2(newParentChanged) 
    CS_SIGNAL_1(Public, void viaChanged())
    CS_SIGNAL_2(viaChanged) 

protected:
    virtual void transition(QDeclarativeStateActions &actions,
                            QDeclarativeProperties &modified,
                            TransitionDirection direction);
    virtual QAbstractAnimation *qtAnimation();
};

class QDeclarativeAnchorAnimationPrivate;
class QDeclarativeAnchorAnimation : public QDeclarativeAbstractAnimation
{
    CS_OBJECT(QDeclarativeAnchorAnimation)
    Q_DECLARE_PRIVATE(QDeclarativeAnchorAnimation)
    CS_PROPERTY_READ(targets, targets)
    CS_PROPERTY_READ(duration, duration)
    CS_PROPERTY_WRITE(duration, setDuration)
    CS_PROPERTY_NOTIFY(duration, durationChanged)
    CS_PROPERTY_READ(easing, easing)
    CS_PROPERTY_WRITE(easing, setEasing)
    CS_PROPERTY_NOTIFY(easing, easingChanged)

public:
    QDeclarativeAnchorAnimation(QObject *parent=0);
    virtual ~QDeclarativeAnchorAnimation();

    QDeclarativeListProperty<QDeclarativeItem> targets();

    int duration() const;
    void setDuration(int);

    QEasingCurve easing() const;
    void setEasing(const QEasingCurve &);

public:
    CS_SIGNAL_1(Public, void durationChanged(int un_named_arg1))
    CS_SIGNAL_2(durationChanged,un_named_arg1) 
    CS_SIGNAL_1(Public, void easingChanged(const QEasingCurve & un_named_arg1))
    CS_SIGNAL_2(easingChanged,un_named_arg1) 

protected:
    virtual void transition(QDeclarativeStateActions &actions,
                            QDeclarativeProperties &modified,
                            TransitionDirection direction);
    virtual QAbstractAnimation *qtAnimation();
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeAbstractAnimation)
QML_DECLARE_TYPE(QDeclarativePauseAnimation)
QML_DECLARE_TYPE(QDeclarativeScriptAction)
QML_DECLARE_TYPE(QDeclarativePropertyAction)
QML_DECLARE_TYPE(QDeclarativePropertyAnimation)
QML_DECLARE_TYPE(QDeclarativeColorAnimation)
QML_DECLARE_TYPE(QDeclarativeNumberAnimation)
QML_DECLARE_TYPE(QDeclarativeSequentialAnimation)
QML_DECLARE_TYPE(QDeclarativeParallelAnimation)
QML_DECLARE_TYPE(QDeclarativeVector3dAnimation)
QML_DECLARE_TYPE(QDeclarativeRotationAnimation)
QML_DECLARE_TYPE(QDeclarativeParentAnimation)
QML_DECLARE_TYPE(QDeclarativeAnchorAnimation)

QT_END_HEADER

#endif // QDECLARATIVEANIMATION_H
