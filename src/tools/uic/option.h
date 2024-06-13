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

#ifndef OPTION_H
#define OPTION_H

#include <qdir.h>
#include <qstring.h>

struct Option {
   enum Generator {
      CppGenerator,
      JavaGenerator
   };

   unsigned int headerProtection : 1;
   unsigned int copyrightHeader : 1;
   unsigned int generateImplemetation : 1;
   unsigned int generateNamespace : 1;
   unsigned int autoConnection : 1;
   unsigned int dependencies : 1;
   unsigned int extractImages : 1;
   unsigned int limitXPM_LineLength : 1;
   unsigned int implicitIncludes: 1;

   Generator generator;

   QString inputFile;
   QString outputFile;
   QString qrcOutputFile;
   QString indent;
   QString prefix;
   QString postfix;
   QString translateFunction;

#ifdef QT_UIC_JAVA_GENERATOR
   QString javaPackage;
   QString javaOutputDirectory;
#endif

   Option()
      : headerProtection(1), copyrightHeader(1), generateImplemetation(0), generateNamespace(1), autoConnection(1),
        dependencies(0), extractImages(0), limitXPM_LineLength(0), implicitIncludes(1),
        generator(CppGenerator), prefix("Ui_")
   {
      indent.fill(' ', 4);
   }

   QString messagePrefix() const {
      return inputFile.isEmpty() ? QString("stdin") : QDir::toNativeSeparators(inputFile);
   }
};

#endif
