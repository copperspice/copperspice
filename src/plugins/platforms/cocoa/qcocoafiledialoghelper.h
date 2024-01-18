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

#ifndef QCOCOAFILEDIALOGHELPER_H
#define QCOCOAFILEDIALOGHELPER_H

#include <qobject.h>
#include <qplatform_dialoghelper.h>

#ifdef __OBJC__
@class QNSOpenSavePanelDelegate;
#else
using QNSOpenSavePanelDelegate = struct objc_object;
#endif

class QFileDialog;
class QFileDialogPrivate;

class QCocoaFileDialogHelper : public QPlatformFileDialogHelper
{
 public:
   QCocoaFileDialogHelper();
   virtual ~QCocoaFileDialogHelper();

   void exec() override;

   bool defaultNameFilterDisables() const override;

   bool show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent) override;
   void hide() override;
   void setDirectory(const QUrl &directory) override;
   QUrl directory() const override;
   void selectFile(const QUrl &filename) override;
   QList<QUrl> selectedFiles() const override;
   void setFilter() override;
   void selectNameFilter(const QString &filter) override;
   QString selectedNameFilter() const override;

 public:
   bool showCocoaFilePanel(Qt::WindowModality windowModality, QWindow *parent);
   bool hideCocoaFilePanel();

   void createNSOpenSavePanelDelegate();
   void QNSOpenSavePanelDelegate_selectionChanged(const QString &newPath);
   void QNSOpenSavePanelDelegate_panelClosed(bool accepted);
   void QNSOpenSavePanelDelegate_directoryEntered(const QString &newDir);
   void QNSOpenSavePanelDelegate_filterSelected(int menuIndex);

 private:
   QNSOpenSavePanelDelegate *mDelegate;
   QUrl mDir;
};

#endif // QCOCOAFILEDIALOGHELPER_H
