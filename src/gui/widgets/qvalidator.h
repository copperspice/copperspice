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

#ifndef QVALIDATOR_H
#define QVALIDATOR_H

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qregexp.h>
#include <QtCore/qlocale.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_VALIDATOR

class QValidatorPrivate;

class Q_GUI_EXPORT QValidator : public QObject
{
   GUI_CS_OBJECT(QValidator)

 public:
   explicit QValidator(QObject *parent = nullptr);
   ~QValidator();

   enum State {
      Invalid,
      Intermediate,
      Acceptable
   };

   void setLocale(const QLocale &locale);
   QLocale locale() const;

   virtual State validate(QString &, int &) const = 0;
   virtual void fixup(QString &) const;

   GUI_CS_SIGNAL_1(Public, void changed())
   GUI_CS_SIGNAL_2(changed)

 protected:
   QValidator(QValidatorPrivate &d, QObject *parent);
   QScopedPointer<QValidatorPrivate> d_ptr;

 private:
   Q_DISABLE_COPY(QValidator)
   Q_DECLARE_PRIVATE(QValidator)

};

class Q_GUI_EXPORT QIntValidator : public QValidator
{
   GUI_CS_OBJECT(QIntValidator)

   GUI_CS_PROPERTY_READ(bottom, bottom)
   GUI_CS_PROPERTY_WRITE(bottom, setBottom)
   GUI_CS_PROPERTY_NOTIFY(bottom, bottomChanged)
   GUI_CS_PROPERTY_READ(top, top)
   GUI_CS_PROPERTY_WRITE(top, setTop)
   GUI_CS_PROPERTY_NOTIFY(top, topChanged)

 public:
   explicit QIntValidator(QObject *parent = nullptr);
   QIntValidator(int bottom, int top, QObject *parent = nullptr);
   ~QIntValidator();

   QValidator::State validate(QString &, int &) const override;
   void fixup(QString &input) const override;

   void setBottom(int);
   void setTop(int);
   virtual void setRange(int bottom, int top);

   inline int bottom() const;
   inline int top() const;

   GUI_CS_SIGNAL_1(Public, void bottomChanged(int bottom))
   GUI_CS_SIGNAL_2(bottomChanged, bottom)
   GUI_CS_SIGNAL_1(Public, void topChanged(int top))
   GUI_CS_SIGNAL_2(topChanged, top)

 private:
   Q_DISABLE_COPY(QIntValidator)

   int b;
   int t;
};

int QIntValidator::bottom() const
{
   return b;
}

int QIntValidator::top() const
{
   return t;
}



#ifndef QT_NO_REGEXP

class QDoubleValidatorPrivate;
class QRegularExpressionValidatorPrivate;

class Q_GUI_EXPORT QDoubleValidator : public QValidator
{
   GUI_CS_OBJECT(QDoubleValidator)

   GUI_CS_PROPERTY_READ(bottom, bottom)
   GUI_CS_PROPERTY_WRITE(bottom, setBottom)
   GUI_CS_PROPERTY_NOTIFY(bottom, bottomChanged)
   GUI_CS_PROPERTY_READ(top, top)
   GUI_CS_PROPERTY_WRITE(top, setTop)
   GUI_CS_PROPERTY_NOTIFY(top, topChanged)
   GUI_CS_PROPERTY_READ(decimals, decimals)
   GUI_CS_PROPERTY_WRITE(decimals, setDecimals)
   GUI_CS_PROPERTY_NOTIFY(decimals, decimalsChanged)

   GUI_CS_ENUM(Notation)
   GUI_CS_PROPERTY_READ(notation, notation)
   GUI_CS_PROPERTY_WRITE(notation, setNotation)
   GUI_CS_PROPERTY_NOTIFY(notation, notationChanged)

 public:
   explicit QDoubleValidator(QObject *parent = nullptr);
   QDoubleValidator(double bottom, double top, int decimals, QObject *parent = nullptr);
   ~QDoubleValidator();

