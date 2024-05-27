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

#ifndef CSOBJECT_MACRO_H
#define CSOBJECT_MACRO_H

#include <cs_signal.h>
#include <cs_slot.h>
#include <qglobal.h>

#include <optional>

class QMetaObject;

#define qPrintable(string)          QString(string).constData()
#define csPrintable(string)         (string).constData()

#define Q_EMIT
#define Q_ARG(type, data)           CSArgument<type>{data, #type}
#define Q_RETURN_ARG(type, data)    CSReturnArgument<type>(data)

#define CS_TOKENPASTE1(x,y)         x ## y
#define CS_TOKENPASTE2(x,y)         CS_TOKENPASTE1(x,y)

#define CS_PLUGIN_IID(data)                                                         \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =          \
         decltype( cs_counter(cs_number<255>{}) )::value;                           \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>       \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);           \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
   {  \
      QMetaObject_T<cs_class> &meta = const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()); \
      meta.register_classInfo("plugin_iid", data);                                  \
      meta.register_classInfo("plugin_version", QString::number(CS_VERSION));       \
      \
      constexpr int cntValue = CS_TOKENPASTE2(cs_counter_value, __LINE__);          \
      \
      QString cname = QString::fromUtf8(cs_className());                            \
      meta.register_method<QObject * (*)()>(                                        \
            cname, &cs_class::CS_TOKENPASTE2(cs_fauxConstructor, __LINE__),         \
            QMetaMethod::Constructor, cname + " " + cname + "()", QMetaMethod::Public);  \
      \
      cs_regTrigger(cs_number<cntValue + 1>{});                                     \
   }                                                                                \
   static QObject * CS_TOKENPASTE2(cs_fauxConstructor, __LINE__)()                  \
   {                                                                                \
      return new cs_class;                                                          \
   }

#define CS_PLUGIN_KEY(y)  CS_CLASSINFO("plugin_key", y)

#define CS_PLUGIN_REGISTER(classname)                                                \
   extern "C" Q_DECL_EXPORT QMetaObject *cs_internal_plugin_metaobject() {           \
      return const_cast<QMetaObject_T<classname> *>(&classname::staticMetaObject()); \
   }

#if ! defined (CS_DOXYPRESS)

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

#endif // doxypress

# define CS_TR_FUNCTIONS \
   static inline QString tr(const char *text, const char *comment = nullptr, std::optional<int> numArg = std::optional<int>()) \
   { return staticMetaObject().tr(text, comment, numArg); }

// ** cs_object
#define CS_OBJECT(classNameX)      \
 public:                           \
   using cs_parent = cs_class;     \
   using cs_class  = classNameX;   \
   CS_OBJECT_INTERNAL(classNameX)  \
 private:

#define CS_OBJECT_MULTIPLE(classNameX,parentX) \
 public:                           \
   using cs_parent = parentX;      \
   using cs_class  = classNameX;   \
   CS_OBJECT_INTERNAL(classNameX)  \
 private:

#define CS_OBJECT_OUTSIDE(classNameX)     \
 public:                                  \
   using cs_parent = cs_class;            \
   using cs_class  = classNameX;          \
   CS_OBJECT_INTERNAL_OUTSIDE(classNameX) \
 private:

#define CS_OBJECT_MULTIPLE_OUTSIDE(classNameX,parentX) \
 public:                                  \
   using cs_parent = parentX;             \
   using cs_class  = classNameX;          \
   CS_OBJECT_INTERNAL_OUTSIDE(classNameX) \
 private:

#define CS_OVERRIDE override

