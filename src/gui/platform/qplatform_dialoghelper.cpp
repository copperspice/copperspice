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

#include <qplatform_dialoghelper.h>

#include <qshareddata.h>
#include <qsettings.h>
#include <qurl.h>
#include <qcolor.h>
#include <qregularexpression.h>
#include <qvariant.h>

#include <algorithm>

static const int buttonRoleLayouts[2][5][14] = {
   // Qt::Horizontal
   {
      // WinLayout
      {
         QPlatformDialogHelper::ResetRole, QPlatformDialogHelper::Stretch, QPlatformDialogHelper::YesRole, QPlatformDialogHelper::AcceptRole,
         QPlatformDialogHelper::AlternateRole, QPlatformDialogHelper::DestructiveRole, QPlatformDialogHelper::NoRole,
         QPlatformDialogHelper::ActionRole, QPlatformDialogHelper::RejectRole, QPlatformDialogHelper::ApplyRole,
         QPlatformDialogHelper::HelpRole, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL
      },

      // MacLayout
      {
         QPlatformDialogHelper::HelpRole, QPlatformDialogHelper::ResetRole, QPlatformDialogHelper::ApplyRole, QPlatformDialogHelper::ActionRole,
         QPlatformDialogHelper::Stretch, QPlatformDialogHelper::DestructiveRole | QPlatformDialogHelper::Reverse,
         QPlatformDialogHelper::AlternateRole | QPlatformDialogHelper::Reverse, QPlatformDialogHelper::RejectRole | QPlatformDialogHelper::Reverse,
         QPlatformDialogHelper::AcceptRole | QPlatformDialogHelper::Reverse, QPlatformDialogHelper::NoRole | QPlatformDialogHelper::Reverse,
         QPlatformDialogHelper::YesRole | QPlatformDialogHelper::Reverse, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL
      },

      // KdeLayout
      {
         QPlatformDialogHelper::HelpRole, QPlatformDialogHelper::ResetRole, QPlatformDialogHelper::Stretch, QPlatformDialogHelper::YesRole,
         QPlatformDialogHelper::NoRole, QPlatformDialogHelper::ActionRole, QPlatformDialogHelper::AcceptRole, QPlatformDialogHelper::AlternateRole,
         QPlatformDialogHelper::ApplyRole, QPlatformDialogHelper::DestructiveRole, QPlatformDialogHelper::RejectRole, QPlatformDialogHelper::EOL
      },

      // GnomeLayout
      {
         QPlatformDialogHelper::HelpRole, QPlatformDialogHelper::ResetRole, QPlatformDialogHelper::Stretch, QPlatformDialogHelper::ActionRole,
         QPlatformDialogHelper::ApplyRole | QPlatformDialogHelper::Reverse, QPlatformDialogHelper::DestructiveRole | QPlatformDialogHelper::Reverse,
         QPlatformDialogHelper::AlternateRole | QPlatformDialogHelper::Reverse, QPlatformDialogHelper::RejectRole | QPlatformDialogHelper::Reverse,
         QPlatformDialogHelper::AcceptRole | QPlatformDialogHelper::Reverse, QPlatformDialogHelper::NoRole | QPlatformDialogHelper::Reverse,
         QPlatformDialogHelper::YesRole | QPlatformDialogHelper::Reverse, QPlatformDialogHelper::EOL
      },

      // MacModelessLayout
      {
         QPlatformDialogHelper::ResetRole, QPlatformDialogHelper::ApplyRole, QPlatformDialogHelper::ActionRole, QPlatformDialogHelper::Stretch,
         QPlatformDialogHelper::HelpRole, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL,
         QPlatformDialogHelper::EOL,
         QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL,
         QPlatformDialogHelper::EOL
      }
   },

   // Qt::Vertical
   {
      // WinLayout
      {
         QPlatformDialogHelper::ActionRole, QPlatformDialogHelper::YesRole, QPlatformDialogHelper::AcceptRole,
         QPlatformDialogHelper::AlternateRole,
         QPlatformDialogHelper::DestructiveRole, QPlatformDialogHelper::NoRole, QPlatformDialogHelper::RejectRole,
         QPlatformDialogHelper::ApplyRole,
         QPlatformDialogHelper::ResetRole,
         QPlatformDialogHelper::HelpRole, QPlatformDialogHelper::Stretch, QPlatformDialogHelper::EOL,
         QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL
      },

      // MacLayout
      {
         QPlatformDialogHelper::YesRole, QPlatformDialogHelper::NoRole, QPlatformDialogHelper::AcceptRole,
         QPlatformDialogHelper::RejectRole,
         QPlatformDialogHelper::AlternateRole, QPlatformDialogHelper::DestructiveRole, QPlatformDialogHelper::Stretch,
         QPlatformDialogHelper::ActionRole,
         QPlatformDialogHelper::ApplyRole,
         QPlatformDialogHelper::ResetRole, QPlatformDialogHelper::HelpRole, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL
      },

      // KdeLayout
      {
         QPlatformDialogHelper::AcceptRole, QPlatformDialogHelper::AlternateRole, QPlatformDialogHelper::ApplyRole,
         QPlatformDialogHelper::ActionRole,
         QPlatformDialogHelper::YesRole, QPlatformDialogHelper::NoRole, QPlatformDialogHelper::Stretch,
         QPlatformDialogHelper::ResetRole,
         QPlatformDialogHelper::DestructiveRole, QPlatformDialogHelper::RejectRole, QPlatformDialogHelper::HelpRole,
         QPlatformDialogHelper::EOL
      },

      // GnomeLayout
      {
         QPlatformDialogHelper::YesRole, QPlatformDialogHelper::NoRole, QPlatformDialogHelper::AcceptRole,
         QPlatformDialogHelper::RejectRole,
         QPlatformDialogHelper::AlternateRole, QPlatformDialogHelper::DestructiveRole, QPlatformDialogHelper::ApplyRole,
         QPlatformDialogHelper::ActionRole,
         QPlatformDialogHelper::Stretch,
         QPlatformDialogHelper::ResetRole, QPlatformDialogHelper::HelpRole, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL,
         QPlatformDialogHelper::EOL
      },

      // MacModelessLayout
      {
         QPlatformDialogHelper::ActionRole, QPlatformDialogHelper::ApplyRole, QPlatformDialogHelper::ResetRole,
         QPlatformDialogHelper::Stretch,
         QPlatformDialogHelper::HelpRole, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL,
         QPlatformDialogHelper::EOL,
         QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL, QPlatformDialogHelper::EOL,
         QPlatformDialogHelper::EOL
      }
   }
};

