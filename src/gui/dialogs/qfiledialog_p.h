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

#ifndef QFILEDIALOG_P_H
#define QFILEDIALOG_P_H

#ifndef QT_NO_FILEDIALOG

#include <qfiledialog.h>
#include <qdialog_p.h>
#include <qplatformdefs.h>
#include <qfilesystemmodel_p.h>
#include <qlistview.h>
#include <qtreeview.h>
#include <qcombobox.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qurl.h>
#include <qstackedwidget.h>
#include <qdialogbuttonbox.h>
#include <qabstractproxymodel.h>
#include <qcompleter.h>
#include <qpointer.h>
#include <qdebug.h>
#include <qsidebar_p.h>
#include <qfscompleter_p.h>
#include <qguiplatformplugin_p.h>

#if defined (Q_OS_UNIX)
#include <unistd.h>
#endif

QT_BEGIN_NAMESPACE

class QFileDialogListView;
class QFileDialogTreeView;
class QFileDialogLineEdit;
class QGridLayout;
class QCompleter;
class QHBoxLayout;
class Ui_QFileDialog;

struct QFileDialogArgs {
   QFileDialogArgs() : parent(0), mode(QFileDialog::AnyFile) {}

   QWidget *parent;
   QString caption;
   QString directory;
   QString selection;
   QString filter;
   QFileDialog::FileMode mode;
   QFileDialog::Options options;
};

#define UrlRole (Qt::UserRole + 1)

class QFileDialogPrivate : public QDialogPrivate
{
   Q_DECLARE_PUBLIC(QFileDialog)

 public:
   QFileDialogPrivate();

   void createToolButtons();
   void createMenuActions();
   void createWidgets();

   void init(const QString &directory = QString(), const QString &nameFilter = QString(),
             const QString &caption = QString());
   bool itemViewKeyboardEvent(QKeyEvent *event);
   QString getEnvironmentVariable(const QString &string);
   static QString workingDirectory(const QString &path);
   static QString initialSelection(const QString &path);
   QStringList typedFiles() const;
   QStringList addDefaultSuffixToFiles(const QStringList filesToFix) const;
   bool removeDirectory(const QString &path);

   inline QModelIndex mapToSource(const QModelIndex &index) const;
   inline QModelIndex mapFromSource(const QModelIndex &index) const;
   inline QModelIndex rootIndex() const;
   inline void setRootIndex(const QModelIndex &index) const;
   inline QModelIndex select(const QModelIndex &index) const;
   inline QString rootPath() const;

   QLineEdit *lineEdit() const;

   int maxNameLength(const QString &path) {

#if defined(Q_OS_UNIX)
      return ::pathconf(QFile::encodeName(path).data(), _PC_NAME_MAX);

#elif defined(Q_OS_WIN)
      DWORD maxLength;
      QString drive = path.left(3);

      if (::GetVolumeInformation(reinterpret_cast<const wchar_t *>(drive.utf16()), NULL, 0, NULL, &maxLength, NULL, NULL, 0) == FALSE) {
         return -1;
      }

      return maxLength;

#else
      Q_UNUSED(path);
#endif

      return -1;
   }

   QString basename(const QString &path) const {
      int separator = QDir::toNativeSeparators(path).lastIndexOf(QDir::separator());
      if (separator != -1) {
         return path.mid(separator + 1);
      }
      return path;
   }

   QDir::Filters filterForMode(QDir::Filters filters) const {
      if (fileMode == QFileDialog::DirectoryOnly) {
         filters |= QDir::Drives | QDir::AllDirs | QDir::Dirs;
         filters &= ~QDir::Files;
      } else {
         filters |= QDir::Drives | QDir::AllDirs | QDir::Files | QDir::Dirs;
      }
      return filters;
   }

   QAbstractItemView *currentView() const;

   static inline QString toInternal(const QString &path) {
#if defined(Q_OS_WIN)
      QString n(path);

      for (int i = 0; i < (int)n.length(); ++i)
         if (n[i] == QLatin1Char('\\')) {
            n[i] = QLatin1Char('/');
         }

      return n;

#else // the compile should optimize this
      return path;
#endif

   }

   void setLastVisitedDirectory(const QString &dir);
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

   QFileDialog::FileMode fileMode;
   QFileDialog::AcceptMode acceptMode;
   bool confirmOverwrite;
   QString defaultSuffix;
   QString setWindowTitle;

   QStringList currentHistory;
   int currentHistoryLocation;

   QAction *renameAction;
   QAction *deleteAction;
   QAction *showHiddenAction;
   QAction *newFolderAction;

   bool useDefaultCaption;
   bool defaultFileTypes;
   bool fileNameLabelExplicitlySat;
   QStringList nameFilters;

   // Members for using native dialogs:
   bool nativeDialogInUse;

