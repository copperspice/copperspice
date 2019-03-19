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

#ifndef QFONTDIALOG_H
#define QFONTDIALOG_H

#include <QtGui/qwindowdefs.h>
#include <QtGui/qdialog.h>
#include <QtGui/qfont.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_FONTDIALOG

class QFontDialogPrivate;

class Q_GUI_EXPORT QFontDialog : public QDialog
{
   GUI_CS_OBJECT(QFontDialog)
   Q_DECLARE_PRIVATE(QFontDialog)

   GUI_CS_ENUM(FontDialogOption)
   GUI_CS_PROPERTY_READ(currentFont, currentFont)
   GUI_CS_PROPERTY_WRITE(currentFont, setCurrentFont)
   GUI_CS_PROPERTY_NOTIFY(currentFont, currentFontChanged)
   GUI_CS_PROPERTY_READ(options, options)
   GUI_CS_PROPERTY_WRITE(options, setOptions)

 public:
   enum FontDialogOption {
      NoButtons           = 0x00000001,
      DontUseNativeDialog = 0x00000002
   };

   using FontDialogOptions = QFlags<FontDialogOption>;

   explicit QFontDialog(QWidget *parent = nullptr);
   explicit QFontDialog(const QFont &initial, QWidget *parent = nullptr);
   ~QFontDialog();

   void setCurrentFont(const QFont &font);
   QFont currentFont() const;

   QFont selectedFont() const;

   void setOption(FontDialogOption option, bool on = true);
   bool testOption(FontDialogOption option) const;
   void setOptions(FontDialogOptions options);
   FontDialogOptions options() const;

   using QDialog::open;
   void open(QObject *receiver, const QString &member);

   void setVisible(bool visible) override;

   static QFont getFont(bool *ok, const QFont &initial, QWidget *parent = nullptr, const QString &title = QString(),
         FontDialogOptions options = FontDialogOptions());

   static QFont getFont(bool *ok, QWidget *parent = nullptr);

   GUI_CS_SIGNAL_1(Public, void currentFontChanged(const QFont &font))
   GUI_CS_SIGNAL_2(currentFontChanged, font)

   GUI_CS_SIGNAL_1(Public, void fontSelected(const QFont &font))
   GUI_CS_SIGNAL_2(fontSelected, font)

 protected:
   void changeEvent(QEvent *event) override;
   void done(int result) override;

   bool eventFilter(QObject *object, QEvent *event) override;

 private:
   Q_DISABLE_COPY(QFontDialog)

   GUI_CS_SLOT_1(Private, void _q_sizeChanged(const QString &un_named_arg1))
   GUI_CS_SLOT_2(_q_sizeChanged)

   GUI_CS_SLOT_1(Private, void _q_familyHighlighted(int un_named_arg1))
   GUI_CS_SLOT_2(_q_familyHighlighted)

   GUI_CS_SLOT_1(Private, void _q_writingSystemHighlighted(int un_named_arg1))
   GUI_CS_SLOT_2(_q_writingSystemHighlighted)

   GUI_CS_SLOT_1(Private, void _q_styleHighlighted(int un_named_arg1))
   GUI_CS_SLOT_2(_q_styleHighlighted)

   GUI_CS_SLOT_1(Private, void _q_sizeHighlighted(int un_named_arg1))
   GUI_CS_SLOT_2(_q_sizeHighlighted)

   GUI_CS_SLOT_1(Private, void _q_updateSample())
   GUI_CS_SLOT_2(_q_updateSample)

#if defined(Q_OS_MAC)
   GUI_CS_SLOT_1(Private, void _q_macRunNativeAppModalPanel())
   GUI_CS_SLOT_2(_q_macRunNativeAppModalPanel)
#endif

};

Q_DECLARE_OPERATORS_FOR_FLAGS(QFontDialog::FontDialogOptions)

#endif // QT_NO_FONTDIALOG

QT_END_NAMESPACE

#endif // QFONTDIALOG_H
