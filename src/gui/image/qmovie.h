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

#ifndef QMOVIE_H
#define QMOVIE_H

#include <qobject.h>

#ifndef QT_NO_MOVIE

#include <qlist.h>
#include <qimagereader.h>

class QColor;
class QIODevice;
class QImage;
class QPixmap;
class QRect;
class QSize;
class QMoviePrivate;

class Q_GUI_EXPORT QMovie : public QObject
{
   GUI_CS_OBJECT(QMovie)
   Q_DECLARE_PRIVATE(QMovie)

   GUI_CS_ENUM(MovieState)
   GUI_CS_ENUM(CacheMode)

   GUI_CS_PROPERTY_READ(speed, speed)
   GUI_CS_PROPERTY_WRITE(speed, setSpeed)

   GUI_CS_PROPERTY_READ(cacheMode, cacheMode)
   GUI_CS_PROPERTY_WRITE(cacheMode, setCacheMode)

 public:
   enum MovieState {
      NotRunning,
      Paused,
      Running
   };

   enum CacheMode {
      CacheNone,
      CacheAll
   };

   explicit QMovie(QObject *parent = nullptr);
   explicit QMovie(QIODevice *device, const QString &format = QString(), QObject *parent = nullptr);
   explicit QMovie(const QString &fileName, const QString &format = QString(), QObject *parent = nullptr);

   QMovie(const QMovie &) = delete;
   QMovie &operator=(const QMovie &) = delete;

   ~QMovie();

   static QList<QString> supportedFormats();

   void setDevice(QIODevice *device);
   QIODevice *device() const;

   void setFileName(const QString &fileName);
   QString fileName() const;

   void setFormat(const QString &format);
   QString format() const;

   void setBackgroundColor(const QColor &color);
   QColor backgroundColor() const;

   MovieState state() const;

   QRect frameRect() const;
   QImage currentImage() const;
   QPixmap currentPixmap() const;

   bool isValid() const;

   bool jumpToFrame(int frameNumber);
   int loopCount() const;
   int frameCount() const;
   int nextFrameDelay() const;
   int currentFrameNumber() const;

   int speed() const;

   QSize scaledSize();
   void setScaledSize(const QSize &size);

   CacheMode cacheMode() const;
   void setCacheMode(CacheMode mode);

   GUI_CS_SIGNAL_1(Public, void started())
   GUI_CS_SIGNAL_2(started)

   GUI_CS_SIGNAL_1(Public, void resized(const QSize &size))
   GUI_CS_SIGNAL_2(resized, size)

   GUI_CS_SIGNAL_1(Public, void updated(const QRect &rect))
   GUI_CS_SIGNAL_2(updated, rect)

   GUI_CS_SIGNAL_1(Public, void stateChanged(QMovie::MovieState state))
   GUI_CS_SIGNAL_2(stateChanged, state)

   GUI_CS_SIGNAL_1(Public, void error(QImageReader::ImageReaderError error))
   GUI_CS_SIGNAL_2(error, error)

   GUI_CS_SIGNAL_1(Public, void finished())
   GUI_CS_SIGNAL_2(finished)

   GUI_CS_SIGNAL_1(Public, void frameChanged(int frameNumber))
   GUI_CS_SIGNAL_2(frameChanged, frameNumber)

   GUI_CS_SLOT_1(Public, void start())
   GUI_CS_SLOT_2(start)

   GUI_CS_SLOT_1(Public, bool jumpToNextFrame())
   GUI_CS_SLOT_2(jumpToNextFrame)

   GUI_CS_SLOT_1(Public, void setPaused(bool paused))
   GUI_CS_SLOT_2(setPaused)

   GUI_CS_SLOT_1(Public, void stop())
   GUI_CS_SLOT_2(stop)

   GUI_CS_SLOT_1(Public, void setSpeed(int percentSpeed))
   GUI_CS_SLOT_2(setSpeed)

 protected:
   QScopedPointer<QMoviePrivate> d_ptr;

 private:
   GUI_CS_SLOT_1(Private, void _q_loadNextFrame())
   GUI_CS_SLOT_2(_q_loadNextFrame)
};

#endif // QT_NO_MOVIE

#endif
