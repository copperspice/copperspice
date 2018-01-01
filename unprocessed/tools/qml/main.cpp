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

#include "qdeclarative.h"
#include "qmlruntime.h"
#include "qdeclarativeengine.h"
#include "loggerwidget.h"
#include <QWidget>
#include <QDir>
#include <QApplication>
#include <QTranslator>
#include <QDebug>
#include <QMessageBox>
#include <QAtomicInt>
#include <QLibraryInfo>
#include "qdeclarativetester.h"

QT_USE_NAMESPACE

QtMsgHandler systemMsgOutput = 0;

static QDeclarativeViewer *openFile(const QString &fileName);
static void showViewer(QDeclarativeViewer *viewer);

QString warnings;
void exitApp(int i)
{
#ifdef Q_OS_WIN
    // Debugging output is not visible by default on Windows -
    // therefore show modal dialog with errors instead.
    if (!warnings.isEmpty()) {
        QMessageBox::warning(0, QApplication::translate("QDeclarativeViewer", "Qt QML Viewer"), warnings);
    }
#endif
    exit(i);
}

QWeakPointer<LoggerWidget> logger;
static QAtomicInt recursiveLock(0);

#if defined (Q_OS_SYMBIAN)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

void myMessageOutput(QtMsgType type, const char *msg)
{
    QString strMsg = QString::fromLatin1(msg);

    if (!QCoreApplication::closingDown()) {
        if (!logger.isNull()) {
            if (recursiveLock.testAndSetOrdered(0, 1)) {
                QMetaObject::invokeMethod(logger.data(), "append", Q_ARG(QString, strMsg));
                recursiveLock = 0;
            }
        } else {
            warnings += strMsg;
            warnings += QLatin1Char('\n');
        }
    }
#if defined (Q_OS_SYMBIAN)
    static int fd = -1;
    if (fd == -1)
        fd = ::open("E:\\qml.log", O_WRONLY | O_CREAT);

    ::write(fd, msg, strlen(msg));
    ::write(fd, "\n", 1);
    ::fsync(fd);
    switch (type) {
    case QtFatalMsg:
        abort();
    }
#endif

    if (systemMsgOutput) {
        systemMsgOutput(type, msg);
    } else { // Unix
        fprintf(stderr, "%s\n", msg);
        fflush(stderr);
    }
}

static QDeclarativeViewer* globalViewer = 0;

// The qml file that is shown if the user didn't specify a QML file
QString initialFile = QLatin1String("qrc:/startup/startup.qml");

void usage()
{
    qWarning("Usage: qmlviewer [options] <filename>");
    qWarning(" ");
    qWarning(" options:");
    qWarning("  -v, -version ............................. display version");
    qWarning("  -frameless ............................... run with no window frame");
    qWarning("  -maximized................................ run maximized");
    qWarning("  -fullscreen............................... run fullscreen");
    qWarning("  -stayontop................................ keep viewer window on top");
    qWarning("  -sizeviewtorootobject .................... the view resizes to the changes in the content");
    qWarning("  -sizerootobjecttoview .................... the content resizes to the changes in the view (default)");
    qWarning("  -qmlbrowser .............................. use a QML-based file browser");
    qWarning("  -warnings [show|hide]..................... show warnings in a separate log window");
    qWarning("  -recordfile <output> ..................... set video recording file");
    qWarning("                                              - ImageMagick 'convert' for GIF)");
    qWarning("                                              - png file for raw frames");
    qWarning("                                              - 'ffmpeg' for other formats");
    qWarning("  -recorddither ordered|threshold|floyd .... set GIF dither recording mode");
    qWarning("  -recordrate <fps> ........................ set recording frame rate");
    qWarning("  -record arg .............................. add a recording process argument");
    qWarning("  -autorecord [from-]<tomilliseconds> ...... set recording to start and stop");
    qWarning("  -devicekeys .............................. use numeric keys (see F1)");
    qWarning("  -dragthreshold <size> .................... set mouse drag threshold size");
    qWarning("  -netcache <size> ......................... set disk cache to size bytes");
    qWarning("  -translation <translationfile> ........... set the language to run in");
    qWarning("  -I <directory> ........................... prepend to the module import search path,");
    qWarning("                                             display path if <directory> is empty");
    qWarning("  -P <directory> ........................... prepend to the plugin search path");
#if defined(Q_WS_MAC)
    qWarning("  -no-opengl ............................... don't use a QGLWidget for the viewport");
    qWarning("  -opengl .................................. use a QGLWidget for the viewport (default)");
#else
    qWarning("  -no-opengl ............................... don't use a QGLWidget for the viewport (default)");
    qWarning("  -opengl .................................. use a QGLWidget for the viewport");
#endif
    qWarning("  -script <path> ........................... set the script to use");
    qWarning("  -scriptopts <options>|help ............... set the script options to use");

    qWarning(" ");
    qWarning(" Press F1 for interactive help");

    exitApp(1);
}

