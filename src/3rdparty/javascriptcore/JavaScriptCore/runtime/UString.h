/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
 *  Copyright (C) 2009 Google Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#ifndef UString_h
#define UString_h

#include "Collector.h"
#include "UStringImpl.h"
#include <stdint.h>
#include <string.h>
#include <wtf/Assertions.h>
#include <wtf/CrossThreadRefCounted.h>
#include <wtf/OwnFastMallocPtr.h>
#include <wtf/PassRefPtr.h>
#include <wtf/PtrAndFlags.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>
#include <wtf/unicode/Unicode.h>

#if PLATFORM(QT)
#include <qstring.h>
#endif

namespace JSC {

    using WTF::PlacementNewAdoptType;
    using WTF::PlacementNewAdopt;

    class CString {
    public:
        CString()
            : m_length(0)
            , m_data(nullptr)
        {
        }

        CString(const char*);
        CString(const char*, size_t);
        CString(const CString&);

        ~CString();

        static CString adopt(char*, size_t); // buffer should be allocated with new[].

        CString& append(const CString&);
        CString& operator=(const char* c);
        CString& operator=(const CString&);
        CString& operator+=(const CString& c) { return append(c); }

        size_t size() const { return m_length; }
        const char* c_str() const { return m_data; }

    private:
        size_t m_length;
        char* m_data;
    };

    bool operator==(const CString&, const CString&);

    typedef Vector<char, 32> CStringBuffer;

    class UString {
        friend class JIT;

    public:
#if PLATFORM(QT)
        operator QT_PREPEND_NAMESPACE(QString)() const
        {
            return QT_PREPEND_NAMESPACE(QString)(reinterpret_cast<const QT_PREPEND_NAMESPACE(QChar)*>(this->data()), this->size());
        }

        UString(const QT_PREPEND_NAMESPACE(QString)& str)
        {
            *this = JSC::UString(reinterpret_cast<const UChar*>(str.constData()), str.length());
        }
#endif
        typedef UStringImpl Rep;

    public:
        // UString constructors passed char*s assume ISO Latin-1 encoding; for UTF8 use 'createFromUTF8', below.
        UString();
        UString(const char*); // Constructor for null-terminated string.
        UString(const char*, int length);
        UString(const UChar*, int length);
        UString(const Vector<UChar>& buffer);

        UString(const UString& s)
            : m_rep(s.m_rep)
        {
        }

        // CS fix warning
        const UString &operator=(const UString& s)
        {
           m_rep = s.m_rep;
           return *this;
        }

        // Special constructor for cases where we overwrite an object in place.
        UString(PlacementNewAdoptType)
            : m_rep(PlacementNewAdopt)
        {
        }

        ~UString()
        {
        }

        template<size_t inlineCapacity>
        static PassRefPtr<UStringImpl> adopt(Vector<UChar, inlineCapacity>& vector)
        {
            return Rep::adopt(vector);
        }

        static UString createFromUTF8(const char*);

        static UString from(int);
        static UString from(long long);
        static UString from(unsigned int);
        static UString from(long);
        static UString from(double);

        struct Range {
        public:
            Range(int pos, int len)
                : position(pos)
                , length(len)
            {
            }

            Range()
            {
            }

            int position;
            int length;
        };

        UString spliceSubstringsWithSeparators(const Range* substringRanges, int rangeCount, const UString* separators, int separatorCount) const;

        UString replaceRange(int rangeStart, int RangeEnd, const UString& replacement) const;

        bool getCString(CStringBuffer&) const;

        // NOTE: This method should only be used for *debugging* purposes as it
        // is neither Unicode safe nor free from side effects nor thread-safe.
        char* ascii() const;

        /**
         * Convert the string to UTF-8, assuming it is UTF-16 encoded.
         * In non-strict mode, this function is tolerant of badly formed UTF-16, it
         * can create UTF-8 strings that are invalid because they have characters in
         * the range U+D800-U+DDFF, U+FFFE, or U+FFFF, but the UTF-8 string is
         * guaranteed to be otherwise valid.
         * In strict mode, error is returned as null CString.
         */
        CString UTF8String(bool strict = false) const;

