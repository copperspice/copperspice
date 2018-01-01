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

#include "qclucenefieldnames_p.h"
#include "qhelpenginecore.h"
#include "qhelp_global.h"
#include "fulltextsearch/qhits_p.h"
#include "fulltextsearch/qquery_p.h"
#include "fulltextsearch/qanalyzer_p.h"
#include "fulltextsearch/qdocument_p.h"
#include "fulltextsearch/qsearchable_p.h"
#include "fulltextsearch/qindexreader_p.h"
#include "fulltextsearch/qindexwriter_p.h"
#include "qhelpsearchindexwriter_clucene_p.h"

#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>

#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QLocalServer>

#include "private/qfunctions_p.h"

QT_BEGIN_NAMESPACE

namespace fulltextsearch {
namespace clucene {

// taken from qtexthtmlparser
static const struct QTextHtmlEntity
{
    const char *name;
    quint16 code;
} entities[] = {
    { "AElig", 0x00c6 },
    { "AMP", 38 },
    { "Aacute", 0x00c1 },
    { "Acirc", 0x00c2 },
    { "Agrave", 0x00c0 },
    { "Alpha", 0x0391 },
    { "Aring", 0x00c5 },
    { "Atilde", 0x00c3 },
    { "Auml", 0x00c4 },
    { "Beta", 0x0392 },
    { "Ccedil", 0x00c7 },
    { "Chi", 0x03a7 },
    { "Dagger", 0x2021 },
    { "Delta", 0x0394 },
    { "ETH", 0x00d0 },
    { "Eacute", 0x00c9 },
    { "Ecirc", 0x00ca },
    { "Egrave", 0x00c8 },
    { "Epsilon", 0x0395 },
    { "Eta", 0x0397 },
    { "Euml", 0x00cb },
    { "GT", 62 },
    { "Gamma", 0x0393 },
    { "Iacute", 0x00cd },
    { "Icirc", 0x00ce },
    { "Igrave", 0x00cc },
    { "Iota", 0x0399 },
    { "Iuml", 0x00cf },
    { "Kappa", 0x039a },
    { "LT", 60 },
    { "Lambda", 0x039b },
    { "Mu", 0x039c },
    { "Ntilde", 0x00d1 },
    { "Nu", 0x039d },
    { "OElig", 0x0152 },
    { "Oacute", 0x00d3 },
    { "Ocirc", 0x00d4 },
    { "Ograve", 0x00d2 },
    { "Omega", 0x03a9 },
    { "Omicron", 0x039f },
    { "Oslash", 0x00d8 },
    { "Otilde", 0x00d5 },
    { "Ouml", 0x00d6 },
    { "Phi", 0x03a6 },
    { "Pi", 0x03a0 },
    { "Prime", 0x2033 },
    { "Psi", 0x03a8 },
    { "QUOT", 34 },
    { "Rho", 0x03a1 },
    { "Scaron", 0x0160 },
    { "Sigma", 0x03a3 },
    { "THORN", 0x00de },
    { "Tau", 0x03a4 },
    { "Theta", 0x0398 },
    { "Uacute", 0x00da },
    { "Ucirc", 0x00db },
    { "Ugrave", 0x00d9 },
    { "Upsilon", 0x03a5 },
    { "Uuml", 0x00dc },
    { "Xi", 0x039e },
    { "Yacute", 0x00dd },
    { "Yuml", 0x0178 },
    { "Zeta", 0x0396 },
    { "aacute", 0x00e1 },
    { "acirc", 0x00e2 },
    { "acute", 0x00b4 },
    { "aelig", 0x00e6 },
    { "agrave", 0x00e0 },
    { "alefsym", 0x2135 },
    { "alpha", 0x03b1 },
    { "amp", 38 },
    { "and", 0x22a5 },
    { "ang", 0x2220 },
    { "apos", 0x0027 },
    { "aring", 0x00e5 },
    { "asymp", 0x2248 },
    { "atilde", 0x00e3 },
    { "auml", 0x00e4 },
    { "bdquo", 0x201e },
    { "beta", 0x03b2 },
    { "brvbar", 0x00a6 },
    { "bull", 0x2022 },
    { "cap", 0x2229 },
    { "ccedil", 0x00e7 },
    { "cedil", 0x00b8 },
    { "cent", 0x00a2 },
    { "chi", 0x03c7 },
    { "circ", 0x02c6 },
    { "clubs", 0x2663 },
    { "cong", 0x2245 },
    { "copy", 0x00a9 },
    { "crarr", 0x21b5 },
    { "cup", 0x222a },
    { "curren", 0x00a4 },
    { "dArr", 0x21d3 },
    { "dagger", 0x2020 },
    { "darr", 0x2193 },
    { "deg", 0x00b0 },
    { "delta", 0x03b4 },
    { "diams", 0x2666 },
    { "divide", 0x00f7 },
    { "eacute", 0x00e9 },
    { "ecirc", 0x00ea },
    { "egrave", 0x00e8 },
    { "empty", 0x2205 },
    { "emsp", 0x2003 },
    { "ensp", 0x2002 },
    { "epsilon", 0x03b5 },
    { "equiv", 0x2261 },
    { "eta", 0x03b7 },
    { "eth", 0x00f0 },
    { "euml", 0x00eb },
    { "euro", 0x20ac },
    { "exist", 0x2203 },
    { "fnof", 0x0192 },
    { "forall", 0x2200 },
    { "frac12", 0x00bd },
    { "frac14", 0x00bc },
    { "frac34", 0x00be },
    { "frasl", 0x2044 },
    { "gamma", 0x03b3 },
    { "ge", 0x2265 },
    { "gt", 62 },
    { "hArr", 0x21d4 },
    { "harr", 0x2194 },
    { "hearts", 0x2665 },
    { "hellip", 0x2026 },
    { "iacute", 0x00ed },
    { "icirc", 0x00ee },
    { "iexcl", 0x00a1 },
    { "igrave", 0x00ec },
    { "image", 0x2111 },
    { "infin", 0x221e },
    { "int", 0x222b },
    { "iota", 0x03b9 },
    { "iquest", 0x00bf },
    { "isin", 0x2208 },
    { "iuml", 0x00ef },
    { "kappa", 0x03ba },
    { "lArr", 0x21d0 },
    { "lambda", 0x03bb },
    { "lang", 0x2329 },
    { "laquo", 0x00ab },
    { "larr", 0x2190 },
    { "lceil", 0x2308 },
    { "ldquo", 0x201c },
    { "le", 0x2264 },
    { "lfloor", 0x230a },
    { "lowast", 0x2217 },
    { "loz", 0x25ca },
    { "lrm", 0x200e },
    { "lsaquo", 0x2039 },
    { "lsquo", 0x2018 },
    { "lt", 60 },
    { "macr", 0x00af },
    { "mdash", 0x2014 },
    { "micro", 0x00b5 },
    { "middot", 0x00b7 },
    { "minus", 0x2212 },
    { "mu", 0x03bc },
    { "nabla", 0x2207 },
    { "nbsp", 0x00a0 },
    { "ndash", 0x2013 },
    { "ne", 0x2260 },
    { "ni", 0x220b },
    { "not", 0x00ac },
    { "notin", 0x2209 },
    { "nsub", 0x2284 },
    { "ntilde", 0x00f1 },
    { "nu", 0x03bd },
    { "oacute", 0x00f3 },
    { "ocirc", 0x00f4 },
    { "oelig", 0x0153 },
    { "ograve", 0x00f2 },
    { "oline", 0x203e },
    { "omega", 0x03c9 },
    { "omicron", 0x03bf },
    { "oplus", 0x2295 },
    { "or", 0x22a6 },
    { "ordf", 0x00aa },
    { "ordm", 0x00ba },
    { "oslash", 0x00f8 },
    { "otilde", 0x00f5 },
    { "otimes", 0x2297 },
    { "ouml", 0x00f6 },
    { "para", 0x00b6 },
    { "part", 0x2202 },
    { "percnt", 0x0025 },
    { "permil", 0x2030 },
    { "perp", 0x22a5 },
    { "phi", 0x03c6 },
    { "pi", 0x03c0 },
    { "piv", 0x03d6 },
    { "plusmn", 0x00b1 },
    { "pound", 0x00a3 },
    { "prime", 0x2032 },
    { "prod", 0x220f },
    { "prop", 0x221d },
    { "psi", 0x03c8 },
    { "quot", 34 },
    { "rArr", 0x21d2 },
    { "radic", 0x221a },
    { "rang", 0x232a },
    { "raquo", 0x00bb },
    { "rarr", 0x2192 },
    { "rceil", 0x2309 },
    { "rdquo", 0x201d },
    { "real", 0x211c },
    { "reg", 0x00ae },
    { "rfloor", 0x230b },
    { "rho", 0x03c1 },
    { "rlm", 0x200f },
    { "rsaquo", 0x203a },
    { "rsquo", 0x2019 },
    { "sbquo", 0x201a },
    { "scaron", 0x0161 },
    { "sdot", 0x22c5 },
    { "sect", 0x00a7 },
    { "shy", 0x00ad },
    { "sigma", 0x03c3 },
    { "sigmaf", 0x03c2 },
    { "sim", 0x223c },
    { "spades", 0x2660 },
    { "sub", 0x2282 },
    { "sube", 0x2286 },
    { "sum", 0x2211 },
    { "sup", 0x2283 },
    { "sup1", 0x00b9 },
    { "sup2", 0x00b2 },
    { "sup3", 0x00b3 },
    { "supe", 0x2287 },
    { "szlig", 0x00df },
    { "tau", 0x03c4 },
    { "there4", 0x2234 },
    { "theta", 0x03b8 },
    { "thetasym", 0x03d1 },
    { "thinsp", 0x2009 },
    { "thorn", 0x00fe },
    { "tilde", 0x02dc },
    { "times", 0x00d7 },
    { "trade", 0x2122 },
    { "uArr", 0x21d1 },
    { "uacute", 0x00fa },
    { "uarr", 0x2191 },
    { "ucirc", 0x00fb },
    { "ugrave", 0x00f9 },
    { "uml", 0x00a8 },
    { "upsih", 0x03d2 },
    { "upsilon", 0x03c5 },
    { "uuml", 0x00fc },
    { "weierp", 0x2118 },
    { "xi", 0x03be },
    { "yacute", 0x00fd },
    { "yen", 0x00a5 },
    { "yuml", 0x00ff },
    { "zeta", 0x03b6 },
    { "zwj", 0x200d },
    { "zwnj", 0x200c }
};

Q_STATIC_GLOBAL_OPERATOR bool operator<(const QString &entityStr, const QTextHtmlEntity &entity)
{
    return entityStr < QLatin1String(entity.name);
}

Q_STATIC_GLOBAL_OPERATOR bool operator<(const QTextHtmlEntity &entity, const QString &entityStr)
{
    return QLatin1String(entity.name) < entityStr;
}

static QChar resolveEntity(const QString &entity)
{
    const QTextHtmlEntity *start = &entities[0];
    const QTextHtmlEntity *end = &entities[(sizeof(entities) / sizeof(entities[0]))];
    const QTextHtmlEntity *e = qBinaryFind(start, end, entity);
    if (e == end)
        return QChar();
    return e->code;
}

static const uint latin1Extended[0xA0 - 0x80] = {
    0x20ac, // 0x80
    0x0081, // 0x81 direct mapping
    0x201a, // 0x82
    0x0192, // 0x83
    0x201e, // 0x84
    0x2026, // 0x85
    0x2020, // 0x86
    0x2021, // 0x87
    0x02C6, // 0x88
    0x2030, // 0x89
    0x0160, // 0x8A
    0x2039, // 0x8B
    0x0152, // 0x8C
    0x008D, // 0x8D direct mapping
    0x017D, // 0x8E
    0x008F, // 0x8F directmapping
    0x0090, // 0x90 directmapping
    0x2018, // 0x91
    0x2019, // 0x92
    0x201C, // 0x93
    0X201D, // 0x94
    0x2022, // 0x95
    0x2013, // 0x96
    0x2014, // 0x97
    0x02DC, // 0x98
    0x2122, // 0x99
    0x0161, // 0x9A
    0x203A, // 0x9B
    0x0153, // 0x9C
    0x009D, // 0x9D direct mapping
    0x017E, // 0x9E
    0x0178  // 0x9F
};
// end taken from qtexthtmlparser

class DocumentHelper
{
public:
    DocumentHelper(const QString &fileName, const QByteArray &data)
        : fileName(fileName) , data(readData(data)) {}
    ~DocumentHelper() {}