void scriptOptsUsage()
{
    qWarning("Usage: qmlviewer -scriptopts <option>[,<option>...] ...");
    qWarning(" options:");
    qWarning("  record ................................... record a new script");
    qWarning("  play ..................................... playback an existing script");
    qWarning("  testimages ............................... record images or compare images on playback");
    qWarning("  testerror ................................ test 'error' property of root item on playback");
    qWarning("  testskip  ................................ test 'skip' property of root item on playback");
    qWarning("  snapshot ................................. file being recorded is static,");
    qWarning("                                             only one frame will be recorded or tested");
    qWarning("  exitoncomplete ........................... cleanly exit the viewer on script completion");
    qWarning("  exitonfailure ............................ immediately exit the viewer on script failure");
    qWarning("  saveonexit ............................... save recording on viewer exit");
    qWarning(" ");
    qWarning(" One of record, play or both must be specified.");

    exitApp(1);
}

enum WarningsConfig { ShowWarnings, HideWarnings, DefaultWarnings };

struct ViewerOptions
{
    ViewerOptions()
        : frameless(false),
          fps(0.0),
          autorecord_from(0),
          autorecord_to(0),
          dither(QLatin1String("none")),
          runScript(false),
          devkeys(false),
          cache(0),
          useGL(false),
          fullScreen(false),
          stayOnTop(false),
          maximized(false),
          useNativeFileBrowser(true),
          experimentalGestures(false),
          warningsConfig(DefaultWarnings),
          sizeToView(true)
    {
#if defined(Q_OS_SYMBIAN)
        maximized = true;
        useNativeFileBrowser = false;
#endif

#if defined(Q_WS_MAC)
        useGL = true;
#endif
    }

    bool frameless;
    double fps;
    int autorecord_from;
    int autorecord_to;
    QString dither;
    QString recordfile;
    QStringList recordargs;
    QStringList imports;
    QStringList plugins;
    QString script;
    QString scriptopts;
    bool runScript;
    bool devkeys;
    int cache;
    QString translationFile;
    bool useGL;
    bool fullScreen;
    bool stayOnTop;
    bool maximized;
    bool useNativeFileBrowser;
    bool experimentalGestures;

    WarningsConfig warningsConfig;
    bool sizeToView;

    QDeclarativeViewer::ScriptOptions scriptOptions;
};

static ViewerOptions opts;
static QStringList fileNames;

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **&argv)
        : QApplication(argc, argv)
    {}

protected:
    bool event(QEvent *ev)
    {
        if (ev->type() != QEvent::FileOpen)
            return QApplication::event(ev);

        QFileOpenEvent *fev = static_cast<QFileOpenEvent *>(ev);

        globalViewer->open(fev->file());
        if (!globalViewer->isVisible())
            showViewer(globalViewer);

        return true;
    }

