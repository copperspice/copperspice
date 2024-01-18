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

#include <qpalette.h>

#include <qapplication.h>
#include <qguiapplication_p.h>
#include <qdatastream.h>
#include <qvariant.h>
#include <qdebug.h>

static int qt_palette_count = 1;

class QPalettePrivate
{
 public:
   QPalettePrivate() : ref(1), ser_no(qt_palette_count++), detach_no(0)
   { }

   QAtomicInt ref;
   QBrush br[QPalette::NColorGroups][QPalette::NColorRoles];

   int ser_no;
   int detach_no;
};

static QColor qt_mix_colors(QColor a, QColor b)
{
   return QColor((a.red() + b.red()) / 2, (a.green() + b.green()) / 2,
         (a.blue() + b.blue()) / 2, (a.alpha() + b.alpha()) / 2);
}

static void qt_palette_from_color(QPalette &pal, const QColor &button)
{
   int h, s, v;
   button.getHsv(&h, &s, &v);

   // inactive and active are the same..
   const QBrush whiteBrush = QBrush(Qt::white);
   const QBrush blackBrush = QBrush(Qt::black);
   const QBrush baseBrush = v > 128 ? whiteBrush : blackBrush;
   const QBrush foregroundBrush = v > 128 ? blackBrush : whiteBrush;
   const QBrush buttonBrush = QBrush(button);
   const QBrush buttonBrushDark = QBrush(button.darker());
   const QBrush buttonBrushDark150 = QBrush(button.darker(150));
   const QBrush buttonBrushLight150 = QBrush(button.lighter(150));

   pal.setColorGroup(QPalette::Active, foregroundBrush, buttonBrush, buttonBrushLight150,
      buttonBrushDark, buttonBrushDark150, foregroundBrush, whiteBrush,
      baseBrush, buttonBrush);
   pal.setColorGroup(QPalette::Inactive, foregroundBrush, buttonBrush, buttonBrushLight150,
      buttonBrushDark, buttonBrushDark150, foregroundBrush, whiteBrush,
      baseBrush, buttonBrush);
   pal.setColorGroup(QPalette::Disabled, buttonBrushDark, buttonBrush, buttonBrushLight150,
      buttonBrushDark, buttonBrushDark150, buttonBrushDark,
      whiteBrush, buttonBrush, buttonBrush);
}

QPalette::QPalette()
   : d(nullptr), current_group(Active), resolve_mask(0)
{
   // Initialize to application palette if present, else default to black.
   // This makes it possible to instantiate QPalette outside QGuiApplication,
   // for example in the platform plugins.

   if (QGuiApplicationPrivate::app_palette) {
      d = QGuiApplicationPrivate::app_palette->d;
      d->ref.ref();

   } else {
      init();
      qt_palette_from_color(*this, Qt::black);
      resolve_mask = 0;
   }
}

QPalette::QPalette(const QColor &button)
{
   init();
   qt_palette_from_color(*this, button);
}

QPalette::QPalette(Qt::GlobalColor button)
{
   init();
   qt_palette_from_color(*this, button);
}

QPalette::QPalette(const QBrush &windowText, const QBrush &button,
   const QBrush &light, const QBrush &dark,
   const QBrush &mid, const QBrush &text,
   const QBrush &bright_text, const QBrush &base,
   const QBrush &window)
{
   init();
   setColorGroup(All, windowText, button, light, dark, mid, text, bright_text,
      base, window);
}

QPalette::QPalette(const QColor &windowText, const QColor &window,
   const QColor &light, const QColor &dark, const QColor &mid,
   const QColor &text, const QColor &base)
{
   init();

   const QBrush windowBrush(window);
   const QBrush lightBrush(light);

   setColorGroup(All, QBrush(windowText), windowBrush, lightBrush,
      QBrush(dark), QBrush(mid), QBrush(text), lightBrush,
      QBrush(base), windowBrush);
}

