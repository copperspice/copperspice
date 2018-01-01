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

#ifndef PHONON_ABSTRACTAUDIOOUTPUT_H
#define PHONON_ABSTRACTAUDIOOUTPUT_H

#include "phonondefs.h"
#include "phonon_export.h"
#include "medianode.h"
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    class AbstractAudioOutputPrivate;


    class PHONON_EXPORT AbstractAudioOutput : public QObject, public MediaNode
    {
        PHN_CS_OBJECT(AbstractAudioOutput)
        K_DECLARE_PRIVATE(AbstractAudioOutput)

       protected:
           AbstractAudioOutput(AbstractAudioOutputPrivate &dd, QObject *parent);

       public:
           ~AbstractAudioOutput();       
    };

} 

QT_END_NAMESPACE

#endif
