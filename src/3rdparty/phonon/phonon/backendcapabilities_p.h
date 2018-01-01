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

#ifndef PHONON_BACKENDCAPABILITIES_P_H
#define PHONON_BACKENDCAPABILITIES_P_H

#include "backendcapabilities.h"
#include <QtCore/QObject>
#include "factory_p.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{

   class BackendCapabilitiesPrivate : public BackendCapabilities::Notifier
   {
       public:
           BackendCapabilitiesPrivate()
           {
               connect(Factory::sender(), SIGNAL(backendChanged()),                      this, SLOT(capabilitiesChanged()));
               connect(Factory::sender(), SIGNAL(availableAudioOutputDevicesChanged()),  this, SLOT(availableAudioOutputDevicesChanged()));
               connect(Factory::sender(), SIGNAL(availableAudioCaptureDevicesChanged()), this, SLOT(availableAudioCaptureDevicesChanged()));
           }
};

}

QT_END_NAMESPACE

#endif
