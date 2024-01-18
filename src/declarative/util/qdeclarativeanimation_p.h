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

#ifndef QDECLARATIVEANIMATION_P_H
#define QDECLARATIVEANIMATION_P_H

#include <qdeclarativetransition_p.h>
#include <qdeclarativestate_p.h>
#include <QtGui/qvector3d.h>
#include <qdeclarativepropertyvaluesource.h>
#include <qdeclarative.h>
#include <qdeclarativescriptstring.h>
#include <QtCore/qvariant.h>
#include <QtCore/qeasingcurve.h>
#include <QtCore/QAbstractAnimation>
#include <QtGui/qcolor.h>

QT_BEGIN_NAMESPACE

class QDeclarativeAbstractAnimationPrivate;
class QDeclarativeAnimationGroup;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeAbstractAnimation
   : public QObject, public QDeclarativePropertyValueSource, public QDeclarativeParserStatus
{
   DECL_CS_OBJECT(QDeclarativeAbstractAnimation)
   Q_DECLARE_PRIVATE(QDeclarativeAbstractAnimation)

   CS_INTERFACES(QDeclarativeParserStatus, QDeclarativePropertyValueSource)

   CS_ENUM(Loops)
   DECL_CS_PROPERTY_READ(running, isRunning)
   DECL_CS_PROPERTY_WRITE(running, setRunning)
   DECL_CS_PROPERTY_NOTIFY(running, runningChanged)
   DECL_CS_PROPERTY_READ(paused, isPaused)
   DECL_CS_PROPERTY_WRITE(paused, setPaused)
   DECL_CS_PROPERTY_NOTIFY(paused, pausedChanged)
   DECL_CS_PROPERTY_READ(alwaysRunToEnd, alwaysRunToEnd)
   DECL_CS_PROPERTY_WRITE(alwaysRunToEnd, setAlwaysRunToEnd)
   DECL_CS_PROPERTY_NOTIFY(alwaysRunToEnd, alwaysRunToEndChanged)
   DECL_CS_PROPERTY_READ(loops, loops)
   DECL_CS_PROPERTY_WRITE(loops, setLoops)
   DECL_CS_PROPERTY_NOTIFY(loops, loopCountChanged)

   DECL_CS_CLASSINFO("DefaultMethod", "start()")

 public:
   QDeclarativeAbstractAnimation(QObject *parent = nullptr);
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
   DECL_CS_SIGNAL_1(Public, void started())
   DECL_CS_SIGNAL_2(started)
   DECL_CS_SIGNAL_1(Public, void completed())
   DECL_CS_SIGNAL_2(completed)
   DECL_CS_SIGNAL_1(Public, void runningChanged(bool un_named_arg1))
   DECL_CS_SIGNAL_2(runningChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void pausedChanged(bool un_named_arg1))
   DECL_CS_SIGNAL_2(pausedChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void alwaysRunToEndChanged(bool un_named_arg1))
   DECL_CS_SIGNAL_2(alwaysRunToEndChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void loopCountChanged(int un_named_arg1))
   DECL_CS_SIGNAL_2(loopCountChanged, un_named_arg1)

 public :
   DECL_CS_SLOT_1(Public, void restart())
   DECL_CS_SLOT_2(restart)
   DECL_CS_SLOT_1(Public, void start())
   DECL_CS_SLOT_2(start)
   DECL_CS_SLOT_1(Public, void pause())
   DECL_CS_SLOT_2(pause)
   DECL_CS_SLOT_1(Public, void resume())
   DECL_CS_SLOT_2(resume)
   DECL_CS_SLOT_1(Public, void stop())
   DECL_CS_SLOT_2(stop)
   DECL_CS_SLOT_1(Public, void complete())
   DECL_CS_SLOT_2(complete)

 protected:
   QDeclarativeAbstractAnimation(QDeclarativeAbstractAnimationPrivate &dd, QObject *parent);

 public:
   enum TransitionDirection { Forward, Backward };
   virtual void transition(QDeclarativeStateActions &actions,
                           QDeclarativeProperties &modified,
                           TransitionDirection direction);
   virtual QAbstractAnimation *qtAnimation() = 0;

 private :
   DECL_CS_SLOT_1(Private, void timelineComplete())
   DECL_CS_SLOT_2(timelineComplete)
   DECL_CS_SLOT_1(Private, void componentFinalized())
   DECL_CS_SLOT_2(componentFinalized)
 private:
   virtual void setTarget(const QDeclarativeProperty &);
   void notifyRunningChanged(bool running);
   friend class QDeclarativeBehavior;


};

