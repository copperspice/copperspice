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
#ifndef DnsPrefetchHelper_h
#define DnsPrefetchHelper_h

#include <QObject>
#include <QCache>
#include <QHostInfo>
#include <QSet>
#include <QString>
#include <QTime>
#include "qwebsettings.h"

namespace WebCore {

    class DnsPrefetchHelper : public QObject {
        WEB_CS_OBJECT(DnsPrefetchHelper)

    public:
        DnsPrefetchHelper() : QObject(), currentLookups(0) { }

        WEB_CS_SLOT_1(Public,void lookup(QString hostname))
        WEB_CS_SLOT_2(lookup)

        WEB_CS_SLOT_1(Public,void lookedUp(const QHostInfo &hostInfo))
        WEB_CS_SLOT_2(lookedUp)


    protected:
        int currentLookups;
    };


}

#endif // DnsPrefetchHelper_h
