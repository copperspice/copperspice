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

#ifndef QMLTOOLBAR_H
#define QMLTOOLBAR_H

#include <QtGui/QToolBar>
#include <QtGui/QIcon>

#include "../qmlinspectorconstants.h"

QT_FORWARD_DECLARE_CLASS(QActionGroup)

namespace QmlJSDebugger {

class ToolBarColorBox;

class QmlToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit QmlToolBar(QWidget *parent = nullptr);
    ~QmlToolBar();

public slots:
    void setDesignModeBehavior(bool inDesignMode);
    void setColorBoxColor(const QColor &color);
    void activateColorPicker();
    void activateSelectTool();
    void activateMarqueeSelectTool();
    void activateZoom();

    void setAnimationSpeed(qreal slowDownFactor);
    void setAnimationPaused(bool paused);

signals:
    void animationSpeedChanged(qreal factor);
    void animationPausedChanged(bool paused);

    void designModeBehaviorChanged(bool inDesignMode);
    void colorPickerSelected();
    void selectToolSelected();
    void marqueeSelectToolSelected();
    void zoomToolSelected();

    void applyChangesToQmlFileSelected();
    void applyChangesFromQmlFileSelected();

private slots:
    void setDesignModeBehaviorOnClick(bool inDesignMode);
    void activatePlayOnClick();
    void activateColorPickerOnClick();
    void activateSelectToolOnClick();
    void activateMarqueeSelectToolOnClick();
    void activateZoomOnClick();

    void activateFromQml();
    void activateToQml();

    void changeAnimationSpeed();

    void updatePlayAction();

private:
    class Ui {
    public:
        QAction *designmode;
        QAction *play;
        QAction *select;
        QAction *selectMarquee;
        QAction *zoom;
        QAction *colorPicker;
        QAction *toQml;
        QAction *fromQml;
        QIcon playIcon;
        QIcon pauseIcon;
        ToolBarColorBox *colorBox;

        QActionGroup *playSpeedMenuActions;
    };

    bool m_emitSignals;
    bool m_paused;
    qreal m_animationSpeed;

    Constants::DesignTool m_activeTool;

    Ui *ui;
};

}

#endif // QMLTOOLBAR_H
