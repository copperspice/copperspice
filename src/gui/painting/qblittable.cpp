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

#include "qblittable_p.h"

#ifndef QT_NO_BLITTABLE
QT_BEGIN_NAMESPACE

class QBlittablePrivate
{
 public:
   QBlittablePrivate(const QSize &size, QBlittable::Capabilities caps)
      : caps(caps), m_size(size), locked(false), cachedImg(0) {
   }
   QBlittable::Capabilities caps;
   QSize m_size;
   bool locked;
   QImage *cachedImg;
};


QBlittable::QBlittable(const QSize &size, Capabilities caps)
   : d_ptr(new QBlittablePrivate(size, caps))
{
}

QBlittable::~QBlittable()
{
   delete d_ptr;
}


QBlittable::Capabilities QBlittable::capabilities() const
{
   Q_D(const QBlittable);
   return d->caps;
}

QSize QBlittable::size() const
{
   Q_D(const QBlittable);
   return d->m_size;
}

bool QBlittable::isLocked() const
{
   Q_D(const QBlittable);
   return d->locked;
}

QImage *QBlittable::lock()
{
   Q_D(QBlittable);
   if (!d->locked) {
      d->cachedImg = doLock();
      d->locked = true;
   }

   return d->cachedImg;
}

void QBlittable::unlock()
{
   Q_D(QBlittable);
   if (d->locked) {
      doUnlock();
      d->locked = false;
   }
}

QT_END_NAMESPACE
#endif //QT_NO_BLITTABLE

