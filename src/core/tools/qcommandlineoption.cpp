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

/*****************************************************
** Copyright (C) 2013 Laszlo Papp <lpapp@kde.org>
** Copyright (C) 2013 David Faure <faure@kde.org>
*****************************************************/

#include <qcommandlineoption.h>
#include <qset.h>

QT_BEGIN_NAMESPACE

class QCommandLineOptionPrivate : public QSharedData
{
 public:
   inline QCommandLineOptionPrivate() {
   }

   void setNames(const QStringList &nameList);

   // The list of names used for this option.
   QStringList names;

   // The documentation name for the value, if one is expected
   // Example: "-o <file>" means valueName == "file"
   QString valueName;

   // The description used for this option.
   QString description;

   // The list of default values used for this option.
   QStringList defaultValues;
};


QCommandLineOption::QCommandLineOption(const QString &name, const QString &description,
                                       const QString &valueName,
                                       const QString &defaultValue)
   : d(new QCommandLineOptionPrivate)
{
   d->setNames(QStringList(name));
   setValueName(valueName);
   setDescription(description);
   setDefaultValue(defaultValue);
}


QCommandLineOption::QCommandLineOption(const QStringList &names, const QString &description,
                                       const QString &valueName, const QString &defaultValue)
   : d(new QCommandLineOptionPrivate)
{
   d->setNames(names);
   setValueName(valueName);
   setDescription(description);
   setDefaultValue(defaultValue);
}

QCommandLineOption::QCommandLineOption(const QCommandLineOption &other)
   : d(other.d)
{
}

QCommandLineOption::~QCommandLineOption()
{
}

QCommandLineOption &QCommandLineOption::operator=(const QCommandLineOption &other)
{
   d = other.d;
   return *this;
}

QStringList QCommandLineOption::names() const
{
   return d->names;
}

void QCommandLineOptionPrivate::setNames(const QStringList &nameList)
{
   names.clear();
   if (nameList.isEmpty()) {
      qWarning("QCommandLineOption: Options must have at least one name");
   }
   for (const QString & name : nameList) {
      if (name.isEmpty()) {
         qWarning("QCommandLineOption: Option names can not be empty");
      } else {
         const QChar c = name.at(0);
         if (c == QLatin1Char('-')) {
            qWarning("QCommandLineOption: Option names cannot start with a '-'");
         } else if (c == QLatin1Char('/')) {
            qWarning("QCommandLineOption: Option names cannot start with a '/'");
         } else if (name.contains(QLatin1Char('='))) {
            qWarning("QCommandLineOption: Option names cannot contain a '='");
         } else {
            names.append(name);
         }
      }
   }
}

void QCommandLineOption::setValueName(const QString &valueName)
{
   d->valueName = valueName;
}

/*!
    Returns the name of the expected value.

    If empty, the option doesn't take a value.

    \sa setValueName()
 */
QString QCommandLineOption::valueName() const
{
   return d->valueName;
}

/*!
    Sets the description used for this option to \a description.

    It is customary to add a "." at the end of the description.

    The description is used by QCommandLineParser::showHelp().

    \sa description()
 */
void QCommandLineOption::setDescription(const QString &description)
{
   d->description = description;
}

/*!
    Returns the description set for this option.

    \sa setDescription()
 */
QString QCommandLineOption::description() const
{
   return d->description;
}

/*!
    Sets the default value used for this option to \a defaultValue.

    The default value is used if the user of the application does not specify
    the option on the command line.

    If \a defaultValue is empty, the option has no default values.

    \sa defaultValues() setDefaultValues()
 */
void QCommandLineOption::setDefaultValue(const QString &defaultValue)
{
   d->defaultValues.clear();
   if (!defaultValue.isEmpty()) {
      d->defaultValues << defaultValue;
   }
}

/*!
    Sets the list of default values used for this option to \a defaultValues.

    The default values are used if the user of the application does not specify
    the option on the command line.

    \sa defaultValues() setDefaultValue()
 */
void QCommandLineOption::setDefaultValues(const QStringList &defaultValues)
{
   d->defaultValues = defaultValues;
}

/*!
    Returns the default values set for this option.

    \sa setDefaultValues()
 */
QStringList QCommandLineOption::defaultValues() const
{
   return d->defaultValues;
}

QT_END_NAMESPACE
