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

#ifndef CSOBJECT_MACRO_H
#define CSOBJECT_MACRO_H

#include <cs_signal.h>
#include <cs_slot.h>
#include <qglobal.h>

#define csPrintable(string)         QString(string).toLatin1().constData()
#define csPrintable8(string)        QString8(string).constData()
#define csPrintable16(string)       QString16(string).toUtf8().constData()

#define Q_EMIT
#define Q_ARG(type, data)           CSArgument<type>{data, #type}
#define Q_RETURN_ARG(type, data)    CSReturnArgument<type>(data)

#define CS_TOKENPASTE1(x,y)         x ## y
#define CS_TOKENPASTE2(x,y)         CS_TOKENPASTE1(x,y)

/**   \cond INTERNAL (notation so DoxyPress will not parse this class  */

template<int N>
class cs_number : public cs_number<N - 1>
{
   public:
      static constexpr int value = N;
};

template<>
class cs_number<0>
{
   public:
      static constexpr int value = 0;
};

/**   \endcond   */


#ifdef QT_NO_TEXTCODEC
# define CS_TR_FUNCTIONS \
   static inline QString tr(const char *s, const char *c = 0, int n = -1) \
      { return staticMetaObject().tr(s, c, n); }
#else
# define CS_TR_FUNCTIONS \
   static inline QString tr(const char *s, const char *c = 0, int n = -1) \
      { return staticMetaObject().tr(s, c, n); } \
   static inline QString trUtf8(const char *s, const char *c = 0, int n = -1) \
      { return staticMetaObject().trUtf8(s, c, n); }
#endif


// ** cs_object
#define CS_OBJECT(classNameX) \
   public: \
      typedef cs_class cs_parent; \
      typedef classNameX cs_class; \
      CS_OBJECT_INTERNAL(classNameX) \
   private:

#define CS_OBJECT_MULTIPLE(classNameX,parentX) \
   public: \
      typedef parentX cs_parent; \
      typedef classNameX cs_class; \
      CS_OBJECT_INTERNAL(classNameX) \
   private:

#define CS_OBJECT_OUTSIDE(classNameX) \
   public: \
      typedef cs_class cs_parent; \
      typedef classNameX cs_class; \
      CS_OBJECT_INTERNAL_OUTSIDE(classNameX) \
   private:

#define CS_OBJECT_MULTIPLE_OUTSIDE(classNameX,parentX) \
   public: \
      typedef parentX cs_parent; \
      typedef classNameX cs_class; \
      CS_OBJECT_INTERNAL_OUTSIDE(classNameX) \
   private:

#define CS_OVERRIDE override

#define CS_OBJECT_INTERNAL(classNameX) \
   public: \
      static const char *cs_className() \
      { \
         return #classNameX; \
      } \
      template<int N> \
      static void cs_regTrigger(cs_number<N>) \
      { \
      } \
      static constexpr cs_number<0> cs_counter(cs_number<0>)   \
      { \
         return cs_number<0>{};   \
      } \
      friend QMetaObject_T<classNameX>; \
      Q_DECL_EXPORT static const QMetaObject_T<classNameX> & staticMetaObject()   \
      { \
         static std::atomic<bool> isCreated(false);                               \
         static std::atomic<QMetaObject_T<classNameX> *> createdObj(nullptr);     \
         if (isCreated) {         \
            return *createdObj;   \
         } \
         std::lock_guard<std::recursive_mutex> lock(m_metaObjectMutex());   \
         if (createdObj != nullptr) { \
            return *createdObj;       \
         } \
         QMap<std::type_index, QMetaObject *> &temp = m_metaObjectsAll(); \
         auto index = temp.find(typeid(cs_class));    \
         QMetaObject_T<classNameX> *newMeta;          \
         if (index == temp.end()) {     \
            newMeta = new QMetaObject_T<classNameX>;  \
            temp.insert(typeid(cs_class), newMeta);   \
            createdObj.store(newMeta);  \
            newMeta->postConstruct();   \
            isCreated = true;    \
            return *newMeta;     \
         } else {  \
            newMeta = dynamic_cast<QMetaObject_T<classNameX> *> (index.value()); \
            createdObj.store(newMeta);  \
            isCreated = true;    \
            return *newMeta;     \
         } \
      } \
      virtual Q_DECL_EXPORT const QMetaObject *metaObject() const CS_OVERRIDE \
      { \
         return &staticMetaObject(); \
      } \
      CS_TR_FUNCTIONS \
   private: \
      struct cs_classname \
      { \
         static constexpr const char *value = #classNameX; \
      };


#define CS_OBJECT_INTERNAL_OUTSIDE(classNameX) \
   public: \
      static const char *cs_className() \
      { \
         return #classNameX; \
      } \
      static const QMetaObject_T<classNameX> & staticMetaObject(); \
      virtual const QMetaObject *metaObject() const CS_OVERRIDE; \
      CS_TR_FUNCTIONS \
   private: \
      struct cs_classname \
      { \
         static constexpr const char *value = #classNameX; \
      };


// ** cs_gadget
#define CS_GADGET(classNameX) \
   public: \
      typedef CSGadget_Fake_Parent cs_parent; \
      typedef classNameX cs_class; \
      CS_GADGET_INTERNAL(classNameX) \
   private:

#define CS_GADGET_OUTSIDE(classNameX) \
   public: \
      typedef CSGadget_Fake_Parent cs_parent; \
      typedef classNameX cs_class; \
      CS_GADGET_INTERNAL_OUTSIDE(classNameX) \
   private:


#define CS_GADGET_INTERNAL(classNameX) \
   public: \
      static const char *cs_className() \
      { \
         return #classNameX; \
      }; \
      template<int N> \
      static void cs_regTrigger(cs_number<N>) \
      { \
      } \
      static constexpr cs_number<0> cs_counter(cs_number<0>)   \
      { \
         return cs_number<0>{};  \
      } \
      friend QMetaObject_T<classNameX>; \
      static const QMetaObject_T<classNameX> & staticMetaObject() \
      { \
         QMap<std::type_index, QMetaObject *> &temp = CSGadget_Fake_Parent::m_metaObjectsAll(); \
         auto index = temp.find(typeid(cs_class));      \
         if (index == temp.end()) {       \
            QMetaObject_T<classNameX> *xx = new QMetaObject_T<classNameX>;  \
            temp.insert(typeid(cs_class), xx);  \
            xx->postConstruct(); \
            return *xx; \
         } else {      \
            return *dynamic_cast<QMetaObject_T<classNameX> *> (index.value()); \
         } \
      } \
      CS_TR_FUNCTIONS \
   private: \
      struct cs_classname \
      { \
         static constexpr const char *value = #classNameX; \
      };

#define CS_GADGET_INTERNAL_OUTSIDE(classNameX) \
   public: \
      static const char *cs_className() \
      { \
         return #classNameX; \
      } \
      static const QMetaObject_T<classNameX> & staticMetaObject(); \
      CS_TR_FUNCTIONS \
   private: \
      struct cs_classname \
      { \
         static constexpr const char *value = #classNameX; \
      };


// ** interface
#define CS_DECLARE_INTERFACE(IFace, IId) \
   template<> \
   inline const char *qobject_interface_iid<IFace *>() \
      { return IId; } \
   template<> \
   inline IFace * qobject_cast<IFace *>(QObject *object) \
      { \
         return dynamic_cast<IFace *>(object); \
      } \
   template<> \
   inline const IFace * qobject_cast<const IFace *>(const QObject *object) \
      {  \
         return dynamic_cast<const IFace *>(object); \
      }


#define CS_INTERFACES(...)    \
   public: \
      bool cs_interface_query(const QString &data) const override \
      {  \
         if (cs_factory_interface_query<__VA_ARGS__>(data)) { \
            return true;   \
         }  \
         return false;  \
      }  \
   private:  \


// ** classInfo
#define CS_CLASSINFO(name, data) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_classInfo(name, data); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


