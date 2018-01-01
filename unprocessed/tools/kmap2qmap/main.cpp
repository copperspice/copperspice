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

#include <cstdio>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextCodec>
#include <QList>
#include <QVector>
#include <QByteArray>
#include <QStringList>
#include <QTextStream>

#include "qkbd_qws_p.h"

using namespace std;


struct modifier_map_t {
    const char *symbol;
    quint8      modifier;
    quint32     qtmodifier;
};

static const struct modifier_map_t modifier_map[] = {
    { "plain",   QWSKeyboard::ModPlain,   Qt::NoModifier },
    { "shift",   QWSKeyboard::ModShift,   Qt::ShiftModifier },
    { "altgr",   QWSKeyboard::ModAltGr,   Qt::AltModifier },
    { "control", QWSKeyboard::ModControl, Qt::ControlModifier },
    { "alt",     QWSKeyboard::ModAlt,     Qt::AltModifier },
    { "meta",    QWSKeyboard::ModAlt,     Qt::AltModifier },
    { "shiftl",  QWSKeyboard::ModShiftL,  Qt::ShiftModifier },
    { "shiftr",  QWSKeyboard::ModShiftR,  Qt::ShiftModifier },
    { "ctrll",   QWSKeyboard::ModCtrlL,   Qt::ControlModifier },
    { "ctrlr",   QWSKeyboard::ModCtrlR,   Qt::ControlModifier },
};

static const int modifier_map_size = sizeof(modifier_map)/sizeof(modifier_map_t);


struct symbol_map_t {
    const char *symbol;
    quint32     qtcode;
};

