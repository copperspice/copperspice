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

#include "path.h"
#include "path_p.h"

#include "phononnamespace_p.h"
#include "backendinterface.h"
#include "factory_p.h"
#include "medianode.h"
#include "medianode_p.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{

class ConnectionTransaction
{
    public:
        ConnectionTransaction(BackendInterface *b, const QSet<QObject*> &x) : backend(b), list(x)
        {
            success = backend->startConnectionChange(list);
        }
        ~ConnectionTransaction()
        {
            backend->endConnectionChange(list);
        }
        operator bool()
        {
            return success;
        }
    private:
        bool success;
        BackendInterface *const backend;
        const QSet<QObject*> list;
};

PathPrivate::~PathPrivate()
{
#ifndef QT_NO_PHONON_EFFECT
    for (int i = 0; i < effects.count(); ++i) {
        effects.at(i)->k_ptr->removeDestructionHandler(this);
    }
    delete effectsParent;
#endif
}

Path::~Path()
{
}

Path::Path()
    : d(new PathPrivate)
{
}

Path::Path(const Path &rhs)
    : d(rhs.d)
{
}

bool Path::isValid() const
{
    return d->sourceNode != 0 && d->sinkNode != 0;
}

#ifndef QT_NO_PHONON_EFFECT
Effect *Path::insertEffect(const EffectDescription &desc, Effect *insertBefore)
{
    if (!d->effectsParent) {
        d->effectsParent = new QObject;
    }
    Effect *e = new Effect(desc, d->effectsParent);
    if (!e->isValid()) {
        delete e;
        return 0;
    }
    bool success = insertEffect(e, insertBefore);
    if (!success) {
        delete e;
        return 0;
    }
    return e;
}

bool Path::insertEffect(Effect *newEffect, Effect *insertBefore)
{
    QObject *newEffectBackend = newEffect ? newEffect->k_ptr->backendObject() : 0;
    if (!isValid() || !newEffectBackend || d->effects.contains(newEffect) ||
            (insertBefore && (!d->effects.contains(insertBefore) || !insertBefore->k_ptr->backendObject()))) {
        return false;
    }
    QObject *leftNode = 0;
    QObject *rightNode = 0;
    const int insertIndex = insertBefore ? d->effects.indexOf(insertBefore) : d->effects.size();
    if (insertIndex == 0) {
        //prepend
        leftNode = d->sourceNode->k_ptr->backendObject();
    } else {
        leftNode = d->effects[insertIndex - 1]->k_ptr->backendObject();
    }

    if (insertIndex == d->effects.size()) {
        //append
        rightNode = d->sinkNode->k_ptr->backendObject();
    } else {
        Q_ASSERT(insertBefore);
        rightNode = insertBefore->k_ptr->backendObject();
    }

    QList<QObjectPair> disconnections, connections;
    disconnections << QObjectPair(leftNode, rightNode);
    connections << QObjectPair(leftNode, newEffectBackend)
        << QObjectPair(newEffectBackend, rightNode);

    if (d->executeTransaction(disconnections, connections)) {
        newEffect->k_ptr->addDestructionHandler(d.data());
        d->effects.insert(insertIndex, newEffect);
        return true;
    } else {
        return false;
    }
}

bool Path::removeEffect(Effect *effect)
{
    return d->removeEffect(effect);
}

QList<Effect *> Path::effects() const
{
    return d->effects;
}
#endif //QT_NO_PHONON_EFFECT

bool Path::reconnect(MediaNode *source, MediaNode *sink)
{
    if (! source || ! sink || ! source->k_ptr->backendObject() || !sink->k_ptr->backendObject()) {
        return false;
    }

    QList<QObjectPair> disconnections, connections;

    //backend objects
    QObject *bnewSource     = source->k_ptr->backendObject();
    QObject *bnewSink       = sink->k_ptr->backendObject();
    QObject *bcurrentSource = d->sourceNode ? d->sourceNode->k_ptr->backendObject() : 0;
    QObject *bcurrentSink   = d->sinkNode ? d->sinkNode->k_ptr->backendObject() : 0;

    if (bnewSource != bcurrentSource) {
        // we need to change the source

#ifndef QT_NO_PHONON_EFFECT
        MediaNode *next = d->effects.isEmpty() ? sink : d->effects.first();
#else
        MediaNode *next = sink;
#endif

        QObject *bnext = next->k_ptr->backendObject();

        if (bcurrentSource) {
            disconnections << QObjectPair(bcurrentSource, bnext);
        } 
        connections << QObjectPair(bnewSource, bnext);
    }

    if (bnewSink != bcurrentSink) {

#ifndef QT_NO_PHONON_EFFECT
        MediaNode *previous = d->effects.isEmpty() ? source : d->effects.last();
#else
        MediaNode *previous = source;
#endif

        QObject *bprevious = previous->k_ptr->backendObject();

        if (bcurrentSink) {
            disconnections << QObjectPair(bprevious, bcurrentSink);
        } 

        QObjectPair pair(bprevious, bnewSink);
        if (!connections.contains(pair)) //avoid connecting twice
            connections << pair;
    }

    if (d->executeTransaction(disconnections, connections)) {

        //everything went well,  update the path and the sink node

        if (d->sinkNode != sink) {
            if (d->sinkNode) {
                d->sinkNode->k_ptr->removeInputPath(*this);
                d->sinkNode->k_ptr->removeDestructionHandler(d.data());
            }
            sink->k_ptr->addInputPath(*this);
            d->sinkNode = sink;
            d->sinkNode->k_ptr->addDestructionHandler(d.data());
        }

        //everything went well, update the path and the source node
        if (d->sourceNode != source) {
            source->k_ptr->addOutputPath(*this);
            if (d->sourceNode) {
                d->sourceNode->k_ptr->removeOutputPath(*this);
                d->sourceNode->k_ptr->removeDestructionHandler(d.data());
            }
            d->sourceNode = source;
            d->sourceNode->k_ptr->addDestructionHandler(d.data());
        }

        return true;

    } else {
        return false;
    }
}

