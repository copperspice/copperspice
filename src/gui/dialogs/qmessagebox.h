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

#ifndef QMESSAGEBOX_H
#define QMESSAGEBOX_H

#include <qdialog.h>
#include <qplatform_dialoghelper.h>

#ifndef QT_NO_MESSAGEBOX

class QLabel;
class QMessageBoxPrivate;
class QAbstractButton;
class QCheckBox;

class Q_GUI_EXPORT QMessageBox : public QDialog
{
   GUI_CS_OBJECT(QMessageBox)

   GUI_CS_ENUM(Icon)
   GUI_CS_FLAG(StandardButton, StandardButtons)

   GUI_CS_PROPERTY_READ(text, text)
   GUI_CS_PROPERTY_WRITE(text, setText)

   GUI_CS_PROPERTY_READ(icon, icon)
   GUI_CS_PROPERTY_WRITE(icon, setIcon)

   GUI_CS_PROPERTY_READ(iconPixmap, iconPixmap)
   GUI_CS_PROPERTY_WRITE(iconPixmap, setIconPixmap)

   GUI_CS_PROPERTY_READ(textFormat, textFormat)
   GUI_CS_PROPERTY_WRITE(textFormat, setTextFormat)

   GUI_CS_PROPERTY_READ(standardButtons, standardButtons)
   GUI_CS_PROPERTY_WRITE(standardButtons, setStandardButtons)

#ifndef QT_NO_TEXTEDIT
   GUI_CS_PROPERTY_READ(detailedText, detailedText)
   GUI_CS_PROPERTY_WRITE(detailedText, setDetailedText)
#endif

   GUI_CS_PROPERTY_READ(informativeText, informativeText)
   GUI_CS_PROPERTY_WRITE(informativeText, setInformativeText)

 public:
   enum Icon {
      NoIcon      = 0,
      Information = 1,
      Warning     = 2,
      Critical    = 3,
      Question    = 4
   };

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

   enum StandardButton {
      // keep this in sync with QDialogButtonBox::StandardButton
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
      RestoreDefaults    = 0x08000000,

      FirstButton        = Ok,                // internal
      LastButton         = RestoreDefaults,   // internal

      YesAll             = YesToAll,          // obsolete
      NoAll              = NoToAll,           // obsolete

      Default            = 0x00000100,        // obsolete
      Escape             = 0x00000200,        // obsolete
      FlagMask           = 0x00000300,        // obsolete
      ButtonMask         = ~FlagMask          // obsolete
   };
   typedef StandardButton Button;  // obsolete

   using StandardButtons = QFlags<StandardButton>;

