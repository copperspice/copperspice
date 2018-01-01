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

#include <qplatformdefs.h>
#include <qtextcodec.h>
#include <qtextcodec_p.h>

#ifndef QT_NO_TEXTCODEC

#include <qlist.h>
#include <qfile.h>
#include <qvarlengtharray.h>
#include <qcoreapplication.h>
#include <qtextcodecplugin.h>
#include <qfactoryloader_p.h>
#include <qstringlist.h>

#ifdef Q_OS_UNIX
#  include <qiconvcodec_p.h>
#endif

#include <qutfcodec_p.h>
#include <qsimplecodec_p.h>
#include <qlatincodec_p.h>

#ifndef QT_NO_CODECS
#  include <qtsciicodec_p.h>
#  include <qisciicodec_p.h>

#  if defined(QT_NO_ICONV) && ! defined(QT_CODEC_PLUGINS)
//   no iconv(3) support, must build all codecs into the library
#    include "../../plugins/codecs/cn/qgb18030codec.h"
#    include "../../plugins/codecs/jp/qeucjpcodec.h"
#    include "../../plugins/codecs/jp/qjiscodec.h"
#    include "../../plugins/codecs/jp/qsjiscodec.h"
#    include "../../plugins/codecs/kr/qeuckrcodec.h"
#    include "../../plugins/codecs/tw/qbig5codec.h"
#  endif

#  if defined(Q_WS_X11)
#    include <qfontlaocodec_p.h>
#    include "../../plugins/codecs/jp/qfontjpcodec.h"
#  endif
#endif

#include <qlocale.h>
#include <qmutex.h>
#include <qhash.h>

#include <stdlib.h>
#include <ctype.h>
#include <locale.h>

#if defined (_XOPEN_UNIX)
#include <langinfo.h>
#endif

// enabling this is not exception safe!
// #define Q_DEBUG_TEXTCODEC

QT_BEGIN_NAMESPACE

#if ! defined(QT_NO_TEXTCODECPLUGIN)
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QTextCodecFactoryInterface_iid, QLatin1String("/codecs")))
#endif

//Cache for QTextCodec::codecForName and codecForMib.
typedef QHash<QByteArray, QTextCodec *> QTextCodecCache;
Q_GLOBAL_STATIC(QTextCodecCache, qTextCodecCache)


static char qtolower(char c)
{
   if (c >= 'A' && c <= 'Z') {
      return c + 0x20;
   }
   return c;
}
static bool qisalnum(char c)
{
   return (c >= '0' && c <= '9') || ((c | 0x20) >= 'a' && (c | 0x20) <= 'z');
}

static bool nameMatch(const QByteArray &name, const QByteArray &test)
{
   // if they're the same, return a perfect score
   if (qstricmp(name.constData(), test.constData()) == 0) {
      return true;
   }

   const char *n = name.constData();
   const char *h = test.constData();

   // if the letters and numbers are the same, we have a match
   while (*n != '\0') {
      if (qisalnum(*n)) {
         for (;;) {
            if (*h == '\0') {
               return false;
            }
            if (qisalnum(*h)) {
               break;
            }
            ++h;
         }
         if (qtolower(*n) != qtolower(*h)) {
            return false;
         }
         ++h;
      }
      ++n;
   }
   while (*h && !qisalnum(*h)) {
      ++h;
   }
   return (*h == '\0');
}


static QTextCodec *createForName(const QByteArray &name)
{
#if ! defined(QT_NO_TEXTCODECPLUGIN)
   QFactoryLoader *l = loader();
   QStringList keys = l->keys();
   for (int i = 0; i < keys.size(); ++i) {
      if (nameMatch(name, keys.at(i).toLatin1())) {
         QString realName = keys.at(i);
         if (QTextCodecFactoryInterface * factory
               = qobject_cast<QTextCodecFactoryInterface *>(l->instance(realName))) {
            return factory->create(realName);
         }
      }
   }
#else
   Q_UNUSED(name);
#endif
   return 0;
}

static QTextCodec *createForMib(int mib)
{
#ifndef QT_NO_TEXTCODECPLUGIN
   QString name = QLatin1String("MIB: ") + QString::number(mib);

   if (QTextCodecFactoryInterface *factory = qobject_cast<QTextCodecFactoryInterface *>(loader()->instance(name))) {
      return factory->create(name);
   }
#else
   Q_UNUSED(mib);

#endif
   return 0;
}

static QList<QTextCodec *> *all = 0;
#ifdef Q_DEBUG_TEXTCODEC
static bool destroying_is_ok = false;
#endif

static QTextCodec *localeMapper = 0;
QTextCodec *QTextCodec::cftr = 0;


class QTextCodecCleanup
{
 public:
   ~QTextCodecCleanup();
};

/*
    Deletes all the created codecs. This destructor is called just
    before exiting to delete any QTextCodec objects that may be lying
    around.
*/
QTextCodecCleanup::~QTextCodecCleanup()
{
   if (!all) {
      return;
   }

#ifdef Q_DEBUG_TEXTCODEC
   destroying_is_ok = true;
#endif

   QList<QTextCodec *> *myAll = all;
   all = 0; // Otherwise the d'tor destroys the iterator
   for (QList<QTextCodec *>::const_iterator it = myAll->constBegin()
         ; it != myAll->constEnd(); ++it) {
      delete *it;
   }
   delete myAll;
   localeMapper = 0;

#ifdef Q_DEBUG_TEXTCODEC
   destroying_is_ok = false;
#endif
}

Q_GLOBAL_STATIC(QTextCodecCleanup, createQTextCodecCleanup)

bool QTextCodec::validCodecs()
{
   return true;
}