// ** enum
#define CS_ENUM(name) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         cs_namespace_register_enum<cs_class>(#name, typeid(name), cs_classname::value);  \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_REGISTER_ENUM(...) \
   __VA_ARGS__ \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_enum_data(#__VA_ARGS__, cs_classname::value);  \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


// ** flag
#define CS_FLAG(enumName, flagName) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         cs_namespace_register_flag<cs_class>(#enumName, cs_classname::value,    \
            #flagName, typeid(flagName));  \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


// ** invoke constructor
#define CS_INVOKABLE_CONSTRUCTOR_1(access, ...) \
   __VA_ARGS__; \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>  \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>) \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};      \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
      {  \
         const char *va_args = #__VA_ARGS__;    \
         QMetaMethod::Access accessType = QMetaMethod::access; \
         constexpr int cntValue = CS_TOKENPASTE2(cs_counter_value, __LINE__);
// do not remove the ";", this is required for part two of the macro


#define CS_INVOKABLE_CONSTRUCTOR_2(className, ...) \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method    \
            <QObject * (*)(__VA_ARGS__)>(   \
            #className, &cs_class::CS_TOKENPASTE2(cs_fauxConstructor, __LINE__)<__VA_ARGS__>,  \
            QMetaMethod::Constructor, va_args, accessType); \
         \
         cs_regTrigger(cs_number<cntValue + 1>{} );  \
      } \
      template <class... Ts> \
   static QObject * CS_TOKENPASTE2(cs_fauxConstructor, __LINE__)(Ts...Vs) \
      { \
         return new className{Vs...};  \
      }


// ** invoke
#define CS_INVOKABLE_METHOD_1(access, ...) \
   __VA_ARGS__; \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>  \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>) \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};      \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
      {  \
         const char *va_args = #__VA_ARGS__;    \
         QMetaMethod::Access accessType = QMetaMethod::access; \
         constexpr int cntValue = CS_TOKENPASTE2(cs_counter_value, __LINE__);
// do not remove the ";", this is required for part two of the macro


#define CS_INVOKABLE_METHOD_2(methodName) \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method(  \
            #methodName, &cs_class::methodName, QMetaMethod::Method, va_args, accessType); \
         \
         cs_regTrigger(cs_number<cntValue + 1>{} );  \
      }


#define CS_INVOKABLE_OVERLOAD(methodName, argTypes, ...) \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method(  \
            #methodName, static_cast<void (cs_class::*)argTypes>(&cs_class::methodName), \
            QMetaMethod::Method, va_args, accessType); \
         \
         cs_regTrigger(cs_number<cntValue + 1>{} );  \
      }


// ** revision
#define CS_REVISION(methodName,revision) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method_rev( \
            &cs_class::methodName, revision); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_REVISION_OVERLOAD(methodName, revision, argTypes) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method_rev(  \
            static_cast<void (cs_class::*)argTypes>(&cs_class::methodName), revision); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


// ** slots
#define CS_SLOT_1(access, ...) \
   __VA_ARGS__; \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype(cs_counter(cs_number<255>{}))::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>  \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>) \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{};      \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
      {  \
         const char *va_args = #__VA_ARGS__;    \
         QMetaMethod::Access accessType = QMetaMethod::access; \
         constexpr int cntValue = CS_TOKENPASTE2(cs_counter_value, __LINE__);
// do not remove the ";", this is required for part two of the macro


#define CS_SLOT_2(slotName) \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method(  \
            #slotName, &cs_class::slotName, QMetaMethod::Slot, va_args, accessType); \
         \
         cs_regTrigger(cs_number<cntValue + 1>{} ); \
      }


// ** signals
#define CS_SIGNAL_1(access, ...) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype(cs_counter(cs_number<255>{}))::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>  \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)   \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{};        \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method_s1(  \
            #__VA_ARGS__, QMetaMethod::access, QMetaMethod::Signal);  \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }  \
   Q_DECL_EXPORT __VA_ARGS__ {   \
      if (this->signalsBlocked()) {          \
         return;                 \
      }
// do not remove the "{", this is required for part two of the macro


#define CS_SIGNAL_2(signalName, ...) \
      CsSignal::activate(*this, &cs_class::signalName, ##__VA_ARGS__); \
   }  \
   static constexpr int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype(cs_counter(cs_number<255>{}))::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>     \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>) \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};      \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method_s2(  \
            #signalName, &cs_class::signalName, QMetaMethod::Signal);   \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }  \


#define CS_SIGNAL_OVERLOAD(signalName, argTypes, ...) \
      CsSignal::activate(*this, static_cast<void (cs_class::*)argTypes>(&cs_class::signalName), ##__VA_ARGS__); \
   }  \
   static constexpr int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype(cs_counter(cs_number<255>{}))::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>       \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)   \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{};        \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method_s2(  \
            #signalName #argTypes, static_cast<void (cs_class::*)argTypes>(&cs_class::signalName), QMetaMethod::Signal);  \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }  \


#define CS_SLOT_OVERLOAD(slotName, argTypes) \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method(    \
            #slotName #argTypes, static_cast<void (cs_class::*)argTypes>(&cs_class::slotName),  \
            QMetaMethod::Slot, va_args, accessType); \
         \
         cs_regTrigger(cs_number<cntValue + 1>{} );  \
      }


#define CS_SLOT_OVERLOAD_BOOL(slotName, argTypes) \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method(     \
            #slotName #argTypes, static_cast<bool (cs_class::*)argTypes>(&cs_class::slotName),   \
            QMetaMethod::Slot, va_args, accessType);  \
         \
         cs_regTrigger(cs_number<cntValue + 1>{} );   \
      }

#ifndef QT_NO_DEBUG


#define CS_NUMBER_TO_STRING_INTERNAL(number)       #number
#define CS_NUMBER_TO_STRING(number)                CS_NUMBER_TO_STRING_INTERNAL(number)

// SIGNAL
#define SIGNAL(...) \
   #__VA_ARGS__ , __FILE__ ":" CS_NUMBER_TO_STRING(__LINE__)

// SLOT
#define SLOT(...) \
   #__VA_ARGS__

#else

// SIGNAL
#define SIGNAL(...) \
   #__VA_ARGS__

// SLOT
#define SLOT(...) \
   #__VA_ARGS__

#endif


// ** metaMetod tag
#define CS_TAG(name, data) \
  static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_tag(#name, #data); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }



// ** properties
#define CS_PROPERTY_READ(name, method) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())   \
            .register_property_read(#name, cs_typeName< cs_returnType<decltype(&cs_class::method)>::type> (),  \
            new SpiceJarRead<cs_class, cs_returnType< decltype(&cs_class::method) >::type> (&cs_class::method));  \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_PROPERTY_WRITE(name, method) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
          const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()) \
            .register_property_write(#name, new SpiceJarWrite<cs_class, \
            cs_argType< decltype(&cs_class::method) >::type> (&cs_class::method));  \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_PROPERTY_NOTIFY(name, method) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_property_notify(#name, &cs_class::method); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_PROPERTY_RESET(name, method) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_property_reset(#name, &cs_class::method); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_PROPERTY_REVISION(name, data) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_property_int(#name, data, QMetaProperty::REVISION); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_PROPERTY_DESIGNABLE(name, data) \
   static bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()  \
      {  \
         return data; \
      } \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())   \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>   \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::DESIGNABLE); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }



#define CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data) \
   bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)() const \
      {  \
         return data; \
      } \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())   \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>   \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::DESIGNABLE); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_PROPERTY_SCRIPTABLE(name, data) \
   static bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()  \
      {  \
         return data; \
      } \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())   \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>   \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::SCRIPTABLE); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data) \
   bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)() const \
      {  \
         return data; \
      } \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())   \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>   \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::SCRIPTABLE); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_PROPERTY_STORED(name, data) \
   static bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()  \
      {  \
         return data; \
      } \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())   \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>   \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::STORED); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }



#define CS_PROPERTY_STORED_NONSTATIC(name, data) \
   bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)() const \
      {  \
         return data; \
      } \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())   \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>   \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::STORED); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }



