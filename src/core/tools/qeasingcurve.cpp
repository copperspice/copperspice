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

#include <qeasingcurve.h>

#include <qdatastream.h>
#include <qdebug.h>
#include <qstring.h>

#include "../../3rdparty/easing/easing.cpp"

static bool isConfigFunction(QEasingCurve::Type type)
{
   return type >= QEasingCurve::InElastic
         && type <= QEasingCurve::OutInBounce;
}

class QEasingCurveFunction
{
 public:
   enum Type {
      In,
      Out,
      InOut,
      OutIn
   };

   QEasingCurveFunction(QEasingCurveFunction::Type type = In, qreal period = 0.3, qreal amplitude = 1.0,
         qreal overshoot = 1.70158)
      : _t(type), _p(period), _a(amplitude), _o(overshoot)
   { }

   virtual ~QEasingCurveFunction()
   { }

   virtual qreal value(qreal t);
   virtual QEasingCurveFunction *copy() const;
   bool operator==(const QEasingCurveFunction &other);

   Type _t;
   qreal _p;
   qreal _a;
   qreal _o;
};

qreal QEasingCurveFunction::value(qreal t)
{
   return t;
}

QEasingCurveFunction *QEasingCurveFunction::copy() const
{
   return new QEasingCurveFunction(_t, _p, _a, _o);
}

bool QEasingCurveFunction::operator==(const QEasingCurveFunction &other)
{
   return _t == other._t &&
         qFuzzyCompare(_p, other._p) &&
         qFuzzyCompare(_a, other._a) &&
         qFuzzyCompare(_o, other._o);
}

class QEasingCurvePrivate
{
 public:
   QEasingCurvePrivate()
      : type(QEasingCurve::Linear),
        config(nullptr),
        func(&easeNone)
   { }
   ~QEasingCurvePrivate()
   {
      delete config;
   }

   void setType_helper(QEasingCurve::Type);

   QEasingCurve::Type type;
   QEasingCurveFunction *config;
   QEasingCurve::EasingFunction func;
};

struct ElasticEase : public QEasingCurveFunction {
   ElasticEase(Type type)
      : QEasingCurveFunction(type, qreal(0.3), qreal(1.0))
   { }

   QEasingCurveFunction *copy() const override {
      ElasticEase *rv = new ElasticEase(_t);
      rv->_p = _p;
      rv->_a = _a;
      return rv;
   }

   qreal value(qreal t) override {
      qreal p = (_p < 0) ? qreal(0.3) : _p;
      qreal a = (_a < 0) ? qreal(1.0) : _a;

      switch (_t) {
         case In:
            return easeInElastic(t, a, p);

         case Out:
            return easeOutElastic(t, a, p);

         case InOut:
            return easeInOutElastic(t, a, p);

         case OutIn:
            return easeOutInElastic(t, a, p);

         default:
            return t;
      }
   }
};

struct BounceEase : public QEasingCurveFunction {
   BounceEase(Type type)
      : QEasingCurveFunction(type, qreal(0.3), qreal(1.0)) {
   }

   QEasingCurveFunction *copy() const override {
      BounceEase *rv = new BounceEase(_t);
      rv->_a = _a;
      return rv;
   }

   qreal value(qreal t) override {
      qreal a = (_a < 0) ? qreal(1.0) : _a;

      switch (_t) {
         case In:
            return easeInBounce(t, a);

         case Out:
            return easeOutBounce(t, a);

         case InOut:
            return easeInOutBounce(t, a);

         case OutIn:
            return easeOutInBounce(t, a);

         default:
            return t;
      }
   }
};

struct BackEase : public QEasingCurveFunction {
   BackEase(Type type)
      : QEasingCurveFunction(type, qreal(0.3), qreal(1.0), qreal(1.70158)) {
   }

   QEasingCurveFunction *copy() const override {
      BackEase *rv = new BackEase(_t);
      rv->_o = _o;
      return rv;
   }

   qreal value(qreal t) override {
      qreal o = (_o < 0) ? qreal(1.70158) : _o;

      switch (_t) {
         case In:
            return easeInBack(t, o);

         case Out:
            return easeOutBack(t, o);

         case InOut:
            return easeInOutBack(t, o);

         case OutIn:
            return easeOutInBack(t, o);

         default:
            return t;
      }
   }
};

