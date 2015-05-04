/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#ifndef PHONON_ABSTRACTVIDEOOUTPUT_H
#define PHONON_ABSTRACTVIDEOOUTPUT_H

#include "phonondefs.h"
#include "phonon_export.h"
#include "medianode.h"
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VIDEO

class QString;

namespace Phonon
{

namespace Experimental
{
    class Visualization;
    class VisualizationPrivate;
} // namespace Experimental

    class AbstractVideoOutputPrivate;

    /** \class AbstractVideoOutput abstractvideooutput.h Phonon/AbstractVideoOutput
     * \brief Common base class for all video outputs.
     *
     * \see VideoWidget
     */
    class PHONON_EXPORT AbstractVideoOutput : public MediaNode
    {
        friend class Experimental::Visualization;
        friend class Experimental::VisualizationPrivate;
        K_DECLARE_PRIVATE(AbstractVideoOutput)
        protected:
            /**
             * \internal
             * Constructor that is called from derived classes.
             *
             * \param d the private object
             */
            AbstractVideoOutput(AbstractVideoOutputPrivate &d);
    };
} //namespace Phonon

#endif //QT_NO_PHONON_VIDEO

QT_END_NAMESPACE

#endif // Phonon_ABSTRACTVIDEOOUTPUTBASE_H
