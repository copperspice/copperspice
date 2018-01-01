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

#ifndef QT7_BACKENDINFO_H
#define QT7_BACKENDINFO_H

#include <phonon/mediasource.h>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class BackendInfo
    {
        public:
            enum Scope {In, Out};

            static QString quickTimeVersionString();
            static bool isQuickTimeVersionAvailable(int minHexVersion);
            static QStringList quickTimeMimeTypes(Scope scope);
            static QStringList quickTimeCompressionFormats();
            static QStringList coreAudioCodecs(Scope scope);
            static QStringList coreAudioFileTypes(Scope scope);
    };

}} // namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_BACKENDINFO_H
