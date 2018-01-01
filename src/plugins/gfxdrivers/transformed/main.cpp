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

#include <qscreendriverplugin_qws.h>
#include <qscreentransformed_qws.h>
#include <qstringlist.h>

QT_BEGIN_NAMESPACE

class GfxTransformedDriver : public QScreenDriverPlugin
{
public:
    GfxTransformedDriver();

    QStringList keys() const;
    QScreen *create(const QString&, int displayId);
};

GfxTransformedDriver::GfxTransformedDriver()
: QScreenDriverPlugin()
{
}

QStringList GfxTransformedDriver::keys() const
{
    QStringList list;
    list << "Transformed";
    return list;
}

QScreen* GfxTransformedDriver::create(const QString& driver, int displayId)
{
#ifndef QT_NO_QWS_TRANSFORMED
    if (driver.toLower() == "transformed")
        return new QTransformedScreen(displayId);
#else //QT_NO_QWS_TRANSFORMED
    printf("QT buildt with QT_NO_QWS_TRANSFORMED. No screen driver returned\n");
#endif //QT_NO_QWS_TRANSFORMED
    return 0;
}

Q_EXPORT_STATIC_PLUGIN(GfxTransformedDriver)
Q_EXPORT_PLUGIN2(qgfxtransformed, GfxTransformedDriver)

QT_END_NAMESPACE

