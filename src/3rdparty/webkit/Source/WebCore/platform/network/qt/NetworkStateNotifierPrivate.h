/*
    Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef NetworkStateNotifierPrivate_h
#define NetworkStateNotifierPrivate_h

#include <QObject>

class QNetworkConfigurationManager;

namespace WebCore {

class NetworkStateNotifier;

class NetworkStateNotifierPrivate : public QObject {
    WEB_CS_OBJECT(NetworkStateNotifierPrivate)

public:
    NetworkStateNotifierPrivate(NetworkStateNotifier* notifier);
    ~NetworkStateNotifierPrivate();

    WEB_CS_SLOT_1(Public, void onlineStateChanged(bool isOnline))
    WEB_CS_SLOT_2(onlineStateChanged)
    WEB_CS_SLOT_1(Public, void networkAccessPermissionChanged(bool isAllowed))
    WEB_CS_SLOT_2(networkAccessPermissionChanged)

    QNetworkConfigurationManager* m_configurationManager;
    bool m_online;
    bool m_networkAccessAllowed;
    NetworkStateNotifier* m_notifier;
};

} // namespace WebCore

#endif