QPlatformDialogHelper::QPlatformDialogHelper()
{
}

QPlatformDialogHelper::~QPlatformDialogHelper()
{
}

QVariant QPlatformDialogHelper::styleHint(StyleHint hint) const
{
   return QPlatformDialogHelper::defaultStyleHint(hint);
}

QVariant  QPlatformDialogHelper::defaultStyleHint(QPlatformDialogHelper::StyleHint hint)
{
   (void) hint;
   return QVariant();
}

// Font dialog

class QFontDialogOptionsPrivate : public QSharedData
{
 public:
   QFontDialogOptionsPrivate()
      : options(Qt::EmptyFlag)
   {
   }

   QFontDialogOptions::FontDialogOptions options;
   QString windowTitle;
};

QFontDialogOptions::QFontDialogOptions() : d(new QFontDialogOptionsPrivate)
{
}

QFontDialogOptions::QFontDialogOptions(const QFontDialogOptions &rhs) : d(rhs.d)
{
}

QFontDialogOptions &QFontDialogOptions::operator=(const QFontDialogOptions &rhs)
{
   if (this != &rhs) {
      d = rhs.d;
   }

   return *this;
}

QFontDialogOptions::~QFontDialogOptions()
{
}

QString QFontDialogOptions::windowTitle() const
{
   return d->windowTitle;
}

void QFontDialogOptions::setWindowTitle(const QString &title)
{
   d->windowTitle = title;
}

void QFontDialogOptions::setOption(QFontDialogOptions::FontDialogOption option, bool on)
{
   if (!(d->options & option) != !on) {
      setOptions(d->options ^ option);
   }
}

bool QFontDialogOptions::testOption(QFontDialogOptions::FontDialogOption option) const
{
   return d->options & option;
}

void QFontDialogOptions::setOptions(FontDialogOptions options)
{
   if (options != d->options) {
      d->options = options;
   }
}

QFontDialogOptions::FontDialogOptions QFontDialogOptions::options() const
{
   return d->options;
}

