/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QSHAREDPOINTER_H
#error Do not include qsharedpointer_impl.h directly
#endif

#ifndef QSHAREDPOINTER_IMPL_H
#define QSHAREDPOINTER_IMPL_H

#include <new>
#include <utility>

#include <qatomic.h>
#include <qhashfwd.h>

class QVariant;
class QObject;

template <class T>
class QWeakPointer;

template <class T>
class QSharedPointer;

template <class T>
class QEnableSharedFromThis;

template <class X, class T>
QSharedPointer<X> qSharedPointerCast(const QSharedPointer<T> &ptr);

template <class X, class T>
QSharedPointer<X> qSharedPointerDynamicCast(const QSharedPointer<T> &ptr);

template <class X, class T>
QSharedPointer<X> qSharedPointerConstCast(const QSharedPointer<T> &ptr);

template <class X, class T>
QSharedPointer<X> qSharedPointerObjectCast(const QSharedPointer<T> &ptr);


#ifdef QT_NO_DEBUG
# define QSHAREDPOINTER_VERIFY_AUTO_CAST(T, X)          qt_noop()
#else

template<typename T> inline void qt_sharedpointer_cast_check(T *) { }
# define QSHAREDPOINTER_VERIFY_AUTO_CAST(T, X)          \
    qt_sharedpointer_cast_check<T>(static_cast<X *>(0))
#endif

namespace QtSharedPointer {

template <class T> class InternalRefCount;
template <class T> class ExternalRefCount;

template <class X, class Y> QSharedPointer<X> copyAndSetPointer(X *ptr, const QSharedPointer<Y> &src);

// used in debug mode to verify the reuse of pointers
Q_CORE_EXPORT void internalSafetyCheckAdd(const void *, const volatile void *);
Q_CORE_EXPORT void internalSafetyCheckRemove(const void *);

template <class T, typename Class, typename RetVal>
inline void executeDeleter(T *t, RetVal (Class:: *memberDeleter)())
{
   (t->*memberDeleter)();
}

template <class T, typename Deleter>
inline void executeDeleter(T *t, Deleter d)
{
   d(t);
}
struct NormalDeleter {};

template <class T> struct RemovePointer;

template <class T> struct RemovePointer<T *> {
   typedef T Type;
};

template <class T> struct RemovePointer<QSharedPointer<T> > {
   typedef T Type;
};

template <class T> struct RemovePointer<QWeakPointer<T> > {
   typedef T Type;
};

// This class is the d-pointer of QSharedPointer and QWeakPointer.

// It is a reference-counted reference counter. "strongref" is the inner
// reference counter, and it tracks the lifetime of the pointer itself.
// "weakref" is the outer reference counter and it tracks the lifetime of
// the ExternalRefCountData object.

// The deleter is stored in the destroyer member and is always a pointer to
// a static function in ExternalRefCountWithCustomDeleter or in
// ExternalRefCountWithContiguousData

struct ExternalRefCountData {
   typedef void (*DestroyerFn)(ExternalRefCountData *);

   QAtomicInt weakref;
   QAtomicInt strongref;

   DestroyerFn destroyer;

   inline ExternalRefCountData(DestroyerFn d)
      : destroyer(d) {
      strongref.store(1);
      weakref.store(1);
   }

   inline ExternalRefCountData(Qt::Initialization) { }
   ~ExternalRefCountData() {
      Q_ASSERT(! weakref.load());
      Q_ASSERT(strongref.load() <= 0);
   }

   void destroy() {
      destroyer(this);
   }

   Q_CORE_EXPORT static ExternalRefCountData *getAndRef(const QObject *);
   Q_CORE_EXPORT void setQObjectShared(const QObject *, bool enable);
   Q_CORE_EXPORT void checkQObjectShared(const QObject *);

   inline void checkQObjectShared(...) { }
   inline void setQObjectShared(...) { }

   inline void operator delete(void *ptr) {
      ::operator delete(ptr);
   }
   inline void operator delete(void *, void *) { }
};
// sizeof(ExternalRefCountData) = 12 (32-bit) / 16 (64-bit)

template <class T, typename Deleter>
struct CustomDeleter {
   Deleter deleter;
   T *ptr;