#define CS_PROPERTY_USER(name, data) \
   static bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()  \
      {  \
         return data; \
      } \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())   \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>   \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::USER); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_PROPERTY_USER_NONSTATIC(name, data) \
   bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()  const \
      {  \
         return data; \
      } \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())   \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>   \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::USER); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_PROPERTY_CONSTANT(name) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_property_int(  \
            #name, 1, QMetaProperty::CONSTANT); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


#define CS_PROPERTY_FINAL(name) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =  \
            decltype( cs_counter(cs_number<255>{}) )::value; \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
            cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>)    \
      {  \
         return cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) +1 >{};         \
      }  \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
      {  \
         const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_property_int(  \
            #name, 1, QMetaProperty::FINAL); \
         \
         cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} ); \
      }


// ** macros used when compiling

// ** 1
#if defined(QT_BUILD_CORE_LIB) || ! defined(Q_OS_WIN)

#define CORE_CS_OBJECT(className)                              CS_OBJECT(className)
#define CORE_CS_OBJECT_MULTIPLE(className, parentX)            CS_OBJECT_MULTIPLE(className, parentX)
#define CORE_CS_OBJECT_INTERNAL(className)                     CS_OBJECT_INTERNAL(className)
#define CORE_CS_GADGET(className)                              CS_GADGET(className)

#define CORE_CS_SLOT_1(access, ...)                            CS_SLOT_1(access, __VA_ARGS__)
#define CORE_CS_SLOT_2(slotName)                               CS_SLOT_2(slotName)
#define CORE_CS_SLOT_OVERLOAD(slotName, argTypes)              CS_SLOT_OVERLOAD(slotName, argTypes)

#define CORE_CS_SIGNAL_1(access, ...)                          CS_SIGNAL_1(access, __VA_ARGS__)
#define CORE_CS_SIGNAL_2(signalName, ...)                      CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define CORE_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)     CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#define CORE_CS_INVOKABLE_CONSTRUCTOR_1(access, ...)           CS_INVOKABLE_CONSTRUCTOR_1(access, __VA_ARGS__)
#define CORE_CS_INVOKABLE_CONSTRUCTOR_2(className, ...)        CS_INVOKABLE_CONSTRUCTOR_2(className, __VA_ARGS__)

#define CORE_CS_ENUM(name)                                     CS_ENUM(name)
#define CORE_CS_REGISTER_ENUM(...)                             CS_REGISTER_ENUM(__VA_ARGS__)
#define CORE_CS_FLAG(enumName, flagName)                       CS_FLAG(enumName, flagName)

#define CORE_CS_PROPERTY_READ(name, method)                    CS_PROPERTY_READ(name, method)
#define CORE_CS_PROPERTY_WRITE(name, method)                   CS_PROPERTY_WRITE(name, method)
#define CORE_CS_PROPERTY_NOTIFY(name, method)                  CS_PROPERTY_NOTIFY(name, method)
#define CORE_CS_PROPERTY_RESET(name, method)                   CS_PROPERTY_RESET(name, method)
#define CORE_CS_PROPERTY_REVISION(name, data)                  CS_PROPERTY_REVISION(name, data)
#define CORE_CS_PROPERTY_DESIGNABLE(name, data)                CS_PROPERTY_DESIGNABLE(name, data)
#define CORE_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)      CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define CORE_CS_PROPERTY_SCRIPTABLE(name, data)                CS_PROPERTY_SCRIPTABLE(name, data)
#define CORE_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)      CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define CORE_CS_PROPERTY_STORED(name, data)                    CS_PROPERTY_STORED(name, data)
#define CORE_CS_PROPERTY_STORED_NONSTATIC(name, data)          CS_PROPERTY_STORED_NONSTATIC(name, data)
#define CORE_CS_PROPERTY_USER(name, data)                      CS_PROPERTY_USER(name, data)
#define CORE_CS_PROPERTY_USER_NONSTATIC(name, data)            CS_PROPERTY_USER_NONSTATIC(name, data)
#define CORE_CS_PROPERTY_CONSTANT(name)                        CS_PROPERTY_CONSTANT(name)
#define CORE_CS_PROPERTY_FINAL(name)                           CS_PROPERTY_FINAL(name)

#else
#define CORE_CS_OBJECT(className)                              CS_OBJECT_OUTSIDE(className)
#define CORE_CS_OBJECT_MULTIPLE(className, parentX)            CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define CORE_CS_OBJECT_INTERNAL(className)                     CS_OBJECT_INTERNAL_OUTSIDE(className)
#define CORE_CS_GADGET(className)                              CS_GADGET_OUTSIDE(className)

#define CORE_CS_SLOT_1(access, ...)                             __VA_ARGS__;
#define CORE_CS_SLOT_2(slotName)
#define CORE_CS_SLOT_OVERLOAD(slotName, argTypes)

#define CORE_CS_SIGNAL_1(access, ...)                          Q_DECL_IMPORT __VA_ARGS__;
#define CORE_CS_SIGNAL_2(signalName, ...)
#define CORE_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define CORE_CS_INVOKABLE_CONSTRUCTOR_1(access, ...)           __VA_ARGS__;
#define CORE_CS_INVOKABLE_CONSTRUCTOR_2(className, ...)

#define CORE_CS_ENUM(name)
#define CORE_CS_REGISTER_ENUM(...)                             __VA_ARGS__           // do not add ;
#define CORE_CS_FLAG(enumName, flagName)

#define CORE_CS_PROPERTY_READ(name, method)
#define CORE_CS_PROPERTY_WRITE(name, method)
#define CORE_CS_PROPERTY_NOTIFY(name, method)
#define CORE_CS_PROPERTY_RESET(name, method)
#define CORE_CS_PROPERTY_REVISION(name, data)
#define CORE_CS_PROPERTY_DESIGNABLE(name, data)
#define CORE_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define CORE_CS_PROPERTY_SCRIPTABLE(name, data)
#define CORE_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define CORE_CS_PROPERTY_STORED(name, data)
#define CORE_CS_PROPERTY_STORED_NONSTATIC(name, data)
#define CORE_CS_PROPERTY_USER(name, data)
#define CORE_CS_PROPERTY_USER_NONSTATIC(name, data)
#define CORE_CS_PROPERTY_CONSTANT(name)
#define CORE_CS_PROPERTY_FINAL(name)

#endif


// ** 2
#if defined(QT_BUILD_GUI_LIB) || ! defined(Q_OS_WIN)

#define GUI_CS_OBJECT(className)                                  CS_OBJECT(className)
#define GUI_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE(className, parentX)
#define GUI_CS_GADGET(className)                                  CS_GADGET(className)
#define GUI_CS_CLASSINFO(name, data)                              CS_CLASSINFO(name, data)

#define GUI_CS_SLOT_1(access, ...)                                CS_SLOT_1(access, __VA_ARGS__)
#define GUI_CS_SLOT_2(slotName)                                   CS_SLOT_2(slotName)
#define GUI_CS_SLOT_OVERLOAD(slotName, argTypes)                  CS_SLOT_OVERLOAD(slotName, argTypes)

#define GUI_CS_SIGNAL_1(access, ...)                              CS_SIGNAL_1(access, __VA_ARGS__)
#define GUI_CS_SIGNAL_2(signalName, ...)                          CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define GUI_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)         CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#define GUI_CS_ENUM(name)                                         CS_ENUM(name)
#define GUI_CS_REGISTER_ENUM(...)                                 CS_REGISTER_ENUM(__VA_ARGS__)
#define GUI_CS_FLAG(enumName, flagName)                           CS_FLAG(enumName, flagName)

#define GUI_CS_PROPERTY_READ(name, method)                        CS_PROPERTY_READ(name, method)
#define GUI_CS_PROPERTY_WRITE(name, method)                       CS_PROPERTY_WRITE(name, method)
#define GUI_CS_PROPERTY_NOTIFY(name, method)                      CS_PROPERTY_NOTIFY(name, method)
#define GUI_CS_PROPERTY_RESET(name, method)                       CS_PROPERTY_RESET(name, method)
#define GUI_CS_PROPERTY_REVISION(name, data)                      CS_PROPERTY_REVISION(name, data)
#define GUI_CS_PROPERTY_DESIGNABLE(name, data)                    CS_PROPERTY_DESIGNABLE(name, data)
#define GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)          CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define GUI_CS_PROPERTY_SCRIPTABLE(name, data)                    CS_PROPERTY_SCRIPTABLE(name, data)
#define GUI_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)          CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define GUI_CS_PROPERTY_STORED(name, data)                        CS_PROPERTY_STORED(name, data)
#define GUI_CS_PROPERTY_STORED_NONSTATIC(name, data)              CS_PROPERTY_STORED_NONSTATIC(name, data)
#define GUI_CS_PROPERTY_USER(name, data)                          CS_PROPERTY_USER(name, data)
#define GUI_CS_PROPERTY_USER_NONSTATIC(name, data)                CS_PROPERTY_USER_NONSTATIC(name, data)
#define GUI_CS_PROPERTY_CONSTANT(name)                            CS_PROPERTY_CONSTANT(name)
#define GUI_CS_PROPERTY_FINAL(name)                               CS_PROPERTY_FINAL(name)

