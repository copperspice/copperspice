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

#ifndef PHONON_ABSTRACTVIDEOOUTPUT_H
#define PHONON_ABSTRACTVIDEOOUTPUT_H

#include "phonondefs.h"
#include "phonon_export.h"
#include "medianode.h"
#include <QtCore/QObject>
#include <qstringfwd.h>

#ifndef QT_NO_PHONON_VIDEO

namespace Phonon
{

namespace Experimental
{
    class Visualization;
    class VisualizationPrivate;
} // namespace Experimental

    class AbstractVideoOutputPrivate;

    class PHONON_EXPORT AbstractVideoOutput : public MediaNode
    {
        friend class Experimental::Visualization;
        friend class Experimental::VisualizationPrivate;
        K_DECLARE_PRIVATE(AbstractVideoOutput)
        protected:
            AbstractVideoOutput(AbstractVideoOutputPrivate &d);
    };
} //namespace Phonon

#endif //QT_NO_PHONON_VIDEO

#endif // Phonon_ABSTRACTVIDEOOUTPUTBASE_H
