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

#ifndef GSTREAMER_GSTHELPER_H
#define GSTREAMER_GSTHELPER_H

#include "common.h"
#include <gst/gst.h>

QT_BEGIN_NAMESPACE

template<class T> class QList;
class QByteArray;

namespace Phonon
{
namespace Gstreamer
{
class GstHelper
{
public:
    static QList<QByteArray> extractProperties(GstElement *elem, const QByteArray &value);
    static bool setProperty(GstElement *elem, const char *propertyName, const QByteArray &propertyValue);
    static QByteArray property(GstElement *elem, const char *propertyName);
    static QByteArray name(GstObject *elem);
    static GstElement* createPluggablePlaybin();
};

} // ns Gstreamer
} // ns Phonon

QT_END_NAMESPACE

#endif // Phonon_GSTREAMER_GSTHELPER_H
