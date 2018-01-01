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

#include "medianode.h"
#include "medianode_p.h"
#include "medianodedestructionhandler_p.h"
#include "factory_p.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{

    MediaNode::MediaNode(MediaNodePrivate &dd)
        : k_ptr(&dd)
    {
        k_ptr->q_ptr = this;
    }

/**
 * Returns true if the plugin loaded 
 *
 * \copperspice
 */
bool MediaNode::pluginLoaded() const
{
    // detected in QLibraryPrivate::loadPlugin(), set in FactoryPrivate::createBackend()
    return k_ptr->m_backendObject;
}


bool MediaNode::isValid() const
{
    return const_cast<MediaNodePrivate *>(k_ptr)->backendObject() != 0;
}

    QList<Path> MediaNode::inputPaths() const
    {
        return k_ptr->inputPaths;
    }

    QList<Path> MediaNode::outputPaths() const
    {
        return k_ptr->outputPaths;
    }

    MediaNode::~MediaNode()
    {
        delete k_ptr;
    }

    QObject *MediaNodePrivate::backendObject()
    {
        if (!m_backendObject && Factory::backend()) {
            createBackendObject();
        }
        return m_backendObject;
    }

    MediaNodePrivate::~MediaNodePrivate()
    {
        for (int i = 0 ; i < handlers.count(); ++i) {
            handlers.at(i)->phononObjectDestroyed(this);
        }
        Factory::deregisterFrontendObject(this);
        delete m_backendObject;
        m_backendObject = 0;
    }

void MediaNodePrivate::deleteBackendObject()
{
    if (m_backendObject && aboutToDeleteBackendObject()) {
        delete m_backendObject;
        m_backendObject = 0;
    }
}

    MediaNodePrivate::MediaNodePrivate(MediaNodePrivate::CastId _castId) : castId(_castId),
        m_backendObject(0)
    {
        Factory::registerFrontendObject(this);
    }

    void MediaNodePrivate::addDestructionHandler(MediaNodeDestructionHandler *handler)
    {
        handlers.append(handler);
    }

    void MediaNodePrivate::removeDestructionHandler(MediaNodeDestructionHandler *handler)
    {
        handlers.removeAll(handler);
    }

    void MediaNodePrivate::addOutputPath(const Path &p)
    {
        outputPaths.append(p);
    }

    void MediaNodePrivate::addInputPath(const Path &p)
    {
        inputPaths.append(p);
    }

    void MediaNodePrivate::removeOutputPath(const Path &p)
    {
        int ret = outputPaths.removeAll(p);
        Q_ASSERT(ret == 1);
        Q_UNUSED(ret);
    }
    
    void MediaNodePrivate::removeInputPath(const Path &p)
    {
        int ret = inputPaths.removeAll(p);
        Q_ASSERT(ret == 1);
        Q_UNUSED(ret);
    }

} // namespace Phonon

QT_END_NAMESPACE