#define CS_OBJECT_INTERNAL(classNameX)  \
 public:                                \
   static const char *cs_className()    \
   { \
      static const char *retval(#classNameX); \
      return retval;                          \
   } \
   template<int N>                            \
   static void cs_regTrigger(cs_number<N>)    \
   { \
   } \
   static constexpr cs_number<0> cs_counter(cs_number<0>) \
   { \
      return cs_number<0>{};                  \
   } \
   friend QMetaObject_T<classNameX>;          \
   [[gnu::used]] Q_EXPORT_MAYBE static const QMetaObject_T<classNameX> &staticMetaObject()  \
   { \
      static std::atomic<bool> isCreated(false);                            \
      static std::atomic<QMetaObject_T<classNameX> *> createdObj(nullptr);  \
      if (isCreated) {                                                      \
         return *createdObj;                                                \
      } \
      std::lock_guard<std::recursive_mutex> lock(m_metaObjectMutex());      \
      if (createdObj != nullptr) {                                          \
         return *createdObj;                                                \
      } \
      QMap<std::type_index, QMetaObject *> &temp = m_metaObjectsAll();      \
      auto index = temp.find(typeid(cs_class));    \
      QMetaObject_T<classNameX> *newMeta;          \
      if (index == temp.end()) {                   \
         newMeta = new QMetaObject_T<classNameX>;  \
         temp.insert(typeid(cs_class), newMeta);   \
         createdObj.store(newMeta);                \
         newMeta->postConstruct();                 \
         isCreated = true;                         \
         return *newMeta;                          \
      } else {  \
         newMeta = dynamic_cast<QMetaObject_T<classNameX> *> (index.value()); \
         createdObj.store(newMeta);  \
         isCreated = true;           \
         return *newMeta;            \
      } \
   } \
   virtual Q_EXPORT_MAYBE const QMetaObject *metaObject() const CS_OVERRIDE \
   { \
      return &staticMetaObject(); \
   } \
   CS_TR_FUNCTIONS \
 private:

#define CS_OBJECT_INTERNAL_OUTSIDE(classNameX) \
 public:                                       \
   static const char *cs_className()           \
   { \
      static const char * retval(#classNameX); \
      return retval;                           \
   } \
   static const QMetaObject_T<classNameX> & staticMetaObject(); \
   virtual const QMetaObject *metaObject() const CS_OVERRIDE;   \
   CS_TR_FUNCTIONS \
 private:

// ** cs_gadget
#define CS_GADGET(classNameX)                \
 public:                                     \
   using cs_parent = CSGadget_Fake_Parent;   \
   using cs_class  = classNameX;             \
   CS_GADGET_INTERNAL(classNameX)            \
 private:

#define CS_GADGET_OUTSIDE(classNameX)        \
 public:                                     \
   using cs_parent = CSGadget_Fake_Parent;   \
   using cs_class  = classNameX;             \
   CS_GADGET_INTERNAL_OUTSIDE(classNameX)    \
 private:

#define CS_GADGET_INTERNAL(classNameX) \
 public:                               \
   static const char *cs_className()   \
   { \
      static const char *retval(#classNameX); \
      return retval;                          \
   } \
   template<int N>                            \
   static void cs_regTrigger(cs_number<N>)    \
   { \
   } \
   static constexpr cs_number<0> cs_counter(cs_number<0>) \
   { \
      return cs_number<0>{};                  \
   } \
   friend QMetaObject_T<classNameX>;          \
   [[gnu::used]] Q_EXPORT_MAYBE static const QMetaObject_T<classNameX> &staticMetaObject() \
   { \
      static std::atomic<bool> isCreated(false);                             \
      static std::atomic<QMetaObject_T<classNameX> *> createdObj(nullptr);   \
      if (isCreated) {               \
         return *createdObj;         \
      } \
      std::lock_guard<std::recursive_mutex> lock(QObject::m_metaObjectMutex()); \
      if (createdObj != nullptr) {   \
         return *createdObj;         \
      } \
      QMap<std::type_index, QMetaObject *> &temp = QObject::m_metaObjectsAll(); \
      auto index = temp.find(typeid(cs_class));       \
      QMetaObject_T<classNameX> *newMeta;             \
      if (index == temp.end()) {                      \
         newMeta = new QMetaObject_T<classNameX>;     \
         temp.insert(typeid(cs_class), newMeta);      \
         newMeta->postConstruct();                    \
         return *newMeta;                             \
      } else {                                        \
         newMeta = dynamic_cast<QMetaObject_T<classNameX> *> (index.value()); \
         createdObj.store(newMeta);  \
         isCreated = true;           \
         return *newMeta;            \
      } \
   } \
   CS_TR_FUNCTIONS                   \
 private:

#define CS_GADGET_INTERNAL_OUTSIDE(classNameX) \
 public:                                       \
   static const char *cs_className()           \
   { \
      static const char *retval(#classNameX);  \
      return retval;                           \
   } \
   static const QMetaObject_T<classNameX> &staticMetaObject(); \
   CS_TR_FUNCTIONS                             \
 private:

// ** interface
#define CS_DECLARE_INTERFACE(IFace, IId)       \
   template<>                                  \
   inline const QString &qobject_interface_iid<IFace *>() \
   {  \
      static QString retval(IId);              \
      return retval;                           \
   }

#define CS_INTERFACES(...)                     \
 public: \
   bool cs_interface_query(const QString &interfaceData) const override \
   {  \
      if (cs_factory_interface_query<__VA_ARGS__>(interfaceData)) { \
         return true;                          \
      }  \
      return false;                            \
   }  \
 private:

// ** classInfo
#define CS_CLASSINFO(name, data) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =           \
         decltype( cs_counter(cs_number<255>{}) )::value;                            \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);            \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
   {  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_classInfo(name, data); \
      \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} );   \
   }

// ** enum
#define CS_ENUM(name) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =           \
         decltype( cs_counter(cs_number<255>{}) )::value;                            \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);            \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
   {  \
      cs_namespace_register_enum<cs_class>(#name, typeid(name), cs_className());     \
      \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} );   \
   }

