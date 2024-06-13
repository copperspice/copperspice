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

#include <globals.h>
#include <mainwindow.h>

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qfile.h>
#include <qlibraryinfo.h>
#include <qlocale.h>
#include <qpixmap.h>
#include <qsettings.h>
#include <qtextcodec.h>
#include <qtranslator.h>

#ifdef Q_OS_DARWIN
#include <qfileopenevent.h>
#include <qurl.h>

class ApplicationEventFilter : public QObject
{
   CS_OBJECT(ApplicationEventFilter)

 public:
   ApplicationEventFilter()
      : m_mainWindow(nullptr) {
   }

   void setMainWindow(MainWindow *mw) {
      m_mainWindow = mw;

      if (! m_filesToOpen.isEmpty() && m_mainWindow) {
         m_mainWindow->openFiles(m_filesToOpen);
         m_filesToOpen.clear();
      }
   }

 protected:
   bool eventFilter(QObject *object, QEvent *event) override {
      if (object == qApp && event->type() == QEvent::FileOpen) {
         QFileOpenEvent *e = static_cast<QFileOpenEvent *>(event);
         QString file = e->url().toLocalFile();

         if (! m_mainWindow) {
            m_filesToOpen << file;
         } else {
            m_mainWindow->openFiles(QStringList() << file);
         }
         return true;
      }

      return QObject::eventFilter(object, event);
   }

 private:
   MainWindow *m_mainWindow;
   QStringList m_filesToOpen;
};
#endif

int main(int argc, char **argv)
{
   Q_INIT_RESOURCE(linguist);

   QApplication app(argc, argv);
   QApplication::setOverrideCursor(Qt::WaitCursor);

#ifdef Q_OS_DARWIN
   ApplicationEventFilter eventFilter;
   app.installEventFilter(&eventFilter);
#endif

   QStringList files;
   QStringList argList = app.arguments();

   // remove first entry which is the program name
   argList.removeFirst();

   QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

   while (! argList.isEmpty()) {
      QString argument = argList.takeFirst();

      if (argument == "-resourcedir") {
         if (argList.isEmpty()) {
            // show warning

         } else {
            resourceDir = argList.takeFirst();
         }

      } else if (! files.contains(argument)) {
         files.append(argument);
      }
   }

   QTranslator translator;
   QTranslator qtTranslator;
   QString sysLocale = QLocale::system().name();

   if (translator.load(QString("linguist_") + sysLocale, resourceDir)) {
      app.installTranslator(&translator);

      if (qtTranslator.load(QString("cs_") + sysLocale, resourceDir)) {
         app.installTranslator(&qtTranslator);
      } else {
         app.removeTranslator(&translator);
      }
   }

   app.setOrganizationName("CopperSpice");
   app.setApplicationName("Linguist");

   QSettings config;

   QWidget tmp;
   tmp.restoreGeometry(config.value(settingPath("Geometry/WindowGeometry")).toByteArray());

   MainWindow mw;

#ifdef Q_OS_DARWIN
   eventFilter.setMainWindow(&mw);
#endif

   mw.show();
   QApplication::restoreOverrideCursor();

   mw.openFiles(files, true);

   return app.exec();
}