const QSharedPointer<QFontDialogOptions> &QPlatformFontDialogHelper::options() const
{
   return m_options;
}

void QPlatformFontDialogHelper::setOptions(const QSharedPointer<QFontDialogOptions> &options)
{
   m_options = options;
}

// Color dialog

class QColorDialogStaticData
{
 public:
   static constexpr const int CustomColorCount   = 16;
   static constexpr const int StandardColorCount = 6 * 8;

   QColorDialogStaticData();
   inline void readSettings();
   inline void writeSettings() const;

   QRgb customRgb[CustomColorCount];
   QRgb standardRgb[StandardColorCount];
   bool customSet;
};

QColorDialogStaticData::QColorDialogStaticData() : customSet(false)
{
   int i = 0;

   for (int g = 0; g < 4; ++g)  {
      for (int r = 0;  r < 4; ++r) {
         for (int b = 0; b < 3; ++b) {
            standardRgb[i++] = qRgb(r * 255 / 3, g * 255 / 3, b * 255 / 2);
         }
      }
   }

   std::fill(customRgb, customRgb + CustomColorCount, 0xffffffff);
   readSettings();
}

void QColorDialogStaticData::readSettings()
{
#ifndef QT_NO_SETTINGS
   const QSettings settings(QSettings::UserScope, "CsProject");

   for (int i = 0; i < int(CustomColorCount); ++i) {
      const QVariant v = settings.value("CS/customColors/" + QString::number(i));
      if (v.isValid()) {
         customRgb[i] = v.toUInt();
      }
   }
#endif
}

void QColorDialogStaticData::writeSettings() const
{
#ifndef QT_NO_SETTINGS
   if (!customSet) {
      QSettings settings(QSettings::UserScope, "CsProject");

      for (int i = 0; i < int(CustomColorCount); ++i) {
         settings.setValue("CS/customColors/" + QString::number(i), customRgb[i]);
      }
   }
#endif
}

static QColorDialogStaticData *qColorDialogStaticData()
{
   static QColorDialogStaticData retval;
   return &retval;
}

class QColorDialogOptionsPrivate : public QSharedData
{
 public:
   QColorDialogOptionsPrivate()
      : options(Qt::EmptyFlag)
   {
   }

   // Write out settings around destruction of dialogs
   ~QColorDialogOptionsPrivate() {
      qColorDialogStaticData()->writeSettings();
   }

   QColorDialogOptions::ColorDialogOptions options;
   QString windowTitle;
};

QColorDialogOptions::QColorDialogOptions() : d(new QColorDialogOptionsPrivate)
{
}

QColorDialogOptions::QColorDialogOptions(const QColorDialogOptions &rhs) : d(rhs.d)
{
}

QColorDialogOptions &QColorDialogOptions::operator=(const QColorDialogOptions &rhs)
{
   if (this != &rhs) {
      d = rhs.d;
   }
   return *this;
}

QColorDialogOptions::~QColorDialogOptions()
{
}

QString QColorDialogOptions::windowTitle() const
{
   return d->windowTitle;
}

void QColorDialogOptions::setWindowTitle(const QString &title)
{
   d->windowTitle = title;
}

void QColorDialogOptions::setOption(QColorDialogOptions::ColorDialogOption option, bool on)
{
   if (!(d->options & option) != !on) {
      setOptions(d->options ^ option);
   }
}

bool QColorDialogOptions::testOption(QColorDialogOptions::ColorDialogOption option) const
{
   return d->options & option;
}

void QColorDialogOptions::setOptions(ColorDialogOptions options)
{
   if (options != d->options) {
      d->options = options;
   }
}

QColorDialogOptions::ColorDialogOptions QColorDialogOptions::options() const
{
   return d->options;
}

int QColorDialogOptions::customColorCount()
{
   return QColorDialogStaticData::CustomColorCount;
}

QRgb QColorDialogOptions::customColor(int index)
{
   if (uint(index) >= uint(QColorDialogStaticData::CustomColorCount)) {
      return qRgb(255, 255, 255);
   }
   return qColorDialogStaticData()->customRgb[index];
}

QRgb *QColorDialogOptions::customColors()
{
   return qColorDialogStaticData()->customRgb;
}

