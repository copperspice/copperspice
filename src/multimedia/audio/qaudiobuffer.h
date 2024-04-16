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

#ifndef QAUDIOBUFFER_H
#define QAUDIOBUFFER_H

#include <qshareddata.h>

#include <qmultimedia.h>
#include <qaudio.h>
#include <qaudioformat.h>

class QAbstractAudioBuffer;
class QAudioBufferPrivate;

class Q_MULTIMEDIA_EXPORT QAudioBuffer
{
 public:
   QAudioBuffer();
   QAudioBuffer(QAbstractAudioBuffer *provider);
   QAudioBuffer(const QAudioBuffer &other);
   QAudioBuffer(const QByteArray &data, const QAudioFormat &format, qint64 startTime = -1);
   QAudioBuffer(int numFrames, const QAudioFormat &format, qint64 startTime = -1); // Initialized to empty

   QAudioBuffer &operator=(const QAudioBuffer &other);

   ~QAudioBuffer();

   bool isValid() const;

   QAudioFormat format() const;

   int frameCount() const;
   int sampleCount() const;
   int byteCount() const;

   qint64 duration() const;
   qint64 startTime() const;

   // Data modification
   // void clear();
   // Other ideas
   // operator *=
   // operator += (need to be careful about different formats)

   // Data access
   const void *constData() const; // Does not detach, preferred
   const void *data() const; // Does not detach
   void *data(); // detaches

   // Structures for easier access to stereo data
   template <typename T> struct StereoFrameDefault {
      enum {
         Default = 0
      };
   };

   template <typename T> struct StereoFrame {

      StereoFrame()
         : left(T(StereoFrameDefault<T>::Default)), right(T(StereoFrameDefault<T>::Default)) {
      }

      StereoFrame(T leftSample, T rightSample)
         : left(leftSample), right(rightSample) {
      }

      StereoFrame &operator=(const StereoFrame &other) {
         // Two straight assigns is probably
         // cheaper than a conditional check on
         // self assignment
         left = other.left;
         right = other.right;
         return *this;
      }

      T left;
      T right;

      T average() const {
         return (left + right) / 2;
      }
      void clear() {
         left = right = T(StereoFrameDefault<T>::Default);
      }
   };

   typedef StereoFrame<unsigned char> S8U;
   typedef StereoFrame<signed char> S8S;
   typedef StereoFrame<unsigned short> S16U;
   typedef StereoFrame<signed short> S16S;
   typedef StereoFrame<float> S32F;

   template <typename T> const T *constData() const {
      return static_cast<const T *>(constData());
   }

   template <typename T> const T *data() const {
      return static_cast<const T *>(data());
   }

   template <typename T> T *data() {
      return static_cast<T *>(data());
   }

 private:
   QAudioBufferPrivate *d;
};

template <>
struct QAudioBuffer::StereoFrameDefault<unsigned char> {
   enum {
      Default = 128
   };
};

template <>
struct QAudioBuffer::StereoFrameDefault<unsigned short> {
   enum {
      Default = 32768
   };
};

#endif