#define CS_REGISTER_ENUM(...) \
   __VA_ARGS__ \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =           \
         decltype( cs_counter(cs_number<255>{}) )::value;                            \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);            \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
   {  \
      cs_namespace_register_enum_data<cs_class>(#__VA_ARGS__);                       \
      \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} );   \
   }

// ** flag
#define CS_FLAG(enumName, flagName) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =           \
         decltype( cs_counter(cs_number<255>{}) )::value;                            \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);            \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
   {  \
      cs_namespace_register_flag<cs_class>(#enumName, cs_className(),                \
            #flagName, typeid(flagName));                                            \
      \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} );   \
   }

// ** invoke constructor
#define CS_INVOKABLE_CONSTRUCTOR_1(access, ...) \
   __VA_ARGS__; \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =          \
         decltype( cs_counter(cs_number<255>{}) )::value;                           \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>       \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);           \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
   {  \
      QString va_args = #__VA_ARGS__;                                               \
      QMetaMethod::Access accessType = QMetaMethod::access;                         \
      constexpr int cntValue = CS_TOKENPASTE2(cs_counter_value, __LINE__);
// do not remove the ";", this is required for part two of the macro

#define CS_INVOKABLE_CONSTRUCTOR_2(className, ...)                                  \
   const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method       \
   <QObject * (*)(__VA_ARGS__)>(                                                            \
         #className, &cs_class::CS_TOKENPASTE2(cs_fauxConstructor, __LINE__)<__VA_ARGS__>,  \
         QMetaMethod::Constructor, QString(#className) + " " + va_args, accessType);        \
   cs_regTrigger(cs_number<cntValue + 1>{} );                                               \
   } \
   template <class... Ts>                                                 \
   static QObject * CS_TOKENPASTE2(cs_fauxConstructor, __LINE__)(Ts...Vs) \
   { \
      return new className{Vs...};                                        \
   }

// ** invoke
#define CS_INVOKABLE_METHOD_1(access, ...) \
   __VA_ARGS__; \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =          \
         decltype( cs_counter(cs_number<255>{}) )::value;                           \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>       \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);           \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
   {  \
      const auto &va_args = #__VA_ARGS__;                   \
      QMetaMethod::Access accessType = QMetaMethod::access; \
      constexpr int cntValue = CS_TOKENPASTE2(cs_counter_value, __LINE__);
// do not remove the ";", this is required for part two of the macro

#define CS_INVOKABLE_METHOD_2(methodName)                   \
   const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method(  \
         #methodName, &cs_class::methodName, QMetaMethod::Method, va_args, accessType); \
   cs_regTrigger(cs_number<cntValue + 1>{});                \
   }

#define CS_INVOKABLE_OVERLOAD(methodName, argTypes, ...)                                \
   const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method(  \
         #methodName, static_cast<void (cs_class::*)argTypes>(&cs_class::methodName),   \
         QMetaMethod::Method, va_args, accessType);         \
   cs_regTrigger(cs_number<cntValue + 1>{});                \
   }

// ** revision
#define CS_REVISION(methodName,revision) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =           \
         decltype( cs_counter(cs_number<255>{}) )::value;                            \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);            \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
   {  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method_rev( \
            &cs_class::methodName, revision);                                        \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} );   \
   }

#define CS_REVISION_OVERLOAD(methodName, revision, argTypes)                         \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =           \
         decltype( cs_counter(cs_number<255>{}) )::value;                            \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);            \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
   {  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method_rev( \
            static_cast<void (cs_class::*)argTypes>(&cs_class::methodName), revision);        \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});    \
   }

