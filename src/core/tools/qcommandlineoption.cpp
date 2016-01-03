/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

   //! The list of names used for this option.
   QStringList names;

   //! The documentation name for the value, if one is expected
   //! Example: "-o <file>" means valueName == "file"
   QString valueName;

   //! The description used for this option.
   QString description;

   //! The list of default values used for this option.
   QStringList defaultValues;
};

/*!
    \since 5.2
    \class QCommandLineOption
    \brief The QCommandLineOption class defines a possible command-line option.
    \inmodule QtCore
    \ingroup shared
    \ingroup tools

    This class is used to describe an option on the command line. It allows
    different ways of defining the same option with multiple aliases possible.
    It is also used to describe how the option is used - it may be a flag (e.g. \c{-v})
    or take an argument (e.g. \c{-o file}).

    Examples:
    \snippet code/src_corelib_tools_qcommandlineoption.cpp 0

    \sa QCommandLineParser
*/

/*!
    Constructs a command line option object with the given arguments.

    The name of the option is set to \a name.
    The name can be either short or long. If the name is one character in
    length, it is considered a short name. Option names must not be empty,
    must not start with a dash or a slash character, must not contain a \c{=}
    and cannot be repeated.

    The description is set to \a description. It is customary to add a "."
    at the end of the description.

    In addition, the \a valueName can be set if the option expects a value.
    The default value for the option is set to \a defaultValue.

    \sa setDescription(), setValueName(), setDefaultValues()
*/
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

/*!
    Constructs a command line option object with the given arguments.

    This overload allows to set multiple names for the option, for instance
    \c{o} and \c{output}.

    The names of the option are set to \a names.
    The names can be either short or long. Any name in the list that is one
    character in length is a short name. Option names must not be empty,
    must not start with a dash or a slash character, must not contain a \c{=}
    and cannot be repeated.

    The description is set to \a description. It is customary to add a "."
    at the end of the description.

    In addition, the \a valueName can be set if the option expects a value.
    The default value for the option is set to \a defaultValue.

    \sa setDescription(), setValueName(), setDefaultValues()
*/
QCommandLineOption::QCommandLineOption(const QStringList &names, const QString &description,
                                       const QString &valueName,
                                       const QString &defaultValue)
   : d(new QCommandLineOptionPrivate)
{
   d->setNames(names);
   setValueName(valueName);
   setDescription(description);
   setDefaultValue(defaultValue);
}

/*!
    Constructs a QCommandLineOption object that is a copy of the QCommandLineOption
    object \a other.

    \sa operator=()
*/
QCommandLineOption::QCommandLineOption(const QCommandLineOption &other)
   : d(other.d)
{
}

/*!
    Destroys the command line option object.
*/
QCommandLineOption::~QCommandLineOption()
{
}

/*!
    Makes a copy of the \a other object and assigns it to this QCommandLineOption
    object.
*/
QCommandLineOption &QCommandLineOption::operator=(const QCommandLineOption &other)
{
   d = other.d;
   return *this;
}

/*!
    \fn void QCommandLineOption::swap(QCommandLineOption &other)

    Swaps option \a other with this option. This operation is very
    fast and never fails.
*/

/*!
    Returns the names set for this option.
 */
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
   foreach (const QString & name, nameList) {
      if (name.isEmpty()) {
         qWarning("QCommandLineOption: Option names cannot be empty");
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

/*!
    Sets the name of the expected value, for the documentation, to \a valueName.

    Options without a value assigned have a boolean-like behavior:
    either the user specifies --option or they don't.

    Options with a value assigned need to set a name for the expected value,
    for the documentation of the option in the help output. An option with names \c{o} and \c{output},
    and a value name of \c{file} will appear as \c{-o, --output <file>}.

    Call QCommandLineParser::argument() if you expect the option to be present
    only once, and QCommandLineParser::arguments() if you expect that option
    to be present multiple times.

    \sa valueName()
 */
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