void QColorDialogOptions::setCustomColor(int index, QRgb color)
{
   if (uint(index) >= uint(QColorDialogStaticData::CustomColorCount)) {
      return;
   }
   qColorDialogStaticData()->customSet = true;
   qColorDialogStaticData()->customRgb[index] = color;
}

QRgb *QColorDialogOptions::standardColors()
{
   return qColorDialogStaticData()->standardRgb;
}

QRgb QColorDialogOptions::standardColor(int index)
{
   if (uint(index) >= uint(QColorDialogStaticData::StandardColorCount)) {
      return qRgb(255, 255, 255);
   }
   return qColorDialogStaticData()->standardRgb[index];
}

void QColorDialogOptions::setStandardColor(int index, QRgb color)
{
   if (uint(index) >= uint(QColorDialogStaticData::StandardColorCount)) {
      return;
   }
   qColorDialogStaticData()->standardRgb[index] = color;
}

const QSharedPointer<QColorDialogOptions> &QPlatformColorDialogHelper::options() const
{
   return m_options;
}

void QPlatformColorDialogHelper::setOptions(const QSharedPointer<QColorDialogOptions> &options)
{
   m_options = options;
}

// File dialog

class QPlatformFileDialogOptionsPrivate : public QSharedData
{
 public:
   QPlatformFileDialogOptionsPrivate()
      : options(Qt::EmptyFlag), viewMode(QPlatformFileDialogOptions::Detail),
        fileMode(QPlatformFileDialogOptions::AnyFile), acceptMode(QPlatformFileDialogOptions::AcceptOpen),
        filters(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs)
   {
   }

   QFileDialog::FileDialogOptions options;
   QString windowTitle;

   QPlatformFileDialogOptions::ViewMode viewMode;
   QPlatformFileDialogOptions::FileMode fileMode;
   QPlatformFileDialogOptions::AcceptMode acceptMode;
   QString labels[QPlatformFileDialogOptions::DialogLabelCount];
   QDir::Filters filters;
   QList<QUrl> sidebarUrls;
   QStringList nameFilters;
   QStringList mimeTypeFilters;
   QString defaultSuffix;
   QStringList history;
   QUrl initialDirectory;
   QString initiallySelectedNameFilter;
   QList<QUrl> initiallySelectedFiles;
   QStringList supportedSchemes;
};

QPlatformFileDialogOptions::QPlatformFileDialogOptions()
   : d(new QPlatformFileDialogOptionsPrivate)
{
}

QPlatformFileDialogOptions::QPlatformFileDialogOptions(const QPlatformFileDialogOptions &other)
   : d(other.d)
{
}

QPlatformFileDialogOptions &QPlatformFileDialogOptions::operator=(const QPlatformFileDialogOptions &other)
{
   if (this != &other) {
      d = other.d;
   }

   return *this;
}

QPlatformFileDialogOptions::~QPlatformFileDialogOptions()
{
}

QString QPlatformFileDialogOptions::windowTitle() const
{
   return d->windowTitle;
}

void QPlatformFileDialogOptions::setWindowTitle(const QString &title)
{
   d->windowTitle = title;
}

void QPlatformFileDialogOptions::setOption(QFileDialog::FileDialogOption option, bool on)
{
   if (! (d->options & option) != !on) {
      setOptions(d->options ^ option);
   }
}

bool QPlatformFileDialogOptions::testOption(QFileDialog::FileDialogOption option) const
{
   return d->options & option;
}

void QPlatformFileDialogOptions::setOptions(QFileDialog::FileDialogOptions options)
{
   if (options != d->options) {
      d->options = options;
   }
}

QFileDialog::FileDialogOptions QPlatformFileDialogOptions::options() const
{
   return d->options;
}

QDir::Filters QPlatformFileDialogOptions::filter() const
{
   return d->filters;
}

void QPlatformFileDialogOptions::setFilter(QDir::Filters filters)
{
   d->filters  = filters;
}

void QPlatformFileDialogOptions::setViewMode(QPlatformFileDialogOptions::ViewMode mode)
{
   d->viewMode = mode;
}

QPlatformFileDialogOptions::ViewMode QPlatformFileDialogOptions::viewMode() const
{
   return d->viewMode;
}

void QPlatformFileDialogOptions::setFileMode(QPlatformFileDialogOptions::FileMode mode)
{
   d->fileMode = mode;
}

