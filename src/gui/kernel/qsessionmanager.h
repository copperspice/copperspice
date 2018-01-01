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

#ifndef QSESSIONMANAGER_H
#define QSESSIONMANAGER_H

#include <QtCore/qobject.h>
#include <QtGui/qwindowdefs.h>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QScopedPointer>

#ifndef QT_NO_SESSIONMANAGER

QT_BEGIN_NAMESPACE

class QSessionManagerPrivate;

class Q_GUI_EXPORT  QSessionManager : public QObject
{
   GUI_CS_OBJECT(QSessionManager)

   Q_DECLARE_PRIVATE(QSessionManager)
   QSessionManager(QApplication *app, QString &id, QString &key);
   ~QSessionManager();

 public:
   QString sessionId() const;
   QString sessionKey() const;

#if defined(Q_WS_X11) || defined(Q_OS_MAC)
   void *handle() const;
#endif

   bool allowsInteraction();
   bool allowsErrorInteraction();
   void release();

   void cancel();

   enum RestartHint {
      RestartIfRunning,
      RestartAnyway,
      RestartImmediately,
      RestartNever
   };
   void setRestartHint(RestartHint);
   RestartHint restartHint() const;

   void setRestartCommand(const QStringList &);
   QStringList restartCommand() const;
   void setDiscardCommand(const QStringList &);
   QStringList discardCommand() const;

   void setManagerProperty(const QString &name, const QString &value);
   void setManagerProperty(const QString &name, const QStringList &value);

   bool isPhase2() const;
   void requestPhase2();

 protected:
   QScopedPointer<QSessionManagerPrivate> d_ptr;

 private:
   friend class QApplication;
   friend class QApplicationPrivate;
   friend class QBaseApplication;
};

QT_END_NAMESPACE

#endif // QT_NO_SESSIONMANAGER

#endif // QSESSIONMANAGER_H
