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

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#include "audiooutputinterface.h"
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QPair>
#include "platform_p.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{

QList<QPair<QByteArray, QString> > AudioOutputInterface42::deviceAccessListFor(const Phonon::AudioOutputDevice &deviceDesc) const
{
    return Platform::deviceAccessListFor(deviceDesc);
}

} // namespace Phonon

QT_END_NAMESPACE