QPalette::QPalette(const QColor &button, const QColor &window)
{
   init();

   int h;
   int s;
   int v;

   window.getHsv(&h, &s, &v);

   const QBrush windowBrush = QBrush(window);
   const QBrush whiteBrush  = QBrush(Qt::white);
   const QBrush blackBrush  = QBrush(Qt::black);

   const QBrush baseBrush          = v > 128 ? whiteBrush : blackBrush;
   const QBrush foregroundBrush    = v > 128 ? blackBrush : whiteBrush;
   const QBrush disabledForeground = QBrush(Qt::darkGray);

   const QBrush buttonBrush         = QBrush(button);
   const QBrush buttonBrushDark     = QBrush(button.darker());
   const QBrush buttonBrushDark150  = QBrush(button.darker(150));
   const QBrush buttonBrushLight150 = QBrush(button.lighter(150));

   //inactive and active are identical
   setColorGroup(Inactive, foregroundBrush, buttonBrush, buttonBrushLight150, buttonBrushDark,
      buttonBrushDark150, foregroundBrush, whiteBrush, baseBrush, windowBrush);

   setColorGroup(Active, foregroundBrush, buttonBrush, buttonBrushLight150, buttonBrushDark,
      buttonBrushDark150, foregroundBrush, whiteBrush, baseBrush, windowBrush);

   setColorGroup(Disabled, disabledForeground, buttonBrush, buttonBrushLight150,
      buttonBrushDark, buttonBrushDark150, disabledForeground, whiteBrush, baseBrush, windowBrush);
}

QPalette::QPalette(const QPalette &p)
   : d(p.d)
{
   d->ref.ref();

   resolve_mask  = p.resolve_mask;
   current_group = p.current_group;
}

QPalette::~QPalette()
{
   if (d && !d->ref.deref()) {
      delete d;
   }
}

// internal
void QPalette::init()
{
   d = new QPalettePrivate;
   resolve_mask  = 0;
   current_group = Active; //as a default..
}

QPalette &QPalette::operator=(const QPalette &p)
{
   p.d->ref.ref();
   resolve_mask  = p.resolve_mask;
   current_group = p.current_group;

   if (d && ! d->ref.deref()) {
      delete d;
   }

   d = p.d;

   return *this;
}

QPalette::operator QVariant() const
{
   return QVariant(QVariant::Palette, this);
}

const QBrush &QPalette::brush(ColorGroup gr, ColorRole cr) const
{
   Q_ASSERT(cr < NColorRoles);

   if (gr >= (int)NColorGroups) {
      if (gr == Current) {
         gr = (ColorGroup)current_group;

      } else {
         qWarning("QPalette::brush() Unknown ColorGroup: %d", (int)gr);
         gr = Active;
      }
   }

   return d->br[gr][cr];
}

void QPalette::setBrush(ColorGroup cg, ColorRole cr, const QBrush &b)
{
   Q_ASSERT(cr < NColorRoles);
   detach();

   if (cg >= (int)NColorGroups) {
      if (cg == All) {
         for (int i = 0; i < (int)NColorGroups; i++) {
            d->br[i][cr] = b;
         }

         resolve_mask |= (1 << cr);
         return;

      } else if (cg == Current) {
         cg = (ColorGroup)current_group;

      } else {
         qWarning("QPalette::setBrush() Unknown ColorGroup: %d", (int)cg);
         cg = Active;
      }
   }

   d->br[cg][cr] = b;
   resolve_mask |= (1 << cr);
}

bool QPalette::isBrushSet(ColorGroup cg, ColorRole cr) const
{
   (void) cg;

   return (resolve_mask & (1 << cr));
}

/*!
    \internal
*/
void QPalette::detach()
{
   if (d->ref.load() != 1) {
      QPalettePrivate *x = new QPalettePrivate;

      for (int grp = 0; grp < (int)NColorGroups; grp++) {
         for (int role = 0; role < (int)NColorRoles; role++) {
            x->br[grp][role] = d->br[grp][role];
         }
      }

      if (!d->ref.deref()) {
         delete d;
      }

      d = x;
   }
   ++d->detach_no;
}

bool QPalette::operator==(const QPalette &p) const
{
   if (isCopyOf(p)) {
      return true;
   }

   for (int grp = 0; grp < (int)NColorGroups; grp++) {
      for (int role = 0; role < (int)NColorRoles; role++) {
         if (d->br[grp][role] != p.d->br[grp][role]) {
            return false;
         }
      }
   }
   return true;
}

bool QPalette::isEqual(QPalette::ColorGroup group1, QPalette::ColorGroup group2) const
{
   if (group1 >= (int)NColorGroups) {
      if (group1 == Current) {
         group1 = (ColorGroup)current_group;
      } else {
         qWarning("QPalette::isEqual() Unknown ColorGroup(1): %d", (int)group1);
         group1 = Active;
      }
   }

   if (group2 >= (int)NColorGroups) {
      if (group2 == Current) {
         group2 = (ColorGroup)current_group;
      } else {
         qWarning("QPalette::isEqual() Unknown ColorGroup(2): %d", (int)group2);
         group2 = Active;
      }
   }

   if (group1 == group2) {
      return true;
   }

   for (int role = 0; role < (int)NColorRoles; role++) {
      if (d->br[group1][role] != d->br[group2][role]) {
         return false;
      }
   }
   return true;
}


