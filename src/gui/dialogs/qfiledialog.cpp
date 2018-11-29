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

#include <qvariant.h>

#include <qfiledialog.h>

#ifndef QT_NO_FILEDIALOG

#include <qfiledialog_p.h>
#include <qfontmetrics.h>
#include <qaction.h>
#include <qheaderview.h>
#include <qshortcut.h>
#include <qgridlayout.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <stdlib.h>
#include <qsettings.h>
#include <qdebug.h>
// emerald   #include <qmimedatabase.h>
#include <qapplication.h>
#include <qstylepainter.h>

#include <qapplication_p.h>
#include <qfileiconprovider_p.h>
#include <qwidgetitemdata_p.h>
#include <ui_qfiledialog.h>

#if defined(Q_OS_UNIX)
#include <pwd.h>
#include <unistd.h>        // for pathconf() on OS X

#elif defined(Q_OS_WIN)
#include <qt_windows.h>
#endif



Q_GLOBAL_STATIC(QUrl, lastVisitedDir)

static const qint32 QFileDialogMagic = 0xbe;



/* broom - may not be used

typedef QString (*_qt_filedialog_existing_directory_hook)(QWidget *parent, const QString &caption, const QString &dir,
      QFileDialog::Options options);

Q_GUI_EXPORT _qt_filedialog_existing_directory_hook qt_filedialog_existing_directory_hook = 0;

typedef QUrl (*_qt_filedialog_existing_directory_url_hook)(QWidget *parent, const QString &caption, const QUrl &dir,
      QFileDialog::Options options, const QStringList &supportedSchemes);
Q_GUI_EXPORT _qt_filedialog_existing_directory_url_hook qt_filedialog_existing_directory_url_hook = 0;

typedef QString (*_qt_filedialog_open_filename_hook)(QWidget *parent, const QString &caption, const QString &dir,
      const QString &filter, QString *selectedFilter, QFileDialog::Options options);
Q_GUI_EXPORT _qt_filedialog_open_filename_hook qt_filedialog_open_filename_hook = 0;

typedef QUrl (*_qt_filedialog_open_file_url_hook)(QWidget *parent, const QString &caption, const QUrl &dir,
      const QString &filter, QString *selectedFilter, QFileDialog::Options options, const QStringList &supportedSchemes);
Q_GUI_EXPORT _qt_filedialog_open_file_url_hook qt_filedialog_open_file_url_hook = 0;

typedef QStringList (*_qt_filedialog_open_filenames_hook)(QWidget *parent, const QString &caption, const QString &dir,
      const QString &filter, QString *selectedFilter, QFileDialog::Options options);
Q_GUI_EXPORT _qt_filedialog_open_filenames_hook qt_filedialog_open_filenames_hook = 0;

typedef QList<QUrl> (*_qt_filedialog_open_file_urls_hook)(QWidget *parent, const QString &caption, const QUrl &dir,
      const QString &filter, QString *selectedFilter, QFileDialog::Options options, const QStringList &supportedSchemes);
Q_GUI_EXPORT _qt_filedialog_open_file_urls_hook qt_filedialog_open_file_urls_hook = 0;

typedef QString (*_qt_filedialog_save_filename_hook)(QWidget *parent, const QString &caption, const QString &dir,
      const QString &filter, QString *selectedFilter, QFileDialog::Options options);
Q_GUI_EXPORT _qt_filedialog_save_filename_hook qt_filedialog_save_filename_hook = 0;

typedef QUrl (*_qt_filedialog_save_file_url_hook)(QWidget *parent, const QString &caption, const QUrl &dir,
      const QString &filter, QString *selectedFilter, QFileDialog::Options options, const QStringList &supportedSchemes);
Q_GUI_EXPORT _qt_filedialog_save_file_url_hook qt_filedialog_save_file_url_hook = 0;

static const qint32 QFileDialogMagic = 0xbe;
static const QString qt_file_dialog_filter_reg_exp = "^(.*)\\(([a-zA-Z0-9_.*? +;#\\-\\[\\]@\\{\\}/!<>\\$%&=^~:\\|]*)\\)$";

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
bool Q_GUI_EXPORT qt_use_native_dialogs = true; // for the benefit of testing tools, until we have a proper API
#endif

#ifdef Q_OS_WIN
#include <qwindowsstyle.h>
#endif

*/

#include <QMetaEnum>
#include <qshortcut.h>


QFileDialog::QFileDialog(QWidget *parent, Qt::WindowFlags f)
   : QDialog(*new QFileDialogPrivate, parent, f)
{
   Q_D(QFileDialog);
   d->init();
}


QFileDialog::QFileDialog(QWidget *parent, const QString &caption, const QString &directory, const QString &filter)
   : QDialog(*new QFileDialogPrivate, parent, 0)
{
   Q_D(QFileDialog);
   d->init(QUrl::fromLocalFile(directory), filter, caption);
}

/*!
    \internal
*/
QFileDialog::QFileDialog(const QFileDialogArgs &args)
   : QDialog(*new QFileDialogPrivate, args.parent, 0)
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
   if (!d->nativeDialogInUse) {
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
   stream << *lastVisitedDir();

   if (d->usingWidgets()) {
      stream << d->qFileDialogUi->treeView->header()->saveState();
   } else {
      stream << d->headerData;
   }

   stream << qint32(viewMode());

   return data;
}

/*!
    \since 4.3
    Restores the dialogs's layout, history and current directory to the \a state specified.

    Typically this is used in conjunction with QSettings to restore the size
    from a past session.

    Returns false if there are errors
*/
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

   stream >> d->headerData
      >> viewMode;


   setDirectoryUrl(lastVisitedDir()->isEmpty() ? currentDirectory : *lastVisitedDir());
   setViewMode(static_cast<QFileDialog::ViewMode>(viewMode));

   if (!d->usingWidgets()) {
      return true;
   }

   return d->restoreWidgetState(history, -1);
}

/*!
    \reimp
*/
void QFileDialog::changeEvent(QEvent *e)
{
   Q_D(QFileDialog);
   if (e->type() == QEvent::LanguageChange) {
      d->retranslateWindowTitle();
      d->retranslateStrings();
   }
   QDialog::changeEvent(e);
}

QFileDialogPrivate::QFileDialogPrivate()
   :
#ifndef QT_NO_PROXYMODEL
   proxyModel(0),
#endif
   model(0),

   currentHistoryLocation(-1),
   renameAction(0),
   deleteAction(0),
   showHiddenAction(0),
   useDefaultCaption(true),
   defaultFileTypes(true),
   qFileDialogUi(0),
   options(new QFileDialogOptions)
{
}

QFileDialogPrivate::~QFileDialogPrivate()
{
}

void QFileDialogPrivate::initHelper(QPlatformDialogHelper *h)
{
   QFileDialog *d = q_func();

   QObject::connect(h, SIGNAL(fileSelected(QUrl)), d, SLOT(_q_emitUrlSelected(QUrl)));
   QObject::connect(h, SIGNAL(filesSelected(QList<QUrl>)), d, SLOT(_q_emitUrlsSelected(QList<QUrl>)));
   QObject::connect(h, SIGNAL(currentChanged(QUrl)), d, SLOT(_q_nativeCurrentChanged(QUrl)));
   QObject::connect(h, SIGNAL(directoryEntered(QUrl)), d, SLOT(_q_nativeEnterDirectory(QUrl)));
   QObject::connect(h, SIGNAL(filterSelected(QString)), d, SLOT(filterSelected(QString)));

   static_cast<QPlatformFileDialogHelper *>(h)->setOptions(options);
   nativeDialogInUse = true;
}
void QFileDialogPrivate::helperPrepareShow(QPlatformDialogHelper *)
{
   Q_Q(QFileDialog);
   options->setWindowTitle(q->windowTitle());
   options->setHistory(q->history());
   if (usingWidgets()) {
      options->setSidebarUrls(qFileDialogUi->sidebar->urls());
   }
   if (options->initiallySelectedNameFilter().isEmpty()) {
      options->setInitiallySelectedNameFilter(q->selectedNameFilter());
   }
   if (options->initiallySelectedFiles().isEmpty()) {
      options->setInitiallySelectedFiles(userSelectedFiles());
   }
}
void QFileDialogPrivate::helperDone(QDialog::DialogCode code, QPlatformDialogHelper *)
{
   if (code == QDialog::Accepted) {
      Q_Q(QFileDialog);
      q->setViewMode(static_cast<QFileDialog::ViewMode>(options->viewMode()));
      q->setSidebarUrls(options->sidebarUrls());
      q->setHistory(options->history());
   }
}
void QFileDialogPrivate::retranslateWindowTitle()
{
   Q_Q(QFileDialog);

   if (!useDefaultCaption || setWindowTitle != q->windowTitle()) {
      return;
   }

   if (q->acceptMode() == QFileDialog::AcceptOpen) {
      const QFileDialog::FileMode fileMode = q->fileMode();

      if (fileMode == QFileDialog::DirectoryOnly || fileMode == QFileDialog::Directory) {
         q->setWindowTitle(QFileDialog::tr("Find Directory"));

      } else {
         q->setWindowTitle(QFileDialog::tr("Open"));
      }

   } else {
      q->setWindowTitle(QFileDialog::tr("Save As"));
   }

   setWindowTitle = q->windowTitle();
}

void QFileDialogPrivate::setLastVisitedDirectory(const QUrl &dir)
{
   *lastVisitedDir() = dir;
}

