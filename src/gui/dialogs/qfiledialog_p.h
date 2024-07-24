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

#ifndef QFILEDIALOG_P_H
#define QFILEDIALOG_P_H

#include <qglobal.h>

#ifndef QT_NO_FILEDIALOG

#include <qfiledialog.h>

#include <qabstractproxymodel.h>
#include <qcombobox.h>
#include <qcompleter.h>
#include <qdebug.h>
#include <qdialogbuttonbox.h>
#include <qevent.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qplatformdefs.h>
#include <qpointer.h>
#include <qstackedwidget.h>
#include <qtoolbutton.h>
#include <qtreeview.h>
#include <qurl.h>

#include <qdialog_p.h>
#include <qfilesystemmodel_p.h>
#include <qfscompleter_p.h>
#include <qsidebar_p.h>

#if defined (Q_OS_UNIX)
#include <unistd.h>
#endif

class QCompleter;
class QFileDialogLineEdit;
class QFileDialogListView;
class QFileDialogTreeView;
class QGridLayout;
class QHBoxLayout;
class QPlatformDialogHelper;
class Ui_QFileDialog;

struct QFileDialogArgs {
   QFileDialogArgs()
      : parent(nullptr), mode(QFileDialog::AnyFile)
   {
   }

   QWidget *parent;
   QString caption;
   QUrl directory;
   QString selection;
   QString filter;
   QFileDialog::FileMode mode;
   QFileDialog::FileDialogOptions options;
};

#define UrlRole (Qt::UserRole + 1)

class QFileDialogPrivate : public QDialogPrivate
{
   Q_DECLARE_PUBLIC(QFileDialog)

 public:
   QFileDialogPrivate();

   QFileDialogPrivate(const QFileDialogPrivate &) = delete;
   QFileDialogPrivate &operator=(const QFileDialogPrivate &) = delete;

   ~QFileDialogPrivate();

   QPlatformFileDialogHelper *platformFileDialogHelper() const {
      return static_cast<QPlatformFileDialogHelper *>(platformHelper());
   }

   void createToolButtons();
   void createMenuActions();
   void createWidgets();

   void init(const QUrl &directory = QUrl(), const QString &nameFilter = QString(),
      const QString &caption = QString());

   bool itemViewKeyboardEvent(QKeyEvent *event);
   QString getEnvironmentVariable(const QString &string);
   static QUrl workingDirectory(const QUrl &path);
   static QString initialSelection(const QUrl &path);

   QStringList typedFiles() const;
   QList<QUrl> userSelectedFiles() const;
   QStringList addDefaultSuffixToFiles(const QStringList &filesToFix) const;
   QList<QUrl> addDefaultSuffixToUrls(const QList<QUrl> &urlsToFix) const;

   bool removeDirectory(const QString &path);
   void setLabelTextControl(QFileDialog::DialogLabel label, const QString &text);
   inline void updateLookInLabel();
   void updateFileNameLabel();
   inline void updateFileTypeLabel();
   void updateOkButtonText(bool saveAsOnFolder = false);
   void updateCancelButtonText();

   inline QModelIndex mapToSource(const QModelIndex &index) const;
   inline QModelIndex mapFromSource(const QModelIndex &index) const;
   QModelIndex rootIndex() const;
   void setRootIndex(const QModelIndex &index) const;
   inline QModelIndex select(const QModelIndex &index) const;
   inline QString rootPath() const;

   QLineEdit *lineEdit() const;

   static int maxNameLength(const QString &path);

   QString basename(const QString &path) const {
      int separator = QDir::toNativeSeparators(path).lastIndexOf(QDir::separator());

      if (separator != -1) {
         return path.mid(separator + 1);
      }

      return path;
   }

   QDir::Filters filterForMode(QDir::Filters filters) const {
      const QFileDialog::FileMode fileMode = q_func()->fileMode();

      if (fileMode == QFileDialog::DirectoryOnly) {
         filters |= QDir::Drives | QDir::AllDirs | QDir::Dirs;
         filters &= ~QDir::Files;
      } else {
         filters |= QDir::Drives | QDir::AllDirs | QDir::Files | QDir::Dirs;
      }

      return filters;
   }

