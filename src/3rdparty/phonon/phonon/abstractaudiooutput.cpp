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

