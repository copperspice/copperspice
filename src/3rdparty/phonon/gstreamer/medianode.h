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

#ifndef GSTREAMER_MEDIANODE_H
#define GSTREAMER_MEDIANODE_H

#include "common.h"
#include "medianodeevent.h"
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QSize>
#include <gst/gst.h>

QT_BEGIN_NAMESPACE

namespace Phonon {
namespace Gstreamer {

class Message;
class MediaObject;
class Backend;

class MediaNode {
public:
    enum NodeDescriptionEnum {
        AudioSource     = 0x1,
        AudioSink       = 0x2,
        VideoSource     = 0x4,
        VideoSink       = 0x8
    };
    using NodeDescription = QFlags<NodeDescriptionEnum>;

    MediaNode(Backend *backend, NodeDescription description);

    virtual ~MediaNode();

    bool connectNode(QObject *other);
    bool disconnectNode(QObject *other);

    bool buildGraph();
    bool breakGraph();

    virtual bool link();
    virtual bool unlink();

    NodeDescription description() const {
        return m_description;
    }

    bool isValid() {
        return m_isValid;
    }

    MediaObject *root() {
        return m_root;
    }

    void setRoot(MediaObject *mediaObject) {
        m_root = mediaObject;
    }

    void notify(const MediaNodeEvent *event);

    Backend *backend() {
        return m_backend;
    }

    const QString &name() {
        return m_name;
    }

    virtual GstElement *audioElement() {
        return m_audioTee;
    }

    virtual GstElement *videoElement() {
        return m_videoTee;
    }

protected:
    bool connectToFakeSink(GstElement *tee, GstElement *sink, GstElement *bin);
    bool releaseFakeSinkIfConnected(GstElement *tee, GstElement *sink, GstElement *bin);
    bool linkMediaNodeList(QList<QObject *> &list, GstElement *bin, GstElement *tee, GstElement *sink, GstElement *src);

    virtual void mediaNodeEvent(const MediaNodeEvent *event);
    QList<QObject *> m_audioSinkList;
    QList<QObject *> m_videoSinkList;

    bool m_isValid;
    MediaObject *m_root;
    GstElement *m_audioTee;
    GstElement *m_videoTee;
    GstElement *m_fakeAudioSink;
    GstElement *m_fakeVideoSink;
   Backend *m_backend;
    QString m_name;
 
private:
    bool addOutput(MediaNode *, GstElement *tee);
    NodeDescription m_description;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MediaNode::NodeDescription)

} // ns Gstreamer
} // ns Phonon

CS_DECLARE_INTERFACE(Phonon::Gstreamer::MediaNode, "org.phonon.gstreamer.MediaNode")

QT_END_NAMESPACE

#endif // Phonon_GSTREAMER_MEDIANODE_H