static const struct symbol_map_t symbol_map[] = {
    { "space", Qt::Key_Space },
    { "exclam", Qt::Key_Exclam },
    { "quotedbl", Qt::Key_QuoteDbl },
    { "numbersign", Qt::Key_NumberSign },
    { "dollar", Qt::Key_Dollar },
    { "percent", Qt::Key_Percent },
    { "ampersand", Qt::Key_Ampersand },
    { "apostrophe", Qt::Key_Apostrophe },
    { "parenleft", Qt::Key_ParenLeft },
    { "parenright", Qt::Key_ParenRight },
    { "asterisk", Qt::Key_Asterisk },
    { "plus", Qt::Key_Plus },
    { "comma", Qt::Key_Comma },
    { "minus", Qt::Key_Minus },
    { "period", Qt::Key_Period },
    { "slash", Qt::Key_Slash },
    { "zero", Qt::Key_0 },
    { "one", Qt::Key_1 },
    { "two", Qt::Key_2 },
    { "three", Qt::Key_3 },
    { "four", Qt::Key_4 },
    { "five", Qt::Key_5 },
    { "six", Qt::Key_6 },
    { "seven", Qt::Key_7 },
    { "eight", Qt::Key_8 },
    { "nine", Qt::Key_9 },
    { "colon", Qt::Key_Colon },
    { "semicolon", Qt::Key_Semicolon },
    { "less", Qt::Key_Less },
    { "equal", Qt::Key_Equal },
    { "greater", Qt::Key_Greater },
    { "question", Qt::Key_Question },
    { "at", Qt::Key_At },
    { "bracketleft", Qt::Key_BracketLeft },
    { "backslash", Qt::Key_Backslash },
    { "bracketright", Qt::Key_BracketRight },
    { "asciicircum", Qt::Key_AsciiCircum },
    { "underscore", Qt::Key_Underscore },
    { "grave", Qt::Key_QuoteLeft },
    { "braceleft", Qt::Key_BraceLeft },
    { "bar", Qt::Key_Bar },
    { "braceright", Qt::Key_BraceRight },
    { "asciitilde", Qt::Key_AsciiTilde },
    { "nobreakspace", Qt::Key_nobreakspace },
    { "exclamdown", Qt::Key_exclamdown },
    { "cent", Qt::Key_cent },
    { "sterling", Qt::Key_sterling },
    { "currency", Qt::Key_currency },
    { "yen", Qt::Key_yen },
    { "brokenbar", Qt::Key_brokenbar },
    { "section", Qt::Key_section },
    { "diaeresis", Qt::Key_diaeresis },
    { "copyright", Qt::Key_copyright },
    { "ordfeminine", Qt::Key_ordfeminine },
    { "guillemotleft", Qt::Key_guillemotleft },
    { "notsign", Qt::Key_notsign },
    { "hyphen", Qt::Key_hyphen },
    { "registered", Qt::Key_registered },
    { "macron", Qt::Key_macron },
    { "degree", Qt::Key_degree },
    { "plusminus", Qt::Key_plusminus },
    { "twosuperior", Qt::Key_twosuperior },
    { "threesuperior", Qt::Key_threesuperior },
    { "acute", Qt::Key_acute },
    { "mu", Qt::Key_mu },
    { "paragraph", Qt::Key_paragraph },
    { "periodcentered", Qt::Key_periodcentered },
    { "cedilla", Qt::Key_cedilla },
    { "onesuperior", Qt::Key_onesuperior },
    { "masculine", Qt::Key_masculine },
    { "guillemotright", Qt::Key_guillemotright },
    { "onequarter", Qt::Key_onequarter },
    { "onehalf", Qt::Key_onehalf },
    { "threequarters", Qt::Key_threequarters },
    { "questiondown", Qt::Key_questiondown },
    { "Agrave", Qt::Key_Agrave },
    { "Aacute", Qt::Key_Aacute },
    { "Acircumflex", Qt::Key_Acircumflex },
    { "Atilde", Qt::Key_Atilde },
    { "Adiaeresis", Qt::Key_Adiaeresis },
    { "Aring", Qt::Key_Aring },
    { "AE", Qt::Key_AE },
    { "Ccedilla", Qt::Key_Ccedilla },
    { "Egrave", Qt::Key_Egrave },
    { "Eacute", Qt::Key_Eacute },
    { "Ecircumflex", Qt::Key_Ecircumflex },
    { "Ediaeresis", Qt::Key_Ediaeresis },
    { "Igrave", Qt::Key_Igrave },
    { "Iacute", Qt::Key_Iacute },
    { "Icircumflex", Qt::Key_Icircumflex },
    { "Idiaeresis", Qt::Key_Idiaeresis },
    { "ETH", Qt::Key_ETH },
    { "Ntilde", Qt::Key_Ntilde },
    { "Ograve", Qt::Key_Ograve },
    { "Oacute", Qt::Key_Oacute },
    { "Ocircumflex", Qt::Key_Ocircumflex },
    { "Otilde", Qt::Key_Otilde },
    { "Odiaeresis", Qt::Key_Odiaeresis },
    { "multiply", Qt::Key_multiply },
    { "Ooblique", Qt::Key_Ooblique },
    { "Ugrave", Qt::Key_Ugrave },
    { "Uacute", Qt::Key_Uacute },
    { "Ucircumflex", Qt::Key_Ucircumflex },
    { "Udiaeresis", Qt::Key_Udiaeresis },
    { "Yacute", Qt::Key_Yacute },
    { "THORN", Qt::Key_THORN },
    { "ssharp", Qt::Key_ssharp },

    { "agrave", 0xe0 /*Qt::Key_agrave*/ },
    { "aacute", 0xe1 /*Qt::Key_aacute*/ },
    { "acircumflex", 0xe2 /*Qt::Key_acircumflex*/ },
    { "atilde", 0xe3 /*Qt::Key_atilde*/ },
    { "adiaeresis", 0xe4 /*Qt::Key_adiaeresis*/ },
    { "aring", 0xe5 /*Qt::Key_aring*/ },
    { "ae", 0xe6 /*Qt::Key_ae*/ },
    { "ccedilla", 0xe7 /*Qt::Key_ccedilla*/ },
    { "egrave", 0xe8 /*Qt::Key_egrave*/ },
    { "eacute", 0xe9 /*Qt::Key_eacute*/ },
    { "ecircumflex", 0xea /*Qt::Key_ecircumflex*/ },
    { "ediaeresis", 0xeb /*Qt::Key_ediaeresis*/ },
    { "igrave", 0xec /*Qt::Key_igrave*/ },
    { "iacute", 0xed /*Qt::Key_iacute*/ },
    { "icircumflex", 0xee /*Qt::Key_icircumflex*/ },
    { "idiaeresis", 0xef /*Qt::Key_idiaeresis*/ },
    { "eth", 0xf0 /*Qt::Key_eth*/ },
    { "ntilde", 0xf1 /*Qt::Key_ntilde*/ },
    { "ograve", 0xf2 /*Qt::Key_ograve*/ },
    { "oacute", 0xf3 /*Qt::Key_oacute*/ },
    { "ocircumflex", 0xf4 /*Qt::Key_ocircumflex*/ },
    { "otilde", 0xf5 /*Qt::Key_otilde*/ },
    { "odiaeresis", 0xf6 /*Qt::Key_odiaeresis*/ },
    { "division", Qt::Key_division },
    { "oslash", 0xf8 /*Qt::Key_oslash*/ },
    { "ugrave", 0xf9 /*Qt::Key_ugrave*/ },
    { "uacute", 0xfa /*Qt::Key_uacute*/ },
    { "ucircumflex", 0xfb /*Qt::Key_ucircumflex*/ },
    { "udiaeresis", 0xfc /*Qt::Key_udiaeresis*/ },
    { "yacute", 0xfd /*Qt::Key_yacute*/ },
    { "thorn", 0xfe /*Qt::Key_thorn*/ },
    { "ydiaeresis", Qt::Key_ydiaeresis },

    { "F1",  Qt::Key_F1 },
    { "F2",  Qt::Key_F2 },
    { "F3",  Qt::Key_F3 },
    { "F4",  Qt::Key_F4 },
    { "F5",  Qt::Key_F5 },
    { "F6",  Qt::Key_F6 },
    { "F7",  Qt::Key_F7 },
    { "F8",  Qt::Key_F8 },
    { "F9",  Qt::Key_F9 },
    { "F10", Qt::Key_F10 },
    { "F11", Qt::Key_F11 },
    { "F12", Qt::Key_F12 },
    { "F13", Qt::Key_F13 },
    { "F14", Qt::Key_F14 },
    { "F15", Qt::Key_F15 },
    { "F16", Qt::Key_F16 },
    { "F17", Qt::Key_F17 },
    { "F18", Qt::Key_F18 },
    { "F19", Qt::Key_F19 },
    { "F20", Qt::Key_F20 },
    { "F21", Qt::Key_F21 },
    { "F22", Qt::Key_F22 },
    { "F23", Qt::Key_F23 },
    { "F24", Qt::Key_F24 },
    { "F25", Qt::Key_F25 },
    { "F26", Qt::Key_F26 },
    { "F27", Qt::Key_F27 },
    { "F28", Qt::Key_F28 },
    { "F29", Qt::Key_F29 },
    { "F30", Qt::Key_F30 },
    { "F31", Qt::Key_F31 },
    { "F32", Qt::Key_F32 },
    { "F33", Qt::Key_F33 },
    { "F34", Qt::Key_F34 },
    { "F35", Qt::Key_F35 },

    { "BackSpace",     Qt::Key_Backspace },
    { "Tab",           Qt::Key_Tab },
    { "Escape",        Qt::Key_Escape },
    { "Delete",        Qt::Key_Backspace }, // what's the difference between "Delete" and "BackSpace"??
    { "Return",        Qt::Key_Return },
    { "Break",         Qt::Key_unknown }, //TODO: why doesn't Qt support the 'Break' key?
    { "Caps_Lock",     Qt::Key_CapsLock },
    { "Num_Lock",      Qt::Key_NumLock },
    { "Scroll_Lock",   Qt::Key_ScrollLock },
    { "Caps_On",       Qt::Key_CapsLock },
    { "Compose",       Qt::Key_Multi_key },
    { "Bare_Num_Lock", Qt::Key_NumLock },
    { "Find",          Qt::Key_Home },
    { "Insert",        Qt::Key_Insert },
    { "Remove",        Qt::Key_Delete },
    { "Select",        Qt::Key_End },
    { "Prior",         Qt::Key_PageUp },
    { "Next",          Qt::Key_PageDown },
    { "Help",          Qt::Key_Help },
    { "Pause",         Qt::Key_Pause },

    { "KP_0",        Qt::Key_0 | Qt::KeypadModifier },
    { "KP_1",        Qt::Key_1 | Qt::KeypadModifier },
    { "KP_2",        Qt::Key_2 | Qt::KeypadModifier },
    { "KP_3",        Qt::Key_3 | Qt::KeypadModifier },
    { "KP_4",        Qt::Key_4 | Qt::KeypadModifier },
    { "KP_5",        Qt::Key_5 | Qt::KeypadModifier },
    { "KP_6",        Qt::Key_6 | Qt::KeypadModifier },
    { "KP_7",        Qt::Key_7 | Qt::KeypadModifier },
    { "KP_8",        Qt::Key_8 | Qt::KeypadModifier },
    { "KP_9",        Qt::Key_9 | Qt::KeypadModifier },
    { "KP_Add",      Qt::Key_Plus | Qt::KeypadModifier },
    { "KP_Subtract", Qt::Key_Minus | Qt::KeypadModifier },
    { "KP_Multiply", Qt::Key_Asterisk | Qt::KeypadModifier },
    { "KP_Divide",   Qt::Key_Slash | Qt::KeypadModifier },
    { "KP_Enter",    Qt::Key_Enter | Qt::KeypadModifier },
    { "KP_Comma",    Qt::Key_Comma | Qt::KeypadModifier },
    { "KP_Period",   Qt::Key_Period | Qt::KeypadModifier },
    { "KP_MinPlus",  Qt::Key_plusminus | Qt::KeypadModifier },

    { "dead_grave",      Qt::Key_Dead_Grave },
    { "dead_acute",      Qt::Key_Dead_Acute },
    { "dead_circumflex", Qt::Key_Dead_Circumflex },
    { "dead_tilde",      Qt::Key_Dead_Tilde },
    { "dead_diaeresis",  Qt::Key_Dead_Diaeresis },
    { "dead_cedilla",    Qt::Key_Dead_Cedilla },

    { "Down",    Qt::Key_Down },
    { "Left",    Qt::Key_Left },
    { "Right",   Qt::Key_Right },
    { "Up",      Qt::Key_Up },
    { "Shift",   Qt::Key_Shift },
    { "AltGr",   Qt::Key_AltGr },
    { "Control", Qt::Key_Control },
    { "Alt",     Qt::Key_Alt },
    { "ShiftL",  Qt::Key_Shift },
    { "ShiftR",  Qt::Key_Shift },
    { "CtrlL",   Qt::Key_Control },
    { "CtrlR",   Qt::Key_Control },
};

