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

#ifndef QPALETTE_H
#define QPALETTE_H

#include <qwindowdefs.h>
#include <qcolor.h>
#include <qbrush.h>

class QPalettePrivate;
class QVariant;

class Q_GUI_EXPORT QPalette
{
   GUI_CS_GADGET(QPalette)

   GUI_CS_ENUM(ColorGroup)
   GUI_CS_ENUM(ColorRole)

 public:
   // order dependent, serialization format depends on it
   enum ColorGroup {
      Active,
      Disabled,
      Inactive,
      NColorGroups,
      Current,
      All,
      Normal = Active
   };

   GUI_CS_REGISTER_ENUM(
      enum ColorRole {
         WindowText,
         Button,
         Light,
         Midlight,
         Dark,
         Mid,
         Text,
         BrightText,
         ButtonText,
         Base,
         Window,
         Shadow,
         Highlight,
         HighlightedText,
         Link,
         LinkVisited,
         AlternateBase,
         NoRole,
         ToolTipBase,
         ToolTipText,
         NColorRoles     = ToolTipText + 1,   // must be the last unique value

         PlaceholderText = Text,
         Foreground      = WindowText,
         Background      = Window
      };
   )

   QPalette();

   QPalette(const QColor &button);
   QPalette(Qt::GlobalColor button);
   QPalette(const QColor &button, const QColor &window);

   QPalette(const QBrush &windowText, const QBrush &button, const QBrush &light,
      const QBrush &dark, const QBrush &mid, const QBrush &text,
      const QBrush &bright_text, const QBrush &base, const QBrush &window);

   QPalette(const QColor &windowText, const QColor &window, const QColor &light,
      const QColor &dark, const QColor &mid, const QColor &text, const QColor &base);

   QPalette(const QPalette &other);

   QPalette(QPalette &&other)
      : d(other.d)
   {
      resolve_mask  = other.resolve_mask;
      current_group = other.current_group;
      other.d = nullptr;
   }

   ~QPalette();

   QPalette &operator=(const QPalette &other);

   QPalette &operator=(QPalette &&other) {
      resolve_mask  = other.resolve_mask;
      current_group = other.current_group;
      qSwap(d, other.d);

      return *this;
   }

   operator QVariant() const;

   void swap(QPalette &other) {
      qSwap(resolve_mask,  other.resolve_mask);
      qSwap(current_group, other.current_group);
      qSwap(d, other.d);
   }

   ColorGroup currentColorGroup() const {
      return static_cast<ColorGroup>(current_group);
   }

   void setCurrentColorGroup(ColorGroup group) {
      current_group = group;
   }

   const QColor &color(ColorGroup group, ColorRole role) const {
      return brush(group, role).color();
   }

   const QBrush &brush(ColorGroup group, ColorRole role) const;

   inline void setColor(ColorGroup group, ColorRole role, const QColor &color);
   inline void setColor(ColorRole role, const QColor &color);
   inline void setBrush(ColorRole role, const QBrush &brush);

   bool isBrushSet(ColorGroup group, ColorRole role) const;
   void setBrush(ColorGroup group, ColorRole role, const QBrush &brush);

   void setColorGroup(ColorGroup group, const QBrush &windowText, const QBrush &button,
      const QBrush &light, const QBrush &dark, const QBrush &mid,
      const QBrush &text, const QBrush &bright_text, const QBrush &base, const QBrush &window);

   bool isEqual(ColorGroup group1, ColorGroup group2) const;

   const QColor &color(ColorRole role) const {
      return color(Current, role);
   }

   const QBrush &brush(ColorRole role) const {
      return brush(Current, role);
   }

   const QBrush &foreground() const {
      return brush(WindowText);
   }

   const QBrush &windowText() const {
      return brush(WindowText);
   }

   const QBrush &button() const {
      return brush(Button);
   }

   const QBrush &light() const {
      return brush(Light);
   }

   const QBrush &dark() const {
      return brush(Dark);
   }

   const QBrush &mid() const {
      return brush(Mid);
   }

   const QBrush &text() const {
      return brush(Text);
   }

   const QBrush &base() const {
      return brush(Base);
   }

   const QBrush &alternateBase() const {
      return brush(AlternateBase);
   }

   const QBrush &toolTipBase() const {
      return brush(ToolTipBase);
   }

   const QBrush &toolTipText() const {
      return brush(ToolTipText);
   }

   const QBrush &background() const {
      return brush(Window);
   }

   const QBrush &window() const {
      return brush(Window);
   }

   const QBrush &midlight() const {
      return brush(Midlight);
   }

   const QBrush &brightText() const {
      return brush(BrightText);
   }

   const QBrush &buttonText() const {
      return brush(ButtonText);
   }

   const QBrush &shadow() const {
      return brush(Shadow);
   }

   const QBrush &highlight() const {
      return brush(Highlight);
   }

   const QBrush &highlightedText() const {
      return brush(HighlightedText);
   }

   const QBrush &link() const {
      return brush(Link);
   }

   const QBrush &linkVisited() const {
      return brush(LinkVisited);
   }

   bool operator==(const QPalette &other) const;
   bool operator!=(const QPalette &other) const {
      return !(operator==(other));
   }

   bool isCopyOf(const QPalette &other) const;

   qint64 cacheKey() const;

   QPalette resolve(const QPalette &other) const;

   uint resolve() const {
      return resolve_mask;
   }

   void resolve(uint mask) {
      resolve_mask = mask;
   }

 private:
   void setColorGroup(ColorGroup cr, const QBrush &windowText, const QBrush &button,
      const QBrush &light, const QBrush &dark, const QBrush &mid,
      const QBrush &text, const QBrush &bright_text,
      const QBrush &base, const QBrush &alternate_base,
      const QBrush &window, const QBrush &midlight,
      const QBrush &button_text, const QBrush &shadow,
      const QBrush &highlight, const QBrush &highlighted_text,
      const QBrush &link, const QBrush &link_visited);

   void setColorGroup(ColorGroup cr, const QBrush &windowText, const QBrush &button,
      const QBrush &light, const QBrush &dark, const QBrush &mid,
      const QBrush &text, const QBrush &bright_text,
      const QBrush &base, const QBrush &alternate_base,
      const QBrush &window, const QBrush &midlight,
      const QBrush &button_text, const QBrush &shadow,
      const QBrush &highlight, const QBrush &highlighted_text,
      const QBrush &link, const QBrush &link_visited,
      const QBrush &toolTipBase, const QBrush &toolTipText);

   void init();
   void detach();

   QPalettePrivate *d;

   uint current_group;
   uint resolve_mask;

   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPalette &palette);
};

inline void QPalette::setColor(ColorGroup group, ColorRole role, const QColor &color)
{
   setBrush(group, role, QBrush(color));
}

inline void QPalette::setColor(ColorRole role, const QColor &color)
{
   setColor(All, role, color);
}

inline void QPalette::setBrush(ColorRole role, const QBrush &brush)
{
   setBrush(All, role, brush);
}

Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPalette &palette);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPalette &palette);
Q_GUI_EXPORT QDebug operator<<(QDebug, const QPalette &palette);

#endif
