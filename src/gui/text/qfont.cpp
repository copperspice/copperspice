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

#include <qfont.h>
#include <qdebug.h>
#include <qpaintdevice.h>
#include <qfontdatabase.h>
#include <qfontmetrics.h>
#include <qfontinfo.h>
#include <qpainter.h>
#include <qhash.h>
#include <qdatastream.h>
#include <qapplication.h>
#include <qstringlist.h>
#include <qscreen.h>

#include <qthread.h>
#include <qthreadstorage.h>
#include <qplatform_screen.h>
#include <qplatform_integration.h>
#include <qplatform_fontdatabase.h>

#include <qmutexlocker.h>
#include <qmutex.h>

#include <qapplication_p.h>
#include <qfont_p.h>
#include <qfontengine_p.h>
#include <qpainter_p.h>
#include <qtextengine_p.h>
#include <qunicodetables_p.h>

#include <limits.h>

static constexpr const uint MinCacheSize = 4 * 1024;        // 4 mb

#ifndef QFONTCACHE_DECREASE_TRIGGER_LIMIT
#  define QFONTCACHE_DECREASE_TRIGGER_LIMIT 256
#endif

#if defined(CS_SHOW_DEBUG_GUI_TEXT)
// fast timeouts for debugging
static constexpr const int fast_timeout =   1000;  // 1s
static constexpr const int slow_timeout =   5000;  // 5s

#else
static constexpr const int fast_timeout =  10000;  // 10s
static constexpr const int slow_timeout = 300000;  //  5m

#endif

bool QFontDef::exactMatch(const QFontDef &other) const
{
   /*
     QFontDef comparison is more complicated than just simple
     per-member comparisons.

     When comparing point/pixel sizes, either point or pixelsize
     could be -1.  in This case we have to compare the non negative
     size value.

     This test will fail if the point-sizes differ by 1/2 point or
     more or they do not round to the same value.  We have to do this
     since our API still uses 'int' point-sizes in the API, but store
     deci-point-sizes internally.

     To compare the family members, we need to parse the font names
     and compare the family/foundry strings separately.  This allows
     us to compare e.g. "Helvetica" and "Helvetica [Adobe]" with
     positive results.
   */
   if (pixelSize != -1 && other.pixelSize != -1) {
      if (pixelSize != other.pixelSize) {
         return false;
      }

   } else if (pointSize != -1 && other.pointSize != -1) {
      if (pointSize != other.pointSize) {
         return false;
      }

   } else {
      return false;
   }

   if (!ignorePitch && !other.ignorePitch && fixedPitch != other.fixedPitch) {
      return false;
   }

   if (stretch != 0 && other.stretch != 0 && stretch != other.stretch) {
      return false;
   }

   QString this_family, this_foundry, other_family, other_foundry;
   QFontDatabase::parseFontName(family, this_foundry, this_family);
   QFontDatabase::parseFontName(other.family, other_foundry, other_family);

   this_family = QFontDatabase::resolveFontFamilyAlias(this_family);
   other_family = QFontDatabase::resolveFontFamilyAlias(other_family);

   return (styleHint     == other.styleHint
         && styleStrategy == other.styleStrategy
         && weight        == other.weight
         && style         == other.style
         && this_family   == other_family
         && (styleName.isEmpty() || other.styleName.isEmpty() || styleName == other.styleName)
         && (this_foundry.isEmpty() || other_foundry.isEmpty() || this_foundry == other_foundry) );
}

Q_GUI_EXPORT int qt_defaultDpiX()
{
   if (QCoreApplication::instance()->testAttribute(Qt::AA_Use96Dpi)) {
      return 96;
   }

   if (! qApp->cs_isRealGuiApp()) {
      return 75;
   }

   if (const QScreen *screen = QGuiApplication::primaryScreen()) {
      return qRound(screen->logicalDotsPerInchX());
   }

   // PI has not been initialised, or it is being initialised. Give a default dpi
   return 100;
}

Q_GUI_EXPORT int qt_defaultDpiY()
{
   if (QCoreApplication::instance()->testAttribute(Qt::AA_Use96Dpi)) {
      return 96;
   }

   if (! qApp->cs_isRealGuiApp()) {
      return 75;
   }

   if (const QScreen *screen = QGuiApplication::primaryScreen()) {
      return qRound(screen->logicalDotsPerInchY());
   }

   //PI has not been initialised, or it is being initialised. Give a default dpi
   return 100;
}

Q_GUI_EXPORT int qt_defaultDpi()
{
   return qt_defaultDpiY();
}

QFontPrivate::QFontPrivate()
   : engineData(nullptr), dpi(qt_defaultDpi()), screen(0),
     underline(false), overline(false), strikeOut(false), kerning(true),
     capital(0), letterSpacingIsAbsolute(false), scFont(nullptr)
{
}

QFontPrivate::QFontPrivate(const QFontPrivate &other)
   : request(other.request), engineData(nullptr), dpi(other.dpi), screen(other.screen),
     underline(other.underline), overline(other.overline),
     strikeOut(other.strikeOut), kerning(other.kerning),
     capital(other.capital), letterSpacingIsAbsolute(other.letterSpacingIsAbsolute),
     letterSpacing(other.letterSpacing), wordSpacing(other.wordSpacing), scFont(other.scFont)
{
   if (scFont && scFont != this) {
      scFont->ref.ref();
   }
}

QFontPrivate::~QFontPrivate()
{
   if (engineData && ! engineData->m_refCount.deref()) {
      delete engineData;
   }

   engineData = nullptr;
   if (scFont && scFont != this) {
      scFont->ref.deref();
   }

   scFont = nullptr;
}

extern QRecursiveMutex *qt_fontdatabase_mutex();

#define QT_FONT_ENGINE_FROM_DATA(data, script) data->engines[script]

