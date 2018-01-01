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

#include "videoplayer.h"
#include "mediaobject.h"
#include "audiooutput.h"
#include "videowidget.h"
#include "path.h"
#include <QtGui/QBoxLayout>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VIDEOPLAYER

namespace Phonon
{

class VideoPlayerPrivate
{
    public:
        VideoPlayerPrivate()
            : player(0)
            , aoutput(0)
            , voutput(0) {}

        void init(VideoPlayer *q, Phonon::Category category);

        MediaObject *player;
        AudioOutput *aoutput;
        VideoWidget *voutput;

        MediaSource src;
};

void VideoPlayerPrivate::init(VideoPlayer *q, Phonon::Category category)
{
    QVBoxLayout *layout = new QVBoxLayout(q);
    layout->setMargin(0);

    aoutput = new AudioOutput(category, q);

    voutput = new VideoWidget(q);
    layout->addWidget(voutput);

    player = new MediaObject(q);
    Phonon::createPath(player, aoutput);
    Phonon::createPath(player, voutput);

    q->connect(player, SIGNAL(finished()), q, SLOT(finished()));
}

VideoPlayer::VideoPlayer(Phonon::Category category, QWidget *parent)
    : QWidget(parent)
    , d(new VideoPlayerPrivate)
{
    d->init(this, category);
}

VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
    , d(new VideoPlayerPrivate)
{
    d->init(this, Phonon::VideoCategory);
}

VideoPlayer::~VideoPlayer()
{
    delete d;
}

MediaObject *VideoPlayer::mediaObject() const
{
    return d->player;
}

AudioOutput *VideoPlayer::audioOutput() const
{
    return d->aoutput;
}

VideoWidget *VideoPlayer::videoWidget() const
{
    return d->voutput;
}

void VideoPlayer::load(const MediaSource &source)
{
    d->player->setCurrentSource(source);
}

void VideoPlayer::play(const MediaSource &source)
{
    if (source == d->player->currentSource()) {
        if (!isPlaying())
            d->player->play();
        return;
    }

    // new URL
    d->player->setCurrentSource(source);
        
    if (ErrorState == d->player->state())
        return;

    d->player->play();
}

void VideoPlayer::play()
{
    d->player->play();
}

void VideoPlayer::pause()
{
    d->player->pause();
}

void VideoPlayer::stop()
{
    d->player->stop();
}

qint64 VideoPlayer::totalTime() const
{
    return d->player->totalTime();
}

qint64 VideoPlayer::currentTime() const
{
    return d->player->currentTime();
}

void VideoPlayer::seek(qint64 ms)
{
    d->player->seek(ms);
}

float VideoPlayer::volume() const
{
    return d->aoutput->volume();
}

void VideoPlayer::setVolume(float v)
{
    d->aoutput->setVolume(v);
}

bool VideoPlayer::isPlaying() const
{
    return (d->player->state() == PlayingState);
}

bool VideoPlayer::isPaused() const
{
    return (d->player->state() == PausedState);
}

} // namespaces

#endif //QT_NO_PHONON_VIDEOPLAYER

QT_END_NAMESPACE