#if defined(Q_OS_WIN32)
class QWindowsLocalCodec: public QTextCodec
{
 public:
   QWindowsLocalCodec();
   ~QWindowsLocalCodec();

   QString convertToUnicode(const char *, int, ConverterState *) const override;
   QByteArray convertFromUnicode(const QChar *, int, ConverterState *) const override;
   QString convertToUnicodeCharByChar(const char *chars, int length, ConverterState *state) const;

   QByteArray name() const override;
   int mibEnum() const override;

};

QWindowsLocalCodec::QWindowsLocalCodec()
{
}

QWindowsLocalCodec::~QWindowsLocalCodec()
{
}

QString QWindowsLocalCodec::convertToUnicode(const char *chars, int length, ConverterState *state) const
{
   const char *mb = chars;
   int mblen = length;

   if (!mb || !mblen) {
      return QString();
   }

   QVarLengthArray<wchar_t, 4096> wc(4096);
   int len;
   QString sp;
   bool prepend = false;
   char state_data = 0;
   int remainingChars = 0;

   //save the current state information
   if (state) {
      state_data = (char)state->state_data[0];
      remainingChars = state->remainingChars;
   }

   //convert the pending charcter (if available)
   if (state && remainingChars) {
      char prev[3] = {0};
      prev[0] = state_data;
      prev[1] = mb[0];
      remainingChars = 0;
      len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                                prev, 2, wc.data(), wc.size());
      if (len) {
         prepend = true;
         sp.append(QChar(wc[0]));
         mb++;
         mblen--;
         wc[0] = 0;
      }
   }

   while (!(len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                                      mb, mblen, wc.data(), wc.size()))) {
      int r = GetLastError();
      if (r == ERROR_INSUFFICIENT_BUFFER) {
         const int wclen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED,
                                               mb, mblen, 0, 0);
         wc.resize(wclen);
      } else if (r == ERROR_NO_UNICODE_TRANSLATION) {
         //find the last non NULL character
         while (mblen > 1  && !(mb[mblen - 1])) {
            mblen--;
         }
         //check whether,  we hit an invalid character in the middle
         if ((mblen <= 1) || (remainingChars && state_data)) {
            return convertToUnicodeCharByChar(chars, length, state);
         }
         //Remove the last character and try again...
         state_data = mb[mblen - 1];
         remainingChars = 1;
         mblen--;
      } else {
         // Fail.
         qWarning("MultiByteToWideChar: Cannot convert multibyte text");
         break;
      }
   }

   if (len <= 0) {
      return QString();
   }

   if (wc[len - 1] == 0) { // len - 1: we don't want terminator
      --len;
   }

   //save the new state information
   if (state) {
      state->state_data[0] = (char)state_data;
      state->remainingChars = remainingChars;
   }
   QString s((QChar *)wc.data(), len);
   if (prepend) {
      return sp + s;
   }
   return s;
}

QString QWindowsLocalCodec::convertToUnicodeCharByChar(const char *chars, int length, ConverterState *state) const
{
   if (!chars || !length) {
      return QString();
   }

   int copyLocation = 0;
   int extra = 2;
   if (state && state->remainingChars) {
      copyLocation = state->remainingChars;
      extra += copyLocation;
   }
   int newLength = length + extra;
   char *mbcs = new char[newLength];
   //ensure that we have a NULL terminated string
   mbcs[newLength - 1] = 0;
   mbcs[newLength - 2] = 0;
   memcpy(&(mbcs[copyLocation]), chars, length);
   if (copyLocation) {
      //copy the last character from the state
      mbcs[0] = (char)state->state_data[0];
      state->remainingChars = 0;
   }
   const char *mb = mbcs;

   const char *next = 0;
   QString s;
   while ((next = CharNextExA(CP_ACP, mb, 0)) != mb) {
      wchar_t wc[2] = {0};
      int charlength = next - mb;
      int len = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, mb, charlength, wc, 2);
      if (len > 0) {
         s.append(QChar(wc[0]));
      } else {
         int r = GetLastError();
         //check if the character being dropped is the last character
         if (r == ERROR_NO_UNICODE_TRANSLATION && mb == (mbcs + newLength - 3) && state) {
            state->remainingChars = 1;
            state->state_data[0] = (char) * mb;
         }
      }
      mb = next;
   }

   delete [] mbcs;
   return s;
}

QByteArray QWindowsLocalCodec::convertFromUnicode(const QChar *ch, int uclen, ConverterState *) const
{
   if (!ch) {
      return QByteArray();
   }
   if (uclen == 0) {
      return QByteArray("");
   }
   BOOL used_def;
   QByteArray mb(4096, 0);
   int len;
   while (!(len = WideCharToMultiByte(CP_ACP, 0, (const wchar_t *)ch, uclen,
                                      mb.data(), mb.size() - 1, 0, &used_def))) {
      int r = GetLastError();
      if (r == ERROR_INSUFFICIENT_BUFFER) {
         mb.resize(1 + WideCharToMultiByte(CP_ACP, 0,
                                           (const wchar_t *)ch, uclen,
                                           0, 0, 0, &used_def));
         // and try again...
      } else {
#ifndef QT_NO_DEBUG
         // Fail.
         qWarning("WideCharToMultiByte: Cannot convert multibyte text (error %d): %s (UTF-8)",
                  r, QString(ch, uclen).toLocal8Bit().data());
#endif
         break;
      }
   }
   mb.resize(len);
   return mb;
}


QByteArray QWindowsLocalCodec::name() const
{
   return "System";
}

int QWindowsLocalCodec::mibEnum() const
{
   return 0;
}

#else

