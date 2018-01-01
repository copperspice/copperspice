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

#include "cppwriteicondeclaration.h"
#include "driver.h"
#include "ui4.h"
#include "uic.h"

#include <QtCore/QTextStream>

QT_BEGIN_NAMESPACE

namespace CPP {

WriteIconDeclaration::WriteIconDeclaration(Uic *uic)
   : driver(uic->driver()), output(uic->output()), option(uic->option())
{
}

void WriteIconDeclaration::acceptUI(DomUI *node)
{
   TreeWalker::acceptUI(node);
}

void WriteIconDeclaration::acceptImages(DomImages *images)
{
   TreeWalker::acceptImages(images);
}

void WriteIconDeclaration::acceptImage(DomImage *image)
{
   QString name = image->attributeName();
   if (name.isEmpty()) {
      return;
   }

   driver->insertPixmap(name);
   output << option.indent << option.indent << name << "_ID,\n";
}

} // namespace CPP

QT_END_NAMESPACE
