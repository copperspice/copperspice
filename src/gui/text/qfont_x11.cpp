/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#define QT_FATAL_ASSERT

#include <qplatformdefs.h>
#include <qfont.h>
#include <qapplication.h>
#include <qfontinfo.h>
#include <qfontdatabase.h>
#include <qfontmetrics.h>
#include <qpaintdevice.h>
#include <qtextcodec.h>
#include <qiodevice.h>
#include <qhash.h>

#include <qunicodetables_p.h>
#include <qfont_p.h>
#include <qfontengine_p.h>
#include <qfontengine_x11_p.h>
#include <qtextengine_p.h>

#include <qt_x11_p.h>
#include <qx11info_x11.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#define QFONTLOADER_DEBUG
#define QFONTLOADER_DEBUG_VERBOSE

QT_BEGIN_NAMESPACE

double qt_pixelSize(double pointSize, int dpi)
{
   if (pointSize < 0) {
      return -1.;
   }
   if (dpi == 75) { // the stupid 75 dpi setting on X11
      dpi = 72;
   }
   return (pointSize * dpi) / 72.;
}

double qt_pointSize(double pixelSize, int dpi)
{
   if (pixelSize < 0) {
      return -1.;
   }
   if (dpi == 75) { // the stupid 75 dpi setting on X11
      dpi = 72;
   }
   return pixelSize * 72. / ((double) dpi);
}

/*
  Removes wildcards from an XLFD.

  Returns \a xlfd with all wildcards removed if a match for \a xlfd is
  found, otherwise it returns \a xlfd.
*/
static QByteArray qt_fixXLFD(const QByteArray &xlfd)
{
   QByteArray ret = xlfd;
   int count = 0;
   char **fontNames = XListFonts(QX11Info::display(), xlfd.constData(), 32768, &count);

   if (count > 0) {
      ret = fontNames[0];
   }

   XFreeFontNames(fontNames);
   return ret ;
}

typedef QHash<int, QString> FallBackHash;
Q_GLOBAL_STATIC(FallBackHash, fallBackHash)

// Returns the user-configured fallback family for the specified script.
QString qt_fallback_font_family(int script)
{
   FallBackHash *hash = fallBackHash();
   return hash->value(script);
}

// Sets the fallback family for the specified script.
Q_GUI_EXPORT void qt_x11_set_fallback_font_family(int script, const QString &family)
{
   FallBackHash *hash = fallBackHash();
   if (!family.isEmpty()) {
      hash->insert(script, family);
   } else {
      hash->remove(script);
   }
}

int QFontPrivate::defaultEncodingID = -1;

void QFont::initialize()
{
   extern int qt_encoding_id_for_mib(int mib); // from qfontdatabase_x11.cpp
   QTextCodec *codec = QTextCodec::codecForLocale();
   // determine the default encoding id using the locale, otherwise
   // fallback to latin1 (mib == 4)
   int mib = codec ? codec->mibEnum() : 4;

   // for asian locales, use the mib for the font codec instead of the locale codec
   switch (mib) {
      case 38: // eucKR
         mib = 36;
         break;

      case 2025: // GB2312
         mib = 57;
         break;

      case 113: // GBK
         mib = -113;
         break;

      case 114: // GB18030
         mib = -114;
         break;

      case 2026: // Big5
         mib = -2026;
         break;

      case 2101: // Big5-HKSCS
         mib = -2101;
         break;

      case 16: // JIS7
         mib = 15;
         break;

      case 17: // SJIS
      case 18: // eucJP
         mib = 63;
         break;
   }

   // get the default encoding id for the locale encoding...
   QFontPrivate::defaultEncodingID = qt_encoding_id_for_mib(mib);
}

void QFont::cleanup()
{
   QFontCache::cleanup();
}

/*!
  \internal
  X11 Only: Returns the screen with which this font is associated.
*/
int QFont::x11Screen() const
{
   return d->screen;
}

/*! \internal
    X11 Only: Associate the font with the specified \a screen.
*/
void QFont::x11SetScreen(int screen)
{
   if (screen < 0) { // assume default
      screen = QX11Info::appScreen();
   }

   if (screen == d->screen) {
      return;   // nothing to do
   }

   detach();
   d->screen = screen;
}

Qt::HANDLE QFont::handle() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);
   if (engine->type() == QFontEngine::Multi) {
      engine = static_cast<QFontEngineMulti *>(engine)->engine(0);
   }
   if (engine->type() == QFontEngine::XLFD) {
      return static_cast<QFontEngineXLFD *>(engine)->fontStruct()->fid;
   }
   return 0;
}


