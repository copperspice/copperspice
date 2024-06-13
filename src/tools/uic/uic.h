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

#ifndef UIC_H
#define UIC_H

#include <customwidgetsinfo.h>
#include <databaseinfo.h>
#include <qhash.h>
#include <qstack.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qxmlstreamreader.h>

class QTextStream;
class QIODevice;

class Driver;
class DomUI;
class DomWidget;
class DomSpacer;
class DomLayout;
class DomLayoutItem;
class DomItem;

struct Option;

class Uic
{
 public:
   Uic(Driver *driver);
   ~Uic();

   bool printDependencies();

   Driver *driver() const {
      return drv;
   }

   QTextStream &output() {
      return out;
   }

   const Option &option() const {
      return opt;
   }

   QString pixmapFunction() const {
      return pixFunction;
   }

   void setPixmapFunction(const QString &f) {
      pixFunction = f;
   }

   bool hasExternalPixmap() const {
      return externalPix;
   }

   void setExternalPixmap(bool b) {
      externalPix = b;
   }

   const DatabaseInfo *databaseInfo() const {
      return &info;
   }

   const CustomWidgetsInfo *customWidgetsInfo() const {
      return &cWidgetsInfo;
   }

   bool write(QIODevice *in);

#ifdef QT_UIC_JAVA_GENERATOR
   bool jwrite(DomUI *ui);
#endif

#ifdef QT_UIC_CPP_GENERATOR
   bool write(DomUI *ui);
#endif

   bool isMainWindow(const QString &className) const;
   bool isToolBar(const QString &className) const;
   bool isStatusBar(const QString &className) const;
   bool isButton(const QString &className) const;
   bool isContainer(const QString &className) const;
   bool isCustomWidgetContainer(const QString &className) const;
   bool isMenuBar(const QString &className) const;
   bool isMenu(const QString &className) const;

 private:
   // copyright header
   void writeCopyrightHeader(DomUI *ui);
   DomUI *parseUiFile(QXmlStreamReader &reader);

#ifdef QT_UIC_CPP_GENERATOR
   // header protection
   void writeHeaderProtectionStart();
   void writeHeaderProtectionEnd();
#endif

   Driver *drv;
   QTextStream &out;
   Option &opt;
   DatabaseInfo info;
   CustomWidgetsInfo cWidgetsInfo;
   QString pixFunction;
   bool externalPix;
};

#endif
