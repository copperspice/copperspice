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

#include <qbasicfontdatabase_p.h>

#if defined(QT_USE_FREETYPE)

#include <qplatform_screen.h>
#include <qdir.h>
#include <qfile.h>
#include <qendian.h>
#include <qlibraryinfo.h>
#include <quuid.h>

#include <qapplication_p.h>
#include <qfontengine_ft_p.h>

#include <ft2build.h>
#include FT_TRUETYPE_TABLES_H
#include FT_ERRORS_H

void QBasicFontDatabase::populateFontDatabase()
{
    QString fontpath = fontDir();
    QDir dir(fontpath);

    if (! dir.exists()) {
       qWarning("QBasicFontDatabase::populateFontDatabase() Unable to locate the font directory %s", csPrintable(fontpath));
       return;
    }

    QStringList nameFilters;
    nameFilters << QString("*.ttf")
                << QString("*.ttc")
                << QString("*.pfa")
                << QString("*.pfb")
                << QString("*.otf");

    for (const QFileInfo &fi : dir.entryInfoList(nameFilters, QDir::Files)) {
       const QByteArray file = QFile::encodeName(fi.absoluteFilePath());
       QBasicFontDatabase::addTTFile(QByteArray(), file);
    }
}

QFontEngine *QBasicFontDatabase::fontEngine(const QFontDef &fontDef, void *usrPtr)
{
    FontFile *fontfile = static_cast<FontFile *> (usrPtr);

    QFontEngine::FaceId fid;
    fid.filename = fontfile->fileName;
    fid.index    = fontfile->indexValue;

    bool antialias = ! (fontDef.styleStrategy & QFont::NoAntialias);
    QFontEngineFT *engine = new QFontEngineFT(fontDef);
    QFontEngineFT::GlyphFormat format = QFontEngineFT::Format_Mono;

    if (antialias) {
        QFontEngine::SubpixelAntialiasingType subpixelType = subpixelAntialiasingTypeHint();

        if (subpixelType == QFontEngine::Subpixel_None || (fontDef.styleStrategy & QFont::NoSubpixelAntialias)) {
            format = QFontEngine::Format_A8;
            engine->subpixelType = QFontEngine::Subpixel_None;
        } else {
            format = QFontEngine::Format_A32;
            engine->subpixelType = subpixelType;
        }
    }

    if (! engine->init(fid, antialias, format) || engine->invalid()) {
        delete engine;
        engine = nullptr;
    } else {
        engine->setQtDefaultHintStyle(static_cast<QFont::HintingPreference>(fontDef.hintingPreference));
    }

    return engine;
}

namespace {

    class QFontEngineFTRawData: public QFontEngineFT
    {
    public:
        QFontEngineFTRawData(const QFontDef &fontDef) : QFontEngineFT(fontDef)
        {
        }

        void updateFamilyNameAndStyle()
        {
            fontDef.family = QString::fromLatin1(freetype->face->family_name);

            if (freetype->face->style_flags & FT_STYLE_FLAG_ITALIC)
                fontDef.style = QFont::StyleItalic;

            if (freetype->face->style_flags & FT_STYLE_FLAG_BOLD)
                fontDef.weight = QFont::Bold;
        }

        bool initFromData(const QByteArray &fontData)
        {
            FaceId faceId;
            faceId.filename = "";
            faceId.index = 0;
            faceId.uuid = QUuid::createUuid().toByteArray();

            return init(faceId, true, Format_None, fontData);
        }
    };

}

QFontEngine *QBasicFontDatabase::fontEngine(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference)
{
    QFontDef fontDef;
    fontDef.pixelSize = pixelSize;
    fontDef.hintingPreference = hintingPreference;

    QFontEngineFTRawData *fe = new QFontEngineFTRawData(fontDef);
    if (!fe->initFromData(fontData)) {
        delete fe;
        return nullptr;
    }

    fe->updateFamilyNameAndStyle();
    fe->setQtDefaultHintStyle(static_cast<QFont::HintingPreference>(fontDef.hintingPreference));

    return fe;
}

