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

#include <QtCore/qdebug.h>
#include <QtCore/qstringlist.h>

#include <qeglproperties_p.h>
#include <qeglcontext_p.h>

QT_BEGIN_NAMESPACE

static void noegl(const char *fn)
{
   qWarning() << fn << " called, but Qt configured without EGL" << endl;
}

#define NOEGL noegl(__FUNCTION__);

// Initialize a property block.
QEglProperties::QEglProperties()
{
   NOEGL
}

QEglProperties::QEglProperties(EGLConfig cfg)
{
   Q_UNUSED(cfg)
   NOEGL
}

// Fetch the current value associated with a property.
int QEglProperties::value(int name) const
{
   Q_UNUSED(name)
   NOEGL
   return 0;
}

// Set the value associated with a property, replacing an existing
// value if there is one.
void QEglProperties::setValue(int name, int value)
{
   Q_UNUSED(name)
   Q_UNUSED(value)
   NOEGL
}

// Remove a property value.  Returns false if the property is not present.
bool QEglProperties::removeValue(int name)
{
   Q_UNUSED(name)
   NOEGL
   return false;
}

void QEglProperties::setDeviceType(int devType)
{
   Q_UNUSED(devType)
   NOEGL
}


// Sets the red, green, blue, and alpha sizes based on a pixel format.
// Normally used to match a configuration request to the screen format.
void QEglProperties::setPixelFormat(QImage::Format pixelFormat)
{
   Q_UNUSED(pixelFormat)
   NOEGL

}

void QEglProperties::setRenderableType(QEgl::API api)
{
   Q_UNUSED(api);
   NOEGL
}

// Reduce the complexity of a configuration request to ask for less
// because the previous request did not result in success.  Returns
// true if the complexity was reduced, or false if no further
// reductions in complexity are possible.
bool QEglProperties::reduceConfiguration()
{
   NOEGL
   return false;
}

static void addTag(QString &str, const QString &tag)
{
   Q_UNUSED(str)
   Q_UNUSED(tag)
   NOEGL
}

// Convert a property list to a string suitable for debug output.
QString QEglProperties::toString() const
{
   NOEGL
   return QString();
}

void QEglProperties::setPaintDeviceFormat(QPaintDevice *dev)
{
   Q_UNUSED(dev)
   NOEGL
}

QT_END_NAMESPACE


