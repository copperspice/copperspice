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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QHash>
#include <QDataStream>
#include <QSet>

#include "configfile.h"

#if 1
typedef int InputType;

enum SpecialInputType {
    DigitInput,
    SpaceInput,
    Letter
};

#else

enum SpecialInputType {
    NoSpecialInput = 0,
    DigitInput,
    SpaceInput,
    LetterOrNumberInput
};

struct InputType
{
    inline InputType() : val(0), specialInput(NoSpecialInput) {}
    inline InputType(const int &val) : val(val), specialInput(NoSpecialInput) {}

    inline operator int() const { return val; }

    inline bool operator==(const InputType &other) const
    { return val == other.val; }
    inline bool operator!=(const InputType &other) const
    { return val != other.val; }

    int val;
    SpecialInputType specialInput;
};

inline int qHash(const InputType &t) { return qHash(t.val); }

inline QDataStream &operator<<(QDataStream &stream, const InputType &i)
{
    return stream << i;
}

inline QDataStream &operator>>(QDataStream &stream, InputType &i)
{
    return stream >> i;
}

#endif

const InputType Epsilon = -1;

struct Config
{
    inline Config() : caseSensitivity(Qt::CaseSensitive), debug(false), cache(false) {}
    QSet<InputType> maxInputSet;
    Qt::CaseSensitivity caseSensitivity;
    QString className;
    bool debug;
    bool cache;
    QString ruleFile;
    ConfigFile::SectionMap configSections;
};

#endif // GLOBAL_H
