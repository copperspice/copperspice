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

#include "uic.h"
#include "ui4.h"
#include "driver.h"
#include "option.h"
#include "treewalker.h"
#include "validator.h"

#ifdef QT_UIC_CPP_GENERATOR
#include "cppwriteincludes.h"
#include "cppwritedeclaration.h"
#endif

#ifdef QT_UIC_JAVA_GENERATOR
#include "javawriteincludes.h"
#include "javawritedeclaration.h"
#endif

#include <QtCore/QXmlStreamReader>
#include <QtCore/QFileInfo>
#include <qregularexpression.h>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>

Uic::Uic(Driver *d)
   : drv(d), out(d->output()), opt(d->option()), info(d), externalPix(true)
{
}

Uic::~Uic()
{
}

bool Uic::printDependencies()
{
   QString fileName = opt.inputFile;

   QFile f;

   if (fileName.isEmpty()) {
      f.open(stdin, QIODevice::ReadOnly);

   } else {
      f.setFileName(fileName);
      if (! f.open(QIODevice::ReadOnly)) {
         return false;
      }
   }

   DomUI *ui = nullptr;
   {
      QXmlStreamReader reader;
      reader.setDevice(&f);
      ui = parseUiFile(reader);

      if (ui == nullptr) {
         return false;
      }
   }

   if (DomIncludes *includes = ui->elementIncludes()) {
      for (DomInclude * incl : includes->elementInclude()) {
         QString file = incl->text();

         if (file.isEmpty()) {
            continue;
         }

         fprintf(stdout, "%s\n", csPrintable(file));
      }
   }

   if (DomCustomWidgets *customWidgets = ui->elementCustomWidgets()) {
      for (DomCustomWidget * customWidget : customWidgets->elementCustomWidget()) {
         if (DomHeader *header = customWidget->elementHeader()) {
            QString file = header->text();
            if (file.isEmpty()) {
               continue;
            }

            fprintf(stdout, "%s\n", csPrintable(file));
         }
      }
   }

   delete ui;

   return true;
}

void Uic::writeCopyrightHeader(DomUI *ui)
{
   QString comment = ui->elementComment();

   if (comment.size()) {
      out << "/*\n" << comment << "\n*/\n\n";
   }

   out << "/********************************************************************************\n";
   out << "** Form generated from reading the UI file '" << QFileInfo(opt.inputFile).fileName() << "'\n";
   out << "**\n";
   out << "** Created by: CopperSpice User Interface Compiler, Version " << CS_VERSION_STR << "\n";
   out << "**\n";
   out << "** WARNING! Any changes made to this file will be lost when the UI file is recompiled\n";
   out << "********************************************************************************/\n\n";
}

// Check the version with a stream reader at the <ui> element.

static double versionFromUiAttribute(QXmlStreamReader &reader)
{
   const QXmlStreamAttributes attributes = reader.attributes();
   const QString versionAttribute = QLatin1String("version");

   if (! attributes.hasAttribute(versionAttribute)) {
      return 4.0;
   }

   const QString version = attributes.value(versionAttribute).toString();
   return version.toDouble();
}

DomUI *Uic::parseUiFile(QXmlStreamReader &reader)
{
   DomUI *ui = nullptr;
   const QString uiElement = "ui";

   while (! reader.atEnd()) {
      if (reader.readNext() == QXmlStreamReader::StartElement) {

         if (QString(reader.name()).compare(uiElement, Qt::CaseInsensitive) == 0 && ! ui) {
            // const double version = versionFromUiAttribute(reader);

            ui = new DomUI();
            ui->read(reader);

         } else {
            reader.raiseError("Unexpected element " + reader.name().toString());
         }
      }
   }

   if (reader.hasError()) {
      delete ui;
      ui = nullptr;

      fprintf(stderr, "%s\n", csPrintable(QString("Uic: Parse error on line %1, column %2 : %3")
                  .formatArg(reader.lineNumber()).formatArg(reader.columnNumber()).formatArg(reader.errorString())));
   }

   return ui;
}

bool Uic::write(QIODevice *in)
{
   if (option().generator == Option::JavaGenerator) {
      // the Java generator ignores header protection
      opt.headerProtection = false;
   }

   DomUI *ui = nullptr;

   {
      QXmlStreamReader reader;
      reader.setDevice(in);

      ui = parseUiFile(reader);

      if (ui == nullptr) {
         return false;
      }
   }

   double version = ui->attributeVersion().toDouble();
   if (version < 4.0) {
      delete ui;

      fprintf(stderr, "Uic: File generated with a version of Designer which is too old\n");
      return false;
   }

   QString language = ui->attributeLanguage();


   bool rtn = false;

   if (option().generator == Option::JavaGenerator) {

#ifdef QT_UIC_JAVA_GENERATOR
      if (language.toLower() != "jambi") {
         fprintf(stderr, "Uic: File is not a 'jambi' form\n");
         return false;
      }

      rtn = jwrite (ui);
#else
      fprintf(stderr, "Uic: option to generate java code not compiled in\n");
#endif

   } else {

#ifdef QT_UIC_CPP_GENERATOR
      if (! language.isEmpty() && language.toLower() != "c++") {
         fprintf(stderr, "Uic: File is not a 'c++' ui file, language=%s\n", qPrintable(language));
         return false;
      }

      rtn = write (ui);
#else
      fprintf(stderr, "Uic: option to generate cpp code not compiled in\n");
#endif
   }

   delete ui;

   return rtn;
}