   CustomDeleter(T *p, Deleter d) : deleter(d), ptr(p) {}
   void execute() {
      executeDeleter(ptr, deleter);
   }
};
// sizeof(CustomDeleter) = sizeof(Deleter) + sizeof(void*) + padding
// for Deleter = stateless functor: 8 (32-bit) / 16 (64-bit) due to padding
// for Deleter = function pointer:  8 (32-bit) / 16 (64-bit)
// for Deleter = PMF: 12 (32-bit) / 24 (64-bit)  (GCC)

// This specialization of CustomDeleter for a deleter of type NormalDeleter
// is an optimization: instead of storing a pointer to a function that does
// the deleting, we simply delete the pointer ourselves.
template <class T>
struct CustomDeleter<T, NormalDeleter> {
   T *ptr;

   CustomDeleter(T *p, NormalDeleter) : ptr(p) {}
   void execute() {
      delete ptr;
   }
};
// sizeof(CustomDeleter specialization) = sizeof(void*)

// This class extends ExternalRefCountData and implements
// the static function that deletes the object. The pointer and the
// custom deleter are kept in the "extra" member so we can construct
// and destruct it independently of the full structure.
template <class T, typename Deleter>
struct ExternalRefCountWithCustomDeleter: public ExternalRefCountData {
   typedef ExternalRefCountWithCustomDeleter Self;
   typedef ExternalRefCountData BaseClass;
   CustomDeleter<T, Deleter> extra;

   static inline void deleter(ExternalRefCountData *self) {
      Self *realself = static_cast<Self *>(self);
      realself->extra.execute();

      // delete the deleter too
      realself->extra.~CustomDeleter<T, Deleter>();
   }
   static void safetyCheckDeleter(ExternalRefCountData *self) {
      internalSafetyCheckRemove(self);
      deleter(self);
   }

   static inline Self *create(T *ptr, Deleter userDeleter, DestroyerFn actualDeleter) {
      Self *d = static_cast<Self *>(::operator new(sizeof(Self)));

      // initialize the two sub-objects
      new (&d->extra) CustomDeleter<T, Deleter>(ptr, userDeleter);
      new (d) BaseClass(actualDeleter); // can't throw

      return d;
   }

 private:
   // prevent construction
   ExternalRefCountWithCustomDeleter()  = delete;
   ~ExternalRefCountWithCustomDeleter() = delete;
   Q_DISABLE_COPY(ExternalRefCountWithCustomDeleter)
};

// This class extends ExternalRefCountData and adds a "T"
// member. That way, when the create() function is called, we allocate
// memory for both QSharedPointer's d-pointer and the actual object being tracked.

template <class T>
struct ExternalRefCountWithContiguousData: public ExternalRefCountData {
   typedef ExternalRefCountData Parent;
   T data;

   static void deleter(ExternalRefCountData *self) {
      ExternalRefCountWithContiguousData *that =
         static_cast<ExternalRefCountWithContiguousData *>(self);
      that->data.~T();
   }
   static void safetyCheckDeleter(ExternalRefCountData *self) {
      internalSafetyCheckRemove(self);
      deleter(self);
   }

   static inline ExternalRefCountData *create(T **ptr, DestroyerFn destroy) {
      ExternalRefCountWithContiguousData *d =
         static_cast<ExternalRefCountWithContiguousData *>(::operator new(sizeof(ExternalRefCountWithContiguousData)));

      // initialize the d-pointer sub-object
      // leave d->data uninitialized
      new (d) Parent(destroy); // can't throw

      *ptr = &d->data;
      return d;
   }

 private:
   // prevent construction
   ExternalRefCountWithContiguousData()  = delete;
   ~ExternalRefCountWithContiguousData() = delete;
   Q_DISABLE_COPY(ExternalRefCountWithContiguousData)
};

Q_CORE_EXPORT QWeakPointer<QObject> weakPointerFromVariant_internal(const QVariant &variant);
Q_CORE_EXPORT QSharedPointer<QObject> sharedPointerFromVariant_internal(const QVariant &variant);
} // namespace QtSharedPointer