QPlatformFileDialogOptions::FileMode QPlatformFileDialogOptions::fileMode() const
{
   return d->fileMode;
}

void QPlatformFileDialogOptions::setAcceptMode(QPlatformFileDialogOptions::AcceptMode mode)
{
   d->acceptMode = mode;
}

QPlatformFileDialogOptions::AcceptMode QPlatformFileDialogOptions::acceptMode() const
{
   return d->acceptMode;
}

void QPlatformFileDialogOptions::setSidebarUrls(const QList<QUrl> &urls)
{
   d->sidebarUrls = urls;
}

QList<QUrl> QPlatformFileDialogOptions::sidebarUrls() const
{
   return d->sidebarUrls;
}

void QPlatformFileDialogOptions::setNameFilters(const QStringList &filters)
{
   d->nameFilters = filters;
}

QStringList QPlatformFileDialogOptions::nameFilters() const
{
   return d->nameFilters;
}

void QPlatformFileDialogOptions::setMimeTypeFilters(const QStringList &filters)
{
   d->mimeTypeFilters = filters;
}

QStringList QPlatformFileDialogOptions::mimeTypeFilters() const
{
   return d->mimeTypeFilters;
}

void QPlatformFileDialogOptions::setDefaultSuffix(const QString &suffix)
{
   d->defaultSuffix = suffix;

   if (d->defaultSuffix.size() > 1 && d->defaultSuffix.startsWith('.')) {
      d->defaultSuffix.remove(0, 1);   // Silently change ".txt" -> "txt".
   }
}

QString QPlatformFileDialogOptions::defaultSuffix() const
{
   return d->defaultSuffix;
}

void QPlatformFileDialogOptions::setHistory(const QStringList &paths)
{
   d->history = paths;
}

QStringList QPlatformFileDialogOptions::history() const
{
   return d->history;
}

void QPlatformFileDialogOptions::setLabelText(QPlatformFileDialogOptions::DialogLabel label, const QString &text)
{
   if (label >= 0 && label < QPlatformFileDialogOptions::DialogLabelCount) {
      d->labels[label] = text;
   }
}

QString QPlatformFileDialogOptions::labelText(QPlatformFileDialogOptions::DialogLabel label) const
{
   return (label >= 0 && label < QPlatformFileDialogOptions::DialogLabelCount) ? d->labels[label] : QString();
}

bool QPlatformFileDialogOptions::isLabelExplicitlySet(DialogLabel label)
{
   return label >= 0 && label < QPlatformFileDialogOptions::DialogLabelCount && ! d->labels[label].isEmpty();
}

QUrl QPlatformFileDialogOptions::initialDirectory() const
{
   return d->initialDirectory;
}

void QPlatformFileDialogOptions::setInitialDirectory(const QUrl &directory)
{
   d->initialDirectory = directory;
}

QString QPlatformFileDialogOptions::initiallySelectedNameFilter() const
{
   return d->initiallySelectedNameFilter;
}

void QPlatformFileDialogOptions::setInitiallySelectedNameFilter(const QString &filter)
{
   d->initiallySelectedNameFilter = filter;
}

QList<QUrl> QPlatformFileDialogOptions::initiallySelectedFiles() const
{
   return d->initiallySelectedFiles;
}

void QPlatformFileDialogOptions::setInitiallySelectedFiles(const QList<QUrl> &files)
{
   d->initiallySelectedFiles = files;
}

// Schemes supported by the application
void QPlatformFileDialogOptions::setSupportedSchemes(const QStringList &schemes)
{
   d->supportedSchemes = schemes;
}

QStringList QPlatformFileDialogOptions::supportedSchemes() const
{
   return d->supportedSchemes;
}

// Return true if the URL is supported by the filedialog implementation *and* by the application.
bool QPlatformFileDialogHelper::isSupportedUrl(const QUrl &url) const
{
   return url.isLocalFile();
}

const QSharedPointer<QPlatformFileDialogOptions> &QPlatformFileDialogHelper::options() const
{
   return m_options;
}

void QPlatformFileDialogHelper::setOptions(const QSharedPointer<QPlatformFileDialogOptions> &options)
{
   m_options = options;
}

const QString QPlatformFileDialogHelper::filterRegExp = "^(.*)\\(([a-zA-Z0-9_.,*? +;#\\-\\[\\]@\\{\\}/!<>\\$%&=^~:\\|]*)\\)$";