/* locale names mostly copied from XFree86 */
static const char *const iso8859_2locales[] = {
   "croatian", "cs", "cs_CS", "cs_CZ", "cz", "cz_CZ", "czech", "hr",
   "hr_HR", "hu", "hu_HU", "hungarian", "pl", "pl_PL", "polish", "ro",
   "ro_RO", "rumanian", "serbocroatian", "sh", "sh_SP", "sh_YU", "sk",
   "sk_SK", "sl", "sl_CS", "sl_SI", "slovak", "slovene", "sr_SP", 0
};

static const char *const iso8859_3locales[] = {
   "eo", 0
};

static const char *const iso8859_4locales[] = {
   "ee", "ee_EE", 0
};

static const char *const iso8859_5locales[] = {
   "mk", "mk_MK", "sp", "sp_YU", 0
};

static const char *const cp_1251locales[] = {
   "be", "be_BY", "bg", "bg_BG", "bulgarian", 0
};

static const char *const pt_154locales[] = {
   "ba_RU", "ky", "ky_KG", "kk", "kk_KZ", 0
};

static const char *const iso8859_6locales[] = {
   "ar_AA", "ar_SA", "arabic", 0
};

static const char *const iso8859_7locales[] = {
   "el", "el_GR", "greek", 0
};

static const char *const iso8859_8locales[] = {
   "hebrew", "he", "he_IL", "iw", "iw_IL", 0
};

static const char *const iso8859_9locales[] = {
   "tr", "tr_TR", "turkish", 0
};

static const char *const iso8859_13locales[] = {
   "lt", "lt_LT", "lv", "lv_LV", 0
};

static const char *const iso8859_15locales[] = {
   "et", "et_EE",
   // Euro countries
   "br_FR", "ca_ES", "de", "de_AT", "de_BE", "de_DE", "de_LU", "en_IE",
   "es", "es_ES", "eu_ES", "fi", "fi_FI", "finnish", "fr", "fr_FR",
   "fr_BE", "fr_LU", "french", "ga_IE", "gl_ES", "it", "it_IT", "oc_FR",
   "nl", "nl_BE", "nl_NL", "pt", "pt_PT", "sv_FI", "wa_BE",
   0
};

static const char *const koi8_ulocales[] = {
   "uk", "uk_UA", "ru_UA", "ukrainian", 0
};

static const char *const tis_620locales[] = {
   "th", "th_TH", "thai", 0
};

// static const char * const tcvnlocales[] = {
//     "vi", "vi_VN", 0 };

static bool try_locale_list(const char *const locale[], const QByteArray &lang)
{
   int i;
   for (i = 0; locale[i] && lang != locale[i]; i++)
      ;
   return locale[i] != 0;
}

// For the probably_koi8_locales we have to look. the standard says
// these are 8859-5, but almost all Russian users use KOI8-R and
// incorrectly set $LANG to ru_RU. We'll check tolower() to see what
// it thinks ru_RU means.

// If you read the history, it seems that many Russians blame ISO and
// Perestroika for the confusion.
//
// The real bug is that some programs break if the user specifies
// ru_RU.KOI8-R.

static const char *const probably_koi8_rlocales[] = {
   "ru", "ru_SU", "ru_RU", "russian", 0
};

static QTextCodec *ru_RU_hack(const char *i)
{
   QTextCodec *ru_RU_codec = 0;

#if !defined(QT_NO_SETLOCALE)
   QByteArray origlocale(setlocale(LC_CTYPE, i));
#else
   QByteArray origlocale(i);
#endif
   // unicode   koi8r   latin5   name
   // 0x044E    0xC0    0xEE     CYRILLIC SMALL LETTER YU
   // 0x042E    0xE0    0xCE     CYRILLIC CAPITAL LETTER YU
   int latin5 = tolower(0xCE);
   int koi8r = tolower(0xE0);
   if (koi8r == 0xC0 && latin5 != 0xEE) {
      ru_RU_codec = QTextCodec::codecForName("KOI8-R");
   } else if (koi8r != 0xC0 && latin5 == 0xEE) {
      ru_RU_codec = QTextCodec::codecForName("ISO 8859-5");
   } else {
      // something else again... let's assume... *throws dice*
      ru_RU_codec = QTextCodec::codecForName("KOI8-R");
      qWarning("QTextCodec: Using KOI8-R, probe failed (%02x %02x %s)",
               koi8r, latin5, i);
   }
#if !defined(QT_NO_SETLOCALE)
   setlocale(LC_CTYPE, origlocale.constData());
#endif

   return ru_RU_codec;
}

#endif

#if !defined(Q_OS_WIN32)
static QTextCodec *checkForCodec(const QByteArray &name)
{
   QTextCodec *c = QTextCodec::codecForName(name);
   if (!c) {
      const int index = name.indexOf('@');
      if (index != -1) {
         c = QTextCodec::codecForName(name.left(index));
      }
   }
   return c;
}
#endif

