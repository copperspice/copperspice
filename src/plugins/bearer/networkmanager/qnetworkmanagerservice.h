/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QNETWORKMANAGERSERVICE_H
#define QNETWORKMANAGERSERVICE_H

#include <QtDBus/QtDBus>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusError>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>

#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusContext>
#include <QMap>
#include <qnmdbushelper.h>

#ifndef QT_NO_DBUS

#ifndef NETWORK_MANAGER_H
typedef enum NMDeviceType
{
    DEVICE_TYPE_UNKNOWN = 0,
    DEVICE_TYPE_802_3_ETHERNET,
    DEVICE_TYPE_802_11_WIRELESS,
    DEVICE_TYPE_GSM,
    DEVICE_TYPE_CDMA
} NMDeviceType;

typedef enum
{
    NM_DEVICE_STATE_UNKNOWN = 0,
    NM_DEVICE_STATE_UNMANAGED,
    NM_DEVICE_STATE_UNAVAILABLE,
    NM_DEVICE_STATE_DISCONNECTED,
    NM_DEVICE_STATE_PREPARE,
    NM_DEVICE_STATE_CONFIG,
    NM_DEVICE_STATE_NEED_AUTH,
    NM_DEVICE_STATE_IP_CONFIG,
    NM_DEVICE_STATE_ACTIVATED,
    NM_DEVICE_STATE_FAILED
} NMDeviceState;

typedef enum
{
    NM_ACTIVE_CONNECTION_STATE_UNKNOWN = 0,
    NM_ACTIVE_CONNECTION_STATE_ACTIVATING,
    NM_ACTIVE_CONNECTION_STATE_ACTIVATED
} NMActiveConnectionState;

#define	NM_DBUS_SERVICE			"org.freedesktop.NetworkManager"

#define	NM_DBUS_PATH				        "/org/freedesktop/NetworkManager"
#define	NM_DBUS_INTERFACE			        "org.freedesktop.NetworkManager"
#define	NM_DBUS_INTERFACE_DEVICE	        NM_DBUS_INTERFACE ".Device"
#define NM_DBUS_INTERFACE_DEVICE_WIRED      NM_DBUS_INTERFACE_DEVICE ".Wired"
#define NM_DBUS_INTERFACE_DEVICE_WIRELESS   NM_DBUS_INTERFACE_DEVICE ".Wireless"
#define NM_DBUS_PATH_ACCESS_POINT           NM_DBUS_PATH "/AccessPoint"
#define NM_DBUS_INTERFACE_ACCESS_POINT      NM_DBUS_INTERFACE ".AccessPoint"

#define NM_DBUS_PATH_SETTINGS             "/org/freedesktop/NetworkManagerSettings"

#define NM_DBUS_IFACE_SETTINGS_CONNECTION "org.freedesktop.NetworkManagerSettings.Connection"
#define NM_DBUS_IFACE_SETTINGS            "org.freedesktop.NetworkManagerSettings"
#define NM_DBUS_INTERFACE_ACTIVE_CONNECTION NM_DBUS_INTERFACE ".Connection.Active"
#define NM_DBUS_INTERFACE_IP4_CONFIG        NM_DBUS_INTERFACE ".IP4Config"

#define NM_DBUS_SERVICE_USER_SETTINGS     "org.freedesktop.NetworkManagerUserSettings"
#define NM_DBUS_SERVICE_SYSTEM_SETTINGS   "org.freedesktop.NetworkManagerSystemSettings"

#define NM_802_11_AP_FLAGS_NONE				0x00000000
#define NM_802_11_AP_FLAGS_PRIVACY			0x00000001
#endif

QT_BEGIN_NAMESPACE

typedef QMap< QString, QMap<QString,QVariant> > QNmSettingsMap;
typedef QList<quint32> ServerThing;

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(QNmSettingsMap))
Q_DECLARE_METATYPE(QT_PREPEND_NAMESPACE(ServerThing))

QT_BEGIN_NAMESPACE

class QNetworkManagerInterfacePrivate;
class QNetworkManagerInterface : public QObject
{
    Q_OBJECT
    
public:
    
    QNetworkManagerInterface(QObject *parent = nullptr);
    ~QNetworkManagerInterface();
    
    QList <QDBusObjectPath> getDevices() const;
    void activateConnection(const QString &serviceName, QDBusObjectPath connection, QDBusObjectPath device, QDBusObjectPath specificObject);
    void deactivateConnection(QDBusObjectPath connectionPath) const;
    
