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

#ifndef QCOLORDIALOG_H
#define QCOLORDIALOG_H

#include <qdialog.h>

#ifndef QT_NO_COLORDIALOG

class QColorDialogPrivate;

class Q_GUI_EXPORT QColorDialog : public QDialog
{
   GUI_CS_OBJECT(QColorDialog)

   GUI_CS_ENUM(ColorDialogOption)
   Q_DECLARE_PRIVATE(QColorDialog)

   GUI_CS_ENUM(ColorDialogOption)
   GUI_CS_PROPERTY_READ(currentColor, currentColor)
   GUI_CS_PROPERTY_WRITE(currentColor, setCurrentColor)
   GUI_CS_PROPERTY_NOTIFY(currentColor, currentColorChanged)
   GUI_CS_PROPERTY_READ(options, options)
   GUI_CS_PROPERTY_WRITE(options, setOptions)

 public:
   enum ColorDialogOption {
      ShowAlphaChannel    = 0x00000001,
      NoButtons           = 0x00000002,
      DontUseNativeDialog = 0x00000004
   };

   using ColorDialogOptions = QFlags<ColorDialogOption>;

   explicit QColorDialog(QWidget *parent = nullptr);
   explicit QColorDialog(const QColor &initial, QWidget *parent = nullptr);

   QColorDialog(const QColorDialog &) = delete;
   QColorDialog &operator=(const QColorDialog &) = delete;

   ~QColorDialog();

   void setCurrentColor(const QColor &color);
   QColor currentColor() const;

   QColor selectedColor() const;

   void setOption(ColorDialogOption option, bool on = true);
   bool testOption(ColorDialogOption option) const;
   void setOptions(ColorDialogOptions options);
   ColorDialogOptions options() const;

   using QDialog::open;
   void open(QObject *receiver, const QString &member);

   void setVisible(bool visible) override;

   static QColor getColor(const QColor &initial = Qt::white, QWidget *parent = nullptr,
      const QString &title = QString(),  ColorDialogOptions options = ColorDialogOptions());

   // obsolete
   static QRgb getRgba(QRgb rgba = 0xffffffff, bool *ok = nullptr, QWidget *parent = nullptr);

   static int customCount();
   static QColor customColor(int index);
   static void setCustomColor(int index, QColor color);
   static QColor standardColor(int index);
   static void setStandardColor(int index, QColor color);

   GUI_CS_SIGNAL_1(Public, void currentColorChanged(const QColor &color))
   GUI_CS_SIGNAL_2(currentColorChanged, color)

   GUI_CS_SIGNAL_1(Public, void colorSelected(const QColor &color))
   GUI_CS_SIGNAL_2(colorSelected, color)

 protected:
   void changeEvent(QEvent *event) override;
   void done(int result) override;

 private:
   GUI_CS_SLOT_1(Private, void _q_addCustom())
   GUI_CS_SLOT_2(_q_addCustom)

   GUI_CS_SLOT_1(Private, void _q_newHsv(int h, int s, int v))
   GUI_CS_SLOT_2(_q_newHsv)

   GUI_CS_SLOT_1(Private, void _q_newColorTypedIn(QRgb rgb))
   GUI_CS_SLOT_2(_q_newColorTypedIn)

   GUI_CS_SLOT_1(Private, void _q_newCustom(int arg1, int arg2))
   GUI_CS_SLOT_2(_q_newCustom)

   GUI_CS_SLOT_1(Private, void _q_nextCustom(int arg1, int arg2))
   GUI_CS_SLOT_2(_q_nextCustom)

   GUI_CS_SLOT_1(Private, void _q_newStandard(int arg1, int arg2))
   GUI_CS_SLOT_2(_q_newStandard)

   GUI_CS_SLOT_1(Private, void _q_pickScreenColor())
   GUI_CS_SLOT_2(_q_pickScreenColor)

   GUI_CS_SLOT_1(Private, void _q_updateColorPicking())
   GUI_CS_SLOT_2(_q_updateColorPicking)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QColorDialog::ColorDialogOptions)

#endif // QT_NO_COLORDIALOG

#endif // QCOLORDIALOG_H
