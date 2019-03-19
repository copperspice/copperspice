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

#ifndef QCOLORDIALOG_H
#define QCOLORDIALOG_H

#include <qdialog.h>

#ifndef QT_NO_COLORDIALOG

class QColorDialogPrivate;

class Q_GUI_EXPORT QColorDialog : public QDialog
{
   GUI_CS_OBJECT(QColorDialog)
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
                  const QString &title = QString(),  ColorDialogOptions options = 0);

   static int customCount();
   static QColor customColor(int index);
   static void setCustomColor(int index, QColor color);
   static void setStandardColor(int index, QColor color);

   GUI_CS_SIGNAL_1(Public, void currentColorChanged(const QColor &color))
   GUI_CS_SIGNAL_2(currentColorChanged, color)

   GUI_CS_SIGNAL_1(Public, void colorSelected(const QColor &color))
   GUI_CS_SIGNAL_2(colorSelected, color)

 protected:
   void changeEvent(QEvent *event) override;
   void done(int result) override;

 private:
   Q_DISABLE_COPY(QColorDialog)

   GUI_CS_SLOT_1(Private, void _q_addCustom())
   GUI_CS_SLOT_2(_q_addCustom)

   GUI_CS_SLOT_1(Private, void _q_newHsv(int h, int s, int v))
   GUI_CS_SLOT_2(_q_newHsv)

   GUI_CS_SLOT_1(Private, void _q_newColorTypedIn(const QRgb &rgb))
   GUI_CS_SLOT_2(_q_newColorTypedIn)

   GUI_CS_SLOT_1(Private, void _q_newCustom(int un_named_arg1, int un_named_arg2))
   GUI_CS_SLOT_2(_q_newCustom)

   GUI_CS_SLOT_1(Private, void _q_newStandard(int un_named_arg1, int un_named_arg2))
   GUI_CS_SLOT_2(_q_newStandard)

#if defined(Q_OS_MAC)
   GUI_CS_SLOT_1(Private, void _q_macRunNativeAppModalPanel())
   GUI_CS_SLOT_2(_q_macRunNativeAppModalPanel)
#endif

   friend class QColorShower;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QColorDialog::ColorDialogOptions)

#endif // QT_NO_COLORDIALOG

#endif // QCOLORDIALOG_H
