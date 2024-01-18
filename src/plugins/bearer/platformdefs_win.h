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

#ifndef QPLATFORMDEFS_WIN_H
#define QPLATFORMDEFS_WIN_H

#include <winsock2.h>
#include <mswsock.h>
#undef interface
#include <winioctl.h>

QT_BEGIN_NAMESPACE

#ifndef NS_NLA

#define NS_NLA 15

#ifndef NLA_NAMESPACE_GUID
enum NLA_BLOB_DATA_TYPE {
    NLA_RAW_DATA = 0,
    NLA_INTERFACE = 1,
    NLA_802_1X_LOCATION = 2,
    NLA_CONNECTIVITY = 3,
    NLA_ICS = 4
};

enum NLA_CONNECTIVITY_TYPE {
    NLA_NETWORK_AD_HOC = 0,
    NLA_NETWORK_MANAGED = 1,
    NLA_NETWORK_UNMANAGED = 2,
    NLA_NETWORK_UNKNOWN = 3
};

enum NLA_INTERNET {
    NLA_INTERNET_UNKNOWN = 0,
    NLA_INTERNET_NO = 1,
    NLA_INTERNET_YES = 2
};

struct NLA_BLOB {
    struct {
        NLA_BLOB_DATA_TYPE type;
        DWORD dwSize;
        DWORD nextOffset;
    } header;

    union {
        // NLA_RAW_DATA
        CHAR rawData[1];

        // NLA_INTERFACE
        struct {
            DWORD dwType;
            DWORD dwSpeed;
            CHAR adapterName[1];
        } interfaceData;

        // NLA_802_1X_LOCATION
        struct {
            CHAR information[1];
        } locationData;

        // NLA_CONNECTIVITY
        struct {
            NLA_CONNECTIVITY_TYPE type;
            NLA_INTERNET internet;
        } connectivity;

        // NLA_ICS
        struct {
            struct {
                DWORD speed;
                DWORD type;
                DWORD state;
                WCHAR machineName[256];
                WCHAR sharedAdapterName[256];
            } remote;
        } ICS;
    } data;
};
#endif // NLA_NAMESPACE_GUID

#endif

enum NDIS_MEDIUM {
    NdisMedium802_3 = 0,
};

enum NDIS_PHYSICAL_MEDIUM {
    NdisPhysicalMediumWirelessLan = 1,
    NdisPhysicalMediumBluetooth = 10,
    NdisPhysicalMediumWiMax = 12,
};

#define OID_GEN_MEDIA_SUPPORTED 0x00010103
#define OID_GEN_PHYSICAL_MEDIUM 0x00010202

#define IOCTL_NDIS_QUERY_GLOBAL_STATS \
    CTL_CODE(FILE_DEVICE_PHYSICAL_NETCARD, 0, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)

QT_END_NAMESPACE

#endif // QPLATFORMDEFS_WIN_H