static const int symbol_map_size = sizeof(symbol_map)/sizeof(symbol_map_t);


struct symbol_dead_unicode_t {
    quint32 dead;
    quint16 unicode;
};

static const symbol_dead_unicode_t symbol_dead_unicode[] = {
    { Qt::Key_Dead_Grave,      '`' },
    { Qt::Key_Dead_Acute,      '\'' },
    { Qt::Key_Dead_Circumflex, '^' },
    { Qt::Key_Dead_Tilde,      '~' },
    { Qt::Key_Dead_Diaeresis,  '"' },
    { Qt::Key_Dead_Cedilla,    ',' },
};

static const int symbol_dead_unicode_size = sizeof(symbol_dead_unicode)/sizeof(symbol_dead_unicode_t);


struct symbol_synonyms_t {
    const char *from;
    const char *to;
};

static const symbol_synonyms_t symbol_synonyms[] = {
    { "Control_h", "BackSpace" },
    { "Control_i", "Tab" },
    { "Control_j", "Linefeed" },
    { "Home", "Find" },
    { "End", "Select" },
    { "PageUp", "Prior" },
    { "PageDown", "Next" },
    { "multiplication", "multiply" },
    { "pound", "sterling" },
    { "pilcrow", "paragraph" },
    { "Oslash", "Ooblique" },
    { "Shift_L", "ShiftL" },
    { "Shift_R", "ShiftR" },
    { "Control_L", "CtrlL" },
    { "Control_R", "CtrlR" },
    { "AltL", "Alt" },
    { "AltR", "AltGr" },
    { "Alt_L", "Alt" },
    { "Alt_R", "AltGr" },
    { "AltGr_L", "Alt" },
    { "AltGr_R", "AltGr" },
    { "tilde", "asciitilde" },
    { "circumflex", "asciicircum" },
    { "dead_ogonek", "dead_cedilla" },
    { "dead_caron", "dead_circumflex" },
    { "dead_breve", "dead_tilde" },
    { "dead_doubleacute", "dead_tilde" },
    { "no-break_space", "nobreakspace" },
    { "paragraph_sign", "section" },
    { "soft_hyphen", "hyphen" },
    { "rightanglequote", "guillemotright" },
};

