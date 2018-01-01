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

#ifndef DBUSDISPATCHER_H
#define DBUSDISPATCHER_H

#include <QObject>
#include <QVariant>

namespace Maemo {

class DBusDispatcherPrivate;
class DBusDispatcher : public QObject
{
    Q_OBJECT

public:
    DBusDispatcher(const QString& service,
                   const QString& path,
                   const QString& interface,
                   QObject *parent = nullptr);
    DBusDispatcher(const QString& service,
                   const QString& path,
                   const QString& interface,
                   const QString& signalPath,
                   QObject *parent = nullptr);
    ~DBusDispatcher();

    QList<QVariant> call(const QString& method, 
                         const QVariant& arg1 = QVariant(),
                         const QVariant& arg2 = QVariant(),
                         const QVariant& arg3 = QVariant(),
                         const QVariant& arg4 = QVariant(),
                         const QVariant& arg5 = QVariant(),
                         const QVariant& arg6 = QVariant(),
                         const QVariant& arg7 = QVariant(),
                         const QVariant& arg8 = QVariant());
    bool callAsynchronous(const QString& method, 
                          const QVariant& arg1 = QVariant(),
                          const QVariant& arg2 = QVariant(),
                          const QVariant& arg3 = QVariant(),
                          const QVariant& arg4 = QVariant(),
                          const QVariant& arg5 = QVariant(),
                          const QVariant& arg6 = QVariant(),
                          const QVariant& arg7 = QVariant(),
                          const QVariant& arg8 = QVariant());
    void emitSignalReceived(const QString& interface, 
                            const QString& signal,
                            const QList<QVariant>& args);
    void emitCallReply(const QString& method,
                       const QList<QVariant>& args,
                       const QString& error = "");
    void synchronousDispatch(int timeout_ms);

Q_SIGNALS:
    void signalReceived(const QString& interface, 
                        const QString& signal,
                        const QList<QVariant>& args);
    void callReply(const QString& method,
                   const QList<QVariant>& args,
                   const QString& error);

protected:
    void setupDBus();

private:
    DBusDispatcherPrivate *d_ptr;
};

}  // Maemo namespace

#endif
