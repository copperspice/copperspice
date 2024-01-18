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

#ifndef QPLATFORM_DIALOGHELPER_H
#define QPLATFORM_DIALOGHELPER_H

#include <qglobal.h>
#include <qfont.h>
#include <qfiledialog.h>
#include <qobject.h>
#include <qlist.h>
#include <qshareddatapointer.h>
#include <qsharedpointer.h>
#include <qstring.h>
#include <qdir.h>
#include <qurl.h>
#include <qrgb.h>

class QColor;
class QWindow;
class QVariant;
class QUrl;
class QColorDialogOptionsPrivate;
class QFontDialogOptionsPrivate;
class QPlatformFileDialogOptionsPrivate;
class QMessageDialogOptionsPrivate;

class Q_GUI_EXPORT QPlatformDialogHelper : public QObject
{
   GUI_CS_OBJECT(QPlatformDialogHelper)

   GUI_CS_ENUM(ButtonRole)
   GUI_CS_ENUM(ButtonLayout)

   GUI_CS_FLAG(StandardButton, StandardButtons)

 public:
   enum StyleHint { };

   enum DialogCode {
      Rejected,
      Accepted
   };

   enum StandardButton {
      // keep this in sync with QDialogButtonBox::StandardButton and QMessageBox::StandardButton
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
      LowestBit          = 10,                // internal: log2(FirstButton)
      HighestBit         = 27                 // internal: log2(LastButton)
   };

   using StandardButtons = QFlags<StandardButton>;

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
      NRoles,

      RoleMask        = 0x0FFFFFFF,
      AlternateRole   = 0x10000000,
      Stretch         = 0x20000000,
      Reverse         = 0x40000000,
      EOL             = InvalidRole
   };

   enum ButtonLayout {
      // keep this in sync with QDialogButtonBox::ButtonLayout and QMessageBox::ButtonLayout
      UnknownLayout = -1,
      WinLayout,
      MacLayout,
      KdeLayout,
      GnomeLayout,
      MacModelessLayout
   };

   QPlatformDialogHelper();
   virtual ~QPlatformDialogHelper();

   virtual QVariant styleHint(StyleHint hint) const;

   virtual void exec() = 0;
   virtual bool show(Qt::WindowFlags flags, Qt::WindowModality windowModality, QWindow *parent) = 0;
   virtual void hide() = 0;

   static QVariant defaultStyleHint(QPlatformDialogHelper::StyleHint hint);

   static const int *buttonLayout(Qt::Orientation orientation = Qt::Horizontal, ButtonLayout policy = UnknownLayout);
   static ButtonRole buttonRole(StandardButton button);

   GUI_CS_SIGNAL_1(Public, void accept())
   GUI_CS_SIGNAL_2(accept)

   GUI_CS_SIGNAL_1(Public, void reject())
   GUI_CS_SIGNAL_2(reject)
};

class Q_GUI_EXPORT QColorDialogOptions
{
   GUI_CS_GADGET(QColorDialogOptions)

   GUI_CS_FLAG(ColorDialogOption, ColorDialogOptions)

 public:
   enum ColorDialogOption {
      ShowAlphaChannel    = 0x00000001,
      NoButtons           = 0x00000002,
      DontUseNativeDialog = 0x00000004
   };

   using ColorDialogOptions = QFlags<ColorDialogOption>;

   QColorDialogOptions();
   QColorDialogOptions(const QColorDialogOptions &rhs);
   QColorDialogOptions &operator=(const QColorDialogOptions &rhs);

   ~QColorDialogOptions();

   void swap(QColorDialogOptions &other) {
      qSwap(d, other.d);
   }

   QString windowTitle() const;
   void setWindowTitle(const QString &);

   void setOption(ColorDialogOption option, bool on = true);
   bool testOption(ColorDialogOption option) const;
   void setOptions(ColorDialogOptions options);
   ColorDialogOptions options() const;

   static int customColorCount();
   static QRgb customColor(int index);
   static QRgb *customColors();
   static void setCustomColor(int index, QRgb color);

   static QRgb *standardColors();
   static QRgb standardColor(int index);
   static void setStandardColor(int index, QRgb color);

 private:
   QSharedDataPointer<QColorDialogOptionsPrivate> d;
};

class Q_GUI_EXPORT QPlatformColorDialogHelper : public QPlatformDialogHelper
{
   GUI_CS_OBJECT(QPlatformColorDialogHelper)

 public:
   const QSharedPointer<QColorDialogOptions> &options() const;
   void setOptions(const QSharedPointer<QColorDialogOptions> &options);

