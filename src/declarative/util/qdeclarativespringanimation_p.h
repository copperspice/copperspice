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

#ifndef QDECLARATIVESPRINGANIMATION_P_H
#define QDECLARATIVESPRINGANIMATION_P_H

#include <qdeclarative.h>
#include <qdeclarativeanimation_p.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QDeclarativeSpringAnimationPrivate;

class QDeclarativeSpringAnimation : public QDeclarativeNumberAnimation
{
   DECL_CS_OBJECT(QDeclarativeSpringAnimation)
   Q_DECLARE_PRIVATE(QDeclarativeSpringAnimation)

   CS_INTERFACES(QDeclarativePropertyValueSource)

   DECL_CS_PROPERTY_READ(velocity, velocity)
   DECL_CS_PROPERTY_WRITE(velocity, setVelocity)
   DECL_CS_PROPERTY_READ(spring, spring)
   DECL_CS_PROPERTY_WRITE(spring, setSpring)
   DECL_CS_PROPERTY_READ(damping, damping)
   DECL_CS_PROPERTY_WRITE(damping, setDamping)
   DECL_CS_PROPERTY_READ(epsilon, epsilon)
   DECL_CS_PROPERTY_WRITE(epsilon, setEpsilon)
   DECL_CS_PROPERTY_READ(modulus, modulus)
   DECL_CS_PROPERTY_WRITE(modulus, setModulus)
   DECL_CS_PROPERTY_NOTIFY(modulus, modulusChanged)
   DECL_CS_PROPERTY_READ(mass, mass)
   DECL_CS_PROPERTY_WRITE(mass, setMass)
   DECL_CS_PROPERTY_NOTIFY(mass, massChanged)

 public:
   QDeclarativeSpringAnimation(QObject *parent = nullptr);
   ~QDeclarativeSpringAnimation();

   qreal velocity() const;
   void setVelocity(qreal velocity);

   qreal spring() const;
   void setSpring(qreal spring);

   qreal damping() const;
   void setDamping(qreal damping);

   qreal epsilon() const;
   void setEpsilon(qreal epsilon);

   qreal mass() const;
   void setMass(qreal modulus);

   qreal modulus() const;
   void setModulus(qreal modulus);

   virtual void transition(QDeclarativeStateActions &actions, QDeclarativeProperties &modified,
                           TransitionDirection direction);

   DECL_CS_SIGNAL_1(Public, void modulusChanged())
   DECL_CS_SIGNAL_2(modulusChanged)
   DECL_CS_SIGNAL_1(Public, void massChanged())
   DECL_CS_SIGNAL_2(massChanged)
   DECL_CS_SIGNAL_1(Public, void syncChanged())
   DECL_CS_SIGNAL_2(syncChanged)

 protected:
   virtual QAbstractAnimation *qtAnimation();

};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeSpringAnimation)

#endif // QDECLARATIVESPRINGANIMATION_H
