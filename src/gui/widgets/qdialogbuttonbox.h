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

#ifndef QDIALOGBUTTONBOX_H
#define QDIALOGBUTTONBOX_H

#include <qwidget.h>

class QAbstractButton;
class QPushButton;
class QDialogButtonBoxPrivate;

class Q_GUI_EXPORT QDialogButtonBox : public QWidget
{
   GUI_CS_OBJECT(QDialogButtonBox)

   GUI_CS_ENUM(StandardButton)
   GUI_CS_FLAG(StandardButton, StandardButtons)

   GUI_CS_PROPERTY_READ(orientation, orientation)
   GUI_CS_PROPERTY_WRITE(orientation, setOrientation)

   GUI_CS_PROPERTY_READ(standardButtons, standardButtons)
   GUI_CS_PROPERTY_WRITE(standardButtons, setStandardButtons)

   GUI_CS_PROPERTY_READ(centerButtons, centerButtons)
   GUI_CS_PROPERTY_WRITE(centerButtons, setCenterButtons)

 public:
   enum ButtonRole {
      // keep QDialogButtonBox::ButtonRole, QMessageBox::ButtonRole, QPlatformDialogHelper ALL in sync

      InvalidRole = -1,
      AcceptRole,
      RejectRole,
      DestructiveRole,
      ActionRole,
      HelpRole,
      YesRole,
      NoRole,
      ResetRole,
      ApplyRole,
      NRoles
   };

   GUI_CS_REGISTER_ENUM(
      enum StandardButton {
         // keep this in sync with QMessageBox::StandardButton
         NoButton           = 0x00000000,
         Ok                 = 0x00000400,
         Save               = 0x00000800,
         SaveAll            = 0x00001000,
         Open               = 0x00002000,
         Yes                = 0x00004000,
         YesToAll           = 0x00008000,
         No                 = 0x00010000,
         NoToAll            = 0x00020000,
         Abort              = 0x00040000,
         Retry              = 0x00080000,
         Ignore             = 0x00100000,
         Close              = 0x00200000,
         Cancel             = 0x00400000,
         Discard            = 0x00800000,
         Help               = 0x01000000,
         Apply              = 0x02000000,
         Reset              = 0x04000000,
         RestoreDefaults    = 0x08000000
      };
   )

   // internal values
   static constexpr const auto FirstButton = StandardButton::Ok;
   static constexpr const auto LastButton  = StandardButton::RestoreDefaults;

   using StandardButtons = QFlags<StandardButton>;

   enum ButtonLayout {
      WinLayout,
      MacLayout,
      KdeLayout,
      GnomeLayout
   };

   QDialogButtonBox(QWidget *parent = nullptr);
   QDialogButtonBox(Qt::Orientation orientation, QWidget *parent = nullptr);
   QDialogButtonBox(StandardButtons buttons, Qt::Orientation orientation, QWidget *parent = nullptr);

   explicit QDialogButtonBox(StandardButtons buttons, QWidget *parent = nullptr);

   QDialogButtonBox(const QDialogButtonBox &) = delete;
   QDialogButtonBox &operator=(const QDialogButtonBox &) = delete;

   ~QDialogButtonBox();

   void setOrientation(Qt::Orientation orientation);
   Qt::Orientation orientation() const;

   void addButton(QAbstractButton *button, ButtonRole role);
   QPushButton *addButton(const QString &text, ButtonRole role);
   QPushButton *addButton(StandardButton button);
   void removeButton(QAbstractButton *button);
   void clear();

   QList<QAbstractButton *> buttons() const;
   ButtonRole buttonRole(QAbstractButton *button) const;

   void setStandardButtons(StandardButtons buttons);
   StandardButtons standardButtons() const;
   StandardButton standardButton(QAbstractButton *button) const;
   QPushButton *button(StandardButton which) const;

   void setCenterButtons(bool center);
   bool centerButtons() const;

   GUI_CS_SIGNAL_1(Public, void clicked(QAbstractButton *button))
   GUI_CS_SIGNAL_2(clicked, button)

   GUI_CS_SIGNAL_1(Public, void accepted())
   GUI_CS_SIGNAL_2(accepted)

   GUI_CS_SIGNAL_1(Public, void helpRequested())
   GUI_CS_SIGNAL_2(helpRequested)

   GUI_CS_SIGNAL_1(Public, void rejected())
   GUI_CS_SIGNAL_2(rejected)

 protected:
   void changeEvent(QEvent *event) override;
   bool event(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QDialogButtonBox)

   GUI_CS_SLOT_1(Private, void _q_handleButtonClicked())
   GUI_CS_SLOT_2(_q_handleButtonClicked)

   GUI_CS_SLOT_1(Private, void _q_handleButtonDestroyed())
   GUI_CS_SLOT_2(_q_handleButtonDestroyed)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDialogButtonBox::StandardButtons)

#endif // QDIALOGBUTTONBOX_H