QFontEngine *QFontPrivate::engineForScript(int script) const
{
   QRecursiveMutexLocker locker(qt_fontdatabase_mutex());

   if (script <= QChar::Script_Latin) {
      script = QChar::Script_Common;
   }

   if (engineData && engineData->fontCacheId != QFontCache::instance()->id()) {
      // throw out engineData that came from a different thread
      if (! engineData->m_refCount.deref()) {
         delete engineData;
      }
      engineData = nullptr;
   }

   if (! engineData || ! QT_FONT_ENGINE_FROM_DATA(engineData, script)) {
      QFontDatabase::load(this, script);
   }

   return QT_FONT_ENGINE_FROM_DATA(engineData, script);
}

void QFontPrivate::alterCharForCapitalization(QChar &c) const
{
   switch (capital) {
      case QFont::AllUppercase:
      case QFont::SmallCaps:
         c = c.toUpper()[0];
         break;

      case QFont::AllLowercase:
         c = c.toLower()[0];
         break;

      case QFont::MixedCase:
         break;
   }
}

QFontPrivate *QFontPrivate::smallCapsFontPrivate() const
{
   if (scFont) {
      return scFont;
   }

   QFont font(const_cast<QFontPrivate *>(this));
   qreal pointSize = font.pointSizeF();

   if (pointSize > 0) {
      font.setPointSizeF(pointSize * .7);
   } else {
      font.setPixelSize((font.pixelSize() * 7 + 5) / 10);
   }

   scFont = font.d.data();
   if (scFont != this) {
      scFont->ref.ref();
   }
   return scFont;
}

void QFontPrivate::resolve(uint mask, const QFontPrivate *other)
{
   Q_ASSERT(other != nullptr);

   dpi = other->dpi;

   if ((mask & QFont::AllPropertiesResolved) == QFont::AllPropertiesResolved) {
      return;
   }

   // assign the unset-bits with the set-bits of the other font def
   if (! (mask & QFont::FamilyResolved)) {
      request.family = other->request.family;
   }

   if (! (mask & QFont::StyleNameResolved)) {
      request.styleName = other->request.styleName;
   }

   if (! (mask & QFont::SizeResolved)) {
      request.pointSize = other->request.pointSize;
      request.pixelSize = other->request.pixelSize;
   }

   if (! (mask & QFont::StyleHintResolved)) {
      request.styleHint = other->request.styleHint;
   }

   if (! (mask & QFont::StyleStrategyResolved)) {
      request.styleStrategy = other->request.styleStrategy;
   }

   if (! (mask & QFont::WeightResolved)) {
      request.weight = other->request.weight;
   }

   if (! (mask & QFont::StyleResolved)) {
      request.style = other->request.style;
   }

   if (! (mask & QFont::FixedPitchResolved)) {
      request.fixedPitch = other->request.fixedPitch;
   }

   if (! (mask & QFont::StretchResolved)) {
      request.stretch = other->request.stretch;
   }

   if (! (mask & QFont::HintingPreferenceResolved)) {
      request.hintingPreference = other->request.hintingPreference;
   }

   if (! (mask & QFont::UnderlineResolved)) {
      underline = other->underline;
   }

   if (! (mask & QFont::OverlineResolved)) {
      overline = other->overline;
   }

   if (! (mask & QFont::StrikeOutResolved)) {
      strikeOut = other->strikeOut;
   }

   if (! (mask & QFont::KerningResolved)) {
      kerning = other->kerning;
   }

   if (! (mask & QFont::LetterSpacingResolved)) {
      letterSpacing = other->letterSpacing;
      letterSpacingIsAbsolute = other->letterSpacingIsAbsolute;
   }
   if (! (mask & QFont::WordSpacingResolved)) {
      wordSpacing = other->wordSpacing;
   }

   if (! (mask & QFont::CapitalizationResolved)) {
      capital = other->capital;
   }
}

QFontEngineData::QFontEngineData()
   : m_refCount(0), fontCacheId(QFontCache::instance()->id())
{
   memset(engines, 0, QChar::ScriptCount * sizeof(QFontEngine *));
}

QFontEngineData::~QFontEngineData()
{
   Q_ASSERT(m_refCount.load() == 0);

   for (int i = 0; i < QChar::ScriptCount; ++i) {
      if (engines[i]) {
         if (! engines[i]->m_refCount.deref()) {
            delete engines[i];
         }

         engines[i] = nullptr;
      }
   }
}

QFont::QFont(const QFont &font, QPaintDevice *pd)
   : resolve_mask(font.resolve_mask)
{
   Q_ASSERT(pd != nullptr);
   int dpi = pd->logicalDpiY();

   const int screen = 0;

   if (font.d->dpi != dpi || font.d->screen != screen ) {
      d = new QFontPrivate(*font.d);
      d->dpi = dpi;
      d->screen = screen;
   } else {
      d = font.d.data();
   }
}

// internal
QFont::QFont(QFontPrivate *data)
   : d(data), resolve_mask(QFont::AllPropertiesResolved)
{
}

// internal
void QFont::detach()
{
   if (d->ref.load() == 1) {
      if (d->engineData && !d->engineData->m_refCount.deref()) {
         delete d->engineData;
      }

      d->engineData = nullptr;

      if (d->scFont && d->scFont != d.data()) {
         d->scFont->ref.deref();
      }

      d->scFont = nullptr;
      return;
   }

   d.detach();
}

void QFontPrivate::detachButKeepEngineData(QFont *font)
{
   if (font->d->ref.load() == 1) {
      return;
   }

   QFontEngineData *engineData = font->d->engineData;
   if (engineData) {
      engineData->m_refCount.ref();
   }

   font->d.detach();
   font->d->engineData = engineData;
}

QFont::QFont()
   : d(QGuiApplicationPrivate::instance() ? QGuiApplication::font().d.data()
        : new QFontPrivate()), resolve_mask(0)
{
}

