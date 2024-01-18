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

#ifndef QINPUTMETHOD_P_H
#define QINPUTMETHOD_P_H

#include <qinputmethod.h>
#include <QWeakPointer>
#include <QTransform>
#include <qplatform_inputcontext.h>
#include <qplatform_integration.h>

#include <qapplication_p.h>

class QInputMethodPrivate
{
   Q_DECLARE_PUBLIC(QInputMethod)

 public:
   inline QInputMethodPrivate()
      : testContext(nullptr)
   {
   }

   QPlatformInputContext *platformInputContext() const {
      if (testContext == nullptr) {
         return QGuiApplicationPrivate::platformIntegration()->inputContext();
      } else {
         return testContext;
      }
   }

   static inline QInputMethodPrivate *get(QInputMethod *inputMethod) {
      return inputMethod->d_func();
   }

   void _q_connectFocusObject();
   void _q_checkFocusObject(QObject *object);
   bool objectAcceptsInputMethod(QObject *object);

   QTransform inputItemTransform;
   QRectF inputRectangle;
   QPlatformInputContext *testContext;

 protected:
   QInputMethod *q_ptr;

};

#endif
