/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

// Most of the code here was originally written by Serika Kurusugawa
// a.k.a. Junji Takagi, and is included in Qt with the author's permission,
// and the grateful thanks of the Qt team.

/*
 * Copyright (C) 1999 Serika Kurusugawa, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef QJPUNICODE_H
#define QJPUNICODE_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QJpUnicodeConv {
public:
    virtual ~QJpUnicodeConv() {}
    enum Rules {
        // "ASCII" is ANSI X.3.4-1986, a.k.a. US-ASCII here.
        Default                        = 0x0000,

        Unicode                        = 0x0001,
        Unicode_JISX0201                = 0x0001,
        Unicode_ASCII                 = 0x0002,
        JISX0221_JISX0201         = 0x0003,
        JISX0221_ASCII                = 0x0004,
        Sun_JDK117                     = 0x0005,
        Microsoft_CP932                = 0x0006,

        NEC_VDC                = 0x0100,                // NEC Vender Defined Char
        UDC                        = 0x0200,                // User Defined Char
        IBM_VDC                = 0x0400                // IBM Vender Defined Char
    };
    static QJpUnicodeConv *newConverter(int rule);

    virtual uint asciiToUnicode(uint h, uint l) const;
    /*virtual*/ uint jisx0201ToUnicode(uint h, uint l) const;
    virtual uint jisx0201LatinToUnicode(uint h, uint l) const;
    /*virtual*/ uint jisx0201KanaToUnicode(uint h, uint l) const;
    virtual uint jisx0208ToUnicode(uint h, uint l) const;
    virtual uint jisx0212ToUnicode(uint h, uint l) const;

    uint asciiToUnicode(uint ascii) const {
        return asciiToUnicode((ascii & 0xff00) >> 8, (ascii & 0x00ff));
    }
    uint jisx0201ToUnicode(uint jis) const {
        return jisx0201ToUnicode((jis & 0xff00) >> 8, (jis & 0x00ff));
    }
    uint jisx0201LatinToUnicode(uint jis) const {
        return jisx0201LatinToUnicode((jis & 0xff00) >> 8, (jis & 0x00ff));
    }
    uint jisx0201KanaToUnicode(uint jis) const {
        return jisx0201KanaToUnicode((jis & 0xff00) >> 8, (jis & 0x00ff));
    }
    uint jisx0208ToUnicode(uint jis) const {
        return jisx0208ToUnicode((jis & 0xff00) >> 8, (jis & 0x00ff));
    }
    uint jisx0212ToUnicode(uint jis) const {
        return jisx0212ToUnicode((jis & 0xff00) >> 8, (jis & 0x00ff));
    }

    virtual uint unicodeToAscii(uint h, uint l) const;
    /*virtual*/ uint unicodeToJisx0201(uint h, uint l) const;
    virtual uint unicodeToJisx0201Latin(uint h, uint l) const;
    /*virtual*/ uint unicodeToJisx0201Kana(uint h, uint l) const;
    virtual uint unicodeToJisx0208(uint h, uint l) const;
    virtual uint unicodeToJisx0212(uint h, uint l) const;

    uint unicodeToAscii(uint unicode) const {
        return unicodeToAscii((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }
    uint unicodeToJisx0201(uint unicode) const {
        return unicodeToJisx0201((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }
    uint unicodeToJisx0201Latin(uint unicode) const {
        return unicodeToJisx0201Latin((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }
    uint unicodeToJisx0201Kana(uint unicode) const {
        return unicodeToJisx0201Kana((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }
    uint unicodeToJisx0208(uint unicode) const {
        return unicodeToJisx0208((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }
    uint unicodeToJisx0212(uint unicode) const {
        return unicodeToJisx0212((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }

    uint sjisToUnicode(uint h, uint l) const;
    uint unicodeToSjis(uint h, uint l) const;
    uint sjisibmvdcToUnicode(uint h, uint l) const;
    uint unicodeToSjisibmvdc(uint h, uint l) const;
    uint cp932ToUnicode(uint h, uint l) const;
    uint unicodeToCp932(uint h, uint l) const;

    uint sjisToUnicode(uint sjis) const {
        return sjisToUnicode((sjis & 0xff00) >> 8, (sjis & 0x00ff));
    }
    uint unicodeToSjis(uint unicode) const {
        return unicodeToSjis((unicode & 0xff00) >> 8, (unicode & 0x00ff));
    }

protected:
    explicit QJpUnicodeConv(int r) : rule(r) {}

private:
    int rule;
};

QT_END_NAMESPACE

#endif // QJPUNICODE_H
