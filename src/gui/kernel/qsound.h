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

#ifndef QSOUND_H
#define QSOUND_H

#include <QtCore/qobject.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SOUND

class QSoundPrivate;

class Q_GUI_EXPORT QSound : public QObject
{
   GUI_CS_OBJECT(QSound)

 public:
   static bool isAvailable();
   static void play(const QString &filename);

   explicit QSound(const QString &filename, QObject *parent = nullptr);
   ~QSound();

   int loops() const;
   int loopsRemaining() const;
   void setLoops(int);
   QString fileName() const;
   bool isFinished() const;

   GUI_CS_SLOT_1(Public, void play())
   GUI_CS_SLOT_OVERLOAD(play, ())

   GUI_CS_SLOT_1(Public, void stop())
   GUI_CS_SLOT_2(stop)

 protected:
   QScopedPointer<QSoundPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QSound)
   friend class QAuServer;
 
};

#endif // QT_NO_SOUND

QT_END_NAMESPACE

#endif // QSOUND_H
