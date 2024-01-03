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

#include "config.h"
#include "DnsPrefetchHelper.h"

#include "PlatformString.h"

namespace WebCore {
// this is called on mouse over a href and on page loading
void prefetchDNS(const String& hostname)
{
    if (QWebSettings::globalSettings()->testAttribute(QWebSettings::DnsPrefetchEnabled)) {
        static DnsPrefetchHelper dnsPrefetchHelper;
        dnsPrefetchHelper.lookup(QString(hostname));
    }
}

void DnsPrefetchHelper::lookup(QString hostname)
{
   if (hostname.isEmpty())
       return; // this actually happens

   if (currentLookups >= 10)
       return; // do not launch more than 10 lookups at the same time

   currentLookups++;
   QHostInfo::lookupHost(hostname, this, SLOT(lookedUp(QHostInfo)));
}

void DnsPrefetchHelper::lookedUp(const QHostInfo &hostInfo)
{
   // we do not cache the result, we throw it away.
   // we currently rely on the OS to cache the results. If it does not do that
   // then at least the ISP nameserver did it.

   (void) hostInfo;

   currentLookups--;
}

}