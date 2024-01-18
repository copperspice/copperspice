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

#ifndef QDECLARATIVEANIMATION_P_P_H
#define QDECLARATIVEANIMATION_P_P_H

#include <qdeclarativeanimation_p.h>
#include <qdeclarativenullablevalue_p_p.h>
#include <qdeclarativetimeline_p_p.h>

#include <qdeclarative.h>
#include <qdeclarativeitem.h>
#include <qdeclarativecontext.h>

#include <QtCore/QPauseAnimation>
#include <QtCore/QVariantAnimation>
#include <QtCore/QAnimationGroup>
#include <QtGui/QColor>
#include <QDebug>

#include <qvariantanimation_p.h>

QT_BEGIN_NAMESPACE

//interface for classes that provide animation actions for QActionAnimation
class QAbstractAnimationAction
{
 public:
   virtual ~QAbstractAnimationAction() {}
   virtual void doAction() = 0;
};

//templated animation action
//allows us to specify an action that calls a function of a class.
//(so that class doesn't have to inherit QDeclarativeAbstractAnimationAction)
template<class T, void (T::*method)()>
class QAnimationActionProxy : public QAbstractAnimationAction
{
 public:
   QAnimationActionProxy(T *p) : m_p(p) {}
   virtual void doAction() {
      (m_p->*method)();
   }

 private:
   T *m_p;
};

//performs an action of type QAbstractAnimationAction
class QActionAnimation : public QAbstractAnimation
{
   DECL_CS_OBJECT(QActionAnimation)
 public:
   QActionAnimation(QObject *parent = nullptr) : QAbstractAnimation(parent), animAction(0), policy(KeepWhenStopped) {}
   QActionAnimation(QAbstractAnimationAction *action, QObject *parent = nullptr)
      : QAbstractAnimation(parent), animAction(action), policy(KeepWhenStopped) {}
   ~QActionAnimation() {
      if (policy == DeleteWhenStopped) {
         delete animAction;
         animAction = 0;
      }
   }
   virtual int duration() const {
      return 0;
   }
   void setAnimAction(QAbstractAnimationAction *action, DeletionPolicy p) {
      if (state() == Running) {
         stop();
      }
      if (policy == DeleteWhenStopped) {
         delete animAction;
      }
      animAction = action;
      policy = p;
   }
 protected:
   virtual void updateCurrentTime(int) {}

   virtual void updateState(State newState, State /*oldState*/) {
      if (newState == Running) {
         if (animAction) {
            animAction->doAction();
            if (state() == Stopped && policy == DeleteWhenStopped) {
               delete animAction;
               animAction = 0;
            }
         }
      }
   }

 private:
   QAbstractAnimationAction *animAction;
   DeletionPolicy policy;
};

class QDeclarativeBulkValueUpdater
{
 public:
   virtual ~QDeclarativeBulkValueUpdater() {}
   virtual void setValue(qreal value) = 0;
};

//animates QDeclarativeBulkValueUpdater (assumes start and end values will be reals or compatible)
class QDeclarativeBulkValueAnimator : public QVariantAnimation
{
   DECL_CS_OBJECT(QDeclarativeBulkValueAnimator)
 public:
   QDeclarativeBulkValueAnimator(QObject *parent = nullptr) : QVariantAnimation(parent), animValue(0), fromSourced(0),
      policy(KeepWhenStopped) {}
   ~QDeclarativeBulkValueAnimator() {
      if (policy == DeleteWhenStopped) {
         delete animValue;
         animValue = 0;
      }
   }
   void setAnimValue(QDeclarativeBulkValueUpdater *value, DeletionPolicy p) {
      if (state() == Running) {
         stop();
      }
      if (policy == DeleteWhenStopped) {
         delete animValue;
      }
      animValue = value;
      policy = p;
   }
   void setFromSourcedValue(bool *value) {
      fromSourced = value;
   }
 protected:
   virtual void updateCurrentValue(const QVariant &value) {
      if (state() == QAbstractAnimation::Stopped) {
         return;
      }

      if (animValue) {
         animValue->setValue(value.toReal());
      }
   }
   virtual void updateState(State newState, State oldState) {
      QVariantAnimation::updateState(newState, oldState);
      if (newState == Running) {
         //check for new from every loop
         if (fromSourced) {
            *fromSourced = false;
         }
      }
   }

 private:
   QDeclarativeBulkValueUpdater *animValue;
   bool *fromSourced;
   DeletionPolicy policy;
};

//an animation that just gives a tick
template<class T, void (T::*method)(int)>
class QTickAnimationProxy : public QAbstractAnimation
{
   //Q_OBJECT //doesn't work with templating

 public:
   QTickAnimationProxy(T *p, QObject *parent = nullptr) : QAbstractAnimation(parent), m_p(p) {}
   virtual int duration() const {
      return -1;
   }

 protected:
   virtual void updateCurrentTime(int msec) {
      (m_p->*method)(msec);
   }

 private:
   T *m_p;
};

class QDeclarativeAbstractAnimationPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeAbstractAnimation)

 public:
   QDeclarativeAbstractAnimationPrivate()
      : running(false), paused(false), alwaysRunToEnd(false),
        connectedTimeLine(false), componentComplete(true),
        avoidPropertyValueSourceStart(false), disableUserControl(false),
        registered(false), loopCount(1), group(0) {}

   bool running: 1;
   bool paused: 1;
   bool alwaysRunToEnd: 1;
   bool connectedTimeLine: 1;
   bool componentComplete: 1;
   bool avoidPropertyValueSourceStart: 1;
   bool disableUserControl: 1;
   bool registered: 1;

   int loopCount;

   void commence();

   QDeclarativeProperty defaultProperty;

   QDeclarativeAnimationGroup *group;

   static QDeclarativeProperty createProperty(QObject *obj, const QString &str, QObject *infoObj);
};

