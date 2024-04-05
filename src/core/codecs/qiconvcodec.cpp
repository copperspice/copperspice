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

#include <qiconvcodec_p.h>

#include <qdebug.h>
#include <qlibrary.h>
#include <qthreadstorage.h>

#include <qtextcodec_p.h>

#include <dlfcn.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>

// unistd.h is needed for the _XOPEN_UNIX macro
#include <unistd.h>
#if defined(_XOPEN_UNIX)
#  include <langinfo.h>
#endif

#if defined(Q_OS_FREEBSD) || defined(Q_OS_DARWIN)
#  define NO_BOM
#  if Q_BYTE_ORDER == Q_BIG_ENDIAN
#    define UTF16 "UTF-16BE"
#  else
#    define UTF16 "UTF-16LE"
#  endif
#else
#  define UTF16 "UTF-16"
#endif

#ifdef Q_OS_DARWIN
#ifndef GNU_LIBICONV
#define GNU_LIBICONV
#endif

using Ptr_iconv_open  = iconv_t (*) (const char *, const char *);
using Ptr_iconv       = size_t (*) (iconv_t, const char **, size_t *, char **, size_t *);
using Ptr_iconv_close = int (*) (iconv_t);

static Ptr_iconv_open ptr_iconv_open = nullptr;
static Ptr_iconv ptr_iconv = nullptr;
static Ptr_iconv_close ptr_iconv_close = nullptr;
#endif

extern bool qt_locale_initialized;

QIconvCodec::QIconvCodec()
   : utf16Codec(nullptr)
{
   utf16Codec = QTextCodec::codecForMib(1015);
   Q_ASSERT_X(utf16Codec != nullptr,
              "QIconvCodec::convertToUnicode", "internal error, UTF-16 codec not found");

   if (! utf16Codec) {
      fprintf(stderr, "QIconvCodec::convertToUnicode: internal error, UTF-16 codec not found\n");
      utf16Codec = reinterpret_cast<QTextCodec *>(~0);
   }

#if defined(Q_OS_DARWIN)

   if (ptr_iconv_open == nullptr) {
      QLibrary libiconv("/usr/lib/libiconv");
      libiconv.setLoadHints(QLibrary::ExportExternalSymbolsHint);

      ptr_iconv_open = reinterpret_cast<Ptr_iconv_open>(libiconv.resolve("libiconv_open"));

      if (! ptr_iconv_open) {
         ptr_iconv_open = reinterpret_cast<Ptr_iconv_open>(libiconv.resolve("iconv_open"));
      }

      ptr_iconv = reinterpret_cast<Ptr_iconv>(libiconv.resolve("libiconv"));

      if (! ptr_iconv) {
         ptr_iconv = reinterpret_cast<Ptr_iconv>(libiconv.resolve("iconv"));
      }

      ptr_iconv_close = reinterpret_cast<Ptr_iconv_close>(libiconv.resolve("libiconv_close"));

      if (! ptr_iconv_close) {
         ptr_iconv_close = reinterpret_cast<Ptr_iconv_close>(libiconv.resolve("iconv_close"));
      }

      Q_ASSERT_X(ptr_iconv_open && ptr_iconv && ptr_iconv_close,
            "QIconvCodec::QIconvCodec()", "internal error, could not resolve the iconv functions");

#       undef iconv_open
#       define iconv_open ptr_iconv_open
#       undef iconv
#       define iconv ptr_iconv
#       undef iconv_close
#       define iconv_close ptr_iconv_close
   }

#endif
}

QIconvCodec::~QIconvCodec()
{
}

QIconvCodec::IconvState::IconvState(iconv_t x)
   : buffer(array), bufferLen(sizeof array), cd(x)
{
}

QIconvCodec::IconvState::~IconvState()
{
   if (cd != reinterpret_cast<iconv_t>(-1)) {
      iconv_close(cd);
   }

   if (buffer != array) {
      delete[] buffer;
   }
}

void QIconvCodec::IconvState::saveChars(const char *c, int count)
{
   if (count > bufferLen) {
      if (buffer != array) {
         delete[] buffer;
      }

      buffer = new char[bufferLen = count];
   }

   memcpy(buffer, c, count);
}

static void qIconvCodecStateFree(QTextCodec::ConverterState *state)
{
   delete reinterpret_cast<QIconvCodec::IconvState *>(state->m_data);
}