QFont::QFont(const QString &family, int pointSize, int weight, bool italic)
   : d(new QFontPrivate()), resolve_mask(QFont::FamilyResolved)
{
   if (pointSize <= 0) {
      pointSize = 12;

   } else {
      resolve_mask |= QFont::SizeResolved;
   }

   if (weight < 0) {
      weight = Normal;
   } else {
      resolve_mask |= QFont::WeightResolved | QFont::StyleResolved;
   }

   if (italic) {
      resolve_mask |= QFont::StyleResolved;
   }

   d->request.family = family;
   d->request.pointSize = qreal(pointSize);
   d->request.pixelSize = -1;
   d->request.weight = weight;
   d->request.style = italic ? QFont::StyleItalic : QFont::StyleNormal;
}


QFont::QFont(const QFont &font)
   : d(font.d.data()), resolve_mask(font.resolve_mask)
{
}

QFont::~QFont()
{
}


QFont &QFont::operator=(const QFont &font)
{
   d = font.d.data();
   resolve_mask = font.resolve_mask;
   return *this;
}

QString QFont::family() const
{
   return d->request.family;
}

void QFont::setFamily(const QString &family)
{
   if ((resolve_mask & QFont::FamilyResolved) && d->request.family == family) {
      return;
   }
   detach();

   d->request.family = family;

   resolve_mask |= QFont::FamilyResolved;
}

QString QFont::styleName() const
{
   return d->request.styleName;
}

void QFont::setStyleName(const QString &styleName)
{
   if ((resolve_mask & QFont::StyleNameResolved) && d->request.styleName == styleName) {
      return;
   }

   detach();

   d->request.styleName = styleName;
   resolve_mask |= QFont::StyleNameResolved;
}

int QFont::pointSize() const
{
   return qRound(d->request.pointSize);
}

void QFont::setHintingPreference(HintingPreference hintingPreference)
{
   if ((resolve_mask & QFont::HintingPreferenceResolved) && d->request.hintingPreference == hintingPreference) {
      return;
   }
   detach();

   d->request.hintingPreference = hintingPreference;

   resolve_mask |= QFont::HintingPreferenceResolved;
}


QFont::HintingPreference QFont::hintingPreference() const
{
   return QFont::HintingPreference(d->request.hintingPreference);
}

void QFont::setPointSize(int pointSize)
{
   if (pointSize <= 0) {
      qWarning("QFont::setPointSize() Point size of %d must be greater than 0", pointSize);
      return;
   }

   if ((resolve_mask & QFont::SizeResolved) && d->request.pointSize == qreal(pointSize)) {
      return;
   }
   detach();

   d->request.pointSize = qreal(pointSize);
   d->request.pixelSize = -1;

   resolve_mask |= QFont::SizeResolved;
}

void QFont::setPointSizeF(qreal pointSize)
{
   if (pointSize <= 0) {
      qWarning("QFont::setPointSizeF() Point size of %f must be greater than 0", pointSize);
      return;
   }

   if ((resolve_mask & QFont::SizeResolved) && d->request.pointSize == pointSize) {
      return;
   }
   detach();

   d->request.pointSize = pointSize;
   d->request.pixelSize = -1;

   resolve_mask |= QFont::SizeResolved;
}

qreal QFont::pointSizeF() const
{
   return d->request.pointSize;
}

void QFont::setPixelSize(int pixelSize)
{
   if (pixelSize <= 0) {
      qWarning("QFont::setPixelSize() Pixel size of %d must be greater than 0", pixelSize);
      return;
   }

   if ((resolve_mask & QFont::SizeResolved) && d->request.pixelSize == qreal(pixelSize)) {
      return;
   }
   detach();

   d->request.pixelSize = pixelSize;
   d->request.pointSize = -1;

   resolve_mask |= QFont::SizeResolved;
}

int QFont::pixelSize() const
{
   return d->request.pixelSize;
}

QFont::Style QFont::style() const
{
   return (QFont::Style)d->request.style;
}

void QFont::setStyle(Style style)
{
   if ((resolve_mask & QFont::StyleResolved) && d->request.style == style) {
      return;
   }
   detach();

   d->request.style = style;
   resolve_mask |= QFont::StyleResolved;
}

int QFont::weight() const
{
   return d->request.weight;
}

void QFont::setWeight(int weight)
{
   Q_ASSERT_X(weight >= 0 && weight <= 99, "QFont::setWeight", "Weight must be between 0 and 99");

   if ((resolve_mask & QFont::WeightResolved) && d->request.weight == weight) {
      return;
   }
   detach();

   d->request.weight = weight;
   resolve_mask |= QFont::WeightResolved;
}

bool QFont::underline() const
{
   return d->underline;
}

void QFont::setUnderline(bool enable)
{
   if ((resolve_mask & QFont::UnderlineResolved) && d->underline == enable) {
      return;
   }

   QFontPrivate::detachButKeepEngineData(this);

   d->underline = enable;
   resolve_mask |= QFont::UnderlineResolved;
}

bool QFont::overline() const
{
   return d->overline;
}

void QFont::setOverline(bool enable)
{
   if ((resolve_mask & QFont::OverlineResolved) && d->overline == enable) {
      return;
   }

   QFontPrivate::detachButKeepEngineData(this);

   d->overline = enable;
   resolve_mask |= QFont::OverlineResolved;
}

bool QFont::strikeOut() const
{
   return d->strikeOut;
}

void QFont::setStrikeOut(bool enable)
{
   if ((resolve_mask & QFont::StrikeOutResolved) && d->strikeOut == enable) {
      return;
   }

   QFontPrivate::detachButKeepEngineData(this);

   d->strikeOut = enable;
   resolve_mask |= QFont::StrikeOutResolved;
}

bool QFont::fixedPitch() const
{
   return d->request.fixedPitch;
}

void QFont::setFixedPitch(bool enable)
{
   if ((resolve_mask & QFont::FixedPitchResolved) && d->request.fixedPitch == enable) {
      return;
   }

   detach();

   d->request.fixedPitch = enable;
   d->request.ignorePitch = false;
   resolve_mask |= QFont::FixedPitchResolved;
}