   QAbstractItemView *currentView() const;

   static QString toInternal(const QString &path) {

#if defined(Q_OS_WIN)
      QString n(path);
      n.replace('\\', '/');

      return n;

#else
      return path;

#endif

   }

   // setVisible_sys returns true if it ends up showing a native
   // dialog. Returning false means that a non-native dialog must be used instead
   bool canBeNativeDialog() const override;
   bool usingWidgets() const;

   void setDirectory_sys(const QUrl &directory);
   QUrl directory_sys() const;
   void selectFile_sys(const QUrl &filename);
   QList<QUrl> selectedFiles_sys() const;

   void setFilter_sys();
   void selectNameFilter_sys(const QString &filter);
   QString selectedNameFilter_sys() const;

#ifndef QT_NO_SETTINGS
   void saveSettings();
   bool restoreFromSettings();
#endif

   bool restoreWidgetState(QStringList &history, int splitterPosition);
   static void setLastVisitedDirectory(const QUrl &dir);
   void retranslateWindowTitle();
   void retranslateStrings();
   void emitFilesSelected(const QStringList &files);

   void _q_goHome();
   void _q_pathChanged(const QString &);
   void _q_navigateBackward();
   void _q_navigateForward();
   void _q_navigateToParent();
   void _q_createDirectory();
   void _q_showListView();
   void _q_showDetailsView();
   void _q_showContextMenu(const QPoint &position);
   void _q_renameCurrent();
   void _q_deleteCurrent();
   void _q_showHidden();
   void _q_showHeader(QAction *);
   void _q_updateOkButton();
   void _q_currentChanged(const QModelIndex &index);
   void _q_enterDirectory(const QModelIndex &index);
   void _q_emitUrlSelected(const QUrl &file);
   void _q_emitUrlsSelected(const QList<QUrl> &files);
   void _q_nativeCurrentChanged(const QUrl &file);
   void _q_nativeEnterDirectory(const QUrl &directory);
   void _q_goToDirectory(const QString &);
   void _q_useNameFilter(int index);
   void _q_selectionChanged();
   void _q_goToUrl(const QUrl &url);
   void _q_autoCompleteFileName(const QString &);
   void _q_rowsInserted(const QModelIndex &parent);
   void _q_fileRenamed(const QString &path, const QString &oldName, const QString &newName);

   // layout
#ifndef QT_NO_PROXYMODEL
   QAbstractProxyModel *proxyModel;
#endif

   // data
   QStringList watching;
   QFileSystemModel *model;

#ifndef QT_NO_FSCOMPLETER
   QFSCompleter *completer;
#endif

   QString setWindowTitle;

   QStringList currentHistory;
   int currentHistoryLocation;

   QAction *renameAction;
   QAction *deleteAction;
   QAction *showHiddenAction;
   QAction *newFolderAction;

   bool useDefaultCaption;
   bool defaultFileTypes;

   QScopedPointer<Ui_QFileDialog> qFileDialogUi;

   QString acceptLabel;

   QPointer<QObject> receiverToDisconnectOnClose;
   QString memberToDisconnectOnClose;
   QString signalToDisconnectOnClose;

   QSharedPointer<QPlatformFileDialogOptions> options;

   // Memory of what was read from QSettings in restoreState() in case widgets are not used
   QByteArray splitterState;
   QByteArray headerData;
   QList<QUrl> sidebarUrls;

 private:
   void initHelper(QPlatformDialogHelper *) override;
   void helperPrepareShow(QPlatformDialogHelper *) override;
   void helperDone(QDialog::DialogCode, QPlatformDialogHelper *) override;
};

class QFileDialogLineEdit : public QLineEdit
{
 public:
   QFileDialogLineEdit(QWidget *parent = nullptr)
      : QLineEdit(parent), d_ptr(nullptr)
   {
   }

