/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

//! [0]
PushStream::PushStream(QObject *parent)
  : AbstractMediaStream(parent), m_timer(new QTimer(this))
{
  setStreamSize(getMediaStreamSize());

  connect(m_timer, SIGNAL(timeout()), SLOT(moreData()));
  m_timer->setInterval(0);
}

void PushStream::moreData()
{
  const QByteArray data = getMediaData();
  if (data.isEmpty()) {
    endOfData();
  } else {
    writeData(data);
  }
}

void PushStream::needData()
{
  m_timer->start();
  moreData();
}

void PushStream::enoughData()
{
  m_timer->stop();
}
//! [0]


//! [1]
PullStream::PullStream(QObject *parent)
  : AbstractMediaStream(parent)
{
  setStreamSize(getMediaStreamSize());
}

void PullStream::needData()
{
  const QByteArray data = getMediaData();
  if (data.isEmpty()) {
    endOfData();
  } else {
    writeData(data);
  }
}
//! [1]


//! [2]
seekStream(0);
//! [2]


//! [3]
MediaObject m;
QString fileName("/home/foo/bar.ogg");
QUrl url("http://www.example.com/stream.mp3");
QBuffer *someBuffer;
m.setCurrentSource(fileName);
m.setCurrentSource(url);
m.setCurrentSource(someBuffer);
m.setCurrentSource(Phonon::Cd);
//! [3]


//! [4]
VideoPlayer *player = new VideoPlayer(Phonon::VideoCategory, parentWidget);
connect(player, SIGNAL(finished()), player, SLOT(deleteLater()));
player->play(url);
//! [4]


//! [5]
audioPlayer->load(url);
audioPlayer->play();
//! [5]


//! [6]
media = new MediaObject(this);
connect(media, SIGNAL(finished()), SLOT(slotFinished());
media->setCurrentSource("/home/username/music/filename.ogg");

...

media->play();
//! [6]


//! [7]
media->setCurrentSource(":/sounds/startsound.ogg");
media->enqueue("/home/username/music/song.mp3");
media->enqueue(":/sounds/endsound.ogg");
//! [7]


//! [8]
  media->setCurrentSource(":/sounds/startsound.ogg");
  connect(media, SIGNAL(aboutToFinish()), SLOT(enqueueNextSource()));
}

void enqueueNextSource()
{
  media->enqueue("/home/username/music/song.mp3");
}
//! [8]


//! [9]
int x = 200;
media->setTickInterval(x);
Q_ASSERT(x == producer->tickInterval());
//! [9]


//! [10]
int x = 200;
media->setTickInterval(x);
Q_ASSERT(x >= producer->tickInterval() &&
         x <= 2producer->tickInterval());
//! [10]


//! [11]
  connect(media, SIGNAL(hasVideoChanged(bool)), hasVideoChanged(bool));
  media->setCurrentSource("somevideo.avi");
  media->hasVideo(); // returns false;
}

void hasVideoChanged(bool b)
{
  // b == true
  media->hasVideo(); // returns true;
}
//! [11]


//! [12]
  connect(media, SIGNAL(hasVideoChanged(bool)), hasVideoChanged(bool));
  media->setCurrentSource("somevideo.avi");
  media->hasVideo(); // returns false;
}

void hasVideoChanged(bool b)
{
  // b == true
  media->hasVideo(); // returns true;
}
//! [12]


//! [13]
setMetaArtist(media->metaData("ARTIST"));
setMetaAlbum(media->metaData("ALBUM"));
setMetaTitle(media->metaData("TITLE"));
setMetaDate(media->metaData("DATE"));
setMetaGenre(media->metaData("GENRE"));
setMetaTrack(media->metaData("TRACKNUMBER"));
setMetaComment(media->metaData("DESCRIPTION"));
//! [13]


//! [14]
QUrl url("http://www.example.com/music.ogg");
media->setCurrentSource(url);
//! [14]


//! [15]
progressBar->setRange(0, 100); // this is the default
connect(media, SIGNAL(bufferStatus(int)), progressBar, SLOT(setValue(int)));
//! [15]


//! [16]
QObject::connect(BackendCapabilities::notifier(), SIGNAL(capabilitiesChanged()), ...
//! [16]


//! [17]
QComboBox *cb = new QComboBox(parentWidget);
ObjectDescriptionModel *model = new ObjectDescriptionModel(cb);
model->setModelData(BackendCapabilities::availableAudioOutputDevices());
cb->setModel(model);
cb->setCurrentIndex(0); // select first entry
//! [17]


//! [18]
int cbIndex = cb->currentIndex();
AudioOutputDevice selectedDevice = model->modelData(cbIndex);
//! [18]


//! [19]
Path path = Phonon::createPath(...);
Effect *effect = new Effect(this);
path.insertEffect(effect);
//! [19]


//! [20]
MediaObject *media = new MediaObject;
AudioOutput *output = new AudioOutput(Phonon::MusicCategory);
Path path = Phonon::createPath(media, output);
Q_ASSERT(path.isValid()); // for this simple case the path should always be
                          //valid - there are unit tests to ensure it
// insert an effect
QList<EffectDescription> effectList = BackendCapabilities::availableAudioEffects();
if (!effectList.isEmpty()) {
    Effect *effect = path.insertEffect(effectList.first());
}
//! [20]


//! [21]
MediaObject *media = new MediaObject(parent);
VideoWidget *vwidget = new VideoWidget(parent);
Phonon::createPath(media, vwidget);
//! [21]
