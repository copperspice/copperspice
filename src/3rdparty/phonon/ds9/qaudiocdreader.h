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

#ifndef DS9_QAUDIOCDREADER_H
#define DS9_QAUDIOCDREADER_H

#include "qasyncreader.h"
#include "qbasefilter.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_MEDIACONTROLLER

namespace Phonon
{
    namespace DS9
    {
        struct CDROM_TOC;
        struct WaveStructure;
        EXTERN_C const IID IID_ITitleInterface;

        //interface for the Titles
        struct ITitleInterface : public IUnknown
        {
            virtual QList<qint64> titles() const = 0;
        };


        class QAudioCDPlayer : public QBaseFilter
        {
        public:
            QAudioCDPlayer();
            ~QAudioCDPlayer();

            STDMETHODIMP QueryInterface(REFIID iid, void** out) override;
        };
    }
}

#endif //QT_NO_PHONON_MEDIACONTROLLER

QT_END_NAMESPACE

#endif
