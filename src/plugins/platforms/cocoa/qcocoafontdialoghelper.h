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

#ifndef QCOCOAFONTDIALOGHELPER_H
#define QCOCOAFONTDIALOGHELPER_H

#include <QObject>
#include <qplatform_dialoghelper.h>

class QCocoaFontDialogHelper : public QPlatformFontDialogHelper
{
 public:
   QCocoaFontDialogHelper();
   ~QCocoaFontDialogHelper();

   void exec() override;

   bool show(Qt::WindowFlags windowFlags, Qt::WindowModality windowModality, QWindow *parent) override;
   void hide() override;

   void setCurrentFont(const QFont &) override;
   QFont currentFont() const override;
};


#endif // QCOCOAFONTDIALOGHELPER_H