/* the next two functions are implicitely thread safe,
   as they are only called by setup() which uses a mutex.
*/
static void setupLocaleMapper()
{

#if defined(Q_OS_WIN32)
   localeMapper = QTextCodec::codecForName("System");
#else

#ifndef QT_NO_ICONV
   localeMapper = QTextCodec::codecForName("System");
#endif

#if defined (_XOPEN_UNIX)
   if (!localeMapper) {
      char *charset = nl_langinfo (CODESET);
      if (charset) {
         localeMapper = QTextCodec::codecForName(charset);
      }
   }
#endif

   if (!localeMapper) {
      // Very poorly defined and followed standards causes lots of
      // code to try to get all the cases... This logic is
      // duplicated in QIconvCodec, so if you change it here, change
      // it there too.

      // Try to determine locale codeset from locale name assigned to
      // LC_CTYPE category.

      // First part is getting that locale name.  First try setlocale() which
      // definitely knows it, but since we cannot fully trust it, get ready
      // to fall back to environment variables.
#if !defined(QT_NO_SETLOCALE)
      const QByteArray ctype = setlocale(LC_CTYPE, 0);
#else
      const QByteArray ctype;
#endif

      // Get the first nonempty value from $LC_ALL, $LC_CTYPE, and $LANG
      // environment variables.
      QByteArray lang = qgetenv("LC_ALL");
      if (lang.isEmpty() || lang == "C") {
         lang = qgetenv("LC_CTYPE");
      }
      if (lang.isEmpty() || lang == "C") {
         lang = qgetenv("LANG");
      }

      // Now try these in order:
      // 1. CODESET from ctype if it contains a .CODESET part (e.g. en_US.ISO8859-15)
      // 2. CODESET from lang if it contains a .CODESET part
      // 3. ctype (maybe the locale is named "ISO-8859-1" or something)
      // 4. locale (ditto)
      // 5. check for "@euro"
      // 6. guess locale from ctype unless ctype is "C"
      // 7. guess locale from lang

      // 1. CODESET from ctype if it contains a .CODESET part (e.g. en_US.ISO8859-15)
      int indexOfDot = ctype.indexOf('.');
      if (indexOfDot != -1) {
         localeMapper = checkForCodec( ctype.mid(indexOfDot + 1) );
      }

      // 2. CODESET from lang if it contains a .CODESET part
      if (!localeMapper) {
         indexOfDot = lang.indexOf('.');
         if (indexOfDot != -1) {
            localeMapper = checkForCodec( lang.mid(indexOfDot + 1) );
         }
      }

      // 3. ctype (maybe the locale is named "ISO-8859-1" or something)
      if (!localeMapper && !ctype.isEmpty() && ctype != "C") {
         localeMapper = checkForCodec(ctype);
      }

      // 4. locale (ditto)
      if (!localeMapper && !lang.isEmpty()) {
         localeMapper = checkForCodec(lang);
      }

      // 5. "@euro"
      if ((!localeMapper && ctype.contains("@euro")) || lang.contains("@euro")) {
         localeMapper = checkForCodec("ISO 8859-15");
      }

      // 6. guess locale from ctype unless ctype is "C"
      // 7. guess locale from lang
      const QByteArray &try_by_name = (!ctype.isEmpty() && ctype != "C") ? lang : ctype;

      // Now do the guessing.
      if (!lang.isEmpty() && !localeMapper && !try_by_name.isEmpty()) {
         if (try_locale_list(iso8859_15locales, lang)) {
            localeMapper = QTextCodec::codecForName("ISO 8859-15");
         } else if (try_locale_list(iso8859_2locales, lang)) {
            localeMapper = QTextCodec::codecForName("ISO 8859-2");
         } else if (try_locale_list(iso8859_3locales, lang)) {
            localeMapper = QTextCodec::codecForName("ISO 8859-3");
         } else if (try_locale_list(iso8859_4locales, lang)) {
            localeMapper = QTextCodec::codecForName("ISO 8859-4");
         } else if (try_locale_list(iso8859_5locales, lang)) {
            localeMapper = QTextCodec::codecForName("ISO 8859-5");
         } else if (try_locale_list(iso8859_6locales, lang)) {
            localeMapper = QTextCodec::codecForName("ISO 8859-6");
         } else if (try_locale_list(iso8859_7locales, lang)) {
            localeMapper = QTextCodec::codecForName("ISO 8859-7");
         } else if (try_locale_list(iso8859_8locales, lang)) {
            localeMapper = QTextCodec::codecForName("ISO 8859-8-I");
         } else if (try_locale_list(iso8859_9locales, lang)) {
            localeMapper = QTextCodec::codecForName("ISO 8859-9");
         } else if (try_locale_list(iso8859_13locales, lang)) {
            localeMapper = QTextCodec::codecForName("ISO 8859-13");
         } else if (try_locale_list(tis_620locales, lang)) {
            localeMapper = QTextCodec::codecForName("ISO 8859-11");
         } else if (try_locale_list(koi8_ulocales, lang)) {
            localeMapper = QTextCodec::codecForName("KOI8-U");
         } else if (try_locale_list(cp_1251locales, lang)) {
            localeMapper = QTextCodec::codecForName("CP 1251");
         } else if (try_locale_list(pt_154locales, lang)) {
            localeMapper = QTextCodec::codecForName("PT 154");
         } else if (try_locale_list(probably_koi8_rlocales, lang)) {
            localeMapper = ru_RU_hack(lang.constData());
         }
      }

   }

   // If everything failed, we default to 8859-1
   // We could perhaps default to 8859-15.
   if (!localeMapper) {
      localeMapper = QTextCodec::codecForName("ISO 8859-1");
   }
#endif
}


Q_GLOBAL_STATIC_WITH_ARGS(QMutex, textCodecsMutex, (QMutex::Recursive));