FT_Face QFont::freetypeFace() const
{
#ifndef QT_NO_FREETYPE
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);

   if (engine->type() == QFontEngine::Multi) {
      engine = static_cast<QFontEngineMulti *>(engine)->engine(0);
   }

   if (engine->type() == QFontEngine::Freetype) {
      const QFontEngineFT *ft = static_cast<const QFontEngineFT *>(engine);
      return ft->non_locked_face();

   } else {
      if (engine->type() == QFontEngine::XLFD) {
         const QFontEngineXLFD *xlfd = static_cast<const QFontEngineXLFD *>(engine);
         return xlfd->non_locked_face();
      }

   }
#endif

   return 0;
}

QString QFont::rawName() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);

   if (engine->type() == QFontEngine::Multi) {
      engine = static_cast<QFontEngineMulti *>(engine)->engine(0);
   }

   if (engine->type() == QFontEngine::XLFD) {
      return engine->fontEngineName();
   }

   return QString();
}
struct QtFontDesc;

void QFont::setRawName(const QString &name)
{
   detach();

   // from qfontdatabase_x11.cpp
   extern bool qt_fillFontDef(const QByteArray & xlfd, QFontDef * fd, int dpi, QtFontDesc * desc);

   if (! qt_fillFontDef(qt_fixXLFD(name.toLatin1()), &d->request, d->dpi, 0)) {
      qWarning("QFont::setRawName: Invalid XLFD: \"%s\"", csPrintable(name));

      setFamily(name);
      setRawMode(true);

   } else {
      resolve_mask = QFont::AllPropertiesResolved;
   }
}

QString QFont::lastResortFamily() const
{
   return QString::fromLatin1("Helvetica");
}

QString QFont::defaultFamily() const
{
   switch (d->request.styleHint) {
      case QFont::Times:
         return QString::fromLatin1("Times");

      case QFont::Courier:
         return QString::fromLatin1("Courier");

      case QFont::Monospace:
         return QString::fromLatin1("Courier New");

      case QFont::Cursive:
         return QString::fromLatin1("Comic Sans MS");

      case QFont::Fantasy:
         return QString::fromLatin1("Impact");

      case QFont::Decorative:
         return QString::fromLatin1("Old English");

      case QFont::Helvetica:
      case QFont::System:
      default:
         return QString::fromLatin1("Helvetica");
   }
}

/*
  Returns a last resort raw font name for the font matching algorithm.
  This is used if even the last resort family is not available. It
  returns \e something, almost no matter what.  The current
  implementation tries a wide variety of common fonts, returning the
  first one it finds. The implementation may change at any time.
*/
static const char *const tryFonts[] = {
   "-*-helvetica-medium-r-*-*-*-120-*-*-*-*-*-*",
   "-*-courier-medium-r-*-*-*-120-*-*-*-*-*-*",
   "-*-times-medium-r-*-*-*-120-*-*-*-*-*-*",
   "-*-lucida-medium-r-*-*-*-120-*-*-*-*-*-*",
   "-*-helvetica-*-*-*-*-*-120-*-*-*-*-*-*",
   "-*-courier-*-*-*-*-*-120-*-*-*-*-*-*",
   "-*-times-*-*-*-*-*-120-*-*-*-*-*-*",
   "-*-lucida-*-*-*-*-*-120-*-*-*-*-*-*",
   "-*-helvetica-*-*-*-*-*-*-*-*-*-*-*-*",
   "-*-courier-*-*-*-*-*-*-*-*-*-*-*-*",
   "-*-times-*-*-*-*-*-*-*-*-*-*-*-*",
   "-*-lucida-*-*-*-*-*-*-*-*-*-*-*-*",
   "-*-fixed-*-*-*-*-*-*-*-*-*-*-*-*",
   "6x13",
   "7x13",
   "8x13",
   "9x15",
   "fixed",
   0
};

// Returns true if the font exists, false otherwise
static bool fontExists(const QString &fontName)
{
   int count;
   char **fontNames = XListFonts(QX11Info::display(), (char *)fontName.toLatin1().constData(), 32768, &count);

   if (fontNames) {
      XFreeFontNames(fontNames);
   }

   return count != 0;
}

QString QFont::lastResortFont() const
{
   static QString last;

   // already found
   if (! last.empty()) {
      return last;
   }

   int i = 0;
   const char *f;

   while ((f = tryFonts[i])) {
      last = QString::fromLatin1(f);

      if (fontExists(last)) {
         return last;
      }

      i++;
   }

#if defined(CHECK_NULL)
   qFatal("QFontPrivate::lastResortFont: Cannot find any reasonable font");
#endif
   return last;
}

QT_END_NAMESPACE