// ** slots
#define CS_SLOT_1(access, ...)                                                      \
   __VA_ARGS__;                                                                     \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =          \
         decltype(cs_counter(cs_number<255>{}))::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>       \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);           \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
   {  \
      const auto &va_args = #__VA_ARGS__;                                           \
      QMetaMethod::Access accessType = QMetaMethod::access;                         \
      constexpr int cntValue = CS_TOKENPASTE2(cs_counter_value, __LINE__);
// do not remove the ";", this is required for part two of the macro

#define CS_SLOT_2(slotName) \
   const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method(  \
         #slotName, &cs_class::slotName, QMetaMethod::Slot, va_args, accessType);       \
   cs_regTrigger(cs_number<cntValue + 1>{} );                                           \
   }

// ** signals
#define CS_SIGNAL_1(access, ...)                                                    \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =          \
         decltype(cs_counter(cs_number<255>{}))::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>       \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);           \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
   {  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method_s1(  \
            #__VA_ARGS__, QMetaMethod::access, QMetaMethod::Signal);                \
      \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} );  \
   }  \
   Q_EXPORT_MAYBE  __VA_ARGS__ {      \
      if (this->signalsBlocked()) {   \
         return;                      \
      }
// do not remove the "{", this is required for part two of the macro

#define CS_SIGNAL_2(signalName, ...) \
   CsSignal::activate(*this, &cs_class::signalName, ##__VA_ARGS__);                 \
   }  \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =          \
         decltype(cs_counter(cs_number<255>{}))::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>       \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);           \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
   {  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method_s2(  \
            #signalName, &cs_class::signalName, QMetaMethod::Signal);               \
      \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} );  \
   }  \

#define CS_SIGNAL_OVERLOAD(signalName, argTypes, ...) \
   CsSignal::activate(*this, static_cast<void (cs_class::*)argTypes>(&cs_class::signalName), ##__VA_ARGS__); \
   }  \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =          \
         decltype(cs_counter(cs_number<255>{}))::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>       \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);           \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>) \
   {  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method_s2(  \
            #signalName #argTypes, static_cast<void (cs_class::*)argTypes>(&cs_class::signalName), QMetaMethod::Signal);  \
      \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{} );  \
   }  \

#define CS_SLOT_OVERLOAD(slotName, argTypes)                                                 \
   const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method(       \
         #slotName #argTypes, static_cast<void (cs_class::*)argTypes>(&cs_class::slotName),  \
         QMetaMethod::Slot, va_args, accessType);                                            \
   cs_regTrigger(cs_number<cntValue + 1>{} );                                                \
   }

#define CS_SLOT_OVERLOAD_BOOL(slotName, argTypes)                                            \
   const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_method(       \
         #slotName #argTypes, static_cast<bool (cs_class::*)argTypes>(&cs_class::slotName),  \
         QMetaMethod::Slot, va_args, accessType);                                            \
   cs_regTrigger(cs_number<cntValue + 1>{} );                                                \
   }

#if defined(CS_SHOW_DEBUG_CORE)

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

// ** metaMethod tag
#define CS_TAG(name, data) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =           \
         decltype( cs_counter(cs_number<255>{}) )::value;                            \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);            \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
   {                                                                                 \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_tag(#name, #data); \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});    \
   }

// ** properties
#define CS_PROPERTY_READ(name, method) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =           \
         decltype( cs_counter(cs_number<255>{}) )::value;                            \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>        \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);            \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)  \
   {                                                                                 \
      using T = decltype(&cs_class::method);                                         \
      using R = cs_returnType<T>::type;                                              \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())             \
            .register_property_read(#name, typeid(R),                                \
            [] () { return cs_typeToName<R>(); },                                    \
            new SpiceJarRead<cs_class, R>(&cs_class::method));                       \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});    \
   }

#define CS_PROPERTY_WRITE(name, method) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =            \
         decltype( cs_counter(cs_number<255>{}) )::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>         \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);             \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)   \
   {                                                                                  \
      using T = decltype(&cs_class::method);                                          \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())              \
            .register_property_write(#name,                                           \
            new SpiceJarWrite<cs_class, cs_argType<T>::type>(&cs_class::method),      \
            #method);                                                                 \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});     \
   }

#define CS_PROPERTY_NOTIFY(name, method) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =            \
         decltype( cs_counter(cs_number<255>{}) )::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>         \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);             \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)   \
   {                                                                                  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())              \
            .register_property_notify(#name, &cs_class::method);                      \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});     \
   }

