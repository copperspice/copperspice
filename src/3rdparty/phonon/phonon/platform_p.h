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


#ifndef PHONON_PLATFORM_P_H
#define PHONON_PLATFORM_P_H

#include <QtCore/QStringList>
#include <QtCore/QtGlobal>
#include <QtCore/QPair>
#include "phonon_export.h"
#include "objectdescription.h"

QT_BEGIN_NAMESPACE

class QIcon;
class QObject;
class QUrl;
class QStyle;

namespace Phonon
{
class AbstractMediaStream;

namespace Platform
{

void saveVolume(const QString &outputName, qreal volume);
qreal loadVolume(const QString &outputName);
AbstractMediaStream *createMediaStream(const QUrl &url, QObject *parent);
QIcon icon(const QString &name, QStyle *style = 0);
void notification(const char *notificationName, const QString &text,
        const QStringList &actions = QStringList(), QObject *receiver = 0,
        const char *actionSlot = 0);
QString applicationName();
QList<QPair<QByteArray, QString> > deviceAccessListFor(const Phonon::AudioOutputDevice &deviceDesc);

} // namespace Platform
} // namespace Phonon

QT_END_NAMESPACE

#endif // PHONON_PLATFORM_P_H