    bool addFieldsToDocument(QCLuceneDocument *document,
        const QString &namespaceName, const QString &attributes = QString())
    {
        if (!document)
            return false;

        if(!data.isEmpty()) {
            QString parsedData = parseData();
            QString parsedTitle = QHelpGlobal::documentTitle(data);

            if(!parsedData.isEmpty()) {
                document->add(new QCLuceneField(ContentField,
                    parsedData,QCLuceneField::INDEX_TOKENIZED));
                document->add(new QCLuceneField(PathField, fileName,
                    QCLuceneField::STORE_YES | QCLuceneField::INDEX_UNTOKENIZED));
                document->add(new QCLuceneField(TitleField, parsedTitle,
                    QCLuceneField::STORE_YES | QCLuceneField::INDEX_UNTOKENIZED));
                document->add(new QCLuceneField(TitleTokenizedField, parsedTitle,
                    QCLuceneField::STORE_YES | QCLuceneField::INDEX_TOKENIZED));
                document->add(new QCLuceneField(NamespaceField, namespaceName,
                    QCLuceneField::STORE_YES | QCLuceneField::INDEX_UNTOKENIZED));
                document->add(new QCLuceneField(AttributeField, attributes,
                    QCLuceneField::STORE_YES | QCLuceneField::INDEX_TOKENIZED));
                return true;
            }
        }

        return false;
    }

private:
    QString readData(const QByteArray &data)
    {
        QTextStream textStream(data);
        const QByteArray &codec = QHelpGlobal::codecFromData(data).toLatin1();
        textStream.setCodec(QTextCodec::codecForName(codec.constData()));

        QString stream = textStream.readAll();
        if (stream.isNull() || stream.isEmpty())
            return QString();

        return stream;
    }