// textCodecsMutex need to be locked to enter this function
static void setup()
{
   if (all) {
      return;
   }

#ifdef Q_DEBUG_TEXTCODEC
   if (destroying_is_ok) {
      qWarning("QTextCodec: Creating new codec during codec cleanup");
   }
#endif
   all = new QList<QTextCodec *>;
   // create the cleanup object to cleanup all codecs on exit
   (void) createQTextCodecCleanup();

#ifndef QT_NO_CODECS
   (void)new QTsciiCodec;
   for (int i = 0; i < 9; ++i) {
      (void)new QIsciiCodec(i);
   }

   for (int i = 0; i < QSimpleTextCodec::numSimpleCodecs; ++i) {
      (void)new QSimpleTextCodec(i);
   }


#  if defined(Q_WS_X11)
   (void)new QFontLaoCodec;
#    if defined(QT_NO_ICONV)
   // no iconv(3) support, must build all codecs into the library
   (void)new QFontGb2312Codec;
   (void)new QFontGbkCodec;
   (void)new QFontGb18030_0Codec;
   (void)new QFontJis0208Codec;
   (void)new QFontJis0201Codec;
   (void)new QFontKsc5601Codec;
   (void)new QFontBig5hkscsCodec;
   (void)new QFontBig5Codec;
#    endif // QT_NO_ICONV
#  endif // Q_WS_X11


#  if defined(QT_NO_ICONV) && !defined(QT_CODEC_PLUGINS)
   (void)new QGb18030Codec;
   (void)new QGbkCodec;
   (void)new QGb2312Codec;
   (void)new QEucJpCodec;
   (void)new QJisCodec;
   (void)new QSjisCodec;
   (void)new QEucKrCodec;
   (void)new QCP949Codec;
   (void)new QBig5Codec;
   (void)new QBig5hkscsCodec;
#  endif // QT_NO_ICONV
#endif // QT_NO_CODECS

#if defined(Q_OS_WIN32)
   (void) new QWindowsLocalCodec;
#endif // Q_OS_WIN32

   (void)new QUtf16Codec;
   (void)new QUtf16BECodec;
   (void)new QUtf16LECodec;
   (void)new QUtf32Codec;
   (void)new QUtf32BECodec;
   (void)new QUtf32LECodec;
   (void)new QLatin15Codec;
   (void)new QLatin1Codec;
   (void)new QUtf8Codec;

#if defined(Q_OS_UNIX) && !defined(QT_NO_ICONV)
   // QIconvCodec depends on the UTF-16 codec, so it needs to be created last
   (void) new QIconvCodec();
#endif


   if (!localeMapper) {
      setupLocaleMapper();
   }
}

/*!
    \enum QTextCodec::ConversionFlag

    \value DefaultConversion  No flag is set.
    \value ConvertInvalidToNull  If this flag is set, each invalid input
                                 character is output as a null character.
    \value IgnoreHeader  Ignore any Unicode byte-order mark and don't generate any.

    \omitvalue FreeFunction
*/

/*!
    \fn QTextCodec::ConverterState::ConverterState(ConversionFlags flags)

    Constructs a ConverterState object initialized with the given \a flags.
*/

/*!
    Destroys the ConverterState object.
*/
QTextCodec::ConverterState::~ConverterState()
{
   if (flags & FreeFunction) {
      (QTextCodecUnalignedPointer::decode(state_data))(this);
   } else if (d) {
      free(d);
   }
}

QTextCodec::QTextCodec()
{
   QMutexLocker locker(textCodecsMutex());

   setup();
   all->prepend(this);
}

QTextCodec::~QTextCodec()
{
#ifdef Q_DEBUG_TEXTCODEC
   if (!destroying_is_ok) {
      qWarning("QTextCodec::~QTextCodec: Called by application");
   }
#endif
   if (all) {
      QMutexLocker locker(textCodecsMutex());

      all->removeAll(this);
      QTextCodecCache *cache = qTextCodecCache();
      if (cache) {
         cache->clear();
      }
   }
}

QTextCodec *QTextCodec::codecForName(const QByteArray &name)
{
   if (name.isEmpty()) {
      return 0;
   }

   QMutexLocker locker(textCodecsMutex());
   setup();

   if (!validCodecs()) {
      return 0;
   }

   QTextCodecCache *cache = qTextCodecCache();
   QTextCodec *codec;
   if (cache) {
      codec = cache->value(name);
      if (codec) {
         return codec;
      }
   }

   for (int i = 0; i < all->size(); ++i) {
      QTextCodec *cursor = all->at(i);
      if (nameMatch(cursor->name(), name)) {
         if (cache) {
            cache->insert(name, cursor);
         }
         return cursor;
      }
      QList<QByteArray> aliases = cursor->aliases();
      for (int y = 0; y < aliases.size(); ++y)
         if (nameMatch(aliases.at(y), name)) {
            if (cache) {
               cache->insert(name, cursor);
            }
            return cursor;
         }
   }

   codec = createForName(name);
   if (codec && cache) {
      cache->insert(name, codec);
   }
   return codec;
}


/*!
    Returns the QTextCodec which matches the \link
    QTextCodec::mibEnum() MIBenum\endlink \a mib.
*/
QTextCodec *QTextCodec::codecForMib(int mib)
{
   QMutexLocker locker(textCodecsMutex());

   setup();

   if (!validCodecs()) {
      return 0;
   }

   QByteArray key = "MIB: " + QByteArray::number(mib);
   QTextCodecCache *cache = qTextCodecCache();
   QTextCodec *codec;
   if (cache) {
      codec = cache->value(key);
      if (codec) {
         return codec;
      }
   }

   QList<QTextCodec *>::ConstIterator i;
   for (int i = 0; i < all->size(); ++i) {
      QTextCodec *cursor = all->at(i);
      if (cursor->mibEnum() == mib) {
         if (cache) {
            cache->insert(key, cursor);
         }
         return cursor;
      }
   }

   codec = createForMib(mib);

   // Qt 3 used 1000 (mib for UCS2) as its identifier for the utf16 codec. Map
   // this correctly for compatibility.
   if (!codec && mib == 1000) {
      return codecForMib(1015);
   }

   if (codec && cache) {
      cache->insert(key, codec);
   }
   return codec;
}

