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

#include <qfiledialog.h>

#ifndef QT_NO_FILEDIALOG

#include <qmessagebox.h>
#include <qstylepainter.h>

#if ! defined(QT_NO_MIMETYPE)
// emerald   #include <qmimedatabase.h>
#endif

#include <qfiledialog_p.h>
#include <ui_qfiledialog.h>

#if defined(Q_OS_UNIX)
#include <pwd.h>
#include <unistd.h>        // for pathconf() on OS X

#elif defined(Q_OS_WIN)
#include <qt_windows.h>
#endif

QUrl *cs_internal_lastVisitedDir() {
   static QUrl retval;
   return &retval;
}

static constexpr const qint32 QFileDialogMagic = 0xbe;

QFileDialog::QFileDialog(QWidget *parent, Qt::WindowFlags flags)
   : QDialog(*new QFileDialogPrivate, parent, flags)
{
   Q_D(QFileDialog);
   d->init();
}

QFileDialog::QFileDialog(QWidget *parent, const QString &caption, const QString &directory, const QString &filter)
   : QDialog(*new QFileDialogPrivate, parent, Qt::EmptyFlag)
{
   Q_D(QFileDialog);
   d->init(QUrl::fromLocalFile(directory), filter, caption);
}

// internal
QFileDialog::QFileDialog(const QFileDialogArgs &args)
   : QDialog(*new QFileDialogPrivate, args.parent, Qt::EmptyFlag)
{
   Q_D(QFileDialog);

   d->init(args.directory, args.filter, args.caption);
   setFileMode(args.mode);
   setOptions(args.options);
   selectFile(args.selection);
}

QFileDialog::~QFileDialog()
{
#ifndef QT_NO_SETTINGS
   Q_D(QFileDialog);
   d->saveSettings();
#endif
}

void QFileDialog::setSidebarUrls(const QList<QUrl> &urls)
{
   Q_D(QFileDialog);

   if (! d->nativeDialogInUse) {
      d->qFileDialogUi->sidebar->setUrls(urls);
   }
}

QList<QUrl> QFileDialog::sidebarUrls() const
{
   Q_D(const QFileDialog);
   return (d->nativeDialogInUse ? QList<QUrl>() : d->qFileDialogUi->sidebar->urls());
}

QByteArray QFileDialog::saveState() const
{
   Q_D(const QFileDialog);
   int version = 4;
   QByteArray data;
   QDataStream stream(&data, QIODevice::WriteOnly);

   stream << qint32(QFileDialogMagic);
   stream << qint32(version);
   if (d->usingWidgets()) {
      stream << d->qFileDialogUi->splitter->saveState();
      stream << d->qFileDialogUi->sidebar->urls();
   } else {
      stream << d->splitterState;
      stream << d->sidebarUrls;
   }
   stream << history();
   stream << *cs_internal_lastVisitedDir();

   if (d->usingWidgets()) {
      stream << d->qFileDialogUi->treeView->header()->saveState();
   } else {
      stream << d->headerData;
   }

   stream << qint32(viewMode());

   return data;
}

bool QFileDialog::restoreState(const QByteArray &state)
{
   Q_D(QFileDialog);

   QByteArray sd = state;
   QDataStream stream(&sd, QIODevice::ReadOnly);

   if (stream.atEnd()) {
      return false;
   }

   QStringList history;
   QUrl currentDirectory;
   qint32 marker;
   qint32 v;
   qint32 viewMode;
   stream >> marker;
   stream >> v;

   if (marker != QFileDialogMagic || (v != 3 && v != 4)) {
      return false;
   }

   stream >> d->splitterState
      >> d->sidebarUrls
      >> history;

   if (v == 3) {
      QString currentDirectoryString;
      stream >> currentDirectoryString;
      currentDirectory = QUrl::fromLocalFile(currentDirectoryString);
   } else {
      stream >> currentDirectory;
   }

   stream >> d->headerData >> viewMode;

   setDirectoryUrl(cs_internal_lastVisitedDir()->isEmpty() ? currentDirectory : *cs_internal_lastVisitedDir());
   setViewMode(static_cast<QFileDialog::ViewMode>(viewMode));

   if (!d->usingWidgets()) {
      return true;
   }

   return d->restoreWidgetState(history, -1);
}

void QFileDialog::changeEvent(QEvent *e)
{
   Q_D(QFileDialog);

   if (e->type() == QEvent::LanguageChange) {
      d->retranslateWindowTitle();
      d->retranslateStrings();
   }

   QDialog::changeEvent(e);
}

void QFileDialog::setOption(FileDialogOption option, bool on)
{
   const QFileDialog::FileDialogOptions previousOptions = options();

   if (! (previousOptions & option) != !on) {
      setOptions(previousOptions ^ option);
   }
}

bool QFileDialog::testOption(FileDialogOption option) const
{
   Q_D(const QFileDialog);
   return d->options->testOption(option);
}

void QFileDialog::setOptions(FileDialogOptions options)
{
   Q_D(QFileDialog);

   FileDialogOptions changed = (options ^ QFileDialog::options());

   if (! changed) {
      return;
   }

   d->options->setOptions(options);

   if ((options & FileDialogOption::DontUseNativeDialog) && ! d->usingWidgets()) {
      d->createWidgets();
   }

   if (d->usingWidgets()) {
      if (changed & FileDialogOption::DontResolveSymlinks) {
         d->model->setResolveSymlinks(! (options & FileDialogOption::DontResolveSymlinks));
      }

      if (changed & FileDialogOption::ReadOnly) {
         bool ro = (options & FileDialogOption::ReadOnly);
         d->model->setReadOnly(ro);
         d->qFileDialogUi->newFolderButton->setEnabled(!ro);
         d->renameAction->setEnabled(!ro);
         d->deleteAction->setEnabled(!ro);
      }

      if (changed & FileDialogOption::DontUseCustomDirectoryIcons) {
         QFileIconProvider::Options providerOptions = iconProvider()->options();

         if (options & FileDialogOption::DontUseCustomDirectoryIcons) {
            providerOptions |= QFileIconProvider::DontUseCustomDirectoryIcons;
         } else {
            providerOptions &= ~QFileIconProvider::DontUseCustomDirectoryIcons;
         }

         iconProvider()->setOptions(providerOptions);
      }
   }

   if (changed & FileDialogOption::HideNameFilterDetails) {
      setNameFilters(d->options->nameFilters());
   }

   if (changed & FileDialogOption::ShowDirsOnly) {
      setFilter((options & FileDialogOption::ShowDirsOnly) ? (filter() & ~QDir::Files) : (filter() | QDir::Files));
   }
}

QFileDialog::FileDialogOptions QFileDialog::options() const
{
   Q_D(const QFileDialog);
   return QFileDialog::FileDialogOptions(int(d->options->options()));
}

void QFileDialog::open(QObject *receiver, const QString &slot)
{
   Q_D(QFileDialog);

   QString signal;

   if (fileMode() == ExistingFiles) {
      signal = "filesSelected(QStringList)";

   } else  {
      signal = "fileSelected(QString)";

   }

   connect(this, signal, receiver, slot);

   d->signalToDisconnectOnClose   = signal;
   d->receiverToDisconnectOnClose = receiver;
   d->memberToDisconnectOnClose   = slot;

   QDialog::open();
}

