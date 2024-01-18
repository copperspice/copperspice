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

#ifndef QFONTCOMBOBOX_H
#define QFONTCOMBOBOX_H

#include <qcombobox.h>
#include <qfontdatabase.h>

#ifndef QT_NO_FONTCOMBOBOX

class QFontComboBoxPrivate;

class Q_GUI_EXPORT QFontComboBox : public QComboBox
{
   GUI_CS_OBJECT(QFontComboBox)

   GUI_CS_ENUM(FontFilter)
   GUI_CS_FLAG(FontFilter, FontFilters)

   GUI_CS_PROPERTY_READ(writingSystem, writingSystem)
   GUI_CS_PROPERTY_WRITE(writingSystem, setWritingSystem)

   GUI_CS_PROPERTY_READ(fontFilters, fontFilters)
   GUI_CS_PROPERTY_WRITE(fontFilters, setFontFilters)

   GUI_CS_PROPERTY_READ(currentFont, currentFont)
   GUI_CS_PROPERTY_WRITE(currentFont, setCurrentFont)
   GUI_CS_PROPERTY_NOTIFY(currentFont, currentFontChanged)

 public:
   explicit QFontComboBox(QWidget *parent = nullptr);

   QFontComboBox(const QFontComboBox &) = delete;
   QFontComboBox &operator=(const QFontComboBox &) = delete;

   ~QFontComboBox();

   void setWritingSystem(QFontDatabase::WritingSystem script);
   QFontDatabase::WritingSystem writingSystem() const;

   GUI_CS_REGISTER_ENUM(
      enum FontFilter {
         AllFonts          = 0,
         ScalableFonts     = 0x1,
         NonScalableFonts  = 0x2,
         MonospacedFonts   = 0x4,
         ProportionalFonts = 0x8
      };
   )
   using FontFilters = QFlags<FontFilter>;

   void setFontFilters(FontFilters filters);
   FontFilters fontFilters() const;

   QFont currentFont() const;
   QSize sizeHint() const override;

   GUI_CS_SLOT_1(Public, void setCurrentFont(const QFont &font))
   GUI_CS_SLOT_2(setCurrentFont)

   GUI_CS_SIGNAL_1(Public, void currentFontChanged(const QFont &font))
   GUI_CS_SIGNAL_2(currentFontChanged, font)

 protected:
   bool event(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QFontComboBox)

   GUI_CS_SLOT_1(Private, void _q_currentChanged(const QString &text))
   GUI_CS_SLOT_2(_q_currentChanged)

   GUI_CS_SLOT_1(Private, void _q_updateModel())
   GUI_CS_SLOT_2(_q_updateModel)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFontComboBox::FontFilters)


#endif // QT_NO_FONTCOMBOBOX
#endif