   enum Notation {
      StandardNotation,
      ScientificNotation
   };
   QValidator::State validate(QString &, int &) const override;

   virtual void setRange(double bottom, double top, int decimals = 0);
   void setBottom(double);
   void setTop(double);
   void setDecimals(int);
   void setNotation(Notation);

   inline double bottom() const;
   inline double top() const;
   inline int decimals() const;
   Notation notation() const;

   GUI_CS_SIGNAL_1(Public, void bottomChanged(double bottom))
   GUI_CS_SIGNAL_2(bottomChanged, bottom)
   GUI_CS_SIGNAL_1(Public, void topChanged(double top))
   GUI_CS_SIGNAL_2(topChanged, top)
   GUI_CS_SIGNAL_1(Public, void decimalsChanged(int decimals))
   GUI_CS_SIGNAL_2(decimalsChanged, decimals)
   GUI_CS_SIGNAL_1(Public, void notationChanged(QDoubleValidator::Notation notation))
   GUI_CS_SIGNAL_2(notationChanged, notation)

 private:
   Q_DECLARE_PRIVATE(QDoubleValidator)
   Q_DISABLE_COPY(QDoubleValidator)

   double b;
   double t;
   int dec;
};

class Q_GUI_EXPORT QRegExpValidator : public QValidator
{
   GUI_CS_OBJECT(QRegExpValidator)

   GUI_CS_PROPERTY_READ(regExp, regExp)
   GUI_CS_PROPERTY_WRITE(regExp, setRegExp)
   GUI_CS_PROPERTY_NOTIFY(regExp, regExpChanged)

 public:
   explicit QRegExpValidator(QObject *parent = nullptr);
   QRegExpValidator(const QRegExp &rx, QObject *parent = nullptr);
   ~QRegExpValidator();

   QValidator::State validate(QString &input, int &pos) const override;

   void setRegExp(const QRegExp &rx);
   inline const QRegExp &regExp() const;

   GUI_CS_SIGNAL_1(Public, void regExpChanged(const QRegExp &regExp))
   GUI_CS_SIGNAL_2(regExpChanged, regExp)

 private:
   Q_DISABLE_COPY(QRegExpValidator)

   QRegExp r;
};

class Q_GUI_EXPORT QRegularExpressionValidator : public QValidator
{
   GUI_CS_OBJECT(QRegularExpressionValidator)

   GUI_CS_PROPERTY_READ(regularExpression, regularExpression)
   GUI_CS_PROPERTY_WRITE(regularExpression, setRegularExpression)
   GUI_CS_PROPERTY_NOTIFY(regularExpression, regularExpressionChanged)

 public:
   explicit QRegularExpressionValidator(QObject *parent = nullptr);

   // broom - change to use, const QRegularExpression & regExp
   explicit QRegularExpressionValidator(const QRegExp &regExp, QObject *parent = nullptr);

   ~QRegularExpressionValidator();

   QValidator::State validate(QString &input, int &pos) const override;

   QRegExp regularExpression() const;

   GUI_CS_SLOT_1(Public, void setRegularExpression(const QRegExp &re))
   GUI_CS_SLOT_2(setRegularExpression)

   GUI_CS_SIGNAL_1(Public, void regularExpressionChanged(const QRegExp &re))
   GUI_CS_SIGNAL_2(regularExpressionChanged, re)

 private:
   Q_DISABLE_COPY(QRegularExpressionValidator)
   Q_DECLARE_PRIVATE(QRegularExpressionValidator)
};

const QRegExp &QRegExpValidator::regExp() const
{
   return r;
}

double QDoubleValidator::bottom() const
{
   return b;
}

double QDoubleValidator::top() const
{
   return t;
}

int QDoubleValidator::decimals() const
{
   return dec;
}

#endif // QT_NO_REGEXP


#endif // QT_NO_VALIDATOR

QT_END_NAMESPACE

#endif // QVALIDATOR_H