bool Path::disconnect()
{
    if (! isValid()) {
        return false;
    }

    QObjectList list;

    if (d->sourceNode)
        list << d->sourceNode->k_ptr->backendObject();

#ifndef QT_NO_PHONON_EFFECT
    for (int i = 0; i < d->effects.count(); ++i) {
        list << d->effects.at(i)->k_ptr->backendObject();
    }
#endif

    if (d->sinkNode) {
        list << d->sinkNode->k_ptr->backendObject();
    }

    //lets build the disconnection list
    QList<QObjectPair> disco;
    if (list.count() >=2 ) {
        QObjectList::const_iterator it = list.constBegin();
        for(;it+1 != list.constEnd();++it) {
            disco << QObjectPair(*it, *(it+1));
        }
    }

    if (d->executeTransaction(disco, QList<QObjectPair>())) {
        //everything went well, let's remove the reference
        //to the paths from the source and sink
        if (d->sourceNode) {
            d->sourceNode->k_ptr->removeOutputPath(*this);
            d->sourceNode->k_ptr->removeDestructionHandler(d.data());
        }
        d->sourceNode = 0;

#ifndef QT_NO_PHONON_EFFECT
        for (int i = 0; i < d->effects.count(); ++i) {
            d->effects.at(i)->k_ptr->removeDestructionHandler(d.data());
        }
        d->effects.clear();
#endif 

        if (d->sinkNode) {
            d->sinkNode->k_ptr->removeInputPath(*this);
            d->sinkNode->k_ptr->removeDestructionHandler(d.data());
        }
        d->sinkNode = 0;
        return true;
    } else {
        return false;
    }
}

MediaNode *Path::source() const
{
    return d->sourceNode;
}

MediaNode *Path::sink() const
{
    return d->sinkNode;
}



bool PathPrivate::executeTransaction( const QList<QObjectPair> &disconnections, const QList<QObjectPair> &connections)
{
    QSet<QObject*> nodesForTransaction;

    for (int i = 0; i < disconnections.count(); ++i) {
        const QObjectPair &pair = disconnections.at(i);
        nodesForTransaction << pair.first;
        nodesForTransaction << pair.second;
    }

    for (int i = 0; i < connections.count(); ++i) {
        const QObjectPair &pair = connections.at(i);
        nodesForTransaction << pair.first;
        nodesForTransaction << pair.second;
    }

    BackendInterface *backend = qobject_cast<BackendInterface *>(Factory::backend());
    if (! backend) {
        return false;
    }

    ConnectionTransaction transaction(backend, nodesForTransaction);
    if (! transaction) {
        return false;
    }

    QList<QObjectPair>::const_iterator it = disconnections.begin();

    for(;it != disconnections.end();++it) {
        const QObjectPair &pair = *it;

        if (! backend->disconnectNodes(pair.first, pair.second)) {
            // Error: a disconnection failed
            QList<QObjectPair>::const_iterator it2 = disconnections.begin();

            // reconnect the nodes that were disconnected
            for(; it2 != it; ++it2) {
                const QObjectPair &pair = *it2;
                bool success = backend->connectNodes(pair.first, pair.second);

                Q_ASSERT(success); //a failure here means it is impossible to reestablish the connection
                Q_UNUSED(success);
            }

            return false;
        }
    }

    for(it = connections.begin(); it != connections.end();++it) {
        const QObjectPair &pair = *it;

        if (! backend->connectNodes(pair.first, pair.second)) {
            // Error: a connection failed
            QList<QObjectPair>::const_iterator it2 = connections.begin();

            for(; it2 != it; ++it2) {
                const QObjectPair &pair = *it2;
                bool success = backend->disconnectNodes(pair.first, pair.second);
                Q_ASSERT(success); //a failure here means it is impossible to reestablish the connection
                Q_UNUSED(success);
            }

            // reconnect the nodes that were disconnected
            for (int i = 0; i < disconnections.count(); ++i) {
                const QObjectPair &pair = disconnections.at(i);
                bool success = backend->connectNodes(pair.first, pair.second);

                Q_ASSERT(success); //a failure here means it is impossible to reestablish the connection
                Q_UNUSED(success);
            }

            return false;
        }
    }

    return true;
}