static QThreadStorage<QIconvCodec::IconvState *> *toUnicodeState()
{
   static QThreadStorage<QIconvCodec::IconvState *> retval;
   return &retval;
}

QString QIconvCodec::convertToUnicode(const char *chars, int len, ConverterState *convState) const
{
   if (utf16Codec == reinterpret_cast<QTextCodec *>(~0)) {
      return QString::fromLatin1(chars, len);
   }

   int invalidCount      = 0;
   int remainingCount    = 0;
   char *remainingBuffer = nullptr;

   IconvState *temporaryState = nullptr;
   IconvState **pstate;

   if (convState) {
      // stateful conversion
      pstate = reinterpret_cast<IconvState **>(&convState->m_data);

      if (convState->m_data) {
         // restore state
         remainingCount = convState->remainingChars;
         remainingBuffer = (*pstate)->buffer;

      } else {
         // first time
         convState->m_flags |= FreeFunction;
         QTextCodecUnalignedPointer::encode(convState->state_data, qIconvCodecStateFree);
      }

   } else {
      QThreadStorage<QIconvCodec::IconvState *> *ts = toUnicodeState();

      if (! qt_locale_initialized || ! ts) {
         // might be running before the QCoreApplication initialization

         pstate = &temporaryState;

      } else {
         // stateless conversion, use thread-local data
         pstate = &toUnicodeState()->localData();
      }
   }

   if (! *pstate) {
      // first time, create the state
      iconv_t cd = QIconvCodec::createIconv_t(UTF16, nullptr);

      if (cd == reinterpret_cast<iconv_t>(-1)) {
         static bool reported = false;

         if (! reported) {
            fprintf(stderr, "QIconvCodec::convertToUnicode: using Latin1 for conversion, iconv_open failed\n");
         }

         reported = true;

         return QString::fromLatin1(chars, len);
      }

      *pstate = new IconvState(cd);
   }

   IconvState *state = *pstate;
   size_t inBytesLeft = len;

#ifdef GNU_LIBICONV
   // GNU doesn't disagree with POSIX :/
   const char *inBytes = chars;
#else
   char *inBytes = const_cast<char *>(chars);
#endif

   QByteArray in;

   if (remainingCount) {
      // we have to prepend the remaining bytes from the previous conversion
      inBytesLeft += remainingCount;
      in.resize(inBytesLeft);
      inBytes = in.data();

      memcpy(in.data(), remainingBuffer, remainingCount);
      memcpy(in.data() + remainingCount, chars, len);

      remainingCount = 0;
   }

   size_t outBytesLeft = len * 2 + 2;
   QByteArray ba(outBytesLeft, Qt::NoData);
   char *outBytes = ba.data();

   do {
      size_t ret = iconv(state->cd, &inBytes, &inBytesLeft, &outBytes, &outBytesLeft);

      if (ret == (size_t) - 1) {
         if (errno == E2BIG) {
            int offset = ba.size() - outBytesLeft;
            ba.resize(ba.size() * 2);
            outBytes = ba.data() + offset;
            outBytesLeft = ba.size() - offset;

            continue;
         }

         if (errno == EILSEQ) {
            // conversion stopped because of an invalid character in the sequence
            ++invalidCount;

         } else if (errno == EINVAL && convState) {
            // conversion stopped because the remaining inBytesLeft make up
            // an incomplete multi-byte sequence; save them for later
            state->saveChars(inBytes, inBytesLeft);
            remainingCount = inBytesLeft;
            break;
         }

         if (errno == EILSEQ || errno == EINVAL) {
            // skip the next character
            ++inBytes;
            --inBytesLeft;
            continue;
         }

         // some other error
         // can not use qWarning() since we are implementing the codecForLocale

         perror("QIconvCodec::convertToUnicode: using Latin1 for conversion, iconv failed");

         if (!convState) {
            // reset state
            iconv(state->cd, nullptr, &inBytesLeft, nullptr, &outBytesLeft);
         }

         delete temporaryState;
         return QString::fromLatin1(chars, len);
      }

   } while (inBytesLeft != 0);

   QString s;

   if (convState) {
      s = utf16Codec->toUnicode(ba.constData(), ba.size() - outBytesLeft, &state->internalState);

      convState->invalidChars = invalidCount;
      convState->remainingChars = remainingCount;
   } else {
      s = utf16Codec->toUnicode(ba.constData(), ba.size() - outBytesLeft);

      // reset state
      iconv(state->cd, nullptr, &inBytesLeft, nullptr, &outBytesLeft);
   }

   delete temporaryState;
   return s;
}