void QFileDialogPrivate::updateLookInLabel()
{
   if (options->isLabelExplicitlySet(QFileDialogOptions::LookIn)) {
      setLabelTextControl(QFileDialog::LookIn, options->labelText(QFileDialogOptions::LookIn));
   }
}
void QFileDialogPrivate::updateFileNameLabel()
{
   if (options->isLabelExplicitlySet(QFileDialogOptions::FileName)) {
      setLabelTextControl(QFileDialog::FileName, options->labelText(QFileDialogOptions::FileName));
   } else {
      switch (q_func()->fileMode()) {
         case QFileDialog::DirectoryOnly:
         case QFileDialog::Directory:
            setLabelTextControl(QFileDialog::FileName, QFileDialog::tr("Directory:"));
            break;
         default:
            setLabelTextControl(QFileDialog::FileName, QFileDialog::tr("File &name:"));
            break;
      }
   }
}
void QFileDialogPrivate::updateFileTypeLabel()
{
   if (options->isLabelExplicitlySet(QFileDialogOptions::FileType)) {
      setLabelTextControl(QFileDialog::FileType, options->labelText(QFileDialogOptions::FileType));
   }
}
void QFileDialogPrivate::updateOkButtonText(bool saveAsOnFolder)
{
   Q_Q(QFileDialog);
   // 'Save as' at a folder: Temporarily change to "Open".
   if (saveAsOnFolder) {
      setLabelTextControl(QFileDialog::Accept, QFileDialog::tr("&Open"));
   } else if (options->isLabelExplicitlySet(QFileDialogOptions::Accept)) {
      setLabelTextControl(QFileDialog::Accept, options->labelText(QFileDialogOptions::Accept));
      return;
   } else {
      switch (q->fileMode()) {
         case QFileDialog::DirectoryOnly:
         case QFileDialog::Directory:
            setLabelTextControl(QFileDialog::Accept, QFileDialog::tr("&Choose"));
            break;
         default:
            setLabelTextControl(QFileDialog::Accept,
               q->acceptMode() == QFileDialog::AcceptOpen ?
               QFileDialog::tr("&Open")  :
               QFileDialog::tr("&Save"));
            break;
      }
   }
}
void QFileDialogPrivate::updateCancelButtonText()
{
   if (options->isLabelExplicitlySet(QFileDialogOptions::Reject)) {
      setLabelTextControl(QFileDialog::Reject, options->labelText(QFileDialogOptions::Reject));
   }
}
void QFileDialogPrivate::retranslateStrings()
{
   Q_Q(QFileDialog);
   /* WIDGETS */
   if (defaultFileTypes) {
      q->setNameFilter(QFileDialog::tr("All Files (*)"));
   }

   if (!usingWidgets()) {
      return;
   }

   QList<QAction *> actions = qFileDialogUi->treeView->header()->actions();
   QAbstractItemModel *abstractModel = model;

#ifndef QT_NO_PROXYMODEL
   if (proxyModel) {
      abstractModel = proxyModel;
   }
#endif
   int total = qMin(abstractModel->columnCount(QModelIndex()), actions.count() + 1);
   for (int i = 1; i < total; ++i) {
      actions.at(i - 1)->setText(QFileDialog::tr("Show ") + abstractModel->headerData(i, Qt::Horizontal,
            Qt::DisplayRole).toString());
   }

   /* MENU ACTIONS */
   renameAction->setText(QFileDialog::tr("&Rename"));
   deleteAction->setText(QFileDialog::tr("&Delete"));
   showHiddenAction->setText(QFileDialog::tr("Show &hidden files"));
   newFolderAction->setText(QFileDialog::tr("&New Folder"));
   qFileDialogUi->retranslateUi(q);

   updateLookInLabel();
   updateFileNameLabel();
   updateFileTypeLabel();
   updateCancelButtonText();
}

void QFileDialogPrivate::emitFilesSelected(const QStringList &files)
{
   Q_Q(QFileDialog);
   emit q->filesSelected(files);
   if (files.count() == 1) {
      emit q->fileSelected(files.first());
   }
}

bool QFileDialogPrivate::canBeNativeDialog() const
{
   // do not use Q_Q here This function is called from ~QDialog,
   const QDialog *const q = static_cast<const QDialog *>(q_ptr);

   if (nativeDialogInUse) {
      return true;
   }

   if (q->testAttribute(Qt::WA_DontShowOnScreen)) {
      return false;
   }

   if (options->options() & QFileDialog::DontUseNativeDialog) {
      return false;
   }

   QString staticName(QFileDialog::staticMetaObject().className());
   QString dynamicName(q->metaObject()->className());

   return (staticName == dynamicName);
}

bool QFileDialogPrivate::usingWidgets() const
{
   return !nativeDialogInUse && qFileDialogUi;
}

void QFileDialog::setOption(Option option, bool on)
{
   const QFileDialog::Options previousOptions = options();
   if (!(previousOptions & option) != !on) {
      setOptions(previousOptions ^ option);
   }
}


bool QFileDialog::testOption(Option option) const
{
   Q_D(const QFileDialog);
   return d->options->testOption(static_cast<QFileDialogOptions::FileDialogOption>(option));
}


void QFileDialog::setOptions(Options options)
{
   Q_D(QFileDialog);

   Options changed = (options ^ QFileDialog::options());

   if (!changed) {
      return;
   }
   d->options->setOptions(QFileDialogOptions::FileDialogOptions(int(options)));

   if ((options & DontUseNativeDialog) && !d->usingWidgets()) {
      d->createWidgets();
   }


   if (d->usingWidgets()) {
      if (changed & DontResolveSymlinks) {
         d->model->setResolveSymlinks(!(options & DontResolveSymlinks));
      }


      if (changed & ReadOnly) {
         bool ro = (options & ReadOnly);
         d->model->setReadOnly(ro);
         d->qFileDialogUi->newFolderButton->setEnabled(!ro);
         d->renameAction->setEnabled(!ro);
         d->deleteAction->setEnabled(!ro);
      }

      if (changed & DontUseCustomDirectoryIcons) {
         QFileIconProvider::Options providerOptions = iconProvider()->options();

         if (options & DontUseCustomDirectoryIcons) {
            providerOptions |= QFileIconProvider::DontUseCustomDirectoryIcons;
         } else {
            providerOptions &= ~QFileIconProvider::DontUseCustomDirectoryIcons;
         }

         iconProvider()->setOptions(providerOptions);
      }
   }

   if (changed & HideNameFilterDetails) {
      setNameFilters(d->options->nameFilters());
   }

   if (changed & ShowDirsOnly) {
      setFilter((options & ShowDirsOnly) ? filter() & ~QDir::Files : filter() | QDir::Files);
   }

}

QFileDialog::Options QFileDialog::options() const
{
   Q_D(const QFileDialog);
   return QFileDialog::Options(int(d->options->options()));
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


/*!
    \reimp
*/
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
            d->completer->setModel(0);
         }
