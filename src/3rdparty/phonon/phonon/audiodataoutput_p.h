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

#ifndef PHONON_AUDIODATAOUTPUT_P_H
#define PHONON_AUDIODATAOUTPUT_P_H

#include "audiodataoutput.h"
#include "abstractaudiooutput_p.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{

class AudioDataOutputPrivate : public AbstractAudioOutputPrivate
{
    Q_DECLARE_PUBLIC(AudioDataOutput)
    PHONON_PRIVATECLASS
    protected:
        AudioDataOutputPrivate()
            : dataSize(512)
        {
        }

        int dataSize;
};

} // namespace Phonon

QT_END_NAMESPACE

#endif