static QEasingCurve::EasingFunction curveToFunc(QEasingCurve::Type curve)
{
   switch (curve) {
      case QEasingCurve::Linear:
         return &easeNone;

      case QEasingCurve::InQuad:
         return &easeInQuad;

      case QEasingCurve::OutQuad:
         return &easeOutQuad;

      case QEasingCurve::InOutQuad:
         return &easeInOutQuad;

      case QEasingCurve::OutInQuad:
         return &easeOutInQuad;

      case QEasingCurve::InCubic:
         return &easeInCubic;

      case QEasingCurve::OutCubic:
         return &easeOutCubic;

      case QEasingCurve::InOutCubic:
         return &easeInOutCubic;

      case QEasingCurve::OutInCubic:
         return &easeOutInCubic;

      case QEasingCurve::InQuart:
         return &easeInQuart;

      case QEasingCurve::OutQuart:
         return &easeOutQuart;

      case QEasingCurve::InOutQuart:
         return &easeInOutQuart;

      case QEasingCurve::OutInQuart:
         return &easeOutInQuart;

      case QEasingCurve::InQuint:
         return &easeInQuint;

      case QEasingCurve::OutQuint:
         return &easeOutQuint;

      case QEasingCurve::InOutQuint:
         return &easeInOutQuint;

      case QEasingCurve::OutInQuint:
         return &easeOutInQuint;

      case QEasingCurve::InSine:
         return &easeInSine;

      case QEasingCurve::OutSine:
         return &easeOutSine;

      case QEasingCurve::InOutSine:
         return &easeInOutSine;

      case QEasingCurve::OutInSine:
         return &easeOutInSine;

      case QEasingCurve::InExpo:
         return &easeInExpo;

      case QEasingCurve::OutExpo:
         return &easeOutExpo;

      case QEasingCurve::InOutExpo:
         return &easeInOutExpo;

      case QEasingCurve::OutInExpo:
         return &easeOutInExpo;

      case QEasingCurve::InCirc:
         return &easeInCirc;

      case QEasingCurve::OutCirc:
         return &easeOutCirc;

      case QEasingCurve::InOutCirc:
         return &easeInOutCirc;

      case QEasingCurve::OutInCirc:
         return &easeOutInCirc;

      // Internal for, compatibility with QTimeLine only ??
      case QEasingCurve::InCurve:
         return &easeInCurve;

      case QEasingCurve::OutCurve:
         return &easeOutCurve;

      case QEasingCurve::SineCurve:
         return &easeSineCurve;

      case QEasingCurve::CosineCurve:
         return &easeCosineCurve;

      default:
         return nullptr;
   };
}

static QEasingCurveFunction *curveToFunctionObject(QEasingCurve::Type type)
{
   QEasingCurveFunction *curveFunc = nullptr;

   switch (type) {
      case QEasingCurve::InElastic:
         curveFunc = new ElasticEase(ElasticEase::In);
         break;

      case QEasingCurve::OutElastic:
         curveFunc = new ElasticEase(ElasticEase::Out);
         break;

      case QEasingCurve::InOutElastic:
         curveFunc = new ElasticEase(ElasticEase::InOut);
         break;

      case QEasingCurve::OutInElastic:
         curveFunc = new ElasticEase(ElasticEase::OutIn);
         break;

      case QEasingCurve::OutBounce:
         curveFunc = new BounceEase(BounceEase::Out);
         break;

      case QEasingCurve::InBounce:
         curveFunc = new BounceEase(BounceEase::In);
         break;

      case QEasingCurve::OutInBounce:
         curveFunc = new BounceEase(BounceEase::OutIn);
         break;

      case QEasingCurve::InOutBounce:
         curveFunc = new BounceEase(BounceEase::InOut);
         break;

      case QEasingCurve::InBack:
         curveFunc = new BackEase(BackEase::In);
         break;

      case QEasingCurve::OutBack:
         curveFunc = new BackEase(BackEase::Out);
         break;

      case QEasingCurve::InOutBack:
         curveFunc = new BackEase(BackEase::InOut);
         break;

      case QEasingCurve::OutInBack:
         curveFunc = new BackEase(BackEase::OutIn);
         break;

      default:
         curveFunc = new QEasingCurveFunction(QEasingCurveFunction::In, qreal(0.3), qreal(1.0), qreal(1.70158));
   }

   return curveFunc;
}

QEasingCurve::QEasingCurve(Type type)
   : d_ptr(new QEasingCurvePrivate)
{
   setType(type);
}

QEasingCurve::QEasingCurve(const QEasingCurve &other)
   : d_ptr(new QEasingCurvePrivate)
{
   // ### non-atomic, requires malloc on shallow copy
   *d_ptr = *other.d_ptr;

   if (other.d_ptr->config) {
      d_ptr->config = other.d_ptr->config->copy();
   }
}

QEasingCurve::~QEasingCurve()
{
   delete d_ptr;
}

QEasingCurve &QEasingCurve::operator=(const QEasingCurve &other)
{
   // ### non-atomic, requires malloc on shallow copy
   if (d_ptr->config) {
      delete d_ptr->config;
      d_ptr->config = nullptr;
   }

   *d_ptr = *other.d_ptr;

   if (other.d_ptr->config) {
      d_ptr->config = other.d_ptr->config->copy();
   }

   return *this;
}

bool QEasingCurve::operator==(const QEasingCurve &other) const
{
   bool res = d_ptr->func == other.d_ptr->func
         && d_ptr->type == other.d_ptr->type;

   if (res) {
      if (d_ptr->config && other.d_ptr->config) {
         // catch the config content
         res = d_ptr->config->operator==(*(other.d_ptr->config));

      } else if (d_ptr->config || other.d_ptr->config) {
         // one one has a config object, which could contain default values
         res = qFuzzyCompare(amplitude(), other.amplitude()) &&
               qFuzzyCompare(period(), other.period()) &&
               qFuzzyCompare(overshoot(), other.overshoot());
      }
   }

   return res;
}

