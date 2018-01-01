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

#ifndef QSCREENLINUXFB_QWS_H
#define QSCREENLINUXFB_QWS_H

#include <QtGui/qscreen_qws.h>

struct fb_cmap;
struct fb_var_screeninfo;
struct fb_fix_screeninfo;

QT_BEGIN_NAMESPACE

#ifndef QT_NO_QWS_LINUXFB

class QLinuxFb_Shared
{
 public:
   volatile int lastop;
   volatile int optype;
   volatile int fifocount;   // Accel drivers only
   volatile int fifomax;
   volatile int forecol;     // Foreground colour caching
   volatile unsigned int buffer_offset;   // Destination
   volatile int linestep;
   volatile int cliptop;    // Clip rectangle
   volatile int clipleft;
   volatile int clipright;
   volatile int clipbottom;
   volatile unsigned int rop;

};

class QLinuxFbScreenPrivate;

class Q_GUI_EXPORT QLinuxFbScreen : public QScreen
{
 public:
   explicit QLinuxFbScreen(int display_id);
   virtual ~QLinuxFbScreen();

   virtual bool initDevice();
   virtual bool connect(const QString &displaySpec);

   virtual bool useOffscreen();

   enum DriverTypes { GenericDriver, EInk8Track };

   virtual void disconnect();
   virtual void shutdownDevice();
   virtual void setMode(int, int, int);
   virtual void save();
   virtual void restore();
   virtual void blank(bool on);
   virtual void set(unsigned int, unsigned int, unsigned int, unsigned int);
   virtual uchar *cache(int);
   virtual void uncache(uchar *);
   virtual int sharedRamSize(void *);
   virtual void setDirty(const QRect &);

   QLinuxFb_Shared *shared;

 protected:

   void deleteEntry(uchar *);

   bool canaccel;
   int dataoffset;
   int cacheStart;

   virtual void fixupScreenInfo(fb_fix_screeninfo &finfo, fb_var_screeninfo &vinfo);
   static void clearCache(QScreen *instance, int);

 private:

   void delete_entry(int);
   void insert_entry(int, int, int);
   void setupOffScreen();
   void createPalette(fb_cmap &cmap, fb_var_screeninfo &vinfo, fb_fix_screeninfo &finfo);
   void setPixelFormat(struct fb_var_screeninfo);

   QLinuxFbScreenPrivate *d_ptr;
};

#endif // QT_NO_QWS_LINUXFB

QT_END_NAMESPACE


#endif // QSCREENLINUXFB_QWS_H