class QDeclarativePauseAnimationPrivate;
class QDeclarativePauseAnimation : public QDeclarativeAbstractAnimation
{
   DECL_CS_OBJECT(QDeclarativePauseAnimation)
   Q_DECLARE_PRIVATE(QDeclarativePauseAnimation)

   DECL_CS_PROPERTY_READ(duration, duration)
   DECL_CS_PROPERTY_WRITE(duration, setDuration)
   DECL_CS_PROPERTY_NOTIFY(duration, durationChanged)

 public:
   QDeclarativePauseAnimation(QObject *parent = nullptr);
   virtual ~QDeclarativePauseAnimation();

   int duration() const;
   void setDuration(int);

 public:
   DECL_CS_SIGNAL_1(Public, void durationChanged(int un_named_arg1))
   DECL_CS_SIGNAL_2(durationChanged, un_named_arg1)

 protected:
   virtual QAbstractAnimation *qtAnimation();
};

class QDeclarativeScriptActionPrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeScriptAction : public QDeclarativeAbstractAnimation
{
   DECL_CS_OBJECT(QDeclarativeScriptAction)
   Q_DECLARE_PRIVATE(QDeclarativeScriptAction)

   DECL_CS_PROPERTY_READ(script, script)
   DECL_CS_PROPERTY_WRITE(script, setScript)
   DECL_CS_PROPERTY_READ(scriptName, stateChangeScriptName)
   DECL_CS_PROPERTY_WRITE(scriptName, setStateChangeScriptName)

 public:
   QDeclarativeScriptAction(QObject *parent = nullptr);
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
   DECL_CS_OBJECT(QDeclarativePropertyAction)
   Q_DECLARE_PRIVATE(QDeclarativePropertyAction)

   DECL_CS_PROPERTY_READ(*target, target)
   DECL_CS_PROPERTY_WRITE(*target, setTarget)
   DECL_CS_PROPERTY_NOTIFY(*target, targetChanged)
   DECL_CS_PROPERTY_READ(property, property)
   DECL_CS_PROPERTY_WRITE(property, setProperty)
   DECL_CS_PROPERTY_NOTIFY(property, propertyChanged)
   DECL_CS_PROPERTY_READ(properties, properties)
   DECL_CS_PROPERTY_WRITE(properties, setProperties)
   DECL_CS_PROPERTY_NOTIFY(properties, propertiesChanged)
   DECL_CS_PROPERTY_READ(targets, targets)
   DECL_CS_PROPERTY_READ(exclude, exclude)
   DECL_CS_PROPERTY_READ(value, value)
   DECL_CS_PROPERTY_WRITE(value, setValue)
   DECL_CS_PROPERTY_NOTIFY(value, valueChanged)

 public:
   QDeclarativePropertyAction(QObject *parent = nullptr);
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
   DECL_CS_SIGNAL_1(Public, void valueChanged(const QVariant &un_named_arg1))
   DECL_CS_SIGNAL_2(valueChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void propertiesChanged(const QString &un_named_arg1))
   DECL_CS_SIGNAL_2(propertiesChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void targetChanged())
   DECL_CS_SIGNAL_2(targetChanged)
   DECL_CS_SIGNAL_1(Public, void propertyChanged())
   DECL_CS_SIGNAL_2(propertyChanged)

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
   DECL_CS_OBJECT(QDeclarativePropertyAnimation)
   Q_DECLARE_PRIVATE(QDeclarativePropertyAnimation)

   DECL_CS_PROPERTY_READ(duration, duration)
   DECL_CS_PROPERTY_WRITE(duration, setDuration)
   DECL_CS_PROPERTY_NOTIFY(duration, durationChanged)
   DECL_CS_PROPERTY_READ(from, from)
   DECL_CS_PROPERTY_WRITE(from, setFrom)
   DECL_CS_PROPERTY_NOTIFY(from, fromChanged)
   DECL_CS_PROPERTY_READ(to, to)
   DECL_CS_PROPERTY_WRITE(to, setTo)
   DECL_CS_PROPERTY_NOTIFY(to, toChanged)
   DECL_CS_PROPERTY_READ(easing, easing)
   DECL_CS_PROPERTY_WRITE(easing, setEasing)
   DECL_CS_PROPERTY_NOTIFY(easing, easingChanged)
   DECL_CS_PROPERTY_READ(*target, target)
   DECL_CS_PROPERTY_WRITE(*target, setTarget)
   DECL_CS_PROPERTY_NOTIFY(*target, targetChanged)
   DECL_CS_PROPERTY_READ(property, property)
   DECL_CS_PROPERTY_WRITE(property, setProperty)
   DECL_CS_PROPERTY_NOTIFY(property, propertyChanged)
   DECL_CS_PROPERTY_READ(properties, properties)
   DECL_CS_PROPERTY_WRITE(properties, setProperties)
   DECL_CS_PROPERTY_NOTIFY(properties, propertiesChanged)
   DECL_CS_PROPERTY_READ(targets, targets)
   DECL_CS_PROPERTY_READ(exclude, exclude)

