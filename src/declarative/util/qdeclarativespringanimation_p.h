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

   CS_PROPERTY_READ(velocity, velocity)
   CS_PROPERTY_WRITE(velocity, setVelocity)
   CS_PROPERTY_READ(spring, spring)
   CS_PROPERTY_WRITE(spring, setSpring)
   CS_PROPERTY_READ(damping, damping)
   CS_PROPERTY_WRITE(damping, setDamping)
   CS_PROPERTY_READ(epsilon, epsilon)
   CS_PROPERTY_WRITE(epsilon, setEpsilon)
   CS_PROPERTY_READ(modulus, modulus)
   CS_PROPERTY_WRITE(modulus, setModulus)
   CS_PROPERTY_NOTIFY(modulus, modulusChanged)
   CS_PROPERTY_READ(mass, mass)
   CS_PROPERTY_WRITE(mass, setMass)
   CS_PROPERTY_NOTIFY(mass, massChanged)

 public:
   QDeclarativeSpringAnimation(QObject *parent = 0);
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

   CS_SIGNAL_1(Public, void modulusChanged())
   CS_SIGNAL_2(modulusChanged)
   CS_SIGNAL_1(Public, void massChanged())
   CS_SIGNAL_2(massChanged)
   CS_SIGNAL_1(Public, void syncChanged())
   CS_SIGNAL_2(syncChanged)

 protected:
   virtual QAbstractAnimation *qtAnimation();

};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeSpringAnimation)

#endif // QDECLARATIVESPRINGANIMATION_H