#else
#define GUI_CS_OBJECT(className)                                  CS_OBJECT_OUTSIDE(className)
#define GUI_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define GUI_CS_GADGET(className)                                  CS_GADGET_OUTSIDE(className)
#define GUI_CS_CLASSINFO(name, data)

#define GUI_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define GUI_CS_SLOT_2(slotName)
#define GUI_CS_SLOT_OVERLOAD(slotName, argTypes)

#define GUI_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define GUI_CS_SIGNAL_2(signalName, ...)
#define GUI_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define GUI_CS_ENUM(name)
#define GUI_CS_REGISTER_ENUM(...)                                 __VA_ARGS__           // do not add ;
#define GUI_CS_FLAG(enumName, flagName)

#define GUI_CS_PROPERTY_READ(name, method)
#define GUI_CS_PROPERTY_WRITE(name, method)
#define GUI_CS_PROPERTY_NOTIFY(name, method)
#define GUI_CS_PROPERTY_RESET(name, method)
#define GUI_CS_PROPERTY_REVISION(name, data)
#define GUI_CS_PROPERTY_DESIGNABLE(name, data)
#define GUI_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define GUI_CS_PROPERTY_SCRIPTABLE(name, data)
#define GUI_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define GUI_CS_PROPERTY_STORED(name, data)
#define GUI_CS_PROPERTY_STORED_NONSTATIC(name, data)
#define GUI_CS_PROPERTY_USER(name, data)
#define GUI_CS_PROPERTY_USER_NONSTATIC(name, data)
#define GUI_CS_PROPERTY_CONSTANT(name)
#define GUI_CS_PROPERTY_FINAL(name)

#endif

// ** 3
#if defined(QT_BUILD_MULTIMEDIA_LIB) || ! defined(Q_OS_WIN)

#define MULTI_CS_OBJECT(className)                                CS_OBJECT(className)
#define MULTI_CS_OBJECT_MULTIPLE(className, parentX)              CS_OBJECT_MULTIPLE(className, parentX)
#define MULTI_CS_GADGET(className)                                CS_GADGET(className)

#define MULTI_CS_SLOT_1(access, ...)                              CS_SLOT_1(access, __VA_ARGS__)
#define MULTI_CS_SLOT_2(slotName)                                 CS_SLOT_2(slotName)
#define MULTI_CS_SLOT_OVERLOAD(slotName, argTypes)                CS_SLOT_OVERLOAD(slotName, argTypes)

#define MULTI_CS_SIGNAL_1(access, ...)                            CS_SIGNAL_1(access, __VA_ARGS__)
#define MULTI_CS_SIGNAL_2(signalName, ...)                        CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define MULTI_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)       CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define MULTI_CS_OBJECT(className)                                CS_OBJECT_OUTSIDE(className)
#define MULTI_CS_OBJECT_MULTIPLE(className, parentX)              CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define MULTI_CS_GADGET(className)                                CS_GADGET_OUTSIDE(className)

#define MULTI_CS_SLOT_1(access, ...)                              __VA_ARGS__;
#define MULTI_CS_SLOT_2(slotName)
#define MULTI_CS_SLOT_OVERLOAD(slotName, argTypes)

#define MULTI_CS_SIGNAL_1(access, ...)                             Q_DECL_IMPORT __VA_ARGS__;
#define MULTI_CS_SIGNAL_2(signalName, ...)
#define MULTI_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif


// ** 4
#if defined(QT_BUILD_NETWORK_LIB) || ! defined(Q_OS_WIN)

#define NET_CS_OBJECT(className)                                  CS_OBJECT(className)
#define NET_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE(className, parentX)
#define NET_CS_GADGET(className)                                  CS_GADGET(className)

#define NET_CS_SLOT_1(access, ...)                                CS_SLOT_1(access, __VA_ARGS__)
#define NET_CS_SLOT_2(slotName)                                   CS_SLOT_2(slotName)
#define NET_CS_SLOT_OVERLOAD(slotName, argTypes)                  CS_SLOT_OVERLOAD(slotName, argTypes)

#define NET_CS_SIGNAL_1(access, ...)                              CS_SIGNAL_1(access, __VA_ARGS__)
#define NET_CS_SIGNAL_2(signalName, ...)                          CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define NET_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)         CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#define NET_CS_INVOKABLE_METHOD_1(access, ...)                    CS_INVOKABLE_METHOD_1(access, __VA_ARGS__)
#define NET_CS_INVOKABLE_METHOD_2(methodName)                     CS_INVOKABLE_METHOD_2(methodName)

#define NET_CS_ENUM(name)                                         CS_ENUM(name)
#define NET_CS_REGISTER_ENUM(...)                                 CS_REGISTER_ENUM(__VA_ARGS__)

#define NET_CS_PROPERTY_READ(name, method)                        CS_PROPERTY_READ(name, method)
#define NET_CS_PROPERTY_WRITE(name, method)                       CS_PROPERTY_WRITE(name, method)
#define NET_CS_PROPERTY_NOTIFY(name, method)                      CS_PROPERTY_NOTIFY(name, method)
#define NET_CS_PROPERTY_RESET(name, method)                       CS_PROPERTY_RESET(name, method)
#define NET_CS_PROPERTY_REVISION(name, data)                      CS_PROPERTY_REVISION(name, data)
#define NET_CS_PROPERTY_DESIGNABLE(name, data)                    CS_PROPERTY_DESIGNABLE(name, data)
#define NET_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)          CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define NET_CS_PROPERTY_SCRIPTABLE(name, data)                    CS_PROPERTY_SCRIPTABLE(name, data)
#define NET_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)          CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define NET_CS_PROPERTY_STORED(name, data)                        CS_PROPERTY_STORED(name, data)
#define NET_CS_PROPERTY_STORED_NONSTATIC(name, data)              CS_PROPERTY_STORED_NONSTATIC(name, data)
#define NET_CS_PROPERTY_USER(name, data)                          CS_PROPERTY_USER(name, data)
#define NET_CS_PROPERTY_USER_NONSTATIC(name, data)                CS_PROPERTY_USER_NONSTATIC(name, data)
#define NET_CS_PROPERTY_CONSTANT(name)                            CS_PROPERTY_CONSTANT(name)
#define NET_CS_PROPERTY_FINAL(name)                               CS_PROPERTY_FINAL(name)

#else
#define NET_CS_OBJECT(className)                                  CS_OBJECT_OUTSIDE(className)
#define NET_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define NET_CS_GADGET(className)                                  CS_GADGET_OUTSIDE(className)

#define NET_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define NET_CS_SLOT_2(slotName)
#define NET_CS_SLOT_OVERLOAD(slotName, argTypes)

#define NET_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define NET_CS_SIGNAL_2(signalName, ...)
#define NET_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define NET_CS_INVOKABLE_METHOD_1(access, ...)                    __VA_ARGS__;
#define NET_CS_INVOKABLE_METHOD_2(methodName)

#define NET_CS_ENUM(name)
#define NET_CS_REGISTER_ENUM(...)                                 __VA_ARGS__           // do not add ;

