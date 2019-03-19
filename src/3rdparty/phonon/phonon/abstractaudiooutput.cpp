/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>
* Copyright (C) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#include "abstractaudiooutput.h"
#include "abstractaudiooutput_p.h"
#include "factory_p.h"

#define PHONON_CLASSNAME AbstractAudioOutput

QT_BEGIN_NAMESPACE

namespace Phonon
{
    AbstractAudioOutput::AbstractAudioOutput(AbstractAudioOutputPrivate &dd, QObject *parent)
      : QObject(parent), MediaNode(dd)

    {
    }

    AbstractAudioOutput::~AbstractAudioOutput()
    {
    }

}

QT_END_NAMESPACE

#undef PHONON_CLASSNAME

