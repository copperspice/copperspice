/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
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

#include "backendnode.h"
#include "mediaobject.h"

#include <ocidl.h> // ISpecifyPropertyPages
#include <olectl.h> // OleCreatePropertyFrame

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {
        // Displays a property dialog for a filter (experimental but should be put into main
        /*static void showPropertyDialog(const Filter &filter)
        {
            ComPointer<ISpecifyPropertyPages> prop(filter, IID_ISpecifyPropertyPages);
            if (prop != 0) {
                IUnknown *iunk[] = {filter};
                // Show the page.
                CAUUID caGUID;
                prop->GetPages(&caGUID);
                OleCreatePropertyFrame(
                    0, // Parent window
                    0, 0,                   // (Reserved)
                    0,                      // Caption for the dialog box
                    1,                      // Number of objects (just the filter)
                    iunk,                  // Array of object pointers.
                    caGUID.cElems,          // Number of property pages
                    caGUID.pElems,          // Array of property page CLSIDs
                    0,                      // Locale identifier
                    0, 0                    // Reserved
                    );
            }
        }*/

        //for now we have 2 graphs that do the same
        BackendNode::BackendNode(QObject *parent) : QObject(parent), m_mediaObject(0)
        {
        }

        BackendNode::~BackendNode()
        {
            //this will remove the filter from the graph
            FILTER_INFO info;
            for(int i = 0; i < FILTER_COUNT; ++i) {
                const Filter &filter = m_filters[i];
                if (!filter)
                    continue;
                filter->QueryFilterInfo(&info);
                if (info.pGraph) {
                    HRESULT hr = info.pGraph->RemoveFilter(filter);

                    if (FAILED(hr) && m_mediaObject) {
                        m_mediaObject->ensureStopped();

                        hr = info.pGraph->RemoveFilter(filter);
                    }
                    Q_ASSERT(SUCCEEDED(hr));
                    info.pGraph->Release();
                }
            }
        }

        void BackendNode::setMediaObject(MediaObject *mo)
        {
            if (m_mediaObject) {
                disconnect(m_mediaObject, SIGNAL(destroyed()), this, SLOT(mediaObjectDestroyed()));
            }
            m_mediaObject = mo;
            connect(mo, SIGNAL(destroyed()), SLOT(mediaObjectDestroyed()));
        }

        void BackendNode::mediaObjectDestroyed()
        {
            //remove the filter from its graph
            FILTER_INFO info;
            for(int i = 0; i < FILTER_COUNT; ++i) {
                const Filter &filter = m_filters[i];
                if (!filter)
                    continue;
                filter->QueryFilterInfo(&info);
                if (info.pGraph) {
                    HRESULT hr = info.pGraph->RemoveFilter(filter);
                    Q_ASSERT(SUCCEEDED(hr));
                    Q_UNUSED(hr);
                    info.pGraph->Release();
                }
            }
            m_mediaObject = 0;
        }

        QList<InputPin> BackendNode::pins(const Filter &filter, PIN_DIRECTION wantedDirection)
        {
            QList<InputPin> ret;

            if (filter) {

                ComPointer<IEnumPins> enumPin;
                HRESULT hr = filter->EnumPins(enumPin.pparam());
                Q_UNUSED(hr);
                Q_ASSERT( SUCCEEDED(hr));

                InputPin pin;

                while (enumPin->Next(1, pin.pparam(), 0) == S_OK) {
                    PIN_DIRECTION dir;
                    hr = pin->QueryDirection(&dir);

                    Q_ASSERT( SUCCEEDED(hr));

                    if (dir == wantedDirection) {
                        ret.append(pin);
                    }
                }
            }

            return ret;
        }
    }
}

QT_END_NAMESPACE

