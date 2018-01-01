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

#ifndef PHONON_PATH_P_H
#define PHONON_PATH_P_H

#include <path.h>

#include <QtCore/QPair>
#include <QtCore/QList>
#include <QtCore/QSharedData>
#include "effect.h"
#include <medianodedestructionhandler_p.h>

QT_BEGIN_NAMESPACE

class QObject;

namespace Phonon
{

class MediaNode;
typedef QPair<QObject*, QObject*> QObjectPair;


class PathPrivate : public QSharedData, private MediaNodeDestructionHandler
{
    friend class Path;
    public:
        PathPrivate()
            : sourceNode(0), sinkNode(0)
#ifndef QT_NO_PHONON_EFFECT
            , effectsParent(0)
#endif //QT_NO_PHONON_EFFECT
        {
        }

        ~PathPrivate();

        MediaNode *sourceNode;
        MediaNode *sinkNode;

    protected:
        void phononObjectDestroyed(MediaNodePrivate *) override;

#ifndef QT_NO_PHONON_EFFECT
        QObject *effectsParent; // used as parent for Effects created in insertEffect
        QList<Effect *> effects;
#endif
    private:
        bool executeTransaction( const QList<QObjectPair> &disconnections, const QList<QObjectPair> &connections);
#ifndef QT_NO_PHONON_EFFECT
        bool removeEffect(Effect *effect);
#endif
};

} // namespace Phonon

QT_END_NAMESPACE

#endif // PATH_P_H
