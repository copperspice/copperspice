/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QPLATFORMEVENTLOOPINTEGRATION_QPA_H
#define QPLATFORMEVENTLOOPINTEGRATION_QPA_H

#include <QtCore/qglobal.h>
#include <QtCore/QScopedPointer>

QT_BEGIN_NAMESPACE

class QPlatformEventLoopIntegrationPrivate;

class Q_GUI_EXPORT QPlatformEventLoopIntegration
{
   Q_DECLARE_PRIVATE(QPlatformEventLoopIntegration);

 public:
   QPlatformEventLoopIntegration();
   virtual ~QPlatformEventLoopIntegration();

   virtual void startEventLoop() = 0;
   virtual void quitEventLoop() = 0;
   virtual void qtNeedsToProcessEvents() = 0;

   qint64 nextTimerEvent() const;
   void setNextTimerEvent(qint64 nextTimerEvent);

   static void processEvents();

 protected:
   QScopedPointer<QPlatformEventLoopIntegrationPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QPlatformEventLoopIntegration);
   friend class QEventDispatcherQPA;
};

QT_END_NAMESPACE

#endif // QPLATFORMEVENTLOOPINTEGRATION_QPA_H