static const int symbol_synonyms_size = sizeof(symbol_synonyms)/sizeof(symbol_synonyms_t);

// makes the generated array in --header mode a bit more human readable
QT_BEGIN_NAMESPACE
static bool operator<(const QWSKeyboard::Mapping &m1, const QWSKeyboard::Mapping &m2)
{
    return m1.keycode != m2.keycode ? m1.keycode < m2.keycode : m1.modifiers < m2.modifiers;
}
QT_END_NAMESPACE

class KeymapParser {
public:
    KeymapParser();
    ~KeymapParser();

    bool parseKmap(QFile *kmap);
    bool generateQmap(QFile *qmap);
    bool generateHeader(QFile *qmap);

    int parseWarningCount() const   { return m_warning_count; }

private:
    bool parseSymbol(const QByteArray &str, const QTextCodec *codec, quint16 &unicode, quint32 &qtcode, quint8 &flags, quint16 &special);
    bool parseCompose(const QByteArray &str, const QTextCodec *codec, quint16 &unicode);
    bool parseModifier(const QByteArray &str, quint8 &modifier);

    void updateMapping(quint16 keycode = 0, quint8 modifiers = 0, quint16 unicode = 0xffff, quint32 qtcode = Qt::Key_unknown, quint8 flags = 0, quint16 = 0);

    static quint32 toQtModifiers(quint8 modifiers);
    static QList<QByteArray> tokenize(const QByteArray &line);


private:
    QList<QWSKeyboard::Mapping> m_keymap;
    QList<QWSKeyboard::Composing> m_keycompose;

    int m_warning_count;
};



int main(int argc, char **argv)
{
    int header = 0;
    if (argc >= 2 && !qstrcmp(argv[1], "--header"))
        header = 1;

    if (argc < (3 + header)) {
        fprintf(stderr, "Usage: kmap2qmap [--header] <kmap> [<additional kmaps> ...] <qmap>\n");
        fprintf(stderr, "  --header   can be used to generate Qt's default compiled in qmap.\n");
        return 1;
    }

    QVector<QFile *> kmaps(argc - header - 2);
    for (int i = 0; i < kmaps.size(); ++i) {
        kmaps [i] = new QFile(QString::fromLocal8Bit(argv[i + 1 + header]));

        if (!kmaps[i]->open(QIODevice::ReadOnly)) {
            fprintf(stderr, "Could not read from '%s'.\n", argv[i + 1 + header]);
            return 2;
        }
    }
    QFile *qmap = new QFile(QString::fromLocal8Bit(argv[argc - 1]));

    if (!qmap->open(QIODevice::WriteOnly)) {
        fprintf(stderr, "Could not write to '%s'.\n", argv[argc - 1]);
        return 3;
    }

    KeymapParser p;

    for (int i = 0; i < kmaps.size(); ++i) {
        if (!p.parseKmap(kmaps[i])) {
            fprintf(stderr, "Parsing kmap '%s' failed.\n", qPrintable(kmaps[i]->fileName()));
            return 4;
        }
    }

    if (p.parseWarningCount()) {
        fprintf(stderr, "\nParsing the specified keymap(s) produced %d warning(s).\n" \
                        "Your generated qmap might not be complete.\n", \
                        p.parseWarningCount());
    }
    if (!(header ? p.generateHeader(qmap) : p.generateQmap(qmap))) {
        fprintf(stderr, "Generating the qmap failed.\n");
        return 5;
    }

    qDeleteAll(kmaps);
    delete qmap;

    return 0;
}


