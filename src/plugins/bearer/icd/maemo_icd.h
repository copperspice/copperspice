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

#ifndef MAEMO_ICD_H
#define MAEMO_ICD_H

#include <QObject>
#include <QStringList>
#include <QByteArray>
#include <QMetaType>
#include <QtDBus>
#include <QDBusArgument>

#include <glib.h>
#include <icd/dbus_api.h>
#include <icd/osso-ic.h>
#include <icd/osso-ic-dbus.h>
#include <icd/network_api_defines.h>

#define ICD_LONG_SCAN_TIMEOUT (30*1000)  /* 30sec */
#define ICD_SHORT_SCAN_TIMEOUT (10*1000)  /* 10sec */
#define ICD_SHORT_CONNECT_TIMEOUT (10*1000) /* 10sec */
#define ICD_LONG_CONNECT_TIMEOUT (150*1000) /* 2.5min */

namespace Maemo {

struct CommonParams {
	QString service_type;
	uint service_attrs;
	QString service_id;
	QString network_type;
	uint network_attrs;
	QByteArray network_id;
};

struct IcdScanResult {
	uint status; // see #icd_scan_status
	uint timestamp; // when last seen
	QString service_name;
	uint service_priority; // within a service type
	QString network_name;
	uint network_priority;
	struct CommonParams scan;
	uint signal_strength; // quality, 0 (none) - 10 (good)
	QString station_id; // e.g. MAC address or similar id
	uint signal_dB; // use signal strength above unless you know what you are doing

	IcdScanResult() {
		status = timestamp = scan.service_attrs = service_priority =
			scan.network_attrs = network_priority = signal_strength =
			signal_dB = 0;
	}
};

struct IcdStateResult {
	struct CommonParams params;
	QString error;
	uint state;
};

struct IcdStatisticsResult {
	struct CommonParams params;
	uint time_active;  // in seconds
	enum icd_nw_levels signal_strength; // see network_api_defines.h in icd2-dev package
	uint bytes_sent;
	uint bytes_received;
};

struct IcdIPInformation {
	QString address;
	QString netmask;
	QString default_gateway;
	QString dns1;
	QString dns2;
	QString dns3;
};

struct IcdAddressInfoResult {
	struct CommonParams params;
	QList<IcdIPInformation> ip_info;
};

enum IcdDbusInterfaceVer {
	IcdOldDbusInterface = 0,  // use the old OSSO-IC interface
	IcdNewDbusInterface       // use the new Icd2 interface (default)
};


class IcdPrivate;
class Icd : public QObject
{
    Q_OBJECT

public:
    Icd(QObject *parent = nullptr);
    Icd(unsigned int timeout, QObject *parent = nullptr);
    Icd(unsigned int timeout, IcdDbusInterfaceVer ver, QObject *parent = nullptr);
    ~Icd();

    /* Icd2 dbus API functions */
    QStringList scan(icd_scan_request_flags flags,
		     QStringList &network_types,
		     QList<IcdScanResult>& scan_results,
		     QString& error);

    uint state(QString& service_type, uint service_attrs,
	       QString& service_id, QString& network_type,
	       uint network_attrs, QByteArray& network_id,
	       IcdStateResult &state_result);

    uint addrinfo(QString& service_type, uint service_attrs,
		  QString& service_id, QString& network_type,
		  uint network_attrs, QByteArray& network_id,
		  IcdAddressInfoResult& addr_result);

    uint state(QList<IcdStateResult>& state_results);
    uint statistics(QList<IcdStatisticsResult>& stats_results);
    uint addrinfo(QList<IcdAddressInfoResult>& addr_results);

private Q_SLOTS:
    void icdSignalReceived(const QString& interface, 
                        const QString& signal,
                        const QList<QVariant>& args);
    void icdCallReply(const QString& method, 
                   const QList<QVariant>& args,
                   const QString& error);

private:
    IcdPrivate *d;
    friend class IcdPrivate;
};

}  // Maemo namespace

#endif