#define CS_PROPERTY_RESET(name, method) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =            \
         decltype( cs_counter(cs_number<255>{}) )::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>         \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);             \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)   \
   {                                                                                  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())              \
            .register_property_reset(#name, &cs_class::method);                       \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});     \
   }

#define CS_PROPERTY_REVISION(name, data) \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =            \
         decltype( cs_counter(cs_number<255>{}) )::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>         \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);             \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)   \
   {                                                                                  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())              \
            .register_property_int(#name, data, QMetaProperty::REVISION);             \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});     \
   }

#define CS_PROPERTY_DESIGNABLE(name, data)                 \
   static bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()   \
   {                                                    \
      return data;                                      \
   }                                                    \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =            \
         decltype( cs_counter(cs_number<255>{}) )::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>         \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);             \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)   \
   {                                                                                  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())              \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>           \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::DESIGNABLE); \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});     \
   }

#define CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)       \
   bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)() const    \
   {                                                       \
      return data;                                         \
   }                                                       \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =            \
         decltype( cs_counter(cs_number<255>{}) )::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>         \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);             \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)   \
   {                                                                                  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())              \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>           \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::DESIGNABLE); \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});     \
   }

#define CS_PROPERTY_SCRIPTABLE(name, data)                 \
   static bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()   \
   {                                                       \
      return data;                                         \
   }                                                       \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =             \
         decltype( cs_counter(cs_number<255>{}) )::value;                              \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>          \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);              \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)    \
   {                                                                                   \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())               \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>            \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::SCRIPTABLE); \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});      \
   }

#define CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)       \
   bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)() const    \
   {                                                       \
      return data;                                         \
   }                                                       \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =            \
         decltype( cs_counter(cs_number<255>{}) )::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>         \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);             \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)   \
   {                                                                                  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())              \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>           \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::SCRIPTABLE); \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});     \
   }

#define CS_PROPERTY_STORED(name, data)                     \
   static bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()   \
   {                                                       \
      return data;                                         \
   }                                                       \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =             \
         decltype( cs_counter(cs_number<255>{}) )::value;                              \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>          \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);              \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)    \
   {                                                                                   \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())               \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>            \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::STORED); \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});      \
   }

#define CS_PROPERTY_STORED_NONSTATIC(name, data)           \
   bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)() const    \
   {                                                       \
      return data;                                         \
   }                                                       \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =            \
         decltype( cs_counter(cs_number<255>{}) )::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>         \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);             \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)   \
   {                                                                                  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())              \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>           \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::STORED); \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});     \
   }

#define CS_PROPERTY_USER(name, data)                       \
   static bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()   \
   {                                                       \
      return data;                                         \
   }                                                       \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =            \
         decltype( cs_counter(cs_number<255>{}) )::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>         \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);             \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)   \
   {                                                                                  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())              \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>           \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::USER); \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});     \
   }

#define CS_PROPERTY_USER_NONSTATIC(name, data)             \
   bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()  const   \
   {                                                       \
      return data;                                         \
   }                                                       \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =            \
         decltype( cs_counter(cs_number<255>{}) )::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>         \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);             \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)   \
   {                                                                                  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())              \
            .register_property_bool(#name, new SpiceJarRead<cs_class, bool>             \
            (&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), QMetaProperty::USER); \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});      \
   }

#define CS_PROPERTY_CONSTANT(name)                                                    \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =            \
         decltype( cs_counter(cs_number<255>{}) )::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>         \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);             \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)   \
   {                                                                                  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject())              \
            .register_property_int(#name, 1, QMetaProperty::CONSTANT);                \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});     \
   }