private Q_SLOTS:
    void showInitialViewer()
    {
        QApplication::processEvents();

        QDeclarativeViewer *viewer = globalViewer;
        if (!viewer)
            return;
        if (viewer->currentFile().isEmpty()) {
            if(opts.useNativeFileBrowser)
                viewer->open(initialFile);
            else
                viewer->openFile();
        }
        if (!viewer->isVisible())
            showViewer(viewer);
    }
};

static void parseScriptOptions()
{
    QStringList options =
        opts.scriptopts.split(QLatin1Char(','), QString::SkipEmptyParts);

    QDeclarativeViewer::ScriptOptions scriptOptions = 0;
    for (int i = 0; i < options.count(); ++i) {
        const QString &option = options.at(i);
        if (option == QLatin1String("help")) {
            scriptOptsUsage();
        } else if (option == QLatin1String("play")) {
            scriptOptions |= QDeclarativeViewer::Play;
        } else if (option == QLatin1String("record")) {
            scriptOptions |= QDeclarativeViewer::Record;
        } else if (option == QLatin1String("testimages")) {
            scriptOptions |= QDeclarativeViewer::TestImages;
        } else if (option == QLatin1String("testerror")) {
            scriptOptions |= QDeclarativeViewer::TestErrorProperty;
        } else if (option == QLatin1String("testskip")) {
            scriptOptions |= QDeclarativeViewer::TestSkipProperty;
        } else if (option == QLatin1String("exitoncomplete")) {
            scriptOptions |= QDeclarativeViewer::ExitOnComplete;
        } else if (option == QLatin1String("exitonfailure")) {
            scriptOptions |= QDeclarativeViewer::ExitOnFailure;
        } else if (option == QLatin1String("saveonexit")) {
            scriptOptions |= QDeclarativeViewer::SaveOnExit;
        } else if (option == QLatin1String("snapshot")) {
            scriptOptions |= QDeclarativeViewer::Snapshot;
        } else {
            scriptOptsUsage();
        }
    }

    opts.scriptOptions = scriptOptions;
}

