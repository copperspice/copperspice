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

#ifndef QTEXTCODEC_H
#define QTEXTCODEC_H

#include <QtCore/qstring.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_TEXTCODEC

class QTextCodec;
class QIODevice;
class QTextDecoder;
class QTextEncoder;

class Q_CORE_EXPORT QTextCodec
{
   Q_DISABLE_COPY(QTextCodec)

 public:
   static QTextCodec *codecForName(const QByteArray &name);
   static QTextCodec *codecForName(const char *name) {
      return codecForName(QByteArray(name));
   }
   static QTextCodec *codecForMib(int mib);

   static QList<QByteArray> availableCodecs();
   static QList<int> availableMibs();

   static QTextCodec *codecForLocale();
   static void setCodecForLocale(QTextCodec *c);

   static QTextCodec *codecForTr();
   static void setCodecForTr(QTextCodec *c);

   static QTextCodec *codecForHtml(const QByteArray &ba);
   static QTextCodec *codecForHtml(const QByteArray &ba, QTextCodec *defaultCodec);

   static QTextCodec *codecForUtfText(const QByteArray &ba);
   static QTextCodec *codecForUtfText(const QByteArray &ba, QTextCodec *defaultCodec);

   bool canEncode(QChar) const;
   bool canEncode(const QString &) const;

   QString toUnicode(const QByteArray &) const;
   QString toUnicode(const char *chars) const;
   QByteArray fromUnicode(const QString &uc) const;

   enum ConversionFlag {
      DefaultConversion,
      ConvertInvalidToNull = 0x80000000,
      IgnoreHeader = 0x1,
      FreeFunction = 0x2
   };
   using ConversionFlags = QFlags<ConversionFlag>;

   struct Q_CORE_EXPORT ConverterState {
      ConverterState(ConversionFlags f = DefaultConversion)
         : flags(f), remainingChars(0), invalidChars(0), d(0) {
         state_data[0] = state_data[1] = state_data[2] = 0;
      }
      ~ConverterState();
      ConversionFlags flags;
      int remainingChars;
      int invalidChars;
      uint state_data[3];
      void *d;
    private:
      Q_DISABLE_COPY(ConverterState)
   };

   QString toUnicode(const char *in, int length, ConverterState *state = 0) const {
      return convertToUnicode(in, length, state);
   }
   QByteArray fromUnicode(const QChar *in, int length, ConverterState *state = 0) const {
      return convertFromUnicode(in, length, state);
   }

   // ### Qt5/merge these functions.
   QTextDecoder *makeDecoder() const;
   QTextDecoder *makeDecoder(ConversionFlags flags) const;
   QTextEncoder *makeEncoder() const;
   QTextEncoder *makeEncoder(ConversionFlags flags) const;

   virtual QByteArray name() const = 0;
   virtual QList<QByteArray> aliases() const;
   virtual int mibEnum() const = 0;

 protected:
   virtual QString convertToUnicode(const char *in, int length, ConverterState *state) const = 0;
   virtual QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const = 0;

   QTextCodec();
   virtual ~QTextCodec();

 private:
   friend class QTextCodecCleanup;
   static QTextCodec *cftr;
   static bool validCodecs();
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QTextCodec::ConversionFlags)

inline QTextCodec *QTextCodec::codecForTr()
{
   return validCodecs() ? cftr : 0;
}

inline void QTextCodec::setCodecForTr(QTextCodec *c)
{
   cftr = c;
}

class Q_CORE_EXPORT QTextEncoder
{
   Q_DISABLE_COPY(QTextEncoder)

 public:
   explicit QTextEncoder(const QTextCodec *codec) : c(codec), state() {}
   QTextEncoder(const QTextCodec *codec, QTextCodec::ConversionFlags flags);
   ~QTextEncoder();
   QByteArray fromUnicode(const QString &str);
   QByteArray fromUnicode(const QChar *uc, int len);
   bool hasFailure() const;

 private:
   const QTextCodec *c;
   QTextCodec::ConverterState state;
};

class Q_CORE_EXPORT QTextDecoder
{
   Q_DISABLE_COPY(QTextDecoder)

 public:
   explicit QTextDecoder(const QTextCodec *codec) : c(codec), state() {}
   QTextDecoder(const QTextCodec *codec, QTextCodec::ConversionFlags flags);
   ~QTextDecoder();
   QString toUnicode(const char *chars, int len);
   QString toUnicode(const QByteArray &ba);
   void toUnicode(QString *target, const char *chars, int len);
   bool hasFailure() const;

 private:
   const QTextCodec *c;
   QTextCodec::ConverterState state;
};

#endif // QT_NO_TEXTCODEC

QT_END_NAMESPACE

#endif // QTEXTCODEC_H