#define CS_PROPERTY_FINAL(name)                                                       \
   static constexpr const int CS_TOKENPASTE2(cs_counter_value, __LINE__) =            \
         decltype( cs_counter(cs_number<255>{}) )::value;                             \
   static constexpr cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>         \
   cs_counter(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>);             \
   static void cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__)>)   \
   {                                                                                  \
      const_cast<QMetaObject_T<cs_class>&>(cs_class::staticMetaObject()).register_property_int(  \
            #name, 1, QMetaProperty::FINAL);                                          \
      cs_regTrigger(cs_number<CS_TOKENPASTE2(cs_counter_value, __LINE__) + 1>{});     \
   }

// ** macros used when compiling

// ** 1
#if defined(QT_BUILD_CORE_LIB) || defined(Q_OS_DARWIN)

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

#define CORE_CS_SLOT_1(access, ...)                            __VA_ARGS__;
#define CORE_CS_SLOT_2(slotName)
#define CORE_CS_SLOT_OVERLOAD(slotName, argTypes)

#define CORE_CS_SIGNAL_1(access, ...)                          __VA_ARGS__;
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
#if defined(QT_BUILD_GUI_LIB) || defined(Q_OS_DARWIN)

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

#define GUI_CS_SIGNAL_1(access, ...)                              __VA_ARGS__;
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
#if defined(QT_BUILD_MULTIMEDIA_LIB) || defined(Q_OS_DARWIN)

#define MULTI_CS_OBJECT(className)                                CS_OBJECT(className)
#define MULTI_CS_OBJECT_MULTIPLE(className, parentX)              CS_OBJECT_MULTIPLE(className, parentX)
#define MULTI_CS_GADGET(className)                                CS_GADGET(className)

#define MULTI_CS_CLASSINFO(name, data)                            CS_CLASSINFO(name, data)

#define MULTI_CS_SLOT_1(access, ...)                              CS_SLOT_1(access, __VA_ARGS__)
#define MULTI_CS_SLOT_2(slotName)                                 CS_SLOT_2(slotName)
#define MULTI_CS_SLOT_OVERLOAD(slotName, argTypes)                CS_SLOT_OVERLOAD(slotName, argTypes)

#define MULTI_CS_SIGNAL_1(access, ...)                            CS_SIGNAL_1(access, __VA_ARGS__)
#define MULTI_CS_SIGNAL_2(signalName, ...)                        CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define MULTI_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)       CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#define MULTI_CS_ENUM(name)                                       CS_ENUM(name)

#define MULTI_CS_PROPERTY_READ(name, method)                      CS_PROPERTY_READ(name, method)
#define MULTI_CS_PROPERTY_WRITE(name, method)                     CS_PROPERTY_WRITE(name, method)
#define MULTI_CS_PROPERTY_NOTIFY(name, method)                    CS_PROPERTY_NOTIFY(name, method)
#define MULTI_CS_PROPERTY_RESET(name, method)                     CS_PROPERTY_RESET(name, method)
#define MULTI_CS_PROPERTY_REVISION(name, da)                      CS_PROPERTY_REVISION(name, data)
#define MULTI_CS_PROPERTY_DESIGNABLE(namedata)                    CS_PROPERTY_DESIGNABLE(name, data)
#define MULTI_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)        CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define MULTI_CS_PROPERTY_SCRIPTABLE(namedata)                    CS_PROPERTY_SCRIPTABLE(name, data)
#define MULTI_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)        CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define MULTI_CS_PROPERTY_STORED(name, data)                      CS_PROPERTY_STORED(name, data)
#define MULTI_CS_PROPERTY_STORED_NONSTATIC(name, data)            CS_PROPERTY_STORED_NONSTATIC(name, data)
#define MULTI_CS_PROPERTY_USER(name, data)                        CS_PROPERTY_USER(name, data)
#define MULTI_CS_PROPERTY_USER_NONSTATIC(name, data)              CS_PROPERTY_USER_NONSTATIC(name, data)
#define MULTI_CS_PROPERTY_CONSTANT(name)                          CS_PROPERTY_CONSTANT(name)
#define MULTI_CS_PROPERTY_FINAL(name)                             CS_PROPERTY_FINAL(name)

#else
#define MULTI_CS_OBJECT(className)                                CS_OBJECT_OUTSIDE(className)
#define MULTI_CS_OBJECT_MULTIPLE(className, parentX)              CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)
#define MULTI_CS_GADGET(className)                                CS_GADGET_OUTSIDE(className)

#define MULTI_CS_CLASSINFO(name, data)

#define MULTI_CS_SLOT_1(access, ...)                              __VA_ARGS__;
#define MULTI_CS_SLOT_2(slotName)
#define MULTI_CS_SLOT_OVERLOAD(slotName, argTypes)

#define MULTI_CS_SIGNAL_1(access, ...)                            __VA_ARGS__;
#define MULTI_CS_SIGNAL_2(signalName, ...)
#define MULTI_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define MULTI_CS_ENUM(name)

#define MULTI_CS_PROPERTY_READ(name, method)
#define MULTI_CS_PROPERTY_WRITE(name, method)
#define MULTI_CS_PROPERTY_NOTIFY(name, method)
#define MULTI_CS_PROPERTY_RESET(name, method)
#define MULTI_CS_PROPERTY_REVISION(name, da)
#define MULTI_CS_PROPERTY_DESIGNABLE(namedata)
#define MULTI_CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data)
#define MULTI_CS_PROPERTY_SCRIPTABLE(namedata)
#define MULTI_CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data)
#define MULTI_CS_PROPERTY_STORED(name, data)
#define MULTI_CS_PROPERTY_STORED_NONSTATIC(name, data)
#define MULTI_CS_PROPERTY_USER(name, data)
#define MULTI_CS_PROPERTY_USER_NONSTATIC(name, data)
#define MULTI_CS_PROPERTY_CONSTANT(name)
#define MULTI_CS_PROPERTY_FINAL(name)

#endif

// ** 4
#if defined(QT_BUILD_NETWORK_LIB) || defined(Q_OS_DARWIN)

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

#define NET_CS_SIGNAL_1(access, ...)                              __VA_ARGS__;
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
#if defined(QT_BUILD_OPENGL_LIB) || defined(Q_OS_DARWIN)

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

#define OPENGL_CS_SIGNAL_1(access, ...)                           __VA_ARGS__;
#define OPENGL_CS_SIGNAL_2(signalName, ...)
#define OPENGL_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif

// ** 6
#if defined(QT_BUILD_SCRIPT_LIB) || defined(Q_OS_DARWIN)

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

#define SCRIPT_CS_SIGNAL_1(access, ...)                           __VA_ARGS__;

#define SCRIPT_CS_SIGNAL_2(signalName, ...)
#define SCRIPT_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif

// ** 7
#if defined(QT_BUILD_SCRIPTTOOLS_LIB) || defined(Q_OS_DARWIN)

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

#define SCRIPT_T_CS_SIGNAL_1(access, ...)                         __VA_ARGS__;
#define SCRIPT_T_CS_SIGNAL_2(signalName, ...)
#define SCRIPT_T_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif

// ** 8
#if defined(QT_BUILD_SQL_LIB) || defined(Q_OS_DARWIN)

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

#define SQL_CS_SIGNAL_1(access, ...)                              __VA_ARGS__;
#define SQL_CS_SIGNAL_2(signalName, ...)
#define SQL_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif

// ** 9
#if defined(QT_BUILD_SVG_LIB) || defined(Q_OS_DARWIN)

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

#define SVG_CS_SIGNAL_1(access, ...)                              __VA_ARGS__;
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

// ** 10
#if defined(QT_BUILD_VULKAN_LIB) || defined(Q_OS_DARWIN)

#define VULKAN_CS_OBJECT(className)                               CS_OBJECT(className)
#define VULKAN_CS_OBJECT_MULTIPLE(className, parentX)             CS_OBJECT_MULTIPLE(className, parentX)

#define VULKAN_CS_SIGNAL_1(access, ...)                           CS_SIGNAL_1(access, __VA_ARGS__)
#define VULKAN_CS_SIGNAL_2(signalName, ...)                       CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define VULKAN_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)      CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define VULKAN_CS_OBJECT(className)                               CS_OBJECT_OUTSIDE(className)
#define VULKAN_CS_OBJECT_MULTIPLE(className, parentX)             CS_OBJECT_MULTIPLE_OUTSIDE(className, parentX)

#define VULKAN_CS_SIGNAL_1(access, ...)                           __VA_ARGS__;
#define VULKAN_CS_SIGNAL_2(signalName, ...)
#define VULKAN_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif

// ** 11
#if defined(QT_BUILD_XMLPATTERNS_LIB) || defined(Q_OS_DARWIN)

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

#define XMLP_CS_SIGNAL_1(access, ...)                             __VA_ARGS__;
#define XMLP_CS_SIGNAL_2(signalName, ...)
#define XMLP_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif

// ** 12
#if defined(BUILDING_WEBKIT) || defined(Q_OS_DARWIN)

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

#define WEB_CS_SIGNAL_1(access, ...)                              __VA_ARGS__;
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
#if defined(QT_BUILD_DECLARE_LIB) || defined(Q_OS_DARWIN)

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

#define DECL_CS_SIGNAL_1(access, ...)                             __VA_ARGS__;
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

#endif // CSOBJECT_MACRO_H