qint64 QPalette::cacheKey() const
{
   return (((qint64) d->ser_no) << 32) | ((qint64) (d->detach_no));
}

QPalette QPalette::resolve(const QPalette &other) const
{
   if ((*this == other && resolve_mask == other.resolve_mask) || resolve_mask == 0) {
      QPalette o = other;
      o.resolve_mask = resolve_mask;
      return o;
   }

   QPalette palette(*this);
   palette.detach();

   for (int role = 0; role < (int)NColorRoles; role++)
      if (! (resolve_mask & (1 << role)))
         for (int grp = 0; grp < (int)NColorGroups; grp++) {
            palette.d->br[grp][role] = other.d->br[grp][role];
         }

   return palette;
}

QDataStream &operator<<(QDataStream &s, const QPalette &p)
{
   for (int grp = 0; grp < (int)QPalette::NColorGroups; grp++) {
      int max = QPalette::ToolTipText + 1;

      for (int r = 0; r < max; r++) {
         s << p.d->br[grp][r];
      }
   }
   return s;
}

QDataStream &operator>>(QDataStream &s, QPalette &p)
{
   int max = QPalette::NColorRoles;

   QBrush tmp;

   for (int grp = 0; grp < (int)QPalette::NColorGroups; ++grp) {
      for (int role = 0; role < max; ++role) {
         s >> tmp;
         p.setBrush((QPalette::ColorGroup)grp, (QPalette::ColorRole)role, tmp);
      }
   }

   return s;
}

bool QPalette::isCopyOf(const QPalette &p) const
{
   return d == p.d;
}

void QPalette::setColorGroup(ColorGroup cg, const QBrush &windowText, const QBrush &button,
   const QBrush &light, const QBrush &dark, const QBrush &mid,
   const QBrush &text, const QBrush &bright_text, const QBrush &base,
   const QBrush &window)
{
   QBrush alt_base = QBrush(qt_mix_colors(base.color(), button.color()));
   QBrush mid_light = QBrush(qt_mix_colors(button.color(), light.color()));
   QColor toolTipBase(255, 255, 220);
   QColor toolTipText(0, 0, 0);

   setColorGroup(cg, windowText, button, light, dark, mid, text, bright_text, base,
      alt_base, window, mid_light, text,
      QBrush(Qt::black), QBrush(Qt::darkBlue), QBrush(Qt::white),
      QBrush(Qt::blue), QBrush(Qt::magenta), QBrush(toolTipBase),
      QBrush(toolTipText));

   resolve_mask &= ~(1 << Highlight);
   resolve_mask &= ~(1 << HighlightedText);
   resolve_mask &= ~(1 << LinkVisited);
   resolve_mask &= ~(1 << Link);
}


/*!\internal*/
void QPalette::setColorGroup(ColorGroup cg, const QBrush &foreground, const QBrush &button,
   const QBrush &light, const QBrush &dark, const QBrush &mid,
   const QBrush &text, const QBrush &bright_text,
   const QBrush &base, const QBrush &alternate_base,
   const QBrush &background, const QBrush &midlight,
   const QBrush &button_text, const QBrush &shadow,
   const QBrush &highlight, const QBrush &highlighted_text,
   const QBrush &link, const QBrush &link_visited)
{
   setColorGroup(cg, foreground, button, light, dark, mid,
      text, bright_text, base, alternate_base, background,
      midlight, button_text, shadow, highlight, highlighted_text,
      link, link_visited, background, foreground);
}