static void parseCommandLineOptions(const QStringList &arguments)
{
    for (int i = 1; i < arguments.count(); ++i) {
        bool lastArg = (i == arguments.count() - 1);
        QString arg = arguments.at(i);
        if (arg == QLatin1String("-frameless")) {
            opts.frameless = true;
        } else if (arg == QLatin1String("-maximized")) {
            opts.maximized = true;
        } else if (arg == QLatin1String("-fullscreen")) {
            opts.fullScreen = true;
        } else if (arg == QLatin1String("-stayontop")) {
            opts.stayOnTop = true;
        } else if (arg == QLatin1String("-netcache")) {
            if (lastArg) usage();
            opts.cache = arguments.at(++i).toInt();
        } else if (arg == QLatin1String("-recordrate")) {
            if (lastArg) usage();
            opts.fps = arguments.at(++i).toDouble();
        } else if (arg == QLatin1String("-recordfile")) {
            if (lastArg) usage();
            opts.recordfile = arguments.at(++i);
        } else if (arg == QLatin1String("-record")) {
            if (lastArg) usage();
            opts.recordargs << arguments.at(++i);
        } else if (arg == QLatin1String("-recorddither")) {
            if (lastArg) usage();
            opts.dither = arguments.at(++i);
        } else if (arg == QLatin1String("-autorecord")) {
            if (lastArg) usage();
            QString range = arguments.at(++i);
            int dash = range.indexOf(QLatin1Char('-'));
            if (dash > 0)
                opts.autorecord_from = range.left(dash).toInt();
            opts.autorecord_to = range.mid(dash+1).toInt();
        } else if (arg == QLatin1String("-devicekeys")) {
            opts.devkeys = true;
        } else if (arg == QLatin1String("-dragthreshold")) {
            if (lastArg) usage();
            qApp->setStartDragDistance(arguments.at(++i).toInt());
        } else if (arg == QLatin1String("-v") || arg == QLatin1String("-version")) {
            qWarning("Qt QML Viewer version %s", QT_VERSION_STR);
            exitApp(0);
        } else if (arg == QLatin1String("-translation")) {
            if (lastArg) usage();
            opts.translationFile = arguments.at(++i);
        } else if (arg == QLatin1String("-no-opengl")) {
            opts.useGL = false;
        } else if (arg == QLatin1String("-opengl")) {
            opts.useGL = true;
        } else if (arg == QLatin1String("-qmlbrowser")) {
            opts.useNativeFileBrowser = false;
        } else if (arg == QLatin1String("-warnings")) {
            if (lastArg) usage();
            QString warningsStr = arguments.at(++i);
            if (warningsStr == QLatin1String("show")) {
                opts.warningsConfig = ShowWarnings;
            } else if (warningsStr == QLatin1String("hide")) {
                opts.warningsConfig = HideWarnings;
            } else {
                usage();
            }
        } else if (arg == QLatin1String("-I") || arg == QLatin1String("-L")) {
            if (arg == QLatin1String("-L"))
                qWarning("-L option provided for compatibility only, use -I instead");
            if (lastArg) {
                QDeclarativeEngine tmpEngine;
                QString paths = tmpEngine.importPathList().join(QLatin1String(":"));
                qWarning("Current search path: %s", paths.toLocal8Bit().constData());
                exitApp(0);
            }
            opts.imports << arguments.at(++i);
        } else if (arg == QLatin1String("-P")) {
            if (lastArg) usage();
            opts.plugins << arguments.at(++i);
        } else if (arg == QLatin1String("-script")) {
            if (lastArg) usage();
            opts.script = arguments.at(++i);
        } else if (arg == QLatin1String("-scriptopts")) {
            if (lastArg) usage();
            opts.scriptopts = arguments.at(++i);
        } else if (arg == QLatin1String("-savescript")) {
            if (lastArg) usage();
            opts.script = arguments.at(++i);
            opts.runScript = false;
        } else if (arg == QLatin1String("-playscript")) {
            if (lastArg) usage();
            opts.script = arguments.at(++i);
            opts.runScript = true;
        } else if (arg == QLatin1String("-sizeviewtorootobject")) {
            opts.sizeToView = false;
        } else if (arg == QLatin1String("-sizerootobjecttoview")) {
            opts.sizeToView = true;
        } else if (arg == QLatin1String("-experimentalgestures")) {
            opts.experimentalGestures = true;
        } else if (!arg.startsWith(QLatin1Char('-'))) {
            fileNames.append(arg);
        } else if (true || arg == QLatin1String("-help")) {
            usage();
        }
    }

    if (!opts.scriptopts.isEmpty()) {

        parseScriptOptions();

        if (opts.script.isEmpty())
            usage();

        if (!(opts.scriptOptions & QDeclarativeViewer::Record) && !(opts.scriptOptions & QDeclarativeViewer::Play))
            scriptOptsUsage();
    }  else if (!opts.script.isEmpty()) {
        usage();
    }

}

static QDeclarativeViewer *createViewer()
{
    Qt::WFlags wflags = (opts.frameless ? Qt::FramelessWindowHint : Qt::Widget);
    if (opts.stayOnTop)
        wflags |= Qt::WindowStaysOnTopHint;

    QDeclarativeViewer *viewer = new QDeclarativeViewer(0, wflags);
    viewer->setAttribute(Qt::WA_DeleteOnClose, true);
    viewer->setUseGL(opts.useGL);

    if (!opts.scriptopts.isEmpty()) {
        viewer->setScriptOptions(opts.scriptOptions);
        viewer->setScript(opts.script);
    }

    logger = viewer->warningsWidget();
    if (opts.warningsConfig == ShowWarnings) {
        logger.data()->setDefaultVisibility(LoggerWidget::ShowWarnings);
        logger.data()->show();
    } else if (opts.warningsConfig == HideWarnings){
        logger.data()->setDefaultVisibility(LoggerWidget::HideWarnings);
    }

    if (opts.experimentalGestures)
        viewer->enableExperimentalGestures();

    foreach (QString lib, opts.imports)
        viewer->addLibraryPath(lib);

    foreach (QString plugin, opts.plugins)
        viewer->addPluginPath(plugin);

    viewer->setNetworkCacheSize(opts.cache);
    viewer->setRecordFile(opts.recordfile);
    viewer->setSizeToView(opts.sizeToView);
    if (opts.fps > 0)
        viewer->setRecordRate(opts.fps);
    if (opts.autorecord_to)
        viewer->setAutoRecord(opts.autorecord_from, opts.autorecord_to);
    if (opts.devkeys)
        viewer->setDeviceKeys(true);
    viewer->setRecordDither(opts.dither);
    if (opts.recordargs.count())
        viewer->setRecordArgs(opts.recordargs);

    viewer->setUseNativeFileBrowser(opts.useNativeFileBrowser);

    return viewer;
}

