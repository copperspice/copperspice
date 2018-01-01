/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPROPERTYANIMATION_P_H
#define QPROPERTYANIMATION_P_H

#include <qpropertyanimation.h>
#include <qvariantanimation_p.h>

#ifndef QT_NO_ANIMATION

QT_BEGIN_NAMESPACE

class QPropertyAnimationPrivate : public QVariantAnimationPrivate
{
   Q_DECLARE_PUBLIC(QPropertyAnimation)

 public:
   QPropertyAnimationPrivate()
      : targetValue(0), propertyType(0), propertyIndex(-1) {
   }

   QWeakPointer<QObject> target;
   //we use targetValue to be able to unregister the target from the global hash
   QObject *targetValue;

   //for the QProperty
   int propertyType;
   int propertyIndex;

   QByteArray propertyName;
   void updateProperty(const QVariant &);
   void updateMetaProperty();
};

QT_END_NAMESPACE

#endif //QT_NO_ANIMATION

#endif //QPROPERTYANIMATION_P_H
