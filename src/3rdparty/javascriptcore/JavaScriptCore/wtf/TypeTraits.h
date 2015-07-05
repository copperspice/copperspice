 /*
 * Copyright (C) 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2009, 2010 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef TypeTraits_h
#define TypeTraits_h

#include "Platform.h"
#include <type_traits>

namespace WTF {

    // The following are provided in this file:
    //
    //   IsInteger<T>::value
    //   IsPod<T>::value, see the definition for a note about its limitations
    //   IsConvertibleToInteger<T>::value
    //
    //   IsSameType<T, U>::value
    //
    //   RemovePointer<T>::Type
    //   RemoveConst<T>::Type
    //   RemoveVolatile<T>::Type
    //   RemoveConstVolatile<T>::Type
    //
    //   COMPILE_ASSERT's in TypeTraits.cpp illustrate their usage and what they do.

    template<typename T> struct IsInteger           { static const bool value = false; };
    template<> struct IsInteger<bool>               { static const bool value = true; };
    template<> struct IsInteger<char>               { static const bool value = true; };
    template<> struct IsInteger<signed char>        { static const bool value = true; };
    template<> struct IsInteger<unsigned char>      { static const bool value = true; };
    template<> struct IsInteger<short>              { static const bool value = true; };
    template<> struct IsInteger<unsigned short>     { static const bool value = true; };
    template<> struct IsInteger<int>                { static const bool value = true; };
    template<> struct IsInteger<unsigned int>       { static const bool value = true; };
    template<> struct IsInteger<long>               { static const bool value = true; };
    template<> struct IsInteger<unsigned long>      { static const bool value = true; };
    template<> struct IsInteger<long long>          { static const bool value = true; };
    template<> struct IsInteger<unsigned long long> { static const bool value = true; };

#if ! COMPILER(MSVC) || defined(_NATIVE_WCHAR_T_DEFINED)
    template<> struct IsInteger<wchar_t>            { static const bool value = true; };
#endif

    // IsPod is misnamed as it doesn't cover all plain old data (pod) types.
    // Specifically, it doesn't allow for enums or for structs.
    template <typename T> struct IsPod           { static const bool value = IsInteger<T>::value; };
    template <> struct IsPod<float>              { static const bool value = true; };
    template <> struct IsPod<double>             { static const bool value = true; };
    template <> struct IsPod<long double>        { static const bool value = true; };
    template <typename P> struct IsPod<P*>       { static const bool value = true; };

    // Avoid "possible loss of data" warning when using Microsoft's C++ compiler
    // by not converting int's to doubles.
    template<bool performCheck, typename U> class CheckedIsConvertibleToDouble;
    template<typename U> class CheckedIsConvertibleToDouble<false, U> {
    public:
        static const bool value = false;
    };

    template<typename U> class CheckedIsConvertibleToDouble<true, U> {
        typedef char YesType;
        struct NoType {
            char padding[8];
        };

        static YesType floatCheck(long double);
        static NoType floatCheck(...);
        static U& t;
    public:
        static const bool value = sizeof(floatCheck(t)) == sizeof(YesType);
    };

    template<typename T> class IsConvertibleToInteger {
    public:
        static const bool value = IsInteger<T>::value || CheckedIsConvertibleToDouble<!IsInteger<T>::value, T>::value;
    };

    template <typename T, typename U> struct IsSameType {
        static const bool value = false;
    };

    template <typename T> struct IsSameType<T, T> {
        static const bool value = true;
    };

    template <typename T, typename U> class IsSubclass {
        typedef char YesType;
        struct NoType {
            char padding[8];
        };

        static YesType subclassCheck(U*);
        static NoType subclassCheck(...);
        static T* t;
    public:
        static const bool value = sizeof(subclassCheck(t)) == sizeof(YesType);
    };

    template <typename T, template<class V> class U> class IsSubclassOfTemplate {
        typedef char YesType;
        struct NoType {
            char padding[8];
        };

        template<typename W> static YesType subclassCheck(U<W>*);
        static NoType subclassCheck(...);
        static T* t;
    public:
        static const bool value = sizeof(subclassCheck(t)) == sizeof(YesType);
    };

    template <typename T, template <class V> class OuterTemplate> struct RemoveTemplate {
        typedef T Type;
    };

    template <typename T, template <class V> class OuterTemplate> struct RemoveTemplate<OuterTemplate<T>, OuterTemplate> {
        typedef T Type;
    };

    template <typename T> struct RemoveConst {
        typedef T Type;
    };

    template <typename T> struct RemoveConst<const T> {
        typedef T Type;
    };

    template <typename T> struct RemoveVolatile {
        typedef T Type;
    };

    template <typename T> struct RemoveVolatile<volatile T> {
        typedef T Type;
    };

    template <typename T> struct RemoveConstVolatile {
        typedef typename RemoveVolatile<typename RemoveConst<T>::Type>::Type Type;
    };

    template <typename T> struct RemovePointer {
        typedef T Type;
    };

    template <typename T> struct RemovePointer<T*> {
        typedef T Type;
    };


} // namespace WTF


// copperspice solution for type traits (used in javascript and webkit)
class cs_alternate{};

class cs_preferred : public cs_alternate{};

namespace std{
   template<class> struct is_trivially_constructible;
   template<class> struct has_trivial_default_constructor;
  
   template<class> struct is_trivially_destructible; 
   template<class> struct has_trivial_destructor;  
}

namespace WTF {

template<typename T> 
  struct HasTrivialConstructor : public std::false_type
{
};

template<typename T>
  struct HasTrivialDestructor : public std::false_type
{
};

} // namespace WTF

#endif // TypeTraits_h
