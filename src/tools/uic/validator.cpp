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

#include <validator.h>

#include <driver.h>
#include <ui4.h>
#include <uic.h>

Validator::Validator(Uic *uic)
   : m_driver(uic->driver())
{
}

void Validator::acceptUI(DomUI *node)
{
   TreeWalker::acceptUI(node);
}

void Validator::acceptWidget(DomWidget *node)
{
   (void) m_driver->findOrInsertWidget(node);

   TreeWalker::acceptWidget(node);
}

void Validator::acceptLayoutItem(DomLayoutItem *node)
{
   (void) m_driver->findOrInsertLayoutItem(node);

   TreeWalker::acceptLayoutItem(node);
}

void Validator::acceptLayout(DomLayout *node)
{
   (void) m_driver->findOrInsertLayout(node);

   TreeWalker::acceptLayout(node);
}

void Validator::acceptActionGroup(DomActionGroup *node)
{
   (void) m_driver->findOrInsertActionGroup(node);

   TreeWalker::acceptActionGroup(node);
}

void Validator::acceptAction(DomAction *node)
{
   (void) m_driver->findOrInsertAction(node);

   TreeWalker::acceptAction(node);
}
