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

#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>

QT_BEGIN_NAMESPACE
/*
    Base class for logger entries;
    description() should return a text for this entry,
*/
class LogEntry
{
public:
    LogEntry(QString type, QString location);
    virtual ~LogEntry(){};
    virtual QString description() const =0;
    virtual void updateLinePos(int threshold, int delta) {Q_UNUSED(threshold); Q_UNUSED(delta)};
protected:
    QString type; // Error, Warning, Info, etc
    QString location;// preprocessor, c++parser, porting, etc
};

class PlainLogEntry: public LogEntry
{
public:
     PlainLogEntry(QString type, QString lcation, QString text);
     QString description() const {return text;};
protected:
     QString text;
};

/*
    A log entry that stores a source point: file, line and column.
*/
class SourcePointLogEntry : public LogEntry
{
public:
    SourcePointLogEntry(QString type, QString location, QString file, int line, int column, QString text);
    QString description() const;
    void updateLinePos(int threshold, int delta);
protected:
    QString file;
    int line;
    int column;
    QString text;
};


class Logger
{
public:
    Logger(){};
    ~Logger();
    static Logger *instance();
    static void deleteInstance();

    void beginSection();
    void commitSection();
    void revertSection();
    void addEntry(LogEntry *entry);

    QStringList fullReport();
    int numEntries();
    void updateLineNumbers(int insertLine, int numLines);
    /*
        glabalState can be used for storage of application state
        together with the logger. This can be useful in some cases,
        for example the current filename is stored here when processing
        files.
    */
    QMap<QString, QString> globalState;
private:
    static Logger *theInstance;
    QList<LogEntry*> logEntries;
    QList<LogEntry*> pendingLogEntries;
};

QT_END_NAMESPACE

#endif