#ifdef QT_UIC_CPP_GENERATOR
bool Uic::write(DomUI *ui)
{
   using namespace CPP;

   if (! ui || ! ui->elementWidget()) {
      return false;
   }

   if (opt.copyrightHeader) {
      writeCopyrightHeader(ui);
   }

   if (opt.headerProtection) {
      writeHeaderProtectionStart();
      out << "\n";
   }

   pixFunction = ui->elementPixmapFunction();
   if (pixFunction == QLatin1String("QPixmap::fromMimeSource")) {
      pixFunction = QLatin1String("qPixmapFromMimeSource");
   }

   externalPix = ui->elementImages() == 0;

   info.acceptUI(ui);
   cWidgetsInfo.acceptUI(ui);
   WriteIncludes writeIncludes(this);
   writeIncludes.acceptUI(ui);

   Validator(this).acceptUI(ui);
   WriteDeclaration(this, writeIncludes.scriptsActivated()).acceptUI(ui);

   if (opt.headerProtection) {
      writeHeaderProtectionEnd();
   }

   return true;
}
#endif

#ifdef QT_UIC_JAVA_GENERATOR
bool Uic::jwrite(DomUI *ui)
{
   using namespace Java;

   if (!ui || !ui->elementWidget()) {
      return false;
   }

   if (opt.copyrightHeader) {
      writeCopyrightHeader(ui);
   }

   pixFunction = ui->elementPixmapFunction();
   if (pixFunction == QLatin1String("QPixmap::fromMimeSource")) {
      pixFunction = QLatin1String("qPixmapFromMimeSource");
   }

   externalPix = ui->elementImages() == 0;

   info.acceptUI(ui);
   cWidgetsInfo.acceptUI(ui);
   WriteIncludes(this).acceptUI(ui);

   Validator(this).acceptUI(ui);
   WriteDeclaration(this).acceptUI(ui);

   return true;
}
#endif

#ifdef QT_UIC_CPP_GENERATOR

void Uic::writeHeaderProtectionStart()
{
   QString h = drv->headerFileName();
   out << "#ifndef " << h << "\n"
       << "#define " << h << "\n";
}

void Uic::writeHeaderProtectionEnd()
{
   QString h = drv->headerFileName();
   out << "#endif // " << h << "\n";
}
#endif

bool Uic::isMainWindow(const QString &className) const
{
   return customWidgetsInfo()->extends(className, QLatin1String("QMainWindow"));
}

bool Uic::isToolBar(const QString &className) const
{
   return customWidgetsInfo()->extends(className, QLatin1String("QToolBar"));
}

bool Uic::isButton(const QString &className) const
{
   return customWidgetsInfo()->extends(className, QLatin1String("QRadioButton"))
          || customWidgetsInfo()->extends(className, QLatin1String("QToolButton"))
          || customWidgetsInfo()->extends(className, QLatin1String("QCheckBox"))
          || customWidgetsInfo()->extends(className, QLatin1String("QPushButton"))
          || customWidgetsInfo()->extends(className, QLatin1String("QCommandLinkButton"));
}

bool Uic::isContainer(const QString &className) const
{
   return customWidgetsInfo()->extends(className, QLatin1String("QStackedWidget"))
          || customWidgetsInfo()->extends(className, QLatin1String("QToolBox"))
          || customWidgetsInfo()->extends(className, QLatin1String("QTabWidget"))
          || customWidgetsInfo()->extends(className, QLatin1String("QScrollArea"))
          || customWidgetsInfo()->extends(className, QLatin1String("QMdiArea"))
          || customWidgetsInfo()->extends(className, QLatin1String("QWizard"))
          || customWidgetsInfo()->extends(className, QLatin1String("QDockWidget"));
}

bool Uic::isCustomWidgetContainer(const QString &className) const
{
   return customWidgetsInfo()->isCustomWidgetContainer(className);
}

bool Uic::isStatusBar(const QString &className) const
{
   return customWidgetsInfo()->extends(className, QLatin1String("QStatusBar"));
}

bool Uic::isMenuBar(const QString &className) const
{
   return customWidgetsInfo()->extends(className, QLatin1String("QMenuBar"));
}

bool Uic::isMenu(const QString &className) const
{
   return customWidgetsInfo()->extends(className, QLatin1String("QMenu"))
          || customWidgetsInfo()->extends(className, QLatin1String("QPopupMenu"));
}