void showViewer(QDeclarativeViewer *viewer)
{
    if (opts.fullScreen)
        viewer->showFullScreen();
    else if (opts.maximized)
        viewer->showMaximized();
    else
        viewer->show();
    viewer->raise();
}

QDeclarativeViewer *openFile(const QString &fileName)
{
    QDeclarativeViewer *viewer = globalViewer;

    viewer->open(fileName);
    showViewer(viewer);

    return viewer;
}

int main(int argc, char ** argv)
{
    systemMsgOutput = qInstallMsgHandler(myMessageOutput);

#if defined (Q_WS_X11) || defined (Q_WS_MAC)
    //### default to using raster graphics backend for now
    bool gsSpecified = false;
    for (int i = 0; i < argc; ++i) {
        QString arg = QString::fromAscii(argv[i]);
        if (arg == QLatin1String("-graphicssystem")) {
            gsSpecified = true;
            break;
        }
    }

    if (!gsSpecified)
        QApplication::setGraphicsSystem(QLatin1String("raster"));
#endif

    Application app(argc, argv);
    app.setApplicationName(QLatin1String("QtQmlViewer"));
    app.setOrganizationName(QLatin1String("Nokia"));
    app.setOrganizationDomain(QLatin1String("nokia.com"));

    QDeclarativeViewer::registerTypes();
    QDeclarativeTester::registerTypes();

    parseCommandLineOptions(app.arguments());

    QTranslator translator;
    QTranslator qtTranslator;
    QString sysLocale = QLocale::system().name();
    if (translator.load(QLatin1String("qmlviewer_") + sysLocale, QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&translator);
        if (qtTranslator.load(QLatin1String("qt_") + sysLocale, QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
            app.installTranslator(&qtTranslator);
        } else {
            app.removeTranslator(&translator);
        }
    }

    QTranslator qmlTranslator;
    if (!opts.translationFile.isEmpty()) {
        if (qmlTranslator.load(opts.translationFile)) {
            app.installTranslator(&qmlTranslator);
        } else {
            qWarning() << "Could not load the translation file" << opts.translationFile;
        }
    }

    if (opts.fullScreen && opts.maximized)
        qWarning() << "Both -fullscreen and -maximized specified. Using -fullscreen.";

    if (fileNames.isEmpty()) {
        QFile qmlapp(QLatin1String("qmlapp"));
        if (qmlapp.exists() && qmlapp.open(QFile::ReadOnly)) {
            QString content = QString::fromUtf8(qmlapp.readAll());
            qmlapp.close();

            int newline = content.indexOf(QLatin1Char('\n'));
            if (newline >= 0)
                fileNames += content.left(newline);
            else
                fileNames += content;
        }
    }

    globalViewer = createViewer();

    if (fileNames.isEmpty()) {
        // show the initial viewer delayed.
        // This prevents an initial viewer popping up while there
        // are FileOpen events coming through the event queue
        QTimer::singleShot(1, &app, SLOT(showInitialViewer()));
    } else {
        foreach (const QString &fileName, fileNames)
            openFile(fileName);
    }

    QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

    return app.exec();
}

#include "main.moc"
