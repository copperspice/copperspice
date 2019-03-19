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

#ifndef QWINDOWSURFACE_QWS_P_H
#define QWINDOWSURFACE_QWS_P_H

#include <qwindowsurface_p.h>
#include <qregion.h>
#include <qimage.h>
#include <qdirectpainter_qws.h>
#include <qmutex.h>
#include <qwssharedmemory_p.h>

QT_BEGIN_NAMESPACE

class QScreen;
class QWSWindowSurfacePrivate;

class Q_GUI_EXPORT QWSWindowSurface : public QWindowSurface
{
 public:
   QWSWindowSurface();
   QWSWindowSurface(QWidget *widget);
   ~QWSWindowSurface();

   virtual bool isValid() const = 0;

   virtual void setGeometry(const QRect &rect);
   virtual void setGeometry(const QRect &rect, const QRegion &mask);
   virtual void flush(QWidget *widget, const QRegion &region, const QPoint &offset);

   virtual bool move(const QPoint &offset);
   virtual QRegion move(const QPoint &offset, const QRegion &newClip);

   virtual QPoint painterOffset() const; // remove!!!

   virtual void beginPaint(const QRegion &);
   virtual void endPaint(const QRegion &);

   virtual bool lock(int timeout = -1);
   virtual void unlock();

   virtual QString key() const = 0;

   // XXX: not good enough
   virtual QByteArray transientState() const;
   virtual QByteArray permanentState() const;
   virtual void setTransientState(const QByteArray &state);
   virtual void setPermanentState(const QByteArray &state);

   virtual QImage image() const = 0;
   virtual QPaintDevice *paintDevice() = 0;

   const QRegion clipRegion() const;
   void setClipRegion(const QRegion &);

#ifdef QT_QWS_CLIENTBLIT
   virtual const QRegion directRegion() const;
   virtual int directRegionId() const;
   virtual void setDirectRegion(const QRegion &, int);
#endif

   enum SurfaceFlag {
      RegionReserved = 0x1,
      Buffered = 0x2,
      Opaque = 0x4
   };
   using SurfaceFlags = QFlags<SurfaceFlag>;

   SurfaceFlags surfaceFlags() const;

   inline bool isRegionReserved() const {
      return surfaceFlags() & RegionReserved;
   }
   inline bool isBuffered() const {
      return surfaceFlags() & Buffered;
   }
   inline bool isOpaque() const {
      return surfaceFlags() & Opaque;
   }

   int winId() const;
   virtual void releaseSurface();

 protected:
   void setSurfaceFlags(SurfaceFlags type);
   void setWinId(int id);

 private:
   friend class QWidgetPrivate;

   void invalidateBuffer();

   QWSWindowSurfacePrivate *d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWSWindowSurface::SurfaceFlags)

class QWSWindowSurfacePrivate
{
 public:
   QWSWindowSurfacePrivate();

   void setWinId(int id);

   QWSWindowSurface::SurfaceFlags flags;
   QRegion clip;
#ifdef QT_QWS_CLIENTBLIT
   QRegion direct;
   int directId;
#endif

   int winId;
};

class QWSLock;

class Q_GUI_EXPORT QWSMemorySurface : public QWSWindowSurface
{
 public:
   QWSMemorySurface();
   QWSMemorySurface(QWidget *widget);
   ~QWSMemorySurface();

   bool isValid() const;

   QPaintDevice *paintDevice() {
      return &img;
   }
   bool scroll(const QRegion &area, int dx, int dy);

   QImage image() const {
      return img;
   }
   QPoint painterOffset() const;

   void beginPaint(const QRegion &rgn);

   bool lock(int timeout = -1);
   void unlock();

 protected:
   QImage::Format preferredImageFormat(const QWidget *widget) const;

#ifndef QT_NO_QWS_MULTIPROCESS
   void setLock(int lockId);
   QWSLock *memlock;
#endif

   QMutex threadLock;
   QImage img;
};

class Q_GUI_EXPORT QWSLocalMemSurface : public QWSMemorySurface
{
 public:
   QWSLocalMemSurface();
   QWSLocalMemSurface(QWidget *widget);
   ~QWSLocalMemSurface();

   void setGeometry(const QRect &rect);

