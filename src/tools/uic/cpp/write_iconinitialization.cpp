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

#include <write_iconinitialization.h>

#include <driver.h>
#include <ui4.h>
#include <uic.h>
#include <utils.h>
#include <write_icondata.h>

#include <qstring.h>
#include <qtextstream.h>

namespace CPP {

WriteIconInitialization::WriteIconInitialization(Uic *uic)
   : driver(uic->driver()), output(uic->output()), option(uic->option())
{
   this->uic = uic;
}

void WriteIconInitialization::acceptUI(DomUI *node)
{
   if (node->elementImages() == nullptr) {
      return;
   }

   QString className = node->elementClass() + option.postfix;

   output << option.indent << "static QPixmap " << iconFromDataFunction() << "(IconID id)\n"
      << option.indent << "{\n";

   WriteIconData(uic).acceptUI(node);

   output << option.indent << "switch (id) {\n";

   TreeWalker::acceptUI(node);

   output << option.indent << option.indent << "default: return QPixmap();\n";

   output << option.indent << "} // switch\n"
          << option.indent << "} // icon\n\n";
}

QString WriteIconInitialization::iconFromDataFunction()
{
   return "qt_get_icon";
}

void WriteIconInitialization::acceptImages(DomImages *images)
{
   TreeWalker::acceptImages(images);
}

void WriteIconInitialization::acceptImage(DomImage *image)
{
   QString img  = image->attributeName() + "_data";
   QString data = image->elementData()->text();
   QString fmt  = image->elementData()->attributeFormat();

   QString imageId   = image->attributeName() + "_ID";
   QString imageData = image->attributeName() + "_data";
   QString ind = option.indent + option.indent;

   output << ind << "case " << imageId << ": ";

   if (fmt == "XPM.GZ") {
      output << "return " << "QPixmap((const char**)" << imageData << ");\n";

   } else {
      output << " { QImage img; img.loadFromData(" << imageData << ", sizeof(" << imageData << "), " << fixString(fmt,
            ind) << "); return QPixmap::fromImage(img); }\n";
   }
}

} // namespace