    QString parseData() const
    {
        const int length = data.length();
        const QChar *buf = data.unicode();

        QString parsedContent;
        parsedContent.reserve(length);

        bool valid = true;
        int j = 0, count = 0;

        QChar c;
        while (j < length) {
            c = buf[j++];
            if (c == QLatin1Char('<') || c == QLatin1Char('&')) {
                if (count > 1 && c != QLatin1Char('&'))
                    parsedContent.append(QLatin1Char(' '));
                else if (c == QLatin1Char('&')) {
                    // Note: this will modify the counter j, in case we sucessful parsed the entity
                    //       we will have modified the counter to stay 1 before the closing ';', so
                    //       the following if condition will be met with if (c == QLatin1Char(';'))
                    parsedContent.append(parseEntity(length, buf, j));
                }

                count = 0;
                valid = false;
                continue;
            }
            if ((c == QLatin1Char('>') || c == QLatin1Char(';')) && !valid) {
                valid = true;
                continue;
            }
            if (!valid)
                continue;

            if (c.isLetterOrNumber() || c.isPrint()) {
                ++count;
                parsedContent.append(c.toLower());
            } else {
                if (count > 1)
                    parsedContent.append(QLatin1Char(' '));
                count = 0;
            }
        }

        return parsedContent;
    }