   void setFileDialogPrivate(QFileDialogPrivate *d_pointer) {
      d_ptr = d_pointer;
   }

   void keyPressEvent(QKeyEvent *event) override;
   bool hideOnEsc;

 private:
   QFileDialogPrivate *d_ptr;
};

class QFileDialogComboBox : public QComboBox
{
 public:
   QFileDialogComboBox(QWidget *parent = nullptr)
      : QComboBox(parent), urlModel(nullptr)
   {
   }

   void setFileDialogPrivate(QFileDialogPrivate *d_pointer);
   void showPopup() override;
   void setHistory(const QStringList &paths);

   QStringList history() const {
      return m_history;
   }

   void paintEvent(QPaintEvent *event) override;

 private:
   QUrlModel *urlModel;
   QFileDialogPrivate *d_ptr;
   QStringList m_history;
};

class QFileDialogListView : public QListView
{
 public:
   QFileDialogListView(QWidget *parent = nullptr);
   void setFileDialogPrivate(QFileDialogPrivate *d_pointer);
   QSize sizeHint() const override;

 protected:
   void keyPressEvent(QKeyEvent *event) override;

 private:
   QFileDialogPrivate *d_ptr;
};

class QFileDialogTreeView : public QTreeView
{
 public:
   QFileDialogTreeView(QWidget *parent);
   void setFileDialogPrivate(QFileDialogPrivate *d_pointer);
   QSize sizeHint() const override;

 protected:
   void keyPressEvent(QKeyEvent *event) override;

 private:
   QFileDialogPrivate *d_ptr;
};

inline QModelIndex QFileDialogPrivate::mapToSource(const QModelIndex &index) const
{
#ifdef QT_NO_PROXYMODEL
   return index;
#else
   return proxyModel ? proxyModel->mapToSource(index) : index;
#endif
}

inline QModelIndex QFileDialogPrivate::mapFromSource(const QModelIndex &index) const
{
#ifdef QT_NO_PROXYMODEL
   return index;
#else
   return proxyModel ? proxyModel->mapFromSource(index) : index;
#endif
}

inline QString QFileDialogPrivate::rootPath() const
{
   return (model ? model->rootPath() : QString("/"));
}

inline void QFileDialogPrivate::setDirectory_sys(const QUrl &directory)
{
   QPlatformFileDialogHelper *helper = platformFileDialogHelper();
   if (! helper) {
      return;
   }
   if (helper->isSupportedUrl(directory)) {
      helper->setDirectory(directory);
   }
}

inline QUrl QFileDialogPrivate::directory_sys() const
{
   if (QPlatformFileDialogHelper *helper = platformFileDialogHelper()) {
      return helper->directory();
   }
   return QUrl();
}

inline void QFileDialogPrivate::selectFile_sys(const QUrl &filename)
{
   QPlatformFileDialogHelper *helper = platformFileDialogHelper();

   if (!helper) {
      return;
   }

   if (helper->isSupportedUrl(filename)) {
      helper->selectFile(filename);
   }
}

inline QList<QUrl> QFileDialogPrivate::selectedFiles_sys() const
{
   if (QPlatformFileDialogHelper *helper = platformFileDialogHelper()) {
      return helper->selectedFiles();
   }
   return QList<QUrl>();
}

inline void QFileDialogPrivate::setFilter_sys()
{
   if (QPlatformFileDialogHelper *helper = platformFileDialogHelper()) {
      helper->setFilter();
   }
}

inline void QFileDialogPrivate::selectNameFilter_sys(const QString &filter)
{
   if (QPlatformFileDialogHelper *helper = platformFileDialogHelper()) {
      helper->selectNameFilter(filter);
   }
}


inline QString QFileDialogPrivate::selectedNameFilter_sys() const
{
   if (QPlatformFileDialogHelper *helper = platformFileDialogHelper()) {
      return helper->selectedNameFilter();
   }
   return QString();
}

#endif // QT_NO_FILEDIALOG

#endif