   // setVisible_sys returns true if it ends up showing a native
   // dialog. Returning false means that a non-native dialog must be used instead
   bool canBeNativeDialog();
   bool setVisible_sys(bool visible);
   void deleteNativeDialog_sys();
   QDialog::DialogCode dialogResultCode_sys();

   void setDirectory_sys(const QString &directory);
   QString directory_sys() const;
   void selectFile_sys(const QString &filename);
   QStringList selectedFiles_sys() const;
   void setFilter_sys();
   void setNameFilters_sys(const QStringList &filters);
   void selectNameFilter_sys(const QString &filter);
   QString selectedNameFilter_sys() const;
   

#if defined(Q_OS_MAC)
   void *mDelegate;

   bool showCocoaFilePanel();
   bool hideCocoaFilePanel();

   void createNSOpenSavePanelDelegate();
   void QNSOpenSavePanelDelegate_selectionChanged(const QString &newPath);
   void QNSOpenSavePanelDelegate_panelClosed(bool accepted);
   void QNSOpenSavePanelDelegate_directoryEntered(const QString &newDir);
   void QNSOpenSavePanelDelegate_filterSelected(int menuIndex);
   void _q_macRunNativeAppModalPanel();
   void mac_nativeDialogModalHelp();
#endif

   QScopedPointer<Ui_QFileDialog> qFileDialogUi;

   QString acceptLabel;

   QPointer<QObject> receiverToDisconnectOnClose;
   QByteArray memberToDisconnectOnClose;
   QByteArray signalToDisconnectOnClose;

   QFileDialog::Options opts;

   ~QFileDialogPrivate();

 private:
   Q_DISABLE_COPY(QFileDialogPrivate)
};

class QFileDialogLineEdit : public QLineEdit
{
 public:
   QFileDialogLineEdit(QWidget *parent = nullptr) : QLineEdit(parent), hideOnEsc(false), d_ptr(0) {}

   void setFileDialogPrivate(QFileDialogPrivate *d_pointer) {
      d_ptr = d_pointer;
   }

   void keyPressEvent(QKeyEvent *e) override;
   bool hideOnEsc;

 private:
   QFileDialogPrivate *d_ptr;
};

class QFileDialogComboBox : public QComboBox
{
 public:
   QFileDialogComboBox(QWidget *parent = nullptr) : QComboBox(parent), urlModel(0) {}
   void setFileDialogPrivate(QFileDialogPrivate *d_pointer);
   void showPopup() override;
   void setHistory(const QStringList &paths);

   QStringList history() const {
      return m_history;
   }

   void paintEvent(QPaintEvent *) override;

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
   void keyPressEvent(QKeyEvent *e) override;

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
   void keyPressEvent(QKeyEvent *e) override;

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
   return model->rootPath();
}

#ifndef Q_OS_MAC
// Dummies for platforms which do not use native dialogs
inline void QFileDialogPrivate::deleteNativeDialog_sys()
{
   qt_guiPlatformPlugin()->fileDialogDelete(q_func());
}

inline bool QFileDialogPrivate::setVisible_sys(bool visible)
{
   return qt_guiPlatformPlugin()->fileDialogSetVisible(q_func(), visible);
}

inline QDialog::DialogCode QFileDialogPrivate::dialogResultCode_sys()
{
   return qt_guiPlatformPlugin()->fileDialogResultCode(q_func());
}

inline void QFileDialogPrivate::setDirectory_sys(const QString &directory)
{
   qt_guiPlatformPlugin()->fileDialogSetDirectory(q_func(), directory);
}

inline QString QFileDialogPrivate::directory_sys() const
{
   return qt_guiPlatformPlugin()->fileDialogDirectory(q_func());
}

inline void QFileDialogPrivate::selectFile_sys(const QString &filename)
{
   qt_guiPlatformPlugin()->fileDialogSelectFile(q_func(), filename);
}

inline QStringList QFileDialogPrivate::selectedFiles_sys() const
{
   return qt_guiPlatformPlugin()->fileDialogSelectedFiles(q_func());
}

inline void QFileDialogPrivate::setFilter_sys()
{
   qt_guiPlatformPlugin()->fileDialogSetFilter(q_func());
}

inline void QFileDialogPrivate::setNameFilters_sys(const QStringList &filters)
{
   qt_guiPlatformPlugin()->fileDialogSetNameFilters(q_func(), filters);
}

inline void QFileDialogPrivate::selectNameFilter_sys(const QString &filter)
{
   qt_guiPlatformPlugin()->fileDialogSelectNameFilter(q_func(), filter);
}

inline QString QFileDialogPrivate::selectedNameFilter_sys() const
{
   return qt_guiPlatformPlugin()->fileDialogSelectedNameFilter(q_func());
}

#endif

QT_END_NAMESPACE

#endif // QT_NO_FILEDIALOG

#endif // QFILEDIALOG_P_H