static QThreadStorage<QIconvCodec::IconvState *> *fromUnicodeState()
{
   static QThreadStorage<QIconvCodec::IconvState *> retval;
   return &retval;
}

static bool setByteOrder(iconv_t cd)
{
#if ! defined(NO_BOM)
   // give iconv() a BOM
   char buf[4];
   ushort bom[] = { QChar::ByteOrderMark };

   char *outBytes = buf;
   char *inBytes = reinterpret_cast<char *>(bom);
   size_t outBytesLeft = sizeof buf;
   size_t inBytesLeft = sizeof bom;

#if defined(GNU_LIBICONV)
   const char **inBytesPtr = const_cast<const char **>(&inBytes);
#else
   char **inBytesPtr = &inBytes;
#endif

   if (iconv(cd, inBytesPtr, &inBytesLeft, &outBytes, &outBytesLeft) == (size_t) - 1) {
      return false;
   }

#endif

   return true;
}

QByteArray QIconvCodec::convertFromUnicode(QStringView str, ConverterState *convState) const
{
   (void) convState;

   /*
      char   *inBytes;
      char   *outBytes;
      size_t inBytesLeft;

   #if defined(GNU_LIBICONV)
      const char **inBytesPtr = const_cast<const char **>(&inBytes);
   #else
      char **inBytesPtr = &inBytes;
   #endif

   */

   // remove when enabled
   perror("QIconvCodec::convertFromUnicode: using Latin1 for conversion, iconv failed for BOM");
   return str.toLatin1();

   IconvState *temporaryState = nullptr;
   QThreadStorage<QIconvCodec::IconvState *> *ts = fromUnicodeState();
   IconvState *&state = (qt_locale_initialized && ts) ? ts->localData() : temporaryState;

   if (! state) {
      iconv_t cd = QIconvCodec::createIconv_t(nullptr, UTF16);

      if (cd != reinterpret_cast<iconv_t>(-1)) {
         if (! setByteOrder(cd)) {
            perror("QIconvCodec::convertFromUnicode: using Latin1 for conversion, iconv failed for BOM");

            iconv_close(cd);
            cd = reinterpret_cast<iconv_t>(-1);

            return str.toLatin1();
         }
      }

      state = new IconvState(cd);
   }

   if (state->cd == reinterpret_cast<iconv_t>(-1)) {
      static int reported = 0;

      if (! reported++) {
         fprintf(stderr, "QIconvCodec::convertFromUnicode: using Latin1 for conversion, iconv_open failed\n");
      }

      delete temporaryState;
      return str.toLatin1();
   }

   QByteArray ba;

   // broom - resolve this code as soon as possible

   /*
      size_t outBytesLeft = len;
      QByteArray ba(outBytesLeft, Qt::NoData);
      outBytes = ba.data();

      // now feed iconv() the real data
      inBytes = const_cast<char *>(reinterpret_cast<const char *>(uc));
      inBytesLeft = len * sizeof(QChar);

      QByteArray in;
      if (convState && convState->remainingChars) {
         // we have one surrogate char to be prepended
         in.resize(sizeof(QChar) + len);
         inBytes = in.data();

         QChar remaining = convState->state_data[0];
         memcpy(in.data(), &remaining, sizeof(QChar));
         memcpy(in.data() + sizeof(QChar), uc, inBytesLeft);

         inBytesLeft += sizeof(QChar);
         convState->remainingChars = 0;
      }

      int invalidCount = 0;
      while (inBytesLeft != 0) {
         if (iconv(state->cd, inBytesPtr, &inBytesLeft, &outBytes, &outBytesLeft) == (size_t) - 1) {
            if (errno == EINVAL && convState) {
               // buffer ends in a surrogate
               Q_ASSERT(inBytesLeft == 2);
               convState->remainingChars = 1;
               convState->state_data[0] = uc[len - 1].unicode();
               break;
            }

            switch (errno) {
               case EILSEQ:
                  ++invalidCount;
               // fall through
               case EINVAL: {
                  inBytes += sizeof(QChar);
                  inBytesLeft -= sizeof(QChar);
                  break;
               }
               case E2BIG: {
                  int offset = ba.size() - outBytesLeft;
                  ba.resize(ba.size() * 2);
                  outBytes = ba.data() + offset;
                  outBytesLeft = ba.size() - offset;
                  break;
               }
               default: {
                  // note, cannot use qWarning() since we are implementing the codecForLocale :)
                  perror("QIconvCodec::convertFromUnicode: using Latin1 for conversion, iconv failed");

                  // reset to initial state
                  iconv(state->cd, 0, &inBytesLeft, 0, &outBytesLeft);

                  delete temporaryState;
                  return QString(uc, len).toLatin1();
               }
            }
         }
      }

      // reset to initial state
      iconv(state->cd, 0, &inBytesLeft, 0, &outBytesLeft);
      setByteOrder(state->cd);

      ba.resize(ba.size() - outBytesLeft);

      if (convState) {
         convState->invalidChars = invalidCount;
      }

      delete temporaryState;
   */

   return ba;
}