    // taken from qtexthtmlparser
    // parses an entity after "&", and returns it
    QString parseEntity(int len, const QChar *buf, int &pos) const
    {
        int recover = pos;
        QString entity;
        while (pos < len) {
            QChar c = buf[pos++];
            if (c.isSpace() || pos - recover > 9) {
                goto error;
            }
            if (c == QLatin1Char(';')) {
                pos--;
                break;
            }
            entity += c;
        }
        {
            QChar resolved = resolveEntity(entity);
            if (!resolved.isNull())
                return QString(resolved);
        }
        if (entity.length() > 1 && entity.at(0) == QLatin1Char('#')) {
            entity.remove(0, 1); // removing leading #

            int base = 10;
            bool ok = false;

            if (entity.at(0).toLower() == QLatin1Char('x')) { // hex entity?
                entity.remove(0, 1);
                base = 16;
            }

            uint uc = entity.toUInt(&ok, base);
            if (ok) {
                if (uc >= 0x80  && uc < 0x80 + (sizeof(latin1Extended) / sizeof(latin1Extended[0])))
                    uc = latin1Extended[uc - 0x80]; // windows latin 1 extended
                QString str;
                if (uc > 0xffff) {
                    // surrogate pair
                    uc -= 0x10000;
                    ushort high = uc/0x400 + 0xd800;
                    ushort low = uc%0x400 + 0xdc00;
                    str.append(QChar(high));
                    str.append(QChar(low));
                } else {
                    str.append(QChar(uc));
                }
                return str;
            }
        }
    error:
        pos = recover;
        return QLatin1String(" ");
    }
    // end taken from qtexthtmlparser

private:
    QString fileName;
    QString data;
};


QHelpSearchIndexWriter::QHelpSearchIndexWriter()
    : QThread(0)
    , m_cancel(false)
{
    // nothing todo
}

QHelpSearchIndexWriter::~QHelpSearchIndexWriter()
{
    mutex.lock();
    this->m_cancel = true;
    waitCondition.wakeOne();
    mutex.unlock();

    wait();
}

void QHelpSearchIndexWriter::cancelIndexing()
{
    mutex.lock();
    this->m_cancel = true;
    mutex.unlock();
}

void QHelpSearchIndexWriter::updateIndex(const QString &collectionFile,
    const QString &indexFilesFolder, bool reindex)
{
    wait();
    mutex.lock();
    this->m_cancel = false;
    this->m_reindex = reindex;
    this->m_collectionFile = collectionFile;
    this->m_indexFilesFolder = indexFilesFolder;
    mutex.unlock();

    start(QThread::LowestPriority);
}

void QHelpSearchIndexWriter::optimizeIndex()
{
#if !defined(QT_NO_EXCEPTIONS)
    try {
#endif
        if (QCLuceneIndexReader::indexExists(m_indexFilesFolder)) {
            if (QCLuceneIndexReader::isLocked(m_indexFilesFolder))
                return;

            QCLuceneStandardAnalyzer analyzer;
            QCLuceneIndexWriter writer(m_indexFilesFolder, analyzer, false);
            writer.optimize();
            writer.close();
        }
#if !defined(QT_NO_EXCEPTIONS)
    } catch (...) {
        qWarning("Full Text Search, could not optimize index.");
        return;
    }
#endif
}

void QHelpSearchIndexWriter::run()
{
#if !defined(QT_NO_EXCEPTIONS)
    try {
#endif
        QMutexLocker mutexLocker(&mutex);

        if (m_cancel)
            return;

        const bool reindex = this->m_reindex;
        const QString collectionFile(this->m_collectionFile);

        mutexLocker.unlock();

        QHelpEngineCore engine(collectionFile, 0);
        if (!engine.setupData())
            return;

        const QLatin1String key("CluceneIndexedNamespaces");
        if (reindex)
            engine.setCustomValue(key, QLatin1String(""));

        QMap<QString, QDateTime> indexMap;
        const QLatin1String oldKey("CluceneSearchNamespaces");
        if (!engine.customValue(oldKey, QString()).isNull()) {
            // old style qhc file < 4.4.2, need to convert...
            const QStringList indexedNamespaces
                = engine.customValue(oldKey).toString()
                  .split(QLatin1String("|"), QString::SkipEmptyParts);
            foreach (const QString &nameSpace, indexedNamespaces)
                indexMap.insert(nameSpace, QDateTime());
            engine.removeCustomValue(oldKey);
        } else {
            QDataStream dataStream(engine.customValue(key).toByteArray());
            dataStream >> indexMap;
        }

        QString indexPath = m_indexFilesFolder;

        QFileInfo fInfo(indexPath);
        if (fInfo.exists() && !fInfo.isWritable()) {
            qWarning("Full Text Search, could not create index (missing permissions for '%s').",
                     qPrintable(indexPath));
            return;
        }

        emit indexingStarted();

        QCLuceneIndexWriter *writer = 0;
        QCLuceneStandardAnalyzer analyzer;
        const QStringList registeredDocs = engine.registeredDocumentations();

        QLocalSocket localSocket;
        localSocket.connectToServer(QString(QLatin1String("QtAssistant%1"))
                                    .arg(QLatin1String(QT_VERSION_STR)));

        QLocalServer localServer;
        bool otherInstancesRunning = true;
        if (!localSocket.waitForConnected()) {
            otherInstancesRunning = false;
            localServer.listen(QString(QLatin1String("QtAssistant%1"))
                               .arg(QLatin1String(QT_VERSION_STR)));
        }

        // check if it's locked, and if the other instance is running
        if (!otherInstancesRunning && QCLuceneIndexReader::isLocked(indexPath))
            QCLuceneIndexReader::unlock(indexPath);

        if (QCLuceneIndexReader::isLocked(indexPath)) {
            // poll unless indexing finished to fake progress
            while (QCLuceneIndexReader::isLocked(indexPath)) {
                mutexLocker.relock();
                if (m_cancel)
                    break;
                mutexLocker.unlock();
                this->sleep(1);
            }
            emit indexingFinished();
            return;
        }

        if (QCLuceneIndexReader::indexExists(indexPath) && !reindex) {
            foreach(const QString &namespaceName, registeredDocs) {
                mutexLocker.relock();
                if (m_cancel) {
                    emit indexingFinished();
                    return;
                }
                mutexLocker.unlock();

                if (!indexMap.contains(namespaceName)) {
                    // make sure we remove some partly indexed stuff
                    removeDocuments(indexPath, namespaceName);
                } else {
                    QString path = engine.documentationFileName(namespaceName);
                    if (indexMap.value(namespaceName)
                        < QFileInfo(path).lastModified()) {
                        // make sure we remove some outdated indexed stuff
                        indexMap.remove(namespaceName);
                        removeDocuments(indexPath, namespaceName);
                    }

                    if (indexMap.contains(namespaceName)) {
                        // make sure we really have content indexed for namespace
                        QCLuceneTermQuery query(QCLuceneTerm(NamespaceField, namespaceName));
                        QCLuceneIndexSearcher indexSearcher(indexPath);
                        QCLuceneHits hits = indexSearcher.search(query);
                        if (hits.length() <= 0)
                            indexMap.remove(namespaceName);
                    }
                }
            }
            writer = new QCLuceneIndexWriter(indexPath, analyzer, false);
        } else {
            indexMap.clear();
            writer = new QCLuceneIndexWriter(indexPath, analyzer, true);
        }

        writer->setMergeFactor(100);
        writer->setMinMergeDocs(1000);
        writer->setMaxFieldLength(QCLuceneIndexWriter::DEFAULT_MAX_FIELD_LENGTH);

        QStringList namespaces;
        foreach(const QString &namespaceName, registeredDocs) {
            mutexLocker.relock();
            if (m_cancel) {
                closeIndexWriter(writer);
                emit indexingFinished();
                return;
            }
            mutexLocker.unlock();

            namespaces.append(namespaceName);
            if (indexMap.contains(namespaceName))
                continue;

            const QList<QStringList> attributeSets =
                    engine.filterAttributeSets(namespaceName);

            if (attributeSets.isEmpty()) {
                const QList<QUrl> docFiles = indexableFiles(&engine, namespaceName,
                                                            QStringList());
                if (!addDocuments(docFiles, engine, QStringList(), namespaceName,
                                  writer, analyzer))
                    break;
            } else {
                bool bail = false;
                foreach (const QStringList &attributes, attributeSets) {
                    const QList<QUrl> docFiles = indexableFiles(&engine,
                                                                namespaceName, attributes);
                    if (!addDocuments(docFiles, engine, attributes, namespaceName,
                                      writer, analyzer)) {
                        bail = true;
                        break;
                    }
                }
                if (bail)
                    break;
            }

            mutexLocker.relock();
            if (!m_cancel) {
                QString path(engine.documentationFileName(namespaceName));
                indexMap.insert(namespaceName, QFileInfo(path).lastModified());
                writeIndexMap(engine, indexMap);
            }
            mutexLocker.unlock();
        }

        closeIndexWriter(writer);

        mutexLocker.relock();
        if (!m_cancel) {
            mutexLocker.unlock();

            QStringList indexedNamespaces = indexMap.keys();
            foreach(const QString &namespaceName, indexedNamespaces) {
                mutexLocker.relock();
                if (m_cancel)
                    break;
                mutexLocker.unlock();

                if (!namespaces.contains(namespaceName)) {
                    indexMap.remove(namespaceName);
                    writeIndexMap(engine, indexMap);
                    removeDocuments(indexPath, namespaceName);
                }
            }
        }

#if !defined(QT_NO_EXCEPTIONS)
    } catch (...) {
        qWarning("%s: Failed because of CLucene exception.", Q_FUNC_INFO);
    }
#endif

    emit indexingFinished();
}

bool QHelpSearchIndexWriter::addDocuments(const QList<QUrl> docFiles,
    const QHelpEngineCore &engine, const QStringList &attributes,
    const QString &namespaceName, QCLuceneIndexWriter *writer,
    QCLuceneAnalyzer &analyzer)
{
    QMutexLocker locker(&mutex);
    const QString attrList = attributes.join(QLatin1String(" "));

    locker.unlock();
    foreach(const QUrl &url, docFiles) {
        QCLuceneDocument document;
        DocumentHelper helper(url.toString(), engine.fileData(url));
        if (helper.addFieldsToDocument(&document, namespaceName, attrList)) {
#if !defined(QT_NO_EXCEPTIONS)
            try {
#endif
                writer->addDocument(document, analyzer);
#if !defined(QT_NO_EXCEPTIONS)
            } catch (...) {
                qWarning("Full Text Search, could not properly add documents.");
                return false;
            }
#endif
        }
        locker.relock();
        if (m_cancel)
            return false;
        locker.unlock();
    }
    return true;
}

void QHelpSearchIndexWriter::removeDocuments(const QString &indexPath,
    const QString &namespaceName)
{
    if (namespaceName.isEmpty() || QCLuceneIndexReader::isLocked(indexPath))
        return;

    QCLuceneIndexReader reader = QCLuceneIndexReader::open(indexPath);
    reader.deleteDocuments(QCLuceneTerm(NamespaceField, namespaceName));

    reader.close();
}

bool QHelpSearchIndexWriter::writeIndexMap(QHelpEngineCore &engine,
    const QMap<QString, QDateTime> &indexMap)
{
    QByteArray bArray;

    QDataStream data(&bArray, QIODevice::ReadWrite);
    data << indexMap;

    return engine.setCustomValue(QLatin1String("CluceneIndexedNamespaces"),
        bArray);
}

QList<QUrl> QHelpSearchIndexWriter::indexableFiles(QHelpEngineCore *helpEngine,
    const QString &namespaceName, const QStringList &attributes) const
{
    QList<QUrl> docFiles = helpEngine->files(namespaceName, attributes,
        QLatin1String("html"));
    docFiles += helpEngine->files(namespaceName, attributes, QLatin1String("htm"));
    docFiles += helpEngine->files(namespaceName, attributes, QLatin1String("txt"));

    return docFiles;
}

void QHelpSearchIndexWriter::closeIndexWriter(QCLuceneIndexWriter *writer)
{
#if !defined(QT_NO_EXCEPTIONS)
    try {
#endif
        writer->close();
        delete writer;
#if !defined(QT_NO_EXCEPTIONS)
    } catch (...) {
        qWarning("Full Text Search, could not properly close index writer.");
    }
#endif
}

}   // namespace clucene
}   // namespace fulltextsearch

QT_END_NAMESPACE