#define NET_CS_PROPERTY_READ(name, method)
#define NET_CS_PROPERTY_WRITE(name, method)
#define NET_CS_PROPERTY_NOTIFY(name, method)
#define NET_CS_PROPERTY_RESET(name, method)
#define NET_CS_PROPERTY_REVISION(name, data)
#define NET_CS_PROPERTY_DESIGNABLE(name, data)
#define NET_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define NET_CS_PROPERTY_SCRIPTABLE(name, data)
#define NET_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define NET_CS_PROPERTY_STORED(name, data)
#define NET_CS_PROPERTY_STORED_NONSTATIC(name, data)
#define NET_CS_PROPERTY_USER(name, data)
#define NET_CS_PROPERTY_USER_NONSTATIC(name, data)
#define NET_CS_PROPERTY_CONSTANT(name)
#define NET_CS_PROPERTY_FINAL(name)

#endif


// ** 5
#if defined(QT_BUILD_OPENGL_LIB) || ! defined(Q_OS_WIN)

#define OPENGL_CS_OBJECT(className)                               CS_OBJECT(className)
#define OPENGL_CS_OBJECT_MULTIPLE(className, parentX)             CS_OBJECT_MULTIPLE(className, parentX)
#define OPENGL_CS_GADGET(className)                               CS_GADGET(className)

#define OPENGL_CS_SLOT_1(access, ...)                             CS_SLOT_1(access, __VA_ARGS__)
#define OPENGL_CS_SLOT_2(slotName)                                CS_SLOT_2(slotName)
#define OPENGL_CS_SLOT_OVERLOAD(slotName, argTypes)               CS_SLOT_OVERLOAD(slotName, argTypes)

#define OPENGL_CS_SIGNAL_1(access, ...)                           CS_SIGNAL_1(access, __VA_ARGS__)
#define OPENGL_CS_SIGNAL_2(signalName, ...)                       CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define OPENGL_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)      CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define OPENGL_CS_OBJECT(className)                               CS_OBJECT_OUTSIDE(className)
#define OPENGL_CS_OBJECT_MULTIPLE(className, parentX)             CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define OPENGL_CS_GADGET(className)                               CS_GADGET_OUTSIDE(className)

#define OPENGL_CS_SLOT_1(access, ...)                             __VA_ARGS__;
#define OPENGL_CS_SLOT_2(slotName)
#define OPENGL_CS_SLOT_OVERLOAD(slotName, argTypes)

#define OPENGL_CS_SIGNAL_1(access, ...)                           Q_DECL_IMPORT __VA_ARGS__;
#define OPENGL_CS_SIGNAL_2(signalName, ...)
#define OPENGL_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif


// ** 6
#if defined(QT_BUILD_PHONON_LIB) || ! defined(Q_OS_WIN)

#define PHN_CS_OBJECT(className)                                  CS_OBJECT(className)
#define PHN_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE(className, parentX)
#define PHN_CS_GADGET(className)                                  CS_GADGET(className)
#define PHN_CS_CLASSINFO(name, data)                              CS_CLASSINFO(name, data)

#define PHN_CS_SLOT_1(access, ...)                                CS_SLOT_1(access, __VA_ARGS__)
#define PHN_CS_SLOT_2(slotName)                                   CS_SLOT_2(slotName)
#define PHN_CS_SLOT_OVERLOAD(slotName, argTypes)                  CS_SLOT_OVERLOAD(slotName, argTypes)

#define PHN_CS_SIGNAL_1(access, ...)                              CS_SIGNAL_1(access, __VA_ARGS__)
#define PHN_CS_SIGNAL_2(signalName, ...)                          CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define PHN_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)         CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#define PHN_CS_ENUM(name)                                         CS_ENUM(name)
#define PHN_CS_REGISTER_ENUM(...)                                 CS_REGISTER_ENUM(__VA_ARGS__)
#define PHN_CS_FLAG(enumName, flagName)                           CS_FLAG(enumName, flagName)

#define PHN_CS_PROPERTY_READ(name, method)                        CS_PROPERTY_READ(name, method)
#define PHN_CS_PROPERTY_WRITE(name, method)                       CS_PROPERTY_WRITE(name, method)
#define PHN_CS_PROPERTY_NOTIFY(name, method)                      CS_PROPERTY_NOTIFY(name, method)
#define PHN_CS_PROPERTY_RESET(name, method)                       CS_PROPERTY_RESET(name, method)
#define PHN_CS_PROPERTY_REVISION(name, data)                      CS_PROPERTY_REVISION(name, data)
#define PHN_CS_PROPERTY_DESIGNABLE(name, data)                    CS_PROPERTY_DESIGNABLE(name, data)
#define PHN_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)          CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define PHN_CS_PROPERTY_SCRIPTABLE(name, data)                    CS_PROPERTY_SCRIPTABLE(name, data)
#define PHN_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)          CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define PHN_CS_PROPERTY_STORED(name, data)                        CS_PROPERTY_STORED(name, data)
#define PHN_CS_PROPERTY_STORED_NONSTATIC(name, data)              CS_PROPERTY_STORED_NONSTATIC(name, data)
#define PHN_CS_PROPERTY_USER(name, data)                          CS_PROPERTY_USER(name, data)
#define PHN_CS_PROPERTY_USER_NONSTATIC(name, data)                CS_PROPERTY_USER_NONSTATIC(name, data)
#define PHN_CS_PROPERTY_CONSTANT(name)                            CS_PROPERTY_CONSTANT(name)
#define PHN_CS_PROPERTY_FINAL(name)                               CS_PROPERTY_FINAL(name)

#else
#define PHN_CS_OBJECT(className)                                  CS_OBJECT_OUTSIDE(className)
#define PHN_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define PHN_CS_GADGET(className)                                  CS_GADGET_OUTSIDE(className)
#define PHN_CS_CLASSINFO(name, data)

#define PHN_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define PHN_CS_SLOT_2(slotName)
#define PHN_CS_SLOT_OVERLOAD(slotName, argTypes)

#define PHN_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define PHN_CS_SIGNAL_2(signalName, ...)
#define PHN_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define PHN_CS_ENUM(name)
#define PHN_CS_REGISTER_ENUM(...)                                 __VA_ARGS__           // do not add ;
#define PHN_CS_FLAG(enumName, flagName)

#define PHN_CS_PROPERTY_READ(name, method)
#define PHN_CS_PROPERTY_WRITE(name, method)
#define PHN_CS_PROPERTY_NOTIFY(name, method)
#define PHN_CS_PROPERTY_RESET(name, method)
#define PHN_CS_PROPERTY_REVISION(name, data)
#define PHN_CS_PROPERTY_DESIGNABLE(name, data)
#define PHN_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define PHN_CS_PROPERTY_SCRIPTABLE(name, data)
#define PHN_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define PHN_CS_PROPERTY_STORED(name, data)
#define PHN_CS_PROPERTY_STORED_NONSTATIC(name, data)
#define PHN_CS_PROPERTY_USER(name, data)
#define PHN_CS_PROPERTY_USER_NONSTATIC(name, data)
#define PHN_CS_PROPERTY_CONSTANT(name)
#define PHN_CS_PROPERTY_FINAL(name)

#endif


// ** 7
#if defined(QT_BUILD_SCRIPT_LIB) || ! defined(Q_OS_WIN)

#define SCRIPT_CS_OBJECT(className)                               CS_OBJECT(className)
#define SCRIPT_CS_OBJECT_MULTIPLE(className, parentX)             CS_OBJECT_MULTIPLE(className, parentX)
#define SCRIPT_CS_GADGET(className)                               CS_GADGET(className)

#define SCRIPT_CS_SLOT_1(access, ...)                             CS_SLOT_1(access, __VA_ARGS__)
#define SCRIPT_CS_SLOT_2(slotName)                                CS_SLOT_2(slotName)
#define SCRIPT_CS_SLOT_OVERLOAD(slotName, argTypes)               CS_SLOT_OVERLOAD(slotName, argTypes)

#define SCRIPT_CS_SIGNAL_1(access, ...)                           CS_SIGNAL_1(access, __VA_ARGS__)
#define SCRIPT_CS_SIGNAL_2(signalName, ...)                       CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define SCRIPT_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)      CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define SCRIPT_CS_OBJECT(className)                               CS_OBJECT_OUTSIDE(className)
#define SCRIPT_CS_OBJECT_MULTIPLE(className, parentX)             CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define SCRIPT_CS_GADGET(className)                               CS_GADGET_OUTSIDE(className)