#endif

      } else {
         d->createWidgets();
         setAttribute(Qt::WA_DontShowOnScreen, false);

#ifndef QT_NO_FSCOMPLETER
         if (!d->nativeDialogInUse) {
            if (d->proxyModel != 0) {
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

/*!
    \internal
    set the directory to url
*/
void QFileDialogPrivate::_q_goToUrl(const QUrl &url)
{
   //The shortcut in the side bar may have a parent that is not fetched yet (e.g. an hidden file)
   //so we force the fetching
   QFileSystemModelPrivate::QFileSystemNode *node = model->d_func()->node(url.toLocalFile(), true);
   QModelIndex idx =  model->d_func()->index(node);
   _q_enterDirectory(idx);
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

   if (!d->nativeDialogInUse) {
      d->qFileDialogUi->newFolderButton->setEnabled(d->model->flags(root) & Qt::ItemIsDropEnabled);
      if (root != d->rootIndex()) {

#ifndef QT_NO_FSCOMPLETER
         if (directory.endsWith(QLatin1Char('/'))) {
            d->completer->setCompletionPrefix(newDirectory);
         } else {
            d->completer->setCompletionPrefix(newDirectory + QLatin1Char('/'));
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
   if (!directory.isValid()) {
      return;
   }
   QFileDialogPrivate::setLastVisitedDirectory(directory);
   d->options->setInitialDirectory(directory);
   if (d->nativeDialogInUse) {
      d->setDirectory_sys(directory);
   } else if (directory.isLocalFile()) {
      setDirectory(directory.toLocalFile());
   } else if (d->usingWidgets()) {
      qWarning("Non-native QFileDialog supports only local files");
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

#if defined(Q_OS_WIN)
   return false;
#elif defined(Q_OS_OSX)
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
   if (!url.isValid()) {
      return;
   }
   if (d->nativeDialogInUse) {
      d->selectFile_sys(url);
   } else if (url.isLocalFile()) {
      selectFile(url.toLocalFile());
   } else {
      qWarning("Non-native QFileDialog supports only local files");
   }
}

#ifdef Q_OS_UNIX
QString qt_tildeExpansion(const QString &path, bool *expanded = 0)
{
   if (expanded != 0) {
      *expanded = false;
   }
   if (!path.startsWith(QLatin1Char('~'))) {
      return path;
   }
   QString ret = path;

   QStringList tokens = ret.split(QDir::separator());
   if (tokens.first() == QLatin1String("~")) {
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

      if (!pw) {
         return ret;
      }

      const QString homePath = QString::fromUtf8(pw->pw_dir);
#endif
      ret.replace(0, tokens.first().length(), homePath);
   }

   if (expanded != 0) {
      *expanded = true;
   }

   return ret;
}
#endif

/**
    Returns the text in the line edit which can be one or more file names
  */
QStringList QFileDialogPrivate::typedFiles() const
{

   Q_Q(const QFileDialog);


   QStringList files;
   QString editText = lineEdit()->text();

   if (!editText.contains(QLatin1Char('"'))) {

#ifdef Q_OS_UNIX
      const QString prefix = q->directory().absolutePath() + QDir::separator();
      if (QFile::exists(prefix + editText)) {
         files << editText;
      } else {
         files << qt_tildeExpansion(editText);
      }
#else
      files << editText;
#endif

   } else {
      // " is used to separate files like so: "file1" "file2" "file3" ...
      // ### need escape character for filenames with quotes (")
      QStringList tokens = editText.split(QLatin1Char('\"'));
      for (int i = 0; i < tokens.size(); ++i) {
         if ((i % 2) == 0) {
            continue;   // Every even token is a separator
         }
#ifdef Q_OS_UNIX
         const QString token = tokens.at(i);
         const QString prefix = q->directory().absolutePath() + QDir::separator();
         if (QFile::exists(prefix + token)) {
            files << token;
         } else {
            files << qt_tildeExpansion(token);
         }
#else
         files << toInternal(tokens.at(i));
#endif
      }
   }
   return addDefaultSuffixToFiles(files);
}

QList<QUrl> QFileDialogPrivate::userSelectedFiles() const
{
   QList<QUrl> files;

   if (!usingWidgets()) {
      return addDefaultSuffixToUrls(selectedFiles_sys());
   }

   const QModelIndexList selectedRows = qFileDialogUi->listView->selectionModel()->selectedRows();

   for (const QModelIndex &index : selectedRows) {
      files.append(QUrl::fromLocalFile(index.data(QFileSystemModel::FilePathRole).toString()));
   }

   if (files.isEmpty() && !lineEdit()->text().isEmpty()) {
      const QStringList typedFilesList = typedFiles();

      for (const QString &path : typedFilesList) {
         files.append(QUrl::fromLocalFile(path));
      }
   }
   return files;
}
QStringList QFileDialogPrivate::addDefaultSuffixToFiles(const QStringList &filesToFix) const
{
   QStringList files;
   for (int i = 0; i < filesToFix.size(); ++i) {
      QString name = toInternal(filesToFix.at(i));
      QFileInfo info(name);

      // if the filename has no suffix, add the default suffix
      const QString defaultSuffix = options->defaultSuffix();

      if (!defaultSuffix.isEmpty() && !info.isDir() && name.lastIndexOf(QLatin1Char('.')) == -1) {
         name += QLatin1Char('.') + defaultSuffix;
      }

      if (info.isAbsolute()) {
         files.append(name);
      } else {
         // at this point the path should only have Qt path separators.
         // This check is needed since we might be at the root directory
         // and on Windows it already ends with slash.
         QString path = rootPath();
         if (!path.endsWith(QLatin1Char('/'))) {
            path += QLatin1Char('/');
         }
         path += name;
         files.append(path);
      }
   }
   return files;
}

QList<QUrl> QFileDialogPrivate::addDefaultSuffixToUrls(const QList<QUrl> &urlsToFix) const
{
   QList<QUrl> urls;
   const int numUrlsToFix = urlsToFix.size();

   for (int i = 0; i < numUrlsToFix; ++i) {
      QUrl url = urlsToFix.at(i);
      const QString defaultSuffix = options->defaultSuffix();
      if (!defaultSuffix.isEmpty() && !url.path().endsWith(QLatin1Char('/')) && url.path().lastIndexOf(QLatin1Char('.')) == -1) {
         url.setPath(url.path() + QLatin1Char('.') + defaultSuffix);
      }
      urls.append(url);
   }
   return urls;
}


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
/*
    Makes a list of filters from ;;-separated text.
    Used by the mac and windows implementations
*/
QStringList qt_make_filter_list(const QString &filter)
{
   QString f(filter);

   if (f.isEmpty()) {
      return QStringList();
   }

   QString sep(QLatin1String(";;"));
   int i = f.indexOf(sep, 0);
   if (i == -1) {
      if (f.indexOf(QLatin1Char('\n'), 0) != -1) {
         sep = QLatin1Char('\n');
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
   setOption(HideNameFilterDetails, !enabled);
}

bool QFileDialog::isNameFilterDetailsVisible() const
{
   return ! testOption(HideNameFilterDetails);
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

   if (!d->usingWidgets()) {
      return;
   }

   d->qFileDialogUi->fileTypeCombo->clear();
   if (cleanedFilters.isEmpty()) {
      return;
   }

   if (testOption(HideNameFilterDetails)) {
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
   if (testOption(HideNameFilterDetails)) {
      const QStringList filters = qt_strip_filters(qt_make_filter_list(filter));
      if (!filters.isEmpty()) {
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

   if (!d->usingWidgets()) {
      d->setFilter_sys();
      return;
   }

   d->model->setFilter(filters);
   d->showHiddenAction->setChecked((filters & QDir::Hidden));
}

#ifndef QT_NO_MIMETYPE
static QString nameFilterForMime(const QString &mimeType)
{

   /* emerald (mine types)

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

   d->options->setViewMode(static_cast<QFileDialogOptions::ViewMode>(mode));
   if (!d->usingWidgets()) {
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
   d->options->setFileMode(static_cast<QFileDialogOptions::FileMode>(mode));


   // keep ShowDirsOnly option in sync with fileMode (BTW, DirectoryOnly is obsolete)
   setOption(ShowDirsOnly, mode == DirectoryOnly);

   if (!d->usingWidgets()) {
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

   d->qFileDialogUi->fileTypeCombo->setEnabled(!testOption(ShowDirsOnly));
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
   d->options->setAcceptMode(static_cast<QFileDialogOptions::AcceptMode>(mode));

   // clear WA_DontShowOnScreen so that d->canBeNativeDialog() doesn't return false incorrectly
   setAttribute(Qt::WA_DontShowOnScreen, false);
   if (!d->usingWidgets()) {
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

/*
    Returns the file system model index that is the root index in the
    views
*/
QModelIndex QFileDialogPrivate::rootIndex() const
{
   return mapToSource(qFileDialogUi->listView->rootIndex());
}

QAbstractItemView *QFileDialogPrivate::currentView() const
{
   if (! qFileDialogUi->stackedWidget) {
      return nullptr;
   }

   if (qFileDialogUi->stackedWidget->currentWidget() == qFileDialogUi->listView->parent()) {
      return qFileDialogUi->listView;
   }

   return qFileDialogUi->treeView;
}

QLineEdit *QFileDialogPrivate::lineEdit() const
{
   return (QLineEdit *)qFileDialogUi->fileNameEdit;
}

int QFileDialogPrivate::maxNameLength(const QString &path)
{
#if defined(Q_OS_UNIX)
   return ::pathconf(QFile::encodeName(path).data(), _PC_NAME_MAX);


#elif defined(Q_OS_WIN)
   DWORD maxLength;
   const QString drive = path.left(3);

   std::wstring tmp = drive.toStdWString();

   if (::GetVolumeInformation(reinterpret_cast<const wchar_t *>(&tmp[0]), NULL, 0, NULL, &maxLength, NULL, NULL, 0) == false) {
      return -1;
   }
   return maxLength;

#endif
   return -1;
}

/*
    Sets the view root index to be the file system model index
*/
void QFileDialogPrivate::setRootIndex(const QModelIndex &index) const
{
   Q_ASSERT(index.isValid() ? index.model() == model : true);
   QModelIndex idx = mapFromSource(index);
   qFileDialogUi->treeView->setRootIndex(idx);
   qFileDialogUi->listView->setRootIndex(idx);
}
/*
    Select a file system model index
    returns the index that was selected (or not depending upon sortfilterproxymodel)
*/
QModelIndex QFileDialogPrivate::select(const QModelIndex &index) const
{
   Q_ASSERT(index.isValid() ? index.model() == model : true);

   QModelIndex idx = mapFromSource(index);
   if (idx.isValid() && !qFileDialogUi->listView->selectionModel()->isSelected(idx))
      qFileDialogUi->listView->selectionModel()->select(idx,
         QItemSelectionModel::Select | QItemSelectionModel::Rows);
   return idx;
}

QFileDialog::AcceptMode QFileDialog::acceptMode() const
{
   Q_D(const QFileDialog);
   return static_cast<AcceptMode>(d->options->acceptMode());
}


void QFileDialog::setReadOnly(bool enabled)
{
   setOption(ReadOnly, enabled);
}

bool QFileDialog::isReadOnly() const
{
   return testOption(ReadOnly);
}


void QFileDialog::setResolveSymlinks(bool enabled)
{
   setOption(DontResolveSymlinks, !enabled);
}

bool QFileDialog::resolveSymlinks() const
{
   return !testOption(DontResolveSymlinks);
}

void QFileDialog::setConfirmOverwrite(bool enabled)
{
   setOption(DontConfirmOverwrite, !enabled);
}

bool QFileDialog::confirmOverwrite() const
{
   return !testOption(DontConfirmOverwrite);
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

/*!
    Sets the browsing history of the filedialog to contain the given
    \a paths.
*/
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

/*!
    Returns the browsing history of the filedialog as a list of paths.
*/
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
      return 0;
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


void QFileDialogPrivate::setLabelTextControl(QFileDialog::DialogLabel label, const QString &text)
{
   if (!qFileDialogUi) {
      return;
   }

   switch (label) {
      case QFileDialog::LookIn:
         qFileDialogUi->lookInLabel->setText(text);
         break;

      case QFileDialog::FileName:
         qFileDialogUi->fileNameLabel->setText(text);
         break;

      case QFileDialog::FileType:
         qFileDialogUi->fileTypeLabel->setText(text);
         break;

      case QFileDialog::Accept:
         if (q_func()->acceptMode() == QFileDialog::AcceptOpen) {
            if (QPushButton *button = qFileDialogUi->buttonBox->button(QDialogButtonBox::Open)) {
               button->setText(text);
            }
         } else {
            if (QPushButton *button = qFileDialogUi->buttonBox->button(QDialogButtonBox::Save)) {
               button->setText(text);
            }
         }
         break;

      case QFileDialog::Reject:
         if (QPushButton *button = qFileDialogUi->buttonBox->button(QDialogButtonBox::Cancel)) {
            button->setText(text);
         }
         break;
   }
}

/*!
void QFileDialog::setLabelText(DialogLabel label, const QString &text)
{
    Q_D(QFileDialog);
    d->options->setLabelText(static_cast<QFileDialogOptions::DialogLabel>(label), text);
    d->setLabelTextControl(label, text);
}
    Returns the text shown in the filedialog in the specified \a label.
*/
QString QFileDialog::labelText(DialogLabel label) const
{
   Q_D(const QFileDialog);

   if (!d->usingWidgets()) {
      return d->options->labelText(static_cast<QFileDialogOptions::DialogLabel>(label));
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
   }
   return QString();
}

QString QFileDialog::getOpenFileName(QWidget *parent,
   const QString &caption,
   const QString &dir,
   const QString &filter,
   QString *selectedFilter,
   Options options)
{
   const QStringList schemes = QStringList("file");
   const QUrl selectedUrl = getOpenFileUrl(parent, caption, QUrl::fromLocalFile(dir), filter, selectedFilter, options, schemes);

   return selectedUrl.toLocalFile();
}

QUrl QFileDialog::getOpenFileUrl(QWidget *parent,
   const QString &caption,
   const QUrl &dir,
   const QString &filter,
   QString *selectedFilter,
   Options options,
   const QStringList &supportedSchemes)
{
   QFileDialogArgs args;
   args.parent = parent;
   args.caption = caption;
   args.directory = QFileDialogPrivate::workingDirectory(dir);
   args.selection = QFileDialogPrivate::initialSelection(dir);
   args.filter = filter;
   args.mode = ExistingFile;
   args.options = options;

   // create a  dialog
   QFileDialog dialog(args);
   dialog.setSupportedSchemes(supportedSchemes);

   if (selectedFilter && !selectedFilter->isEmpty())  {
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

QStringList QFileDialog::getOpenFileNames(QWidget *parent,
   const QString &caption,
   const QString &dir,
   const QString &filter,
   QString *selectedFilter,
   Options options)
{
   const QStringList schemes = QStringList("file");
   const QList<QUrl> selectedUrls = getOpenFileUrls(parent, caption, QUrl::fromLocalFile(dir), filter, selectedFilter, options, schemes);

   QStringList fileNames;

   for (const QUrl &url : selectedUrls) {
      fileNames << url.toLocalFile();
   }

   return fileNames;
}


QList<QUrl> QFileDialog::getOpenFileUrls(QWidget *parent,
   const QString &caption,
   const QUrl &dir,
   const QString &filter,
   QString *selectedFilter,
   Options options,
   const QStringList &supportedSchemes)
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

QString QFileDialog::getSaveFileName(QWidget *parent,
   const QString &caption,
   const QString &dir,
   const QString &filter,
   QString *selectedFilter,
   Options options)
{
   const QStringList schemes = QStringList("file");
   const QUrl selectedUrl = getSaveFileUrl(parent, caption, QUrl::fromLocalFile(dir), filter, selectedFilter, options, schemes);
   return selectedUrl.toLocalFile();
}

QUrl QFileDialog::getSaveFileUrl(QWidget *parent,
   const QString &caption,
   const QUrl &dir,
   const QString &filter,
   QString *selectedFilter,
   Options options,
   const QStringList &supportedSchemes)
{
   QFileDialogArgs args;
   args.parent = parent;
   args.caption = caption;
   args.directory = QFileDialogPrivate::workingDirectory(dir);
   args.selection = QFileDialogPrivate::initialSelection(dir);
   args.filter = filter;
   args.mode = AnyFile;
   args.options = options;

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

QString QFileDialog::getExistingDirectory(QWidget *parent,
   const QString &caption,
   const QString &dir,
   Options options)
{
   const QStringList schemes = QStringList("file");
   const QUrl selectedUrl = getExistingDirectoryUrl(parent, caption, QUrl::fromLocalFile(dir), options, schemes);
   return selectedUrl.toLocalFile();
}

QUrl QFileDialog::getExistingDirectoryUrl(QWidget *parent,
   const QString &caption,
   const QUrl &dir,
   Options options,
   const QStringList &supportedSchemes)
{
   QFileDialogArgs args;
   args.parent = parent;
   args.caption = caption;
   args.directory = QFileDialogPrivate::workingDirectory(dir);
   args.mode = (options & ShowDirsOnly ? DirectoryOnly : Directory);
   args.options = options;

   QFileDialog dialog(args);
   dialog.setSupportedSchemes(supportedSchemes);
   if (dialog.exec() == QDialog::Accepted) {
      return dialog.selectedUrls().value(0);
   }
   return QUrl();
}


inline static QUrl _qt_get_directory(const QUrl &url)
{
   if (url.isLocalFile()) {
      QFileInfo info = QFileInfo(QDir::current(), url.toLocalFile());
      if (info.exists() && info.isDir()) {
         return QUrl::fromLocalFile(QDir::cleanPath(info.absoluteFilePath()));
      }
      info.setFile(info.absolutePath());
      if (info.exists() && info.isDir()) {
         return QUrl::fromLocalFile(info.absoluteFilePath());
      }
      return QUrl();
   } else {
      return url;
   }
}

QUrl QFileDialogPrivate::workingDirectory(const QUrl &url)
{
   if (!url.isEmpty()) {
      QUrl directory = _qt_get_directory(url);
      if (!directory.isEmpty()) {
         return directory;
      }
   }
   QUrl directory = _qt_get_directory(*lastVisitedDir());

   if (!directory.isEmpty()) {
      return directory;
   }

   return QUrl::fromLocalFile(QDir::currentPath());
}

QString QFileDialogPrivate::initialSelection(const QUrl &url)
{
   if (url.isEmpty()) {
      return QString();
   }
   if (url.isLocalFile()) {
      QFileInfo info(url.toLocalFile());
      if (!info.isDir()) {
         return info.fileName();
      } else {
         return QString();
      }
   }
   // With remote URLs we can only assume.
   return url.fileName();
}

/*!
 \reimp
*/
void QFileDialog::done(int result)
{
   Q_D(QFileDialog);

   QDialog::done(result);

   if (d->receiverToDisconnectOnClose) {
      disconnect(this, d->signalToDisconnectOnClose, d->receiverToDisconnectOnClose, d->memberToDisconnectOnClose);
      d->receiverToDisconnectOnClose = 0;
   }

   d->memberToDisconnectOnClose.clear();
   d->signalToDisconnectOnClose.clear();
}

/*!
 \reimp
*/
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

   if (lineEditText == QLatin1String("..")) {
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
            QString message = tr("%1\nDirectory not found.\nPlease verify the "
                  "correct directory name was given.");
            QMessageBox::warning(this, windowTitle(), message.formatArg(info.fileName()));
#endif // QT_NO_MESSAGEBOX
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
            if (QMessageBox::warning(this, windowTitle(),
                  tr("%1 already exists.\nDo you want to replace it?")
                  .formatArg(info.fileName()),
                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
               == QMessageBox::Yes) {
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
            if (!info.exists()) {
               info = QFileInfo(d->getEnvironmentVariable(files.at(i)));
            }
            if (!info.exists()) {
#ifndef QT_NO_MESSAGEBOX
               QString message = tr("%1\nFile not found.\nPlease verify the "
                     "correct file name was given.");
               QMessageBox::warning(this, windowTitle(), message.formatArg(info.fileName()));
#endif // QT_NO_MESSAGEBOX
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

#ifndef QT_NO_SETTINGS
void QFileDialogPrivate::saveSettings()
{
   Q_Q(QFileDialog);
   QSettings settings(QSettings::UserScope, QLatin1String("CsProject"));
   settings.beginGroup(QLatin1String("FileDialog"));

   if (usingWidgets()) {
      settings.setValue(QLatin1String("sidebarWidth"), qFileDialogUi->splitter->sizes().first());
      settings.setValue(QLatin1String("shortcuts"), QUrl::toStringList(qFileDialogUi->sidebar->urls()));
      settings.setValue(QLatin1String("treeViewHeader"), qFileDialogUi->treeView->header()->saveState());
   }

   QStringList historyUrls;
   const QStringList history = q->history();

   for (const QString &path : history) {
      historyUrls << QUrl::fromLocalFile(path).toString();
   }

   settings.setValue("history",     historyUrls);
   settings.setValue("lastVisited", lastVisitedDir()->toString());

   const QMetaEnum &viewModeMeta = q->metaObject()->enumerator(q->metaObject()->indexOfEnumerator("ViewMode"));

   settings.setValue("viewMode",  viewModeMeta.key(q->viewMode()));
   settings.setValue("csVersion", QString(CS_VERSION_STR));
}

bool QFileDialogPrivate::restoreFromSettings()
{
   Q_Q(QFileDialog);

   QSettings settings(QSettings::UserScope, "CsProject");
   if (!settings.childGroups().contains("FileDialog")) {
      return false;
   }

   settings.beginGroup("FileDialog");
   q->setDirectoryUrl(lastVisitedDir()->isEmpty() ? settings.value(QLatin1String("lastVisited")).toUrl() : *lastVisitedDir());

   QString viewModeStr = settings.value("viewMode").toString();
   const QMetaEnum &viewModeMeta = q->metaObject()->enumerator(q->metaObject()->indexOfEnumerator("ViewMode"));

   int viewMode = viewModeMeta.keyToValue(viewModeStr);

   q->setViewMode(static_cast<QFileDialog::ViewMode>(viewMode));
   sidebarUrls = QUrl::fromStringList(settings.value("shortcuts").toStringList());
   headerData  = settings.value("treeViewHeader").toByteArray();

   if (! usingWidgets()) {
      return true;
   }

   QStringList history;
   for (const QString &urlStr : settings.value("history").toStringList()) {
      QUrl url(urlStr);
      if (url.isLocalFile()) {
         history << url.toLocalFile();
      }
   }

   return restoreWidgetState(history, settings.value(QLatin1String("sidebarWidth"), -1).toInt());
}

#endif // QT_NO_SETTINGS
bool QFileDialogPrivate::restoreWidgetState(QStringList &history, int splitterPosition)
{
   Q_Q(QFileDialog);

   if (splitterPosition >= 0) {
      QList<int> splitterSizes;
      splitterSizes.append(splitterPosition);
      splitterSizes.append(qFileDialogUi->splitter->widget(1)->sizeHint().width());
      qFileDialogUi->splitter->setSizes(splitterSizes);

   } else {
      if (! qFileDialogUi->splitter->restoreState(splitterState)) {
         return false;
      }

      QList<int> list = qFileDialogUi->splitter->sizes();

      if (list.count() >= 2 && (list.at(0) == 0 || list.at(1) == 0)) {
         for (int i = 0; i < list.count(); ++i) {
            list[i] = qFileDialogUi->splitter->widget(i)->sizeHint().width();
         }
         qFileDialogUi->splitter->setSizes(list);
      }
   }

   qFileDialogUi->sidebar->setUrls(sidebarUrls);
   while (history.count() > 5) {
      history.pop_front();
   }
   q->setHistory(history);

   QHeaderView *headerView = qFileDialogUi->treeView->header();
   if (!headerView->restoreState(headerData)) {
      return false;
   }

   QList<QAction *> actions = headerView->actions();
   QAbstractItemModel *abstractModel = model;

#ifndef QT_NO_PROXYMODEL
   if (proxyModel) {
      abstractModel = proxyModel;
   }
#endif

   int total = qMin(abstractModel->columnCount(QModelIndex()), actions.count() + 1);
   for (int i = 1; i < total; ++i) {
      actions.at(i - 1)->setChecked(!headerView->isSectionHidden(i));
   }

   return true;
}

/*!
    \internal

    Create widgets, layout and set default values
*/
void QFileDialogPrivate::init(const QUrl &directory, const QString &nameFilter, const QString &caption)
{
   Q_Q(QFileDialog);

   if (!caption.isEmpty()) {
      useDefaultCaption = false;
      setWindowTitle = caption;
      q->setWindowTitle(caption);
   }

   q->setAcceptMode(QFileDialog::AcceptOpen);
   nativeDialogInUse = (canBeNativeDialog() && platformFileDialogHelper() != 0);
   if (!nativeDialogInUse) {
      createWidgets();
   }
   q->setFileMode(QFileDialog::AnyFile);
   if (!nameFilter.isEmpty()) {
      q->setNameFilter(nameFilter);
   }
   q->setDirectoryUrl(workingDirectory(directory));
   q->selectFile(initialSelection(directory));

#ifndef QT_NO_SETTINGS
   if (! restoreFromSettings()) {
      const QSettings settings(QSettings::UserScope, "CsProject");
      q->restoreState(settings.value("CS/filedialog").toByteArray());
   }

#endif

   const QSize sizeHint = q->sizeHint();
   if (sizeHint.isValid()) {
      q->resize(sizeHint);
   }
}

/*!
    \internal

    Create the widgets, set properties and connections
*/
void QFileDialogPrivate::createWidgets()
{
   Q_Q(QFileDialog);

   if (qFileDialogUi) {
      return;
   }

   QSize preSize = q->testAttribute(Qt::WA_Resized) ? q->size() : QSize();
   Qt::WindowStates preState = q->windowState();
   model = new QFileSystemModel(q);
   model->setFilter(options->filter());
   model->setObjectName(QLatin1String("qt_filesystem_model"));
   if (QPlatformFileDialogHelper *helper = platformFileDialogHelper()) {
      model->setNameFilterDisables(helper->defaultNameFilterDisables());
   } else {
      model->setNameFilterDisables(false);
   }

   if (nativeDialogInUse) {
      deletePlatformHelper();
   }

   model->d_func()->disableRecursiveSort = true;
   QFileDialog::connect(model, SIGNAL(fileRenamed(QString, QString, QString)),
      q, SLOT(_q_fileRenamed(QString, QString, QString )));

   QFileDialog::connect(model, SIGNAL(rootPathChanged(QString)),
      q, SLOT(_q_pathChanged(QString)));

   QFileDialog::connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)),
      q, SLOT(_q_rowsInserted(QModelIndex)));

   model->setReadOnly(false);

   qFileDialogUi.reset(new Ui_QFileDialog());
   qFileDialogUi->setupUi(q);

   QList<QUrl> initialBookmarks;
   initialBookmarks << QUrl("file:")
      << QUrl::fromLocalFile(QDir::homePath());

   qFileDialogUi->sidebar->setModelAndUrls(model, initialBookmarks);
   QFileDialog::connect(qFileDialogUi->sidebar, SIGNAL(goToUrl(QUrl)), q, SLOT(_q_goToUrl(QUrl)));

   QObject::connect(qFileDialogUi->buttonBox, SIGNAL(accepted()), q, SLOT(accept()));
   QObject::connect(qFileDialogUi->buttonBox, SIGNAL(rejected()), q, SLOT(reject()));

   qFileDialogUi->lookInCombo->setFileDialogPrivate(this);
   QObject::connect(qFileDialogUi->lookInCombo, SIGNAL(activated(QString)), q,
      SLOT(_q_goToDirectory(QString)));

   qFileDialogUi->lookInCombo->setInsertPolicy(QComboBox::NoInsert);
   qFileDialogUi->lookInCombo->setDuplicatesEnabled(false);

   // filename
   qFileDialogUi->fileNameEdit->setFileDialogPrivate(this);

#ifndef QT_NO_SHORTCUT
   qFileDialogUi->fileNameLabel->setBuddy(qFileDialogUi->fileNameEdit);
#endif

#ifndef QT_NO_FSCOMPLETER
   completer = new QFSCompleter(model, q);
   qFileDialogUi->fileNameEdit->setCompleter(completer);
#endif

   QObject::connect(qFileDialogUi->fileNameEdit, SIGNAL(textChanged(QString)),
      q, SLOT(_q_autoCompleteFileName(QString)));

   QObject::connect(qFileDialogUi->fileNameEdit, SIGNAL(textChanged(QString)),
      q, SLOT(_q_updateOkButton()));

   QObject::connect(qFileDialogUi->fileNameEdit, SIGNAL(returnPressed()), q, SLOT(accept()));

   // filetype
   qFileDialogUi->fileTypeCombo->setDuplicatesEnabled(false);
   qFileDialogUi->fileTypeCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
   qFileDialogUi->fileTypeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

   QObject::connect(qFileDialogUi->fileTypeCombo, SIGNAL(activated(int)), q, SLOT(_q_useNameFilter(int)));

   QObject::connect(qFileDialogUi->fileTypeCombo, SIGNAL(activated(QString)), q,
      SLOT(filterSelected(QString)));

   qFileDialogUi->listView->setFileDialogPrivate(this);
   qFileDialogUi->listView->setModel(model);

   QObject::connect(qFileDialogUi->listView, SIGNAL(activated(QModelIndex)),
      q, SLOT(_q_enterDirectory(QModelIndex)));

   QObject::connect(qFileDialogUi->listView, SIGNAL(customContextMenuRequested(QPoint)),
      q, SLOT(_q_showContextMenu(QPoint)));

#ifndef QT_NO_SHORTCUT
   QShortcut *shortcut = new QShortcut(qFileDialogUi->listView);
   shortcut->setKey(QKeySequence(QLatin1String("Delete")));
   QObject::connect(shortcut, SIGNAL(activated()), q, SLOT(_q_deleteCurrent()));
#endif

   qFileDialogUi->treeView->setFileDialogPrivate(this);
   qFileDialogUi->treeView->setModel(model);
   QHeaderView *treeHeader = qFileDialogUi->treeView->header();
   QFontMetrics fm(q->font());
   treeHeader->resizeSection(0, fm.width(QLatin1String("wwwwwwwwwwwwwwwwwwwwwwwwww")));
   treeHeader->resizeSection(1, fm.width(QLatin1String("128.88 GB")));
   treeHeader->resizeSection(2, fm.width(QLatin1String("mp3Folder")));
   treeHeader->resizeSection(3, fm.width(QLatin1String("10/29/81 02:02PM")));
   treeHeader->setContextMenuPolicy(Qt::ActionsContextMenu);

   QActionGroup *showActionGroup = new QActionGroup(q);
   showActionGroup->setExclusive(false);
   QObject::connect(showActionGroup, SIGNAL(triggered(QAction *)), q, SLOT(_q_showHeader(QAction *)));;

   QAbstractItemModel *abstractModel = model;
#ifndef QT_NO_PROXYMODEL
   if (proxyModel) {
      abstractModel = proxyModel;
   }
#endif
   for (int i = 1; i < abstractModel->columnCount(QModelIndex()); ++i) {
      QAction *showHeader = new QAction(showActionGroup);
      showHeader->setCheckable(true);
      showHeader->setChecked(true);
      treeHeader->addAction(showHeader);
   }

   QScopedPointer<QItemSelectionModel> selModel(qFileDialogUi->treeView->selectionModel());
   qFileDialogUi->treeView->setSelectionModel(qFileDialogUi->listView->selectionModel());

   QObject::connect(qFileDialogUi->treeView, SIGNAL(activated(QModelIndex)),
      q, SLOT(_q_enterDirectory(QModelIndex)));

   QObject::connect(qFileDialogUi->treeView, SIGNAL(customContextMenuRequested(QPoint)),
      q, SLOT(_q_showContextMenu(QPoint)));

#ifndef QT_NO_SHORTCUT
   shortcut = new QShortcut(qFileDialogUi->treeView);
   shortcut->setKey(QKeySequence(QLatin1String("Delete")));
   QObject::connect(shortcut, SIGNAL(activated()), q, SLOT(_q_deleteCurrent()));
#endif

   // Selections
   QItemSelectionModel *selections = qFileDialogUi->listView->selectionModel();
   QObject::connect(selections, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
      q, SLOT(_q_selectionChanged()));

   QObject::connect(selections, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
      q, SLOT(_q_currentChanged(QModelIndex)));

   qFileDialogUi->splitter->setStretchFactor(qFileDialogUi->splitter->indexOf(qFileDialogUi->splitter->widget(1)),
      QSizePolicy::Expanding);

   createToolButtons();
   createMenuActions();
#ifndef QT_NO_SETTINGS
   // Try to restore from the FileDialog settings group; if it fails, fall back
   // to the pre-5.5 QByteArray serialized settings.
   if (!restoreFromSettings()) {
      const QSettings settings(QSettings::UserScope, "CsProject");
      q->restoreState(settings.value("CS/filedialog").toByteArray());
   }
#endif
   // Initial widget states from options
   q->setFileMode(static_cast<QFileDialog::FileMode>(options->fileMode()));
   q->setAcceptMode(static_cast<QFileDialog::AcceptMode>(options->acceptMode()));
   q->setViewMode(static_cast<QFileDialog::ViewMode>(options->viewMode()));
   q->setOptions(static_cast<QFileDialog::Options>(static_cast<int>(options->options())));
   if (!options->sidebarUrls().isEmpty()) {
      q->setSidebarUrls(options->sidebarUrls());
   }
   q->setDirectoryUrl(options->initialDirectory());
#ifndef QT_NO_MIMETYPE
   if (!options->mimeTypeFilters().isEmpty()) {
      q->setMimeTypeFilters(options->mimeTypeFilters());
   } else
#endif
      if (!options->nameFilters().isEmpty()) {
         q->setNameFilters(options->nameFilters());
      }
   q->selectNameFilter(options->initiallySelectedNameFilter());
   q->setDefaultSuffix(options->defaultSuffix());
   q->setHistory(options->history());
   if (options->initiallySelectedFiles().count() == 1) {
      q->selectFile(options->initiallySelectedFiles().first().fileName());
   }
   foreach (const QUrl &url, options->initiallySelectedFiles()) {
      q->selectUrl(url);
   }
   lineEdit()->selectAll();
   _q_updateOkButton();
   retranslateStrings();
   q->resize(preSize.isValid() ? preSize : q->sizeHint());
   q->setWindowState(preState);
}

void QFileDialogPrivate::_q_showHeader(QAction *action)
{
   Q_Q(QFileDialog);
   QActionGroup *actionGroup = qobject_cast<QActionGroup *>(q->sender());
   qFileDialogUi->treeView->header()->setSectionHidden(actionGroup->actions().indexOf(action) + 1, !action->isChecked());
}

#ifndef QT_NO_PROXYMODEL

void QFileDialog::setProxyModel(QAbstractProxyModel *proxyModel)
{
   Q_D(QFileDialog);

   if (!d->usingWidgets()) {
      return;
   }

   if ((! proxyModel && ! d->proxyModel) || (proxyModel == d->proxyModel)) {
      return;
   }

   QModelIndex idx = d->rootIndex();
   if (d->proxyModel) {
      disconnect(d->proxyModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
         this, SLOT(_q_rowsInserted(QModelIndex)));

   } else {
      disconnect(d->model, SIGNAL(rowsInserted(QModelIndex, int, int)),
         this, SLOT(_q_rowsInserted(QModelIndex)));
   }

   if (proxyModel != 0) {
      proxyModel->setParent(this);
      d->proxyModel = proxyModel;
      proxyModel->setSourceModel(d->model);
      d->qFileDialogUi->listView->setModel(d->proxyModel);
      d->qFileDialogUi->treeView->setModel(d->proxyModel);

#ifndef QT_NO_FSCOMPLETER
      d->completer->setModel(d->proxyModel);
      d->completer->proxyModel = d->proxyModel;
#endif

      connect(d->proxyModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
         this, SLOT(_q_rowsInserted(QModelIndex)));

   } else {
      d->proxyModel = 0;
      d->qFileDialogUi->listView->setModel(d->model);
      d->qFileDialogUi->treeView->setModel(d->model);
#ifndef QT_NO_FSCOMPLETER
      d->completer->setModel(d->model);
      d->completer->sourceModel = d->model;
      d->completer->proxyModel = 0;
#endif

      connect(d->model, SIGNAL(rowsInserted(QModelIndex, int, int)),
         this, SLOT(_q_rowsInserted(QModelIndex)));
   }

   QScopedPointer<QItemSelectionModel> selModel(d->qFileDialogUi->treeView->selectionModel());
   d->qFileDialogUi->treeView->setSelectionModel(d->qFileDialogUi->listView->selectionModel());

   d->setRootIndex(idx);

   // reconnect selection
   QItemSelectionModel *selections = d->qFileDialogUi->listView->selectionModel();
   QObject::connect(selections, SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
      this, SLOT(_q_selectionChanged()));

   QObject::connect(selections, SIGNAL(currentChanged(QModelIndex, QModelIndex)),
      this, SLOT(_q_currentChanged(QModelIndex)));
}


QAbstractProxyModel *QFileDialog::proxyModel() const
{
   Q_D(const QFileDialog);
   return d->proxyModel;
}
#endif // QT_NO_PROXYMODEL

/*!
    \internal

    Create tool buttons, set properties and connections
*/
void QFileDialogPrivate::createToolButtons()
{
   Q_Q(QFileDialog);
   qFileDialogUi->backButton->setIcon(q->style()->standardIcon(QStyle::SP_ArrowBack, 0, q));
   qFileDialogUi->backButton->setAutoRaise(true);
   qFileDialogUi->backButton->setEnabled(false);
   QObject::connect(qFileDialogUi->backButton, SIGNAL(clicked()), q, SLOT(_q_navigateBackward()));

   qFileDialogUi->forwardButton->setIcon(q->style()->standardIcon(QStyle::SP_ArrowForward, 0, q));
   qFileDialogUi->forwardButton->setAutoRaise(true);
   qFileDialogUi->forwardButton->setEnabled(false);
   QObject::connect(qFileDialogUi->forwardButton, SIGNAL(clicked()), q, SLOT(_q_navigateForward()));

   qFileDialogUi->toParentButton->setIcon(q->style()->standardIcon(QStyle::SP_FileDialogToParent, 0, q));
   qFileDialogUi->toParentButton->setAutoRaise(true);
   qFileDialogUi->toParentButton->setEnabled(false);
   QObject::connect(qFileDialogUi->toParentButton, SIGNAL(clicked()), q, SLOT(_q_navigateToParent()));

   qFileDialogUi->listModeButton->setIcon(q->style()->standardIcon(QStyle::SP_FileDialogListView, 0, q));
   qFileDialogUi->listModeButton->setAutoRaise(true);
   qFileDialogUi->listModeButton->setDown(true);
   QObject::connect(qFileDialogUi->listModeButton, SIGNAL(clicked()), q, SLOT(_q_showListView()));

   qFileDialogUi->detailModeButton->setIcon(q->style()->standardIcon(QStyle::SP_FileDialogDetailedView, 0, q));
   qFileDialogUi->detailModeButton->setAutoRaise(true);
   QObject::connect(qFileDialogUi->detailModeButton, SIGNAL(clicked()), q, SLOT(_q_showDetailsView()));

   QSize toolSize(qFileDialogUi->fileNameEdit->sizeHint().height(), qFileDialogUi->fileNameEdit->sizeHint().height());
   qFileDialogUi->backButton->setFixedSize(toolSize);
   qFileDialogUi->listModeButton->setFixedSize(toolSize);
   qFileDialogUi->detailModeButton->setFixedSize(toolSize);
   qFileDialogUi->forwardButton->setFixedSize(toolSize);
   qFileDialogUi->toParentButton->setFixedSize(toolSize);

   qFileDialogUi->newFolderButton->setIcon(q->style()->standardIcon(QStyle::SP_FileDialogNewFolder, 0, q));
   qFileDialogUi->newFolderButton->setFixedSize(toolSize);
   qFileDialogUi->newFolderButton->setAutoRaise(true);
   qFileDialogUi->newFolderButton->setEnabled(false);
   QObject::connect(qFileDialogUi->newFolderButton, SIGNAL(clicked()), q, SLOT(_q_createDirectory()));
}

/*!
    \internal

    Create actions which will be used in the right click.
*/
void QFileDialogPrivate::createMenuActions()
{
   Q_Q(QFileDialog);

   QAction *goHomeAction =  new QAction(q);
#ifndef QT_NO_SHORTCUT
   goHomeAction->setShortcut(Qt::CTRL + Qt::Key_H + Qt::SHIFT);
#endif
   QObject::connect(goHomeAction, SIGNAL(triggered()), q, SLOT(_q_goHome()));
   q->addAction(goHomeAction);

   // ### TODO add Desktop & Computer actions

   QAction *goToParent =  new QAction(q);
   goToParent->setObjectName(QLatin1String("qt_goto_parent_action"));

#ifndef QT_NO_SHORTCUT
   goToParent->setShortcut(Qt::CTRL + Qt::UpArrow);
#endif

   QObject::connect(goToParent, SIGNAL(triggered()), q, SLOT(_q_navigateToParent()));
   q->addAction(goToParent);

   renameAction = new QAction(q);
   renameAction->setEnabled(false);
   renameAction->setObjectName(QLatin1String("qt_rename_action"));
   QObject::connect(renameAction, SIGNAL(triggered()), q, SLOT(_q_renameCurrent()));

   deleteAction = new QAction(q);
   deleteAction->setEnabled(false);
   deleteAction->setObjectName(QLatin1String("qt_delete_action"));
   QObject::connect(deleteAction, SIGNAL(triggered()), q, SLOT(_q_deleteCurrent()));

   showHiddenAction = new QAction(q);
   showHiddenAction->setObjectName(QLatin1String("qt_show_hidden_action"));
   showHiddenAction->setCheckable(true);
   QObject::connect(showHiddenAction, SIGNAL(triggered()), q, SLOT(_q_showHidden()));

   newFolderAction = new QAction(q);
   newFolderAction->setObjectName(QLatin1String("qt_new_folder_action"));
   QObject::connect(newFolderAction, SIGNAL(triggered()), q, SLOT(_q_createDirectory()));
}

void QFileDialogPrivate::_q_goHome()
{
   Q_Q(QFileDialog);
   q->setDirectory(QDir::homePath());
}

/*!
    \internal

    Update history with new path, buttons, and combo
*/
void QFileDialogPrivate::_q_pathChanged(const QString &newPath)
{
   Q_Q(QFileDialog);
   QDir dir(model->rootDirectory());
   qFileDialogUi->toParentButton->setEnabled(dir.exists());
   qFileDialogUi->sidebar->selectUrl(QUrl::fromLocalFile(newPath));
   q->setHistory(qFileDialogUi->lookInCombo->history());

   if (currentHistoryLocation < 0 || currentHistory.value(currentHistoryLocation) != QDir::toNativeSeparators(newPath)) {
      while (currentHistoryLocation >= 0 && currentHistoryLocation + 1 < currentHistory.count()) {
         currentHistory.removeLast();
      }
      currentHistory.append(QDir::toNativeSeparators(newPath));
      ++currentHistoryLocation;
   }
   qFileDialogUi->forwardButton->setEnabled(currentHistory.size() - currentHistoryLocation > 1);
   qFileDialogUi->backButton->setEnabled(currentHistoryLocation > 0);
}

/*!
    \internal

    Navigates to the last directory viewed in the dialog.
*/
void QFileDialogPrivate::_q_navigateBackward()
{
   Q_Q(QFileDialog);
   if (!currentHistory.isEmpty() && currentHistoryLocation > 0) {
      --currentHistoryLocation;
      QString previousHistory = currentHistory.at(currentHistoryLocation);
      q->setDirectory(previousHistory);
   }
}

/*!
    \internal

    Navigates to the last directory viewed in the dialog.
*/
void QFileDialogPrivate::_q_navigateForward()
{
   Q_Q(QFileDialog);
   if (!currentHistory.isEmpty() && currentHistoryLocation < currentHistory.size() - 1) {
      ++currentHistoryLocation;
      QString nextHistory = currentHistory.at(currentHistoryLocation);
      q->setDirectory(nextHistory);
   }
}

/*!
    \internal

    Navigates to the parent directory of the currently displayed directory
    in the dialog.
*/
void QFileDialogPrivate::_q_navigateToParent()
{
   Q_Q(QFileDialog);
   QDir dir(model->rootDirectory());
   QString newDirectory;
   if (dir.isRoot()) {
      newDirectory = model->myComputer().toString();
   } else {
      dir.cdUp();
      newDirectory = dir.absolutePath();
   }
   q->setDirectory(newDirectory);
   emit q->directoryEntered(newDirectory);
}

/*!
    \internal

    Creates a new directory, first asking the user for a suitable name.
*/
void QFileDialogPrivate::_q_createDirectory()
{
   Q_Q(QFileDialog);
   qFileDialogUi->listView->clearSelection();

   QString newFolderString = QFileDialog::tr("New Folder");
   QString folderName = newFolderString;
   QString prefix = q->directory().absolutePath() + QDir::separator();

   if (QFile::exists(prefix + folderName)) {
      qint64 suffix = 2;
      while (QFile::exists(prefix + folderName)) {
         folderName = newFolderString + QString::number(suffix++);
      }
   }

   QModelIndex parent = rootIndex();
   QModelIndex index = model->mkdir(parent, folderName);
   if (!index.isValid()) {
      return;
   }

   index = select(index);
   if (index.isValid()) {
      qFileDialogUi->treeView->setCurrentIndex(index);
      currentView()->edit(index);
   }
}

void QFileDialogPrivate::_q_showListView()
{
   qFileDialogUi->listModeButton->setDown(true);
   qFileDialogUi->detailModeButton->setDown(false);
   qFileDialogUi->treeView->hide();
   qFileDialogUi->listView->show();
   qFileDialogUi->stackedWidget->setCurrentWidget(qFileDialogUi->listView->parentWidget());
   qFileDialogUi->listView->doItemsLayout();
}

void QFileDialogPrivate::_q_showDetailsView()
{
   qFileDialogUi->listModeButton->setDown(false);
   qFileDialogUi->detailModeButton->setDown(true);
   qFileDialogUi->listView->hide();
   qFileDialogUi->treeView->show();
   qFileDialogUi->stackedWidget->setCurrentWidget(qFileDialogUi->treeView->parentWidget());
   qFileDialogUi->treeView->doItemsLayout();
}

/*!
    \internal

    Show the context menu for the file/dir under position
*/
void QFileDialogPrivate::_q_showContextMenu(const QPoint &position)
{
#ifdef QT_NO_MENU
   Q_UNUSED(position);
#else
   Q_Q(QFileDialog);
   QAbstractItemView *view = 0;
   if (q->viewMode() == QFileDialog::Detail) {
      view = qFileDialogUi->treeView;
   } else {
      view = qFileDialogUi->listView;
   }
   QModelIndex index = view->indexAt(position);
   index = mapToSource(index.sibling(index.row(), 0));

   QMenu menu(view);
   if (index.isValid()) {
      // file context menu
      const bool ro = model && model->isReadOnly();

      QFile::Permissions p(index.parent().data(QFileSystemModel::FilePermissions).toInt());
      renameAction->setEnabled(! ro && p & QFile::WriteUser);
      menu.addAction(renameAction);
      deleteAction->setEnabled(! ro && p & QFile::WriteUser);

      menu.addAction(deleteAction);
      menu.addSeparator();
   }
   menu.addAction(showHiddenAction);
   if (qFileDialogUi->newFolderButton->isVisible()) {
      newFolderAction->setEnabled(qFileDialogUi->newFolderButton->isEnabled());
      menu.addAction(newFolderAction);
   }
   menu.exec(view->viewport()->mapToGlobal(position));
#endif // QT_NO_MENU
}

/*!
    \internal
*/
void QFileDialogPrivate::_q_renameCurrent()
{
   Q_Q(QFileDialog);
   QModelIndex index = qFileDialogUi->listView->currentIndex();
   index = index.sibling(index.row(), 0);
   if (q->viewMode() == QFileDialog::List) {
      qFileDialogUi->listView->edit(index);
   } else {
      qFileDialogUi->treeView->edit(index);
   }
}

bool QFileDialogPrivate::removeDirectory(const QString &path)
{
   QModelIndex modelIndex = model->index(path);
   return model->remove(modelIndex);
}

/*!
    \internal

    Deletes the currently selected item in the dialog.
*/
void QFileDialogPrivate::_q_deleteCurrent()
{
   if (model->isReadOnly()) {
      return;
   }

   QModelIndexList list = qFileDialogUi->listView->selectionModel()->selectedRows();
   for (int i = list.count() - 1; i >= 0; --i) {
      QModelIndex index = list.at(i);
      if (index == qFileDialogUi->listView->rootIndex()) {
         continue;
      }

      index = mapToSource(index.sibling(index.row(), 0));
      if (!index.isValid()) {
         continue;
      }

      QString fileName = index.data(QFileSystemModel::FileNameRole).toString();
      QString filePath = index.data(QFileSystemModel::FilePathRole).toString();
      bool isDir = model->isDir(index);

      QFile::Permissions p(index.parent().data(QFileSystemModel::FilePermissions).toInt());
#ifndef QT_NO_MESSAGEBOX
      Q_Q(QFileDialog);
      if (!(p & QFile::WriteUser) && (QMessageBox::warning(q_func(), QFileDialog::tr("Delete"),
               QFileDialog::tr("'%1' is write protected.\nDo you want to delete it anyway?")
               .formatArg(fileName),
               QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)) {
         return;
      } else if (QMessageBox::warning(q_func(), QFileDialog::tr("Delete"),
            QFileDialog::tr("Are you sure you want to delete '%1'?")
            .formatArg(fileName),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
         return;
      }

#else
      if (!(p & QFile::WriteUser)) {
         return;
      }
#endif // QT_NO_MESSAGEBOX

      // the event loop has run, we can NOT reuse index because the model might have removed it.
      if (isDir) {
         if (!removeDirectory(filePath)) {
#ifndef QT_NO_MESSAGEBOX
            QMessageBox::warning(q, q->windowTitle(),
               QFileDialog::tr("Could not delete directory."));
#endif
         }
      } else {
         model->remove(index);
      }
   }
}

void QFileDialogPrivate::_q_autoCompleteFileName(const QString &text)
{
   if (text.startsWith(QLatin1String("//")) || text.startsWith(QLatin1Char('\\'))) {
      qFileDialogUi->listView->selectionModel()->clearSelection();
      return;
   }

   QStringList multipleFiles = typedFiles();
   if (multipleFiles.count() > 0) {
      QModelIndexList oldFiles = qFileDialogUi->listView->selectionModel()->selectedRows();
      QModelIndexList newFiles;
      for (int i = 0; i < multipleFiles.count(); ++i) {
         QModelIndex idx = model->index(multipleFiles.at(i));
         if (oldFiles.contains(idx)) {
            oldFiles.removeAll(idx);
         } else {
            newFiles.append(idx);
         }
      }
      for (int i = 0; i < newFiles.count(); ++i) {
         select(newFiles.at(i));
      }
      if (lineEdit()->hasFocus())
         for (int i = 0; i < oldFiles.count(); ++i)
            qFileDialogUi->listView->selectionModel()->select(oldFiles.at(i),
               QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
   }
}

/*!
    \internal
*/
void QFileDialogPrivate::_q_updateOkButton()
{
   Q_Q(QFileDialog);
   QPushButton *button =  qFileDialogUi->buttonBox->button((q->acceptMode() == QFileDialog::AcceptOpen)
         ? QDialogButtonBox::Open : QDialogButtonBox::Save);

   if (! button) {
      return;
   }

   const QFileDialog::FileMode fileMode = q->fileMode();

   bool enableButton = true;
   bool isOpenDirectory = false;

   QStringList files = q->selectedFiles();
   QString lineEditText = lineEdit()->text();

   if (lineEditText.startsWith(QLatin1String("//")) || lineEditText.startsWith(QLatin1Char('\\'))) {
      button->setEnabled(true);
      updateOkButtonText();
      return;
   }

   if (files.isEmpty()) {
      enableButton = false;
   } else if (lineEditText == QLatin1String("..")) {
      isOpenDirectory = true;
   } else {
      switch (fileMode) {
         case QFileDialog::DirectoryOnly:
         case QFileDialog::Directory: {
            QString fn = files.first();
            QModelIndex idx = model->index(fn);
            if (!idx.isValid()) {
               idx = model->index(getEnvironmentVariable(fn));
            }
            if (!idx.isValid() || !model->isDir(idx)) {
               enableButton = false;
            }
            break;
         }
         case QFileDialog::AnyFile: {
            QString fn = files.first();
            QFileInfo info(fn);
            QModelIndex idx = model->index(fn);
            QString fileDir;
            QString fileName;
            if (info.isDir()) {
               fileDir = info.canonicalFilePath();
            } else {
               fileDir = fn.mid(0, fn.lastIndexOf(QLatin1Char('/')));
               fileName = fn.mid(fileDir.length() + 1);
            }
            if (lineEditText.contains(QLatin1String(".."))) {
               fileDir = info.canonicalFilePath();
               fileName = info.fileName();
            }

            if (fileDir == q->directory().canonicalPath() && fileName.isEmpty()) {
               enableButton = false;
               break;
            }
            if (idx.isValid() && model->isDir(idx)) {
               isOpenDirectory = true;
               enableButton = true;
               break;
            }
            if (!idx.isValid()) {
               int maxLength = maxNameLength(fileDir);
               enableButton = maxLength < 0 || fileName.length() <= maxLength;
            }
            break;
         }
         case QFileDialog::ExistingFile:
         case QFileDialog::ExistingFiles:
            for (int i = 0; i < files.count(); ++i) {
               QModelIndex idx = model->index(files.at(i));
               if (!idx.isValid()) {
                  idx = model->index(getEnvironmentVariable(files.at(i)));
               }
               if (!idx.isValid()) {
                  enableButton = false;
                  break;
               }
               if (idx.isValid() && model->isDir(idx)) {
                  isOpenDirectory = true;
                  break;
               }
            }
            break;
         default:
            break;
      }
   }

   button->setEnabled(enableButton);
   updateOkButtonText(isOpenDirectory);
}

/*!
    \internal
*/
void QFileDialogPrivate::_q_currentChanged(const QModelIndex &index)
{
   _q_updateOkButton();
   emit q_func()->currentChanged(index.data(QFileSystemModel::FilePathRole).toString());
}

/*!
    \internal

    This is called when the user double clicks on a file with the corresponding
    model item \a index.
*/
void QFileDialogPrivate::_q_enterDirectory(const QModelIndex &index)
{
   Q_Q(QFileDialog);
   // My Computer or a directory
   QModelIndex sourceIndex = index.model() == proxyModel ? mapToSource(index) : index;
   QString path = sourceIndex.data(QFileSystemModel::FilePathRole).toString();

   if (path.isEmpty() || model->isDir(sourceIndex)) {
      const QFileDialog::FileMode fileMode = q->fileMode();
      q->setDirectory(path);
      emit q->directoryEntered(path);

      if (fileMode == QFileDialog::Directory
         || fileMode == QFileDialog::DirectoryOnly) {
         // ### find out why you have to do both of these.
         lineEdit()->setText(QString());
         lineEdit()->clear();
      }
   } else {
      // Do not accept when shift-clicking to multi-select a file in environments with single-click-activation (KDE)
      if (!q->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick, nullptr, qFileDialogUi->treeView)
         || q->fileMode() != QFileDialog::ExistingFiles || !(QGuiApplication::keyboardModifiers() & Qt::CTRL)) {
         q->accept();
      }
   }
}

/*!
    \internal

    Changes the file dialog's current directory to the one specified
    by \a path.
*/
void QFileDialogPrivate::_q_goToDirectory(const QString &path)
{
#ifndef QT_NO_MESSAGEBOX
   Q_Q(QFileDialog);
#endif
   QModelIndex index = qFileDialogUi->lookInCombo->model()->index(qFileDialogUi->lookInCombo->currentIndex(),
         qFileDialogUi->lookInCombo->modelColumn(),
         qFileDialogUi->lookInCombo->rootModelIndex());
   QString path2 = path;
   if (!index.isValid()) {
      index = mapFromSource(model->index(getEnvironmentVariable(path)));
   } else {
      path2 = index.data(UrlRole).toUrl().toLocalFile();
      index = mapFromSource(model->index(path2));
   }
   QDir dir(path2);
   if (!dir.exists()) {
      dir = getEnvironmentVariable(path2);
   }

   if (dir.exists() || path2.isEmpty() || path2 == model->myComputer().toString()) {
      _q_enterDirectory(index);
#ifndef QT_NO_MESSAGEBOX
   } else {
      QString message = QFileDialog::tr("%1\nDirectory not found.\nPlease verify the "
            "correct directory name was given.");
      QMessageBox::warning(q, q->windowTitle(), message.formatArg(path2));
#endif // QT_NO_MESSAGEBOX
   }
}


/*!
    \internal

    Sets the current name filter to be nameFilter and
    update the qFileDialogUi->fileNameEdit when in AcceptSave mode with the new extension.
*/
void QFileDialogPrivate::_q_useNameFilter(int index)
{
   QStringList nameFilters = options->nameFilters();
   if (index == nameFilters.size()) {
      QAbstractItemModel *comboModel = qFileDialogUi->fileTypeCombo->model();
      nameFilters.append(comboModel->index(comboModel->rowCount() - 1, 0).data().toString());
      options->setNameFilters(nameFilters);
   }

   QString nameFilter = nameFilters.at(index);
   QStringList newNameFilters = QPlatformFileDialogHelper::cleanFilterList(nameFilter);

   if (q_func()->acceptMode() == QFileDialog::AcceptSave) {
      QString newNameFilterExtension;

      if (newNameFilters.count() > 0) {
         newNameFilterExtension = QFileInfo(newNameFilters.at(0)).suffix();
      }

      QString fileName = lineEdit()->text();
      const QString fileNameExtension = QFileInfo(fileName).suffix();
      if (!fileNameExtension.isEmpty() && !newNameFilterExtension.isEmpty()) {
         const int fileNameExtensionLength = fileNameExtension.count();
         fileName.replace(fileName.count() - fileNameExtensionLength,
            fileNameExtensionLength, newNameFilterExtension);
         qFileDialogUi->listView->clearSelection();
         lineEdit()->setText(fileName);
      }
   }

   model->setNameFilters(newNameFilters);
}

/*!
    \internal

    This is called when the model index corresponding to the current file is changed
    from \a index to \a current.
*/
void QFileDialogPrivate::_q_selectionChanged()
{
   const QFileDialog::FileMode fileMode = q_func()->fileMode();
   QModelIndexList indexes = qFileDialogUi->listView->selectionModel()->selectedRows();
   bool stripDirs = (fileMode != QFileDialog::DirectoryOnly && fileMode != QFileDialog::Directory);

   QStringList allFiles;
   for (int i = 0; i < indexes.count(); ++i) {
      if (stripDirs && model->isDir(mapToSource(indexes.at(i)))) {
         continue;
      }
      allFiles.append(indexes.at(i).data().toString());
   }
   if (allFiles.count() > 1)
      for (int i = 0; i < allFiles.count(); ++i) {
         allFiles.replace(i, QString(QLatin1Char('"') + allFiles.at(i) + QLatin1Char('"')));
      }

   QString finalFiles = allFiles.join(" ");

   if (!finalFiles.isEmpty() && !lineEdit()->hasFocus() && lineEdit()->isVisible()) {
      lineEdit()->setText(finalFiles);
   } else {
      _q_updateOkButton();
   }
}

/*!
    \internal

    Includes hidden files and directories in the items displayed in the dialog.
*/
void QFileDialogPrivate::_q_showHidden()
{
   Q_Q(QFileDialog);
   QDir::Filters dirFilters = q->filter();
   if (showHiddenAction->isChecked()) {
      dirFilters |= QDir::Hidden;
   } else {
      dirFilters &= ~QDir::Hidden;
   }
   q->setFilter(dirFilters);
}

/*!
    \internal

    When parent is root and rows have been inserted when none was there before
    then select the first one.
*/
void QFileDialogPrivate::_q_rowsInserted(const QModelIndex &parent)
{
   if (!qFileDialogUi->treeView
      || parent != qFileDialogUi->treeView->rootIndex()
      || !qFileDialogUi->treeView->selectionModel()
      || qFileDialogUi->treeView->selectionModel()->hasSelection()
      || qFileDialogUi->treeView->model()->rowCount(parent) == 0) {
      return;
   }
}

void QFileDialogPrivate::_q_fileRenamed(const QString &path, const QString &oldName, const QString &newName)
{
   const QFileDialog::FileMode fileMode = q_func()->fileMode();

   if (fileMode == QFileDialog::Directory || fileMode == QFileDialog::DirectoryOnly) {
      if (path == rootPath() && lineEdit()->text() == oldName) {
         lineEdit()->setText(newName);
      }
   }
}

void QFileDialogPrivate::_q_emitUrlSelected(const QUrl &file)
{
   Q_Q(QFileDialog);
   emit q->urlSelected(file);
   if (file.isLocalFile()) {
      emit q->fileSelected(file.toLocalFile());
   }
}

void QFileDialogPrivate::_q_emitUrlsSelected(const QList<QUrl> &files)
{
   Q_Q(QFileDialog);
   emit q->urlsSelected(files);
   QStringList localFiles;
   foreach (const QUrl &file, files)
      if (file.isLocalFile()) {
         localFiles.append(file.toLocalFile());
      }
   if (!localFiles.isEmpty()) {
      emit q->filesSelected(localFiles);
   }
}

void QFileDialogPrivate::_q_nativeCurrentChanged(const QUrl &file)
{
   Q_Q(QFileDialog);
   emit q->currentUrlChanged(file);
   if (file.isLocalFile()) {
      emit q->currentChanged(file.toLocalFile());
   }
}

void QFileDialogPrivate::_q_nativeEnterDirectory(const QUrl &directory)
{
   Q_Q(QFileDialog);
   emit q->directoryUrlEntered(directory);
   if (!directory.isEmpty()) { // Windows native dialogs occasionally emit signals with empty strings.
      *lastVisitedDir() = directory;
      if (directory.isLocalFile()) {
         emit q->directoryEntered(directory.toLocalFile());
      }
   }
}

void QFileDialog::_q_pathChanged(const QString &un_named_arg1)
{
   Q_D(QFileDialog);
   d->_q_pathChanged(un_named_arg1);
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

void QFileDialog::_q_showContextMenu(const QPoint &un_named_arg1)
{
   Q_D(QFileDialog);
   d->_q_showContextMenu(un_named_arg1);
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

void QFileDialog::_q_showHeader(QAction *un_named_arg1)
{
   Q_D(QFileDialog);
   d->_q_showHeader(un_named_arg1);
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

#if defined(Q_OS_MAC)
void QFileDialog::_q_macRunNativeAppModalPanel()
{
   Q_D(QFileDialog);
   d->_q_macRunNativeAppModalPanel();
}
#endif


/*!
    \internal

    For the list and tree view watch keys to goto parent and back in the history

    returns true if handled
*/
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
#endif // QT_KEYPAD_NAVIGATION

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

   if (!currentLocation.isEmpty() && path.startsWith(currentLocation)) {
#if defined(Q_OS_UNIX)
      if (currentLocation == QDir::separator()) {
         return path.mid(currentLocation.length());
      }
#endif
      if (currentLocation.endsWith(QLatin1Char('/'))) {
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
   if (pathCopy == QLatin1String("\\") || pathCopy == QLatin1String("\\\\")) {
      return QStringList(pathCopy);
   }
   QString doubleSlash(QLatin1String("\\\\"));
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

   QRegularExpression re(QLatin1Char('[') + QRegularExpression::escape(sep) + ']');

#if defined(Q_OS_WIN)
   QStringList parts = pathCopy.split(re, QStringParser::SkipEmptyParts);
   if (!doubleSlash.isEmpty() && !parts.isEmpty()) {
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
   bool startsFromRoot = !parts.isEmpty() && parts[0].endsWith(QLatin1Char(':'));
#else
   bool startsFromRoot = pathCopy[0] == sep[0];
#endif
   if (parts.count() == 1 || (parts.count() > 1 && !startsFromRoot)) {
      const QFileSystemModel *dirModel;
      if (proxyModel) {
         dirModel = qobject_cast<const QFileSystemModel *>(proxyModel->sourceModel());
      } else {
         dirModel = sourceModel;
      }
      QString currentLocation = QDir::toNativeSeparators(dirModel->rootPath());
#if defined(Q_OS_WIN)
      if (currentLocation.endsWith(QLatin1Char(':'))) {
         currentLocation.append(sep);
      }
#endif
      if (currentLocation.contains(sep) && path != currentLocation) {
         QStringList currentLocationList = splitPath(currentLocation);
         while (!currentLocationList.isEmpty()
            && parts.count() > 0
            && parts.at(0) == QLatin1String("..")) {
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

#endif // QT_NO_FILEDIALOG