#ifndef QT_NO_PHONON_EFFECT
bool PathPrivate::removeEffect(Effect *effect)
{
    if (!effects.contains(effect))
        return false;

    QObject *leftNode = 0;
    QObject *rightNode = 0;
    const int index = effects.indexOf(effect);
    if (index == 0) {
        leftNode = sourceNode->k_ptr->backendObject(); //append
    } else {
        leftNode = effects[index - 1]->k_ptr->backendObject();
    }
    if (index == effects.size()-1) {
        rightNode = sinkNode->k_ptr->backendObject(); //prepend
    } else {
        rightNode = effects[index + 1]->k_ptr->backendObject();
    }

    QList<QObjectPair> disconnections, connections;
    QObject *beffect = effect->k_ptr->backendObject();
    disconnections << QObjectPair(leftNode, beffect) << QObjectPair(beffect, rightNode);
    connections << QObjectPair(leftNode, rightNode);

    if (executeTransaction(disconnections, connections)) {
        effect->k_ptr->removeDestructionHandler(this);
        effects.removeAt(index);
        return true;
    }
    return false;
}
#endif //QT_NO_PHONON_EFFECT


void PathPrivate::phononObjectDestroyed(MediaNodePrivate *mediaNodePrivate)
{
    Q_ASSERT(mediaNodePrivate);
    if (mediaNodePrivate == sinkNode->k_ptr || mediaNodePrivate == sourceNode->k_ptr) {
        //let's first disconnectq the path from its source and sink
        QObject *bsink = sinkNode->k_ptr->backendObject();
        QObject *bsource = sourceNode->k_ptr->backendObject();
        QList<QObjectPair> disconnections;
#ifndef QT_NO_PHONON_EFFECT
        disconnections << QObjectPair(bsource, effects.isEmpty() ? bsink : effects.first()->k_ptr->backendObject());
        if (!effects.isEmpty())
            disconnections << QObjectPair(effects.last()->k_ptr->backendObject(), bsink);
#else
        disconnections << QObjectPair(bsource, bsink);
#endif //QT_NO_PHONON_EFFECT

        executeTransaction(disconnections, QList<QObjectPair>());

        Path p; //temporary path
        p.d = this;
        if (mediaNodePrivate == sinkNode->k_ptr) {
            sourceNode->k_ptr->removeOutputPath(p);
            sourceNode->k_ptr->removeDestructionHandler(this);
        } else {
            sinkNode->k_ptr->removeInputPath(p);
            sinkNode->k_ptr->removeDestructionHandler(this);
        }
        sourceNode = 0;
        sinkNode = 0;
    } else {

#ifndef QT_NO_PHONON_EFFECT
        for (int i = 0; i < effects.count(); ++i) {
            Effect *e = effects.at(i);
            if (e->k_ptr == mediaNodePrivate) {
                removeEffect(e);
            }
        }
#endif
    }
}

Path createPath(MediaNode *source, MediaNode *sink)
{
    Path p;

    if (! p.reconnect(source, sink)) {

        const QObject *const src = source ? (source->k_ptr->qObject()
                ? source->k_ptr->qObject() : dynamic_cast<QObject *>(source) ) : nullptr;

        const QObject *const snk = sink ? (sink->k_ptr->qObject()
                ? sink->k_ptr->qObject() : dynamic_cast<QObject *>(sink) ) : nullptr;

        pWarning() << "Phonon::createPath: Can not connect "
            << (src ? src->metaObject()->className() : "")
            << '(' << (src ? (src->objectName().isEmpty() ? "no objectName" : qPrintable(src->objectName())) : "null") << ") to "
            << (snk ? snk->metaObject()->className() : "")
            << '(' << (snk ? (snk->objectName().isEmpty() ? "no objectName" : qPrintable(snk->objectName())) : "null")
            << ").";
    }
    return p;
}


Path & Path::operator=(const Path &other)
{
    d = other.d;
    return *this;
}

bool Path::operator==(const Path &other) const
{
    return d == other.d;
}

bool Path::operator!=(const Path &other) const
{
    return !operator==(other);
}

} // namespace Phonon

QT_END_NAMESPACE