bool QFont::kerning() const
{
   return d->kerning;
}

void QFont::setKerning(bool enable)
{
   if ((resolve_mask & QFont::KerningResolved) && d->kerning == enable) {
      return;
   }

   QFontPrivate::detachButKeepEngineData(this);
   d->kerning = enable;
   resolve_mask |= QFont::KerningResolved;
}

QFont::StyleStrategy QFont::styleStrategy() const
{
   return (StyleStrategy) d->request.styleStrategy;
}

QFont::StyleHint QFont::styleHint() const
{
   return (StyleHint) d->request.styleHint;
}

void QFont::setStyleHint(StyleHint hint, StyleStrategy strategy)
{
   if ((resolve_mask & (QFont::StyleHintResolved | QFont::StyleStrategyResolved)) &&
      (StyleHint) d->request.styleHint == hint &&
      (StyleStrategy) d->request.styleStrategy == strategy) {
      return;
   }

   detach();

   d->request.styleHint = hint;
   d->request.styleStrategy = strategy;
   resolve_mask |= QFont::StyleHintResolved;
   resolve_mask |= QFont::StyleStrategyResolved;
}

void QFont::setStyleStrategy(StyleStrategy s)
{
   if ((resolve_mask & QFont::StyleStrategyResolved) &&
      s == (StyleStrategy)d->request.styleStrategy) {
      return;
   }
   detach();

   d->request.styleStrategy = s;
   resolve_mask |= QFont::StyleStrategyResolved;
}

int QFont::stretch() const
{
   return d->request.stretch;
}

void QFont::setStretch(int factor)
{
   if (factor < 1 || factor > 4000) {
      qWarning("QFont::setStretch() Parameter '%d' is out of range", factor);
      return;
   }

   if ((resolve_mask & QFont::StretchResolved) &&
      d->request.stretch == (uint)factor) {
      return;
   }

   detach();

   d->request.stretch = (uint)factor;
   resolve_mask |= QFont::StretchResolved;
}

qreal QFont::letterSpacing() const
{
   return d->letterSpacing.toReal();
}

void QFont::setLetterSpacing(SpacingType type, qreal spacing)
{
   const QFixed newSpacing = QFixed::fromReal(spacing);
   const bool absoluteSpacing = type == AbsoluteSpacing;

   if ((resolve_mask & QFont::LetterSpacingResolved) &&
      d->letterSpacingIsAbsolute == absoluteSpacing &&
      d->letterSpacing == newSpacing) {
      return;
   }

   QFontPrivate::detachButKeepEngineData(this);

   d->letterSpacing = newSpacing;
   d->letterSpacingIsAbsolute = absoluteSpacing;
   resolve_mask |= QFont::LetterSpacingResolved;
}

QFont::SpacingType QFont::letterSpacingType() const
{
   return d->letterSpacingIsAbsolute ? AbsoluteSpacing : PercentageSpacing;
}

qreal QFont::wordSpacing() const
{
   return d->wordSpacing.toReal();
}

void QFont::setWordSpacing(qreal spacing)
{
   const QFixed newSpacing = QFixed::fromReal(spacing);
   if ((resolve_mask & QFont::WordSpacingResolved) &&
      d->wordSpacing == newSpacing) {
      return;
   }

   QFontPrivate::detachButKeepEngineData(this);

   d->wordSpacing = newSpacing;
   resolve_mask |= QFont::WordSpacingResolved;
}

void QFont::setCapitalization(Capitalization caps)
{
   if ((resolve_mask & QFont::CapitalizationResolved) &&
      capitalization() == caps) {
      return;
   }

   QFontPrivate::detachButKeepEngineData(this);

   d->capital = caps;
   resolve_mask |= QFont::CapitalizationResolved;
}

QFont::Capitalization QFont::capitalization() const
{
   return static_cast<QFont::Capitalization> (d->capital);
}

bool QFont::exactMatch() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return d->request.exactMatch(engine->fontDef);
}

bool QFont::operator==(const QFont &f) const
{
   return (f.d == d
         || (f.d->request   == d->request
            && f.d->request.pointSize == d->request.pointSize
            && f.d->underline == d->underline
            && f.d->overline  == d->overline
            && f.d->strikeOut == d->strikeOut
            && f.d->kerning == d->kerning
            && f.d->capital == d->capital
            && f.d->letterSpacingIsAbsolute == d->letterSpacingIsAbsolute
            && f.d->letterSpacing == d->letterSpacing
            && f.d->wordSpacing == d->wordSpacing
         ));
}

bool QFont::operator<(const QFont &f) const
{
   if (f.d == d) {
      return false;
   }

   // the < operator for fontdefs ignores point sizes.
   QFontDef &r1 = f.d->request;
   QFontDef &r2 = d->request;

   if (r1.pointSize != r2.pointSize) {
      return r1.pointSize < r2.pointSize;
   }
   if (r1.pixelSize != r2.pixelSize) {
      return r1.pixelSize < r2.pixelSize;
   }
   if (r1.weight != r2.weight) {
      return r1.weight < r2.weight;
   }
   if (r1.style != r2.style) {
      return r1.style < r2.style;
   }
   if (r1.stretch != r2.stretch) {
      return r1.stretch < r2.stretch;
   }
   if (r1.styleHint != r2.styleHint) {
      return r1.styleHint < r2.styleHint;
   }
   if (r1.styleStrategy != r2.styleStrategy) {
      return r1.styleStrategy < r2.styleStrategy;
   }
   if (r1.family != r2.family) {
      return r1.family < r2.family;
   }

   if (f.d->capital != d->capital) {
      return f.d->capital < d->capital;
   }

   if (f.d->letterSpacingIsAbsolute != d->letterSpacingIsAbsolute) {
      return f.d->letterSpacingIsAbsolute < d->letterSpacingIsAbsolute;
   }
   if (f.d->letterSpacing != d->letterSpacing) {
      return f.d->letterSpacing < d->letterSpacing;
   }
   if (f.d->wordSpacing != d->wordSpacing) {
      return f.d->wordSpacing < d->wordSpacing;
   }

   int f1attrs = (f.d->underline << 3) + (f.d->overline << 2) + (f.d->strikeOut << 1) + f.d->kerning;
   int f2attrs = (d->underline << 3) + (d->overline << 2) + (d->strikeOut << 1) + d->kerning;

   return f1attrs < f2attrs;
}

