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

#ifndef PHONON_MEDIANODE_H
#define PHONON_MEDIANODE_H

#include "phonondefs.h"
#include "phonon_export.h"
#include "path.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
    class Path;
    class MediaNodePrivate;

    class PHONON_EXPORT MediaNode
    {
        friend class Path;
        friend class PathPrivate;
        friend PHONON_EXPORT Path createPath(MediaNode *source, MediaNode *sink);
        K_DECLARE_PRIVATE(MediaNode)

      public:
         virtual ~MediaNode();
         
         bool isValid() const;
         bool pluginLoaded() const;

         QList<Path> inputPaths() const;
         QList<Path> outputPaths() const;

      protected:
         MediaNode(MediaNodePrivate &dd);
         MediaNodePrivate *const k_ptr;

    };

} // namespace Phonon

QT_END_NAMESPACE

#endif // PHONON_MEDIANODE_H