template <class T> class QSharedPointer
{
   typedef T *QSharedPointer::*RestrictedBool;
   typedef QtSharedPointer::ExternalRefCountData Data;

 public:
   typedef T Type;
   typedef T element_type;
   typedef T value_type;
   typedef value_type *pointer;
   typedef const value_type *const_pointer;
   typedef value_type &reference;
   typedef const value_type &const_reference;
   typedef qptrdiff difference_type;

   inline T *data() const {
      return value;
   }

   inline bool isNull() const {
      return !data();
   }

   inline operator RestrictedBool() const {
      return isNull() ? 0 : &QSharedPointer::value;
   }

   inline bool operator !() const {
      return isNull();
   }

   inline T &operator*() const {
      return *data();
   }

/*
   // method ptr call
   inline T &operator->*() const {
      return *data();
   }
*/

   inline T *operator->() const {
      return data();
   }

   QSharedPointer() : value(0), d(0) { }
   ~QSharedPointer() {
      deref();
   }

   inline explicit QSharedPointer(T *ptr) : value(ptr) { // noexcept
      internalConstruct(ptr, QtSharedPointer::NormalDeleter());
   }

   template <typename Deleter>
   inline QSharedPointer(T *ptr, Deleter deleter) : value(ptr) { // throws
      internalConstruct(ptr, deleter);
   }

   inline QSharedPointer(const QSharedPointer<T> &other) : value(other.value), d(other.d) {
      if (d) {
         ref();
      }
   }
   inline QSharedPointer<T> &operator=(const QSharedPointer<T> &other) {
      QSharedPointer copy(other);
      swap(copy);
      return *this;
   }

   inline QSharedPointer<T> &operator=(QSharedPointer<T> && other) {
      swap(other);
      return *this;
   }

   template <class X>
   inline QSharedPointer(const QSharedPointer<X> &other) : value(other.value), d(other.d) {
      if (d) {
         ref();
      }
   }

   template <class X>
   inline QSharedPointer<T> &operator=(const QSharedPointer<X> &other) {
      QSHAREDPOINTER_VERIFY_AUTO_CAST(T, X); // if you get an error in this line, the cast is invalid
      internalCopy(other);
      return *this;
   }

   template <class X>
   inline QSharedPointer(const QWeakPointer<X> &other) : value(0), d(0) {
      *this = other;
   }

   template <class X>
   inline QSharedPointer<T> &operator=(const QWeakPointer<X> &other) {
      internalSet(other.d, other.value);
      return *this;
   }

   inline void swap(QSharedPointer &other) {
      QSharedPointer<T>::internalSwap(other);
   }

   inline void reset() {
      clear();
   }
   inline void reset(T *t) {
      QSharedPointer copy(t);
      swap(copy);
   }
   template <typename Deleter>
   inline void reset(T *t, Deleter deleter) {
      QSharedPointer copy(t, deleter);
      swap(copy);
   }

   template <class X>
   QSharedPointer<X> staticCast() const {
      return qSharedPointerCast<X, T>(*this);
   }

   template <class X>
   QSharedPointer<X> dynamicCast() const {
      return qSharedPointerDynamicCast<X, T>(*this);
   }

   template <class X>
   QSharedPointer<X> constCast() const {
      return qSharedPointerConstCast<X, T>(*this);
   }

   template <class X>
   QSharedPointer<X> objectCast() const {
      return qSharedPointerObjectCast<X, T>(*this);
   }

   inline void clear() {
      QSharedPointer copy;
      swap(copy);
   }

   QWeakPointer<T> toWeakRef() const;

   template <typename... Args>
   static QSharedPointer<T> create(Args  &&...arguments) {
      typedef QtSharedPointer::ExternalRefCountWithContiguousData<T> Private;

# ifdef QT_SHAREDPOINTER_TRACK_POINTERS
      typename Private::DestroyerFn destroy = &Private::safetyCheckDeleter;
# else
      typename Private::DestroyerFn destroy = &Private::deleter;
# endif

      QSharedPointer<T> result(Qt::Uninitialized);
      result.d = Private::create(&result.value, destroy);

      // now initialize the data
      new (result.data()) T(std::forward<Args>(arguments)...);
      result.d->setQObjectShared(result.value, true);

# ifdef QT_SHAREDPOINTER_TRACK_POINTERS
      internalSafetyCheckAdd(result.d, result.value);

# endif
      return result;
   }

 private:
   explicit QSharedPointer(Qt::Initialization) {}

   template <class X>
   inline void enableSharedFromThis(const QEnableSharedFromThis<X> *ptr)
   {
       ptr->initializeFromSharedPointer(*this);
   }

   inline void enableSharedFromThis(...) {}

   inline void deref() {
      deref(d);
   }
   static inline void deref(Data *d) {
      if (!d) {
         return;
      }
      if (!d->strongref.deref()) {
         d->destroy();
      }
      if (!d->weakref.deref()) {
         delete d;
      }
   }

   template <typename Deleter>
   inline void internalConstruct(T *ptr, Deleter deleter) {
      if (!ptr) {
         d = 0;
         return;
      }

      typedef QtSharedPointer::ExternalRefCountWithCustomDeleter<T, Deleter> Private;
# ifdef QT_SHAREDPOINTER_TRACK_POINTERS
      typename Private::DestroyerFn actualDeleter = &Private::safetyCheckDeleter;
# else
      typename Private::DestroyerFn actualDeleter = &Private::deleter;
# endif
      d = Private::create(ptr, deleter, actualDeleter);

#ifdef QT_SHAREDPOINTER_TRACK_POINTERS
      internalSafetyCheckAdd(d, ptr);
#endif
      d->setQObjectShared(ptr, true);
      enableSharedFromThis(ptr);
   }

   template <class X>
   inline void internalCopy(const QSharedPointer<X> &other) {
      Data *o = other.d;
      T *actual = other.value;
      if (o) {
         other.ref();
      }
      qSwap(d, o);
      qSwap(this->value, actual);
      deref(o);
   }

   inline void internalSwap(QSharedPointer &other) {
      qSwap(d, other.d);
      qSwap(this->value, other.value);
   }

#if ! defined (CS_DOXYPRESS)
   template <class X>
   friend class QSharedPointer;

   template <class X>
   friend class QWeakPointer;

   template <class X, class Y>
   friend QSharedPointer<X> QtSharedPointer::copyAndSetPointer(X *ptr,
         const QSharedPointer<Y> &src);
#endif

   inline void ref() const {
      d->weakref.ref();
      d->strongref.ref();
   }

   inline void internalSet(Data *o, T *actual) {

      if (o) {
         // increase the strongref, but never up from zero
         // or less (-1 is used by QWeakPointer on untracked QObject)
         int tmp = o->strongref.load();

         while (tmp > 0) {
            // try to increment from "tmp" to "tmp + 1"
            if (o->strongref.testAndSetRelaxed(tmp, tmp + 1)) {
               break;   // succeeded
            }
            tmp = o->strongref.load();  // failed, try again
         }

         if (tmp > 0) {
            o->weakref.ref();
         } else {
            o->checkQObjectShared(actual);
            o = 0;
         }
      }

      qSwap(d, o);
      qSwap(this->value, actual);
      if (!d || d->strongref.load() == 0) {
         this->value = 0;
      }

      // dereference saved data
      deref(o);
   }

   Type *value;
   Data *d;
};

