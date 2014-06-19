/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "treewalker.h"

QT_BEGIN_NAMESPACE

class QTextStream;
class Driver;
class Uic;

struct Option;

struct Validator : public TreeWalker {
   Validator(Uic *uic);

   void acceptUI(DomUI *node);
   void acceptWidget(DomWidget *node);

   void acceptLayoutItem(DomLayoutItem *node);
   void acceptLayout(DomLayout *node);

   void acceptActionGroup(DomActionGroup *node);
   void acceptAction(DomAction *node);

 private:
   Driver *m_driver;
};

QT_END_NAMESPACE

#endif // VALIDATOR_H