    QDBusObjectPath path() const;
    QDBusInterface *connectionInterface() const;
    
    bool wirelessEnabled() const;
    bool wirelessHardwareEnabled() const;
    QList <QDBusObjectPath> activeConnections() const;
    quint32 state();
    bool setConnections();
    bool isValid();
    
Q_SIGNALS:
    void deviceAdded(QDBusObjectPath);
    void deviceRemoved(QDBusObjectPath);
    void propertiesChanged( const QString &, QMap<QString,QVariant>);
    void stateChanged(const QString&, quint32);
    void activationFinished(QDBusPendingCallWatcher*);

private Q_SLOTS:
private:
    QNetworkManagerInterfacePrivate *d;
    QNmDBusHelper *nmDBusHelper;
};

class QNetworkManagerInterfaceAccessPointPrivate;
class QNetworkManagerInterfaceAccessPoint : public QObject
{
    Q_OBJECT
    
public:
    
    enum DeviceState {
        Unknown = 0,
        Unmanaged,
        Unavailable,
        Disconnected,
        Prepare,
        Config,
        NeedAuthentication,
        IpConfig,
        Activated,
        Failed
    };
    
    enum ApFlag {
        ApNone = 0x0,
        Privacy = 0x1
    };
    
    Q_DECLARE_FLAGS(ApFlags, ApFlag);
    
    enum ApSecurityFlag {
        ApSecurityNone = 0x0,
        PairWep40 = 0x1,
        PairWep104 = 0x2,
        PairTkip = 0x4,
        PairCcmp = 0x8,
        GroupWep40 = 0x10,
        GroupWep104 = 0x20,
        GroupTkip = 0x40,
        GroupCcmp = 0x80,
        KeyPsk = 0x100,
        Key8021x = 0x200
    };
    
    Q_DECLARE_FLAGS(ApSecurityFlags, ApSecurityFlag);
    
    explicit QNetworkManagerInterfaceAccessPoint(const QString &dbusPathName, QObject *parent = nullptr);
    ~QNetworkManagerInterfaceAccessPoint();

    QDBusInterface *connectionInterface() const;

    quint32 flags() const;
    quint32 wpaFlags() const;
    quint32 rsnFlags() const;
    QString ssid() const;
    quint32 frequency() const;
    QString hwAddress() const;
    quint32 mode() const;
    quint32 maxBitrate() const;
    quint32 strength() const;
    bool setConnections();
    bool isValid();
    
Q_SIGNALS:
    void propertiesChanged(QMap <QString,QVariant>);
    void propertiesChanged( const QString &, QMap<QString,QVariant>);
private:
    QNetworkManagerInterfaceAccessPointPrivate *d;
    QNmDBusHelper *nmDBusHelper;

};

class QNetworkManagerInterfaceDevicePrivate;
class QNetworkManagerInterfaceDevice : public QObject
{
    Q_OBJECT
    
public:
    
    explicit QNetworkManagerInterfaceDevice(const QString &deviceObjectPath, QObject *parent = nullptr);
    ~QNetworkManagerInterfaceDevice();
    
    QString udi() const;
    QString networkInterface() const;
    QDBusInterface *connectionInterface() const;
    quint32 ip4Address() const;
    quint32 state() const;
    quint32 deviceType() const;
    
    QDBusObjectPath ip4config() const;
    bool setConnections();
    bool isValid();
    
Q_SIGNALS:
    void stateChanged(const QString &, quint32);

private:
    QNetworkManagerInterfaceDevicePrivate *d;
    QNmDBusHelper *nmDBusHelper;
};

class QNetworkManagerInterfaceDeviceWiredPrivate;
class QNetworkManagerInterfaceDeviceWired : public QObject
{
    Q_OBJECT
    
public:
    
    explicit QNetworkManagerInterfaceDeviceWired(const QString &ifaceDevicePath,
                                                 QObject *parent = nullptr);
    ~QNetworkManagerInterfaceDeviceWired();
    
    QDBusInterface  *connectionInterface() const;
    QString hwAddress() const;
    quint32 speed() const;
    bool carrier() const;
    bool setConnections();
    bool isValid();
    
Q_SIGNALS:
    void propertiesChanged( const QString &, QMap<QString,QVariant>);
private:
    QNetworkManagerInterfaceDeviceWiredPrivate *d;
    QNmDBusHelper *nmDBusHelper;
};