class QDeclarativePauseAnimationPrivate : public QDeclarativeAbstractAnimationPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativePauseAnimation)
 public:
   QDeclarativePauseAnimationPrivate()
      : QDeclarativeAbstractAnimationPrivate(), pa(0) {}

   void init();

   QPauseAnimation *pa;
};

class QDeclarativeScriptActionPrivate : public QDeclarativeAbstractAnimationPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeScriptAction)
 public:
   QDeclarativeScriptActionPrivate();

   void init();

   QDeclarativeScriptString script;
   QString name;
   QDeclarativeScriptString runScriptScript;
   bool hasRunScriptScript;
   bool reversing;

   void execute();

   QAnimationActionProxy<QDeclarativeScriptActionPrivate,
                         &QDeclarativeScriptActionPrivate::execute> proxy;
   QActionAnimation *rsa;
};

class QDeclarativePropertyActionPrivate : public QDeclarativeAbstractAnimationPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativePropertyAction)
 public:
   QDeclarativePropertyActionPrivate()
      : QDeclarativeAbstractAnimationPrivate(), target(0), spa(0) {}

   void init();

   QObject *target;
   QString propertyName;
   QString properties;
   QList<QObject *> targets;
   QList<QObject *> exclude;

   QDeclarativeNullableValue<QVariant> value;

   QActionAnimation *spa;
};

class QDeclarativeAnimationGroupPrivate : public QDeclarativeAbstractAnimationPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeAnimationGroup)
 public:
   QDeclarativeAnimationGroupPrivate()
      : QDeclarativeAbstractAnimationPrivate(), ag(0) {}

   static void append_animation(QDeclarativeListProperty<QDeclarativeAbstractAnimation> *list,
                                QDeclarativeAbstractAnimation *role);
   static void clear_animation(QDeclarativeListProperty<QDeclarativeAbstractAnimation> *list);
   QList<QDeclarativeAbstractAnimation *> animations;
   QAnimationGroup *ag;
};

class QDeclarativePropertyAnimationPrivate : public QDeclarativeAbstractAnimationPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativePropertyAnimation)
 public:
   QDeclarativePropertyAnimationPrivate()
      : QDeclarativeAbstractAnimationPrivate(), target(0), fromSourced(false), fromIsDefined(false), toIsDefined(false),
        rangeIsSet(false), defaultToInterpolatorType(0), interpolatorType(0), interpolator(0), va(0), actions(0) {}

   void init();

   QVariant from;
   QVariant to;

   QObject *target;
   QString propertyName;
   QString properties;
   QList<QObject *> targets;
   QList<QObject *> exclude;
   QString defaultProperties;

   bool fromSourced;
   bool fromIsDefined: 1;
   bool toIsDefined: 1;
   bool rangeIsSet: 1;
   bool defaultToInterpolatorType: 1;
   int interpolatorType;
   QVariantAnimation::Interpolator interpolator;

   QDeclarativeBulkValueAnimator *va;

   // for animations that don't use the QDeclarativeBulkValueAnimator
   QDeclarativeStateActions *actions;

   static QVariant interpolateVariant(const QVariant &from, const QVariant &to, qreal progress);
   static void convertVariant(QVariant &variant, int type);
};

class QDeclarativeRotationAnimationPrivate : public QDeclarativePropertyAnimationPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeRotationAnimation)
 public:
   QDeclarativeRotationAnimationPrivate() : direction(QDeclarativeRotationAnimation::Numerical) {}

   QDeclarativeRotationAnimation::RotationDirection direction;
};

class QDeclarativeParentAnimationPrivate : public QDeclarativeAnimationGroupPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeParentAnimation)
 public:
   QDeclarativeParentAnimationPrivate()
      : QDeclarativeAnimationGroupPrivate(), target(0), newParent(0),
        via(0), topLevelGroup(0), startAction(0), endAction(0) {}

   QDeclarativeItem *target;
   QDeclarativeItem *newParent;
   QDeclarativeItem *via;

   QSequentialAnimationGroup *topLevelGroup;
   QActionAnimation *startAction;
   QActionAnimation *endAction;

   QPointF computeTransformOrigin(QDeclarativeItem::TransformOrigin origin, qreal width, qreal height) const;
};

class QDeclarativeAnchorAnimationPrivate : public QDeclarativeAbstractAnimationPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeAnchorAnimation)
 public:
   QDeclarativeAnchorAnimationPrivate() : rangeIsSet(false), va(0),
      interpolator(QVariantAnimationPrivate::getInterpolator(QMetaType::QReal)) {}

   bool rangeIsSet;
   QDeclarativeBulkValueAnimator *va;
   QVariantAnimation::Interpolator interpolator;
   QList<QDeclarativeItem *> targets;
};

class QDeclarativeAnimationPropertyUpdater : public QDeclarativeBulkValueUpdater
{
 public:
   QDeclarativeStateActions actions;
   int interpolatorType;       //for Number/ColorAnimation
   int prevInterpolatorType;   //for generic
   QVariantAnimation::Interpolator interpolator;
   bool reverse;
   bool fromSourced;
   bool fromDefined;
   bool *wasDeleted;
   QDeclarativeAnimationPropertyUpdater() : prevInterpolatorType(0), wasDeleted(0) {}
   ~QDeclarativeAnimationPropertyUpdater() {
      if (wasDeleted) {
         *wasDeleted = true;
      }
   }
   void setValue(qreal v);
};

QT_END_NAMESPACE

#endif // QDECLARATIVEANIMATION_P_H
