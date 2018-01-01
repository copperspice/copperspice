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
**  Copyright (C) 2005-2007 Matthias Kretz <kretz@kde.org
********************************************************/

#ifndef PHONON_FRONTENDINTERFACE_P_H
#define PHONON_FRONTENDINTERFACE_P_H

#include "addoninterface.h"
#include "mediaobject_p.h"
#include "phononnamespace_p.h"
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_MEDIACONTROLLER

namespace Phonon
{
class FrontendInterfacePrivate
{
    public:
        FrontendInterfacePrivate(MediaObject *mp) : media(mp) {
            Q_ASSERT(media);
            MediaObjectPrivate *d = media->k_func();
            d->interfaceList << this;
        }
        virtual ~FrontendInterfacePrivate() {
            if (media) {
                MediaObjectPrivate *d = media->k_func();
                d->interfaceList << this;
            }
        }
        virtual void backendObjectChanged(QObject *iface) = 0;
        void _backendObjectChanged() {
            pDebug() << Q_FUNC_INFO;
            QObject *x = media->k_ptr->backendObject();
            if (x) {
                backendObjectChanged(x);
            }
        }
        AddonInterface *iface() { return qobject_cast<AddonInterface *>(media->k_ptr->backendObject()); }
        QPointer<MediaObject> media;
};
} // namespace Phonon

#endif //QT_NO_PHONON_MEDIACONTROLLER

QT_END_NAMESPACE

#endif // PHONON_FRONTENDINTERFACEPRIVATE_H