void QFileDialog::setVisible(bool visible)
{
   Q_D(QFileDialog);

   if (visible) {
      if (testAttribute(Qt::WA_WState_ExplicitShowHide) && !testAttribute(Qt::WA_WState_Hidden)) {
         return;
      }

   } else if (testAttribute(Qt::WA_WState_ExplicitShowHide) && testAttribute(Qt::WA_WState_Hidden)) {
      return;
   }

   if (d->canBeNativeDialog()) {
      if (d->setNativeDialogVisible(visible)) {
         // Set WA_DontShowOnScreen so that QDialog::setVisible(visible) below
         // updates the state correctly, but skips showing the non-native version:
         setAttribute(Qt::WA_DontShowOnScreen);

#ifndef QT_NO_FSCOMPLETER
         //So the completer don't try to complete and therefore to show a popup
         if (!d->nativeDialogInUse) {
            d->completer->setModel(nullptr);
         }
#endif

      } else {
         d->createWidgets();
         setAttribute(Qt::WA_DontShowOnScreen, false);

#ifndef QT_NO_FSCOMPLETER
         if (!d->nativeDialogInUse) {
            if (d->proxyModel != nullptr) {
               d->completer->setModel(d->proxyModel);

            } else {
               d->completer->setModel(d->model);
            }
         }
#endif
      }
   }

   if (d->usingWidgets()) {
      d->qFileDialogUi->fileNameEdit->setFocus();
   }

   QDialog::setVisible(visible);
}

void QFileDialog::setDirectory(const QString &directory)
{
   Q_D(QFileDialog);
   QString newDirectory = directory;

   //we remove .. and . from the given path if exist
   if (!directory.isEmpty()) {
      newDirectory = QDir::cleanPath(directory);
   }

   if (!directory.isEmpty() && newDirectory.isEmpty()) {
      return;
   }

   QUrl newDirUrl = QUrl::fromLocalFile(newDirectory);
   QFileDialogPrivate::setLastVisitedDirectory(newDirUrl);

   d->options->setInitialDirectory(QUrl::fromLocalFile(directory));

   if (!d->usingWidgets()) {
      d->setDirectory_sys(newDirUrl);
      return;
   }

   if (d->rootPath() == newDirectory) {
      return;
   }

   QModelIndex root = d->model->setRootPath(newDirectory);

   if (! d->nativeDialogInUse) {
      d->qFileDialogUi->newFolderButton->setEnabled(d->model->flags(root) & Qt::ItemIsDropEnabled);
      if (root != d->rootIndex()) {

#ifndef QT_NO_FSCOMPLETER
         if (directory.endsWith('/')) {
            d->completer->setCompletionPrefix(newDirectory);
         } else {
            d->completer->setCompletionPrefix(newDirectory + '/');
         }
#endif

         d->setRootIndex(root);
      }

      d->qFileDialogUi->listView->selectionModel()->clear();
   }
}

QDir QFileDialog::directory() const
{
   Q_D(const QFileDialog);

   if (d->nativeDialogInUse) {
      QString dir = d->directory_sys().toLocalFile();
      return QDir(dir.isEmpty() ? d->options->initialDirectory().toLocalFile() : dir);
   }
   return d->rootPath();
}

void QFileDialog::setDirectoryUrl(const QUrl &directory)
{
   Q_D(QFileDialog);

   if (! directory.isValid()) {
      return;
   }

   QFileDialogPrivate::setLastVisitedDirectory(directory);
   d->options->setInitialDirectory(directory);

   if (d->nativeDialogInUse) {
      d->setDirectory_sys(directory);

   } else if (directory.isLocalFile()) {
      setDirectory(directory.toLocalFile());

   } else if (d->usingWidgets()) {
      qWarning("QFileDialog::setDirectoryUrl() Internal file dialog does not support "
            "a URL starting with %s", csPrintable(directory.scheme()));
   }
}

QUrl QFileDialog::directoryUrl() const
{
   Q_D(const QFileDialog);

   if (d->nativeDialogInUse) {
      return d->directory_sys();

   } else {
      return QUrl::fromLocalFile(directory().absolutePath());
   }
}
static inline bool isCaseSensitiveFileSystem(const QString &path)
{
   (void) path;

#if defined(Q_OS_WIN)
   return false;
#elif defined(Q_OS_DARWIN)
   return pathconf(QFile::encodeName(path).constData(), _PC_CASE_SENSITIVE);
#else
   return true;
#endif

}

