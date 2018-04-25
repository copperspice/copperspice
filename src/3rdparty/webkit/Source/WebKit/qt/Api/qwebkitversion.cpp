/*
    Copyright (C) 2009 Robert Hogan <robert@roberthogan.net>

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
#include <qwebkitversion.h>
#include <WebKitVersion.h>

QString qWebKitVersion()
{
    return QString("%1.%2").formatArg(WEBKIT_MAJOR_VERSION).formatArg(WEBKIT_MINOR_VERSION);
}

int qWebKitMajorVersion()
{
    return WEBKIT_MAJOR_VERSION;
}

int qWebKitMinorVersion()
{
    return WEBKIT_MINOR_VERSION;
}
