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

#ifndef QSCRIPTPROGRAM_H
#define QSCRIPTPROGRAM_H

#include <qsharedpointer.h>

#include <qstring.h>

class QScriptProgramPrivate;

class Q_SCRIPT_EXPORT QScriptProgram
{
 public:
   QScriptProgram();
   QScriptProgram(const QString &sourceCode,
      const QString fileName = QString(),
      int firstLineNumber = 1);
   QScriptProgram(const QScriptProgram &other);
   ~QScriptProgram();

   QScriptProgram &operator=(const QScriptProgram &other);

   bool isNull() const;

   QString sourceCode() const;
   QString fileName() const;
   int firstLineNumber() const;

   bool operator==(const QScriptProgram &other) const;
   bool operator!=(const QScriptProgram &other) const;

 private:
   QExplicitlySharedDataPointer<QScriptProgramPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QScriptProgram)
};

#endif // QSCRIPTPROGRAM_H
