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

#ifndef QFILEDIALOG_H
#define QFILEDIALOG_H

#include <QtCore/qdir.h>
#include <QtCore/qstring.h>
#include <QtGui/qdialog.h>
#include <QtCore/qurl.h>
#include <QModelIndex>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_FILEDIALOG

class QItemSelection;
class QFileIconProvider;
class QFileDialogPrivate;
class QAbstractItemDelegate;
class QAbstractProxyModel;
struct QFileDialogArgs;

class Q_GUI_EXPORT QFileDialog : public QDialog
{
   GUI_CS_OBJECT(QFileDialog)

   GUI_CS_ENUM(ViewMode)
   GUI_CS_ENUM(FileMode)
   GUI_CS_ENUM(AcceptMode)
   GUI_CS_ENUM(Option)

   GUI_CS_FLAG(Option, Options)

   GUI_CS_PROPERTY_READ(viewMode, viewMode)
   GUI_CS_PROPERTY_WRITE(viewMode, setViewMode)
   GUI_CS_PROPERTY_READ(fileMode, fileMode)
   GUI_CS_PROPERTY_WRITE(fileMode, setFileMode)
   GUI_CS_PROPERTY_READ(acceptMode, acceptMode)
   GUI_CS_PROPERTY_WRITE(acceptMode, setAcceptMode)
   GUI_CS_PROPERTY_READ(readOnly, isReadOnly)
   GUI_CS_PROPERTY_WRITE(readOnly, setReadOnly)
   GUI_CS_PROPERTY_DESIGNABLE(readOnly, false)
   GUI_CS_PROPERTY_READ(resolveSymlinks, resolveSymlinks)
   GUI_CS_PROPERTY_WRITE(resolveSymlinks, setResolveSymlinks)
   GUI_CS_PROPERTY_DESIGNABLE(resolveSymlinks, false)
   GUI_CS_PROPERTY_READ(confirmOverwrite, confirmOverwrite)
   GUI_CS_PROPERTY_WRITE(confirmOverwrite, setConfirmOverwrite)
   GUI_CS_PROPERTY_DESIGNABLE(confirmOverwrite, false)
   GUI_CS_PROPERTY_READ(defaultSuffix, defaultSuffix)
   GUI_CS_PROPERTY_WRITE(defaultSuffix, setDefaultSuffix)
   GUI_CS_PROPERTY_READ(nameFilterDetailsVisible, isNameFilterDetailsVisible)
   GUI_CS_PROPERTY_WRITE(nameFilterDetailsVisible, setNameFilterDetailsVisible)
   GUI_CS_PROPERTY_DESIGNABLE(nameFilterDetailsVisible, false)
   GUI_CS_PROPERTY_READ(options, options)
   GUI_CS_PROPERTY_WRITE(options, setOptions)

 public:
   enum ViewMode { Detail, List };
   enum FileMode { AnyFile, ExistingFile, Directory, ExistingFiles, DirectoryOnly };
   enum AcceptMode { AcceptOpen, AcceptSave };
   enum DialogLabel { LookIn, FileName, FileType, Accept, Reject };

   // ### Rename to FileDialogOption and FileDialogOptions for Qt5
   enum Option {
      ShowDirsOnly          = 0x00000001,
      DontResolveSymlinks   = 0x00000002,
      DontConfirmOverwrite  = 0x00000004,
      DontUseSheet          = 0x00000008,
      DontUseNativeDialog   = 0x00000010,
      ReadOnly              = 0x00000020,
      HideNameFilterDetails = 0x00000040,
      ForceInitialDir_Win7  = 0x00000080,
      DontUseCustomDirectoryIcons = 0x00000100
   };
   using Options = QFlags<Option>;

   QFileDialog(QWidget *parent, Qt::WindowFlags f);
   explicit QFileDialog(QWidget *parent = nullptr,
                        const QString &caption = QString(),
                        const QString &directory = QString(),
                        const QString &filter = QString());
   ~QFileDialog();

   void setDirectory(const QString &directory);
   inline void setDirectory(const QDir &directory);
   QDir directory() const;

   void selectFile(const QString &filename);
   QStringList selectedFiles() const;

#ifdef QT_DEPRECATED
   QT_DEPRECATED void setFilter(const QString &filter);
   QT_DEPRECATED void setFilters(const QStringList &filters);
   QT_DEPRECATED QStringList filters() const;
   QT_DEPRECATED void selectFilter(const QString &filter);
   QT_DEPRECATED QString selectedFilter() const;
#endif

