/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

#ifndef QDECLARATIVEANIMATEDIMAGE_P_H
#define QDECLARATIVEANIMATEDIMAGE_P_H

#include <qdeclarativeimage_p.h>

#ifndef QT_NO_MOVIE

QT_BEGIN_NAMESPACE

class QMovie;
class QDeclarativeAnimatedImagePrivate;

class QDeclarativeAnimatedImage : public QDeclarativeImage
{
   DECL_CS_OBJECT(QDeclarativeAnimatedImage)

   DECL_CS_PROPERTY_READ(playing, isPlaying)
   DECL_CS_PROPERTY_WRITE(playing, setPlaying)
   DECL_CS_PROPERTY_NOTIFY(playing, playingChanged)
   DECL_CS_PROPERTY_READ(paused, isPaused)
   DECL_CS_PROPERTY_WRITE(paused, setPaused)
   DECL_CS_PROPERTY_NOTIFY(paused, pausedChanged)
   DECL_CS_PROPERTY_READ(currentFrame, currentFrame)
   DECL_CS_PROPERTY_WRITE(currentFrame, setCurrentFrame)
   DECL_CS_PROPERTY_NOTIFY(currentFrame, frameChanged)
   DECL_CS_PROPERTY_READ(frameCount, frameCount)

   // read-only for AnimatedImage
   DECL_CS_PROPERTY_READ(sourceSize, sourceSize)
   DECL_CS_PROPERTY_NOTIFY(sourceSize, sourceSizeChanged)

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
   DECL_CS_SIGNAL_1(Public, void playingChanged())
   DECL_CS_SIGNAL_2(playingChanged)
   DECL_CS_SIGNAL_1(Public, void pausedChanged())
   DECL_CS_SIGNAL_2(pausedChanged)
   DECL_CS_SIGNAL_1(Public, void frameChanged())
   DECL_CS_SIGNAL_2(frameChanged)
   DECL_CS_SIGNAL_1(Public, void sourceSizeChanged())
   DECL_CS_SIGNAL_2(sourceSizeChanged)

 private :
   DECL_CS_SLOT_1(Private, void movieUpdate())
   DECL_CS_SLOT_2(movieUpdate)
   DECL_CS_SLOT_1(Private, void movieRequestFinished())
   DECL_CS_SLOT_2(movieRequestFinished)
   DECL_CS_SLOT_1(Private, void playingStatusChanged())
   DECL_CS_SLOT_2(playingStatusChanged)

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
