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

#ifndef SOURCECODEVIEW_H
#define SOURCECODEVIEW_H

#include <QDir>
#include <QHash>
#include <QPlainTextEdit>

class SourceCodeView : public QPlainTextEdit
{
   CS_OBJECT(SourceCodeView)

 public:
   SourceCodeView(QWidget *parent = nullptr);
   void setSourceContext(const QString &fileName, const int lineNum);

   CS_SLOT_1(Public, void setActivated(bool activated))
   CS_SLOT_2(setActivated)

 private:
   void showSourceCode(const QString &fileName, const int lineNum);

   bool m_isActive;
   QString m_fileToLoad;
   int m_lineNumToLoad;
   QString m_currentFileName;

   QHash<QString, QString> fileHash;
};

#endif
