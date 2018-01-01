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

#ifndef SOURCECODEVIEW_H
#define SOURCECODEVIEW_H

#include <QDir>
#include <QHash>
#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE

class SourceCodeView : public QPlainTextEdit
{
   Q_OBJECT
 public:
   SourceCodeView(QWidget *parent = nullptr);
   void setSourceContext(const QString &fileName, const int lineNum);
   void setCodecName(const QByteArray &codecName);

 public slots:
   void setActivated(bool activated);

 private:
   void showSourceCode(const QString &fileName, const int lineNum);

   bool m_isActive;
   QString m_fileToLoad;
   int m_lineNumToLoad;
   QString m_currentFileName;
   QByteArray m_codecName;

   QHash<QString, QString> fileHash;
};

QT_END_NAMESPACE

#endif // SOURCECODEVIEW_H