#define SCRIPT_CS_SLOT_1(access, ...)                             __VA_ARGS__;
#define SCRIPT_CS_SLOT_2(slotName)
#define SCRIPT_CS_SLOT_OVERLOAD(slotName, argTypes)

#define SCRIPT_CS_SIGNAL_1(access, ...)                           Q_DECL_IMPORT __VA_ARGS__;
#define SCRIPT_CS_SIGNAL_2(signalName, ...)
#define SCRIPT_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif

// ** 8
#if defined(QT_BUILD_SCRIPTTOOLS_LIB) || ! defined(Q_OS_WIN)

#define SCRIPT_T_CS_OBJECT(className)                             CS_OBJECT(className)
#define SCRIPT_T_CS_OBJECT_MULTIPLE(className, parentX)           CS_OBJECT_MULTIPLE(className, parentX)
#define SCRIPT_T_CS_GADGET(className)                             CS_GADGET(className)

#define SCRIPT_T_CS_SLOT_1(access, ...)                           CS_SLOT_1(access, __VA_ARGS__)
#define SCRIPT_T_CS_SLOT_2(slotName)                              CS_SLOT_2(slotName)
#define SCRIPT_T_CS_SLOT_OVERLOAD(slotName, argTypes)             CS_SLOT_OVERLOAD(slotName, argTypes)

#define SCRIPT_T_CS_SIGNAL_1(access, ...)                         CS_SIGNAL_1(access, __VA_ARGS__)
#define SCRIPT_T_CS_SIGNAL_2(signalName, ...)                     CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define SCRIPT_T_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)    CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define SCRIPT_T_CS_OBJECT(className)                             CS_OBJECT_OUTSIDE(className)
#define SCRIPT_T_CS_OBJECT_MULTIPLE(className, parentX)           CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define SCRIPT_T_CS_GADGET(className)                             CS_GADGET_OUTSIDE(className)

#define SCRIPT_T_CS_SLOT_1(access, ...)                           __VA_ARGS__;
#define SCRIPT_T_CS_SLOT_2(slotName)
#define SCRIPT_T_CS_SLOT_OVERLOAD(slotName, argTypes)

#define SCRIPT_T_CS_SIGNAL_1(access, ...)                          Q_DECL_IMPORT __VA_ARGS__;
#define SCRIPT_T_CS_SIGNAL_2(signalName, ...)
#define SCRIPT_T_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif

// ** 9
#if defined(QT_BUILD_SQL_LIB) || ! defined(Q_OS_WIN)

#define SQL_CS_OBJECT(className)                                  CS_OBJECT(className)
#define SQL_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE(className, parentX)
#define SQL_CS_GADGET(className)                                  CS_GADGET(className)

#define SQL_CS_SLOT_1(access, ...)                                CS_SLOT_1(access, __VA_ARGS__)
#define SQL_CS_SLOT_2(slotName)                                   CS_SLOT_2(slotName)
#define SQL_CS_SLOT_OVERLOAD(slotName, argTypes)                  CS_SLOT_OVERLOAD(slotName, argTypes)

#define SQL_CS_SIGNAL_1(access, ...)                              CS_SIGNAL_1(access, __VA_ARGS__)
#define SQL_CS_SIGNAL_2(signalName, ...)                          CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define SQL_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)         CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define SQL_CS_OBJECT(className)                                  CS_OBJECT_OUTSIDE(className)
#define SQL_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define SQL_CS_GADGET(className)                                  CS_GADGET_OUTSIDE(className)

#define SQL_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define SQL_CS_SLOT_2(slotName)
#define SQL_CS_SLOT_OVERLOAD(slotName, argTypes)

#define SQL_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define SQL_CS_SIGNAL_2(signalName, ...)
#define SQL_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif


// ** 10
#if defined(QT_BUILD_SVG_LIB) || ! defined(Q_OS_WIN)

#define SVG_CS_OBJECT(className)                                  CS_OBJECT(className)
#define SVG_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE(className, parentX)
#define SVG_CS_GADGET(className)                                  CS_GADGET(className)

#define SVG_CS_SLOT_1(access, ...)                                CS_SLOT_1(access, __VA_ARGS__)
#define SVG_CS_SLOT_2(slotName)                                   CS_SLOT_2(slotName)
#define SVG_CS_SLOT_OVERLOAD(slotName, argTypes)                  CS_SLOT_OVERLOAD(slotName, argTypes)
#define SVG_CS_SLOT_OVERLOAD_BOOL(slotName, argTypes)             CS_SLOT_OVERLOAD_BOOL(slotName, argTypes)

#define SVG_CS_SIGNAL_1(access, ...)                              CS_SIGNAL_1(access, __VA_ARGS__)
#define SVG_CS_SIGNAL_2(signalName, ...)                          CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define SVG_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)         CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#define SVG_CS_PROPERTY_READ(name, method)                        CS_PROPERTY_READ(name, method)
#define SVG_CS_PROPERTY_WRITE(name, method)                       CS_PROPERTY_WRITE(name, method)
#define SVG_CS_PROPERTY_NOTIFY(name, method)                      CS_PROPERTY_NOTIFY(name, method)
#define SVG_CS_PROPERTY_RESET(name, method)                       CS_PROPERTY_RESET(name, method)
#define SVG_CS_PROPERTY_REVISION(name, data)                      CS_PROPERTY_REVISION(name, data)
#define SVG_CS_PROPERTY_DESIGNABLE(name, data)                    CS_PROPERTY_DESIGNABLE(name, data)
#define SVG_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)          CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define SVG_CS_PROPERTY_SCRIPTABLE(name, data)                    CS_PROPERTY_SCRIPTABLE(name, data)
#define SVG_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)          CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define SVG_CS_PROPERTY_STORED(name, data)                        CS_PROPERTY_STORED(name, data)
#define SVG_CS_PROPERTY_STORED_NONSTATIC(name, data)              CS_PROPERTY_STORED_NONSTATIC(name, data)
#define SVG_CS_PROPERTY_USER(name, data)                          CS_PROPERTY_USER(name, data)
#define SVG_CS_PROPERTY_USER_NONSTATIC(name, data)                CS_PROPERTY_USER_NONSTATIC(name, data)
#define SVG_CS_PROPERTY_CONSTANT(name)                            CS_PROPERTY_CONSTANT(name)
#define SVG_CS_PROPERTY_FINAL(name)                               CS_PROPERTY_FINAL(name)

#else
#define SVG_CS_OBJECT(className)                                  CS_OBJECT_OUTSIDE(className)
#define SVG_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define SVG_CS_GADGET(className)                                  CS_GADGET_OUTSIDE(className)

#define SVG_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define SVG_CS_SLOT_2(slotName)
#define SVG_CS_SLOT_OVERLOAD(slotName, argTypes)
#define SVG_CS_SLOT_OVERLOAD_BOOL(slotName, argTypes)

#define SVG_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define SVG_CS_SIGNAL_2(signalName, ...)
#define SVG_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define SVG_CS_PROPERTY_READ(name, method)
#define SVG_CS_PROPERTY_WRITE(name, method)
#define SVG_CS_PROPERTY_NOTIFY(name, method)
#define SVG_CS_PROPERTY_RESET(name, method)
#define SVG_CS_PROPERTY_REVISION(name, data)
#define SVG_CS_PROPERTY_DESIGNABLE(name, data)
#define SVG_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define SVG_CS_PROPERTY_SCRIPTABLE(name, data)
#define SVG_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define SVG_CS_PROPERTY_STORED(name, data)
#define SVG_CS_PROPERTY_STORED_NONSTATIC(name, data)
#define SVG_CS_PROPERTY_USER(name, data)
#define SVG_CS_PROPERTY_USER_NONSTATIC(name, data)
#define SVG_CS_PROPERTY_CONSTANT(name)
#define SVG_CS_PROPERTY_FINAL(name)

#endif


// ** 11
#if defined(QT_BUILD_XMLPATTERNS_LIB) || ! defined(Q_OS_WIN)

#define XMLP_CS_OBJECT(className)                                 CS_OBJECT(className)
#define XMLP_CS_OBJECT_MULTIPLE(className, parentX)               CS_OBJECT_MULTIPLE(className, parentX)
#define XMLP_CS_GADGET(className)                                 CS_GADGET(className)

