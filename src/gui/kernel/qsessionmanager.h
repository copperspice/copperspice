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

#ifndef QSESSIONMANAGER_H
#define QSESSIONMANAGER_H

#include <qobject.h>
#include <qscopedpointer.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qwindowdefs.h>

#ifndef QT_NO_SESSIONMANAGER

class QApplication;
class QSessionManagerPrivate;

class Q_GUI_EXPORT QSessionManager : public QObject
{
   GUI_CS_OBJECT(QSessionManager)
   Q_DECLARE_PRIVATE(QSessionManager)

   QSessionManager(QApplication *app, QString &id, QString &key);
   ~QSessionManager();

 public:
   QString sessionId() const;
   QString sessionKey() const;

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

   void setRestartHint(RestartHint hint);
   RestartHint restartHint() const;

   void setRestartCommand(const QStringList &list);
   QStringList restartCommand() const;
   void setDiscardCommand(const QStringList &list);
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
};

#endif // QT_NO_SESSIONMANAGER

#endif // QSESSIONMANAGER_H
