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

#include "../shared/collectionconfiguration.h"
#include "../shared/helpgenerator.h"

#include <private/qhelpgenerator_p.h>
#include <private/qhelpprojectdata_p.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QMap>
#include <QtCore/QFileInfo>
#include <QtCore/QDateTime>
#include <QtCore/QBuffer>
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QLibraryInfo>
#include <QtHelp/QHelpEngineCore>
#include <QtXml/QXmlStreamReader>


QT_USE_NAMESPACE

class QCG {
    Q_DECLARE_TR_FUNCTIONS(QCollectionGenerator)
};

class CollectionConfigReader : public QXmlStreamReader
{
public:
    void readData(const QByteArray &contents);

    QString title() const { return m_title; }
    QString homePage() const { return m_homePage; }
    QString startPage() const { return m_startPage; }
    QString applicationIcon() const { return m_applicationIcon; }
    QString currentFilter() const { return m_currentFilter; }
    bool enableFilterFunctionality() const
        { return m_enableFilterFunctionality; }
    bool hideFilterFunctionality() const
        { return m_hideFilterFunctionality; }
        bool enableAddressBar() const { return m_enableAddressBar; }
    bool hideAddressBar() const { return m_hideAddressBar; }
    bool enableDocumentationManager() const
        { return m_enableDocumentationManager; }

    QMap<QString, QString> aboutMenuTexts() const
        { return m_aboutMenuTexts; }
    QString aboutIcon() const { return m_aboutIcon; }
    QMap<QString, QString> aboutTextFiles() const
        { return m_aboutTextFiles; }

    QMap<QString, QString> filesToGenerate() const
        { return m_filesToGenerate; }

    QStringList filesToRegister() const { return m_filesToRegister; }

    QString cacheDirectory() const { return m_cacheDirectory; }
    bool cacheDirRelativeToCollection() const { return m_cacheDirRelativeToCollection; }

    bool fullTextSearchFallbackEnabled() const {
        return m_enableFullTextSearchFallback;
    }

private:
    void raiseErrorWithLine();
    void readConfig();
    void readAssistantSettings();
    void readMenuTexts();
    void readAboutDialog();
    void readDocFiles();
    void readGenerate();
    void readFiles();
    void readRegister();

    QString m_title;
    QString m_homePage;
    QString m_startPage;
    QString m_applicationIcon;
    QString m_currentFilter;
    bool m_enableFilterFunctionality;
    bool m_hideFilterFunctionality;
    bool m_enableAddressBar;
    bool m_hideAddressBar;
    bool m_enableDocumentationManager;
    QMap<QString, QString> m_aboutMenuTexts;
    QString m_aboutIcon;
    QMap<QString, QString> m_aboutTextFiles;
    QMap<QString, QString> m_filesToGenerate;
    QStringList m_filesToRegister;
    QString m_cacheDirectory;
    bool m_cacheDirRelativeToCollection;
    bool m_enableFullTextSearchFallback;
};

void CollectionConfigReader::raiseErrorWithLine()
{
    raiseError(QCG::tr("Unknown token at line %1.").arg(lineNumber()));
}

void CollectionConfigReader::readData(const QByteArray &contents)
{
    m_enableFilterFunctionality = true;
    m_hideFilterFunctionality = true;
    m_enableAddressBar = true;
    m_hideAddressBar = true;
    m_enableDocumentationManager = true;
    m_enableFullTextSearchFallback = false;

    addData(contents);
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("QHelpCollectionProject")
                && attributes().value(QLatin1String("version")) == QLatin1String("1.0"))
                readConfig();
            else
                raiseError(QCG::tr("Unknown token at line %1. "
                                   "Expected \"QtHelpCollectionProject\".")
                           .arg(lineNumber()));
        }
    }
}

void CollectionConfigReader::readConfig()
{
    bool ok = false;
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("assistant"))
                readAssistantSettings();
            else if (name() == QLatin1String("docFiles"))
                readDocFiles();
            else
                raiseErrorWithLine();
        } else if (isEndElement() && name() == QLatin1String("QHelpCollectionProject")) {
            ok = true;
        }
    }
    if (!ok && !hasError())
        raiseError(QCG::tr("Missing end tags."));
}

