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

#include <qsharedpointer.h>

// ensure we are not causing a namespace clash
#include <qshareddata.h>

#include <qset.h>
#include <qmutex.h>

void QtSharedPointer::ExternalRefCountData::setQObjectShared(const QObject *, bool)
{
}

void QtSharedPointer::ExternalRefCountData::checkQObjectShared(const QObject *)
{
   if (strongref.load() < 0) {
      qWarning("QSharedPointer: Can not create a QSharedPointer from a QObject, tracking QWeakPointer");
   }
}

QtSharedPointer::ExternalRefCountData *QtSharedPointer::ExternalRefCountData::getAndRef(const QObject *obj)
{
   Q_ASSERT(obj);

   bool wasDeleted = CSInternalRefCount::get_m_wasDeleted(obj);
   std::atomic<QtSharedPointer::ExternalRefCountData *> &sharedRefcount = CSInternalRefCount::get_m_SharedRefCount(obj);

   Q_ASSERT_X(! wasDeleted, "QWeakPointer", "Detected QWeakPointer creation in a QObject being deleted");

   ExternalRefCountData *that = sharedRefcount.load();
   if (that) {
      that->weakref.ref();
      return that;
   }

   // 2 = QWeakPointer that called us plus the QObject itself
   ExternalRefCountData *x = new ExternalRefCountData(-1, 2);

   ExternalRefCountData *newPtr = nullptr;

   if (! sharedRefcount.compare_exchange_strong(newPtr, x, std::memory_order_release, std::memory_order_acquire)) {
      delete x;

      newPtr->weakref.ref();
      x = newPtr;
   }

   return x;
}

QSharedPointer<QObject> QtSharedPointer::sharedPointerFromVariant_internal(const QVariant &variant)
{
   return variant.getData<QSharedPointer<QObject>>();
}

QWeakPointer<QObject> QtSharedPointer::weakPointerFromVariant_internal(const QVariant &variant)
{
   return variant.getData<QWeakPointer<QObject>>();
}

//#  define QT_SHARED_POINTER_BACKTRACE_SUPPORT
#ifdef QT_SHARED_POINTER_BACKTRACE_SUPPORT

#if defined(__GLIBC__) && (__GLIBC__ >= 2) && ! defined(__UCLIBC__) && ! defined(QT_LINUXBASE)
#      define BACKTRACE_SUPPORTED

#elif defined(Q_OS_DARWIN)
#      define BACKTRACE_SUPPORTED
#endif

#endif

#if defined(BACKTRACE_SUPPORTED)
#    include <sys/types.h>
#    include <execinfo.h>
#    include <stdio.h>
#    include <unistd.h>
#    include <sys/wait.h>

static inline QByteArray saveBacktrace() __attribute__((always_inline));
static inline QByteArray saveBacktrace()
{
   static const int maxFrames = 32;

   QByteArray stacktrace;
   stacktrace.resize(sizeof(void *) * maxFrames);
   int stack_size = backtrace((void **)stacktrace.data(), maxFrames);
   stacktrace.resize(sizeof(void *) * stack_size);

   return stacktrace;
}

static void printBacktrace(QByteArray stacktrace)
{
   void *const *stack = (void *const *)stacktrace.constData();
   int stack_size = stacktrace.size() / sizeof(void *);
   char **stack_symbols = backtrace_symbols(stack, stack_size);

   int filter[2];
   pid_t child = -1;
   if (pipe(filter) != -1) {
      child = fork();
   }

   if (child == 0) {
      // child process
      dup2(fileno(stderr), fileno(stdout));
      dup2(filter[0], fileno(stdin));
      close(filter[0]);
      close(filter[1]);
      execlp("c++filt", "c++filt", "-n", nullptr);

      // execlp failed
      execl("/bin/cat", "/bin/cat", nullptr);
      _exit(127);
   }

   // parent process
   close(filter[0]);
   FILE *output;
   if (child == -1) {
      // failed forking
      close(filter[1]);
      output = stderr;
   } else {
      output = fdopen(filter[1], "w");
   }

   fprintf(stderr, "Backtrace of the first creation (most recent frame first):\n");
   for (int i = 0; i < stack_size; ++i) {
      if (strlen(stack_symbols[i])) {
         fprintf(output, "#%-2d %s\n", i, stack_symbols[i]);
      } else {
         fprintf(output, "#%-2d %p\n", i, stack[i]);
      }
   }

   if (child != -1) {
      fclose(output);
      waitpid(child, 0, 0);
   }
}

#endif  // BACKTRACE_SUPPORTED

namespace {

struct Data {
   const volatile void *pointer;

#ifdef BACKTRACE_SUPPORTED
   QByteArray backtrace;
#endif
};

class KnownPointers
{
 public:
   QMutex mutex;
   QHash<const void *, Data> dPointers;
   QHash<const volatile void *, const void *> dataPointers;
};
}

Q_GLOBAL_STATIC(KnownPointers, knownPointers)

void QtSharedPointer::internalSafetyCheckAdd(const void *d_ptr, const volatile void *ptr)
{
   KnownPointers *const kp = knownPointers();
   if (! kp) {
      return;   // end-game: the application is being destroyed already
   }

   QMutexLocker lock(&kp->mutex);
   Q_ASSERT(!kp->dPointers.contains(d_ptr));

   //qDebug("Adding d=%p value=%p", d_ptr, ptr);

   const void *other_d_ptr = kp->dataPointers.value(ptr, nullptr);
   if (other_d_ptr) {

#ifdef BACKTRACE_SUPPORTED
      printBacktrace(knownPointers()->dPointers.value(other_d_ptr).backtrace);
#endif

      qFatal("QSharedPointer: internal self-check failed: pointer %p was already tracked "
             "by another QSharedPointer object %p", ptr, other_d_ptr);
   }

   Data data;
   data.pointer = ptr;

#ifdef BACKTRACE_SUPPORTED
   data.backtrace = saveBacktrace();
#endif

   kp->dPointers.insert(d_ptr, data);
   kp->dataPointers.insert(ptr, d_ptr);
   Q_ASSERT(kp->dPointers.size() == kp->dataPointers.size());
}

void QtSharedPointer::internalSafetyCheckRemove(const void *d_ptr)
{
   KnownPointers *const kp = knownPointers();
   if (! kp) {
      return;   // end-game, the application is being destroyed already
   }

   QMutexLocker lock(&kp->mutex);

   QHash<const void *, Data>::iterator it = kp->dPointers.find(d_ptr);

   if (it == kp->dPointers.end()) {
      qFatal("QSharedPointer: internal self check inconsistency: Pointer %p was not valid. "
             "To use QT_SHAREDPOINTER_TRACK_POINTERS enable it in your application", d_ptr);
   }

   QHash<const volatile void *, const void *>::iterator it2 = kp->dataPointers.find(it->pointer);
   Q_ASSERT(it2 != kp->dataPointers.end());

   // remove entries
   kp->dataPointers.erase(it2);
   kp->dPointers.erase(it);
   Q_ASSERT(kp->dPointers.size() == kp->dataPointers.size());
}

