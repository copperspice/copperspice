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

#include <qsound.h>

#ifndef QT_NO_SOUND

#include <qhash.h>
#include <qsocketnotifier.h>
#include <qapplication.h>
#include <qsound_p.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_NAS

QT_BEGIN_INCLUDE_NAMESPACE
#include <audio/audiolib.h>
#include <audio/soundlib.h>
QT_END_INCLUDE_NAMESPACE

static AuServer *nas = 0;

static AuBool eventPred(AuServer *, AuEvent *e, AuPointer p)
{
   if (e && (e->type == AuEventTypeElementNotify)) {
      if (e->auelementnotify.flow == *((AuFlowID *)p)) {
         return true;
      }
   }
   return false;
}

class QAuBucketNAS : public QAuBucket
{
 public:
   QAuBucketNAS(AuBucketID b, AuFlowID f = 0) : id(b), flow(f), stopped(true), numplaying(0) { }
   ~QAuBucketNAS() {
      if (nas) {
         AuSync(nas, false);
         AuDestroyBucket(nas, id, NULL);

         AuEvent ev;
         while (AuScanEvents(nas, AuEventsQueuedAfterFlush, true, eventPred, &flow, &ev))
            ;
      }
   }

   AuBucketID id;
   AuFlowID flow;
   bool     stopped;
   int      numplaying;
};

class QAuServerNAS : public QAuServer
{
   GUI_CS_OBJECT(QAuServerNAS)

   QSocketNotifier *sn;

 public:
   QAuServerNAS(QObject *parent);
   ~QAuServerNAS();

   void init(QSound *) override;
   void play(const QString &filename) override;
   void play(QSound *) override;
   void stop(QSound *) override;
   bool okay() override;
   void setDone(QSound *);

   GUI_CS_SLOT_1(Public, void dataReceived())
   GUI_CS_SLOT_2(dataReceived)

   GUI_CS_SLOT_1(Public, void soundDestroyed(QObject *o))
   GUI_CS_SLOT_2(soundDestroyed)

 private:
   QAuBucketNAS *bucket(QSound *s) {
      return (QAuBucketNAS *)QAuServer::bucket(s);
   }
};

QAuServerNAS::QAuServerNAS(QObject *parent) :
   QAuServer(parent)
{
   setObjectName(QLatin1String("Network Audio System"));
   nas = AuOpenServer(NULL, 0, NULL, 0, NULL, NULL);
   if (nas) {
      AuSetCloseDownMode(nas, AuCloseDownDestroy, NULL);
      // Ask Qt for async messages...

      sn = new QSocketNotifier(AuServerConnectionNumber(nas), QSocketNotifier::Read);
      QObject::connect(sn, SIGNAL(activated(int)), this, SLOT(dataReceived()));

   } else {
      sn = 0;
   }
}

QAuServerNAS::~QAuServerNAS()
{
   if (nas) {
      AuCloseServer(nas);
   }
   delete sn;
   nas = 0;
}

typedef QHash<void *, QAuServerNAS *> AuServerHash;
static AuServerHash *inprogress = 0;

void QAuServerNAS::soundDestroyed(QObject *o)
{
   if (inprogress) {
      QSound *so = static_cast<QSound *>(o);
      while (inprogress->remove(so))
         ; // Loop while remove returns true
   }
}

void QAuServerNAS::play(const QString &filename)
{
   if (nas) {
      int iv = 100;
      AuFixedPoint volume = AuFixedPointFromFraction(iv, 100);
      AuSoundPlayFromFile(nas, filename.toLocal8Bit().constData(), AuNone, volume,
                          NULL, NULL, NULL, NULL, NULL, NULL);
      AuFlush(nas);
      dataReceived();
      AuFlush(nas);
      qApp->flush();
   }
}

static void callback(AuServer *, AuEventHandlerRec *, AuEvent *e, AuPointer p)
{
   if (inprogress->contains(p) && e) {
      if (e->type == AuEventTypeElementNotify &&
            e->auelementnotify.kind == AuElementNotifyKindState) {
         if (e->auelementnotify.cur_state == AuStateStop) {
            AuServerHash::Iterator it = inprogress->find(p);
            if (it != inprogress->end()) {
               (*it)->setDone((QSound *)p);
            }
         }
      }
   }
}

void QAuServerNAS::setDone(QSound *s)
{
   if (nas) {
      decLoop(s);
      if (s->loopsRemaining() && !bucket(s)->stopped) {
         bucket(s)->stopped = true;
         play(s);
      } else {
         if (--(bucket(s)->numplaying) == 0) {
            bucket(s)->stopped = true;
         }
         inprogress->remove(s);
      }
   }
}

void QAuServerNAS::play(QSound *s)
{
   if (nas) {
      ++(bucket(s)->numplaying);
      if (!bucket(s)->stopped) {
         stop(s);
      }

      bucket(s)->stopped = false;
      if (!inprogress) {
         inprogress = new AuServerHash;
      }
      inprogress->insert(s, this);
      int iv = 100;
      AuFixedPoint volume = AuFixedPointFromFraction(iv, 100);
      QAuBucketNAS *b = bucket(s);
      AuSoundPlayFromBucket(nas, b->id, AuNone, volume,
                            callback, s, 0, &b->flow, NULL, NULL, NULL);
      AuFlush(nas);
      dataReceived();
      AuFlush(nas);
      qApp->flush();
   }
}

void QAuServerNAS::stop(QSound *s)
{
   if (nas && !bucket(s)->stopped) {
      bucket(s)->stopped = true;
      AuStopFlow(nas, bucket(s)->flow, NULL);
      AuFlush(nas);
      dataReceived();
      AuFlush(nas);
      qApp->flush();
   }
}

void QAuServerNAS::init(QSound *s)
{
   connect(s, SIGNAL(destroyed(QObject *)), this, SLOT(soundDestroyed(QObject *)));

   if (nas) {
      AuBucketID b_id = AuSoundCreateBucketFromFile(
                           nas, s->fileName().toLocal8Bit().constData(), 0 /*AuAccessAllMasks*/, NULL, NULL);

      setBucket(s, new QAuBucketNAS(b_id));
   }
}

bool QAuServerNAS::okay()
{
   return !!nas;
}

void QAuServerNAS::dataReceived()
{
   AuHandleEvents(nas);
}

#endif


class QAuServerNull : public QAuServer
{
 public:
   QAuServerNull(QObject *parent);

   void play(const QString &) override { }
   void play(QSound *s) override {
      while (decLoop(s) > 0) /* nothing */ ;
   }
   void stop(QSound *) override { }
   bool okay() override {
      return false;
   }
};

QAuServerNull::QAuServerNull(QObject *parent)
   : QAuServer(parent)
{
}


QAuServer *qt_new_audio_server()
{
#ifndef QT_NO_NAS
   QAuServer *s = new QAuServerNAS(qApp);
   if (s->okay()) {
      return s;
   } else {
      delete s;
   }
#endif
   return new QAuServerNull(qApp);
}

QT_END_NAMESPACE

#endif // QT_NO_SOUND