KeymapParser::KeymapParser()
     : m_warning_count(0)
{ }


KeymapParser::~KeymapParser()
{ }


bool KeymapParser::generateHeader(QFile *f)
{
    QTextStream ts(f);

    ts << "#ifndef QWSKEYBOARDHANDLER_DEFAULTMAP_H" << endl;
    ts << "#define QWSKEYBOARDHANDLER_DEFAULTMAP_H" << endl << endl;

    ts << "const QWSKeyboard::Mapping QWSKbPrivate::s_keymap_default[] = {" << endl;

    for (int i = 0; i < m_keymap.size(); ++i) {
        const QWSKeyboard::Mapping &m = m_keymap.at(i);
        QString s;
        s.sprintf("    { %3d, 0x%04x, 0x%08x, 0x%02x, 0x%02x, 0x%04x },\n", m.keycode, m.unicode, m.qtcode, m.modifiers, m.flags, m.special);
        ts << s;
    }

    ts << "};" << endl << endl;

    ts << "const QWSKeyboard::Composing QWSKbPrivate::s_keycompose_default[] = {" << endl;

    for (int i = 0; i < m_keycompose.size(); ++i) {
        const QWSKeyboard::Composing &c = m_keycompose.at(i);
        QString s;
        s.sprintf("    { 0x%04x, 0x%04x, 0x%04x },\n", c.first, c.second, c.result);
        ts << s;
    }
    ts << "};" << endl << endl;

    ts << "#endif" << endl;

    return (ts.status() == QTextStream::Ok);
}


bool KeymapParser::generateQmap(QFile *f)
{
    QDataStream ds(f);

    ds << quint32(QWSKeyboard::FileMagic) << quint32(1 /* version */) << quint32(m_keymap.size()) << quint32(m_keycompose.size());

    if (ds.status() != QDataStream::Ok)
        return false;

    for (int i = 0; i < m_keymap.size(); ++i)
        ds << m_keymap[i];

    for (int i = 0; i < m_keycompose.size(); ++i)
        ds << m_keycompose[i];

    return (ds.status() == QDataStream::Ok);
}


QList<QByteArray> KeymapParser::tokenize(const QByteArray &line)
{
    bool quoted = false, separator = true;
    QList<QByteArray> result;
    QByteArray token;

    for (int i = 0; i < line.length(); ++i) {
        QChar c = line.at(i);

        if (!quoted && c == '#' && separator)
            break;
        else if (!quoted && c == '"' && separator)
            quoted = true;
        else if (quoted && c == '"')
            quoted = false;
        else if (!quoted && c.isSpace()) {
            separator = true;
            if (!token.isEmpty()) {
                result.append(token);
                token.truncate(0);
            }
        }
        else {
            separator = false;
            token.append(c);
        }
    }
    if (!token.isEmpty())
        result.append(token);
    return result;
}


#define parseWarning(s)  do { qWarning("Warning: keymap file '%s', line %d: %s", qPrintable(f->fileName()), lineno, s); ++m_warning_count; } while (false)