static inline QString fileFromPath(const QString &rootPath, QString path)
{
   if (!QFileInfo(path).isAbsolute()) {
      return path;
   }

   if (path.startsWith(rootPath, isCaseSensitiveFileSystem(rootPath) ? Qt::CaseSensitive : Qt::CaseInsensitive)) {
      path.remove(0, rootPath.size());
   }

   if (path.isEmpty()) {
      return path;
   }

#ifdef Q_OS_WIN
   if (path.at(0) == QDir::separator() || path.at(0) == '/') {

#else
   if (path.at(0) == QDir::separator()) {

#endif

      path.remove(0, 1);
   }

   return path;
}

void QFileDialog::selectFile(const QString &filename)
{
   Q_D(QFileDialog);

   if (filename.isEmpty()) {
      return;
   }

   if (!d->usingWidgets()) {
      QUrl url = QUrl::fromLocalFile(filename);
      if (QFileInfo(filename).isRelative()) {
         QDir dir(d->options->initialDirectory().toLocalFile());
         url = QUrl::fromLocalFile(dir.absoluteFilePath(filename));
      }
      d->selectFile_sys(url);
      d->options->setInitiallySelectedFiles(QList<QUrl>() << url);
      return;
   }

   if (!QDir::isRelativePath(filename)) {
      QFileInfo info(filename);
      QString filenamePath = info.absoluteDir().path();

      if (d->model->rootPath() != filenamePath) {
         setDirectory(filenamePath);
      }
   }

   QModelIndex index = d->model->index(filename);
   d->qFileDialogUi->listView->selectionModel()->clear();

   if (! isVisible() || ! d->lineEdit()->hasFocus()) {
      d->lineEdit()->setText(index.isValid() ? index.data().toString() : fileFromPath(d->rootPath(), filename));
   }
}

void QFileDialog::selectUrl(const QUrl &url)
{
   Q_D(QFileDialog);

   if (! url.isValid()) {
      return;
   }

   if (d->nativeDialogInUse) {
      d->selectFile_sys(url);

   } else if (url.isLocalFile()) {
      selectFile(url.toLocalFile());

   } else {
      qWarning("QFileDialog::selectUrl() Internal file dialog does not support "
            "a URL starting with %s", csPrintable(url.scheme()));
   }
}

#ifdef Q_OS_UNIX
QString qt_tildeExpansion(const QString &path, bool *expanded = nullptr)
{
   if (expanded != nullptr) {
      *expanded = false;
   }

   if (! path.startsWith('~')) {
      return path;
   }

   QString ret = path;

   QStringList tokens = ret.split(QDir::separator());

   if (tokens.first() == "~") {
      ret.replace(0, 1, QDir::homePath());

   } else {
      QString userName = tokens.first();
      userName.remove(0, 1);

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && !defined(Q_OS_OPENBSD)
      passwd pw;
      passwd *tmpPw;
      char buf[200];
      const int bufSize = sizeof(buf);

      int err = getpwnam_r(userName.toUtf8().constData(), &pw, buf, bufSize, &tmpPw);
      if (err || !tmpPw) {
         return ret;
      }

      const QString homePath = QString::fromUtf8(pw.pw_dir);
#else
      passwd *pw = getpwnam(userName.toUtf8().constData());

      if (! pw) {
         return ret;
      }

      const QString homePath = QString::fromUtf8(pw->pw_dir);
#endif
      ret.replace(0, tokens.first().length(), homePath);
   }

   if (expanded != nullptr) {
      *expanded = true;
   }

   return ret;
}
#endif

QStringList QFileDialog::selectedFiles() const
{
   Q_D(const QFileDialog);

   QStringList files;
   const QList<QUrl> userSelectedFiles = d->userSelectedFiles();

   for (const QUrl &file : userSelectedFiles) {
      files.append(file.toLocalFile());
   }
   if (files.isEmpty() && d->usingWidgets()) {
      const FileMode fm = fileMode();
      if (fm != ExistingFile && fm != ExistingFiles) {
         files.append(d->rootIndex().data(QFileSystemModel::FilePathRole).toString());
      }
   }
   return files;
}

QList<QUrl> QFileDialog::selectedUrls() const
{
   Q_D(const QFileDialog);

   if (d->nativeDialogInUse) {
      return d->userSelectedFiles();

   } else {
      QList<QUrl> urls;
      const QStringList selectedFileList = selectedFiles();

      for (const QString &file : selectedFileList) {
         urls.append(QUrl::fromLocalFile(file));
      }

      return urls;
   }
}

QStringList qt_make_filter_list(const QString &filter)
{
   QString f(filter);

   if (f.isEmpty()) {
      return QStringList();
   }

   QString sep(";;");
   int i = f.indexOf(sep, 0);

   if (i == -1) {
      if (f.indexOf('\n', 0) != -1) {
         sep = '\n';
         i = f.indexOf(sep, 0);
      }
   }

   return f.split(sep);
}

void QFileDialog::setNameFilter(const QString &filter)
{
   setNameFilters(qt_make_filter_list(filter));
}

void QFileDialog::setNameFilterDetailsVisible(bool enabled)
{
   setOption(FileDialogOption::HideNameFilterDetails, ! enabled);
}

bool QFileDialog::isNameFilterDetailsVisible() const
{
   return ! testOption(FileDialogOption::HideNameFilterDetails);
}

QStringList qt_strip_filters(const QStringList &filters)
{
   QStringList strippedFilters;

   static QRegularExpression regExp(QPlatformFileDialogHelper::filterRegExp);
   QRegularExpressionMatch match;

   for (int i = 0; i < filters.count(); ++i) {
      QString filterName;
      match = regExp.match(filters[i]);

      if (match.hasMatch()) {
         filterName = match.captured(1);
      }

      strippedFilters.append(filterName.simplified());
   }

   return strippedFilters;
}

void QFileDialog::setNameFilters(const QStringList &filters)
{
   Q_D(QFileDialog);

   d->defaultFileTypes = (filters == QStringList(QFileDialog::tr("All Files (*)")));
   QStringList cleanedFilters;

   for (int i = 0; i < filters.count(); ++i) {
      cleanedFilters << filters[i].simplified();
   }

   d->options->setNameFilters(cleanedFilters);

   if (! d->usingWidgets()) {
      return;
   }

   d->qFileDialogUi->fileTypeCombo->clear();
   if (cleanedFilters.isEmpty()) {
      return;
   }

   if (testOption(FileDialogOption::HideNameFilterDetails)) {
      d->qFileDialogUi->fileTypeCombo->addItems(qt_strip_filters(cleanedFilters));
   } else {
      d->qFileDialogUi->fileTypeCombo->addItems(cleanedFilters);
   }

   d->_q_useNameFilter(0);
}

QStringList QFileDialog::nameFilters() const
{
   return d_func()->options->nameFilters();
}

void QFileDialog::selectNameFilter(const QString &filter)
{
   Q_D(QFileDialog);

   d->options->setInitiallySelectedNameFilter(filter);

   if (!d->usingWidgets()) {
      d->selectNameFilter_sys(filter);
      return;
   }

   int i = -1;
   if (testOption(FileDialogOption::HideNameFilterDetails)) {
      const QStringList filters = qt_strip_filters(qt_make_filter_list(filter));

      if (! filters.isEmpty()) {
         i = d->qFileDialogUi->fileTypeCombo->findText(filters.first());
      }

   } else {
      i = d->qFileDialogUi->fileTypeCombo->findText(filter);
   }

   if (i >= 0) {
      d->qFileDialogUi->fileTypeCombo->setCurrentIndex(i);
      d->_q_useNameFilter(d->qFileDialogUi->fileTypeCombo->currentIndex());
   }
}

QString QFileDialog::selectedNameFilter() const
{
   Q_D(const QFileDialog);
   if (!d->usingWidgets()) {
      return d->selectedNameFilter_sys();
   }

   return d->qFileDialogUi->fileTypeCombo->currentText();
}

QDir::Filters QFileDialog::filter() const
{
   Q_D(const QFileDialog);
   if (d->usingWidgets()) {
      return d->model->filter();
   }

   return d->options->filter();
}

void QFileDialog::setFilter(QDir::Filters filters)
{
   Q_D(QFileDialog);

   d->options->setFilter(filters);

   if (! d->usingWidgets()) {
      d->setFilter_sys();
      return;
   }

   d->model->setFilter(filters);
   d->showHiddenAction->setChecked((filters & QDir::Hidden));
}

#if ! defined(QT_NO_MIMETYPE)
static QString nameFilterForMime(const QString &mimeType)
{
   (void) mimeType;

   /* emerald (mime types)

       QMimeDatabase db;
       QMimeType mime(db.mimeTypeForName(mimeType));

       if (mime.isValid()) {
           if (mime.isDefault()) {
               return QFileDialog::tr("All files (*)");

           } else {
               const QString patterns = mime.globPatterns().join(' ');
               return mime.comment() + " (" + patterns + ')';
           }
       }

   */

   return QString();
}

void QFileDialog::setMimeTypeFilters(const QStringList &filters)
{
   Q_D(QFileDialog);

   QStringList nameFilters;

   for (const QString &mimeType : filters) {
      const QString text = nameFilterForMime(mimeType);

      if (!text.isEmpty()) {
         nameFilters.append(text);
      }
   }

   setNameFilters(nameFilters);
   d->options->setMimeTypeFilters(filters);
}

QStringList QFileDialog::mimeTypeFilters() const
{
   return d_func()->options->mimeTypeFilters();
}

void QFileDialog::selectMimeTypeFilter(const QString &filter)
{
   const QString text = nameFilterForMime(filter);

   if (!text.isEmpty()) {
      selectNameFilter(text);
   }
}
#endif // QT_NO_MIMETYPE

void QFileDialog::setViewMode(QFileDialog::ViewMode mode)
{
   Q_D(QFileDialog);

   d->options->setViewMode(static_cast<QPlatformFileDialogOptions::ViewMode>(mode));
   if (! d->usingWidgets()) {
      return;
   }

   if (mode == Detail) {
      d->_q_showDetailsView();
   } else {
      d->_q_showListView();
   }
}

QFileDialog::ViewMode QFileDialog::viewMode() const
{
   Q_D(const QFileDialog);
   if (!d->usingWidgets()) {
      return static_cast<QFileDialog::ViewMode>(d->options->viewMode());
   }

   return (d->qFileDialogUi->stackedWidget->currentWidget() == d->qFileDialogUi->listView->parent() ? QFileDialog::List :
         QFileDialog::Detail);
}

void QFileDialog::setFileMode(QFileDialog::FileMode mode)
{
   Q_D(QFileDialog);
   d->options->setFileMode(static_cast<QPlatformFileDialogOptions::FileMode>(mode));


   // keep ShowDirsOnly option in sync with fileMode (BTW, DirectoryOnly is obsolete)
   setOption(FileDialogOption::ShowDirsOnly, mode == FileMode::DirectoryOnly);

   if (! d->usingWidgets()) {
      return;
   }

   d->retranslateWindowTitle();

   // set selection mode and behavior
   QAbstractItemView::SelectionMode selectionMode;

   if (mode == QFileDialog::ExistingFiles) {
      selectionMode = QAbstractItemView::ExtendedSelection;
   } else {
      selectionMode = QAbstractItemView::SingleSelection;
   }

   d->qFileDialogUi->listView->setSelectionMode(selectionMode);
   d->qFileDialogUi->treeView->setSelectionMode(selectionMode);
   // set filter
   d->model->setFilter(d->filterForMode(filter()));

   // setup file type for directory
   if (mode == DirectoryOnly || mode == Directory) {
      d->qFileDialogUi->fileTypeCombo->clear();
      d->qFileDialogUi->fileTypeCombo->addItem(tr("Directories"));
      d->qFileDialogUi->fileTypeCombo->setEnabled(false);
   }

   d->updateFileNameLabel();
   d->updateOkButtonText();

   d->qFileDialogUi->fileTypeCombo->setEnabled(!testOption(FileDialogOption::ShowDirsOnly));
   d->_q_updateOkButton();
}

QFileDialog::FileMode QFileDialog::fileMode() const
{
   Q_D(const QFileDialog);
   return static_cast<FileMode>(d->options->fileMode());
}

void QFileDialog::setAcceptMode(QFileDialog::AcceptMode mode)
{
   Q_D(QFileDialog);
   d->options->setAcceptMode(static_cast<QPlatformFileDialogOptions::AcceptMode>(mode));

   // clear WA_DontShowOnScreen so that d->canBeNativeDialog() doesn't return false incorrectly
   setAttribute(Qt::WA_DontShowOnScreen, false);
   if (! d->usingWidgets()) {
      return;
   }

   QDialogButtonBox::StandardButton button = (mode == AcceptOpen ? QDialogButtonBox::Open : QDialogButtonBox::Save);
   d->qFileDialogUi->buttonBox->setStandardButtons(button | QDialogButtonBox::Cancel);
   d->qFileDialogUi->buttonBox->button(button)->setEnabled(false);
   d->_q_updateOkButton();

   if (mode == AcceptSave) {
      d->qFileDialogUi->lookInCombo->setEditable(false);
   }

   d->retranslateWindowTitle();
}

void QFileDialog::setSupportedSchemes(const QStringList &schemes)
{
   Q_D(QFileDialog);
   d->options->setSupportedSchemes(schemes);
}

QStringList QFileDialog::supportedSchemes() const
{
   return d_func()->options->supportedSchemes();
}

QFileDialog::AcceptMode QFileDialog::acceptMode() const
{
   Q_D(const QFileDialog);
   return static_cast<AcceptMode>(d->options->acceptMode());
}

void QFileDialog::setReadOnly(bool enabled)
{
   setOption(FileDialogOption::ReadOnly, enabled);
}

bool QFileDialog::isReadOnly() const
{
   return testOption(FileDialogOption::ReadOnly);
}

void QFileDialog::setResolveSymlinks(bool enabled)
{
   setOption(FileDialogOption::DontResolveSymlinks, ! enabled);
}

bool QFileDialog::resolveSymlinks() const
{
   return !testOption(FileDialogOption::DontResolveSymlinks);
}

void QFileDialog::setConfirmOverwrite(bool enabled)
{
   setOption(FileDialogOption::DontConfirmOverwrite, !enabled);
}

bool QFileDialog::confirmOverwrite() const
{
   return !testOption(FileDialogOption::DontConfirmOverwrite);
}

void QFileDialog::setDefaultSuffix(const QString &suffix)
{
   Q_D(QFileDialog);
   d->options->setDefaultSuffix(suffix);
}

QString QFileDialog::defaultSuffix() const
{
   Q_D(const QFileDialog);
   return d->options->defaultSuffix();
}

void QFileDialog::setHistory(const QStringList &paths)
{
   Q_D(QFileDialog);

   if (d->usingWidgets()) {
      d->qFileDialogUi->lookInCombo->setHistory(paths);
   }
}

void QFileDialogComboBox::setHistory(const QStringList &paths)
{
   m_history = paths;
   // Only populate the first item, showPopup will populate the rest if needed
   QList<QUrl> list;

   QModelIndex idx = d_ptr->model->index(d_ptr->rootPath());

   //On windows the popup display the "C:\", convert to nativeSeparators
   QUrl url = QUrl::fromLocalFile(QDir::toNativeSeparators(idx.data(QFileSystemModel::FilePathRole).toString()));
   if (url.isValid()) {
      list.append(url);
   }
   urlModel->setUrls(list);
}

QStringList QFileDialog::history() const
{
   Q_D(const QFileDialog);

   if (!d->usingWidgets()) {
      return QStringList();
   }

   QStringList currentHistory = d->qFileDialogUi->lookInCombo->history();

   //On windows the popup display the "C:\", convert to nativeSeparators
   QString newHistory = QDir::toNativeSeparators(d->rootIndex().data(QFileSystemModel::FilePathRole).toString());
   if (!currentHistory.contains(newHistory)) {
      currentHistory << newHistory;
   }
   return currentHistory;
}


void QFileDialog::setItemDelegate(QAbstractItemDelegate *delegate)
{
   Q_D(QFileDialog);
   if (!d->usingWidgets()) {
      return;
   }
   d->qFileDialogUi->listView->setItemDelegate(delegate);
   d->qFileDialogUi->treeView->setItemDelegate(delegate);
}

/*!
  Returns the item delegate used to render the items in the views in the filedialog.
*/
QAbstractItemDelegate *QFileDialog::itemDelegate() const
{
   Q_D(const QFileDialog);
   if (!d->usingWidgets()) {
      return nullptr;
   }
   return d->qFileDialogUi->listView->itemDelegate();
}

/*!
    Sets the icon provider used by the filedialog to the specified \a provider.
*/
void QFileDialog::setIconProvider(QFileIconProvider *provider)
{
   Q_D(QFileDialog);
   if (!d->usingWidgets()) {
      return;
   }
   d->model->setIconProvider(provider);
   //It forces the refresh of all entries in the side bar, then we can get new icons
   d->qFileDialogUi->sidebar->setUrls(d->qFileDialogUi->sidebar->urls());
}

/*!
    Returns the icon provider used by the filedialog.
*/
QFileIconProvider *QFileDialog::iconProvider() const
{
   Q_D(const QFileDialog);
   if (!d->model) {
      return nullptr;
   }

   return d->model->iconProvider();
}

void QFileDialog::setLabelText(DialogLabel label, const QString &text)
{
    Q_D(QFileDialog);
    d->options->setLabelText(static_cast<QPlatformFileDialogOptions::DialogLabel>(label), text);
    d->setLabelTextControl(label, text);
}

QString QFileDialog::labelText(DialogLabel label) const
{
   Q_D(const QFileDialog);

   if (! d->usingWidgets()) {
      return d->options->labelText(static_cast<QPlatformFileDialogOptions::DialogLabel>(label));
   }

   QPushButton *button;

   switch (label) {
      case LookIn:
         return d->qFileDialogUi->lookInLabel->text();

      case FileName:
         return d->qFileDialogUi->fileNameLabel->text();

      case FileType:
         return d->qFileDialogUi->fileTypeLabel->text();

      case Accept:
         if (acceptMode() == AcceptOpen) {
            button = d->qFileDialogUi->buttonBox->button(QDialogButtonBox::Open);
         } else {
            button = d->qFileDialogUi->buttonBox->button(QDialogButtonBox::Save);
         }

         if (button) {
            return button->text();
         }

         break;

      case Reject:
         button = d->qFileDialogUi->buttonBox->button(QDialogButtonBox::Cancel);
         if (button) {
            return button->text();
         }

         break;

      case DialogLabelCount:
         // do nothing
         break;
   }

   return QString();
}

QString QFileDialog::getOpenFileName(QWidget *parent, const QString &caption, const QString &dir,
            const QString &filter, QString *selectedFilter, FileDialogOptions options)
{
   const QStringList schemes = QStringList("file");
   const QUrl selectedUrl = getOpenFileUrl(parent, caption, QUrl::fromLocalFile(dir), filter, selectedFilter,
            options, schemes);

   return selectedUrl.toLocalFile();
}

QUrl QFileDialog::getOpenFileUrl(QWidget *parent, const QString &caption, const QUrl &dir,
      const QString &filter, QString *selectedFilter, FileDialogOptions options, const
      QStringList &supportedSchemes)
{
   QFileDialogArgs args;
   args.parent    = parent;
   args.caption   = caption;
   args.directory = QFileDialogPrivate::workingDirectory(dir);
   args.selection = QFileDialogPrivate::initialSelection(dir);
   args.filter    = filter;
   args.mode      = ExistingFile;
   args.options   = options;

   // create a dialog
   QFileDialog dialog(args);
   dialog.setSupportedSchemes(supportedSchemes);

   if (selectedFilter && ! selectedFilter->isEmpty())  {
      dialog.selectNameFilter(*selectedFilter);
   }

   if (dialog.exec() == QDialog::Accepted) {
      if (selectedFilter) {
         *selectedFilter = dialog.selectedNameFilter();
      }

      return dialog.selectedUrls().value(0);
   }

   return QUrl();
}

QStringList QFileDialog::getOpenFileNames(QWidget *parent, const QString &caption, const QString &dir,
            const QString &filter, QString *selectedFilter, FileDialogOptions options)
{
   const QStringList schemes = QStringList("file");
   const QList<QUrl> selectedUrls = getOpenFileUrls(parent, caption, QUrl::fromLocalFile(dir), filter, selectedFilter, options, schemes);

   QStringList fileNames;

   for (const QUrl &url : selectedUrls) {
      fileNames << url.toLocalFile();
   }

   return fileNames;
}

QList<QUrl> QFileDialog::getOpenFileUrls(QWidget *parent, const QString &caption, const QUrl &dir,
            const QString &filter, QString *selectedFilter, FileDialogOptions options, const QStringList &supportedSchemes)
{
   QFileDialogArgs args;
   args.parent    = parent;
   args.caption   = caption;
   args.directory = QFileDialogPrivate::workingDirectory(dir);
   args.selection = QFileDialogPrivate::initialSelection(dir);
   args.filter    = filter;
   args.mode      = ExistingFiles;
   args.options   = options;

   // create a dialog
   QFileDialog dialog(args);
   dialog.setSupportedSchemes(supportedSchemes);

   if (selectedFilter && !selectedFilter->isEmpty()) {
      dialog.selectNameFilter(*selectedFilter);
   }

   if (dialog.exec() == QDialog::Accepted) {
      if (selectedFilter) {
         *selectedFilter = dialog.selectedNameFilter();
      }

      return dialog.selectedUrls();
   }

   return QList<QUrl>();
}

QString QFileDialog::getSaveFileName(QWidget *parent, const QString &caption, const QString &dir,
            const QString &filter, QString *selectedFilter, FileDialogOptions options)
{
   const QStringList schemes = QStringList("file");
   const QUrl selectedUrl = getSaveFileUrl(parent, caption, QUrl::fromLocalFile(dir), filter, selectedFilter, options, schemes);
   return selectedUrl.toLocalFile();
}

QUrl QFileDialog::getSaveFileUrl(QWidget *parent, const QString &caption, const QUrl &dir,
            const QString &filter, QString *selectedFilter, FileDialogOptions options, const QStringList &supportedSchemes)
{
   QFileDialogArgs args;
   args.parent    = parent;
   args.caption   = caption;
   args.directory = QFileDialogPrivate::workingDirectory(dir);
   args.selection = QFileDialogPrivate::initialSelection(dir);
   args.filter    = filter;
   args.mode      = AnyFile;
   args.options   = options;

   // create a qt dialog
   QFileDialog dialog(args);
   dialog.setSupportedSchemes(supportedSchemes);

   dialog.setAcceptMode(AcceptSave);
   if (selectedFilter && !selectedFilter->isEmpty()) {
      dialog.selectNameFilter(*selectedFilter);
   }

   if (dialog.exec() == QDialog::Accepted) {
      if (selectedFilter) {
         *selectedFilter = dialog.selectedNameFilter();
      }
      return dialog.selectedUrls().value(0);
   }

   return QUrl();
}

QString QFileDialog::getExistingDirectory(QWidget *parent, const QString &caption, const QString &dir, FileDialogOptions options)
{
   const QStringList schemes = QStringList("file");
   const QUrl selectedUrl    = getExistingDirectoryUrl(parent, caption, QUrl::fromLocalFile(dir), options, schemes);
   return selectedUrl.toLocalFile();
}

QUrl QFileDialog::getExistingDirectoryUrl(QWidget *parent,  const QString &caption, const QUrl &dir,
            FileDialogOptions options, const QStringList &supportedSchemes)
{
   QFileDialogArgs args;
   args.parent     = parent;
   args.caption    = caption;
   args.directory  = QFileDialogPrivate::workingDirectory(dir);
   args.mode       = (options & FileDialogOption::ShowDirsOnly ? FileMode::DirectoryOnly : FileMode::Directory);
   args.options    = options;

   QFileDialog dialog(args);
   dialog.setSupportedSchemes(supportedSchemes);

   if (dialog.exec() == QDialog::Accepted) {
      return dialog.selectedUrls().value(0);
   }

   return QUrl();
}

void QFileDialog::done(int result)
{
   Q_D(QFileDialog);

   QDialog::done(result);

   if (d->receiverToDisconnectOnClose) {
      disconnect(this, d->signalToDisconnectOnClose, d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);
      d->receiverToDisconnectOnClose = nullptr;
   }

   d->memberToDisconnectOnClose.clear();
   d->signalToDisconnectOnClose.clear();
}

void QFileDialog::accept()
{
   Q_D(QFileDialog);

   if (!d->usingWidgets()) {
      const QList<QUrl> urls = selectedUrls();
      if (urls.isEmpty()) {
         return;
      }

      d->_q_emitUrlsSelected(urls);
      if (urls.count() == 1) {
         d->_q_emitUrlSelected(urls.first());
      }

      QDialog::accept();
      return;
   }

   QStringList files = selectedFiles();
   if (files.isEmpty()) {
      return;
   }

   QString lineEditText = d->lineEdit()->text();
   // "hidden feature" type .. and then enter, and it will move up a dir
   // special case for ".."

   if (lineEditText == "..") {
      d->_q_navigateToParent();

      bool wasBlocked = d->qFileDialogUi->fileNameEdit->blockSignals(true);
      d->lineEdit()->selectAll();

      d->qFileDialogUi->fileNameEdit->blockSignals(wasBlocked);
      return;
   }

   switch (fileMode()) {
      case DirectoryOnly:
      case Directory: {
         QString fn = files.first();
         QFileInfo info(fn);
         if (!info.exists()) {
            info = QFileInfo(d->getEnvironmentVariable(fn));
         }

         if (!info.exists()) {
#ifndef QT_NO_MESSAGEBOX
            QString message = tr("%1\nDirectory not found");
            QMessageBox::warning(this, windowTitle(), message.formatArg(info.fileName()));
#endif
            return;
         }

         if (info.isDir()) {
            d->emitFilesSelected(files);
            QDialog::accept();
         }

         return;
      }

      case AnyFile: {
         QString fn = files.first();
         QFileInfo info(fn);
         if (info.isDir()) {
            setDirectory(info.absoluteFilePath());
            return;
         }

         if (!info.exists()) {
            int maxNameLength = d->maxNameLength(info.path());
            if (maxNameLength >= 0 && info.fileName().length() > maxNameLength) {
               return;
            }
         }

         // check if we have to ask for permission to overwrite the file
         if (!info.exists() || !confirmOverwrite() || acceptMode() == AcceptOpen) {
            d->emitFilesSelected(QStringList(fn));
            QDialog::accept();

#ifndef QT_NO_MESSAGEBOX
         } else {

            auto result = QMessageBox::warning(this, windowTitle(),
                  tr("%1 already exists.\nDo you want to replace it?").formatArg(info.fileName()),
                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

            if (result == QMessageBox::Yes) {
               d->emitFilesSelected(QStringList(fn));
               QDialog::accept();
            }
#endif
         }
         return;
      }

      case ExistingFile:
      case ExistingFiles:
         for (int i = 0; i < files.count(); ++i) {
            QFileInfo info(files.at(i));
            if (! info.exists()) {
               info = QFileInfo(d->getEnvironmentVariable(files.at(i)));
            }

            if (!info.exists()) {
#ifndef QT_NO_MESSAGEBOX
               QString message = tr("%1\nFile not found.");
               QMessageBox::warning(this, windowTitle(), message.formatArg(info.fileName()));
#endif
               return;
            }

            if (info.isDir()) {
               setDirectory(info.absoluteFilePath());
               d->lineEdit()->clear();
               return;
            }
         }
         d->emitFilesSelected(files);
         QDialog::accept();
         return;
   }
}

#ifndef QT_NO_PROXYMODEL
void QFileDialog::setProxyModel(QAbstractProxyModel *proxyModel)
{
   Q_D(QFileDialog);

   if (! d->usingWidgets()) {
      return;
   }

   if ((! proxyModel && ! d->proxyModel) || (proxyModel == d->proxyModel)) {
      return;
   }

   QModelIndex idx = d->rootIndex();

   if (d->proxyModel) {
      disconnect(d->proxyModel, &QAbstractProxyModel::rowsInserted, this, &QFileDialog::_q_rowsInserted);

   } else {
      disconnect(d->model, &QAbstractProxyModel::rowsInserted, this, &QFileDialog::_q_rowsInserted);

   }

   if (proxyModel != nullptr) {
      proxyModel->setParent(this);
      d->proxyModel = proxyModel;
      proxyModel->setSourceModel(d->model);
      d->qFileDialogUi->listView->setModel(d->proxyModel);
      d->qFileDialogUi->treeView->setModel(d->proxyModel);

#ifndef QT_NO_FSCOMPLETER
      d->completer->setModel(d->proxyModel);
      d->completer->proxyModel = d->proxyModel;
#endif

      connect(d->proxyModel, &QAbstractProxyModel::rowsInserted, this, &QFileDialog::_q_rowsInserted);

   } else {
      d->proxyModel = nullptr;
      d->qFileDialogUi->listView->setModel(d->model);
      d->qFileDialogUi->treeView->setModel(d->model);

#ifndef QT_NO_FSCOMPLETER
      d->completer->setModel(d->model);
      d->completer->sourceModel = d->model;
      d->completer->proxyModel  = nullptr;
#endif

      connect(d->model, &QAbstractProxyModel::rowsInserted, this, &QFileDialog::_q_rowsInserted);
   }

   QScopedPointer<QItemSelectionModel> selModel(d->qFileDialogUi->treeView->selectionModel());
   d->qFileDialogUi->treeView->setSelectionModel(d->qFileDialogUi->listView->selectionModel());

   d->setRootIndex(idx);

   // reconnect selection
   QItemSelectionModel *selections = d->qFileDialogUi->listView->selectionModel();

   QObject::connect(selections, &QItemSelectionModel::selectionChanged, this, &QFileDialog::_q_selectionChanged);
   QObject::connect(selections, &QItemSelectionModel::currentChanged,   this, &QFileDialog::_q_currentChanged);
}

QAbstractProxyModel *QFileDialog::proxyModel() const
{
   Q_D(const QFileDialog);
   return d->proxyModel;
}

#endif // QT_NO_PROXYMODEL

bool QFileDialogPrivate::itemViewKeyboardEvent(QKeyEvent *event)
{
   Q_Q(QFileDialog);
   if (event->matches(QKeySequence::Cancel)) {
      q->reject();
      return true;
   }
   switch (event->key()) {
      case Qt::Key_Backspace:
         _q_navigateToParent();
         return true;

      case Qt::Key_Back:
#ifdef QT_KEYPAD_NAVIGATION
         if (QApplication::keypadNavigationEnabled()) {
            return false;
         }
#endif

      case Qt::Key_Left:
         if (event->key() == Qt::Key_Back || event->modifiers() == Qt::AltModifier) {
            _q_navigateBackward();
            return true;
         }
         break;

      default:
         break;
   }
   return false;
}

QString QFileDialogPrivate::getEnvironmentVariable(const QString &string)
{
#ifdef Q_OS_UNIX
   if (string.size() > 1 && string.startsWith('$')) {
      return QString::fromUtf8(getenv(string.mid(1).toLatin1().constData()));
   }
#else
   if (string.size() > 2 && string.startsWith('%') && string.endsWith('%')) {
      return QString::fromUtf8(qgetenv(string.mid(1, string.size() - 2).toLatin1().constData()));
   }
#endif

   return string;
}

void QFileDialogComboBox::setFileDialogPrivate(QFileDialogPrivate *d_pointer)
{
   d_ptr = d_pointer;
   urlModel = new QUrlModel(this);
   urlModel->showFullPath = true;
   urlModel->setFileSystemModel(d_ptr->model);
   setModel(urlModel);
}

void QFileDialogComboBox::showPopup()
{
   if (model()->rowCount() > 1) {
      QComboBox::showPopup();
   }

   urlModel->setUrls(QList<QUrl>());
   QList<QUrl> list;
   QModelIndex idx = d_ptr->model->index(d_ptr->rootPath());

   while (idx.isValid()) {
      QUrl url = QUrl::fromLocalFile(idx.data(QFileSystemModel::FilePathRole).toString());
      if (url.isValid()) {
         list.append(url);
      }
      idx = idx.parent();
   }

   // add "my computer"
   list.append(QUrl("file:"));
   urlModel->addUrls(list, 0);
   idx = model()->index(model()->rowCount() - 1, 0);

   // append history
   QList<QUrl> urls;
   for (int i = 0; i < m_history.count(); ++i) {
      QUrl path = QUrl::fromLocalFile(m_history.at(i));
      if (!urls.contains(path)) {
         urls.prepend(path);
      }
   }

   if (urls.count() > 0) {
      model()->insertRow(model()->rowCount());
      idx = model()->index(model()->rowCount() - 1, 0);
      // ### TODO maybe add a horizontal line before this
      model()->setData(idx, QFileDialog::tr("Recent Places"));
      QStandardItemModel *m = qobject_cast<QStandardItemModel *>(model());
      if (m) {
         Qt::ItemFlags flags = m->flags(idx);
         flags &= ~Qt::ItemIsEnabled;
         m->item(idx.row(), idx.column())->setFlags(flags);
      }
      urlModel->addUrls(urls, -1, false);
   }
   setCurrentIndex(0);

   QComboBox::showPopup();
}

// Exact same as QComboBox::paintEvent(), except we elide the text.
void QFileDialogComboBox::paintEvent(QPaintEvent *)
{
   QStylePainter painter(this);
   painter.setPen(palette().color(QPalette::Text));

   // draw the combobox frame, focusrect and selected etc.
   QStyleOptionComboBox opt;
   initStyleOption(&opt);

   QRect editRect = style()->subControlRect(QStyle::CC_ComboBox, &opt,
         QStyle::SC_ComboBoxEditField, this);
   int size = editRect.width() - opt.iconSize.width() - 4;
   opt.currentText = opt.fontMetrics.elidedText(opt.currentText, Qt::ElideMiddle, size);
   painter.drawComplexControl(QStyle::CC_ComboBox, opt);

   // draw the icon and text
   painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

QFileDialogListView::QFileDialogListView(QWidget *parent) : QListView(parent)
{
}

void QFileDialogListView::setFileDialogPrivate(QFileDialogPrivate *d_pointer)
{
   d_ptr = d_pointer;
   setSelectionBehavior(QAbstractItemView::SelectRows);
   setWrapping(true);
   setResizeMode(QListView::Adjust);
   setEditTriggers(QAbstractItemView::EditKeyPressed);
   setContextMenuPolicy(Qt::CustomContextMenu);

#ifndef QT_NO_DRAGANDDROP
   setDragDropMode(QAbstractItemView::InternalMove);
#endif
}

QSize QFileDialogListView::sizeHint() const
{
   int height = qMax(10, sizeHintForRow(0));
   return QSize(QListView::sizeHint().width() * 2, height * 30);
}

void QFileDialogListView::keyPressEvent(QKeyEvent *e)
{
#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::navigationMode() == Qt::NavigationModeKeypadDirectional) {
      QListView::keyPressEvent(e);
      return;
   }
#endif

   if (! d_ptr->itemViewKeyboardEvent(e)) {
      QListView::keyPressEvent(e);
   }
   e->accept();
}

QFileDialogTreeView::QFileDialogTreeView(QWidget *parent) : QTreeView(parent)
{
}

void QFileDialogTreeView::setFileDialogPrivate(QFileDialogPrivate *d_pointer)
{
   d_ptr = d_pointer;
   setSelectionBehavior(QAbstractItemView::SelectRows);
   setRootIsDecorated(false);
   setItemsExpandable(false);
   setSortingEnabled(true);
   header()->setSortIndicator(0, Qt::AscendingOrder);
   header()->setStretchLastSection(false);
   setTextElideMode(Qt::ElideMiddle);
   setEditTriggers(QAbstractItemView::EditKeyPressed);
   setContextMenuPolicy(Qt::CustomContextMenu);

#ifndef QT_NO_DRAGANDDROP
   setDragDropMode(QAbstractItemView::InternalMove);
#endif
}

void QFileDialogTreeView::keyPressEvent(QKeyEvent *e)
{
#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::navigationMode() == Qt::NavigationModeKeypadDirectional) {
      QTreeView::keyPressEvent(e);
      return;
   }
#endif // QT_KEYPAD_NAVIGATION

   if (!d_ptr->itemViewKeyboardEvent(e)) {
      QTreeView::keyPressEvent(e);
   }
   e->accept();
}

QSize QFileDialogTreeView::sizeHint() const
{
   int height = qMax(10, sizeHintForRow(0));
   QSize sizeHint = header()->sizeHint();
   return QSize(sizeHint.width() * 4, height * 30);
}

/*!
    // FIXME: this is a hack to avoid propagating key press events
    // to the dialog and from there to the "Ok" button
*/
void QFileDialogLineEdit::keyPressEvent(QKeyEvent *e)
{
#ifdef QT_KEYPAD_NAVIGATION
   if (QApplication::navigationMode() == Qt::NavigationModeKeypadDirectional) {
      QLineEdit::keyPressEvent(e);
      return;
   }
#endif // QT_KEYPAD_NAVIGATION

   int key = e->key();
   QLineEdit::keyPressEvent(e);

   if (!e->matches(QKeySequence::Cancel) && key != Qt::Key_Back) {
      e->accept();
   }
}

#ifndef QT_NO_FSCOMPLETER

QString QFSCompleter::pathFromIndex(const QModelIndex &index) const
{
   const QFileSystemModel *dirModel;
   if (proxyModel) {
      dirModel = qobject_cast<const QFileSystemModel *>(proxyModel->sourceModel());
   } else {
      dirModel = sourceModel;
   }

   QString currentLocation = dirModel->rootPath();
   QString path = index.data(QFileSystemModel::FilePathRole).toString();

   if (! currentLocation.isEmpty() && path.startsWith(currentLocation)) {

#if defined(Q_OS_UNIX)
      if (currentLocation == QString(QDir::separator())) {
         return path.mid(currentLocation.length());
      }
#endif

      if (currentLocation.endsWith('/')) {
         return path.mid(currentLocation.length());
      } else {
         return path.mid(currentLocation.length() + 1);
      }
   }

   return index.data(QFileSystemModel::FilePathRole).toString();
}

QStringList QFSCompleter::splitPath(const QString &path) const
{
   if (path.isEmpty()) {
      return QStringList(completionPrefix());
   }

   QString pathCopy = QDir::toNativeSeparators(path);
   QString sep = QDir::separator();

#if defined(Q_OS_WIN)
   if (pathCopy == "\\" || pathCopy == "\\\\") {
      return QStringList(pathCopy);
   }

   QString doubleSlash("\\\\");

   if (pathCopy.startsWith(doubleSlash)) {
      pathCopy = pathCopy.mid(2);
   } else {
      doubleSlash.clear();
   }

#elif defined(Q_OS_UNIX)
   bool expanded;
   pathCopy = qt_tildeExpansion(pathCopy, &expanded);

   if (expanded) {
      QFileSystemModel *dirModel;
      if (proxyModel) {
         dirModel = qobject_cast<QFileSystemModel *>(proxyModel->sourceModel());
      } else {
         dirModel = sourceModel;
      }
      dirModel->fetchMore(dirModel->index(pathCopy));
   }
#endif

   QRegularExpression re('[' + QRegularExpression::escape(sep) + ']');

#if defined(Q_OS_WIN)
   QStringList parts = pathCopy.split(re, QStringParser::SkipEmptyParts);

   if (! doubleSlash.isEmpty() && ! parts.isEmpty()) {
      parts[0].prepend(doubleSlash);
   }

   if (pathCopy.endsWith(sep)) {
      parts.append(QString());
   }

#else
   QStringList parts = pathCopy.split(re);
   if (pathCopy[0] == sep[0]) { // read the "/" at the beginning as the split removed it
      parts[0] = sep[0];
   }
#endif

#if defined(Q_OS_WIN)
   bool startsFromRoot = ! parts.isEmpty() && parts[0].endsWith(':');
#else
   bool startsFromRoot = pathCopy[0] == sep[0];
#endif

   if (parts.count() == 1 || (parts.count() > 1 && ! startsFromRoot)) {
      const QFileSystemModel *dirModel;

      if (proxyModel) {
         dirModel = qobject_cast<const QFileSystemModel *>(proxyModel->sourceModel());
      } else {
         dirModel = sourceModel;
      }

      QString currentLocation = QDir::toNativeSeparators(dirModel->rootPath());

#if defined(Q_OS_WIN)
      if (currentLocation.endsWith(':')) {
         currentLocation.append(sep);
      }
#endif

      if (currentLocation.contains(sep) && path != currentLocation) {
         QStringList currentLocationList = splitPath(currentLocation);

         while (! currentLocationList.isEmpty() && parts.count() > 0 && parts.at(0) == "..") {
            parts.removeFirst();
            currentLocationList.removeLast();
         }

         if (!currentLocationList.isEmpty() && currentLocationList.last().isEmpty()) {
            currentLocationList.removeLast();
         }

         return currentLocationList + parts;
      }
   }

   return parts;
}

#endif // QT_NO_COMPLETER

void QFileDialog::_q_pathChanged(const QString &path)
{
   Q_D(QFileDialog);
   d->_q_pathChanged(path);
}

void QFileDialog::_q_navigateBackward()
{
   Q_D(QFileDialog);
   d->_q_navigateBackward();
}

void QFileDialog::_q_navigateForward()
{
   Q_D(QFileDialog);
   d->_q_navigateForward();
}

void QFileDialog::_q_navigateToParent()
{
   Q_D(QFileDialog);
   d->_q_navigateToParent();
}

void QFileDialog::_q_createDirectory()
{
   Q_D(QFileDialog);
   d->_q_createDirectory();
}

void QFileDialog::_q_showListView()
{
   Q_D(QFileDialog);
   d->_q_showListView();
}

void QFileDialog::_q_showDetailsView()
{
   Q_D(QFileDialog);
   d->_q_showDetailsView();
}

void QFileDialog::_q_showContextMenu(const QPoint &point)
{
   Q_D(QFileDialog);
   d->_q_showContextMenu(point);
}

void QFileDialog::_q_renameCurrent()
{
   Q_D(QFileDialog);
   d->_q_renameCurrent();
}

void QFileDialog::_q_deleteCurrent()
{
   Q_D(QFileDialog);
   d->_q_deleteCurrent();
}

void QFileDialog::_q_showHidden()
{
   Q_D(QFileDialog);
   d->_q_showHidden();
}

void QFileDialog::_q_updateOkButton()
{
   Q_D(QFileDialog);
   d->_q_updateOkButton();
}

void QFileDialog::_q_currentChanged(const QModelIndex &index)
{
   Q_D(QFileDialog);
   d->_q_currentChanged(index);
}

void QFileDialog::_q_enterDirectory(const QModelIndex &index)
{
   Q_D(QFileDialog);
   d->_q_enterDirectory(index);
}

void QFileDialog::_q_emitUrlSelected(const QUrl &url)
{
   Q_D(QFileDialog);
   d->_q_emitUrlSelected(url);
}

void QFileDialog::_q_emitUrlsSelected(const QList<QUrl> &url)
{
   Q_D(QFileDialog);
   d->_q_emitUrlsSelected(url);
}

void QFileDialog::_q_nativeCurrentChanged(const QUrl &url)
{
   Q_D(QFileDialog);
   d->_q_nativeCurrentChanged(url);
}

void QFileDialog::_q_nativeEnterDirectory(const QUrl &url)
{
   Q_D(QFileDialog);
   d->_q_nativeEnterDirectory(url);
}

void QFileDialog::_q_goToDirectory(const QString &path)
{
   Q_D(QFileDialog);
   d->_q_goToDirectory(path);
}

void QFileDialog::_q_useNameFilter(int index)
{
   Q_D(QFileDialog);
   d->_q_useNameFilter(index);
}

void QFileDialog::_q_selectionChanged()
{
   Q_D(QFileDialog);
   d->_q_selectionChanged();
}

void QFileDialog::_q_goToUrl(const QUrl &url)
{
   Q_D(QFileDialog);
   d->_q_goToUrl(url);
}

void QFileDialog::_q_goHome()
{
   Q_D(QFileDialog);
   d->_q_goHome();
}

void QFileDialog::_q_showHeader(QAction *action)
{
   Q_D(QFileDialog);
   d->_q_showHeader(action);
}

void QFileDialog::_q_autoCompleteFileName(const QString &text)
{
   Q_D(QFileDialog);
   d->_q_autoCompleteFileName(text);
}

void QFileDialog::_q_rowsInserted(const QModelIndex &parent)
{
   Q_D(QFileDialog);
   d->_q_rowsInserted(parent);
}

void QFileDialog::_q_fileRenamed(const QString &path, const QString &oldName, const QString &newName)
{
   Q_D(QFileDialog);
   d->_q_fileRenamed(path, oldName, newName);
}

#endif // QT_NO_FILEDIALOG
