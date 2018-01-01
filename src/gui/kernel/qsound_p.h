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

#ifndef QSOUND_P_H
#define QSOUND_P_H

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SOUND

class QSound;
/*
  QAuServer is an INTERNAL class.  If you wish to provide support for
  additional audio servers, you can make a subclass of QAuServer to do
  so, HOWEVER, your class may need to be re-engineered to some degree
  with each new Qt release, including minor releases.

  QAuBucket is whatever you want.
*/

class QAuBucket
{
 public:
   virtual ~QAuBucket();
};

class QAuServer : public QObject
{
   GUI_CS_OBJECT(QAuServer)

 public:
   explicit QAuServer(QObject *parent);
   ~QAuServer();

   virtual void init(QSound *);
   virtual void play(const QString &filename);
   virtual void play(QSound *) = 0;
   virtual void stop(QSound *) = 0;
   virtual bool okay() = 0;

 protected:
   void setBucket(QSound *, QAuBucket *);
   QAuBucket *bucket(QSound *);
   int decLoop(QSound *);
};

#endif // QT_NO_SOUND

QT_END_NAMESPACE

#endif // QSOUND_P_H