QString QIconvCodec::name() const
{
   return QString("System");
}

int QIconvCodec::mibEnum() const
{
   return 0;
}

iconv_t QIconvCodec::createIconv_t(const char *to, const char *from)
{
   Q_ASSERT((to == nullptr && from != nullptr) || (to != nullptr && from == nullptr));

   iconv_t cd = (iconv_t) - 1;

#if defined(__GLIBC__) || defined(GNU_LIBICONV)
   // both GLIBC and libgnuiconv will use the locale's encoding if from or to is an empty string

   const char *codeset = "";
   cd = iconv_open(to ? to : codeset, from ? from : codeset);

#else
   char *codeset = nullptr;

#endif

#if defined(_XOPEN_UNIX)

   if (cd == (iconv_t) - 1) {
      codeset = nl_langinfo(CODESET);

      if (codeset) {
         cd = iconv_open(to ? to : codeset, from ? from : codeset);
      }
   }

#endif

   if (cd == (iconv_t) - 1) {

      // This logic is duplicated in QTextCodec, so if you change it here, change
      // it there too.

      // Try to determine locale codeset from locale name assigned to LC_CTYPE category.

      // First part is getting that locale name.  First try setlocale() which
      // definitely knows it, but since we cannot fully trust it, get ready
      // to fall back to environment variables.
      char *ctype = qstrdup(setlocale(LC_CTYPE, nullptr));

      // Get the first nonempty value from $LC_ALL, $LC_CTYPE, and $LANG environment variables.
      char *lang = qstrdup(qgetenv("LC_ALL").constData());

      if (! lang || lang[0] == 0 || strcmp(lang, "C") == 0) {
         if (lang) {
            delete [] lang;
         }

         lang = qstrdup(qgetenv("LC_CTYPE").constData());
      }

      if (! lang || lang[0] == 0 || strcmp(lang, "C") == 0) {
         if (lang) {
            delete [] lang;
         }

         lang = qstrdup(qgetenv("LANG").constData());
      }

      // Now try these in order:
      // 1. CODESET from ctype if it contains a .CODESET part (e.g. en_US.ISO8859-15)
      // 2. CODESET from lang if it contains a .CODESET part
      // 3. ctype (maybe the locale is named "ISO-8859-1" or something)
      // 4. locale (ditto)
      // 5. check for "@euro"

      // 1. CODESET from ctype if it contains a .CODESET part (e.g. en_US.ISO8859-15)
      codeset = ctype ? strchr(ctype, '.') : nullptr;

      if (codeset && *codeset == '.') {
         ++codeset;
         cd = iconv_open(to ? to : codeset, from ? from : codeset);
      }

      // 2. CODESET from lang if it contains a .CODESET part
      codeset = lang ? strchr(lang, '.') : nullptr;

      if (cd == (iconv_t) - 1 && codeset && *codeset == '.') {
         ++codeset;
         cd = iconv_open(to ? to : codeset, from ? from : codeset);
      }

      // 3. ctype (maybe the locale is named "ISO-8859-1" or something)
      if (cd == (iconv_t) - 1 && ctype && *ctype != 0 && strcmp (ctype, "C") != 0) {
         cd = iconv_open(to ? to : ctype, from ? from : ctype);
      }

      // 4. locale (ditto)
      if (cd == (iconv_t) - 1 && lang && *lang != 0) {
         cd = iconv_open(to ? to : lang, from ? from : lang);
      }

      // 5. "@euro"
      if ((cd == (iconv_t) - 1 && ctype && strstr(ctype, "@euro")) || (lang && strstr(lang, "@euro"))) {
         cd = iconv_open(to ? to : "ISO8859-15", from ? from : "ISO8859-15");
      }

      delete [] ctype;
      delete [] lang;
   }

   return cd;
}
