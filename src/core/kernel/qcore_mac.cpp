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

#include <qcore_mac_p.h>
#include <new>

QT_BEGIN_NAMESPACE

QString QCFString::toQString(CFStringRef str)
{
   if (!str) {
      return QString();
   }

   CFIndex length = CFStringGetLength(str);
   if (length == 0) {
      return QString();
   }

   QString string(length, Qt::Uninitialized);
   CFStringGetCharacters(str, CFRangeMake(0, length), reinterpret_cast<UniChar *>(const_cast<QChar *>(string.unicode())));

   return string;
}

QCFString::operator QString() const
{
   if (string.isEmpty() && type) {
      const_cast<QCFString *>(this)->string = toQString(type);
   }
   return string;
}

CFStringRef QCFString::toCFStringRef(const QString &string)
{
   return CFStringCreateWithCharacters(0, reinterpret_cast<const UniChar *>(string.unicode()),
                                       string.length());
}

QCFString::operator CFStringRef() const
{
   if (!type) {
      const_cast<QCFString *>(this)->type =
         CFStringCreateWithCharactersNoCopy(0,
                                            reinterpret_cast<const UniChar *>(string.unicode()),
                                            string.length(),
                                            kCFAllocatorNull);
   }
   return type;
}


#if !defined(Q_OS_IOS)
void qt_mac_to_pascal_string(const QString &s, Str255 str, TextEncoding encoding, int len)
{
   if (len == -1) {
      len = s.length();
   }

   Q_UNUSED(encoding);
   CFStringGetPascalString(QCFString(s), str, 256, CFStringGetSystemEncoding());

}

QString qt_mac_from_pascal_string(const Str255 pstr)
{
   return QCFString(CFStringCreateWithPascalString(0, pstr, CFStringGetSystemEncoding()));
}

OSErr qt_mac_create_fsref(const QString &file, FSRef *fsref)
{
   return FSPathMakeRef(reinterpret_cast<const UInt8 *>(file.toUtf8().constData()), fsref, 0);
}

// Don't use this function, it won't work in 10.5 (Leopard) and up
OSErr qt_mac_create_fsspec(const QString &file, FSSpec *spec)
{
   FSRef fsref;
   OSErr ret = qt_mac_create_fsref(file, &fsref);
   if (ret == noErr) {
      ret = FSGetCatalogInfo(&fsref, kFSCatInfoNone, 0, 0, spec, 0);
   }
   return ret;
}
#endif // !defined(Q_OS_IOS)

QT_END_NAMESPACE