// Makes a list of filters from a normal filter string "Image Files (*.png *.jpg)"
QStringList QPlatformFileDialogHelper::cleanFilterList(const QString &filter)
{
   QRegularExpression regexp(filterRegExp);
   Q_ASSERT(regexp.isValid());

   QRegularExpressionMatch match;

   QString f = filter;
   match = regexp.match(filter);

   if (match.hasMatch()) {
      f = match.capturedView(2);
   }

   return QStringParser::split(f, ' ', QStringParser::SkipEmptyParts);
}

// Message dialog
class QMessageDialogOptionsPrivate : public QSharedData
{
 public:
   QMessageDialogOptionsPrivate() :
      icon(QMessageDialogOptions::NoIcon), buttons(QPlatformDialogHelper::Ok)
   {}

   QString windowTitle;
   QMessageDialogOptions::Icon icon;
   QString text;
   QString informativeText;
   QString detailedText;
   QPlatformDialogHelper::StandardButtons buttons;
};

QMessageDialogOptions::QMessageDialogOptions() : d(new QMessageDialogOptionsPrivate)
{
}

QMessageDialogOptions::QMessageDialogOptions(const QMessageDialogOptions &rhs) : d(rhs.d)
{
}

QMessageDialogOptions &QMessageDialogOptions::operator=(const QMessageDialogOptions &rhs)
{
   if (this != &rhs) {
      d = rhs.d;
   }
   return *this;
}

QMessageDialogOptions::~QMessageDialogOptions()
{
}

QString QMessageDialogOptions::windowTitle() const
{
   return d->windowTitle;
}

void QMessageDialogOptions::setWindowTitle(const QString &title)
{
   d->windowTitle = title;
}

QMessageDialogOptions::Icon QMessageDialogOptions::icon() const
{
   return d->icon;
}

void QMessageDialogOptions::setIcon(Icon icon)
{
   d->icon = icon;
}

QString QMessageDialogOptions::text() const
{
   return d->text;
}

void QMessageDialogOptions::setText(const QString &text)
{
   d->text = text;
}

QString QMessageDialogOptions::informativeText() const
{
   return d->informativeText;
}

void QMessageDialogOptions::setInformativeText(const QString &informativeText)
{
   d->informativeText = informativeText;
}

QString QMessageDialogOptions::detailedText() const
{
   return d->detailedText;
}

void QMessageDialogOptions::setDetailedText(const QString &detailedText)
{
   d->detailedText = detailedText;
}

void QMessageDialogOptions::setStandardButtons(QPlatformDialogHelper::StandardButtons buttons)
{
   d->buttons = buttons;
}

QPlatformDialogHelper::StandardButtons QMessageDialogOptions::standardButtons() const
{
   return d->buttons;
}

QPlatformDialogHelper::ButtonRole QPlatformDialogHelper::buttonRole(QPlatformDialogHelper::StandardButton button)
{
   switch (button) {
      case Ok:
      case Save:
      case Open:
      case SaveAll:
      case Retry:
      case Ignore:
         return AcceptRole;

      case Cancel:
      case Close:
      case Abort:
         return RejectRole;

      case Discard:
         return DestructiveRole;

      case Help:
         return HelpRole;

      case Apply:
         return ApplyRole;

      case Yes:
      case YesToAll:
         return YesRole;

      case No:
      case NoToAll:
         return NoRole;

      case RestoreDefaults:
      case Reset:
         return ResetRole;

      default:
         break;
   }
   return InvalidRole;
}

const int *QPlatformDialogHelper::buttonLayout(Qt::Orientation orientation, ButtonLayout policy)
{
   if (policy == UnknownLayout) {
#if defined (Q_OS_DARWIN)
      policy = MacLayout;
#elif defined (Q_OS_LINUX) || defined (Q_OS_UNIX)
      policy = KdeLayout;
#else
      policy = WinLayout;
#endif
   }
   return buttonRoleLayouts[orientation == Qt::Vertical][policy];
}

const QSharedPointer<QMessageDialogOptions> &QPlatformMessageDialogHelper::options() const
{
   return m_options;
}

void QPlatformMessageDialogHelper::setOptions(const QSharedPointer<QMessageDialogOptions> &options)
{
   m_options = options;
}
