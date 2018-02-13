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

#include <qpalette.h>
#include <qapplication.h>
#include <qdatastream.h>
#include <qvariant.h>

static int qt_palette_count = 1;

class QPalettePrivate
{
 public:
   QPalettePrivate() : ref(1), ser_no(qt_palette_count++), detach_no(0) { }
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

QPalette::QPalette()
   : d(QApplication::palette().d),
     current_group(Active),
     resolve_mask(0)
{
   d->ref.ref();
}

static void qt_palette_from_color(QPalette &pal, const QColor &button)
{
   QColor bg = button,
          btn = button,
          fg, base;

   int h, s, v;
   bg.getHsv(&h, &s, &v);

   if (v > 128) {
      fg   = Qt::black;
      base = Qt::white;
   } else {
      fg   = Qt::white;
      base = Qt::black;
   }
   //inactive and active are the same..
   pal.setColorGroup(QPalette::Active, QBrush(fg), QBrush(btn), QBrush(btn.lighter(150)),
                     QBrush(btn.darker()), QBrush(btn.darker(150)), QBrush(fg), QBrush(Qt::white),
                     QBrush(base), QBrush(bg));
   pal.setColorGroup(QPalette::Inactive, QBrush(fg), QBrush(btn), QBrush(btn.lighter(150)),
                     QBrush(btn.darker()), QBrush(btn.darker(150)), QBrush(fg), QBrush(Qt::white),
                     QBrush(base), QBrush(bg));
   pal.setColorGroup(QPalette::Disabled, QBrush(btn.darker()), QBrush(btn), QBrush(btn.lighter(150)),
                     QBrush(btn.darker()), QBrush(btn.darker(150)), QBrush(btn.darker()),
                     QBrush(Qt::white), QBrush(bg), QBrush(bg));
}


/*!
  Constructs a palette from the \a button color. The other colors are
  automatically calculated, based on this color. \c Window will be
  the button color as well.
*/
QPalette::QPalette(const QColor &button)
{
   init();
   qt_palette_from_color(*this, button);
}

/*!
  Constructs a palette from the \a button color. The other colors are
  automatically calculated, based on this color. \c Window will be
  the button color as well.
*/
QPalette::QPalette(Qt::GlobalColor button)
{
   init();
   qt_palette_from_color(*this, button);
}

/*!
    Constructs a palette. You can pass either brushes, pixmaps or
    plain colors for \a windowText, \a button, \a light, \a dark, \a
    mid, \a text, \a bright_text, \a base and \a window.

    \sa QBrush
*/
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


/*!\obsolete

  Constructs a palette with the specified \a windowText, \a
  window, \a light, \a dark, \a mid, \a text, and \a base colors.
  The button color will be set to the window color.
*/
QPalette::QPalette(const QColor &windowText, const QColor &window,
                   const QColor &light, const QColor &dark, const QColor &mid,
                   const QColor &text, const QColor &base)
{
   init();
   setColorGroup(All, QBrush(windowText), QBrush(window), QBrush(light),
                 QBrush(dark), QBrush(mid), QBrush(text), QBrush(light),
                 QBrush(base), QBrush(window));
}

/*!
    Constructs a palette from a \a button color and a \a window.
    The other colors are automatically calculated, based on these
    colors.
*/
QPalette::QPalette(const QColor &button, const QColor &window)
{
   init();
   QColor bg = window, btn = button, fg, base, disfg;
   int h, s, v;
   bg.getHsv(&h, &s, &v);
   if (v > 128) {
      fg   = Qt::black;
      base = Qt::white;
      disfg = Qt::darkGray;
   } else {
      fg   = Qt::white;
      base = Qt::black;
      disfg = Qt::darkGray;
   }
   //inactive and active are identical
   setColorGroup(Inactive, QBrush(fg), QBrush(btn), QBrush(btn.lighter(150)), QBrush(btn.darker()),
                 QBrush(btn.darker(150)), QBrush(fg), QBrush(Qt::white), QBrush(base),
                 QBrush(bg));
   setColorGroup(Active, QBrush(fg), QBrush(btn), QBrush(btn.lighter(150)), QBrush(btn.darker()),
                 QBrush(btn.darker(150)), QBrush(fg), QBrush(Qt::white), QBrush(base),
                 QBrush(bg));
   setColorGroup(Disabled, QBrush(disfg), QBrush(btn), QBrush(btn.lighter(150)),
                 QBrush(btn.darker()), QBrush(btn.darker(150)), QBrush(disfg),
                 QBrush(Qt::white), QBrush(base), QBrush(bg));
}

/*!
    Constructs a copy of \a p.

    This constructor is fast thanks to \l{implicit sharing}.
*/
QPalette::QPalette(const QPalette &p)
{
   d = p.d;
   d->ref.ref();
   resolve_mask = p.resolve_mask;
   current_group = p.current_group;
}

QPalette::~QPalette()
{
   if (!d->ref.deref()) {
      delete d;
   }
}

/*!\internal*/
void QPalette::init()
{
   d = new QPalettePrivate;
   resolve_mask = 0;
   current_group = Active; //as a default..
}

QPalette &QPalette::operator=(const QPalette &p)
{
   p.d->ref.ref();
   resolve_mask = p.resolve_mask;
   current_group = p.current_group;
   if (!d->ref.deref()) {
      delete d;
   }
   d = p.d;
   return *this;
}

/*!
   Returns the palette as a QVariant
*/
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
         qWarning("QPalette::brush: Unknown ColorGroup: %d", (int)gr);
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
         qWarning("QPalette::setBrush: Unknown ColorGroup: %d", (int)cg);
         cg = Active;
      }
   }
   d->br[cg][cr] = b;
   resolve_mask |= (1 << cr);
}