   virtual void setCurrentColor(const QColor &) = 0;
   virtual QColor currentColor() const = 0;

   GUI_CS_SIGNAL_1(Public, void currentColorChanged(const QColor &color))
   GUI_CS_SIGNAL_2(currentColorChanged, color)

   GUI_CS_SIGNAL_1(Public, void colorSelected(const QColor &color))
   GUI_CS_SIGNAL_2(colorSelected, color)

 private:
   QSharedPointer<QColorDialogOptions> m_options;
};

class Q_GUI_EXPORT QFontDialogOptions
{
   GUI_CS_GADGET(QFontDialogOptions)

   GUI_CS_FLAG(FontDialogOption, FontDialogOptions)

 public:
   enum FontDialogOption {
      NoButtons           = 0x00000001,
      DontUseNativeDialog = 0x00000002,
      ScalableFonts       = 0x00000004,
      NonScalableFonts    = 0x00000008,
      MonospacedFonts     = 0x00000010,
      ProportionalFonts   = 0x00000020
   };

   using FontDialogOptions = QFlags<FontDialogOption>;

   QFontDialogOptions();
   QFontDialogOptions(const QFontDialogOptions &rhs);
   QFontDialogOptions &operator=(const QFontDialogOptions &rhs);
   ~QFontDialogOptions();

   void swap(QFontDialogOptions &other) {
      qSwap(d, other.d);
   }

   QString windowTitle() const;
   void setWindowTitle(const QString &);

   void setOption(FontDialogOption option, bool on = true);
   bool testOption(FontDialogOption option) const;
   void setOptions(FontDialogOptions options);
   FontDialogOptions options() const;

 private:
   QSharedDataPointer<QFontDialogOptionsPrivate> d;
};


class Q_GUI_EXPORT QPlatformFontDialogHelper : public QPlatformDialogHelper
{
   GUI_CS_OBJECT(QPlatformFontDialogHelper)

 public:
   virtual void setCurrentFont(const QFont &) = 0;
   virtual QFont currentFont() const = 0;

   const QSharedPointer<QFontDialogOptions> &options() const;
   void setOptions(const QSharedPointer<QFontDialogOptions> &options);

   GUI_CS_SIGNAL_1(Public, void currentFontChanged(const QFont &font))
   GUI_CS_SIGNAL_2(currentFontChanged, font)

   GUI_CS_SIGNAL_1(Public, void fontSelected(const QFont &font))
   GUI_CS_SIGNAL_2(fontSelected, font)

 private:
   QSharedPointer<QFontDialogOptions> m_options;
};

class Q_GUI_EXPORT QPlatformFileDialogOptions
{
   GUI_CS_GADGET(QPlatformFileDialogOptions)

   GUI_CS_ENUM(ViewMode)
   GUI_CS_ENUM(FileMode)
   GUI_CS_ENUM(AcceptMode)
   GUI_CS_ENUM(DialogLabel)

 public:
   enum ViewMode { Detail, List };
   enum FileMode { AnyFile, ExistingFile, Directory, ExistingFiles, DirectoryOnly };
   enum AcceptMode { AcceptOpen, AcceptSave };
   enum DialogLabel { LookIn, FileName, FileType, Accept, Reject, DialogLabelCount };

   QPlatformFileDialogOptions();

   QPlatformFileDialogOptions(const QPlatformFileDialogOptions &other);
   QPlatformFileDialogOptions &operator=(const QPlatformFileDialogOptions &other);

   ~QPlatformFileDialogOptions();

   void swap(QPlatformFileDialogOptions &other) {
      qSwap(d, other.d);
   }

   QString windowTitle() const;
   void setWindowTitle(const QString &);

   void setOption(QFileDialog::FileDialogOption option, bool on = true);
   bool testOption(QFileDialog::FileDialogOption option) const;
   void setOptions(QFileDialog::FileDialogOptions options);

   QFileDialog::FileDialogOptions options() const;

   QDir::Filters filter() const;
   void setFilter(QDir::Filters filters);

   void setViewMode(ViewMode mode);
   ViewMode viewMode() const;

   void setFileMode(FileMode mode);
   FileMode fileMode() const;

   void setAcceptMode(AcceptMode mode);
   AcceptMode acceptMode() const;

   void setSidebarUrls(const QList<QUrl> &urls);
   QList<QUrl> sidebarUrls() const;

   void setNameFilters(const QStringList &filters);
   QStringList nameFilters() const;

   void setMimeTypeFilters(const QStringList &filters);
   QStringList mimeTypeFilters() const;

