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

#ifndef DS9_BACKENDNODE_H
#define DS9_BACKENDNODE_H

#include "phononds9_namespace.h"
#include <QtCore/QObject>
#include <QtCore/QVector>
#include "compointer.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {
        class MediaObject;

        typedef ComPointer<IPin> InputPin;
        typedef ComPointer<IPin> OutputPin;
        typedef ComPointer<IBaseFilter> Filter;
        typedef ComPointer<IGraphBuilder> Graph;

        class BackendNode : public QObject
        {
            DS9_CS_OBJECT(BackendNode)

        public:
            BackendNode(QObject *parent);
            virtual ~BackendNode();

            MediaObject *mediaObject() const {return m_mediaObject;}

            static QList<InputPin> pins(const Filter &, PIN_DIRECTION);

            Filter filter(int index) const {
               return m_filters[index];
            }

            //add a pointer to the base Media Object (giving access to the graph and error management)
            void setMediaObject(MediaObject *mo);

            //called by the connections to tell the node that it's been connection to anothe one through its 'inpin' input port
            virtual void connected(BackendNode *, const InputPin& inpin) {}

         protected:
            Filter m_filters[FILTER_COUNT];
            MediaObject *m_mediaObject;

         private :
            DS9_CS_SLOT_1(Private, void mediaObjectDestroyed())
            DS9_CS_SLOT_2(mediaObjectDestroyed)

        };
    }
}

QT_END_NAMESPACE

#endif