        UString& operator=(const char*c);

        const UChar* data() const { return m_rep->data(); }

        bool isNull() const { return m_rep == &Rep::null(); }
        bool isEmpty() const { return !m_rep->size(); }

        bool is8Bit() const;

        int size() const { return m_rep->size(); }

        UChar operator[](int pos) const;

        double toDouble(bool tolerateTrailingJunk, bool tolerateEmptyString) const;
        double toDouble(bool tolerateTrailingJunk) const;
        double toDouble() const;

        uint32_t toUInt32(bool* ok = nullptr) const;
        uint32_t toUInt32(bool* ok, bool tolerateEmptyString) const;
        uint32_t toStrictUInt32(bool* ok = nullptr) const;

        unsigned toArrayIndex(bool* ok = nullptr) const;

        int find(const UString& f, int pos = 0) const;
        int find(UChar, int pos = 0) const;
        int rfind(const UString& f, int pos) const;
        int rfind(UChar, int pos) const;

        UString substr(int pos = 0, int len = -1) const;

        static const UString& null() { return *nullUString; }

        Rep* rep() const { return m_rep.get(); }
        static Rep* nullRep();

        UString(PassRefPtr<Rep> r)
            : m_rep(r)
        {
            ASSERT(m_rep);
        }

        size_t cost() const { return m_rep->cost(); }

    private:
        void makeNull();

        RefPtr<Rep> m_rep;
        static UString* nullUString;

        friend void initializeUString();
        friend bool operator==(const UString&, const UString&);
    };

    ALWAYS_INLINE bool operator==(const UString& s1, const UString& s2)
    {
        int size = s1.size();
        switch (size) {
        case 0:
            return !s2.size();
        case 1:
            return s2.size() == 1 && s1.data()[0] == s2.data()[0];
        case 2: {
            if (s2.size() != 2)
                return false;
            const UChar* d1 = s1.data();
            const UChar* d2 = s2.data();
            return (d1[0] == d2[0]) & (d1[1] == d2[1]);
        }
        default:
            return s2.size() == size && memcmp(s1.data(), s2.data(), size * sizeof(UChar)) == 0;
        }
    }


    inline bool operator!=(const UString& s1, const UString& s2)
    {
        return !JSC::operator==(s1, s2);
    }

    bool operator<(const UString& s1, const UString& s2);
    bool operator>(const UString& s1, const UString& s2);

    bool operator==(const UString& s1, const char* s2);

    inline bool operator!=(const UString& s1, const char* s2)
    {
        return !JSC::operator==(s1, s2);
    }

    inline bool operator==(const char *s1, const UString& s2)
    {
        return operator==(s2, s1);
    }

    inline bool operator!=(const char *s1, const UString& s2)
    {
        return !JSC::operator==(s1, s2);
    }

    int compare(const UString&, const UString&);

    inline UString::UString()
        : m_rep(&Rep::null())
    {
    }

    // Rule from ECMA 15.2 about what an array index is.
    // Must exactly match string form of an unsigned integer, and be less than 2^32 - 1.
    inline unsigned UString::toArrayIndex(bool* ok) const
    {
        unsigned i = toStrictUInt32(ok);
        if (ok && i >= 0xFFFFFFFFU)
            *ok = false;
        return i;
    }

    // We'd rather not do shared substring append for small strings, since
    // this runs too much risk of a tiny initial string holding down a
    // huge buffer.
    // FIXME: this should be size_t but that would cause warnings until we
    // fix UString sizes to be size_t instead of int
    static const int minShareSize = Heap::minExtraCost / sizeof(UChar);