void CollectionConfigReader::readAssistantSettings()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("title")) {
                m_title = readElementText();
            } else if (name() == QLatin1String("homePage")) {
                m_homePage = readElementText();
            } else if (name() == QLatin1String("startPage")) {
                m_startPage = readElementText();
            } else if (name() == QLatin1String("currentFilter")) {
                m_currentFilter = readElementText();
            } else if (name() == QLatin1String("applicationIcon")) {
                m_applicationIcon = readElementText();
            } else if (name() == QLatin1String("enableFilterFunctionality")) {
                if (attributes().value(QLatin1String("visible")) == QLatin1String("true"))
                    m_hideFilterFunctionality = false;
                if (readElementText() == QLatin1String("false"))
                    m_enableFilterFunctionality = false;
            } else if (name() == QLatin1String("enableDocumentationManager")) {
                if (readElementText() == QLatin1String("false"))
                    m_enableDocumentationManager = false;
            } else if (name() == QLatin1String("enableAddressBar")) {
                if (attributes().value(QLatin1String("visible")) == QLatin1String("true"))
                    m_hideAddressBar = false;
                if (readElementText() == QLatin1String("false"))
                    m_enableAddressBar = false;
            } else if (name() == QLatin1String("aboutMenuText")) {
                readMenuTexts();
            } else if (name() == QLatin1String("aboutDialog")) {
                readAboutDialog();
            } else if (name() == "cacheDirectory") {
                m_cacheDirRelativeToCollection =
                    attributes().value(QLatin1String("base"))
                    == QLatin1String("collection");
                m_cacheDirectory = readElementText();
            } else if (name() == QLatin1String("enableFullTextSearchFallback")) {
                if (readElementText() == QLatin1String("true"))
                    m_enableFullTextSearchFallback = true;
            } else {
                raiseErrorWithLine();
            }
        } else if (isEndElement() && name() == QLatin1String("assistant")) {
            break;
        }
    }
}

void CollectionConfigReader::readMenuTexts()
{
    while (!atEnd())
    {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("text")) {
                QString lang = attributes().value(QLatin1String("language")).toString();
                if (lang.isEmpty())
                    lang = QLatin1String("default");
                m_aboutMenuTexts.insert(lang, readElementText());
            } else {
                raiseErrorWithLine();
            }
        } else if (isEndElement() && name() == QLatin1String("aboutMenuText")) {
            break;
        }
    }
}

void CollectionConfigReader::readAboutDialog()
{
    while (!atEnd())
    {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("file")) {
                QString lang = attributes().value(QLatin1String("language")).toString();
                if (lang.isEmpty())
                    lang = QLatin1String("default");
                m_aboutTextFiles.insert(lang, readElementText());
            } else if (name() == QLatin1String("icon")) {
                m_aboutIcon = readElementText();
            } else {
                raiseErrorWithLine();
            }
        } else if (isEndElement() && name() == QLatin1String("aboutDialog")) {
            break;
        }
    }
}

void CollectionConfigReader::readDocFiles()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("generate")) {
                readGenerate();
            } else if (name() == QLatin1String("register")) {
                readRegister();
            } else {
                raiseErrorWithLine();
            }
        } else if (isEndElement() && name() == QLatin1String("docFiles")) {
            break;
        }
    }
}

void CollectionConfigReader::readGenerate()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("file"))
                readFiles();
            else
                raiseErrorWithLine();
        } else if (isEndElement() && name() == QLatin1String("generate")) {
            break;
        }
    }
}

void CollectionConfigReader::readFiles()
{
    QString input;
    QString output;
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("input"))
                input = readElementText();
            else if (name() == QLatin1String("output"))
                output = readElementText();
            else
                raiseErrorWithLine();
        } else if (isEndElement() && name() == QLatin1String("file")) {
            break;
        }
    }
    if (input.isEmpty() || output.isEmpty()) {
        raiseError(QCG::tr("Missing input or output file for help file generation."));
        return;
    }
    m_filesToGenerate.insert(input, output);
}

void CollectionConfigReader::readRegister()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("file"))
                m_filesToRegister.append(readElementText());
            else
                raiseErrorWithLine();
        } else if (isEndElement() && name() == QLatin1String("register")) {
            break;
        }
    }
}

namespace {
    QString absoluteFileName(const QString &basePath, const QString &fileName)
    {
        return QFileInfo(fileName).isAbsolute() ?
            fileName : basePath + QDir::separator() + fileName;
    }
}