template <class T>
class QWeakPointer
{
   typedef T *QWeakPointer::*RestrictedBool;

   typedef QtSharedPointer::ExternalRefCountData Data;

 public:
   typedef T element_type;
   typedef T value_type;
   typedef value_type *pointer;
   typedef const value_type *const_pointer;
   typedef value_type &reference;
   typedef const value_type &const_reference;
   typedef qptrdiff difference_type;

   inline bool isNull() const {
      return d == 0 || d->strongref.load() == 0 || value == 0;
   }

   inline operator RestrictedBool() const {
      return isNull() ? 0 : &QWeakPointer::value;
   }

   inline bool operator !() const {
      return isNull();
   }

   inline T *data() const {
      return d == 0 || d->strongref.load() == 0 ? 0 : value;
   }

   inline QWeakPointer() : d(0), value(0) { }
   inline ~QWeakPointer() {
      if (d && !d->weakref.deref()) {
         delete d;
      }
   }

   // special constructor enabled only if X derives from QObject
   template <class X, typename O = QObject, typename = typename std::enable_if<std::is_base_of<O, X>::value>::type>
   inline QWeakPointer(X *ptr) : d(ptr ? Data::getAndRef(ptr) : 0), value(ptr)
   { }

   template <class X, typename O = QObject, typename = typename std::enable_if<std::is_base_of<O, X>::value>::type>
   inline QWeakPointer &operator=(X *ptr)
   {
      return *this = QWeakPointer(ptr);
   }

