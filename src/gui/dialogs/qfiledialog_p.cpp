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

#include <qaction.h>
#include <qheaderview.h>
#include <qmenu.h>
#include <qmessagebox.h>
#include <qsettings.h>
#include <qshortcut.h>

#include <qfiledialog_p.h>

#include <ui_qfiledialog.h>

#if defined(Q_OS_UNIX)
#include <pwd.h>
#include <unistd.h>        // for pathconf() on OS X

#elif defined(Q_OS_WIN)
#include <qt_windows.h>
#endif

QUrl *cs_internal_lastVisitedDir();

QFileDialogPrivate::QFileDialogPrivate()
   :
#ifndef QT_NO_PROXYMODEL
     proxyModel(nullptr),
#endif
     model(nullptr), currentHistoryLocation(-1), renameAction(nullptr), deleteAction(nullptr), showHiddenAction(nullptr),
     useDefaultCaption(true), defaultFileTypes(true), qFileDialogUi(nullptr), options(new QPlatformFileDialogOptions)
{
}

QFileDialogPrivate::~QFileDialogPrivate()
{
}

void QFileDialogPrivate::initHelper(QPlatformDialogHelper *h)
{
   QFileDialog *d = q_func();

   auto obj = dynamic_cast<QPlatformFileDialogHelper *>(h);
   assert(obj != nullptr);

   QObject::connect(obj, &QPlatformFileDialogHelper::fileSelected,     d, &QFileDialog::_q_emitUrlSelected);
   QObject::connect(obj, &QPlatformFileDialogHelper::filesSelected,    d, &QFileDialog::_q_emitUrlsSelected);
   QObject::connect(obj, &QPlatformFileDialogHelper::currentChanged,   d, &QFileDialog::_q_nativeCurrentChanged);
   QObject::connect(obj, &QPlatformFileDialogHelper::directoryEntered, d, &QFileDialog::_q_nativeEnterDirectory);
   QObject::connect(obj, &QPlatformFileDialogHelper::filterSelected,   d, &QFileDialog::filterSelected);

   obj->setOptions(options);

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
   *cs_internal_lastVisitedDir() = dir;
}

void QFileDialogPrivate::updateLookInLabel()
{
   if (options->isLabelExplicitlySet(QPlatformFileDialogOptions::LookIn)) {
      setLabelTextControl(QFileDialog::LookIn, options->labelText(QPlatformFileDialogOptions::LookIn));
   }
}

