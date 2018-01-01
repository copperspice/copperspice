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

#ifndef QFONTCOMBOBOX_H
#define QFONTCOMBOBOX_H

#include <QtGui/qcombobox.h>
#include <QtGui/qfontdatabase.h>

#ifndef QT_NO_FONTCOMBOBOX

QT_BEGIN_NAMESPACE

class QFontComboBoxPrivate;

class Q_GUI_EXPORT QFontComboBox : public QComboBox
{
   GUI_CS_OBJECT(QFontComboBox)

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
   ~QFontComboBox();

   void setWritingSystem(QFontDatabase::WritingSystem);
   QFontDatabase::WritingSystem writingSystem() const;

   enum FontFilter {
      AllFonts = 0,
      ScalableFonts = 0x1,
      NonScalableFonts = 0x2,
      MonospacedFonts = 0x4,
      ProportionalFonts = 0x8
   };
   using FontFilters = QFlags<FontFilter>;

   void setFontFilters(FontFilters filters);
   FontFilters fontFilters() const;

   QFont currentFont() const;
   QSize sizeHint() const override;

   GUI_CS_SLOT_1(Public, void setCurrentFont(const QFont &f))
   GUI_CS_SLOT_2(setCurrentFont)

   GUI_CS_SIGNAL_1(Public, void currentFontChanged(const QFont &f))
   GUI_CS_SIGNAL_2(currentFontChanged, f)

 protected:
   bool event(QEvent *e) override;

 private:
   Q_DISABLE_COPY(QFontComboBox)
   Q_DECLARE_PRIVATE(QFontComboBox)

   GUI_CS_SLOT_1(Private, void _q_currentChanged(const QString &un_named_arg1))
   GUI_CS_SLOT_2(_q_currentChanged)

   GUI_CS_SLOT_1(Private, void _q_updateModel())
   GUI_CS_SLOT_2(_q_updateModel)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFontComboBox::FontFilters)

QT_END_NAMESPACE

#endif // QT_NO_FONTCOMBOBOX
#endif