    struct IdentifierRepHash : PtrHash<RefPtr<JSC::UString::Rep> > {
        static unsigned hash(const RefPtr<JSC::UString::Rep>& key) { return key->existingHash(); }
        static unsigned hash(JSC::UString::Rep* key) { return key->existingHash(); }
    };

    void initializeUString();

    template<typename StringType>
    class StringTypeAdapter {
    };

    template<>
    class StringTypeAdapter<char*> {
    public:
        StringTypeAdapter<char*>(char* buffer)
            : m_buffer((unsigned char*)buffer)
            , m_length(strlen(buffer))
        {
        }

        unsigned length() { return m_length; }

        void writeTo(UChar* destination)
        {
            for (unsigned i = 0; i < m_length; ++i)
                destination[i] = m_buffer[i];
        }

    private:
        const unsigned char* m_buffer;
        unsigned m_length;
    };

    template<>
    class StringTypeAdapter<const char*> {
    public:
        StringTypeAdapter<const char*>(const char* buffer)
            : m_buffer((unsigned char*)buffer)
            , m_length(strlen(buffer))
        {
        }

        unsigned length() { return m_length; }

        void writeTo(UChar* destination)
        {
            for (unsigned i = 0; i < m_length; ++i)
                destination[i] = m_buffer[i];
        }

    private:
        const unsigned char* m_buffer;
        unsigned m_length;
    };

    template<>
    class StringTypeAdapter<UString> {
    public:
        StringTypeAdapter<UString>(UString& string)
            : m_data(string.data())
            , m_length(string.size())
        {
        }

        unsigned length() { return m_length; }

        void writeTo(UChar* destination)
        {
            for (unsigned i = 0; i < m_length; ++i)
                destination[i] = m_data[i];
        }

    private:
        const UChar* m_data;
        unsigned m_length;
    };

    template<typename StringType1, typename StringType2>
    UString makeString(StringType1 string1, StringType2 string2)
    {
        StringTypeAdapter<StringType1> adapter1(string1);
        StringTypeAdapter<StringType2> adapter2(string2);

        UChar* buffer;
        unsigned length = adapter1.length() + adapter2.length();
        PassRefPtr<UStringImpl> resultImpl = UStringImpl::tryCreateUninitialized(length, buffer);
        if (!resultImpl)
            return UString();

        UChar* result = buffer;
        adapter1.writeTo(result);
        result += adapter1.length();
        adapter2.writeTo(result);

        return resultImpl;
    }

    template<typename StringType1, typename StringType2, typename StringType3>
    UString makeString(StringType1 string1, StringType2 string2, StringType3 string3)
    {
        StringTypeAdapter<StringType1> adapter1(string1);
        StringTypeAdapter<StringType2> adapter2(string2);
        StringTypeAdapter<StringType3> adapter3(string3);

        UChar* buffer;
        unsigned length = adapter1.length() + adapter2.length() + adapter3.length();
        PassRefPtr<UStringImpl> resultImpl = UStringImpl::tryCreateUninitialized(length, buffer);
        if (!resultImpl)
            return UString();

        UChar* result = buffer;
        adapter1.writeTo(result);
        result += adapter1.length();
        adapter2.writeTo(result);
        result += adapter2.length();
        adapter3.writeTo(result);

        return resultImpl;
    }

    template<typename StringType1, typename StringType2, typename StringType3, typename StringType4>
    UString makeString(StringType1 string1, StringType2 string2, StringType3 string3, StringType4 string4)
    {
        StringTypeAdapter<StringType1> adapter1(string1);
        StringTypeAdapter<StringType2> adapter2(string2);
        StringTypeAdapter<StringType3> adapter3(string3);
        StringTypeAdapter<StringType4> adapter4(string4);

        UChar* buffer;
        unsigned length = adapter1.length() + adapter2.length() + adapter3.length() + adapter4.length();
        PassRefPtr<UStringImpl> resultImpl = UStringImpl::tryCreateUninitialized(length, buffer);
        if (!resultImpl)
            return UString();

        UChar* result = buffer;
        adapter1.writeTo(result);
        result += adapter1.length();
        adapter2.writeTo(result);
        result += adapter2.length();
        adapter3.writeTo(result);
        result += adapter3.length();
        adapter4.writeTo(result);

        return resultImpl;
    }