bool QFont::operator!=(const QFont &f) const
{
   return !(operator==(f));
}

QFont::operator QVariant() const
{
   return QVariant(QVariant::Font, this);
}

bool QFont::isCopyOf(const QFont &f) const
{
   return d == f.d;
}

QFont QFont::resolve(const QFont &other) const
{
   if (resolve_mask == 0 || (resolve_mask == other.resolve_mask && *this == other)) {
      QFont o(other);
      o.resolve_mask = resolve_mask;
      return o;
   }

   QFont font(*this);
   font.detach();
   font.d->resolve(resolve_mask, other.d.data());

   return font;
}

using QFontSubst = QHash<QString, QStringList>;

static QFontSubst *globalFontSubst()
{
   static QFontSubst retval;
   return &retval;
}

QString QFont::substitute(const QString &familyName)
{
   QFontSubst *fontSubst = globalFontSubst();
   Q_ASSERT(fontSubst != nullptr);

   QFontSubst::const_iterator it = fontSubst->constFind(familyName.toLower());
   if (it != fontSubst->constEnd() && !(*it).isEmpty()) {
      return (*it).first();
   }

   return familyName;
}

QStringList QFont::substitutes(const QString &familyName)
{
   QFontSubst *fontSubst = globalFontSubst();
   Q_ASSERT(fontSubst != nullptr);

   return fontSubst->value(familyName.toLower(), QStringList());
}

void QFont::insertSubstitution(const QString &familyName, const QString &substituteName)
{
   QFontSubst *fontSubst = globalFontSubst();
   Q_ASSERT(fontSubst != nullptr);

   QStringList &list = (*fontSubst)[familyName.toLower()];
   QString s = substituteName.toLower();

   if (! list.contains(s)) {
      list.append(s);
   }
}

void QFont::insertSubstitutions(const QString &familyName, const QStringList &substituteNames)
{
   QFontSubst *fontSubst = globalFontSubst();
   Q_ASSERT(fontSubst != nullptr);

   QStringList &list = (*fontSubst)[familyName.toLower()];

   for (const QString &item : substituteNames) {
      QString s = item.toLower();

      if (! list.contains(s)) {
         list.append(s);
      }
   }
}

void QFont::removeSubstitutions(const QString &familyName)
{
   QFontSubst *fontSubst = globalFontSubst();
   Q_ASSERT(fontSubst != nullptr);

   fontSubst->remove(familyName.toLower());
}

QStringList QFont::substitutions()
{
   QFontSubst *fontSubst = globalFontSubst();
   Q_ASSERT(fontSubst != nullptr);

   QStringList ret = fontSubst->keys();

   ret.sort();

   return ret;
}

static quint8 get_font_bits(int version, const QFontPrivate *f)
{
   (void) version;

   Q_ASSERT(f != nullptr);

   quint8 bits = 0;
   if (f->request.style) {
      bits |= 0x01;
   }

   if (f->underline) {
      bits |= 0x02;
   }

   if (f->overline) {
      bits |= 0x40;
   }

   if (f->strikeOut) {
      bits |= 0x04;
   }

   if (f->request.fixedPitch) {
      bits |= 0x08;
   }

   // if (f.hintSetByUser)
   // bits |= 0x10;

   if (f->kerning) {
      bits |= 0x10;
   }

   if (f->request.style == QFont::StyleOblique) {
      bits |= 0x80;
   }

   return bits;
}

static quint8 get_extended_font_bits(const QFontPrivate *f)
{
   Q_ASSERT(f != nullptr);
   quint8 bits = 0;

   if (f->request.ignorePitch) {
      bits |= 0x01;
   }

   if (f->letterSpacingIsAbsolute) {
      bits |= 0x02;
   }

   return bits;
}

static void set_font_bits(int version, quint8 bits, QFontPrivate *f)
{
   (void) version;

   Q_ASSERT(f != nullptr);

   f->request.style         = (bits & 0x01) != 0 ? QFont::StyleItalic : QFont::StyleNormal;
   f->underline             = (bits & 0x02) != 0;
   f->overline              = (bits & 0x40) != 0;
   f->strikeOut             = (bits & 0x04) != 0;
   f->request.fixedPitch    = (bits & 0x08) != 0;
   // f->hintSetByUser      = (bits & 0x10) != 0;


   f->kerning               = (bits & 0x10) != 0;

   if ((bits & 0x80) != 0) {
      f->request.style         = QFont::StyleOblique;
   }
}

static void set_extended_font_bits(quint8 bits, QFontPrivate *f)
{
   Q_ASSERT(f != nullptr);
   f->request.ignorePitch = (bits & 0x01) != 0;
   f->letterSpacingIsAbsolute = (bits & 0x02) != 0;
}

QString QFont::key() const
{
   return toString();
}

QString QFont::toString() const
{
   const QChar comma(QLatin1Char(','));
   return family() + comma +
      QString::number(     pointSizeF()) + comma +
      QString::number(      pixelSize()) + comma +
      QString::number((int) styleHint()) + comma +
      QString::number(         weight()) + comma +
      QString::number((int)     style()) + comma +
      QString::number((int) underline()) + comma +
      QString::number((int) strikeOut()) + comma +
      QString::number((int)fixedPitch()) + comma +
      QString::number((int)   false);
}

uint qHash(const QFont &font, uint seed)
{
   return qHash(QFontPrivate::get(font)->request, seed);
}

