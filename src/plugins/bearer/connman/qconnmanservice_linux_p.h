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

#ifndef QCONNMANSERVICE_H
#define QCONNMANSERVICE_H

#include <QtDBus/QtDBus>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusArgument>

#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusContext>
#include <QMap>

#ifndef QT_NO_BEARERMANAGEMENT
#ifndef QT_NO_DBUS

#ifndef __CONNMAN_DBUS_H

#define	CONNMAN_SERVICE     "net.connman"
#define	CONNMAN_PATH        "/net/connman"

#define CONNMAN_DEBUG_INTERFACE		CONNMAN_SERVICE ".Debug"
#define CONNMAN_ERROR_INTERFACE		CONNMAN_SERVICE ".Error"
#define CONNMAN_AGENT_INTERFACE		CONNMAN_SERVICE ".Agent"
#define CONNMAN_COUNTER_INTERFACE	CONNMAN_SERVICE ".Counter"

#define CONNMAN_MANAGER_INTERFACE	CONNMAN_SERVICE ".Manager"
#define CONNMAN_MANAGER_PATH		"/"

#define CONNMAN_TASK_INTERFACE		CONNMAN_SERVICE ".Task"
#define CONNMAN_PROFILE_INTERFACE	CONNMAN_SERVICE ".Profile"
#define CONNMAN_SERVICE_INTERFACE	CONNMAN_SERVICE ".Service"
#define CONNMAN_PROVIDER_INTERFACE	CONNMAN_SERVICE ".Provider"
#define CONNMAN_TECHNOLOGY_INTERFACE	CONNMAN_SERVICE ".Technology"
#endif

QT_BEGIN_NAMESPACE

struct ConnmanMap {
    QDBusObjectPath objectPath;
    QVariantMap propertyMap;
};

typedef QList< ConnmanMap > ConnmanMapList;

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(ConnmanMap));
Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(ConnmanMapList))

QT_BEGIN_NAMESPACE

QDBusArgument &operator<<(QDBusArgument &argument, const ConnmanMap &obj);
const QDBusArgument &operator>>(const QDBusArgument &argument, ConnmanMap &obj);


class QConnmanManagerInterface : public  QDBusAbstractInterface
{
    Q_OBJECT

public:

    QConnmanManagerInterface( QObject *parent = nullptr);
    ~QConnmanManagerInterface();

     QDBusObjectPath path() const;

    QVariantMap getProperties();
    bool setProperty(const QString &name, const QDBusVariant &value);
    QDBusObjectPath createProfile(const QString &name);
    bool removeProfile(QDBusObjectPath path);
    bool requestScan(const QString &type);
    bool enableTechnology(const QString &type);
    bool disableTechnology(const QString &type);
    QDBusObjectPath connectService(QVariantMap &map);
    void registerAgent(QDBusObjectPath &path);
    void unregisterAgent(QDBusObjectPath path);
    void registerCounter(const QString &path, quint32 interval);
    void unregisterCounter(const QString &path);

    QString requestSession(const QString &bearerName);
    void releaseSession();
    
      // properties
    QString getState();
    QStringList getAvailableTechnologies();
    QStringList getEnabledTechnologies();
    QStringList getConnectedTechnologies();
    QString getDefaultTechnology();
    bool getOfflineMode();
    QString getActiveProfile();
    QStringList getProfiles();
    QStringList  getTechnologies();
    QStringList getServices();
    QDBusObjectPath lookupService(const QString &);

    QString getPathForTechnology(const QString &tech);


Q_SIGNALS:
    void propertyChanged(const QString &, const QDBusVariant &value);
    void stateChanged(const QString &);
    void propertyChangedContext(const QString &,const QString &,const QDBusVariant &);

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);
    QVariant getProperty(const QString &);
};

class QConnmanProfileInterfacePrivate;
class QConnmanProfileInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:

    QConnmanProfileInterface(const QString &dbusPathName,QObject *parent = nullptr);
    ~QConnmanProfileInterface();

    QVariantMap getProperties();
// properties
    QString getName();
    bool isOfflineMode();
    QStringList getServices();

Q_SIGNALS:
    void propertyChanged(const QString &, const QDBusVariant &value);
private:
    QConnmanProfileInterfacePrivate *d;

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);
    QVariant getProperty(const QString &);
};

class QConnmanServiceInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:

    QConnmanServiceInterface(const QString &dbusPathName,QObject *parent = nullptr);
    ~QConnmanServiceInterface();

    QVariantMap getProperties();
      // clearProperty
    void connect();
    void disconnect();
    void remove();
      // void moveBefore(QDBusObjectPath &service);
      // void moveAfter(QDBusObjectPath &service);

// properties
    QString getState();
    QString getError();
    QString getName();
    QString getType();
    QString getMode();
    QString getSecurity();
    QString getPassphrase();
    bool isPassphraseRequired();
    quint8 getSignalStrength();
    bool isFavorite();
    bool isImmutable();
    bool isAutoConnect();
    bool isSetupRequired();
    QString getAPN();
    QString getMCC();
    QString getMNC();
    bool isRoaming();
    QStringList getNameservers();
    QStringList getDomains();
    QVariantMap getIPv4();
    QVariantMap getIPv4Configuration();
    QVariantMap getProxy();
    QVariantMap getEthernet();

    QString getMethod();
    QString getInterface();
    QString getMacAddress();
    quint16 getMtu();
    quint16 getSpeed();
    QString getDuplex();

    bool isOfflineMode();
    QStringList getServices();

Q_SIGNALS:
    void propertyChanged(const QString &, const QDBusVariant &value);
    void propertyChangedContext(const QString &,const QString &,const QDBusVariant &);

protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);
    QVariant getProperty(const QString &);
};

class QConnmanTechnologyInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:

    QConnmanTechnologyInterface(const QString &dbusPathName,QObject *parent = nullptr);
    ~QConnmanTechnologyInterface();

    QVariantMap getProperties();
// properties
    QString getState();
    QString getName();
    QString getType();

Q_SIGNALS:
    void propertyChanged(const QString &, const QDBusVariant &value);
    void propertyChangedContext(const QString &,const QString &,const QDBusVariant &);
protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);
    QVariant getProperty(const QString &);

};

class QConnmanAgentInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:

    QConnmanAgentInterface(const QString &dbusPathName,QObject *parent = nullptr);
    ~QConnmanAgentInterface();

    void release();
    void reportError(QDBusObjectPath &path, const QString &error);
//    dict requestInput(QDBusObjectPath &path, dict fields);
    void cancel();
protected:
    void connectNotify(const char *signal);
    void disconnectNotify(const char *signal);
};

class QConnmanCounterInterfacePrivate;
class QConnmanCounterInterface : public QDBusAbstractInterface
{
    Q_OBJECT

public:

    QConnmanCounterInterface(const QString &dbusPathName, QObject *parent = nullptr);
    ~QConnmanCounterInterface();

//    void release();
    QString getInterface();
    quint32 getReceivedByteCount();
    quint32 getTransmittedByteCount();
    quint64 getTimeOnline();

private:
    QConnmanCounterInterfacePrivate *d;
};

class QConnmanDBusHelper: public QObject, protected QDBusContext
 {
     Q_OBJECT
 public:
    QConnmanDBusHelper(QObject *parent = nullptr);
    ~QConnmanDBusHelper();

 public slots:
    void propertyChanged(const QString &, const QDBusVariant &);

Q_SIGNALS:
    void propertyChangedContext(const QString &,const QString &,const QDBusVariant &);
};

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif // QT_NO_BEARERMANAGEMENT

#endif //QCONNMANSERVICE_H
