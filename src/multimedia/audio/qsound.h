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

#ifndef QSOUND_H
#define QSOUND_H

#include <qobject.h>

class QSoundEffect;

class Q_MULTIMEDIA_EXPORT QSound : public QObject
{
   MULTI_CS_OBJECT(QSound)

 public:
   enum Loop {
      Infinite = -1
   };

   explicit QSound(const QString &filename, QObject *parent = nullptr);
   ~QSound();

   static void play(const QString &filename);

   int loops() const;
   int loopsRemaining() const;
   void setLoops(int count);
   QString fileName() const;

   bool isFinished() const;

   MULTI_CS_SLOT_1(Public, void play())
   MULTI_CS_SLOT_OVERLOAD(play, ())

   MULTI_CS_SLOT_1(Public, void stop())
   MULTI_CS_SLOT_2(stop)

 private:
   MULTI_CS_SLOT_1(Private, void deleteOnComplete())
   MULTI_CS_SLOT_2(deleteOnComplete)

   QSoundEffect *m_soundEffect;
};

#endif