bool QPalette::isBrushSet(ColorGroup cg, ColorRole cr) const
{
   Q_UNUSED(cg);
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
         qWarning("QPalette::brush: Unknown ColorGroup(1): %d", (int)group1);
         group1 = Active;
      }
   }
   if (group2 >= (int)NColorGroups) {
      if (group2 == Current) {
         group2 = (ColorGroup)current_group;
      } else {
         qWarning("QPalette::brush: Unknown ColorGroup(2): %d", (int)group2);
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

/*! \obsolete
*/
int QPalette::serialNumber() const
{
   return d->ser_no;
}

qint64 QPalette::cacheKey() const
{
   return (((qint64) d->ser_no) << 32) | ((qint64) (d->detach_no));
}

QPalette QPalette::resolve(const QPalette &other) const
{
   if ((*this == other && resolve_mask == other.resolve_mask)
         || resolve_mask == 0) {
      QPalette o = other;
      o.resolve_mask = resolve_mask;
      return o;
   }

   QPalette palette(*this);
   palette.detach();

   for (int role = 0; role < (int)NColorRoles; role++)
      if (!(resolve_mask & (1 << role)))
         for (int grp = 0; grp < (int)NColorGroups; grp++) {
            palette.d->br[grp][role] = other.d->br[grp][role];
         }

   return palette;
}

static const int NumOldRoles = 7;
static const int oldRoles[7] = { QPalette::Foreground, QPalette::Background, QPalette::Light,
                                 QPalette::Dark, QPalette::Mid, QPalette::Text, QPalette::Base
                               };

QDataStream &operator<<(QDataStream &s, const QPalette &p)
{
   for (int grp = 0; grp < (int)QPalette::NColorGroups; grp++) {
      int max = QPalette::ToolTipText + 1;

      if (s.version() <= QDataStream::Qt_4_3) {
         max = QPalette::AlternateBase + 1;
      }

      for (int r = 0; r < max; r++) {
         s << p.d->br[grp][r];
      }
   }
   return s;
}

static void readV1ColorGroup(QDataStream &s, QPalette &pal, QPalette::ColorGroup grp)
{
   for (int i = 0; i < NumOldRoles; ++i) {
      QColor col;
      s >> col;
      pal.setColor(grp, (QPalette::ColorRole)oldRoles[i], col);
   }
}

QDataStream &operator>>(QDataStream &s, QPalette &p)
{
   int max = QPalette::NColorRoles;

   if (s.version() <= QDataStream::Qt_4_3) {
      p = QPalette();
      max = QPalette::AlternateBase + 1;
   }

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
                  const QBrush &light, const QBrush &dark, const QBrush &mid, const QBrush &text,
                  const QBrush &bright_text, const QBrush &base, const QBrush &window)
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
void
QPalette::setColorGroup(ColorGroup cg, const QBrush &foreground, const QBrush &button,
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