   explicit QMessageBox(QWidget *parent = nullptr);
   QMessageBox(Icon icon, const QString &title, const QString &text, StandardButtons buttons = NoButton,
      QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

   QMessageBox(const QMessageBox &) = delete;
   QMessageBox &operator=(const QMessageBox &) = delete;

   ~QMessageBox();

   void addButton(QAbstractButton *button, ButtonRole role);
   QPushButton *addButton(const QString &text, ButtonRole role);
   QPushButton *addButton(StandardButton button);
   void removeButton(QAbstractButton *button);

   using QDialog::open;
   void open(QObject *receiver, const QString &member);

   QList<QAbstractButton *> buttons() const;
   ButtonRole buttonRole(QAbstractButton *button) const;

   void setStandardButtons(StandardButtons buttons);
   StandardButtons standardButtons() const;
   StandardButton standardButton(QAbstractButton *button) const;
   QAbstractButton *button(StandardButton which) const;

   QPushButton *defaultButton() const;
   void setDefaultButton(QPushButton *button);
   void setDefaultButton(StandardButton button);

   QAbstractButton *escapeButton() const;
   void setEscapeButton(QAbstractButton *button);
   void setEscapeButton(StandardButton button);

   QAbstractButton *clickedButton() const;

   QString text() const;
   void setText(const QString &text);

   Icon icon() const;
   void setIcon(QMessageBox::Icon icon);

   QPixmap iconPixmap() const;
   void setIconPixmap(const QPixmap &pixmap);

   Qt::TextFormat textFormat() const;
   void setTextFormat(Qt::TextFormat format);

   void setTextInteractionFlags(Qt::TextInteractionFlags flags);
   Qt::TextInteractionFlags textInteractionFlags() const;

   void setCheckBox(QCheckBox *cb);
   QCheckBox *checkBox() const;

   static StandardButton information(QWidget *parent, const QString &title,
      const QString &text, StandardButtons buttons = Ok, StandardButton defaultButton = NoButton);

   static StandardButton question(QWidget *parent, const QString &title,
      const QString &text, StandardButtons buttons = StandardButtons(Yes | No),
      StandardButton defaultButton = NoButton);

   static StandardButton warning(QWidget *parent, const QString &title,
      const QString &text, StandardButtons buttons = Ok, StandardButton defaultButton = NoButton);

   static StandardButton critical(QWidget *parent, const QString &title,
      const QString &text, StandardButtons buttons = Ok, StandardButton defaultButton = NoButton);

   static void about(QWidget *parent, const QString &title, const QString &text);
   static void aboutCs(QWidget *parent, const QString &title = QString());
   static void aboutQt(QWidget *parent, const QString &title = QString());


   // some of the following are obsolete since they use button and StandardButton
   // emerald - consider removing

   QMessageBox(const QString &title, const QString &text, Icon icon,
      int button0, int button1, int button2, QWidget *parent = nullptr,
      Qt::WindowFlags flags = Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

   static int information(QWidget *parent, const QString &title, const QString &text,
      int button0, int button1 = 0, int button2 = 0);

   static int information(QWidget *parent, const QString &title, const QString &text, const QString &button0Text,
      const QString &button1Text = QString(), const QString &button2Text = QString(),
      int defaultButtonNumber = 0, int escapeButtonNumber = -1);

   static StandardButton information(QWidget *parent, const QString &title, const QString &text,
      StandardButton button0, StandardButton button1 = NoButton) {
      return information(parent, title, text, StandardButtons(button0), button1);
   }

   static int question(QWidget *parent, const QString &title, const QString &text,
      int button0, int button1 = 0, int button2 = 0);

   static int question(QWidget *parent, const QString &title, const QString &text, const QString &button0Text,
      const QString &button1Text = QString(), const QString &button2Text = QString(),
      int defaultButtonNumber = 0, int escapeButtonNumber = -1);

   static int question(QWidget *parent, const QString &title, const QString &text,
      StandardButton button0, StandardButton button1) {
      return question(parent, title, text, StandardButtons(button0), button1);
   }

   static int warning(QWidget *parent, const QString &title, const QString &text, int button0, int button1,
      int button2 = 0);

   static int warning(QWidget *parent, const QString &title, const QString &text, const QString &button0Text,
      const QString &button1Text = QString(), const QString &button2Text = QString(),
      int defaultButtonNumber = 0, int escapeButtonNumber = -1);

   static int warning(QWidget *parent, const QString &title, const QString &text, StandardButton button0, StandardButton button1) {
      return warning(parent, title, text, StandardButtons(button0), button1);
   }

   static int critical(QWidget *parent, const QString &title, const QString &text, int button0, int button1, int button2 = 0);

   static int critical(QWidget *parent, const QString &title, const QString &text,
      const QString &button0Text, const QString &button1Text = QString(), const QString &button2Text = QString(),
      int defaultButtonNumber = 0, int escapeButtonNumber = -1);

   static int critical(QWidget *parent, const QString &title, const QString &text, StandardButton button0, StandardButton button1) {
      return critical(parent, title, text, StandardButtons(button0), button1);
   }

   QString buttonText(int button) const;
   void setButtonText(int button, const QString &text);

   QString informativeText() const;
   void setInformativeText(const QString &text);

#ifndef QT_NO_TEXTEDIT
   QString detailedText() const;
   void setDetailedText(const QString &text);
#endif

   void setWindowTitle(const QString &title);
   void setWindowModality(Qt::WindowModality windowModality);

   static QPixmap standardIcon(Icon icon);
   GUI_CS_SIGNAL_1(Public, void buttonClicked(QAbstractButton *button))
   GUI_CS_SIGNAL_2(buttonClicked, button)

 protected:
   bool event(QEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void showEvent(QShowEvent *event) override;
   void closeEvent(QCloseEvent *event) override;
   void keyPressEvent(QKeyEvent *event) override;
   void changeEvent(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QMessageBox)

   GUI_CS_SLOT_1(Private, void _q_clicked(QPlatformDialogHelper::StandardButton button, QPlatformDialogHelper::ButtonRole role))
   GUI_CS_SLOT_2(_q_clicked)

   GUI_CS_SLOT_1(Private, void _q_buttonClicked(QAbstractButton *button))
   GUI_CS_SLOT_2(_q_buttonClicked)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMessageBox::StandardButtons)

#define QT_REQUIRE_VERSION(argc, argv, str) static_assert(0, "Macro QT_REQUIRE_VERSION(argc, argv, str) has been" \
   " removed, use function cs_require_version(argc, argv, str) instead");

Q_GUI_EXPORT void cs_require_version(int argc, char *argv[], const char *str);

#endif // QT_NO_MESSAGEBOX


#endif