bool QFont::fromString(const QString &descrip)
{
   QStringList l(descrip.split(QLatin1Char(',')));

   int count = l.count();
   if (! count || (count > 2 && count < 9) || count > 11) {

      if (descrip.isEmpty()) {
         qWarning("QFont::fromString() Font description was empty");
      } else {
         qWarning("QFont::fromString() Invalid font description of %s ", csPrintable(descrip));
      }

      return false;
   }

   setFamily(l[0]);
   if (count > 1 && l[1].toDouble() > 0.0) {
      setPointSizeF(l[1].toDouble());
   }

   if (count == 9) {
      setStyleHint((StyleHint) l[2].toInteger<int>());
      setWeight(qMax(qMin(99, l[3].toInteger<int>()), 0));
      setItalic(l[4].toInteger<int>());
      setUnderline(l[5].toInteger<int>());
      setStrikeOut(l[6].toInteger<int>());
      setFixedPitch(l[7].toInteger<int>());

   } else if (count == 10) {
      if (l[2].toInteger<int>() > 0) {
         setPixelSize(l[2].toInteger<int>());
      }

      setStyleHint((StyleHint) l[3].toInteger<int>());
      setWeight(qMax(qMin(99, l[4].toInteger<int>()), 0));
      setStyle((QFont::Style)l[5].toInteger<int>());
      setUnderline(l[6].toInteger<int>());
      setStrikeOut(l[7].toInteger<int>());
      setFixedPitch(l[8].toInteger<int>());

   }

   if (count >= 9 && !d->request.fixedPitch) {
      // assume 'false' fixedPitch equals default
      d->request.ignorePitch = true;
   }

   return true;
}

void QFont::initialize()
{
}

void QFont::cleanup()
{
   QFontCache::cleanup();
}

void QFont::cacheStatistics()
{
}

QString QFont::lastResortFamily() const
{
   return QString("helvetica");
}

extern QStringList qt_fallbacksForFamily(const QString &family, QFont::Style style,
   QFont::StyleHint styleHint, QChar::Script script);
QString QFont::defaultFamily() const
{
   const QStringList fallbacks = qt_fallbacksForFamily(QString(), QFont::StyleNormal,
         QFont::StyleHint(d->request.styleHint), QChar::Script_Common);

   if (! fallbacks.isEmpty()) {
      return fallbacks.first();
   }

   return QString();
}

QString QFont::lastResortFont() const
{
   return QString("arial");
}

QDataStream &operator<<(QDataStream &s, const QFont &font)
{
   if (s.version() == 1) {
      s << font.d->request.family.toLatin1();

   } else {
      s << font.d->request.family;
      s << font.d->request.styleName;
   }

   double pointSize = font.d->request.pointSize;
   qint32 pixelSize = font.d->request.pixelSize;
   s << pointSize;
   s << pixelSize;


   s << (quint8) font.d->request.styleHint;

   // Continue writing 8 bits for versions < 5.4 so that we don't write too much,
   // even though we need 16 to store styleStrategy, so there is some data loss.

   s << (quint16) font.d->request.styleStrategy;

   s << (quint8) 0
      << (quint8) font.d->request.weight
      << get_font_bits(s.version(), font.d.data());

   s << (quint16)font.d->request.stretch;
   s << get_extended_font_bits(font.d.data());
   s << font.d->letterSpacing.value();
   s << font.d->wordSpacing.value();

   s << (quint8)font.d->request.hintingPreference;
   s << (quint8)font.d->capital;

   return s;
}

QDataStream &operator>>(QDataStream &s, QFont &font)
{
   font.d = new QFontPrivate;
   font.resolve_mask = QFont::AllPropertiesResolved;

   quint8 styleHint, charSet, weight, bits;
   quint16 styleStrategy = QFont::PreferDefault;

   s >> font.d->request.family;
   s >> font.d->request.styleName;

   double pointSize;
   qint32 pixelSize;
   s >> pointSize;
   s >> pixelSize;
   font.d->request.pointSize = qreal(pointSize);
   font.d->request.pixelSize = pixelSize;

   s >> styleHint;
   s >> styleStrategy;

   s >> charSet;
   s >> weight;
   s >> bits;

   font.d->request.styleHint = styleHint;
   font.d->request.styleStrategy = styleStrategy;
   font.d->request.weight = weight;

   set_font_bits(s.version(), bits, font.d.data());


   quint16 stretch;
   s >> stretch;
   font.d->request.stretch = stretch;

   quint8 extendedBits;
   s >> extendedBits;
   set_extended_font_bits(extendedBits, font.d.data());

   int value;
   s >> value;
   font.d->letterSpacing.setValue(value);

   s >> value;
   font.d->wordSpacing.setValue(value);

   quint8 value8;
   s >> value8;
   font.d->request.hintingPreference = QFont::HintingPreference(value8);

   s >> value8;
   font.d->capital = QFont::Capitalization(value8);

   return s;
}

QFontInfo::QFontInfo(const QFont &font)
   : d(font.d.data())
{
}

QFontInfo::QFontInfo(const QFontInfo &fi)
   : d(fi.d.data())
{
}

QFontInfo::~QFontInfo()
{
}

QFontInfo &QFontInfo::operator=(const QFontInfo &fi)
{
   d = fi.d.data();
   return *this;
}

QString QFontInfo::family() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->fontDef.family;
}

QString QFontInfo::styleName() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->fontDef.styleName;
}

int QFontInfo::pointSize() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return qRound(engine->fontDef.pointSize);
}

qreal QFontInfo::pointSizeF() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->fontDef.pointSize;
}

int QFontInfo::pixelSize() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->fontDef.pixelSize;
}

bool QFontInfo::italic() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->fontDef.style != QFont::StyleNormal;
}

QFont::Style QFontInfo::style() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return (QFont::Style)engine->fontDef.style;
}

int QFontInfo::weight() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->fontDef.weight;

}

bool QFontInfo::underline() const
{
   return d->underline;
}

bool QFontInfo::overline() const
{
   return d->overline;
}