bool KeymapParser::parseKmap(QFile *f)
{
    QByteArray line;
    int lineno = 0;
    QList<int> keymaps;
    QTextCodec *codec = QTextCodec::codecForName("iso8859-1");

    for (int i = 0; i <= 256; ++i)
        keymaps << i;

    while (!f->atEnd() && !f->error()) {
        line = f->readLine();
        lineno++;

        QList<QByteArray> tokens = tokenize(line);

        if (tokens.isEmpty())
            continue;

        if (tokens[0] == "keymaps") {
            keymaps.clear();

            if (tokens.count() > 1) {
                foreach (const QByteArray &section, tokens[1].split(',')) {
                    int dashpos = section.indexOf('-');

                    //qWarning("Section %s", section.constData());
                    int end = section.mid(dashpos + 1).toInt();
                    int start = end;
                    if (dashpos > 0)
                        start = section.left(dashpos).toInt();

                    if (start <= end && start >=0 && end <= 256) {
                        for (int i = start; i <= end; ++i) {
                            //qWarning("appending keymap %d", i);
                            keymaps.append(i);
                        }
                    }
                    else
                        parseWarning("keymaps has an invalid range");
                }
                qSort(keymaps);
            }
            else
                parseWarning("keymaps with more than one argument");
        }
        else if (tokens[0] == "alt_is_meta") {
            // simply ignore it for now
        }
        else if (tokens[0] == "include") {
            if (tokens.count() == 2) {
                QString incname = QString::fromLocal8Bit(tokens[1]);
                bool found = false;
                QList<QDir> searchpath;
                QFileInfo fi(*f);

                if (!incname.endsWith(QLatin1String(".kmap")) && !incname.endsWith(QLatin1String(".inc")))
                    incname.append(QLatin1String(".inc"));

                QDir d = fi.dir();
                searchpath << d;
                if (d.cdUp() && d.cd(QLatin1String("include")))
                    searchpath << d;
                searchpath << QDir::current();

                foreach (const QDir &path, searchpath) {
                    QFile f2(path.filePath(incname));
                    //qWarning("  -- trying to include %s", qPrintable(f2.fileName()));
                    if (f2.open(QIODevice::ReadOnly)) {
                        if (!parseKmap(&f2))
                            parseWarning("could not parse keymap include");
                        found = true;
                    }
                }

                if (!found)
                    parseWarning("could not locate keymap include");
            } else
                parseWarning("include doesn't have exactly one argument");
        }
        else if (tokens[0] == "charset") {
            if (tokens.count() == 2) {
                codec = QTextCodec::codecForName(tokens[1]);
                if (!codec) {
                    parseWarning("could not parse codec definition");
                    codec = QTextCodec::codecForName("iso8859-1");
                }
            } else
                parseWarning("codec doesn't habe exactly one argument");
        }
        else if (tokens[0] == "strings") {
            // simply ignore those - they have no meaning for QWS
        }
        else if (tokens[0] == "compose") {
            if (tokens.count() == 5 && tokens[3] == "to") {
                QWSKeyboard::Composing c = { 0xffff, 0xffff, 0xffff };

                if (!parseCompose(tokens[1], codec, c.first))
                    parseWarning("could not parse first compose symbol");
                if (!parseCompose(tokens[2], codec, c.second))
                    parseWarning("could not parse second compose symbol");
                if (!parseCompose(tokens[4], codec, c.result))
                    parseWarning("could not parse resulting compose symbol");

                if (c.first != 0xffff && c.second != 0xffff && c.result != 0xffff) {
                    m_keycompose << c;
                }
            } else
                parseWarning("non-standard compose line");
        }
        else {
            int kcpos = tokens.indexOf("keycode");

            if (kcpos >= 0 && kcpos < (tokens.count()-3) && tokens[kcpos+2] == "=") {
                quint16 keycode = tokens[kcpos+1].toInt();

                if (keycode <= 0 || keycode > 0x1ff /* KEY_MAX */) {
                    parseWarning("keycode out of range [0..0x1ff]");
                    break;
                }

                bool line_modifiers = (kcpos > 0);

                quint8 modifiers = 0; //, modifiers_mask = 0xff;
                for (int i = 0; i < kcpos; ++i) {
                    quint8 mod;
                    if (!parseModifier(tokens[i], mod)) {
                        parseWarning("unknown modifier prefix for keycode");
                        continue;
                    }
                    modifiers |= mod;
                }

                int kccount = tokens.count() - kcpos - 3; // 3 : 'keycode' 'X' '='

                if (line_modifiers && kccount > 1) {
                    parseWarning("line has modifiers, but more than one keycode");
                    break;
                }

                // only process one symbol when a prefix modifer was specified
                for (int i = 0; i < (line_modifiers ? 1 : kccount); ++i) {
                    if (!line_modifiers)
                        modifiers = keymaps[i];

                    quint32 qtcode;
                    quint16 unicode;
                    quint16 special;
                    quint8 flags;
                    if (!parseSymbol(tokens[i + kcpos + 3], codec, unicode, qtcode, flags, special)) {
                        parseWarning((QByteArray("symbol could not be parsed: ") + tokens[i + kcpos + 3]).constData());
                        break;
                    }

                    if (qtcode == Qt::Key_unknown && unicode == 0xffff) // VoidSymbol
                        continue;

                    if (!line_modifiers && kccount == 1) {
                        if ((unicode >= 'A' && unicode <= 'Z') || (unicode >= 'a' && unicode <= 'z')) {
                            quint16 other_unicode = (unicode >= 'A' && unicode <= 'Z') ? unicode - 'A' + 'a' : unicode - 'a' + 'A';
                            quint16 lower_unicode = (unicode >= 'A' && unicode <= 'Z') ? unicode - 'A' + 'a' : unicode;

                            // a single a-z|A-Z value results in a very flags mapping: see below

                            updateMapping(keycode, QWSKeyboard::ModPlain, unicode, qtcode, flags, 0);

                            updateMapping(keycode, QWSKeyboard::ModShift, other_unicode, qtcode, flags, 0);

                            updateMapping(keycode, QWSKeyboard::ModAltGr,                         unicode, qtcode, flags, 0);
                            updateMapping(keycode, QWSKeyboard::ModAltGr | QWSKeyboard::ModShift, other_unicode, qtcode, flags, 0);

                            updateMapping(keycode, QWSKeyboard::ModControl,                                                 lower_unicode, qtcode | Qt::ControlModifier, flags, 0);
                            updateMapping(keycode, QWSKeyboard::ModControl | QWSKeyboard::ModShift,                         lower_unicode, qtcode | Qt::ControlModifier, flags, 0);
                            updateMapping(keycode, QWSKeyboard::ModControl | QWSKeyboard::ModAltGr,                         lower_unicode, qtcode | Qt::ControlModifier, flags, 0);
                            updateMapping(keycode, QWSKeyboard::ModControl | QWSKeyboard::ModAltGr | QWSKeyboard::ModShift, lower_unicode, qtcode | Qt::ControlModifier, flags, 0);

                            updateMapping(keycode, QWSKeyboard::ModAlt,                                                 unicode, qtcode | Qt::AltModifier, flags, 0);
                            updateMapping(keycode, QWSKeyboard::ModAlt | QWSKeyboard::ModShift,                         unicode, qtcode | Qt::AltModifier, flags, 0);
                            updateMapping(keycode, QWSKeyboard::ModAlt | QWSKeyboard::ModAltGr,                         unicode, qtcode | Qt::AltModifier, flags, 0);
                            updateMapping(keycode, QWSKeyboard::ModAlt | QWSKeyboard::ModAltGr | QWSKeyboard::ModShift, unicode, qtcode | Qt::AltModifier, flags, 0);

                            updateMapping(keycode, QWSKeyboard::ModAlt | QWSKeyboard::ModControl,                                                 lower_unicode, qtcode | Qt::ControlModifier | Qt::AltModifier, flags, 0);
                            updateMapping(keycode, QWSKeyboard::ModAlt | QWSKeyboard::ModControl | QWSKeyboard::ModShift,                         lower_unicode, qtcode | Qt::ControlModifier | Qt::AltModifier, flags, 0);
                            updateMapping(keycode, QWSKeyboard::ModAlt | QWSKeyboard::ModControl | QWSKeyboard::ModAltGr,                         lower_unicode, qtcode | Qt::ControlModifier | Qt::AltModifier, flags, 0);
                            updateMapping(keycode, QWSKeyboard::ModAlt | QWSKeyboard::ModControl | QWSKeyboard::ModAltGr | QWSKeyboard::ModShift, lower_unicode, qtcode | Qt::ControlModifier | Qt::AltModifier, flags, 0);
                        }
                        else {
                            // a single value results in that mapping regardless of the modifier
                            //for (int mod = 0; mod <= 255; ++mod)
                            //    updateMapping(keycode, quint8(mod), unicode, qtcode | toQtModifiers(mod), flags, special);

                            // we can save a lot of space in the qmap, since we do that anyway in the kbd handler:
                            updateMapping(keycode, QWSKeyboard::ModPlain, unicode, qtcode, flags, special);
                        }
                    }
                    else {
                        // "normal" mapping
                        updateMapping(keycode, modifiers, unicode, qtcode, flags, special);
                    }
                }
            }
        }
    }
    qSort(m_keymap);
    return !m_keymap.isEmpty();
}



