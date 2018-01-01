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

#ifndef QUILOADER_H
#define QUILOADER_H

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QWidget;
class QLayout;
class QAction;
class QActionGroup;
class QString;
class QIODevice;
class QDir;

class QUiLoaderPrivate;
class QUiLoader : public QObject
{
    Q_OBJECT
public:
    QUiLoader(QObject *parent = 0);
    virtual ~QUiLoader();

    QStringList pluginPaths() const;
    void clearPluginPaths();
    void addPluginPath(const QString &path);

    QWidget *load(QIODevice *device, QWidget *parentWidget = 0);
    QStringList availableWidgets() const;
    QStringList availableLayouts() const;

    virtual QWidget *createWidget(const QString &className, QWidget *parent = 0, const QString &name = QString());
    virtual QLayout *createLayout(const QString &className, QObject *parent = 0, const QString &name = QString());
    virtual QActionGroup *createActionGroup(QObject *parent = 0, const QString &name = QString());
    virtual QAction *createAction(QObject *parent = 0, const QString &name = QString());

    void setWorkingDirectory(const QDir &dir);
    QDir workingDirectory() const;

    void setScriptingEnabled(bool enabled);
    bool isScriptingEnabled() const;

    void setLanguageChangeEnabled(bool enabled);
    bool isLanguageChangeEnabled() const;

    void setTranslationEnabled(bool enabled);
    bool isTranslationEnabled() const;

private:
    QScopedPointer<QUiLoaderPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QUiLoader)
    Q_DISABLE_COPY(QUiLoader)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QUILOADER_H
