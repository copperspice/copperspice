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

#ifndef QDECLARATIVEANIMATEDIMAGE_P_H
#define QDECLARATIVEANIMATEDIMAGE_P_H

#include <qdeclarativeimage_p.h>

#ifndef QT_NO_MOVIE

QT_BEGIN_NAMESPACE

class QMovie;
class QDeclarativeAnimatedImagePrivate;

class QDeclarativeAnimatedImage : public QDeclarativeImage
{
   CS_OBJECT(QDeclarativeAnimatedImage)

   CS_PROPERTY_READ(playing, isPlaying)
   CS_PROPERTY_WRITE(playing, setPlaying)
   CS_PROPERTY_NOTIFY(playing, playingChanged)
   CS_PROPERTY_READ(paused, isPaused)
   CS_PROPERTY_WRITE(paused, setPaused)
   CS_PROPERTY_NOTIFY(paused, pausedChanged)
   CS_PROPERTY_READ(currentFrame, currentFrame)
   CS_PROPERTY_WRITE(currentFrame, setCurrentFrame)
   CS_PROPERTY_NOTIFY(currentFrame, frameChanged)
   CS_PROPERTY_READ(frameCount, frameCount)

   // read-only for AnimatedImage
   CS_PROPERTY_READ(sourceSize, sourceSize)
   CS_PROPERTY_NOTIFY(sourceSize, sourceSizeChanged)

 public:
   QDeclarativeAnimatedImage(QDeclarativeItem *parent = 0);
   ~QDeclarativeAnimatedImage();

   bool isPlaying() const;
   void setPlaying(bool play);

   bool isPaused() const;
   void setPaused(bool pause);

   int currentFrame() const;
   void setCurrentFrame(int frame);

   int frameCount() const;

   // Extends QDeclarativeImage's src property*/
   virtual void setSource(const QUrl &);

 public:
   CS_SIGNAL_1(Public, void playingChanged())
   CS_SIGNAL_2(playingChanged)
   CS_SIGNAL_1(Public, void pausedChanged())
   CS_SIGNAL_2(pausedChanged)
   CS_SIGNAL_1(Public, void frameChanged())
   CS_SIGNAL_2(frameChanged)
   CS_SIGNAL_1(Public, void sourceSizeChanged())
   CS_SIGNAL_2(sourceSizeChanged)

 private :
   CS_SLOT_1(Private, void movieUpdate())
   CS_SLOT_2(movieUpdate)
   CS_SLOT_1(Private, void movieRequestFinished())
   CS_SLOT_2(movieRequestFinished)
   CS_SLOT_1(Private, void playingStatusChanged())
   CS_SLOT_2(playingStatusChanged)

 protected:
   virtual void load();
   void componentComplete();

 private:
   Q_DISABLE_COPY(QDeclarativeAnimatedImage)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeAnimatedImage)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeAnimatedImage)

#endif // QT_NO_MOVIE

#endif