/*!
    Returns the list of all available codecs, by name. Call
    QTextCodec::codecForName() to obtain the QTextCodec for the name.

    The list may contain many mentions of the same codec
    if the codec has aliases.

    \sa availableMibs(), name(), aliases()
*/
QList<QByteArray> QTextCodec::availableCodecs()
{
   QMutexLocker locker(textCodecsMutex());
   setup();

   QList<QByteArray> codecs;

   if (!validCodecs()) {
      return codecs;
   }

   for (int i = 0; i < all->size(); ++i) {
      codecs += all->at(i)->name();
      codecs += all->at(i)->aliases();
   }

   locker.unlock();


#if ! defined(QT_NO_TEXTCODECPLUGIN)
   QFactoryLoader *l = loader();
   QStringList keys = l->keys();
   for (int i = 0; i < keys.size(); ++i) {
      if (!keys.at(i).startsWith(QLatin1String("MIB: "))) {
         QByteArray name = keys.at(i).toLatin1();
         if (!codecs.contains(name)) {
            codecs += name;
         }
      }
   }
#endif

   return codecs;
}

/*!
    Returns the list of MIBs for all available codecs. Call
    QTextCodec::codecForMib() to obtain the QTextCodec for the MIB.

    \sa availableCodecs(), mibEnum()
*/
QList<int> QTextCodec::availableMibs()
{
   QMutexLocker locker(textCodecsMutex());
   setup();

   QList<int> codecs;

   if (!validCodecs()) {
      return codecs;
   }

   for (int i = 0; i < all->size(); ++i) {
      codecs += all->at(i)->mibEnum();
   }

   locker.unlock();


#if ! defined(QT_NO_TEXTCODECPLUGIN)
   QFactoryLoader *l = loader();
   QStringList keys = l->keys();
   for (int i = 0; i < keys.size(); ++i) {
      if (keys.at(i).startsWith(QLatin1String("MIB: "))) {
         int mib = keys.at(i).mid(5).toInt();
         if (!codecs.contains(mib)) {
            codecs += mib;
         }
      }
   }
#endif

   return codecs;
}

/*!
    Set the codec to \a c; this will be returned by
    codecForLocale(). If \a c is a null pointer, the codec is reset to
    the default.

    This might be needed for some applications that want to use their
    own mechanism for setting the locale.

    \sa codecForLocale()
*/
void QTextCodec::setCodecForLocale(QTextCodec *c)
{
   QMutexLocker locker(textCodecsMutex());

   localeMapper = c;
   if (!localeMapper) {
      setupLocaleMapper();
   }
}

/*!
    Returns a pointer to the codec most suitable for this locale.

    On Windows, the codec will be based on a system locale. On Unix
    systems, starting with Qt 4.2, the codec will be using the \e
    iconv library. Note that in both cases the codec's name will be
    "System".
*/

QTextCodec *QTextCodec::codecForLocale()
{
   if (!validCodecs()) {
      return 0;
   }

   if (localeMapper) {
      return localeMapper;
   }

   QMutexLocker locker(textCodecsMutex());
   setup();

   return localeMapper;
}


/*!
    \fn QByteArray QTextCodec::name() const

    QTextCodec subclasses must reimplement this function. It returns
    the name of the encoding supported by the subclass.

    If the codec is registered as a character set in the
    \l{IANA character-sets encoding file} this method should
    return the preferred mime name for the codec if defined,
    otherwise its name.
*/

/*!
    \fn int QTextCodec::mibEnum() const

    Subclasses of QTextCodec must reimplement this function. It
    returns the MIBenum (see \l{IANA character-sets encoding file}
    for more information). It is important that each QTextCodec
    subclass returns the correct unique value for this function.
*/

/*!
  Subclasses can return a number of aliases for the codec in question.

  Standard aliases for codecs can be found in the
  \l{IANA character-sets encoding file}.
*/
QList<QByteArray> QTextCodec::aliases() const
{
   return QList<QByteArray>();
}

/*!
    \fn QString QTextCodec::convertToUnicode(const char *chars, int len,
                                             ConverterState *state) const

    QTextCodec subclasses must reimplement this function.

    Converts the first \a len characters of \a chars from the
    encoding of the subclass to Unicode, and returns the result in a
    QString.

    \a state can be 0, in which case the conversion is stateless and
    default conversion rules should be used. If state is not 0, the
    codec should save the state after the conversion in \a state, and
    adjust the remainingChars and invalidChars members of the struct.
*/

/*!
    \fn QByteArray QTextCodec::convertFromUnicode(const QChar *input, int number,
                                                  ConverterState *state) const

    QTextCodec subclasses must reimplement this function.

    Converts the first \a number of characters from the \a input array
    from Unicode to the encoding of the subclass, and returns the result
    in a QByteArray.

    \a state can be 0 in which case the conversion is stateless and
    default conversion rules should be used. If state is not 0, the
    codec should save the state after the conversion in \a state, and
    adjust the remainingChars and invalidChars members of the struct.
*/

/*!
    Creates a QTextDecoder which stores enough state to decode chunks
    of \c{char *} data to create chunks of Unicode data.

    The caller is responsible for deleting the returned object.
*/
QTextDecoder *QTextCodec::makeDecoder() const
{
   return new QTextDecoder(this);
}

/*!
    Creates a QTextDecoder with a specified \a flags to decode chunks
    of \c{char *} data to create chunks of Unicode data.

    The caller is responsible for deleting the returned object.

    \since 4.7
*/
QTextDecoder *QTextCodec::makeDecoder(QTextCodec::ConversionFlags flags) const
{
   return new QTextDecoder(this, flags);
}