 public:
   QDeclarativePropertyAnimation(QObject *parent = nullptr);
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
   DECL_CS_SIGNAL_1(Public, void durationChanged(int un_named_arg1))
   DECL_CS_SIGNAL_2(durationChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void fromChanged(QVariant un_named_arg1))
   DECL_CS_SIGNAL_2(fromChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void toChanged(QVariant un_named_arg1))
   DECL_CS_SIGNAL_2(toChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void easingChanged(const QEasingCurve &un_named_arg1))
   DECL_CS_SIGNAL_2(easingChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void propertiesChanged(const QString &un_named_arg1))
   DECL_CS_SIGNAL_2(propertiesChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void targetChanged())
   DECL_CS_SIGNAL_2(targetChanged)
   DECL_CS_SIGNAL_1(Public, void propertyChanged())
   DECL_CS_SIGNAL_2(propertyChanged)
};

class QDeclarativeColorAnimation : public QDeclarativePropertyAnimation
{
   DECL_CS_OBJECT(QDeclarativeColorAnimation)
   Q_DECLARE_PRIVATE(QDeclarativePropertyAnimation)
   DECL_CS_PROPERTY_READ(from, from)
   DECL_CS_PROPERTY_WRITE(from, setFrom)
   DECL_CS_PROPERTY_READ(to, to)
   DECL_CS_PROPERTY_WRITE(to, setTo)

 public:
   QDeclarativeColorAnimation(QObject *parent = nullptr);
   virtual ~QDeclarativeColorAnimation();

   QColor from() const;
   void setFrom(const QColor &);

   QColor to() const;
   void setTo(const QColor &);
};

class QDeclarativeNumberAnimation : public QDeclarativePropertyAnimation
{
   DECL_CS_OBJECT(QDeclarativeNumberAnimation)
   Q_DECLARE_PRIVATE(QDeclarativePropertyAnimation)

   DECL_CS_PROPERTY_READ(from, from)
   DECL_CS_PROPERTY_WRITE(from, setFrom)
   DECL_CS_PROPERTY_READ(to, to)
   DECL_CS_PROPERTY_WRITE(to, setTo)

 public:
   QDeclarativeNumberAnimation(QObject *parent = nullptr);
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
   DECL_CS_OBJECT(QDeclarativeVector3dAnimation)
   Q_DECLARE_PRIVATE(QDeclarativePropertyAnimation)

   DECL_CS_PROPERTY_READ(from, from)
   DECL_CS_PROPERTY_WRITE(from, setFrom)
   DECL_CS_PROPERTY_READ(to, to)
   DECL_CS_PROPERTY_WRITE(to, setTo)

 public:
   QDeclarativeVector3dAnimation(QObject *parent = nullptr);
   virtual ~QDeclarativeVector3dAnimation();

   QVector3D from() const;
   void setFrom(QVector3D);

   QVector3D to() const;
   void setTo(QVector3D);
};

class QDeclarativeRotationAnimationPrivate;
class QDeclarativeRotationAnimation : public QDeclarativePropertyAnimation
{
   DECL_CS_OBJECT(QDeclarativeRotationAnimation)
   Q_DECLARE_PRIVATE(QDeclarativeRotationAnimation)
   CS_ENUM(RotationDirection)

   DECL_CS_PROPERTY_READ(from, from)
   DECL_CS_PROPERTY_WRITE(from, setFrom)
   DECL_CS_PROPERTY_READ(to, to)
   DECL_CS_PROPERTY_WRITE(to, setTo)
   DECL_CS_PROPERTY_READ(direction, direction)
   DECL_CS_PROPERTY_WRITE(direction, setDirection)
   DECL_CS_PROPERTY_NOTIFY(direction, directionChanged)

 public:
   QDeclarativeRotationAnimation(QObject *parent = nullptr);
   virtual ~QDeclarativeRotationAnimation();

   qreal from() const;
   void setFrom(qreal);

   qreal to() const;
   void setTo(qreal);

   enum RotationDirection { Numerical, Shortest, Clockwise, Counterclockwise };
   RotationDirection direction() const;
   void setDirection(RotationDirection direction);

 public:
   DECL_CS_SIGNAL_1(Public, void directionChanged())
   DECL_CS_SIGNAL_2(directionChanged)
};