class QNetworkManagerInterfaceDeviceWirelessPrivate;
class QNetworkManagerInterfaceDeviceWireless : public QObject
{
    Q_OBJECT
    
public:
    
    enum DeviceCapability {
        None = 0x0,
        Wep40 = 0x1,
        Wep104 = 0x2,
        Tkip = 0x4,
        Ccmp = 0x8,
        Wpa = 0x10,
        Rsn = 0x20
       };
    
    explicit QNetworkManagerInterfaceDeviceWireless(const QString &ifaceDevicePath,
                                                    QObject *parent = nullptr);
    ~QNetworkManagerInterfaceDeviceWireless();
    
    QDBusObjectPath path() const;
    QList <QDBusObjectPath> getAccessPoints();
    QDBusInterface *connectionInterface() const;

    QString hwAddress() const;
    quint32 mode() const;
    quint32 bitrate() const;
    QDBusObjectPath activeAccessPoint() const;
    quint32 wirelessCapabilities() const;
    bool setConnections();
    bool isValid();
    
Q_SIGNALS:
    void propertiesChanged( const QString &, QMap<QString,QVariant>);
    void accessPointAdded(const QString &,QDBusObjectPath);
    void accessPointRemoved(const QString &,QDBusObjectPath);
private:
    QNetworkManagerInterfaceDeviceWirelessPrivate *d;
    QNmDBusHelper *nmDBusHelper;
};

class QNetworkManagerSettingsPrivate;
class QNetworkManagerSettings : public QObject
{
    Q_OBJECT
    
public:
    
    explicit QNetworkManagerSettings(const QString &settingsService, QObject *parent = nullptr);
    ~QNetworkManagerSettings();
    
    QDBusInterface  *connectionInterface() const;
    QList <QDBusObjectPath> listConnections();
    bool setConnections();
    bool isValid();

Q_SIGNALS:
    void newConnection(QDBusObjectPath);
private:
    QNetworkManagerSettingsPrivate *d;
};

class QNetworkManagerSettingsConnectionPrivate;
class QNetworkManagerSettingsConnection : public QObject
{
    Q_OBJECT
    
public:
    
    QNetworkManagerSettingsConnection(const QString &settingsService, const QString &connectionObjectPath, QObject *parent = nullptr);
    ~QNetworkManagerSettingsConnection();
    
    QDBusInterface  *connectionInterface() const;
    QNmSettingsMap getSettings();
    bool setConnections();
    NMDeviceType getType();
    bool isAutoConnect();
    quint64 getTimestamp();
    QString getId();
    QString getUuid();
    QString getSsid();
    QString getMacAddress();
    QStringList getSeenBssids();
    bool isValid();

Q_SIGNALS:
    
    void updated(const QNmSettingsMap &settings);
    void removed(const QString &path);

private:
    QNmDBusHelper *nmDBusHelper;
    QNetworkManagerSettingsConnectionPrivate *d;
};

class QNetworkManagerConnectionActivePrivate;
class QNetworkManagerConnectionActive : public QObject
{
    Q_OBJECT
    
public:
    
    enum ActiveConnectionState {
        Unknown = 0,
        Activating = 1,
        Activated = 2
       };
    
    explicit QNetworkManagerConnectionActive(const QString &dbusPathName, QObject *parent = nullptr);
    ~ QNetworkManagerConnectionActive();
    
    QDBusInterface  *connectionInterface() const;
    QString serviceName() const;
    QDBusObjectPath connection() const;
    QDBusObjectPath specificObject() const;
    QList<QDBusObjectPath> devices() const;
    quint32 state() const;
    bool defaultRoute() const;
    bool setConnections();
    bool isValid();

    
Q_SIGNALS:
    void propertiesChanged(QList<QDBusObjectPath>);
    void propertiesChanged( const QString &, QMap<QString,QVariant>);
private:
    QNetworkManagerConnectionActivePrivate *d;
    QNmDBusHelper *nmDBusHelper;
};

class QNetworkManagerIp4ConfigPrivate;
class QNetworkManagerIp4Config : public QObject
{
    Q_OBJECT
    
public:
    explicit QNetworkManagerIp4Config(const QString &dbusPathName, QObject *parent = nullptr);
	~QNetworkManagerIp4Config();

    QStringList domains() const;
    bool isValid();

 private:
	QNetworkManagerIp4ConfigPrivate *d;    
};

QT_END_NAMESPACE

#endif // QT_NO_DBUS
#endif //QNETWORKMANAGERSERVICE_H
