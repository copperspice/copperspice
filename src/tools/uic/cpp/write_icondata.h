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

#ifndef CPPWRITEICONDATA_H
#define CPPWRITEICONDATA_H

#include <treewalker.h>

#include <qiodevice.h>
#include <qstring.h>
#include <qtextstream.h>

class Driver;
class Uic;

struct Option;

namespace CPP {

class WriteIconData : public TreeWalker
{
 public:
   WriteIconData(Uic *uic);

   void acceptUI(DomUI *node) override;
   void acceptImages(DomImages *images) override;
   void acceptImage(DomImage *image) override;

   static void writeImage(QTextStream &output, const QString &indent, bool limitXPM_LineLength, const DomImage *image);
   static void writeImage(QIODevice &output, DomImage *image);

 private:
   Driver *driver;
   QTextStream &output;
   const Option &option;
};

} // namespace CPP

#endif