void KeymapParser::updateMapping(quint16 keycode, quint8 modifiers, quint16 unicode, quint32 qtcode, quint8 flags, quint16 special)
{
    for (int i = 0; i < m_keymap.size(); ++i) {
        QWSKeyboard::Mapping &m = m_keymap[i];

        if (m.keycode == keycode && m.modifiers == modifiers) {
            m.unicode = unicode;
            m.qtcode  = qtcode;
            m.flags   = flags;
            m.special = special;
            return;
        }
    }
    QWSKeyboard::Mapping m = { keycode, unicode, qtcode, modifiers, flags, special };
    m_keymap << m;
}


quint32 KeymapParser::toQtModifiers(quint8 modifiers)
{
    quint32 qtmodifiers = Qt::NoModifier;

    for (int i = 0; i < modifier_map_size; ++i) {
        if (modifiers & modifier_map[i].modifier)
            qtmodifiers |= modifier_map[i].qtmodifier;
    }
    return qtmodifiers;
}


bool KeymapParser::parseModifier(const QByteArray &str, quint8 &modifier)
{
    QByteArray lstr = str.toLower();

    for (int i = 0; i < modifier_map_size; ++i) {
        if (lstr == modifier_map[i].symbol) {
            modifier = modifier_map[i].modifier;
            return true;
        }
    }
    return false;
}


bool KeymapParser::parseCompose(const QByteArray &str, const QTextCodec *codec, quint16 &unicode)
{
    if (str == "'\\''") {
        unicode = '\'';
        return true;
    } else if (str.length() == 3 && str.startsWith('\'') && str.endsWith('\'')) {
        QString temp = codec->toUnicode(str.constData() + 1, str.length() - 2);
        if (temp.length() != 1)
            return false;
        unicode = temp[0].unicode();
        return true;
    } else {
        quint32 code = str.toUInt();
        if (code > 255)
            return false;
        char c[2];
        c[0] = char(code);
        c[1] = 0;
        QString temp = codec->toUnicode(c);
        if (temp.length() != 1)
            return false;
        unicode = temp[0].unicode();
        return true;
    }
}


