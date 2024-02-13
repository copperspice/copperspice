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

#ifndef QTEXTCODEC_H
#define QTEXTCODEC_H

#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>

#ifndef QT_NO_TEXTCODEC

class QTextCodec;
class QIODevice;
class QTextDecoder;
class QTextEncoder;

class Q_CORE_EXPORT QTextCodec
{
 public:
   QTextCodec(const QTextCodec &) = delete;
   QTextCodec &operator=(const QTextCodec &) = delete;

   static QTextCodec *codecForName(const QString &name);

   static QTextCodec *codecForName(const char *name) {
      return codecForName(QString::fromUtf8(name));
   }

   static QTextCodec *codecForMib(int mib);

   static QStringList availableCodecs();
   static QList<int> availableMibs();

   static QTextCodec *codecForLocale();
   static void setCodecForLocale(QTextCodec *c);

   static QTextCodec *codecForTr();
   static void setCodecForTr(QTextCodec *c);

   static QTextCodec *codecForHtml(const QByteArray &data);
   static QTextCodec *codecForHtml(const QByteArray &data, QTextCodec *defaultCodec);

   static QTextCodec *codecForUtfText(const QByteArray &data);
   static QTextCodec *codecForUtfText(const QByteArray &data, QTextCodec *defaultCodec);

   bool canEncode(QChar ch) const;
   bool canEncode(const QString &str) const;

   enum ConversionFlag {
      DefaultConversion,
      ConvertInvalidToNull = 0x80000000,
      IgnoreHeader = 0x1,
      FreeFunction = 0x2
   };
   using ConversionFlags = QFlags<ConversionFlag>;

   struct Q_CORE_EXPORT ConverterState {
      ConverterState(ConversionFlags flags = DefaultConversion)
         : m_flags(flags), remainingChars(0), invalidChars(0), m_data(nullptr)
      {
         state_data[0] = 0;
         state_data[1] = 0;
         state_data[2] = 0;
      }

      ConverterState(const ConverterState &) = delete;
      ConverterState &operator=(const ConverterState &) = delete;

      ~ConverterState();

      ConversionFlags m_flags;
      int remainingChars;
      int invalidChars;
      uint state_data[3];
      void *m_data;
   };

   QString toUnicode(const QByteArray &input) const;
   QString toUnicode(const char *input) const;

   QString toUnicode(const char *input, int len, ConverterState *state = nullptr) const {
      return convertToUnicode(input, len, state);
   }

   QByteArray fromUnicode(const QString &str, ConverterState *state = nullptr) const {
      return convertFromUnicode(str, state);
   }

   QByteArray fromUnicode(QStringView str, ConverterState *state = nullptr) const {
      return convertFromUnicode(str, state);
   }

   QTextDecoder *makeDecoder(ConversionFlags flags = DefaultConversion) const;
   QTextEncoder *makeEncoder(ConversionFlags flags = DefaultConversion) const;

   virtual QString name() const = 0;
   virtual QStringList aliases() const;
   virtual int mibEnum() const = 0;

 protected:
   virtual QString convertToUnicode(const char *input, int len, ConverterState *state) const = 0;
   virtual QByteArray convertFromUnicode(QStringView str, ConverterState *state) const = 0;

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
   return validCodecs() ? cftr : nullptr;
}

inline void QTextCodec::setCodecForTr(QTextCodec *c)
{
   cftr = c;
}

class Q_CORE_EXPORT QTextEncoder
{
 public:
   explicit QTextEncoder(const QTextCodec *codec) : c(codec), state() {}
   QTextEncoder(const QTextCodec *codec, QTextCodec::ConversionFlags flags);

   QTextEncoder(const QTextEncoder &) = delete;
   QTextEncoder &operator=(const QTextEncoder &) = delete;

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
 public:
   explicit QTextDecoder(const QTextCodec *codec)
      : c(codec), state()
   {
   }

   QTextDecoder(const QTextCodec *codec, QTextCodec::ConversionFlags flags);

   QTextDecoder(const QTextDecoder &) = delete;
   QTextDecoder &operator=(const QTextDecoder &) = delete;

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

#endif