qreal QEasingCurve::amplitude() const
{
   return d_ptr->config ? d_ptr->config->_a : qreal(1.0);
}

void QEasingCurve::setAmplitude(qreal amplitude)
{
   if (! d_ptr->config) {
      d_ptr->config = curveToFunctionObject(d_ptr->type);
   }

   d_ptr->config->_a = amplitude;
}

qreal QEasingCurve::period() const
{
   return d_ptr->config ? d_ptr->config->_p : qreal(0.3);
}

void QEasingCurve::setPeriod(qreal period)
{
   if (! d_ptr->config) {
      d_ptr->config = curveToFunctionObject(d_ptr->type);
   }

   d_ptr->config->_p = period;
}
qreal QEasingCurve::overshoot() const
{
   return d_ptr->config ? d_ptr->config->_o : qreal(1.70158) ;
}

void QEasingCurve::setOvershoot(qreal overshoot)
{
   if (!d_ptr->config) {
      d_ptr->config = curveToFunctionObject(d_ptr->type);
   }

   d_ptr->config->_o = overshoot;
}

QEasingCurve::Type QEasingCurve::type() const
{
   return d_ptr->type;
}

void QEasingCurvePrivate::setType_helper(QEasingCurve::Type newType)
{
   qreal amp = -1.0;
   qreal period = -1.0;
   qreal overshoot = -1.0;

   if (config) {
      amp = config->_a;
      period = config->_p;
      overshoot = config->_o;
      delete config;
      config = nullptr;
   }

   if (isConfigFunction(newType) || (amp != -1.0) || (period != -1.0) || (overshoot != -1.0)) {
      config = curveToFunctionObject(newType);

      if (amp != -1.0) {
         config->_a = amp;
      }

      if (period != -1.0) {
         config->_p = period;
      }

      if (overshoot != -1.0) {
         config->_o = overshoot;
      }

      func = nullptr;
   } else if (newType != QEasingCurve::Custom) {
      func = curveToFunc(newType);
   }

   Q_ASSERT((func == nullptr) == (config != nullptr));
   type = newType;
}

void QEasingCurve::setType(Type type)
{
   if (d_ptr->type == type) {
      return;
   }

   if (type < Linear || type >= NCurveTypes - 1) {
      qWarning("QEasingCurve::setType() Invalid curve type %d", type);
      return;
   }

   d_ptr->setType_helper(type);
}

void QEasingCurve::setCustomType(EasingFunction func)
{
   if (!func) {
      qWarning("QEasingCurve::setCustomType() Invalid callback (nullptr)");
      return;
   }

   d_ptr->func = func;
   d_ptr->setType_helper(Custom);
}

QEasingCurve::EasingFunction QEasingCurve::customType() const
{
   return d_ptr->type == Custom ? d_ptr->func : nullptr;
}

qreal QEasingCurve::valueForProgress(qreal progress) const
{
   progress = qBound(0, progress, 1);

   if (d_ptr->func) {
      return d_ptr->func(progress);

   } else if (d_ptr->config) {
      return d_ptr->config->value(progress);

   } else {
      return progress;

   }
}

QDebug operator<<(QDebug debug, const QEasingCurve &item)
{
   debug << "type:" << item.d_ptr->type
         << "func:" << item.d_ptr->func;

   if (item.d_ptr->config) {
      debug << QString::fromLatin1("period:%1").formatArg(item.d_ptr->config->_p, 0, 'f', 20)
            << QString::fromLatin1("amp:%1").formatArg(item.d_ptr->config->_a, 0, 'f', 20)
            << QString::fromLatin1("overshoot:%1").formatArg(item.d_ptr->config->_o, 0, 'f', 20);
   }

   return debug;
}

QDataStream &operator<<(QDataStream &stream, const QEasingCurve &easing)
{
   stream << quint8(easing.d_ptr->type);
   stream << quint64(quintptr(easing.d_ptr->func));

   bool hasConfig = easing.d_ptr->config;
   stream << hasConfig;

   if (hasConfig) {
      stream << easing.d_ptr->config->_p;
      stream << easing.d_ptr->config->_a;
      stream << easing.d_ptr->config->_o;
   }

   return stream;
}

QDataStream &operator>>(QDataStream &stream, QEasingCurve &easing)
{
   QEasingCurve::Type type;
   quint8 int_type;
   stream >> int_type;
   type = static_cast<QEasingCurve::Type>(int_type);
   easing.setType(type);

   quint64 ptr_func;
   stream >> ptr_func;
   easing.d_ptr->func = QEasingCurve::EasingFunction(quintptr(ptr_func));

   bool hasConfig;
   stream >> hasConfig;

   if (hasConfig) {
      QEasingCurveFunction *config = curveToFunctionObject(type);
      stream >> config->_p;
      stream >> config->_a;
      stream >> config->_o;
      easing.d_ptr->config = config;
   }

   return stream;
}