   void setDefaultSuffix(const QString &suffix);
   QString defaultSuffix() const;

   void setHistory(const QStringList &paths);
   QStringList history() const;

   void setLabelText(DialogLabel label, const QString &text);
   QString labelText(DialogLabel label) const;
   bool isLabelExplicitlySet(DialogLabel label);

   QUrl initialDirectory() const;
   void setInitialDirectory(const QUrl &);

   QString initiallySelectedNameFilter() const;
   void setInitiallySelectedNameFilter(const QString &);

   QList<QUrl> initiallySelectedFiles() const;
   void setInitiallySelectedFiles(const QList<QUrl> &);

   void setSupportedSchemes(const QStringList &schemes);
   QStringList supportedSchemes() const;

 private:
   QSharedDataPointer<QPlatformFileDialogOptionsPrivate> d;
};

class Q_GUI_EXPORT QPlatformFileDialogHelper : public QPlatformDialogHelper
{
   GUI_CS_OBJECT(QPlatformFileDialogHelper)

 public:
   virtual bool defaultNameFilterDisables() const = 0;
   virtual void setDirectory(const QUrl &directory) = 0;
   virtual QUrl directory() const = 0;
   virtual void selectFile(const QUrl &filename) = 0;
   virtual QList<QUrl> selectedFiles() const = 0;
   virtual void setFilter() = 0;
   virtual void selectNameFilter(const QString &filter) = 0;
   virtual QString selectedNameFilter() const = 0;

   virtual bool isSupportedUrl(const QUrl &url) const;

   const QSharedPointer<QPlatformFileDialogOptions> &options() const;
   void setOptions(const QSharedPointer<QPlatformFileDialogOptions> &options);

   static QStringList cleanFilterList(const QString &filter);
   static const QString filterRegExp;

   GUI_CS_SIGNAL_1(Public, void fileSelected(const QUrl &file))
   GUI_CS_SIGNAL_2(fileSelected, file)
   GUI_CS_SIGNAL_1(Public, void filesSelected(const QList <QUrl> &files))
   GUI_CS_SIGNAL_2(filesSelected, files)
   GUI_CS_SIGNAL_1(Public, void currentChanged(const QUrl &path))
   GUI_CS_SIGNAL_2(currentChanged, path)
   GUI_CS_SIGNAL_1(Public, void directoryEntered(const QUrl &directory))
   GUI_CS_SIGNAL_2(directoryEntered, directory)
   GUI_CS_SIGNAL_1(Public, void filterSelected(const QString &filter))
   GUI_CS_SIGNAL_2(filterSelected, filter)

 private:
   QSharedPointer<QPlatformFileDialogOptions> m_options;
};

class Q_GUI_EXPORT QMessageDialogOptions
{
   GUI_CS_GADGET(QMessageDialogOptions)
   GUI_CS_ENUM(Icon)

 public:
   // Keep in sync with QMessageBox::Icon
   enum Icon { NoIcon, Information, Warning, Critical, Question };

   QMessageDialogOptions();
   QMessageDialogOptions(const QMessageDialogOptions &rhs);
   QMessageDialogOptions &operator=(const QMessageDialogOptions &rhs);
   ~QMessageDialogOptions();

   void swap(QMessageDialogOptions &other) {
      qSwap(d, other.d);
   }

   QString windowTitle() const;
   void setWindowTitle(const QString &);

   void setIcon(Icon icon);
   Icon icon() const;

   void setText(const QString &text);
   QString text() const;

   void setInformativeText(const QString &text);
   QString informativeText() const;

   void setDetailedText(const QString &text);
   QString detailedText() const;

   void setStandardButtons(QPlatformDialogHelper::StandardButtons buttons);
   QPlatformDialogHelper::StandardButtons standardButtons() const;

 private:
   QSharedDataPointer<QMessageDialogOptionsPrivate> d;
};

class Q_GUI_EXPORT QPlatformMessageDialogHelper : public QPlatformDialogHelper
{
   GUI_CS_OBJECT(QPlatformMessageDialogHelper)

 public:
   const QSharedPointer<QMessageDialogOptions> &options() const;
   void setOptions(const QSharedPointer<QMessageDialogOptions> &options);

   GUI_CS_SIGNAL_1(Public, void clicked(QPlatformDialogHelper::StandardButton button, QPlatformDialogHelper::ButtonRole role))
   GUI_CS_SIGNAL_2(clicked, button, role)

 private:
   QSharedPointer<QMessageDialogOptions> m_options;
};

#endif