class QDeclarativeAnimationGroupPrivate;
class QDeclarativeAnimationGroup : public QDeclarativeAbstractAnimation
{
   DECL_CS_OBJECT(QDeclarativeAnimationGroup)
   Q_DECLARE_PRIVATE(QDeclarativeAnimationGroup)

   DECL_CS_CLASSINFO("DefaultProperty", "animations")
   DECL_CS_PROPERTY_READ(animations, animations)

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
   DECL_CS_OBJECT(QDeclarativeSequentialAnimation)
   Q_DECLARE_PRIVATE(QDeclarativeAnimationGroup)

 public:
   QDeclarativeSequentialAnimation(QObject *parent = nullptr);
   virtual ~QDeclarativeSequentialAnimation();

 protected:
   virtual void transition(QDeclarativeStateActions &actions,
                           QDeclarativeProperties &modified,
                           TransitionDirection direction);
   virtual QAbstractAnimation *qtAnimation();
};

class QDeclarativeParallelAnimation : public QDeclarativeAnimationGroup
{
   DECL_CS_OBJECT(QDeclarativeParallelAnimation)
   Q_DECLARE_PRIVATE(QDeclarativeAnimationGroup)

 public:
   QDeclarativeParallelAnimation(QObject *parent = nullptr);
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
   DECL_CS_OBJECT(QDeclarativeParentAnimation)
   Q_DECLARE_PRIVATE(QDeclarativeParentAnimation)

   DECL_CS_PROPERTY_READ(*target, target)
   DECL_CS_PROPERTY_WRITE(*target, setTarget)
   DECL_CS_PROPERTY_NOTIFY(*target, targetChanged)
   DECL_CS_PROPERTY_READ(*newParent, newParent)
   DECL_CS_PROPERTY_WRITE(*newParent, setNewParent)
   DECL_CS_PROPERTY_NOTIFY(*newParent, newParentChanged)
   DECL_CS_PROPERTY_READ(*via, via)
   DECL_CS_PROPERTY_WRITE(*via, setVia)
   DECL_CS_PROPERTY_NOTIFY(*via, viaChanged)

 public:
   QDeclarativeParentAnimation(QObject *parent = nullptr);
   virtual ~QDeclarativeParentAnimation();

   QDeclarativeItem *target() const;
   void setTarget(QDeclarativeItem *);

   QDeclarativeItem *newParent() const;
   void setNewParent(QDeclarativeItem *);

   QDeclarativeItem *via() const;
   void setVia(QDeclarativeItem *);

 public:
   DECL_CS_SIGNAL_1(Public, void targetChanged())
   DECL_CS_SIGNAL_2(targetChanged)
   DECL_CS_SIGNAL_1(Public, void newParentChanged())
   DECL_CS_SIGNAL_2(newParentChanged)
   DECL_CS_SIGNAL_1(Public, void viaChanged())
   DECL_CS_SIGNAL_2(viaChanged)

 protected:
   virtual void transition(QDeclarativeStateActions &actions,
                           QDeclarativeProperties &modified,
                           TransitionDirection direction);
   virtual QAbstractAnimation *qtAnimation();
};

class QDeclarativeAnchorAnimationPrivate;
class QDeclarativeAnchorAnimation : public QDeclarativeAbstractAnimation
{
   DECL_CS_OBJECT(QDeclarativeAnchorAnimation)
   Q_DECLARE_PRIVATE(QDeclarativeAnchorAnimation)
   DECL_CS_PROPERTY_READ(targets, targets)
   DECL_CS_PROPERTY_READ(duration, duration)
   DECL_CS_PROPERTY_WRITE(duration, setDuration)
   DECL_CS_PROPERTY_NOTIFY(duration, durationChanged)
   DECL_CS_PROPERTY_READ(easing, easing)
   DECL_CS_PROPERTY_WRITE(easing, setEasing)
   DECL_CS_PROPERTY_NOTIFY(easing, easingChanged)

 public:
   QDeclarativeAnchorAnimation(QObject *parent = nullptr);
   virtual ~QDeclarativeAnchorAnimation();

   QDeclarativeListProperty<QDeclarativeItem> targets();

   int duration() const;
   void setDuration(int);

   QEasingCurve easing() const;
   void setEasing(const QEasingCurve &);


   DECL_CS_SIGNAL_1(Public, void durationChanged(int un_named_arg1))
   DECL_CS_SIGNAL_2(durationChanged, un_named_arg1)
   DECL_CS_SIGNAL_1(Public, void easingChanged(const QEasingCurve &un_named_arg1))
   DECL_CS_SIGNAL_2(easingChanged, un_named_arg1)

 protected:
   virtual void transition(QDeclarativeStateActions &actions, QDeclarativeProperties &modified,
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

#endif // QDECLARATIVEANIMATION_H
