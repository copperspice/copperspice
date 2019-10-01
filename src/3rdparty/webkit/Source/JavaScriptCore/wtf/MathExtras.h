/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WTF_MathExtras_h
#define WTF_MathExtras_h

#include <algorithm>
#include <cmath>
#include <float.h>
#include <limits>
#include <stdlib.h>

#if OS(OPENBSD)
#include <sys/types.h>
#include <machine/ieee.h>
#endif

#ifndef M_PI
const double piDouble = 3.14159265358979323846;
const float piFloat = 3.14159265358979323846f;
#else
const double piDouble = M_PI;
const float piFloat = static_cast<float>(M_PI);
#endif

#ifndef M_PI_2
const double piOverTwoDouble = 1.57079632679489661923;
const float piOverTwoFloat = 1.57079632679489661923f;
#else
const double piOverTwoDouble = M_PI_2;
const float piOverTwoFloat = static_cast<float>(M_PI_2);
#endif

#ifndef M_PI_4
const double piOverFourDouble = 0.785398163397448309616;
const float piOverFourFloat = 0.785398163397448309616f;
#else
const double piOverFourDouble = M_PI_4;
const float piOverFourFloat = static_cast<float>(M_PI_4);
#endif

#ifndef isinf
inline bool isinf(double x) { return !finite(x) && !isnand(x); }
#endif

#ifndef signbit
inline bool signbit(double x) { return copysign(1.0, x) < 0; }
#endif

#endif

#if OS(OPENBSD)

#ifndef isfinite
inline bool isfinite(double x) { return finite(x); }
#endif

#ifndef signbit
inline bool signbit(double x) { struct ieee_double *p = (struct ieee_double *)&x; return p->dbl_sign; }
#endif

#endif

inline double deg2rad(double d)  { return d * piDouble / 180.0; }
inline double rad2deg(double r)  { return r * 180.0 / piDouble; }
inline double deg2grad(double d) { return d * 400.0 / 360.0; }
inline double grad2deg(double g) { return g * 360.0 / 400.0; }
inline double turn2deg(double t) { return t * 360.0; }
inline double deg2turn(double d) { return d / 360.0; }
inline double rad2grad(double r) { return r * 200.0 / piDouble; }
inline double grad2rad(double g) { return g * piDouble / 200.0; }

inline float deg2rad(float d)  { return d * piFloat / 180.0f; }
inline float rad2deg(float r)  { return r * 180.0f / piFloat; }
inline float deg2grad(float d) { return d * 400.0f / 360.0f; }
inline float grad2deg(float g) { return g * 360.0f / 400.0f; }
inline float turn2deg(float t) { return t * 360.0f; }
inline float deg2turn(float d) { return d / 360.0f; }
inline float rad2grad(float r) { return r * 200.0f / piFloat; }
inline float grad2rad(float g) { return g * piFloat / 200.0f; }

inline int clampToInteger(double d)
{
    const double minIntAsDouble = std::numeric_limits<int>::min();
    const double maxIntAsDouble = std::numeric_limits<int>::max();
    return static_cast<int>(std::max(std::min(d, maxIntAsDouble), minIntAsDouble));
}

inline int clampToPositiveInteger(double d)
{
    const double maxIntAsDouble = std::numeric_limits<int>::max();
    return static_cast<int>(std::max<double>(std::min(d, maxIntAsDouble), 0));
}

inline int clampToInteger(float x)
{
    static const int s_intMax = std::numeric_limits<int>::max();
    static const int s_intMin = std::numeric_limits<int>::min();

    if (x >= static_cast<float>(s_intMax))
        return s_intMax;

    if (x < static_cast<float>(s_intMin))
        return s_intMin;

    return static_cast<int>(x);
}

inline int clampToPositiveInteger(float x)
{
    static const int s_intMax = std::numeric_limits<int>::max();

    if (x >= static_cast<float>(s_intMax))
        return s_intMax;
    if (x < 0)
        return 0;
    return static_cast<int>(x);
}

inline int clampToInteger(unsigned value)
{
    return static_cast<int>(std::min(value, static_cast<unsigned>(std::numeric_limits<int>::max())));
}


#endif