   inline QWeakPointer(const QWeakPointer<T> &other) : d(other.d), value(other.value) {
      if (d) {
         d->weakref.ref();
      }
   }

   inline QWeakPointer<T> &operator=(const QWeakPointer<T> &other) {
      internalSet(other.d, other.value);
      return *this;
   }

   inline QWeakPointer(const QSharedPointer<T> &other) : d(other.d), value(other.data()) {
      if (d) {
         d->weakref.ref();
      }
   }

   inline QWeakPointer<T> &operator=(const QSharedPointer<T> &other) {
      internalSet(other.d, other.value);
      return *this;
   }

   template <class X>
   inline QWeakPointer(const QWeakPointer<X> &other) : d(0), value(0) {
      *this = other;
   }

   template <class X>
   inline QWeakPointer<T> &operator=(const QWeakPointer<X> &other) {
      // conversion between X and T could require access to the virtual table
      // so force the operation to go through QSharedPointer
      *this = other.toStrongRef();
      return *this;
   }

   template <class X>
   inline bool operator==(const QWeakPointer<X> &other) const {
      return d == other.d && value == static_cast<const T *>(other.value);
   }

   template <class X>
   inline bool operator!=(const QWeakPointer<X> &other) const {
      return !(*this == other);
   }

   template <class X>
   inline QWeakPointer(const QSharedPointer<X> &other) : d(0), value(0) {
      *this = other;
   }

   template <class X>
   inline QWeakPointer<T> &operator=(const QSharedPointer<X> &other) {
      QSHAREDPOINTER_VERIFY_AUTO_CAST(T, X); // if you get an error in this line, the cast is invalid
      internalSet(other.d, other.data());
      return *this;
   }

   template <class X>
   inline bool operator==(const QSharedPointer<X> &other) const {
      return d == other.d;
   }

   template <class X>
   inline bool operator!=(const QSharedPointer<X> &other) const {
      return !(*this == other);
   }

   inline void clear() {
      *this = QWeakPointer<T>();
   }

   inline QSharedPointer<T> toStrongRef() const {
      return QSharedPointer<T>(*this);
   }

#if defined(QWEAKPOINTER_ENABLE_ARROW)
   inline T *operator->() const {
      return data();
   }
#endif

 private:
   template <class X>
   inline QWeakPointer &assign(X *ptr) {
      return *this = QWeakPointer<X>(ptr, true);
   }

   template <class X>
   inline QWeakPointer(X *ptr, bool) : d(ptr ? Data::getAndRef(ptr) : 0), value(ptr) {
   }

   inline void internalSet(Data *o, T *actual) {
      if (d == o) {
         return;
      }
      if (o) {
         o->weakref.ref();
      }
      if (d && !d->weakref.deref()) {
         delete d;
      }
      d = o;
      value = actual;
   }

   Data *d;
   T *value;

   template <class X>
   friend class QSharedPointer;

   template <class X>
   friend class QPointer;
};

template <class T>
class QEnableSharedFromThis
{
 public:
   QSharedPointer<T> sharedFromThis() {
	   return QSharedPointer<T>(weakPointer);
   }