QStringList QBasicFontDatabase::addApplicationFont(const QByteArray &fontData, const QString &fileName)
{
    return QBasicFontDatabase::addTTFile(fontData, fileName.toUtf8());
}

void QBasicFontDatabase::releaseHandle(void *handle)
{
    FontFile *file = static_cast<FontFile *>(handle);
    delete file;
}

extern FT_Library qt_getFreetype();

QStringList QBasicFontDatabase::addTTFile(const QByteArray &fontData, const QByteArray &file)
{
    FT_Library library = qt_getFreetype();

    int index    = 0;
    int numFaces = 0;
    QStringList families;

    do {
        FT_Face face;
        FT_Error error;

        if (! fontData.isEmpty()) {
            error = FT_New_Memory_Face(library, (const FT_Byte *)fontData.constData(), fontData.size(), index, &face);
        } else {
            error = FT_New_Face(library, file.constData(), index, &face);
        }

        if (error != FT_Err_Ok) {
            qDebug() << "FT_New_Face failed with index" << index << ':' << hex << error;
            break;
        }
        numFaces = face->num_faces;

        QFont::Weight weight = QFont::Normal;

        QFont::Style style = QFont::StyleNormal;
        if (face->style_flags & FT_STYLE_FLAG_ITALIC)
            style = QFont::StyleItalic;

        if (face->style_flags & FT_STYLE_FLAG_BOLD)
            weight = QFont::Bold;

        bool fixedPitch = (face->face_flags & FT_FACE_FLAG_FIXED_WIDTH);

        QSupportedWritingSystems writingSystems;

        // detect symbol fonts
        for (int i = 0; i < face->num_charmaps; ++i) {
            FT_CharMap cm = face->charmaps[i];

            if (cm->encoding == FT_ENCODING_ADOBE_CUSTOM || cm->encoding == FT_ENCODING_MS_SYMBOL) {
                writingSystems.setSupported(QFontDatabase::Symbol);
                break;
            }
        }

        TT_OS2 *os2 = (TT_OS2 *)FT_Get_Sfnt_Table(face, ft_sfnt_os2);
        if (os2) {
            quint32 unicodeRange[4] = {
                quint32(os2->ulUnicodeRange1),
                quint32(os2->ulUnicodeRange2),
                quint32(os2->ulUnicodeRange3),
                quint32(os2->ulUnicodeRange4)
            };
            quint32 codePageRange[2] = {
                quint32(os2->ulCodePageRange1),
                quint32(os2->ulCodePageRange2)
            };

            writingSystems = QPlatformFontDatabase::writingSystemsFromTrueTypeBits(unicodeRange, codePageRange);

            if (os2->usWeightClass) {
                weight = QPlatformFontDatabase::weightFromInteger(os2->usWeightClass);

            } else if (os2->panose[2]) {
                int w = os2->panose[2];
                if (w <= 1)
                    weight = QFont::Thin;
                else if (w <= 2)
                    weight = QFont::ExtraLight;
                else if (w <= 3)
                    weight = QFont::Light;
                else if (w <= 5)
                    weight = QFont::Normal;
                else if (w <= 6)
                    weight = QFont::Medium;
                else if (w <= 7)
                    weight = QFont::DemiBold;
                else if (w <= 8)
                    weight = QFont::Bold;
                else if (w <= 9)
                    weight = QFont::ExtraBold;
                else if (w <= 10)
                    weight = QFont::Black;
            }
        }

        QString family = QString::fromLatin1(face->family_name);
        FontFile *fontFile = new FontFile;
        fontFile->fileName = QFile::decodeName(file);
        fontFile->indexValue = index;

        QFont::Stretch stretch = QFont::Unstretched;

        registerFont(family,QString::fromLatin1(face->style_name),QString(),weight,style,stretch,true,true,0,fixedPitch,writingSystems,fontFile);

        families.append(family);

        FT_Done_Face(face);
        ++index;

    } while (index < numFaces);

    return families;
}

#endif