bool QFontInfo::strikeOut() const
{
   return d->strikeOut;
}

bool QFontInfo::fixedPitch() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

#ifdef Q_OS_DARWIN
   if (! engine->fontDef.fixedPitchComputed) {
      QGlyphLayoutArray<2> g;
      int l = 2;

      static const QString imStr = QString("im");
      engine->stringToCMap(imStr, &g, &l, nullptr);

      engine->fontDef.fixedPitch = g.advances[0] == g.advances[1];
      engine->fontDef.fixedPitchComputed = true;
   }
#endif

   return engine->fontDef.fixedPitch;
}

QFont::StyleHint QFontInfo::styleHint() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return (QFont::StyleHint) engine->fontDef.styleHint;
}

bool QFontInfo::exactMatch() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return d->request.exactMatch(engine->fontDef);
}

static QThreadStorage<QFontCache *> *theFontCache()
{
   static QThreadStorage<QFontCache *> retval;
   return &retval;
}

QFontCache *QFontCache::instance()
{
   QFontCache *&fontCache = theFontCache()->localData();

   if (! fontCache) {
      fontCache = new QFontCache;
   }

   return fontCache;
}

void QFontCache::cleanup()
{
   QThreadStorage<QFontCache *> *cache = nullptr;

   try {
      cache = theFontCache();

   } catch (const std::bad_alloc &) {
      // no cache - just ignore
   }

   if (cache && cache->hasLocalData()) {
      cache->setLocalData(nullptr);
   }
}

std::atomic<int> font_cache_id{1};

QFontCache::QFontCache()
   : QObject(), total_cost(0), max_cost(MinCacheSize), current_timestamp(0), fast(false),
     timer_id(-1), m_id(font_cache_id.fetch_add(1, std::memory_order_relaxed))
{
}

QFontCache::~QFontCache()
{
   clear();
}

void QFontCache::clear()
{
   {
      EngineDataCache::iterator it  = engineDataCache.begin();
      EngineDataCache::iterator end = engineDataCache.end();

      while (it != end) {
         QFontEngineData *data = it.value();


         for (int i = 0; i < QChar::ScriptCount; ++i) {
            if (data->engines[i]) {
               if (! data->engines[i]->m_refCount.deref()) {
                  Q_ASSERT(engineCacheCount.value(data->engines[i]) == 0);
                  delete data->engines[i];
               }

               data->engines[i] = nullptr;
            }
         }

         if (! data->m_refCount.deref()) {
            delete data;
         }

         ++it;
      }
   }

   engineDataCache.clear();
   bool mightHaveEnginesLeftForCleanup;

   do {
      mightHaveEnginesLeftForCleanup = false;

      for (EngineCache::iterator it = engineCache.begin(), end = engineCache.end(); it != end; ++it) {
         QFontEngine *engine = it.value().data;

         if (engine) {
            const int cacheCount = --engineCacheCount[engine];
            Q_ASSERT(cacheCount >= 0);

            if (! engine->m_refCount.deref()) {
               Q_ASSERT(cacheCount == 0);

               mightHaveEnginesLeftForCleanup = engine->type() == QFontEngine::Multi;
               delete engine;

            } else if (cacheCount == 0) {
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
               qDebug("QFontCache::clear() Engine %p still has refcount %d",
                     static_cast<void *>(engine), engine->m_refCount.load());
#endif
            }

            it.value().data = nullptr;
         }
      }

   } while (mightHaveEnginesLeftForCleanup);

   engineCache.clear();
   engineCacheCount.clear();

   total_cost = 0;
   max_cost   = MinCacheSize;
}

QFontEngineData *QFontCache::findEngineData(const QFontDef &def) const
{
   EngineDataCache::const_iterator iter = engineDataCache.constFind(def);

   if (iter == engineDataCache.constEnd()) {
      return nullptr;
   }

   // found
   return iter.value();
}

void QFontCache::insertEngineData(const QFontDef &def, QFontEngineData *engineData)
{
   Q_ASSERT(!engineDataCache.contains(def));

   engineData->m_refCount.ref();

   // decrease now rather than waiting
   if (total_cost > MinCacheSize * 2 && engineDataCache.size() >= QFONTCACHE_DECREASE_TRIGGER_LIMIT) {
      decreaseCache();
   }

   engineDataCache.insert(def, engineData);
   increaseCost(sizeof(QFontEngineData));
}

QFontEngine *QFontCache::findEngine(const Key &key)
{
   EngineCache::iterator it  = engineCache.find(key);
   EngineCache::iterator end = engineCache.end();

   if (it == end) {
      return nullptr;
   }

   Q_ASSERT(it.value().data != nullptr);
   Q_ASSERT(key.multi == (it.value().data->type() == QFontEngine::Multi));

   // found... update the hitcount and timestamp
   updateHitCountAndTimeStamp(it.value());

   return it.value().data;
}

void QFontCache::updateHitCountAndTimeStamp(Engine &value)
{
   value.hits++;
   value.timestamp = ++current_timestamp;
}

void QFontCache::insertEngine(const Key &key, QFontEngine *engine, bool insertMulti)
{
   Q_ASSERT(engine != nullptr);
   Q_ASSERT(key.multi == (engine->type() == QFontEngine::Multi));

#if defined(CS_SHOW_DEBUG_GUI_TEXT)
   qDebug("QFontCache::insertEngine() Inserting new engine %p", static_cast<void *>(engine));
#endif

   engine->m_refCount.ref();

   // decrease now rather than waiting
   if (total_cost > MinCacheSize * 2 && engineCache.size() >= QFONTCACHE_DECREASE_TRIGGER_LIMIT) {
      decreaseCache();
   }

   Engine data(engine);

   ++current_timestamp;
   data.timestamp = current_timestamp;

   if (insertMulti) {
      engineCache.insertMulti(key, data);

   } else {
      engineCache.replace(key, data);
   }

   // only increase the cost if this is the first time we insert the engine
   if (++engineCacheCount[engine] == 1)  {
      increaseCost(engine->cache_cost);
   }
}

