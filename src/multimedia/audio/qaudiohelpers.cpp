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

#include <qaudiohelpers_p.h>

#include <qdebug.h>

namespace QAudioHelperInternal
{

template <class T>
void adjustSamples(qreal factor, const void *src, void *dst, int samples)
{
    const T *pSrc = (const T *)src;
    T *pDst = (T*)dst;

    for ( int i = 0; i < samples; i++ )
        pDst[i] = pSrc[i] * factor;
}

// Unsigned samples are biased around 0x80/0x8000 :/
// This makes a pure template solution a bit unwieldy but possible
template <class T>
struct signedVersion {};

template <>
struct signedVersion<quint8>
{
   typedef qint8 TS;

   enum {
      offset = 0x80
   };
};

template <>
struct signedVersion<quint16>
{
   typedef qint16 TS;

   enum {
      offset = 0x8000
   };
};

template <>
struct signedVersion<quint32>
{
   typedef qint32 TS;

   enum {
      offset = 0x80000000
   };
};

template <class T>
void adjustUnsignedSamples(qreal factor, const void *src, void *dst, int samples)
{
    const T *pSrc = (const T *)src;

    T *pDst = (T*)dst;
    for ( int i = 0; i < samples; i++ ) {
        pDst[i] = signedVersion<T>::offset + ((typename signedVersion<T>::TS)(pSrc[i] - signedVersion<T>::offset) * factor);
    }
}

void qMultiplySamples(qreal factor, const QAudioFormat &format, const void* src, void* dest, int len)
{
    int samplesCount = len / (format.sampleSize()/8);

    switch ( format.sampleSize() ) {

    case 8:
        if (format.sampleType() == QAudioFormat::SignedInt)
            QAudioHelperInternal::adjustSamples<qint8>(factor,src,dest,samplesCount);
        else if (format.sampleType() == QAudioFormat::UnSignedInt)
            QAudioHelperInternal::adjustUnsignedSamples<quint8>(factor,src,dest,samplesCount);
        break;

    case 16:
        if (format.sampleType() == QAudioFormat::SignedInt)
            QAudioHelperInternal::adjustSamples<qint16>(factor,src,dest,samplesCount);
        else if (format.sampleType() == QAudioFormat::UnSignedInt)
            QAudioHelperInternal::adjustUnsignedSamples<quint16>(factor,src,dest,samplesCount);
        break;

    default:
        if (format.sampleType() == QAudioFormat::SignedInt)
            QAudioHelperInternal::adjustSamples<qint32>(factor,src,dest,samplesCount);
        else if (format.sampleType() == QAudioFormat::UnSignedInt)
            QAudioHelperInternal::adjustUnsignedSamples<quint32>(factor,src,dest,samplesCount);
        else if (format.sampleType() == QAudioFormat::Float)
            QAudioHelperInternal::adjustSamples<float>(factor,src,dest,samplesCount);
    }
}

}  // namespace