/*!
    Creates a QTextEncoder which stores enough state to encode chunks
    of Unicode data as \c{char *} data.

    The caller is responsible for deleting the returned object.
*/
QTextEncoder *QTextCodec::makeEncoder() const
{
   return new QTextEncoder(this);
}

/*!
    Creates a QTextEncoder with a specified \a flags to encode chunks
    of Unicode data as \c{char *} data.

    The caller is responsible for deleting the returned object.

    \since 4.7
*/
QTextEncoder *QTextCodec::makeEncoder(QTextCodec::ConversionFlags flags) const
{
   return new QTextEncoder(this, flags);
}

/*!
    \fn QByteArray QTextCodec::fromUnicode(const QChar *input, int number,
                                           ConverterState *state) const

    Converts the first \a number of characters from the \a input array
    from Unicode to the encoding of this codec, and returns the result
    in a QByteArray.

    The \a state of the convertor used is updated.
*/

/*!
    Converts \a str from Unicode to the encoding of this codec, and
    returns the result in a QByteArray.
*/
QByteArray QTextCodec::fromUnicode(const QString &str) const
{
   return convertFromUnicode(str.constData(), str.length(), 0);
}

/*!
    \fn QString QTextCodec::toUnicode(const char *input, int size,
                                      ConverterState *state) const

    Converts the first \a size characters from the \a input from the
    encoding of this codec to Unicode, and returns the result in a
    QString.

    The \a state of the convertor used is updated.
*/

/*!
    Converts \a a from the encoding of this codec to Unicode, and
    returns the result in a QString.
*/
QString QTextCodec::toUnicode(const QByteArray &a) const
{
   return convertToUnicode(a.constData(), a.length(), 0);
}

/*!
    Returns true if the Unicode character \a ch can be fully encoded
    with this codec; otherwise returns false.
*/
bool QTextCodec::canEncode(QChar ch) const
{
   ConverterState state;
   state.flags = ConvertInvalidToNull;
   convertFromUnicode(&ch, 1, &state);
   return (state.invalidChars == 0);
}

/*!
    \overload

    \a s contains the string being tested for encode-ability.
*/
bool QTextCodec::canEncode(const QString &s) const
{
   ConverterState state;
   state.flags = ConvertInvalidToNull;
   convertFromUnicode(s.constData(), s.length(), &state);
   return (state.invalidChars == 0);
}


/*!
    \overload

    \a chars contains the source characters.
*/
QString QTextCodec::toUnicode(const char *chars) const
{
   int len = qstrlen(chars);
   return convertToUnicode(chars, len, 0);
}


/*!
    \class QTextEncoder
    \brief The QTextEncoder class provides a state-based encoder.
    \reentrant
    \ingroup i18n

    A text encoder converts text from Unicode into an encoded text format
    using a specific codec.

    The encoder converts Unicode into another format, remembering any
    state that is required between calls.

    \sa QTextCodec::makeEncoder(), QTextDecoder
*/

/*!
    \fn QTextEncoder::QTextEncoder(const QTextCodec *codec)

    Constructs a text encoder for the given \a codec.
*/

/*!
    Constructs a text encoder for the given \a codec and conversion \a flags.

    \since 4.7
*/
QTextEncoder::QTextEncoder(const QTextCodec *codec, QTextCodec::ConversionFlags flags)
   : c(codec), state()
{
   state.flags = flags;
}

/*!
    Destroys the encoder.
*/
QTextEncoder::~QTextEncoder()
{
}

/*! \internal
    \since 4.5
    Determines whether the eecoder encountered a failure while decoding the input. If
    an error was encountered, the produced result is undefined, and gets converted as according
    to the conversion flags.
 */
bool QTextEncoder::hasFailure() const
{
   return state.invalidChars != 0;
}

/*!
    Converts the Unicode string \a str into an encoded QByteArray.
*/
QByteArray QTextEncoder::fromUnicode(const QString &str)
{
   QByteArray result = c->fromUnicode(str.constData(), str.length(), &state);
   return result;
}

/*!
    \overload

    Converts \a len characters (not bytes) from \a uc, and returns the
    result in a QByteArray.
*/
QByteArray QTextEncoder::fromUnicode(const QChar *uc, int len)
{
   QByteArray result = c->fromUnicode(uc, len, &state);
   return result;
}

/*!
    \class QTextDecoder
    \brief The QTextDecoder class provides a state-based decoder.
    \reentrant
    \ingroup i18n

    A text decoder converts text from an encoded text format into Unicode
    using a specific codec.

    The decoder converts text in this format into Unicode, remembering any
    state that is required between calls.

    \sa QTextCodec::makeDecoder(), QTextEncoder
*/

/*!
    \fn QTextDecoder::QTextDecoder(const QTextCodec *codec)

    Constructs a text decoder for the given \a codec.
*/

/*!
    Constructs a text decoder for the given \a codec and conversion \a flags.

    \since 4.7
*/

QTextDecoder::QTextDecoder(const QTextCodec *codec, QTextCodec::ConversionFlags flags)
   : c(codec), state()
{
   state.flags = flags;
}

/*!
    Destroys the decoder.
*/
QTextDecoder::~QTextDecoder()
{
}

/*!
    \fn QString QTextDecoder::toUnicode(const char *chars, int len)

    Converts the first \a len bytes in \a chars to Unicode, returning
    the result.

    If not all characters are used (e.g. if only part of a multi-byte
    encoding is at the end of the characters), the decoder remembers
    enough state to continue with the next call to this function.
*/
QString QTextDecoder::toUnicode(const char *chars, int len)
{
   return c->toUnicode(chars, len, &state);
}