bool KeymapParser::parseSymbol(const QByteArray &str, const QTextCodec * /*codec*/, quint16 &unicode, quint32 &qtcode, quint8 &flags, quint16 &special)
{
    flags = (str[0] == '+') ? QWSKeyboard::IsLetter : 0;
    QByteArray sym = (flags & QWSKeyboard::IsLetter) ? str.right(str.length() - 1) : str;

    special = 0;
    qtcode = Qt::Key_unknown;
    unicode = 0xffff;

    if (sym == "VoidSymbol" || sym == "nul")
        return true;

    bool try_to_find_qtcode = false;

    if (sym[0] >= '0' && sym[0] <= '9') {  // kernel internal action number
        return false;
    } else if (sym.length() == 6 && sym[1] == '+' && (sym[0] == 'U' || sym[0] == 'u')) { // unicode
        bool ok;
        unicode = sym.mid(2).toUInt(&ok, 16);
        if (!ok)
            return false;
        try_to_find_qtcode = true;
    } else { // symbolic
        for (int i = 0; i < symbol_synonyms_size; ++i) {
            if (sym == symbol_synonyms[i].from) {
                sym = symbol_synonyms[i].to;
                break;
            }
        }

        quint32 qtmod = 0;

        // parse prepended modifiers
        forever {
            int underpos = sym.indexOf('_');

            if (underpos <= 0)
                break;
            QByteArray modsym = sym.left(underpos);
            QByteArray nomodsym = sym.mid(underpos + 1);
            quint8 modifier = 0;

            if (!parseModifier(modsym, modifier))
                break;

            qtmod |= toQtModifiers(modifier);
            sym = nomodsym;
        }

        if (qtcode == Qt::Key_unknown) {
            quint8 modcode;
            // check if symbol is a modifier
            if (parseModifier(sym, modcode)) {
                special = modcode;
                flags |= QWSKeyboard::IsModifier;
            }

            // map symbol to Qt key code
            for  (int i = 0; i < symbol_map_size; ++i) {
                if (sym == symbol_map[i].symbol) {
                    qtcode = symbol_map[i].qtcode;
                    break;
                }
            }

            // a-zA-Z is not in the table to save space
            if (qtcode == Qt::Key_unknown && sym.length() == 1) {
                char letter = sym.at(0);

                if (letter >= 'a' && letter <= 'z') {
                    qtcode = Qt::Key_A + letter - 'a';
                    unicode = letter;
                }
                else if (letter >= 'A' && letter <= 'Z') {
                    qtcode = Qt::Key_A + letter - 'A';
                    unicode = letter;
                }
            }
            // System keys
            if (qtcode == Qt::Key_unknown) {
                quint16 sys = 0;

                if (sym == "Decr_Console") {
                    sys = QWSKeyboard::SystemConsolePrevious;
                } else if (sym == "Incr_Console") {
                    sys = QWSKeyboard::SystemConsoleNext;
                } else if (sym.startsWith("Console_")) {
                    int console = sym.mid(8).toInt() - 1;
                    if (console >= 0 && console <= (QWSKeyboard::SystemConsoleLast - QWSKeyboard::SystemConsoleFirst)) {
                        sys = QWSKeyboard::SystemConsoleFirst + console;
                    }
                } else if (sym == "Boot") {
                    sys = QWSKeyboard::SystemReboot;
                } else if (sym == "QtZap") {
                    sys = QWSKeyboard::SystemZap;
                }

                if (sys) {
                    flags |= QWSKeyboard::IsSystem;
                    special = sys;
                    qtcode = Qt::Key_Escape; // just a dummy
                }
            }

            // map Qt key codes in the iso-8859-1 range to unicode
            if (qtcode != Qt::Key_unknown && unicode == 0xffff) {
                quint32 qtcode_no_mod = qtcode & ~(Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier);
                 if (qtcode_no_mod <= 0x000000ff) // iso-8859-1
                     unicode = quint16(qtcode_no_mod);
            }

            // flag dead keys
            if (qtcode >= Qt::Key_Dead_Grave && qtcode <= Qt::Key_Dead_Horn) {
                flags = QWSKeyboard::IsDead;

                for (int i = 0; i < symbol_dead_unicode_size; ++i) {
                    if (symbol_dead_unicode[i].dead == qtcode) {
                        unicode = symbol_dead_unicode[i].unicode;
                        break;
                    }
                }
            }
        }
        if ((qtcode == Qt::Key_unknown) && (unicode == 0xffff))
            return false;

        qtcode |= qtmod;
    }

    // map unicode in the iso-8859-1 range to Qt key codes
    if (unicode >= 0x0020 && unicode <= 0x00ff && qtcode == Qt::Key_unknown)
        qtcode = unicode; // iso-8859-1

    return true;
}