   QString key() const {
      return QLatin1String("mem");
   }
   QByteArray permanentState() const;

   void setPermanentState(const QByteArray &data);
   virtual void releaseSurface();

 protected:
   uchar *mem;
   int memsize;
};

#ifndef QT_NO_QWS_MULTIPROCESS
class Q_GUI_EXPORT QWSSharedMemSurface : public QWSMemorySurface
{
 public:
   QWSSharedMemSurface();
   QWSSharedMemSurface(QWidget *widget);
   ~QWSSharedMemSurface();

   void setGeometry(const QRect &rect);

   QString key() const {
      return QLatin1String("shm");
   }
   QByteArray permanentState() const;

   void setPermanentState(const QByteArray &data);

#ifdef QT_QWS_CLIENTBLIT
   virtual void setDirectRegion(const QRegion &, int);
   virtual const QRegion directRegion() const;
#endif
   virtual void releaseSurface();

 private:
   bool setMemory(int memId);

   QWSSharedMemory mem;
};
#endif // QT_NO_QWS_MULTIPROCESS

#ifndef QT_NO_PAINTONSCREEN
class Q_GUI_EXPORT QWSOnScreenSurface : public QWSMemorySurface
{
 public:
   QWSOnScreenSurface();
   QWSOnScreenSurface(QWidget *widget);
   ~QWSOnScreenSurface();

   bool isValid() const;
   QPoint painterOffset() const;

   QString key() const {
      return QLatin1String("OnScreen");
   }
   QByteArray permanentState() const;

   void setPermanentState(const QByteArray &data);

 private:
   void attachToScreen(const QScreen *screen);

   const QScreen *screen;
};
#endif // QT_NO_PAINTONSCREEN

#ifndef QT_NO_PAINT_DEBUG
class Q_GUI_EXPORT QWSYellowSurface : public QWSWindowSurface
{
 public:
   QWSYellowSurface(bool isClient = false);
   ~QWSYellowSurface();

   void setDelay(int msec) {
      delay = msec;
   }

   bool isValid() const {
      return true;
   }

   void flush(QWidget *widget, const QRegion &region, const QPoint &offset);

   QString key() const {
      return QLatin1String("Yellow");
   }
   QByteArray permanentState() const;

   void setPermanentState(const QByteArray &data);

   QPaintDevice *paintDevice() {
      return &img;
   }
   QImage image() const {
      return img;
   }

 private:
   int delay;
   QSize surfaceSize; // client side
   QImage img; // server side
};
#endif // QT_NO_PAINT_DEBUG

#ifndef QT_NO_DIRECTPAINTER

class QScreen;

class Q_GUI_EXPORT QWSDirectPainterSurface : public QWSWindowSurface
{
 public:
   QWSDirectPainterSurface(bool isClient = false, QDirectPainter::SurfaceFlag flags = QDirectPainter::NonReserved);
   ~QWSDirectPainterSurface();

   void setReserved() {
      setSurfaceFlags(RegionReserved);
   }

   void setGeometry(const QRect &rect) {
      setRegion(rect);
   }

   void setRegion(const QRegion &region);
   QRegion region() const {
      return clipRegion();
   }

   void flush(QWidget *, const QRegion &, const QPoint &);

   bool isValid() const {
      return false;
   }

   QString key() const {
      return QLatin1String("DirectPainter");
   }
   QByteArray permanentState() const;

   void setPermanentState(const QByteArray &);

   QImage image() const {
      return QImage();
   }
   QPaintDevice *paintDevice() {
      return 0;
   }

   // hw: get rid of this
   WId windowId() const {
      return static_cast<WId>(winId());
   }

   QScreen *screen() const {
      return _screen;
   }

   void beginPaint(const QRegion &);
   bool lock(int timeout = -1);
   void unlock();

   void setLocking(bool b) {
      doLocking = b;
   }

   bool hasPendingRegionEvents() const;

 private:
   QScreen *_screen;
   QMutex threadLock;

   friend void qt_directpainter_region(QDirectPainter *, const QRegion &, int);
   bool flushingRegionEvents;
   bool synchronous;
   bool doLocking;
};

#endif // QT_NO_DIRECTPAINTER

QT_END_NAMESPACE

#endif // QWINDOWSURFACE_QWS_P_H
