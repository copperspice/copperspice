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

/*****************************************************
** Copyright (c) 2013 Laszlo Papp <lpapp@kde.org>
** Copyright (c) 2013 David Faure <faure@kde.org>
*****************************************************/

#include <qcommandlineoption.h>
#include <qset.h>

class QCommandLineOptionPrivate : public QSharedData
{
 public:
   QCommandLineOptionPrivate()
   { }

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
      const QString &valueName, const QString &defaultValue)
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
      qWarning("QCommandLineOption:setName() List of options can not be empty");
   }

   for (const QString &name : nameList) {
      if (name.isEmpty()) {
         qWarning("QCommandLineOption:setName() Option names can not be empty");

      } else {
         const QChar c = name.at(0);

         if (c == '-') {
            qWarning("QCommandLineOption:setName() Option names can not start with a '-'");

         } else if (c == '/') {
            qWarning("QCommandLineOption:setName() Option names can not start with a '/'");

         } else if (name.contains('=')) {
            qWarning("QCommandLineOption:setName() Option names can not contain a '='");

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

QString QCommandLineOption::valueName() const
{
   return d->valueName;
}

void QCommandLineOption::setDescription(const QString &description)
{
   d->description = description;
}

QString QCommandLineOption::description() const
{
   return d->description;
}

void QCommandLineOption::setDefaultValue(const QString &defaultValue)
{
   d->defaultValues.clear();

   if (! defaultValue.isEmpty()) {
      d->defaultValues << defaultValue;
   }
}

void QCommandLineOption::setDefaultValues(const QStringList &defaultValues)
{
   d->defaultValues = defaultValues;
}

QStringList QCommandLineOption::defaultValues() const
{
   return d->defaultValues;
}