   void setNameFilterDetailsVisible(bool enabled);
   bool isNameFilterDetailsVisible() const;

   void setNameFilter(const QString &filter);
   void setNameFilters(const QStringList &filters);
   QStringList nameFilters() const;
   void selectNameFilter(const QString &filter);
   QString selectedNameFilter() const;

   QDir::Filters filter() const;
   void setFilter(QDir::Filters filters);

   void setViewMode(ViewMode mode);
   ViewMode viewMode() const;

   void setFileMode(FileMode mode);
   FileMode fileMode() const;

   void setAcceptMode(AcceptMode mode);
   AcceptMode acceptMode() const;

   void setReadOnly(bool enabled);
   bool isReadOnly() const;

   void setResolveSymlinks(bool enabled);
   bool resolveSymlinks() const;

   void setSidebarUrls(const QList<QUrl> &urls);
   QList<QUrl> sidebarUrls() const;

   QByteArray saveState() const;
   bool restoreState(const QByteArray &state);

   void setConfirmOverwrite(bool enabled);
   bool confirmOverwrite() const;

   void setDefaultSuffix(const QString &suffix);
   QString defaultSuffix() const;

   void setHistory(const QStringList &paths);
   QStringList history() const;

   void setItemDelegate(QAbstractItemDelegate *delegate);
   QAbstractItemDelegate *itemDelegate() const;

   void setIconProvider(QFileIconProvider *provider);
   QFileIconProvider *iconProvider() const;

   void setLabelText(DialogLabel label, const QString &text);
   QString labelText(DialogLabel label) const;

#ifndef QT_NO_PROXYMODEL
   void setProxyModel(QAbstractProxyModel *model);
   QAbstractProxyModel *proxyModel() const;
#endif

   void setOption(Option option, bool on = true);
   bool testOption(Option option) const;
   void setOptions(Options options);
   Options options() const;

   using QDialog::open;
   void open(QObject *receiver, const QString &member);

   void setVisible(bool visible) override;

   GUI_CS_SIGNAL_1(Public, void fileSelected(const QString &file))
   GUI_CS_SIGNAL_2(fileSelected, file)
   GUI_CS_SIGNAL_1(Public, void filesSelected(const QStringList &files))
   GUI_CS_SIGNAL_2(filesSelected, files)
   GUI_CS_SIGNAL_1(Public, void currentChanged(const QString &path))
   GUI_CS_SIGNAL_2(currentChanged, path)
   GUI_CS_SIGNAL_1(Public, void directoryEntered(const QString &directory))
   GUI_CS_SIGNAL_2(directoryEntered, directory)
   GUI_CS_SIGNAL_1(Public, void filterSelected(const QString &filter))
   GUI_CS_SIGNAL_2(filterSelected, filter)

   static QString getOpenFileName(QWidget *parent = nullptr, const QString &caption = QString(), const QString &dir = QString(),
                  const QString &filter = QString(), QString *selectedFilter = 0, Options options = 0);

   static QUrl getOpenFileUrl(QWidget *parent = nullptr, const QString &caption = QString(), const QUrl &dir = QUrl(),
                  const QString &filter = QString(), QString *selectedFilter = 0, Options options = 0,
                  const QStringList &supportedSchemes = QStringList());

   static QString getSaveFileName(QWidget *parent = nullptr, const QString &caption = QString(), const QString &dir = QString(),
                  const QString &filter = QString(), QString *selectedFilter = 0, Options options = 0);

   static QUrl getSaveFileUrl(QWidget *parent = nullptr, const QString &caption = QString(), const QUrl &dir = QUrl(),
                  const QString &filter = QString(), QString *selectedFilter = 0, Options options = 0,
                  const QStringList &supportedSchemes = QStringList());

   static QString getExistingDirectory(QWidget *parent = nullptr, const QString &caption = QString(),
                  const QString &dir = QString(), Options options = ShowDirsOnly);

   static QUrl getExistingDirectoryUrl(QWidget *parent = nullptr, const QString &caption = QString(), const QUrl &dir = QUrl(),
                  Options options = ShowDirsOnly, const QStringList &supportedSchemes = QStringList());

   static QStringList getOpenFileNames(QWidget *parent = nullptr, const QString &caption = QString(), const QString &dir = QString(),
                  const QString &filter = QString(), QString *selectedFilter = 0, Options options = 0);

