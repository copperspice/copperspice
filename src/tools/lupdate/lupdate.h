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

#ifndef LUPDATE_H
#define LUPDATE_H

#include <qglobal.h>
#include <qlist.h>
#include <qstringfwd.h>

class ConversionData;
class QStringList;
class Translator;
class TranslatorMessage;

enum UpdateOption {
   Verbose    = 1,
   NoObsolete = 2,
   PluralOnly = 4,
   NoSorting  = 8,
   HeuristicSameText = 16,
   HeuristicSimilarText = 32,
   HeuristicNumber = 64,
   AbsoluteLocations = 256,
   RelativeLocations = 512,
   NoLocations = 1024,
   NoUiLines = 2048
};

using UpdateOptions = QFlags<UpdateOption>;

Q_DECLARE_OPERATORS_FOR_FLAGS(UpdateOptions)

Translator merge(const Translator &tor, const Translator &virginTor, const QList<Translator> &aliens,
                 UpdateOptions options, QString &err);

void loadCPP(Translator &translator, const QStringList &filenames, ConversionData &cd);
bool loadJava(Translator &translator, const QString &filename, ConversionData &cd);
bool loadUI(Translator &translator, const QString &filename, ConversionData &cd);

#endif
