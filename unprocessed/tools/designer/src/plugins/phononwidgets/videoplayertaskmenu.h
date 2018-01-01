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

#ifndef VIDEOPLAYERTASKMENU_H
#define VIDEOPLAYERTASKMENU_H


#include <QtCore/QObject>
#include <QtDesigner/QDesignerTaskMenuExtension>
#include <QtDesigner/private/extensionfactory_p.h>

#include <phonon/backendcapabilities.h>
#include <phonon/videoplayer.h>

QT_BEGIN_NAMESPACE

class VideoPlayerTaskMenu: public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES(QDesignerTaskMenuExtension)
public:
    explicit VideoPlayerTaskMenu(Phonon::VideoPlayer *object, QObject *parent = 0);
    virtual QList<QAction*> taskActions() const;

private slots:
    void slotLoad();
    void slotMimeTypes();
    void mediaObjectStateChanged(Phonon::State newstate, Phonon::State oldstate);

private:
    Phonon::VideoPlayer *m_widget;
    QAction *m_displayMimeTypesAction;
    QAction *m_loadAction;
    QAction *m_playAction;
    QAction *m_pauseAction;
    QAction *m_stopAction;

    QList<QAction*> m_taskActions;
};

typedef qdesigner_internal::ExtensionFactory<QDesignerTaskMenuExtension, Phonon::VideoPlayer, VideoPlayerTaskMenu>  VideoPlayerTaskMenuFactory;

QT_END_NAMESPACE

#endif // VIDEOPLAYERTASKMENU_H