   static QList<QUrl> getOpenFileUrls(QWidget *parent = nullptr, const QString &caption = QString(), const QUrl &dir = QUrl(),
                  const QString &filter = QString(), QString *selectedFilter = 0, Options options = 0,
                  const QStringList &supportedSchemes = QStringList());

 protected:
   QFileDialog(const QFileDialogArgs &args);
   void done(int result) override;
   void accept() override;
   void changeEvent(QEvent *e) override;

 private:
   Q_DECLARE_PRIVATE(QFileDialog)
   Q_DISABLE_COPY(QFileDialog)

   GUI_CS_SLOT_1(Private, void _q_pathChanged(const QString &un_named_arg1))
   GUI_CS_SLOT_2(_q_pathChanged)

   GUI_CS_SLOT_1(Private, void _q_navigateBackward())
   GUI_CS_SLOT_2(_q_navigateBackward)

   GUI_CS_SLOT_1(Private, void _q_navigateForward())
   GUI_CS_SLOT_2(_q_navigateForward)

   GUI_CS_SLOT_1(Private, void _q_navigateToParent())
   GUI_CS_SLOT_2(_q_navigateToParent)

   GUI_CS_SLOT_1(Private, void _q_createDirectory())
   GUI_CS_SLOT_2(_q_createDirectory)

   GUI_CS_SLOT_1(Private, void _q_showListView())
   GUI_CS_SLOT_2(_q_showListView)

   GUI_CS_SLOT_1(Private, void _q_showDetailsView())
   GUI_CS_SLOT_2(_q_showDetailsView)

   GUI_CS_SLOT_1(Private, void _q_showContextMenu(const QPoint &un_named_arg1))
   GUI_CS_SLOT_2(_q_showContextMenu)

   GUI_CS_SLOT_1(Private, void _q_renameCurrent())
   GUI_CS_SLOT_2(_q_renameCurrent)

   GUI_CS_SLOT_1(Private, void _q_deleteCurrent())
   GUI_CS_SLOT_2(_q_deleteCurrent)

   GUI_CS_SLOT_1(Private, void _q_showHidden())
   GUI_CS_SLOT_2(_q_showHidden)

   GUI_CS_SLOT_1(Private, void _q_updateOkButton())
   GUI_CS_SLOT_2(_q_updateOkButton)

   GUI_CS_SLOT_1(Private, void _q_currentChanged(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_currentChanged)

   GUI_CS_SLOT_1(Private, void _q_enterDirectory(const QModelIndex &index))
   GUI_CS_SLOT_2(_q_enterDirectory)

   GUI_CS_SLOT_1(Private, void _q_goToDirectory(const QString &path))
   GUI_CS_SLOT_2(_q_goToDirectory)

   GUI_CS_SLOT_1(Private, void _q_useNameFilter(int index))
   GUI_CS_SLOT_2(_q_useNameFilter)

   GUI_CS_SLOT_1(Private, void _q_selectionChanged())
   GUI_CS_SLOT_2(_q_selectionChanged)

   GUI_CS_SLOT_1(Private, void _q_goToUrl(const QUrl &url))
   GUI_CS_SLOT_2(_q_goToUrl)

   GUI_CS_SLOT_1(Private, void _q_goHome())
   GUI_CS_SLOT_2(_q_goHome)

   GUI_CS_SLOT_1(Private, void _q_showHeader(QAction *un_named_arg1))
   GUI_CS_SLOT_2(_q_showHeader)

   GUI_CS_SLOT_1(Private, void _q_autoCompleteFileName(const QString &text))
   GUI_CS_SLOT_2(_q_autoCompleteFileName)

   GUI_CS_SLOT_1(Private, void _q_rowsInserted(const QModelIndex &parent))
   GUI_CS_SLOT_2(_q_rowsInserted)

   GUI_CS_SLOT_1(Private, void _q_fileRenamed(const QString &path, const QString &oldName, const QString &newName))
   GUI_CS_SLOT_2(_q_fileRenamed)

#if defined(Q_OS_MAC)
   GUI_CS_SLOT_1(Private, void _q_macRunNativeAppModalPanel())
   GUI_CS_SLOT_2(_q_macRunNativeAppModalPanel)
#endif

};

inline void QFileDialog::setDirectory(const QDir &adirectory)
{
   setDirectory(adirectory.absolutePath());
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QFileDialog::Options)

#endif // QT_NO_FILEDIALOG

QT_END_NAMESPACE

#endif // QFILEDIALOG_H