#define XMLP_CS_SLOT_1(access, ...)                               CS_SLOT_1(access, __VA_ARGS__)
#define XMLP_CS_SLOT_2(slotName)                                  CS_SLOT_2(slotName)
#define XMLP_CS_SLOT_OVERLOAD(slotName, argTypes)                 CS_SLOT_OVERLOAD(slotName, argTypes)

#define XMLP_CS_SIGNAL_1(access, ...)                             CS_SIGNAL_1(access, __VA_ARGS__)
#define XMLP_CS_SIGNAL_2(signalName, ...)                         CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define XMLP_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)        CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define XMLP_CS_OBJECT(className)                                 CS_OBJECT_OUTSIDE(className)
#define XMLP_CS_OBJECT_MULTIPLE(className, parentX)               CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define XMLP_CS_GADGET(className)                                 CS_GADGET_OUTSIDE(className)

#define XMLP_CS_SLOT_1(access, ...)                               __VA_ARGS__;
#define XMLP_CS_SLOT_2(slotName)
#define XMLP_CS_SLOT_OVERLOAD(slotName, argTypes)

#define XMLP_CS_SIGNAL_1(access, ...)                             Q_DECL_IMPORT __VA_ARGS__;
#define XMLP_CS_SIGNAL_2(signalName, ...)
#define XMLP_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif

// ** 12
#if defined(BUILDING_WEBKIT) || ! defined(Q_OS_WIN)

#define WEB_CS_OBJECT(className)                                  CS_OBJECT(className)
#define WEB_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE(className, parentX)
#define WEB_CS_GADGET(className)                                  CS_GADGET(className)

#define WEB_CS_SLOT_1(access, ...)                                CS_SLOT_1(access, __VA_ARGS__)
#define WEB_CS_SLOT_2(slotName)                                   CS_SLOT_2(slotName)
#define WEB_CS_SLOT_OVERLOAD(slotName, argTypes)                  CS_SLOT_OVERLOAD(slotName, argTypes)

#define WEB_CS_SIGNAL_1(access, ...)                              CS_SIGNAL_1(access, __VA_ARGS__)
#define WEB_CS_SIGNAL_2(signalName, ...)                          CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define WEB_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)         CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#define WEB_CS_INVOKABLE_METHOD_1(access, ...)                    CS_INVOKABLE_METHOD_1(access, __VA_ARGS__)
#define WEB_CS_INVOKABLE_METHOD_2(methodName)                     CS_INVOKABLE_METHOD_2(methodName)

#define WEB_CS_ENUM(name)                                         CS_ENUM(name)
#define WEB_CS_REGISTER_ENUM(...)                                 CS_REGISTER_ENUM(__VA_ARGS__)
#define WEB_CS_FLAG(enumName, flagName)                           CS_FLAG(enumName, flagName)

#define WEB_CS_PROPERTY_READ(name, method)                        CS_PROPERTY_READ(name, method)
#define WEB_CS_PROPERTY_WRITE(name, method)                       CS_PROPERTY_WRITE(name, method)
#define WEB_CS_PROPERTY_NOTIFY(name, method)                      CS_PROPERTY_NOTIFY(name, method)
#define WEB_CS_PROPERTY_RESET(name, method)                       CS_PROPERTY_RESET(name, method)
#define WEB_CS_PROPERTY_REVISION(name, data)                      CS_PROPERTY_REVISION(name, data)
#define WEB_CS_PROPERTY_DESIGNABLE(name, data)                    CS_PROPERTY_DESIGNABLE(name, data)
#define WEB_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)          CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define WEB_CS_PROPERTY_SCRIPTABLE(name, data)                    CS_PROPERTY_SCRIPTABLE(name, data)
#define WEB_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)          CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define WEB_CS_PROPERTY_STORED(name, data)                        CS_PROPERTY_STORED(name, data)
#define WEB_CS_PROPERTY_STORED_NONSTATIC(name, data)              CS_PROPERTY_STORED_NONSTATIC(name, data)
#define WEB_CS_PROPERTY_USER(name, data)                          CS_PROPERTY_USER(name, data)
#define WEB_CS_PROPERTY_USER_NONSTATIC(name, data)                CS_PROPERTY_USER_NONSTATIC(name, data)
#define WEB_CS_PROPERTY_CONSTANT(name)                            CS_PROPERTY_CONSTANT(name)
#define WEB_CS_PROPERTY_FINAL(name)                               CS_PROPERTY_FINAL(name)

#else
#define WEB_CS_OBJECT(className)                                  CS_OBJECT_OUTSIDE(className)
#define WEB_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define WEB_CS_GADGET(className)                                  CS_GADGET_OUTSIDE(className)

#define WEB_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define WEB_CS_SLOT_2(slotName)
#define WEB_CS_SLOT_OVERLOAD(slotName, argTypes)

#define WEB_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define WEB_CS_SIGNAL_2(signalName, ...)
#define WEB_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define WEB_CS_INVOKABLE_METHOD_1(access, ...)                    __VA_ARGS__;
#define WEB_CS_INVOKABLE_METHOD_2(methodName)

#define WEB_CS_ENUM(name)
#define WEB_CS_REGISTER_ENUM(...)                                 __VA_ARGS__           // do not add ;
#define WEB_CS_FLAG(enumName, flagName)

#define WEB_CS_PROPERTY_READ(name, method)
#define WEB_CS_PROPERTY_WRITE(name, method)
#define WEB_CS_PROPERTY_NOTIFY(name, method)
#define WEB_CS_PROPERTY_RESET(name, method)
#define WEB_CS_PROPERTY_REVISION(name, data)
#define WEB_CS_PROPERTY_DESIGNABLE(name, data)
#define WEB_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define WEB_CS_PROPERTY_SCRIPTABLE(name, data)
#define WEB_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define WEB_CS_PROPERTY_STORED(name, data)
#define WEB_CS_PROPERTY_STORED_NONSTATIC(name, data)
#define WEB_CS_PROPERTY_USER(name, data)
#define WEB_CS_PROPERTY_USER_NONSTATIC(name, data)
#define WEB_CS_PROPERTY_CONSTANT(name)
#define WEB_CS_PROPERTY_FINAL(name)

#endif


// ** 13
#if defined(QT_BUILD_DECLARE_LIB) || ! defined(Q_OS_WIN)

#define DECL_CS_OBJECT(className)                                 CS_OBJECT(className)
#define DECL_CS_OBJECT_MULTIPLE(className, parentX)               CS_OBJECT_MULTIPLE(className, parentX)
#define DECL_CS_GADGET(className)                                 CS_GADGET(className)
#define DECL_CS_CLASSINFO(name, data)                             CS_CLASSINFO(name, data)

#define DECL_CS_SLOT_1(access, ...)                               CS_SLOT_1(access, __VA_ARGS__)
#define DECL_CS_SLOT_2(slotName)                                  CS_SLOT_2(slotName)
#define DECL_CS_SLOT_OVERLOAD(slotName, argTypes)                 CS_SLOT_OVERLOAD(slotName, argTypes)

#define DECL_CS_SIGNAL_1(access, ...)                             CS_SIGNAL_1(access, __VA_ARGS__)
#define DECL_CS_SIGNAL_2(signalName, ...)                         CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define DECL_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)        CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#define DECL_CS_INVOKABLE_METHOD_1(access, ...)                   CS_INVOKABLE_METHOD_1(access, __VA_ARGS__)
#define DECL_CS_INVOKABLE_METHOD_2(methodName)                    CS_INVOKABLE_METHOD_2(methodName)
#define DECL_CS_REVISION(methodName,revision)                     CS_REVISION(methodName,revision)
#define DECL_CS_REVISION_OVERLOAD(methodName, revision, argTypes) CS_REVISION_OVERLOAD(methodName, revision, argTypes)

#define DECL_CS_ENUM(name)                                        CS_ENUM(name)
#define DECL_CS_REGISTER_ENUM(...)                                CS_REGISTER_ENUM(__VA_ARGS__)