   QSharedPointer<const T> sharedFromThis() const {
	   return QSharedPointer<const T>(weakPointer);
   }

 protected:
   QEnableSharedFromThis() = default;
   QEnableSharedFromThis(const QEnableSharedFromThis &) { }

   QEnableSharedFromThis &operator=(const QEnableSharedFromThis &) {
      return *this;
   }

 private:
   template <class X>
   void initializeFromSharedPointer(const QSharedPointer<X> &ptr) const {
      weakPointer = ptr;
   }

   mutable QWeakPointer<T> weakPointer;

   template <class X>
   friend class QSharedPointer;
};

//
// operator== and operator!=
//
template <class T, class X>
bool operator==(const QSharedPointer<T> &ptr1, const QSharedPointer<X> &ptr2)
{
   return ptr1.data() == ptr2.data();
}
template <class T, class X>
bool operator!=(const QSharedPointer<T> &ptr1, const QSharedPointer<X> &ptr2)
{
   return ptr1.data() != ptr2.data();
}

template <class T, class X>
bool operator==(const QSharedPointer<T> &ptr1, const X *ptr2)
{
   return ptr1.data() == ptr2;
}
template <class T, class X>
bool operator==(const T *ptr1, const QSharedPointer<X> &ptr2)
{
   return ptr1 == ptr2.data();
}
template <class T, class X>
bool operator!=(const QSharedPointer<T> &ptr1, const X *ptr2)
{
   return !(ptr1 == ptr2);
}
template <class T, class X>
bool operator!=(const T *ptr1, const QSharedPointer<X> &ptr2)
{
   return !(ptr2 == ptr1);
}

template <class T, class X>
bool operator==(const QSharedPointer<T> &ptr1, const QWeakPointer<X> &ptr2)
{
   return ptr2 == ptr1;
}
template <class T, class X>
bool operator!=(const QSharedPointer<T> &ptr1, const QWeakPointer<X> &ptr2)
{
   return ptr2 != ptr1;
}

//
// operator-
//
template <class T, class X>
inline typename QSharedPointer<T>::difference_type operator-(const QSharedPointer<T> &ptr1,
      const QSharedPointer<X> &ptr2)
{
   return ptr1.data() - ptr2.data();
}
template <class T, class X>
inline typename QSharedPointer<T>::difference_type operator-(const QSharedPointer<T> &ptr1, X *ptr2)
{
   return ptr1.data() - ptr2;
}
template <class T, class X>
inline typename QSharedPointer<X>::difference_type operator-(T *ptr1, const QSharedPointer<X> &ptr2)
{
   return ptr1 - ptr2.data();
}

//
// operator<
//
template <class T, class X>
inline bool operator<(const QSharedPointer<T> &ptr1, const QSharedPointer<X> &ptr2)
{
   return ptr1.data() < ptr2.data();
}
template <class T, class X>
inline bool operator<(const QSharedPointer<T> &ptr1, X *ptr2)
{
   return ptr1.data() < ptr2;
}
template <class T, class X>
inline bool operator<(T *ptr1, const QSharedPointer<X> &ptr2)
{
   return ptr1 < ptr2.data();
}

//
// qHash
//
template <class T>
inline uint qHash(const QSharedPointer<T> &ptr, uint seed = 0)
{
   return QT_PREPEND_NAMESPACE(qHash)(ptr.data(), seed);
}

template<class T, class...Args>
inline QSharedPointer<T> QMakeShared(Args&&... args)
{
    return QSharedPointer<T>(new T(std::forward<Args>(args)...));
}

template <class T>
inline QWeakPointer<T> QSharedPointer<T>::toWeakRef() const
{
   return QWeakPointer<T>(*this);
}

template <class T>
inline void qSwap(QSharedPointer<T> &p1, QSharedPointer<T> &p2)
{
   p1.swap(p2);
}

QT_END_NAMESPACE
namespace std {
template <class T>
inline void swap(QT_PREPEND_NAMESPACE(QSharedPointer)<T> &p1, QT_PREPEND_NAMESPACE(QSharedPointer)<T> &p2)
{
   p1.swap(p2);
}
}
QT_BEGIN_NAMESPACE

