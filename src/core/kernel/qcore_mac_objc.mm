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

#include <qcore_mac_p.h>

#include <qdebug.h>

#if defined(Q_OS_DARWIN) && ! defined(Q_OS_IOS)
#include <AppKit/NSText.h>
#include <Carbon/Carbon.h>
#endif

#ifdef Q_OS_IOS
#import <UIKit/UIKit.h>
#endif

NSString *QCFString::toNSString(const QString &string)
{
   // The const cast below is safe: CfStringRef is immutable and so is NSString.
   return [const_cast<NSString *>(reinterpret_cast<const NSString *>(toCFStringRef(string))) autorelease];
}

QString QCFString::toQString(const NSString *nsstr)
{
   return toQString(reinterpret_cast<CFStringRef>(nsstr));
}

QDebug operator<<(QDebug dbg, const NSObject *nsObject)
{
   return dbg << (nsObject ? nsObject.description.UTF8String : "NSObject(0x0)");
}

QDebug operator<<(QDebug dbg, CFStringRef stringRef)
{
   if (! stringRef) {
      return dbg << "CFStringRef(0x0)";
   }

   dbg << QString::fromCFString(stringRef);

   return dbg;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CFArrayRef ref)
{
   if (! ref)  {
      return debug << "CFArrayRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CFURLRef ref)
{
   if (! ref)  {
      return debug << "CFURLRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CFDataRef ref)
{
   if (! ref)  {
      return debug << "CFDataRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CFNumberRef ref)
{
   if (! ref)  {
      return debug << "CFNumberRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CFDictionaryRef ref)
{
   if (! ref)  {
      return debug << "CFDictionaryRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CFLocaleRef ref)
{
   if (! ref)  {
      return debug << "CFLocaleRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CFDateRef ref)
{
   if (! ref)  {
      return debug << "CFDateRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CFBooleanRef ref)
{
   if (! ref)  {
      return debug << "CFBooleanRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CFTimeZoneRef ref)
{
   if (! ref)  {
      return debug << "CFTimeZoneRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CFErrorRef ref)
{
   if (! ref)  {
      return debug << "CFErrorRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CFBundleRef ref)
{
   if (! ref)  {
      return debug << "CFBundleRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CGPathRef ref)
{
   if (! ref)  {
      return debug << "CGPathRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CGColorSpaceRef ref)
{
   if (! ref)  {
      return debug << "CGColorSpaceRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CGImageRef ref)
{
   if (! ref)  {
      return debug << "CGImageRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CGFontRef ref)
{
   if (! ref)  {
      return debug << "CGFontRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

__attribute__((weak)) QDebug operator<<(QDebug debug, CGColorRef ref)
{
   if (! ref)  {
      return debug << "CGColorRef(0x0)";
   }

   if (CFStringRef description = CFCopyDescription(ref)) {
      // QDebugStateSaver saver(debug);
      // debug.noquote();

      debug << description;
      CFRelease(description);
   }

   return debug;
}

QAppleOperatingSystemVersion qt_apple_os_version()
{
   QAppleOperatingSystemVersion v = {0, 0, 0};

   if ([NSProcessInfo instancesRespondToSelector:@selector(operatingSystemVersion)]) {
      NSOperatingSystemVersion osv = NSProcessInfo.processInfo.operatingSystemVersion;
      v.major = osv.majorVersion;
      v.minor = osv.minorVersion;
      v.patch = osv.patchVersion;
      return v;
   }

   // Use temporary variables so we can return 0.0.0 (unknown version)
   // in case of an error partway through determining the OS version
   qint32 major = 0, minor = 0, patch = 0;

   v.major = major;
   v.minor = minor;
   v.patch = patch;

   return v;
}

QMacAutoReleasePool::QMacAutoReleasePool()
   : pool([[NSAutoreleasePool alloc] init])
{
}

QMacAutoReleasePool::~QMacAutoReleasePool()
{
   // Drain behaves the same as release, with the advantage that
   // if we're ever used in a garbage-collected environment, the
   // drain acts as a hint to the garbage collector to collect.
   [static_cast<NSAutoreleasePool *>(pool) drain];
}

#if defined(Q_OS_DARWIN) && ! defined(Q_OS_IOS)

// Use this method to keep all the information in the TextSegment. As long as it is ordered
// we are in OK shape, and we can influence that ourselves.
struct KeyPair {
   QChar cocoaKey;
   Qt::Key qtKey;
};

bool operator==(const KeyPair &entry, QChar qchar)
{
   return entry.cocoaKey == qchar;
}

bool operator<(const KeyPair &entry, QChar qchar)
{
   return entry.cocoaKey < qchar;
}

bool operator<(QChar qchar, const KeyPair &entry)
{
   return qchar < entry.cocoaKey;
}

bool operator<(const Qt::Key &key, const KeyPair &entry)
{
   return key < entry.qtKey;
}

bool operator<(const KeyPair &entry, const Qt::Key &key)
{
   return entry.qtKey < key;
}

struct qtKey2CocoaKeySortLessThan {
   using result_type = bool;

   constexpr result_type operator()(const KeyPair &entry1, const KeyPair &entry2) const {
      return entry1.qtKey < entry2.qtKey;
   }
};

static constexpr const int NumEntries = 59;

static const KeyPair entries[NumEntries] = {
   { NSEnterCharacter, Qt::Key_Enter },
   { NSBackspaceCharacter, Qt::Key_Backspace },
   { NSTabCharacter, Qt::Key_Tab },
   { NSNewlineCharacter, Qt::Key_Return },
   { NSCarriageReturnCharacter, Qt::Key_Return },
   { NSBackTabCharacter, Qt::Key_Backtab },
   { kEscapeCharCode, Qt::Key_Escape },
   // Cocoa sends us delete when pressing backspace!
   // (NB when we reverse this list in qtKey2CocoaKey, there
   // will be two indices of Qt::Key_Backspace. But is seems to work
   // ok for menu shortcuts (which uses that function):
   { NSDeleteCharacter, Qt::Key_Backspace },
   { NSUpArrowFunctionKey, Qt::Key_Up },
   { NSDownArrowFunctionKey, Qt::Key_Down },
   { NSLeftArrowFunctionKey, Qt::Key_Left },
   { NSRightArrowFunctionKey, Qt::Key_Right },
   { NSF1FunctionKey, Qt::Key_F1 },
   { NSF2FunctionKey, Qt::Key_F2 },
   { NSF3FunctionKey, Qt::Key_F3 },
   { NSF4FunctionKey, Qt::Key_F4 },
   { NSF5FunctionKey, Qt::Key_F5 },
   { NSF6FunctionKey, Qt::Key_F6 },
   { NSF7FunctionKey, Qt::Key_F7 },
   { NSF8FunctionKey, Qt::Key_F8 },
   { NSF9FunctionKey, Qt::Key_F9 },
   { NSF10FunctionKey, Qt::Key_F10 },
   { NSF11FunctionKey, Qt::Key_F11 },
   { NSF12FunctionKey, Qt::Key_F12 },
   { NSF13FunctionKey, Qt::Key_F13 },
   { NSF14FunctionKey, Qt::Key_F14 },
   { NSF15FunctionKey, Qt::Key_F15 },
   { NSF16FunctionKey, Qt::Key_F16 },
   { NSF17FunctionKey, Qt::Key_F17 },
   { NSF18FunctionKey, Qt::Key_F18 },
   { NSF19FunctionKey, Qt::Key_F19 },
   { NSF20FunctionKey, Qt::Key_F20 },
   { NSF21FunctionKey, Qt::Key_F21 },
   { NSF22FunctionKey, Qt::Key_F22 },
   { NSF23FunctionKey, Qt::Key_F23 },
   { NSF24FunctionKey, Qt::Key_F24 },
   { NSF25FunctionKey, Qt::Key_F25 },
   { NSF26FunctionKey, Qt::Key_F26 },
   { NSF27FunctionKey, Qt::Key_F27 },
   { NSF28FunctionKey, Qt::Key_F28 },
   { NSF29FunctionKey, Qt::Key_F29 },
   { NSF30FunctionKey, Qt::Key_F30 },
   { NSF31FunctionKey, Qt::Key_F31 },
   { NSF32FunctionKey, Qt::Key_F32 },
   { NSF33FunctionKey, Qt::Key_F33 },
   { NSF34FunctionKey, Qt::Key_F34 },
   { NSF35FunctionKey, Qt::Key_F35 },
   { NSInsertFunctionKey, Qt::Key_Insert },
   { NSDeleteFunctionKey, Qt::Key_Delete },
   { NSHomeFunctionKey, Qt::Key_Home },
   { NSEndFunctionKey, Qt::Key_End },
   { NSPageUpFunctionKey, Qt::Key_PageUp },
   { NSPageDownFunctionKey, Qt::Key_PageDown },
   { NSPrintScreenFunctionKey, Qt::Key_Print },
   { NSScrollLockFunctionKey, Qt::Key_ScrollLock },
   { NSPauseFunctionKey, Qt::Key_Pause },
   { NSSysReqFunctionKey, Qt::Key_SysReq },
   { NSMenuFunctionKey, Qt::Key_Menu },
   { NSHelpFunctionKey, Qt::Key_Help },
};

static constexpr const KeyPair *const end = entries + NumEntries;

QChar qt_mac_qtKey2CocoaKey(Qt::Key key)
{
   // first time this function is called create a lookup table sorted on our Key rather than Cocoa key
   static QVector<KeyPair> rev_entries(NumEntries);
   static bool mustInit = true;

   if (mustInit) {
      mustInit = false;

      for (int i = 0; i < NumEntries; ++i) {
         rev_entries[i] = entries[i];
      }

      std::sort(rev_entries.begin(), rev_entries.end(), qtKey2CocoaKeySortLessThan());
   }

   const QVector<KeyPair>::iterator i = std::lower_bound(rev_entries.begin(), rev_entries.end(), key);

   if ((i == rev_entries.end()) || (key < *i)) {
      return QChar();
   }

   return i->cocoaKey;
}

Qt::Key qt_mac_cocoaKey2QtKey(QChar keyCode)
{
   const KeyPair *i = std::lower_bound(entries, end, keyCode);

   if ((i == end) || (keyCode < *i)) {
      return Qt::Key(keyCode.toUpper()[0].unicode());
   }

   return i->qtKey;
}

#endif