/*! \overload

    The converted string is returned in \a target.
 */
void QTextDecoder::toUnicode(QString *target, const char *chars, int len)
{
   Q_ASSERT(target);
   switch (c->mibEnum()) {
      case 106: // utf8
         static_cast<const QUtf8Codec *>(c)->convertToUnicode(target, chars, len, &state);
         break;
      case 4: { // latin1
         target->resize(len);
         ushort *data = (ushort *)target->data();
         for (int i = len; i >= 0; --i) {
            data[i] = (uchar) chars[i];
         }
      }
      break;
      default:
         *target = c->toUnicode(chars, len, &state);
   }
}


/*!
    \overload

    Converts the bytes in the byte array specified by \a ba to Unicode
    and returns the result.
*/
QString QTextDecoder::toUnicode(const QByteArray &ba)
{
   return c->toUnicode(ba.constData(), ba.length(), &state);
}

QTextCodec *QTextCodec::codecForHtml(const QByteArray &ba, QTextCodec *defaultCodec)
{
   // determine charset
   int pos;
   QTextCodec *c = 0;

   c = QTextCodec::codecForUtfText(ba, c);
   if (!c) {
      QByteArray header = ba.left(512).toLower();
      if ((pos = header.indexOf("http-equiv=")) != -1) {
         if ((pos = header.lastIndexOf("meta ", pos)) != -1) {
            pos = header.indexOf("charset=", pos) + int(strlen("charset="));
            if (pos != -1) {
               int pos2 = header.indexOf('\"', pos + 1);
               QByteArray cs = header.mid(pos, pos2 - pos);
               //            qDebug("found charset: %s", cs.data());
               c = QTextCodec::codecForName(cs);
            }
         }
      }
   }
   if (!c) {
      c = defaultCodec;
   }

   return c;
}

/*!
    \overload

    Tries to detect the encoding of the provided snippet of HTML in
    the given byte array, \a ba, by checking the BOM (Byte Order Mark)
    and the content-type meta header and returns a QTextCodec instance
    that is capable of decoding the html to unicode. If the codec cannot
    be detected, this overload returns a Latin-1 QTextCodec.
*/
QTextCodec *QTextCodec::codecForHtml(const QByteArray &ba)
{
   return codecForHtml(ba, QTextCodec::codecForMib(/*Latin 1*/ 4));
}

/*!
    \since 4.6

    Tries to detect the encoding of the provided snippet \a ba by
    using the BOM (Byte Order Mark) and returns a QTextCodec instance
    that is capable of decoding the text to unicode. If the codec
    cannot be detected from the content provided, \a defaultCodec is
    returned.

    \sa codecForHtml()
*/
QTextCodec *QTextCodec::codecForUtfText(const QByteArray &ba, QTextCodec *defaultCodec)
{
   const int arraySize = ba.size();

   if (arraySize > 3) {
      if ((uchar)ba[0] == 0x00
            && (uchar)ba[1] == 0x00
            && (uchar)ba[2] == 0xFE
            && (uchar)ba[3] == 0xFF) {
         return QTextCodec::codecForMib(1018);   // utf-32 be
      } else if ((uchar)ba[0] == 0xFF
                 && (uchar)ba[1] == 0xFE
                 && (uchar)ba[2] == 0x00
                 && (uchar)ba[3] == 0x00) {
         return QTextCodec::codecForMib(1019);   // utf-32 le
      }
   }

   if (arraySize < 2) {
      return defaultCodec;
   }
   if ((uchar)ba[0] == 0xfe && (uchar)ba[1] == 0xff) {
      return QTextCodec::codecForMib(1013);   // utf16 be
   } else if ((uchar)ba[0] == 0xff && (uchar)ba[1] == 0xfe) {
      return QTextCodec::codecForMib(1014);   // utf16 le
   }

   if (arraySize < 3) {
      return defaultCodec;
   }
   if ((uchar)ba[0] == 0xef
         && (uchar)ba[1] == 0xbb
         && (uchar)ba[2] == 0xbf) {
      return QTextCodec::codecForMib(106);   // utf-8
   }

   return defaultCodec;
}

/*!
    \overload

    Tries to detect the encoding of the provided snippet \a ba by
    using the BOM (Byte Order Mark) and returns a QTextCodec instance
    that is capable of decoding the text to unicode. If the codec
    cannot be detected, this overload returns a Latin-1 QTextCodec.

    \sa codecForHtml()
*/
QTextCodec *QTextCodec::codecForUtfText(const QByteArray &ba)
{
   return codecForUtfText(ba, QTextCodec::codecForMib(/*Latin 1*/ 4));
}


/*! \internal
    \since 4.3
    Determines whether the decoder encountered a failure while decoding the input. If
    an error was encountered, the produced result is undefined, and gets converted as according
    to the conversion flags.
 */
bool QTextDecoder::hasFailure() const
{
   return state.invalidChars != 0;
}

/*!
    \fn QTextCodec *QTextCodec::codecForContent(const char *str, int size)

    This functionality is no longer provided by Qt. This
    compatibility function always returns a null pointer.
*/

/*!
    \fn QTextCodec *QTextCodec::codecForName(const char *hint, int accuracy)

    Use the codecForName(const QByteArray &) overload instead.
*/

/*!
    \fn QTextCodec *QTextCodec::codecForIndex(int i)

    Use availableCodecs() or availableMibs() instead and iterate
    through the resulting list.
*/


/*!
    \fn QByteArray QTextCodec::mimeName() const

    Use name() instead.
*/

QT_END_NAMESPACE

#endif // QT_NO_TEXTCODEC