/* internal*/
void QPalette::setColorGroup(ColorGroup cg, const QBrush &foreground, const QBrush &button,
   const QBrush &light, const QBrush &dark, const QBrush &mid,
   const QBrush &text, const QBrush &bright_text,
   const QBrush &base, const QBrush &alternate_base,
   const QBrush &background, const QBrush &midlight,
   const QBrush &button_text, const QBrush &shadow,
   const QBrush &highlight, const QBrush &highlighted_text,
   const QBrush &link, const QBrush &link_visited,
   const QBrush &toolTipBase, const QBrush &toolTipText)
{
   detach();
   setBrush(cg, WindowText, foreground);
   setBrush(cg, Button, button);
   setBrush(cg, Light, light);
   setBrush(cg, Dark, dark);
   setBrush(cg, Mid, mid);
   setBrush(cg, Text, text);
   setBrush(cg, BrightText, bright_text);
   setBrush(cg, Base, base);
   setBrush(cg, AlternateBase, alternate_base);
   setBrush(cg, Window, background);
   setBrush(cg, Midlight, midlight);
   setBrush(cg, ButtonText, button_text);
   setBrush(cg, Shadow, shadow);
   setBrush(cg, Highlight, highlight);
   setBrush(cg, HighlightedText, highlighted_text);
   setBrush(cg, Link, link);
   setBrush(cg, LinkVisited, link_visited);
   setBrush(cg, ToolTipBase, toolTipBase);
   setBrush(cg, ToolTipText, toolTipText);
}

Q_GUI_EXPORT QPalette qt_fusionPalette()
{
   QColor backGround(239, 235, 231);
   QColor light = backGround.lighter(150);
   QColor mid(backGround.darker(130));
   QColor midLight = mid.lighter(110);
   QColor base = Qt::white;
   QColor disabledBase(backGround);
   QColor dark = backGround.darker(150);
   QColor darkDisabled = QColor(209, 200, 191).darker(110);
   QColor text = Qt::black;
   QColor hightlightedText = Qt::white;
   QColor disabledText = QColor(190, 190, 190);
   QColor button = backGround;
   QColor shadow = dark.darker(135);
   QColor disabledShadow = shadow.lighter(150);

   QPalette fusionPalette(Qt::black, backGround, light, dark, mid, text, base);
   fusionPalette.setBrush(QPalette::Midlight, midLight);
   fusionPalette.setBrush(QPalette::Button, button);
   fusionPalette.setBrush(QPalette::Shadow, shadow);
   fusionPalette.setBrush(QPalette::HighlightedText, hightlightedText);

   fusionPalette.setBrush(QPalette::Disabled, QPalette::Text, disabledText);
   fusionPalette.setBrush(QPalette::Disabled, QPalette::WindowText, disabledText);
   fusionPalette.setBrush(QPalette::Disabled, QPalette::ButtonText, disabledText);
   fusionPalette.setBrush(QPalette::Disabled, QPalette::Base, disabledBase);
   fusionPalette.setBrush(QPalette::Disabled, QPalette::Dark, darkDisabled);
   fusionPalette.setBrush(QPalette::Disabled, QPalette::Shadow, disabledShadow);

   fusionPalette.setBrush(QPalette::Active, QPalette::Highlight, QColor(48, 140, 198));
   fusionPalette.setBrush(QPalette::Inactive, QPalette::Highlight, QColor(48, 140, 198));
   fusionPalette.setBrush(QPalette::Disabled, QPalette::Highlight, QColor(145, 141, 126));
   return fusionPalette;
}
QDebug operator<<(QDebug dbg, const QPalette &p)
{
   const char *colorGroupNames[] = {"Active", "Disabled", "Inactive"};
   const char *colorRoleNames[] = {
      "WindowText", "Button", "Light", "Midlight", "Dark", "Mid", "Text",
      "BrightText", "ButtonText", "Base", "Window", "Shadow", "Highlight",
      "HighlightedText", "Link", "LinkVisited", "AlternateBase", "NoRole",
      "ToolTipBase", "ToolTipText"
   };

   QDebugStateSaver saver(dbg);
   QDebug nospace = dbg.nospace();

   const uint mask = p.resolve();
   nospace << "QPalette(resolve=" << hex << showbase << mask << ',';

   for (int role = 0; role < (int)QPalette::NColorRoles; ++role) {
      if (mask & (1 << role)) {
         if (role) {
            nospace << ',';
         }
         nospace << colorRoleNames[role] << ":[";
         for (int group = 0; group < (int)QPalette::NColorGroups; ++group) {
            if (group) {
               nospace << ',';
            }
            const QRgb color = p.color(static_cast<QPalette::ColorGroup>(group),
                  static_cast<QPalette::ColorRole>(role)).rgba();
            nospace << colorGroupNames[group] << ':' << color;
         }
         nospace << ']';
      }
   }

   nospace << ')' << noshowbase << dec;
   return dbg;
}