int main(int argc, char *argv[])
{
    QString error;
    QString arg;
    QString collectionFile;
    QString configFile;
    QString basePath;
    bool showHelp = false;
    bool showVersion = false;

    QCoreApplication app(argc, argv);
#ifndef Q_OS_WIN32
    QTranslator translator;
    QTranslator qtTranslator;
    QTranslator qt_helpTranslator;
    QString sysLocale = QLocale::system().name();
    QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if (translator.load(QLatin1String("assistant_") + sysLocale, resourceDir)
        && qtTranslator.load(QLatin1String("qt_") + sysLocale, resourceDir)
        && qt_helpTranslator.load(QLatin1String("qt_help_") + sysLocale, resourceDir)) {
        app.installTranslator(&translator);
        app.installTranslator(&qtTranslator);
        app.installTranslator(&qt_helpTranslator);
    }
#endif // Q_OS_WIN32

    for (int i=1; i<argc; ++i) {
        arg = QString::fromLocal8Bit(argv[i]);
        if (arg == QLatin1String("-o")) {
            if (++i < argc) {
                QFileInfo fi(QString::fromLocal8Bit(argv[i]));
                collectionFile = fi.absoluteFilePath();
            } else {
                error = QCG::tr("Missing output file name.");
            }
        } else if (arg == QLatin1String("-h")) {
            showHelp = true;
        } else if (arg == QLatin1String("-v")) {
            showVersion = true;
        } else {
            QFileInfo fi(arg);
            configFile = fi.absoluteFilePath();
            basePath = fi.absolutePath();
        }
    }

    if (showVersion) {
        fputs(qPrintable(QCG::tr("Qt Collection Generator version 1.0 (Qt %1)\n")
                .arg(QT_VERSION_STR)), stdout);
        return 0;
    }

    if (configFile.isEmpty() && !showHelp)
        error = QCG::tr("Missing collection config file.");

    QString help = QCG::tr("\nUsage:\n\n"
        "qcollectiongenerator <collection-config-file> [options]\n\n"
        "  -o <collection-file>   Generates a collection file\n"
        "                         called <collection-file>. If\n"
        "                         this option is not specified\n"
        "                         a default name will be used.\n"
        "  -v                     Displays the version of\n"
        "                         qcollectiongenerator.\n\n");

    if (showHelp) {
        fputs(qPrintable(help), stdout);
        return 0;
    }else if (!error.isEmpty()) {
        fprintf(stderr, "%s\n\n%s", qPrintable(error), qPrintable(help));
        return -1;
    }

    QFile file(configFile);
    if (!file.open(QIODevice::ReadOnly)) {
        fputs(qPrintable(QCG::tr("Could not open %1.\n").arg(configFile)), stderr);
        return -1;
    }

    if (collectionFile.isEmpty()) {
        QFileInfo fi(configFile);
        collectionFile = basePath + QDir::separator()
            + fi.baseName() + QLatin1String(".qhc");
    }

    fputs(qPrintable(QCG::tr("Reading collection config file...\n")), stdout);
    CollectionConfigReader config;
    config.readData(file.readAll());
    if (config.hasError()) {
        fputs(qPrintable(QCG::tr("Collection config file error: %1\n")
                         .arg(config.errorString())), stderr);
        return -1;
    }

    QMap<QString, QString>::const_iterator it = config.filesToGenerate().constBegin();
    while (it != config.filesToGenerate().constEnd()) {
        fputs(qPrintable(QCG::tr("Generating help for %1...\n").arg(it.key())), stdout);
        QHelpProjectData helpData;
        if (!helpData.readData(absoluteFileName(basePath, it.key()))) {
            fprintf(stderr, "%s\n", qPrintable(helpData.errorMessage()));
            return -1;
        }

        HelpGenerator helpGenerator;
        if (!helpGenerator.generate(&helpData, absoluteFileName(basePath, it.value()))) {
            fprintf(stderr, "%s\n", qPrintable(helpGenerator.error()));
            return -1;
        }
        ++it;
    }

    fputs(qPrintable(QCG::tr("Creating collection file...\n")), stdout);

    QFileInfo colFi(collectionFile);
    if (colFi.exists()) {
        if (!colFi.dir().remove(colFi.fileName())) {
            fputs(qPrintable(QCG::tr("The file %1 cannot be overwritten.\n")
                             .arg(collectionFile)), stderr);
            return -1;
        }
    }

    QHelpEngineCore helpEngine(collectionFile);
    if (!helpEngine.setupData()) {
        fprintf(stderr, "%s\n", qPrintable(helpEngine.error()));
        return -1;
    }

    foreach (const QString &file, config.filesToRegister()) {
        if (!helpEngine.registerDocumentation(absoluteFileName(basePath, file))) {
            fprintf(stderr, "%s\n", qPrintable(helpEngine.error()));
            return -1;
        }
    }
    if (!config.filesToRegister().isEmpty())
        CollectionConfiguration::updateLastRegisterTime(helpEngine);

    if (!config.title().isEmpty())
        CollectionConfiguration::setWindowTitle(helpEngine, config.title());

    if (!config.homePage().isEmpty()) {
        CollectionConfiguration::setDefaultHomePage(helpEngine,
            config.homePage());
    }

    if (!config.startPage().isEmpty()) {
        CollectionConfiguration::setLastShownPages(helpEngine,
            QStringList(config.startPage()));
    }

    if (!config.currentFilter().isEmpty()) {
        helpEngine.setCurrentFilter(config.currentFilter());
    }

    if (!config.cacheDirectory().isEmpty()) {
        CollectionConfiguration::setCacheDir(helpEngine, config.cacheDirectory(),
            config.cacheDirRelativeToCollection());
    }

    CollectionConfiguration::setFilterFunctionalityEnabled(helpEngine,
        config.enableFilterFunctionality());
    CollectionConfiguration::setFilterToolbarVisible(helpEngine,
        !config.hideFilterFunctionality());
    CollectionConfiguration::setDocumentationManagerEnabled(helpEngine,
        config.enableDocumentationManager());
    CollectionConfiguration::setAddressBarEnabled(helpEngine,
        config.enableAddressBar());
    CollectionConfiguration::setAddressBarVisible(helpEngine,
         !config.hideAddressBar());
    CollectionConfiguration::setCreationTime(helpEngine,
        QDateTime::currentDateTime().toTime_t());
    CollectionConfiguration::setFullTextSearchFallbackEnabled(helpEngine,
        config.fullTextSearchFallbackEnabled());

    if (!config.applicationIcon().isEmpty()) {
        QFile icon(absoluteFileName(basePath, config.applicationIcon()));
        if (!icon.open(QIODevice::ReadOnly)) {
            fputs(qPrintable(QCG::tr("Cannot open %1.\n").arg(icon.fileName())), stderr);
            return -1;
        }
        CollectionConfiguration::setApplicationIcon(helpEngine, icon.readAll());
    }

    if (config.aboutMenuTexts().count()) {
        QByteArray ba;
        QDataStream s(&ba, QIODevice::WriteOnly);
        QMap<QString, QString>::const_iterator it = config.aboutMenuTexts().constBegin();
        while (it != config.aboutMenuTexts().constEnd()) {
            s << it.key();
            s << it.value();
            ++it;
        }
        CollectionConfiguration::setAboutMenuTexts(helpEngine, ba);
    }

    if (!config.aboutIcon().isEmpty()) {
        QFile icon(absoluteFileName(basePath, config.aboutIcon()));
        if (!icon.open(QIODevice::ReadOnly)) {
            fputs(qPrintable(QCG::tr("Cannot open %1.\n").arg(icon.fileName())), stderr);
            return -1;
        }
        CollectionConfiguration::setAboutIcon(helpEngine, icon.readAll());
    }

    if (config.aboutTextFiles().count()) {
        QByteArray ba;
        QDataStream s(&ba, QIODevice::WriteOnly);
        QMap<QString, QString>::const_iterator it = config.aboutTextFiles().constBegin();
        QMap<QString, QByteArray> imgData;

        QRegExp srcRegExp(QLatin1String("src=(\"(.+)\"|([^\"\\s]+)).*>"));
        srcRegExp.setMinimal(true);
        QRegExp imgRegExp(QLatin1String("(<img[^>]+>)"));
        imgRegExp.setMinimal(true);

        while (it != config.aboutTextFiles().constEnd()) {
            s << it.key();
            QFileInfo fi(absoluteFileName(basePath, it.value()));
            QFile f(fi.absoluteFilePath());
            if (!f.open(QIODevice::ReadOnly)) {
                fputs(qPrintable(QCG::tr("Cannot open %1.\n").arg(f.fileName())), stderr);
                return -1;
            }
            QByteArray data = f.readAll();
            s << data;

            QString contents = QString::fromUtf8(data);
            int pos = 0;
            while ((pos = imgRegExp.indexIn(contents, pos)) != -1) {
                QString imgTag = imgRegExp.cap(1);
                pos += imgRegExp.matchedLength();

                if (srcRegExp.indexIn(imgTag, 0) != -1) {
                    QString src = srcRegExp.cap(2);
                    if (src.isEmpty())
                        src = srcRegExp.cap(3);

                    QFile img(fi.absolutePath() + QDir::separator() + src);
                    if (img.open(QIODevice::ReadOnly)) {
                        if (!imgData.contains(src))
                            imgData.insert(src, img.readAll());
                    } else {
                        fputs(qPrintable(QCG::tr("Cannot open referenced image file %1.\n")
                                         .arg(img.fileName())), stderr);
                    }
                }
            }
            ++it;
        }
        CollectionConfiguration::setAboutTexts(helpEngine, ba);
        if (imgData.count()) {
            QByteArray imageData;
            QBuffer buffer(&imageData);
            buffer.open(QIODevice::WriteOnly);
            QDataStream out(&buffer);
            out << imgData;
            CollectionConfiguration::setAboutImages(helpEngine, imageData);
        }
    }

    return 0;
}
