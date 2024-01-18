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

#ifndef QDECLARATIVETIMELINE_P_P_H
#define QDECLARATIVETIMELINE_P_P_H

#include <QtCore/QObject>
#include <QtCore/QAbstractAnimation>

QT_BEGIN_NAMESPACE

class QEasingCurve;
class QDeclarativeTimeLineValue;
class QDeclarativeTimeLineCallback;
struct QDeclarativeTimeLinePrivate;
class QDeclarativeTimeLineObject;

class QDeclarativeTimeLine : public QAbstractAnimation
{
   DECL_CS_OBJECT(QDeclarativeTimeLine)

 public:
   QDeclarativeTimeLine(QObject *parent = nullptr);
   ~QDeclarativeTimeLine();

   enum SyncMode { LocalSync, GlobalSync };
   SyncMode syncMode() const;
   void setSyncMode(SyncMode);

   void pause(QDeclarativeTimeLineObject &, int);
   void callback(const QDeclarativeTimeLineCallback &);
   void set(QDeclarativeTimeLineValue &, qreal);

   int accel(QDeclarativeTimeLineValue &, qreal velocity, qreal accel);
   int accel(QDeclarativeTimeLineValue &, qreal velocity, qreal accel, qreal maxDistance);
   int accelDistance(QDeclarativeTimeLineValue &, qreal velocity, qreal distance);

   void move(QDeclarativeTimeLineValue &, qreal destination, int time = 500);
   void move(QDeclarativeTimeLineValue &, qreal destination, const QEasingCurve &, int time = 500);
   void moveBy(QDeclarativeTimeLineValue &, qreal change, int time = 500);
   void moveBy(QDeclarativeTimeLineValue &, qreal change, const QEasingCurve &, int time = 500);

   void sync();
   void setSyncPoint(int);
   int syncPoint() const;

   void sync(QDeclarativeTimeLineValue &);
   void sync(QDeclarativeTimeLineValue &, QDeclarativeTimeLineValue &);

   void reset(QDeclarativeTimeLineValue &);

   void complete();
   void clear();
   bool isActive() const;

   int time() const;

   virtual int duration() const;
 public:
   DECL_CS_SIGNAL_1(Public, void updated())
   DECL_CS_SIGNAL_2(updated)
   DECL_CS_SIGNAL_1(Public, void completed())
   DECL_CS_SIGNAL_2(completed)

 protected:
   virtual void updateCurrentTime(int);

 private:
   void remove(QDeclarativeTimeLineObject *);
   friend class QDeclarativeTimeLineObject;
   friend struct QDeclarativeTimeLinePrivate;
   QDeclarativeTimeLinePrivate *d;
};

class QDeclarativeTimeLineObject
{
 public:
   QDeclarativeTimeLineObject();
   virtual ~QDeclarativeTimeLineObject();

 protected:
   friend class QDeclarativeTimeLine;
   friend struct QDeclarativeTimeLinePrivate;
   QDeclarativeTimeLine *_t;
};

class QDeclarativeTimeLineValue : public QDeclarativeTimeLineObject
{
 public:
   QDeclarativeTimeLineValue(qreal v = 0.) : _v(v) {}

   virtual qreal value() const {
      return _v;
   }
   virtual void setValue(qreal v) {
      _v = v;
   }

   QDeclarativeTimeLine *timeLine() const {
      return _t;
   }

   operator qreal() const {
      return _v;
   }
   QDeclarativeTimeLineValue &operator=(qreal v) {
      setValue(v);
      return *this;
   }
 private:
   friend class QDeclarativeTimeLine;
   friend struct QDeclarativeTimeLinePrivate;
   qreal _v;
};

class QDeclarativeTimeLineCallback
{
 public:
   typedef void (*Callback)(void *);

   QDeclarativeTimeLineCallback();
   QDeclarativeTimeLineCallback(QDeclarativeTimeLineObject *b, Callback, void * = 0);
   QDeclarativeTimeLineCallback(const QDeclarativeTimeLineCallback &o);

   QDeclarativeTimeLineCallback &operator=(const QDeclarativeTimeLineCallback &o);
   QDeclarativeTimeLineObject *callbackObject() const;

 private:
   friend struct QDeclarativeTimeLinePrivate;
   Callback d0;
   void *d1;
   QDeclarativeTimeLineObject *d2;
};

template<class T>
class QDeclarativeTimeLineValueProxy : public QDeclarativeTimeLineValue
{
 public:
   QDeclarativeTimeLineValueProxy(T *cls, void (T::*func)(qreal), qreal v = 0.)
      : QDeclarativeTimeLineValue(v), _class(cls), _setFunctionReal(func), _setFunctionInt(0) {
      Q_ASSERT(_class);
   }

   QDeclarativeTimeLineValueProxy(T *cls, void (T::*func)(int), qreal v = 0.)
      : QDeclarativeTimeLineValue(v), _class(cls), _setFunctionReal(0), _setFunctionInt(func) {
      Q_ASSERT(_class);
   }

   virtual void setValue(qreal v) {
      QDeclarativeTimeLineValue::setValue(v);
      if (_setFunctionReal) {
         (_class->*_setFunctionReal)(v);
      } else if (_setFunctionInt) {
         (_class->*_setFunctionInt)((int)v);
      }
   }

 private:
   T *_class;
   void (T::*_setFunctionReal)(qreal);
   void (T::*_setFunctionInt)(int);
};

QT_END_NAMESPACE

#endif
