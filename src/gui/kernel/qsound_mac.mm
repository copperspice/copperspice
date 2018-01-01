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

#include <qapplication.h>
#include <qsound.h>
#include <qsound_p.h>
#include <qt_mac_p.h>
#include <qhash.h>
#include <qdebug.h>

#import <AppKit/AppKit.h>

#include <AppKit/NSSound.h>

QT_BEGIN_NAMESPACE

void qt_mac_beep()
{
   NSBeep();
}

QT_END_NAMESPACE

#ifndef QT_NO_SOUND

QT_BEGIN_NAMESPACE

typedef QHash<QSound *, NSSound const *> Sounds;
static Sounds sounds;

class QAuServerMac : public QAuServer
{
   GUI_CS_OBJECT(QAuServerMac)

 public:
   QAuServerMac(QObject *parent) : QAuServer(parent) { }

   void play(const QString &filename) override;
   void play(QSound *s) override;
   void stop(QSound *) override;
   bool okay() override {
      return true;
   }

   // promote to public.
   using QAuServer::decLoop;

 protected:
   NSSound *createNSSound(const QString &filename, QSound *qSound);
};

QT_END_NAMESPACE

QT_USE_NAMESPACE

@interface QT_MANGLE_NAMESPACE(QMacSoundDelegate) : NSObject<NSSoundDelegate>
{
   QSound *qSound; // may be null.
   QAuServerMac *server;
}
-(id)initWithQSound: (QSound *)sound: (QAuServerMac *)server;
@end

@implementation QT_MANGLE_NAMESPACE(QMacSoundDelegate)
-(id)initWithQSound: (QSound *)s: (QAuServerMac *)serv
{
   self = [super init];
   if (self) {
      qSound = s;
      server = serv;
   }
   return self;
}

// Delegate function that gets called each time a sound finishes.
-(void)sound: (NSSound *)sound didFinishPlaying: (BOOL)finishedOk
{
   // qSound is null if this sound was started by play(QString),
   // in which case there is no corresponding QSound object.
   if (qSound == 0) {
      [sound release];
      [self release];
      return;
   }

   // finishedOk is false if the sound cold not be played or
   // if it was interrupted by stop().
   if (finishedOk == false) {
      sounds.remove(qSound);
      [sound release];
      [self release];
      return;
   }

   // Check if the sound should loop "forever" (until stop).
   if (qSound->loops() == -1) {
      [sound play];
      return;
   }

   const int remainingIterations = server->decLoop(qSound);
   if (remainingIterations > 0) {
      [sound play];
   } else {
      sounds.remove(qSound);
      [sound release];
      [self release];
   }
}
@end

QT_BEGIN_NAMESPACE

void QAuServerMac::play(const QString &fileName)
{
   QMacCocoaAutoReleasePool pool;
   NSSound *const nsSound = createNSSound(fileName, 0);
   [nsSound play];
}

void QAuServerMac::play(QSound *qSound)
{
   QMacCocoaAutoReleasePool pool;
   NSSound *const nsSound = createNSSound(qSound->fileName(), qSound);
   [nsSound play];
   // Keep track of the nsSound object so we can find it again in stop().
   sounds[qSound] = nsSound;
}

void QAuServerMac::stop(QSound *qSound)
{
   Sounds::const_iterator it = sounds.constFind(qSound);
   if (it != sounds.constEnd()) {
      [*it stop];
   }
}

// Creates an NSSound object and installs a "sound finished" callack delegate on it.
NSSound *QAuServerMac::createNSSound(const QString &fileName, QSound *qSound)
{
   NSString *nsFileName = const_cast<NSString *>(reinterpret_cast<const NSString *>(QCFString::toCFStringRef(fileName)));
   NSSound *const nsSound = [[NSSound alloc] initWithContentsOfFile: nsFileName byReference: YES];
   QT_MANGLE_NAMESPACE(QMacSoundDelegate) * const delegate = [[QT_MANGLE_NAMESPACE(QMacSoundDelegate) alloc]
         initWithQSound: qSound: this];
   [nsSound setDelegate: delegate];
   [nsFileName release];
   return nsSound;
}

QAuServer *qt_new_audio_server()
{
   return new QAuServerMac(qApp);
}

QT_END_NAMESPACE

#endif // QT_NO_SOUND