    template<typename StringType1, typename StringType2, typename StringType3, typename StringType4, typename StringType5>
    UString makeString(StringType1 string1, StringType2 string2, StringType3 string3, StringType4 string4, StringType5 string5)
    {
        StringTypeAdapter<StringType1> adapter1(string1);
        StringTypeAdapter<StringType2> adapter2(string2);
        StringTypeAdapter<StringType3> adapter3(string3);
        StringTypeAdapter<StringType4> adapter4(string4);
        StringTypeAdapter<StringType5> adapter5(string5);

        UChar* buffer;
        unsigned length = adapter1.length() + adapter2.length() + adapter3.length() + adapter4.length() + adapter5.length();
        PassRefPtr<UStringImpl> resultImpl = UStringImpl::tryCreateUninitialized(length, buffer);
        if (!resultImpl)
            return UString();

        UChar* result = buffer;
        adapter1.writeTo(result);
        result += adapter1.length();
        adapter2.writeTo(result);
        result += adapter2.length();
        adapter3.writeTo(result);
        result += adapter3.length();
        adapter4.writeTo(result);
        result += adapter4.length();
        adapter5.writeTo(result);

        return resultImpl;
    }

    template<typename StringType1, typename StringType2, typename StringType3, typename StringType4, typename StringType5, typename StringType6>
    UString makeString(StringType1 string1, StringType2 string2, StringType3 string3, StringType4 string4, StringType5 string5, StringType6 string6)
    {
        StringTypeAdapter<StringType1> adapter1(string1);
        StringTypeAdapter<StringType2> adapter2(string2);
        StringTypeAdapter<StringType3> adapter3(string3);
        StringTypeAdapter<StringType4> adapter4(string4);
        StringTypeAdapter<StringType5> adapter5(string5);
        StringTypeAdapter<StringType6> adapter6(string6);

        UChar* buffer;
        unsigned length = adapter1.length() + adapter2.length() + adapter3.length() + adapter4.length() + adapter5.length() + adapter6.length();
        PassRefPtr<UStringImpl> resultImpl = UStringImpl::tryCreateUninitialized(length, buffer);
        if (!resultImpl)
            return UString();

        UChar* result = buffer;
        adapter1.writeTo(result);
        result += adapter1.length();
        adapter2.writeTo(result);
        result += adapter2.length();
        adapter3.writeTo(result);
        result += adapter3.length();
        adapter4.writeTo(result);
        result += adapter4.length();
        adapter5.writeTo(result);
        result += adapter5.length();
        adapter6.writeTo(result);

        return resultImpl;
    }

    template<typename StringType1, typename StringType2, typename StringType3, typename StringType4, typename StringType5, typename StringType6, typename StringType7>
    UString makeString(StringType1 string1, StringType2 string2, StringType3 string3, StringType4 string4, StringType5 string5, StringType6 string6, StringType7 string7)
    {
        StringTypeAdapter<StringType1> adapter1(string1);
        StringTypeAdapter<StringType2> adapter2(string2);
        StringTypeAdapter<StringType3> adapter3(string3);
        StringTypeAdapter<StringType4> adapter4(string4);
        StringTypeAdapter<StringType5> adapter5(string5);
        StringTypeAdapter<StringType6> adapter6(string6);
        StringTypeAdapter<StringType7> adapter7(string7);

        UChar* buffer;
        unsigned length = adapter1.length() + adapter2.length() + adapter3.length() + adapter4.length() + adapter5.length() + adapter6.length() + adapter7.length();
        PassRefPtr<UStringImpl> resultImpl = UStringImpl::tryCreateUninitialized(length, buffer);
        if (!resultImpl)
            return UString();

        UChar* result = buffer;
        adapter1.writeTo(result);
        result += adapter1.length();
        adapter2.writeTo(result);
        result += adapter2.length();
        adapter3.writeTo(result);
        result += adapter3.length();
        adapter4.writeTo(result);
        result += adapter4.length();
        adapter5.writeTo(result);
        result += adapter5.length();
        adapter6.writeTo(result);
        result += adapter6.length();
        adapter7.writeTo(result);

        return resultImpl;
    }