void QFileDialogPrivate::updateFileNameLabel()
{
   if (options->isLabelExplicitlySet(QPlatformFileDialogOptions::FileName)) {
      setLabelTextControl(QFileDialog::FileName, options->labelText(QPlatformFileDialogOptions::FileName));

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
   if (options->isLabelExplicitlySet(QPlatformFileDialogOptions::FileType)) {
      setLabelTextControl(QFileDialog::FileType, options->labelText(QPlatformFileDialogOptions::FileType));
   }
}

void QFileDialogPrivate::updateOkButtonText(bool saveAsOnFolder)
{
   Q_Q(QFileDialog);

   // 'Save as' at a folder: Temporarily change to "Open".

   if (saveAsOnFolder) {
      setLabelTextControl(QFileDialog::Accept, QFileDialog::tr("&Open"));

   } else if (options->isLabelExplicitlySet(QPlatformFileDialogOptions::Accept)) {
      setLabelTextControl(QFileDialog::Accept, options->labelText(QPlatformFileDialogOptions::Accept));
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
   if (options->isLabelExplicitlySet(QPlatformFileDialogOptions::Reject)) {
      setLabelTextControl(QFileDialog::Reject, options->labelText(QPlatformFileDialogOptions::Reject));
   }
}

void QFileDialogPrivate::retranslateStrings()
{
   Q_Q(QFileDialog);

   // widgets
   if (defaultFileTypes) {
      q->setNameFilter(QFileDialog::tr("All Files (*)"));
   }

   if (! usingWidgets()) {
      return;
   }

   QList<QAction *> newActions = qFileDialogUi->treeView->header()->actions();
   QAbstractItemModel *abstractModel = model;

#ifndef QT_NO_PROXYMODEL
   if (proxyModel) {
      abstractModel = proxyModel;
   }
#endif

   int total = qMin(abstractModel->columnCount(QModelIndex()), newActions.count() + 1);

   for (int i = 1; i < total; ++i) {
      newActions.at(i - 1)->setText(QFileDialog::tr("Show ") + abstractModel->headerData(i, Qt::Horizontal,
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
   // do not use Q_Q, this method is called from ~QDialog
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
   return ! nativeDialogInUse && qFileDialogUi != nullptr;
}

void QFileDialogPrivate::_q_goToUrl(const QUrl &url)
{
   //The shortcut in the side bar may have a parent that is not fetched yet (e.g. an hidden file)
   //so we force the fetching
   QFileSystemModelPrivate::QFileSystemNode *node = model->d_func()->node(url.toLocalFile(), true);
   QModelIndex idx =  model->d_func()->index(node);
   _q_enterDirectory(idx);
}

#ifdef Q_OS_UNIX
QString cs_tildeExpansion(const QString &path, bool *expanded = nullptr)
{
   if (expanded != nullptr) {
      *expanded = false;
   }

   if (! path.startsWith('~')) {
      return path;
   }

   QString retval = path;

   QStringList tokens = retval.split(QDir::separator());
   if (tokens.first() == "~") {
      retval.replace(0, 1, QDir::homePath());

   } else {
      QString userName = tokens.first();
      userName.remove(0, 1);

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && ! defined(Q_OS_OPENBSD)
      passwd pw;
      passwd *tmpPw;
      char buf[200];

      const int bufSize = sizeof(buf);
      int err = getpwnam_r(userName.constData(), &pw, buf, bufSize, &tmpPw);

      if (err || ! tmpPw) {
         return retval;
      }

      const QString homePath = QString::fromUtf8(pw.pw_dir);

#else
      passwd *pw = getpwnam(userName.constData());

      if (! pw) {
         return ret;
      }
      const QString homePath = QString::frommUtf8(pw->pw_dir);
#endif

      retval.replace(0, tokens.first().length(), homePath);
   }

   if (expanded != nullptr) {
      *expanded = true;
   }

   return retval;
}
#endif

QStringList QFileDialogPrivate::typedFiles() const
{
#ifdef Q_OS_UNIX
   Q_Q(const QFileDialog);
#endif

   QStringList files;
   QString editText = lineEdit()->text();

   if (! editText.contains('"')) {

#ifdef Q_OS_UNIX
      const QString prefix = q->directory().absolutePath() + QDir::separator();

      if (QFile::exists(prefix + editText)) {
         files << editText;
      } else {
         files << cs_tildeExpansion(editText);
      }
#else
      files << editText;
#endif

   } else {
      // " is used to separate files like so: "file1" "file2" "file3" ...
      // ### need escape character for filenames with quotes (")
      QStringList tokens = editText.split('\"');

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
            files << cs_tildeExpansion(token);
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

      if (! defaultSuffix.isEmpty() && ! info.isDir() && name.lastIndexOf('.') == -1) {
         name += QChar('.') + defaultSuffix;
      }

      if (info.isAbsolute()) {
         files.append(name);
      } else {
         // path should only have CS path separators
         // check is needed since we might be at the root directory
         // and on Windows it already ends with slash.
         QString path = rootPath();

         if (! path.endsWith('/')) {
            path += '/';
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

      if (!defaultSuffix.isEmpty() && ! url.path().endsWith('/') && url.path().lastIndexOf('.') == -1) {
         url.setPath(url.path() + QChar('.') + defaultSuffix);
      }

      urls.append(url);
   }

   return urls;
}

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

   if (::GetVolumeInformation(reinterpret_cast<const wchar_t *>(&tmp[0]), nullptr, 0, nullptr,
                  &maxLength, nullptr, nullptr, 0) == false) {
      return -1;
   }

   return maxLength;

#endif
   return -1;
}

void QFileDialogPrivate::setRootIndex(const QModelIndex &index) const
{
   Q_ASSERT(index.isValid() ? index.model() == model : true);
   QModelIndex idx = mapFromSource(index);
   qFileDialogUi->treeView->setRootIndex(idx);
   qFileDialogUi->listView->setRootIndex(idx);
}

QModelIndex QFileDialogPrivate::select(const QModelIndex &index) const
{
   Q_ASSERT(index.isValid() ? index.model() == model : true);

   QModelIndex idx = mapFromSource(index);
   if (idx.isValid() && !qFileDialogUi->listView->selectionModel()->isSelected(idx))
      qFileDialogUi->listView->selectionModel()->select(idx,
         QItemSelectionModel::Select | QItemSelectionModel::Rows);
   return idx;
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

      case QFileDialog::DialogLabelCount:
         // do nothing
         break;
   }
}

static inline QUrl _qt_get_directory(const QUrl &url)
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
   if (! url.isEmpty()) {

      QUrl directory = _qt_get_directory(url);
      if (!directory.isEmpty()) {
         return directory;
      }
   }
   QUrl directory = _qt_get_directory(*cs_internal_lastVisitedDir());

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

#ifndef QT_NO_SETTINGS
void QFileDialogPrivate::saveSettings()
{
   Q_Q(QFileDialog);
   QSettings settings(QSettings::UserScope, "CsProject");
   settings.beginGroup("FileDialog");

   if (usingWidgets()) {
      settings.setValue("sidebarWidth", qFileDialogUi->splitter->sizes().first());
      settings.setValue("shortcuts", QUrl::toStringList(qFileDialogUi->sidebar->urls()));
      settings.setValue("treeViewHeader", qFileDialogUi->treeView->header()->saveState());
   }

   QStringList historyUrls;
   const QStringList history = q->history();

   for (const QString &path : history) {
      historyUrls << QUrl::fromLocalFile(path).toString();
   }

   settings.setValue("history",     historyUrls);
   settings.setValue("lastVisited", cs_internal_lastVisitedDir()->toString());

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
   q->setDirectoryUrl(cs_internal_lastVisitedDir()->isEmpty() ? settings.value("lastVisited").toUrl() : *cs_internal_lastVisitedDir());

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

   return restoreWidgetState(history, settings.value("sidebarWidth", -1).toInt());
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

   QList<QAction *> newActions = headerView->actions();
   QAbstractItemModel *abstractModel = model;

#ifndef QT_NO_PROXYMODEL
   if (proxyModel) {
      abstractModel = proxyModel;
   }
#endif

   int total = qMin(abstractModel->columnCount(QModelIndex()), newActions.count() + 1);
   for (int i = 1; i < total; ++i) {
      newActions.at(i - 1)->setChecked(!headerView->isSectionHidden(i));
   }

   return true;
}

void QFileDialogPrivate::init(const QUrl &directory, const QString &nameFilter, const QString &caption)
{
   Q_Q(QFileDialog);

   if (!caption.isEmpty()) {
      useDefaultCaption = false;
      setWindowTitle = caption;
      q->setWindowTitle(caption);
   }

   q->setAcceptMode(QFileDialog::AcceptOpen);
   nativeDialogInUse = (canBeNativeDialog() && platformFileDialogHelper() != nullptr);

   if (! nativeDialogInUse) {
      createWidgets();
   }

   q->setFileMode(QFileDialog::AnyFile);

   if (! nameFilter.isEmpty()) {
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
   model->setObjectName("qt_filesystem_model");

   if (QPlatformFileDialogHelper *helper = platformFileDialogHelper()) {
      model->setNameFilterDisables(helper->defaultNameFilterDisables());
   } else {
      model->setNameFilterDisables(false);
   }

   if (nativeDialogInUse) {
      deletePlatformHelper();
   }

   model->d_func()->disableRecursiveSort = true;
   QFileDialog::connect(model, &QFileSystemModel::fileRenamed,     q, &QFileDialog::_q_fileRenamed);
   QFileDialog::connect(model, &QFileSystemModel::rootPathChanged, q, &QFileDialog::_q_pathChanged);
   QFileDialog::connect(model, &QFileSystemModel::rowsInserted,    q, &QFileDialog::_q_rowsInserted);

   model->setReadOnly(false);

   qFileDialogUi.reset(new Ui_QFileDialog());
   qFileDialogUi->setupUi(q);

   QList<QUrl> initialBookmarks;
   initialBookmarks << QUrl("file:") << QUrl::fromLocalFile(QDir::homePath());

   qFileDialogUi->sidebar->setModelAndUrls(model, initialBookmarks);
   QFileDialog::connect(qFileDialogUi->sidebar, &QSidebar::goToUrl, q, &QFileDialog::_q_goToUrl);

   QObject::connect(qFileDialogUi->buttonBox, &QDialogButtonBox::accepted, q, &QFileDialog::accept);
   QObject::connect(qFileDialogUi->buttonBox, &QDialogButtonBox::rejected, q, &QFileDialog::reject);

   qFileDialogUi->lookInCombo->setFileDialogPrivate(this);
   QObject::connect(qFileDialogUi->lookInCombo, cs_mp_cast<const QString &>(&QComboBox::activated), q, &QFileDialog::_q_goToDirectory);

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

   QObject::connect(qFileDialogUi->fileNameEdit, &QLineEdit::textChanged,   q, &QFileDialog::_q_autoCompleteFileName);
   QObject::connect(qFileDialogUi->fileNameEdit, &QLineEdit::textChanged,   q, &QFileDialog::_q_updateOkButton);
   QObject::connect(qFileDialogUi->fileNameEdit, &QLineEdit::returnPressed, q, &QFileDialog::accept);

   // filetype
   qFileDialogUi->fileTypeCombo->setDuplicatesEnabled(false);
   qFileDialogUi->fileTypeCombo->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
   qFileDialogUi->fileTypeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

   QObject::connect(qFileDialogUi->fileTypeCombo, cs_mp_cast<int>(&QComboBox::activated), q, &QFileDialog::_q_useNameFilter);
   QObject::connect(qFileDialogUi->fileTypeCombo, cs_mp_cast<const QString &>(&QComboBox::activated), q, &QFileDialog::filterSelected);

   qFileDialogUi->listView->setFileDialogPrivate(this);
   qFileDialogUi->listView->setModel(model);

   QObject::connect(qFileDialogUi->listView, &QListView::activated,                  q, &QFileDialog::_q_enterDirectory);
   QObject::connect(qFileDialogUi->listView, &QListView::customContextMenuRequested, q, &QFileDialog::_q_showContextMenu);

#ifndef QT_NO_SHORTCUT
   QShortcut *shortcut = new QShortcut(qFileDialogUi->listView);
   shortcut->setKey(QKeySequence("Delete"));
   QObject::connect(shortcut, &QShortcut::activated, q, &QFileDialog::_q_deleteCurrent);
#endif

   qFileDialogUi->treeView->setFileDialogPrivate(this);
   qFileDialogUi->treeView->setModel(model);

   QHeaderView *treeHeader = qFileDialogUi->treeView->header();
   QFontMetrics fm(q->font());

   treeHeader->resizeSection(0, fm.width(QString("wwwwwwwwwwwwwwwwwwwwwwwwww")));
   treeHeader->resizeSection(1, fm.width(QString("128.88 GB")));
   treeHeader->resizeSection(2, fm.width(QString("mp3Folder")));
   treeHeader->resizeSection(3, fm.width(QString("10/29/81 02:02PM")));
   treeHeader->setContextMenuPolicy(Qt::ActionsContextMenu);

   QActionGroup *showActionGroup = new QActionGroup(q);
   showActionGroup->setExclusive(false);
   QObject::connect(showActionGroup, &QActionGroup::triggered, q, &QFileDialog::_q_showHeader);

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

   QObject::connect(qFileDialogUi->treeView, &QTreeView::activated,                  q, &QFileDialog::_q_enterDirectory);
   QObject::connect(qFileDialogUi->treeView, &QTreeView::customContextMenuRequested, q, &QFileDialog::_q_showContextMenu);

#ifndef QT_NO_SHORTCUT
   shortcut = new QShortcut(qFileDialogUi->treeView);
   shortcut->setKey(QKeySequence("Delete"));
   QObject::connect(shortcut, &QShortcut::activated, q, &QFileDialog::_q_deleteCurrent);
#endif

   // Selections
   QItemSelectionModel *selections = qFileDialogUi->listView->selectionModel();
   QObject::connect(selections, &QItemSelectionModel::selectionChanged, q, &QFileDialog::_q_selectionChanged);
   QObject::connect(selections, &QItemSelectionModel::currentChanged,   q, &QFileDialog::_q_currentChanged);

   qFileDialogUi->splitter->setStretchFactor(qFileDialogUi->splitter->indexOf(qFileDialogUi->splitter->widget(1)),
      QSizePolicy::Expanding);

   createToolButtons();
   createMenuActions();

#ifndef QT_NO_SETTINGS
   // Try to restore from the FileDialog settings group,
   // if it fails, fall back to the older QByteArray serialized settings
   if (! restoreFromSettings()) {
      const QSettings settings(QSettings::UserScope, "CsProject");
      q->restoreState(settings.value("CS/filedialog").toByteArray());
   }
#endif

   // Initial widget states from options
   q->setFileMode(static_cast<QFileDialog::FileMode>(options->fileMode()));
   q->setAcceptMode(static_cast<QFileDialog::AcceptMode>(options->acceptMode()));
   q->setViewMode(static_cast<QFileDialog::ViewMode>(options->viewMode()));
   q->setOptions(static_cast<QFileDialog::FileDialogOptions>(static_cast<int>(options->options())));

   if (! options->sidebarUrls().isEmpty()) {
      q->setSidebarUrls(options->sidebarUrls());
   }

   q->setDirectoryUrl(options->initialDirectory());

#ifndef QT_NO_MIMETYPE
   if (! options->mimeTypeFilters().isEmpty()) {
      q->setMimeTypeFilters(options->mimeTypeFilters());

   } else {
#endif

      if (! options->nameFilters().isEmpty()) {
         q->setNameFilters(options->nameFilters());
      }

#ifndef QT_NO_MIMETYPE
   }
#endif

   q->selectNameFilter(options->initiallySelectedNameFilter());
   q->setDefaultSuffix(options->defaultSuffix());
   q->setHistory(options->history());

   if (options->initiallySelectedFiles().count() == 1) {
      q->selectFile(options->initiallySelectedFiles().first().fileName());
   }

   for (const QUrl &url : options->initiallySelectedFiles()) {
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

void QFileDialogPrivate::createToolButtons()
{
   Q_Q(QFileDialog);

   qFileDialogUi->backButton->setIcon(q->style()->standardIcon(QStyle::SP_ArrowBack, nullptr, q));
   qFileDialogUi->backButton->setAutoRaise(true);
   qFileDialogUi->backButton->setEnabled(false);

   qFileDialogUi->forwardButton->setIcon(q->style()->standardIcon(QStyle::SP_ArrowForward, nullptr, q));
   qFileDialogUi->forwardButton->setAutoRaise(true);
   qFileDialogUi->forwardButton->setEnabled(false);

   qFileDialogUi->toParentButton->setIcon(q->style()->standardIcon(QStyle::SP_FileDialogToParent, nullptr, q));
   qFileDialogUi->toParentButton->setAutoRaise(true);
   qFileDialogUi->toParentButton->setEnabled(false);

   qFileDialogUi->listModeButton->setIcon(q->style()->standardIcon(QStyle::SP_FileDialogListView, nullptr, q));
   qFileDialogUi->listModeButton->setAutoRaise(true);
   qFileDialogUi->listModeButton->setDown(true);

   qFileDialogUi->detailModeButton->setIcon(q->style()->standardIcon(QStyle::SP_FileDialogDetailedView, nullptr, q));
   qFileDialogUi->detailModeButton->setAutoRaise(true);

   QSize toolSize(qFileDialogUi->fileNameEdit->sizeHint().height(), qFileDialogUi->fileNameEdit->sizeHint().height());

   qFileDialogUi->backButton->setFixedSize(toolSize);
   qFileDialogUi->listModeButton->setFixedSize(toolSize);
   qFileDialogUi->detailModeButton->setFixedSize(toolSize);
   qFileDialogUi->forwardButton->setFixedSize(toolSize);
   qFileDialogUi->toParentButton->setFixedSize(toolSize);

   qFileDialogUi->newFolderButton->setIcon(q->style()->standardIcon(QStyle::SP_FileDialogNewFolder, nullptr, q));
   qFileDialogUi->newFolderButton->setFixedSize(toolSize);
   qFileDialogUi->newFolderButton->setAutoRaise(true);
   qFileDialogUi->newFolderButton->setEnabled(false);

   QObject::connect(qFileDialogUi->backButton,       &QToolButton::clicked, q, &QFileDialog::_q_navigateBackward);
   QObject::connect(qFileDialogUi->forwardButton,    &QToolButton::clicked, q, &QFileDialog::_q_navigateForward);
   QObject::connect(qFileDialogUi->toParentButton,   &QToolButton::clicked, q, &QFileDialog::_q_navigateToParent);
   QObject::connect(qFileDialogUi->listModeButton,   &QToolButton::clicked, q, &QFileDialog::_q_showListView);
   QObject::connect(qFileDialogUi->detailModeButton, &QToolButton::clicked, q, &QFileDialog::_q_showDetailsView);
   QObject::connect(qFileDialogUi->newFolderButton,  &QToolButton::clicked, q, &QFileDialog::_q_createDirectory);
}

void QFileDialogPrivate::createMenuActions()
{
   Q_Q(QFileDialog);

   QAction *goHomeAction = new QAction(q);

#ifndef QT_NO_SHORTCUT
   goHomeAction->setShortcut(Qt::ControlModifier + Qt::Key_H + Qt::ShiftModifier);
#endif

   QObject::connect(goHomeAction, &QAction::triggered, q, &QFileDialog::_q_goHome);
   q->addAction(goHomeAction);

   // ### TODO add Desktop & Computer actions

   QAction *goToParent = new QAction(q);
   goToParent->setObjectName("qt_goto_parent_action");

#ifndef QT_NO_SHORTCUT
   goToParent->setShortcut(Qt::ControlModifier + Qt::UpArrow);
#endif

   QObject::connect(goToParent, &QAction::triggered, q, &QFileDialog::_q_navigateToParent);
   q->addAction(goToParent);

   renameAction = new QAction(q);
   renameAction->setEnabled(false);
   renameAction->setObjectName("qt_rename_action");

   deleteAction = new QAction(q);
   deleteAction->setEnabled(false);
   deleteAction->setObjectName("qt_delete_action");

   showHiddenAction = new QAction(q);
   showHiddenAction->setObjectName("qt_show_hidden_action");
   showHiddenAction->setCheckable(true);

   newFolderAction = new QAction(q);
   newFolderAction->setObjectName("qt_new_folder_action");

   QObject::connect(renameAction,     &QAction::triggered, q, &QFileDialog::_q_renameCurrent);
   QObject::connect(deleteAction,     &QAction::triggered, q, &QFileDialog::_q_deleteCurrent);
   QObject::connect(showHiddenAction, &QAction::triggered, q, &QFileDialog::_q_showHidden);
   QObject::connect(newFolderAction,  &QAction::triggered, q, &QFileDialog::_q_createDirectory);
}

void QFileDialogPrivate::_q_goHome()
{
   Q_Q(QFileDialog);
   q->setDirectory(QDir::homePath());
}

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

void QFileDialogPrivate::_q_navigateBackward()
{
   Q_Q(QFileDialog);

   if (! currentHistory.isEmpty() && currentHistoryLocation > 0) {
      --currentHistoryLocation;
      QString previousHistory = currentHistory.at(currentHistoryLocation);
      q->setDirectory(previousHistory);
   }
}

void QFileDialogPrivate::_q_navigateForward()
{
   Q_Q(QFileDialog);
   if (!currentHistory.isEmpty() && currentHistoryLocation < currentHistory.size() - 1) {
      ++currentHistoryLocation;
      QString nextHistory = currentHistory.at(currentHistoryLocation);
      q->setDirectory(nextHistory);
   }
}

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

void QFileDialogPrivate::_q_showContextMenu(const QPoint &position)
{
#ifndef QT_NO_MENU

   Q_Q(QFileDialog);

   QAbstractItemView *view = nullptr;
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
   if (text.startsWith("//") || text.startsWith('\\')) {
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

   if (lineEditText.startsWith("//") || lineEditText.startsWith('\\')) {
      button->setEnabled(true);
      updateOkButtonText();
      return;
   }

   if (files.isEmpty()) {
      enableButton = false;

   } else if (lineEditText == "..") {
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
               fileDir = fn.mid(0, fn.lastIndexOf('/'));
               fileName = fn.mid(fileDir.length() + 1);
            }

            if (lineEditText.contains("..")) {
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
         || q->fileMode() != QFileDialog::ExistingFiles || !(QGuiApplication::keyboardModifiers() & Qt::ControlModifier)) {
         q->accept();
      }
   }
}

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
         allFiles.replace(i, QString('"' + allFiles.at(i) + '"'));
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

   for (const QUrl &file : files) {
      if (file.isLocalFile()) {
         localFiles.append(file.toLocalFile());
      }
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

   if (!directory.isEmpty()) {
      // Windows native dialogs occasionally emit signals with empty strings.
      *cs_internal_lastVisitedDir() = directory;

      if (directory.isLocalFile()) {
         emit q->directoryEntered(directory.toLocalFile());
      }
   }
}

#endif // QT_NO_FILEDIALOG