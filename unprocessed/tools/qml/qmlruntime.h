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

#ifndef QDECLARATIVEVIEWER_H
#define QDECLARATIVEVIEWER_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include <QList>

#include "loggerwidget.h"

QT_BEGIN_NAMESPACE

class QDeclarativeView;
class PreviewDeviceSkin;
class QDeclarativeTestEngine;
class QProcess;
class RecordingDialog;
class QDeclarativeTester;
class QNetworkReply;
class QNetworkCookieJar;
class NetworkAccessManagerFactory;
class QTranslator;
class QActionGroup;
class QMenuBar;

class QDeclarativeViewer
    : public QMainWindow
{
    Q_OBJECT

public:
    QDeclarativeViewer(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~QDeclarativeViewer();

    static void registerTypes();

    enum ScriptOption {
        Play = 0x00000001,
        Record = 0x00000002,
        TestImages = 0x00000004,
        TestErrorProperty = 0x00000008,
        SaveOnExit = 0x00000010,
        ExitOnComplete = 0x00000020,
        ExitOnFailure = 0x00000040,
        Snapshot = 0x00000080,
        TestSkipProperty = 0x00000100
    };
    Q_DECLARE_FLAGS(ScriptOptions, ScriptOption)
    void setScript(const QString &s) { m_script = s; }
    void setScriptOptions(ScriptOptions opt) { m_scriptOptions = opt; }
    void setRecordDither(const QString& s) { record_dither = s; }
    void setRecordRate(int fps);
    void setRecordFile(const QString&);
    void setRecordArgs(const QStringList&);
    void setRecording(bool on);
    bool isRecording() const { return recordTimer.isActive(); }
    void setAutoRecord(int from, int to);
    void setDeviceKeys(bool);
    void setNetworkCacheSize(int size);
    void addLibraryPath(const QString& lib);
    void addPluginPath(const QString& plugin);
    void setUseGL(bool use);
    void setUseNativeFileBrowser(bool);
    void setSizeToView(bool sizeToView);

    QDeclarativeView *view() const;
    LoggerWidget *warningsWidget() const;
    QString currentFile() const { return currentFileOrUrl; }

    void enableExperimentalGestures();

public slots:
    void sceneResized(QSize size);
    bool open(const QString&);
    void openFile();
    void openUrl();
    void reload();
    void takeSnapShot();
    void toggleRecording();
    void toggleRecordingWithSelection();
    void ffmpegFinished(int code);
    void showProxySettings ();
    void proxySettingsChanged ();
    void rotateOrientation();
    void statusChanged();
    void setSlowMode(bool);
    void launch(const QString &);

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual bool event(QEvent *);
    void createMenu();

private slots:
    void appAboutToQuit();

    void autoStartRecording();
    void autoStopRecording();
    void recordFrame();
    void chooseRecordingOptions();
    void pickRecordingFile();
    void toggleFullScreen();
    void changeOrientation(QAction*);
    void orientationChanged();

    void showWarnings(bool show);
    void warningsWidgetOpened();
    void warningsWidgetClosed();

private:
    void updateSizeHints(bool initial = false);

    QString getVideoFileName();

    LoggerWidget *loggerWindow;
    QDeclarativeView *canvas;
    QSize initialSize;
    QString currentFileOrUrl;
    QTimer recordTimer;
    QString frame_fmt;
    QImage frame;
    QList<QImage*> frames;
    QProcess* frame_stream;
    QTimer autoStartTimer;
    QTimer autoStopTimer;
    QString record_dither;
    QString record_file;
    QSize record_outsize;
    QStringList record_args;
    int record_rate;
    int record_autotime;
    bool devicemode;
    QAction *recordAction;
    RecordingDialog *recdlg;

    void senseImageMagick();
    void senseFfmpeg();
    QWidget *ffmpegHelpWindow;
    bool ffmpegAvailable;
    bool convertAvailable;

    QAction *rotateAction;
    QActionGroup *orientation;
    QAction *showWarningsWindow;

    QString m_script;
    ScriptOptions m_scriptOptions;
    QDeclarativeTester *tester;

    NetworkAccessManagerFactory *namFactory;

    bool useQmlFileBrowser;

    QTranslator *translator;
    void loadTranslationFile(const QString& directory);

    void loadDummyDataFiles(const QString& directory);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativeViewer::ScriptOptions)

QT_END_NAMESPACE

#endif