    template<typename StringType1, typename StringType2, typename StringType3, typename StringType4, typename StringType5, typename StringType6, typename StringType7, typename StringType8>
    UString makeString(StringType1 string1, StringType2 string2, StringType3 string3, StringType4 string4, StringType5 string5, StringType6 string6, StringType7 string7, StringType8 string8)
    {
        StringTypeAdapter<StringType1> adapter1(string1);
        StringTypeAdapter<StringType2> adapter2(string2);
        StringTypeAdapter<StringType3> adapter3(string3);
        StringTypeAdapter<StringType4> adapter4(string4);
        StringTypeAdapter<StringType5> adapter5(string5);
        StringTypeAdapter<StringType6> adapter6(string6);
        StringTypeAdapter<StringType7> adapter7(string7);
        StringTypeAdapter<StringType8> adapter8(string8);

        UChar* buffer;
        unsigned length = adapter1.length() + adapter2.length() + adapter3.length() + adapter4.length() + adapter5.length() + adapter6.length() + adapter7.length() + adapter8.length();
        PassRefPtr<UStringImpl> resultImpl = UStringImpl::tryCreateUninitialized(length, buffer);
        if (!resultImpl)
            return UString();

        UChar* result = buffer;
        adapter1.writeTo(result);
        result += adapter1.length();
        adapter2.writeTo(result);
        result += adapter2.length();
        adapter3.writeTo(result);
        result += adapter3.length();
        adapter4.writeTo(result);
        result += adapter4.length();
        adapter5.writeTo(result);
        result += adapter5.length();
        adapter6.writeTo(result);
        result += adapter6.length();
        adapter7.writeTo(result);
        result += adapter7.length();
        adapter8.writeTo(result);

        return resultImpl;
    }

} // namespace JSC

namespace WTF {

    template<typename T> struct DefaultHash;
    template<typename T> struct StrHash;

    template<> struct StrHash<JSC::UString::Rep*> {
        static unsigned hash(const JSC::UString::Rep* key) { return key->hash(); }
        static bool equal(const JSC::UString::Rep* a, const JSC::UString::Rep* b) { return JSC::equal(a, b); }
        static const bool safeToCompareToEmptyOrDeleted = false;
    };

    template<> struct StrHash<RefPtr<JSC::UString::Rep> > : public StrHash<JSC::UString::Rep*> {
        using StrHash<JSC::UString::Rep*>::hash;
        static unsigned hash(const RefPtr<JSC::UString::Rep>& key) { return key->hash(); }
        using StrHash<JSC::UString::Rep*>::equal;
        static bool equal(const RefPtr<JSC::UString::Rep>& a, const RefPtr<JSC::UString::Rep>& b) { return JSC::equal(a.get(), b.get()); }
        static bool equal(const JSC::UString::Rep* a, const RefPtr<JSC::UString::Rep>& b) { return JSC::equal(a, b.get()); }
        static bool equal(const RefPtr<JSC::UString::Rep>& a, const JSC::UString::Rep* b) { return JSC::equal(a.get(), b); }

        static const bool safeToCompareToEmptyOrDeleted = false;
    };

    template<> struct DefaultHash<JSC::UString::Rep*> {
        typedef StrHash<JSC::UString::Rep*> Hash;
    };

    template<> struct DefaultHash<RefPtr<JSC::UString::Rep> > {
        typedef StrHash<RefPtr<JSC::UString::Rep> > Hash;

    };

} // namespace WTF

#endif