#define DECL_CS_PROPERTY_READ(name, method)                       CS_PROPERTY_READ(name, method)
#define DECL_CS_PROPERTY_WRITE(name, method)                      CS_PROPERTY_WRITE(name, method)
#define DECL_CS_PROPERTY_NOTIFY(name, method)                     CS_PROPERTY_NOTIFY(name, method)
#define DECL_CS_PROPERTY_RESET(name, method)                      CS_PROPERTY_RESET(name, method)
#define DECL_CS_PROPERTY_REVISION(name, data)                     CS_PROPERTY_REVISION(name, data)
#define DECL_CS_PROPERTY_DESIGNABLE(name, data)                   CS_PROPERTY_DESIGNABLE(name, data)
#define DECL_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)         CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define DECL_CS_PROPERTY_SCRIPTABLE(name, data)                   CS_PROPERTY_SCRIPTABLE(name, data)
#define DECL_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)         CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define DECL_CS_PROPERTY_STORED(name, data)                       CS_PROPERTY_STORED(name, data)
#define DECL_CS_PROPERTY_STORED_NONSTATIC(name, data)             CS_PROPERTY_STORED_NONSTATIC(name, data)
#define DECL_CS_PROPERTY_USER(name, data)                         CS_PROPERTY_USER(name, data)
#define DECL_CS_PROPERTY_USER_NONSTATIC(name, data)               CS_PROPERTY_USER_NONSTATIC(name, data)
#define DECL_CS_PROPERTY_CONSTANT(name)                           CS_PROPERTY_CONSTANT(name)
#define DECL_CS_PROPERTY_FINAL(name)                              CS_PROPERTY_FINAL(name)

#else
#define DECL_CS_OBJECT(className)                                 CS_OBJECT_OUTSIDE(className)
#define DECL_CS_OBJECT_MULTIPLE(className, parentX)               CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define DECL_CS_GADGET(className)                                 CS_GADGET_OUTSIDE(className)
#define DECL_CS_CLASSINFO(name, data)

#define DECL_CS_SLOT_1(access, ...)                               __VA_ARGS__;
#define DECL_CS_SLOT_2(slotName)
#define DECL_CS_SLOT_OVERLOAD(slotName, argTypes)

#define DECL_CS_SIGNAL_1(access, ...)                             Q_DECL_IMPORT __VA_ARGS__;
#define DECL_CS_SIGNAL_2(signalName, ...)
#define DECL_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define DECL_CS_INVOKABLE_METHOD_1(access, ...)                   __VA_ARGS__;
#define DECL_CS_INVOKABLE_METHOD_2(methodName)
#define DECL_CS_REVISION(methodName,revision)
#define DECL_CS_REVISION_OVERLOAD(methodName, revision, argTypes)

#define DECL_CS_ENUM(name)
#define DECL_CS_REGISTER_ENUM(...)                                __VA_ARGS__           // do not add ;

#define DECL_CS_PROPERTY_READ(name, method)
#define DECL_CS_PROPERTY_WRITE(name, method)
#define DECL_CS_PROPERTY_NOTIFY(name, method)
#define DECL_CS_PROPERTY_RESET(name, method)
#define DECL_CS_PROPERTY_REVISION(name, data)
#define DECL_CS_PROPERTY_DESIGNABLE(name, data)
#define DECL_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define DECL_CS_PROPERTY_SCRIPTABLE(name, data)
#define DECL_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define DECL_CS_PROPERTY_STORED(name, data)
#define DECL_CS_PROPERTY_STORED_NONSTATIC(name, data)
#define DECL_CS_PROPERTY_USER(name, data)
#define DECL_CS_PROPERTY_USER_NONSTATIC(name, data)
#define DECL_CS_PROPERTY_CONSTANT(name)
#define DECL_CS_PROPERTY_FINAL(name)

#endif


// ** 14
#if defined(QT_BUILD_GSTREAMER_LIB) || ! defined(Q_OS_WIN)

#define GSTRM_CS_OBJECT(className)                                CS_OBJECT(className)
#define GSTRM_CS_OBJECT_MULTIPLE(className, parentX)              CS_OBJECT_MULTIPLE(className, parentX)
#define GSTRM_CS_GADGET(className)                                CS_GADGET(className)

#define GSTRM_CS_SLOT_1(access, ...)                              CS_SLOT_1(access, __VA_ARGS__)
#define GSTRM_CS_SLOT_2(slotName)                                 CS_SLOT_2(slotName)
#define GSTRM_CS_SLOT_OVERLOAD(slotName, argTypes)                CS_SLOT_OVERLOAD(slotName, argTypes)

#define GSTRM_CS_SIGNAL_1(access, ...)                            CS_SIGNAL_1(access, __VA_ARGS__)
#define GSTRM_CS_SIGNAL_2(signalName, ...)                        CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define GSTRM_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)       CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define GSTRM_CS_OBJECT(className)                                CS_OBJECT_OUTSIDE(className)
#define GSTRM_CS_OBJECT_MULTIPLE(className, parentX)              CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define GSTRM_CS_GADGET(className)                                CS_GADGET_OUTSIDE(className)

#define GSTRM_CS_SLOT_1(access, ...)                              __VA_ARGS__;
#define GSTRM_CS_SLOT_2(slotName)
#define GSTRM_CS_SLOT_OVERLOAD(slotName, argTypes)

#define GSTRM_CS_SIGNAL_1(access, ...)                            Q_DECL_IMPORT __VA_ARGS__;
#define GSTRM_CS_SIGNAL_2(signalName, ...)
#define GSTRM_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif


// ** 15
#if defined(QT_BUILD_DS9_LIB) || ! defined(Q_OS_WIN)

#define DS9_CS_OBJECT(className)                                  CS_OBJECT(className)
#define DS9_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE(className, parentX)
#define DS9_CS_GADGET(className)                                  CS_GADGET(className)

#define DS9_CS_SLOT_1(access, ...)                                CS_SLOT_1(access, __VA_ARGS__)
#define DS9_CS_SLOT_2(slotName)                                   CS_SLOT_2(slotName)
#define DS9_CS_SLOT_OVERLOAD(slotName, argTypes)                  CS_SLOT_OVERLOAD(slotName, argTypes)

#define DS9_CS_SIGNAL_1(access, ...)                              CS_SIGNAL_1(access, __VA_ARGS__)
#define DS9_CS_SIGNAL_2(signalName, ...)                          CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define DS9_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)         CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define DS9_CS_OBJECT(className)                                  CS_OBJECT_OUTSIDE(className)
#define DS9_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define DS9_CS_GADGET(className)                                  CS_GADGET_OUTSIDE(className)

#define DS9_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define DS9_CS_SLOT_2(slotName)
#define DS9_CS_SLOT_OVERLOAD(slotName, argTypes)

#define DS9_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define DS9_CS_SIGNAL_2(signalName, ...)
#define DS9_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif


// ** 16
#if defined(QT_BUILD_QT7_LIB) || ! defined(Q_OS_WIN)

#define QT7_CS_OBJECT(className)                                  CS_OBJECT(className)
#define QT7_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE(className, parentX)
#define QT7_CS_GADGET(className)                                  CS_GADGET(className)

#define QT7_CS_SLOT_1(access, ...)                                CS_SLOT_1(access, __VA_ARGS__)
#define QT7_CS_SLOT_2(slotName)                                   CS_SLOT_2(slotName)
#define QT7_CS_SLOT_OVERLOAD(slotName, argTypes)                  CS_SLOT_OVERLOAD(slotName, argTypes)

#define QT7_CS_SIGNAL_1(access, ...)                              CS_SIGNAL_1(access, __VA_ARGS__)
#define QT7_CS_SIGNAL_2(signalName, ...)                          CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define QT7_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)         CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define QT7_CS_OBJECT(className)                                  CS_OBJECT_OUTSIDE(className)
#define QT7_CS_OBJECT_MULTIPLE(className, parentX)                CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define QT7_CS_GADGET(className)                                  CS_GADGET_OUTSIDE(className)

#define QT7_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define QT7_CS_SLOT_2(slotName)
#define QT7_CS_SLOT_OVERLOAD(slotName, argTypes)

#define QT7_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define QT7_CS_SIGNAL_2(signalName, ...)
#define QT7_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif


#endif // CSOBJECT_MACRO_H
