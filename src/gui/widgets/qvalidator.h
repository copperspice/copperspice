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

#ifndef QVALIDATOR_H
#define QVALIDATOR_H

#include <qobject.h>
#include <qstring.h>
#include <qregularexpression.h>
#include <qlocale.h>
#include <qscopedpointer.h>

#ifndef QT_NO_VALIDATOR

class QValidatorPrivate;
class QDoubleValidatorPrivate;
class QRegularExpressionValidatorPrivate;

class Q_GUI_EXPORT QValidator : public QObject
{
   GUI_CS_OBJECT(QValidator)

 public:
   explicit QValidator(QObject *parent = nullptr);

   QValidator(const QValidator &) = delete;
   QValidator &operator=(const QValidator &) = delete;

   ~QValidator();

   enum State {
      Invalid,
      Intermediate,
      Acceptable
   };

   void setLocale(const QLocale &locale);
   QLocale locale() const;

   virtual State validate(QString &input, int &pos) const = 0;
   virtual void fixup(QString &input) const;

   GUI_CS_SIGNAL_1(Public, void changed())
   GUI_CS_SIGNAL_2(changed)

 protected:
   QValidator(QValidatorPrivate &d, QObject *parent);
   QScopedPointer<QValidatorPrivate> d_ptr;

 private:
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
   QIntValidator(int minimum, int maximum, QObject *parent = nullptr);

   QIntValidator(const QIntValidator &) = delete;
   QIntValidator &operator=(const QIntValidator &) = delete;

   ~QIntValidator();

   QValidator::State validate(QString &input, int &pos) const override;
   void fixup(QString &input) const override;

   void setBottom(int newValue);
   void setTop(int newValue);
   virtual void setRange(int minimum, int maximum);

   inline int bottom() const;
   inline int top() const;

   GUI_CS_SIGNAL_1(Public, void bottomChanged(int newValue))
   GUI_CS_SIGNAL_2(bottomChanged, newValue)

   GUI_CS_SIGNAL_1(Public, void topChanged(int newValue))
   GUI_CS_SIGNAL_2(topChanged, newValue)

 private:
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
   enum Notation {
      StandardNotation,
      ScientificNotation
   };

   explicit QDoubleValidator(QObject *parent = nullptr);
   QDoubleValidator(double minimum, double maximum, int decimals, QObject *parent = nullptr);

   QDoubleValidator(const QDoubleValidator &) = delete;
   QDoubleValidator &operator=(const QDoubleValidator &) = delete;

   ~QDoubleValidator();

   QValidator::State validate(QString &input, int &pos) const override;
   virtual void setRange(double minimum, double maximum, int decimals = 0);

   void setBottom(double newValue);
   void setTop(double newValue);
   void setDecimals(int decimals);
   void setNotation(Notation notation);

   inline double bottom() const;
   inline double top() const;
   inline int decimals() const;
   Notation notation() const;

   GUI_CS_SIGNAL_1(Public, void bottomChanged(double newValue))
   GUI_CS_SIGNAL_2(bottomChanged, newValue)

   GUI_CS_SIGNAL_1(Public, void topChanged(double newValue))
   GUI_CS_SIGNAL_2(topChanged, newValue)

   GUI_CS_SIGNAL_1(Public, void decimalsChanged(int decimals))
   GUI_CS_SIGNAL_2(decimalsChanged, decimals)

   GUI_CS_SIGNAL_1(Public, void notationChanged(QDoubleValidator::Notation notation))
   GUI_CS_SIGNAL_2(notationChanged, notation)

 private:
   Q_DECLARE_PRIVATE(QDoubleValidator)

   double b;
   double t;
   int dec;
};

class Q_GUI_EXPORT QRegularExpressionValidator : public QValidator
{
   GUI_CS_OBJECT(QRegularExpressionValidator)

   GUI_CS_PROPERTY_READ(regularExpression, regularExpression)
   GUI_CS_PROPERTY_WRITE(regularExpression, setRegularExpression)
   GUI_CS_PROPERTY_NOTIFY(regularExpression, regularExpressionChanged)

 public:
   explicit QRegularExpressionValidator(QObject *parent = nullptr);
   explicit QRegularExpressionValidator(const QRegularExpression &regExp, QObject *parent = nullptr);

   QRegularExpressionValidator(const QRegularExpressionValidator &) = delete;
   QRegularExpressionValidator &operator=(const QRegularExpressionValidator &) = delete;

   ~QRegularExpressionValidator();

   QValidator::State validate(QString &input, int &pos) const override;

   const QRegularExpression &regularExpression() const;

   GUI_CS_SIGNAL_1(Public, void regularExpressionChanged(const QRegularExpression &regExp))
   GUI_CS_SIGNAL_2(regularExpressionChanged, regExp)

   GUI_CS_SLOT_1(Public, void setRegularExpression(const QRegularExpression &regExp))
   GUI_CS_SLOT_2(setRegularExpression)

 private:
   Q_DECLARE_PRIVATE(QRegularExpressionValidator)

   QRegularExpression m_regexp;
};

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

#endif // QT_NO_VALIDATOR

#endif