namespace QtSharedPointer {
// helper functions:
template <class X, class T>
inline QSharedPointer<X> copyAndSetPointer(X *ptr, const QSharedPointer<T> &src)
{
   QSharedPointer<X> result;
   result.internalSet(src.d, ptr);
   return result;
}
}

// cast operators
template <class X, class T>
inline QSharedPointer<X> qSharedPointerCast(const QSharedPointer<T> &src)
{
   X *ptr = static_cast<X *>(src.data()); // if you get an error in this line, the cast is invalid
   return QtSharedPointer::copyAndSetPointer(ptr, src);
}
template <class X, class T>
inline QSharedPointer<X> qSharedPointerCast(const QWeakPointer<T> &src)
{
   return qSharedPointerCast<X, T>(src.toStrongRef());
}

template <class X, class T>
inline QSharedPointer<X> qSharedPointerDynamicCast(const QSharedPointer<T> &src)
{
   X *ptr = dynamic_cast<X *>(src.data()); // if you get an error in this line, the cast is invalid
   if (!ptr) {
      return QSharedPointer<X>();
   }
   return QtSharedPointer::copyAndSetPointer(ptr, src);
}
template <class X, class T>
inline QSharedPointer<X> qSharedPointerDynamicCast(const QWeakPointer<T> &src)
{
   return qSharedPointerDynamicCast<X, T>(src.toStrongRef());
}

template <class X, class T>
inline QSharedPointer<X> qSharedPointerConstCast(const QSharedPointer<T> &src)
{
   X *ptr = const_cast<X *>(src.data()); // if you get an error in this line, the cast is invalid
   return QtSharedPointer::copyAndSetPointer(ptr, src);
}

template <class X, class T>
inline QSharedPointer<X> qSharedPointerConstCast(const QWeakPointer<T> &src)
{
   return qSharedPointerConstCast<X, T>(src.toStrongRef());
}

template <class X, class T>
inline QWeakPointer<X> qWeakPointerCast(const QSharedPointer<T> &src)
{
   return qSharedPointerCast<X, T>(src).toWeakRef();
}


template <class X, class T>
inline QSharedPointer<X> qSharedPointerObjectCast(const QSharedPointer<T> &src)
{
   X *ptr = dynamic_cast<X *>(src.data());
   return QtSharedPointer::copyAndSetPointer(ptr, src);
}

template <class X, class T>
inline QSharedPointer<X> qSharedPointerObjectCast(const QWeakPointer<T> &src)
{
   return qSharedPointerObjectCast<X>(src.toStrongRef());
}

template <class X, class T>
inline QSharedPointer<typename QtSharedPointer::RemovePointer<X>::Type>
qobject_cast(const QSharedPointer<T> &src)
{
   return qSharedPointerObjectCast<typename QtSharedPointer::RemovePointer<X>::Type, T>(src);
}

template <class X, class T>
inline QSharedPointer<typename QtSharedPointer::RemovePointer<X>::Type>
qobject_cast(const QWeakPointer<T> &src)
{
   return qSharedPointerObjectCast<typename QtSharedPointer::RemovePointer<X>::Type, T>(src);
}

template<typename T>
QWeakPointer<typename std::enable_if<std::is_base_of<QObject, T>::value, T>::type>
qWeakPointerFromVariant(const QVariant &variant)
{
   return QWeakPointer<T>(dynamic_cast<T *>(QtSharedPointer::weakPointerFromVariant_internal(variant).data()));
}

template<typename T>
QSharedPointer<typename std::enable_if<std::is_base_of<QObject, T>::value, T>::Type>
qSharedPointerFromVariant(const QVariant &variant)
{
   return qSharedPointerObjectCast<T>(QtSharedPointer::sharedPointerFromVariant_internal(variant));
}

template<typename T> Q_DECLARE_TYPEINFO_BODY(QWeakPointer<T>, Q_MOVABLE_TYPE);
template<typename T> Q_DECLARE_TYPEINFO_BODY(QSharedPointer<T>, Q_MOVABLE_TYPE);
#endif