void QFontCache::increaseCost(uint cost)
{
   cost = (cost + 512) / 1024; // store cost in kb
   cost = cost > 0 ? cost : 1;
   total_cost += cost;

#if defined(CS_SHOW_DEBUG_GUI_TEXT)
   qDebug("QFontCache::increaseCost() Increased %u kb, total_cost %u kb, max_cost %u kb",
      cost, total_cost, max_cost);
#endif

   if (total_cost > max_cost) {
      max_cost = total_cost;

      if (timer_id == -1 || ! fast) {
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
         qDebug("QFontCache::increaseCost() Starting fast timer (%d ms)", fast_timeout);
#endif

         if (timer_id != -1) {
            killTimer(timer_id);
         }

         timer_id = startTimer(fast_timeout);
         fast = true;
      }
   }
}

void QFontCache::decreaseCost(uint cost)
{
   cost = (cost + 512) / 1024; // cost is stored in kb
   cost = cost > 0 ? cost : 1;

   Q_ASSERT(cost <= total_cost);
   total_cost -= cost;

#if defined(CS_SHOW_DEBUG_GUI_TEXT)
   qDebug("QFontCache::decreaseCost) Decreased %u kb, total_cost %u kb, max_cost %u kb",
      cost, total_cost, max_cost);
#endif
}

void QFontCache::timerEvent(QTimerEvent *)
{
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
   qDebug("QFontCache::timerEvent() Performing cache maintenance (timestamp %u)",
      current_timestamp);
#endif

   if (total_cost <= max_cost && max_cost <= MinCacheSize) {
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
      qDebug("QFontCache::timerEvent() Cache reduce sufficiently, stopping timer");
#endif

      killTimer(timer_id);
      timer_id = -1;
      fast = false;

      return;
   }

   decreaseCache();
}

void QFontCache::decreaseCache()
{
   // go through the cache and count up everything in use
   uint in_use_cost = 0;

   {
      // make sure the cost of each engine data is at least 1kb
      const uint engine_data_cost = sizeof(QFontEngineData) > 1024 ? sizeof(QFontEngineData) : 1024;

      EngineDataCache::const_iterator it  = engineDataCache.constBegin();
      EngineDataCache::const_iterator end = engineDataCache.constEnd();

      for (; it != end; ++it) {

         if (it.value()->m_refCount.load() != 1) {
            in_use_cost += engine_data_cost;
         }
      }
   }

   {
      for (const auto &item : engineCache) {
         if (item.data->m_refCount.load() != 0) {
            in_use_cost += item.data->cache_cost / engineCacheCount.value(item.data);
         }
      }

      // attempt to make up for rounding errors
      in_use_cost += engineCache.size();
   }

   in_use_cost = (in_use_cost + 512) / 1024; // cost is stored in kb

   /*
     calculate the new maximum cost for the cache

     in_use_cost is *not* correct due to rounding errors in the above algorithm.
     instead of trying to get the calculation exact, more interested in speed so
     in_use_cost as a floor for new_max_cost
   */

   uint new_max_cost = qMax(qMax(max_cost / 2, in_use_cost), MinCacheSize);

#if defined(CS_SHOW_DEBUG_GUI_TEXT)
   qDebug("QFontCache::decreaseCache() In use %u kb, total %u kb, max %u kb, new max %u kb",
      in_use_cost, total_cost, max_cost, new_max_cost);
#endif

   if (new_max_cost == max_cost) {
      if (fast) {
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
         qDebug("QFontCache::decreaseCache() Uunable to shrink cache, slowing timer");
#endif

         killTimer(timer_id);
         timer_id = startTimer(slow_timeout);
         fast = false;
      }

      return;

   } else if (! fast) {
      killTimer(timer_id);
      timer_id = startTimer(fast_timeout);
      fast = true;
   }

   max_cost = new_max_cost;

   {
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
      qDebug("QFontCache::decreaseCache() Expire engine data");
#endif

      // clean out all unused engine data
      EngineDataCache::iterator it  = engineDataCache.begin();
      EngineDataCache::iterator end = engineDataCache.end();

      while (it != end) {
         if (it.value()->m_refCount.load() == 1) {
            decreaseCost(sizeof(QFontEngineData));

            it.value()->m_refCount.deref();
            delete it.value();

            it = engineDataCache.erase(it);

         } else {
            ++it;
         }

      }
   }

   // clean out the engine cache just enough to get below our new max cost
   bool cost_decreased;

   do {
      cost_decreased = false;

      EngineCache::iterator it  = engineCache.begin();
      EngineCache::iterator end = engineCache.end();

      // determine the oldest and least popular of the unused engines
      uint oldest = ~0u;
      uint least_popular = ~0u;

      EngineCache::iterator jt = end;
      for (; it != end; ++it) {
         if (it.value().data->m_refCount.load() != engineCacheCount.value(it.value().data)) {
            continue;
         }

         if (it.value().timestamp < oldest && it.value().hits <= least_popular) {
            oldest = it.value().timestamp;
            least_popular = it.value().hits;
            jt = it;
         }
      }

      it = jt;

      if (it != end) {

         QFontEngine *fontEngine = it.value().data;

         // get rid of all occurrences
         it = engineCache.begin();

         while (it != engineCache.end()) {
            if (it.value().data == fontEngine) {
               fontEngine->m_refCount.deref();

               it = engineCache.erase(it);

            } else {
               ++it;
            }
         }

         // and delete the last occurrence
         Q_ASSERT(fontEngine->m_refCount.load() == 0);
         decreaseCost(fontEngine->cache_cost);
         delete fontEngine;

         engineCacheCount.remove(fontEngine);

         cost_decreased = true;
      }

   } while (cost_decreased && total_cost > max_cost);
}

QDebug operator<<(QDebug stream, const QFont &font)
{
   return stream << "QFont(" << font.toString() << ')';
}
