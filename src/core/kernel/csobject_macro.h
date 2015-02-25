/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef CSOBJECT_MACRO_H
#define CSOBJECT_MACRO_H

#ifndef csPrintable
#  define csPrintable(string)       QString(string).toLatin1().constData()
#endif

#define Q_EMIT
#define Q_ARG(type, data)           CSArgument<type>{data, #type}
#define Q_RETURN_ARG(type, data)    CSReturnArgument<type>(data)

#define CS_TOKENPASTE1(x,y)         x ## y
#define CS_TOKENPASTE2(x,y)         CS_TOKENPASTE1(x,y)


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

#define CS_OBJECT_INTERNAL(classNameX) \
   public: \
      static const char *cs_className() \
      { \
         return #classNameX; \
      } \
      static const QMetaObject_T<classNameX> & staticMetaObject() \
      { \
         QMap<std::type_index, QMetaObject *> &temp = m_metaObjectsAll(); \
         auto index = temp.find(typeid(cs_class));    \
         if (index == temp.end()) {     \
            QMetaObject_T<classNameX> *xx = new QMetaObject_T<classNameX>;  \
            temp.insert(typeid(cs_class), xx);  \
            return *xx; \
         } else {    \
            return *dynamic_cast<QMetaObject_T<classNameX> *> (index.value()); \
         } \
      } \
      virtual const QMetaObject *metaObject() const \
      { \
         return &staticMetaObject(); \
      } \
      CS_TR_FUNCTIONS \
   private: \
      struct cs_classname \
      { \
         static constexpr const char *value = #classNameX; \
      }; \
   public:


// ** cs_gadget
#define CS_GADGET(classNameX) \
   public: \
      typedef CSGadget_Fake_Parent cs_parent; \
      typedef classNameX cs_class; \
      CS_GADGET_INTERNAL(classNameX) \
   private:


#define CS_GADGET_INTERNAL(classNameX) \
   public: \
      static const char *cs_className() \
      { \
         return #classNameX; \
      }; \
      static const QMetaObject_T<classNameX> & staticMetaObject() \
      { \
         QMap<std::type_index, QMetaObject *> &temp = CSGadget_Fake_Parent::m_metaObjectsAll(); \
         auto index = temp.find(typeid(cs_class));    \
         if (index == temp.end()) {     \
            QMetaObject_T<classNameX> *xx = new QMetaObject_T<classNameX>;  \
            temp.insert(typeid(cs_class), xx);  \
            return *xx; \
         } else {    \
            return *dynamic_cast<QMetaObject_T<classNameX> *> (index.value()); \
         } \
      } \
      CS_TR_FUNCTIONS \
   private: \
      struct cs_classname \
      { \
         static constexpr const char *value = #classNameX; \
      }; \
   public:


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


#define CS_INTERFACES(...)  \
   public: \
      bool cs_interface_query(const char *data) const  \
      {  \
         if (cs_factory_interface_query<__VA_ARGS__>(data)) { \
            return true;  \
         }  \
         return false;  \
      }  \
   private:  \
 

// ** classInfo
#define CS_CLASSINFO(name, data) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = name; \
      }; \
   struct CS_TOKENPASTE2(cs_fauxData, __LINE__) \
      { \
         static constexpr const char *value = data; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_classInfo<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          CS_TOKENPASTE2(cs_fauxData, __LINE__)>::placeholder; \
      }

// ** enum
#define CS_ENUM(name) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_enum<cs_class, name, CS_TOKENPASTE2(cs_fauxName, __LINE__), cs_classname>::placeholder; \
      }


#define CS_REGISTER_ENUM(...) \
   __VA_ARGS__ \
   struct CS_TOKENPASTE2(cs_fauxArgs, __LINE__) \
      { \
         static constexpr const char *value = #__VA_ARGS__; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_enum_data<cs_class, CS_TOKENPASTE2(cs_fauxArgs, __LINE__), cs_classname>::placeholder; \
      }


// ** flag
#define CS_FLAG(enumName, flagName) \
   struct CS_TOKENPASTE2(cs_fauxEnumName, __LINE__) \
      { \
         static constexpr const char *value = #enumName; \
      }; \
   struct CS_TOKENPASTE2(cs_fauxFlagName, __LINE__) \
      { \
         static constexpr const char *value = #flagName; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_flag<cs_class, flagName, CS_TOKENPASTE2(cs_fauxEnumName, __LINE__), cs_classname, \
                                          CS_TOKENPASTE2(cs_fauxFlagName, __LINE__) >::placeholder; \
      }


// ** invoke constructor
#define CS_INVOKABLE_CONSTRUCTOR_1(access, ...) \
   __VA_ARGS__;   \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #__VA_ARGS__; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_method<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), QMetaMethod::access, \
                                          QMetaMethod::Constructor>::placeholder;   \
// do not remove the ";", this is required for part two of the macro


#define CS_INVOKABLE_CONSTRUCTOR_2(className, ...) \
      }  \
   template<class ...Ts> \
   static QObject * CS_TOKENPASTE2(cs_faux_constructor, __LINE__)(Ts... Vs)  { \
      return new className{Vs...}; \
   }  \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #className; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_method_ptr<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                              decltype(&cs_class::CS_TOKENPASTE2(cs_faux_constructor, __LINE__)<__VA_ARGS__>), \
                              &cs_class::CS_TOKENPASTE2(cs_faux_constructor, __LINE__)<__VA_ARGS__>, \
                              QMetaMethod::Constructor>::placeholder; \
      }


// ** invoke
#define CS_INVOKABLE_METHOD_1(access, ...) \
   __VA_ARGS__; \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #__VA_ARGS__; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_method<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), QMetaMethod::access, \
                                          QMetaMethod::Method>::placeholder;  \
// do not remove the ";", this is required for part two of the macro


#define CS_INVOKABLE_METHOD_2(methodName) \
      }  \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #methodName; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_method_ptr<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::methodName), \
                                          &cs_class::methodName, QMetaMethod::Method>::placeholder; \
      }


#define CS_INVOKABLE_OVERLOAD(methodName, argTypes, ...) \
      }  \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #methodName #argTypes; \
      };    \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_method_ptr<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          void (cs_class::*)argTypes, \
                                          &cs_class::methodName, QMetaMethod::Methodl>::placeholder; \
      }


// ** revision
#define CS_REVISION(methodName,revision) \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_method_rev<cs_class, decltype(&cs_class::methodName), \
                                          &cs_class::methodName, revision>::placeholder; \
      }

#define CS_REVISION_OVERLOAD(methodName, revision, argTypes) \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_method_rev<cs_class, void (cs_class::*)argTypes, \
                                          &cs_class::methodName, revision>::placeholder; \
      }



// ** slots
#define CS_SLOT_1(access, ...) \
   __VA_ARGS__; \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #__VA_ARGS__; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_method<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), QMetaMethod::access, \
                                          QMetaMethod::Slot>::placeholder; \
// do not remove the ";", this is required for part two of the macro


#define CS_SLOT_2(slotName) \
      }  \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #slotName; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_method_ptr<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::slotName), \
                                          &cs_class::slotName, QMetaMethod::Slot>::placeholder; \
      }


// ** signals
#define CS_SIGNAL_1(access, ...) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
          static constexpr const char *value = #__VA_ARGS__; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_method<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), QMetaMethod::access, \
                                          QMetaMethod::Signal>::placeholder; \
      } \
   __VA_ARGS__ {
// do not remove the "{", this is required for part two of the macro


#define CS_SIGNAL_2(signalName, ...) \
      QMetaObject::activate(this, &cs_class::signalName, ##__VA_ARGS__); \
   }  \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #signalName; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_method_ptr<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::signalName), \
                                          &cs_class::signalName, QMetaMethod::Signal>::placeholder; \
      }


#define CS_SIGNAL_OVERLOAD(signalName, argTypes, ...) \
      QMetaObject::activate(this, \
                  static_cast<void (cs_class::*)argTypes>(&cs_class::signalName),  \
                  ##__VA_ARGS__);   \
      }  \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #signalName #argTypes; \
      };    \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_method_ptr<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          void (cs_class::*)argTypes, \
                                          &cs_class::signalName, \
                                          QMetaMethod::Signal>::placeholder; \
      }

#define CS_SLOT_OVERLOAD(slotName, argTypes) \
      }  \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      {  \
         static constexpr const char *value = #slotName #argTypes; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      {  \
         return cs_record_method_ptr<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                        void (cs_class::*)argTypes, \
                                        &cs_class::slotName, \
                                        QMetaMethod::Slot>::placeholder; \
      }


#define CS_SLOT_OVERLOAD_BOOL(slotName, argTypes) \
      }  \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      {  \
         static constexpr const char *value = #slotName #argTypes; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      {  \
         return cs_record_method_ptr<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                        bool (cs_class::*)argTypes, \
                                        &cs_class::slotName, \
                                        QMetaMethod::Slot>::placeholder; \
      }


#ifndef QT_NO_DEBUG


#define CS_NUMBER_TO_STRING_INTERNAL(number)      #number
#define CS_NUMBER_TO_STRING(number)               CS_NUMBER_TO_STRING_INTERNAL(number)

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
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = name; \
      }; \
   struct CS_TOKENPASTE2(cs_fauxData, __LINE__) \
      { \
         static constexpr const char *value = #data; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_tag<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          CS_TOKENPASTE2(cs_fauxData, __LINE__)>::placeholder; \
      }


// ** properties
#define CS_PROPERTY_READ(name, method) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_read<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::method), \
                                          &cs_class::method, QMetaProperty::READ>::placeholder; \
      }


#define CS_PROPERTY_WRITE(name, method) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_write<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                        decltype(&cs_class::method), \
                                        &cs_class::method, QMetaProperty::WRITE>::placeholder; \
      }


#define CS_PROPERTY_NOTIFY(name, method) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_notify<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::method), \
                                          &cs_class::method, QMetaProperty::NOTIFY>::placeholder; \
      }


#define CS_PROPERTY_RESET(name, method) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_reset<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                        decltype(&cs_class::method), \
                                        &cs_class::method, QMetaProperty::RESET>::placeholder; \
      }


#define CS_PROPERTY_REVISION(name, data) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_int<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          data, QMetaProperty::REVISION>::placeholder; \
      }


#define CS_PROPERTY_DESIGNABLE(name, data) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   static bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()  \
      {  \
         return data; \
      } \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_bool<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), \
                                          &cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__), \
                                          QMetaProperty::DESIGNABLE>::placeholder; \
      }

#define CS_PROPERTY_DESIGNABLE_NONSTATIC(name, data) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)() const \
      {  \
         return data; \
      } \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_bool<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), \
                                          &cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__), \
                                          QMetaProperty::DESIGNABLE>::placeholder; \
      }


#define CS_PROPERTY_SCRIPTABLE(name, data) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   static bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()  \
      {  \
         return data; \
      } \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_bool<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), \
                                          &cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__), \
                                          QMetaProperty::SCRIPTABLE>::placeholder; \
      }


#define CS_PROPERTY_SCRIPTABLE_NONSTATIC(name, data) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)() const \
      {  \
         return data; \
      } \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_bool<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), \
                                          &cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__), \
                                          QMetaProperty::SCRIPTABLE>::placeholder; \
      }


#define CS_PROPERTY_STORED(name, data) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   static bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)()  \
      {  \
         return data; \
      } \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_bool<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), \
                                          &cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__), \
                                          QMetaProperty::STORED>::placeholder; \
      }

#define CS_PROPERTY_STORED_NONSTATIC(name, data) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)() const \
      {  \
         return data; \
      } \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_bool<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), \
                                          &cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__), \
                                          QMetaProperty::STORED>::placeholder; \
      }


#define CS_PROPERTY_USER(name, data) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   static bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)() \
      {  \
         return data; \
      } \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_bool<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), \
                                          &cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__), \
                                          QMetaProperty::USER>::placeholder; \
      }


#define CS_PROPERTY_USER_NONSTATIC(name, data) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   bool CS_TOKENPASTE2(cs_fauxMethod, __LINE__)() const \
      {  \
         return data; \
      } \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_bool<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), \
                                          decltype(&cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__)), \
                                          &cs_class::CS_TOKENPASTE2(cs_fauxMethod, __LINE__), \
                                          QMetaProperty::USER>::placeholder; \
      }


#define CS_PROPERTY_CONSTANT(name) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_int<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), 1, \
                                          QMetaProperty::CONSTANT>::placeholder; \
      }

#define CS_PROPERTY_FINAL(name) \
   struct CS_TOKENPASTE2(cs_fauxName, __LINE__) \
      { \
         static constexpr const char *value = #name; \
      }; \
   int CS_TOKENPASTE2(cs_faux, __LINE__)() \
      { \
         return cs_record_property_int<cs_class, CS_TOKENPASTE2(cs_fauxName, __LINE__), 1, \
                                          QMetaProperty::FINAL>::placeholder; \
      }


// ** macros used when compiling

// ** 1
#ifdef QT_BUILD_CORE_LIB
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
#define CORE_CS_SLOT_1(access, ...)                            __VA_ARGS__;
#define CORE_CS_SLOT_2(slotName)
#define CORE_CS_SLOT_OVERLOAD(slotName, argTypes)

#define CORE_CS_SIGNAL_1(access, ...)                          Q_DECL_IMPORT __VA_ARGS__;
#define CORE_CS_SIGNAL_2(signalName, ...)
#define CORE_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define CORE_CS_INVOKABLE_CONSTRUCTOR_1(access, ...)           __VA_ARGS__;
#define CORE_CS_INVOKABLE_CONSTRUCTOR_2(className, ...)

#define CORE_CS_ENUM(name)
#define CORE_CS_REGISTER_ENUM(...)                             __VA_ARGS__          // do not add ;
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
#ifdef QT_BUILD_GUI_LIB
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
#define GUI_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define GUI_CS_SLOT_2(slotName)
#define GUI_CS_SLOT_OVERLOAD(slotName, argTypes)

#define GUI_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define GUI_CS_SIGNAL_2(signalName, ...)
#define GUI_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define GUI_CS_ENUM(name)
#define GUI_CS_REGISTER_ENUM(...)                                 __VA_ARGS__          // do not add ;
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
#ifdef QT_BUILD_MULTIMEDIA_LIB
#define MULTI_CS_SLOT_1(access, ...)                              CS_SLOT_1(access, __VA_ARGS__)
#define MULTI_CS_SLOT_2(slotName)                                 CS_SLOT_2(slotName)
#define MULTI_CS_SLOT_OVERLOAD(slotName, argTypes)                CS_SLOT_OVERLOAD(slotName, argTypes)

#define MULTI_CS_SIGNAL_1(access, ...)                            CS_SIGNAL_1(access, __VA_ARGS__)
#define MULTI_CS_SIGNAL_2(signalName, ...)                        CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define MULTI_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)       CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define MULTI_CS_SLOT_1(access, ...)                              __VA_ARGS__;
#define MULTI_CS_SLOT_2(slotName)
#define MULTI_CS_SLOT_OVERLOAD(slotName, argTypes)

#define MULTI_CS_SIGNAL_1(access, ...)                            Q_DECL_IMPORT __VA_ARGS__;
#define MULTI_CS_SIGNAL_2(signalName, ...)
#define MULTI_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif


// ** 4
#ifdef QT_BUILD_NETWORK_LIB
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
#define NET_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define NET_CS_SLOT_2(slotName)
#define NET_CS_SLOT_OVERLOAD(slotName, argTypes)

#define NET_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define NET_CS_SIGNAL_2(signalName, ...)
#define NET_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define NET_CS_INVOKABLE_METHOD_1(access, ...)                    __VA_ARGS__;
#define NET_CS_INVOKABLE_METHOD_2(methodName)

#define NET_CS_ENUM(name)
#define NET_CS_REGISTER_ENUM(...)                                 __VA_ARGS__          // do not add ;

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
#ifdef QT_BUILD_OPENGL_LIB
#define OPENGL_CS_SLOT_1(access, ...)                             CS_SLOT_1(access, __VA_ARGS__)
#define OPENGL_CS_SLOT_2(slotName)                                CS_SLOT_2(slotName)
#define OPENGL_CS_SLOT_OVERLOAD(slotName, argTypes)               CS_SLOT_OVERLOAD(slotName, argTypes)

#define OPENGL_CS_SIGNAL_1(access, ...)                           CS_SIGNAL_1(access, __VA_ARGS__)
#define OPENGL_CS_SIGNAL_2(signalName, ...)                       CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define OPENGL_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)      CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define OPENGL_CS_SLOT_1(access, ...)                             __VA_ARGS__;
#define OPENGL_CS_SLOT_2(slotName)
#define OPENGL_CS_SLOT_OVERLOAD(slotName, argTypes)

#define OPENGL_CS_SIGNAL_1(access, ...)                           Q_DECL_IMPORT __VA_ARGS__;
#define OPENGL_CS_SIGNAL_2(signalName, ...)
#define OPENGL_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif


// ** 6
#ifdef QT_BUILD_PHONON_LIB
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
#define PHN_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define PHN_CS_SLOT_2(slotName)
#define PHN_CS_SLOT_OVERLOAD(slotName, argTypes)

#define PHN_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define PHN_CS_SIGNAL_2(signalName, ...)
#define PHN_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define PHN_CS_ENUM(name)
#define PHN_CS_REGISTER_ENUM(...)                                 __VA_ARGS__          // do not add ;
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
#ifdef QT_BUILD_SCRIPT_LIB
#define SCRIPT_CS_SLOT_1(access, ...)                             CS_SLOT_1(access, __VA_ARGS__)
#define SCRIPT_CS_SLOT_2(slotName)                                CS_SLOT_2(slotName)
#define SCRIPT_CS_SLOT_OVERLOAD(slotName, argTypes)               CS_SLOT_OVERLOAD(slotName, argTypes)

#define SCRIPT_CS_SIGNAL_1(access, ...)                           CS_SIGNAL_1(access, __VA_ARGS__)
#define SCRIPT_CS_SIGNAL_2(signalName, ...)                       CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define SCRIPT_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)      CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define SCRIPT_CS_SLOT_1(access, ...)                             __VA_ARGS__;
#define SCRIPT_CS_SLOT_2(slotName)
#define SCRIPT_CS_SLOT_OVERLOAD(slotName, argTypes)

#define SCRIPT_CS_SIGNAL_1(access, ...)                           Q_DECL_IMPORT __VA_ARGS__;
#define SCRIPT_CS_SIGNAL_2(signalName, ...)
#define SCRIPT_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif

// ** 8
#ifdef QT_BUILD_SCRIPTTOOLS_LIB
#define SCRIPT_T_CS_SLOT_1(access, ...)                           CS_SLOT_1(access, __VA_ARGS__)
#define SCRIPT_T_CS_SLOT_2(slotName)                              CS_SLOT_2(slotName)
#define SCRIPT_T_CS_SLOT_OVERLOAD(slotName, argTypes)             CS_SLOT_OVERLOAD(slotName, argTypes)

#define SCRIPT_T_CS_SIGNAL_1(access, ...)                         CS_SIGNAL_1(access, __VA_ARGS__)
#define SCRIPT_T_CS_SIGNAL_2(signalName, ...)                     CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define SCRIPT_T_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)    CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define SCRIPT_T_CS_SLOT_1(access, ...)                           __VA_ARGS__;
#define SCRIPT_T_CS_SLOT_2(slotName)
#define SCRIPT_T_CS_SLOT_OVERLOAD(slotName, argTypes)

#define SCRIPT_T_CS_SIGNAL_1(access, ...)                         Q_DECL_IMPORT __VA_ARGS__;
#define SCRIPT_T_CS_SIGNAL_2(signalName, ...)
#define SCRIPT_T_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif

// ** 9
#ifdef QT_BUILD_SQL_LIB
#define SQL_CS_SLOT_1(access, ...)                                CS_SLOT_1(access, __VA_ARGS__)
#define SQL_CS_SLOT_2(slotName)                                   CS_SLOT_2(slotName)
#define SQL_CS_SLOT_OVERLOAD(slotName, argTypes)                  CS_SLOT_OVERLOAD(slotName, argTypes)

#define SQL_CS_SIGNAL_1(access, ...)                              CS_SIGNAL_1(access, __VA_ARGS__)
#define SQL_CS_SIGNAL_2(signalName, ...)                          CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define SQL_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)         CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define SQL_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define SQL_CS_SLOT_2(slotName)
#define SQL_CS_SLOT_OVERLOAD(slotName, argTypes)

#define SQL_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define SQL_CS_SIGNAL_2(signalName, ...)
#define SQL_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif


// ** 10
#ifdef QT_BUILD_SVG_LIB
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
#ifdef QT_BUILD_XMLPATTERNS_LIB
#define XMLP_CS_SLOT_1(access, ...)                               CS_SLOT_1(access, __VA_ARGS__)
#define XMLP_CS_SLOT_2(slotName)                                  CS_SLOT_2(slotName)
#define XMLP_CS_SLOT_OVERLOAD(slotName, argTypes)                 CS_SLOT_OVERLOAD(slotName, argTypes)

#define XMLP_CS_SIGNAL_1(access, ...)                             CS_SIGNAL_1(access, __VA_ARGS__)
#define XMLP_CS_SIGNAL_2(signalName, ...)                         CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define XMLP_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)        CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define XMLP_CS_SLOT_1(access, ...)                               __VA_ARGS__;
#define XMLP_CS_SLOT_2(slotName)
#define XMLP_CS_SLOT_OVERLOAD(slotName, argTypes)

#define XMLP_CS_SIGNAL_1(access, ...)                             Q_DECL_IMPORT __VA_ARGS__;
#define XMLP_CS_SIGNAL_2(signalName, ...)
#define XMLP_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif

// ** 12
#ifdef BUILDING_WEBKIT
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
#define WEB_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define WEB_CS_SLOT_2(slotName)
#define WEB_CS_SLOT_OVERLOAD(slotName, argTypes)

#define WEB_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define WEB_CS_SIGNAL_2(signalName, ...)
#define WEB_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#define WEB_CS_INVOKABLE_METHOD_1(access, ...)                    __VA_ARGS__;
#define WEB_CS_INVOKABLE_METHOD_2(methodName)

#define WEB_CS_ENUM(name)
#define WEB_CS_REGISTER_ENUM(...)                                 __VA_ARGS__          // do not add ;
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
#ifdef QT_BUILD_DECLARE_LIB
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
#define DECL_CS_REGISTER_ENUM(...)                                __VA_ARGS__          // do not add ;

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
#ifdef QT_BUILD_GSTREAMER_LIB
#define GSTRM_CS_SLOT_1(access, ...)                              CS_SLOT_1(access, __VA_ARGS__)
#define GSTRM_CS_SLOT_2(slotName)                                 CS_SLOT_2(slotName)
#define GSTRM_CS_SLOT_OVERLOAD(slotName, argTypes)                CS_SLOT_OVERLOAD(slotName, argTypes)

#define GSTRM_CS_SIGNAL_1(access, ...)                            CS_SIGNAL_1(access, __VA_ARGS__)
#define GSTRM_CS_SIGNAL_2(signalName, ...)                        CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define GSTRM_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)       CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define GSTRM_CS_SLOT_1(access, ...)                              __VA_ARGS__;
#define GSTRM_CS_SLOT_2(slotName)
#define GSTRM_CS_SLOT_OVERLOAD(slotName, argTypes)

#define GSTRM_CS_SIGNAL_1(access, ...)                            Q_DECL_IMPORT __VA_ARGS__;
#define GSTRM_CS_SIGNAL_2(signalName, ...)
#define GSTRM_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif


// ** 15
#ifdef QT_BUILD_DS9_LIB
#define DS9_CS_SLOT_1(access, ...)                                CS_SLOT_1(access, __VA_ARGS__)
#define DS9_CS_SLOT_2(slotName)                                   CS_SLOT_2(slotName)
#define DS9_CS_SLOT_OVERLOAD(slotName, argTypes)                  CS_SLOT_OVERLOAD(slotName, argTypes)

#define DS9_CS_SIGNAL_1(access, ...)                              CS_SIGNAL_1(access, __VA_ARGS__)
#define DS9_CS_SIGNAL_2(signalName, ...)                          CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define DS9_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)         CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define DS9_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define DS9_CS_SLOT_2(slotName)
#define DS9_CS_SLOT_OVERLOAD(slotName, argTypes)

#define DS9_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define DS9_CS_SIGNAL_2(signalName, ...)
#define DS9_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif


// ** 16
#ifdef QT_BUILD_QT7_LIB
#define QT7_CS_SLOT_1(access, ...)                                CS_SLOT_1(access, __VA_ARGS__)
#define QT7_CS_SLOT_2(slotName)                                   CS_SLOT_2(slotName)
#define QT7_CS_SLOT_OVERLOAD(slotName, argTypes)                  CS_SLOT_OVERLOAD(slotName, argTypes)

#define QT7_CS_SIGNAL_1(access, ...)                              CS_SIGNAL_1(access, __VA_ARGS__)
#define QT7_CS_SIGNAL_2(signalName, ...)                          CS_SIGNAL_2(signalName, ## __VA_ARGS__)
#define QT7_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)         CS_SIGNAL_OVERLOAD(signalName, argTypes, ## __VA_ARGS__)

#else
#define QT7_CS_SLOT_1(access, ...)                                __VA_ARGS__;
#define QT7_CS_SLOT_2(slotName)
#define QT7_CS_SLOT_OVERLOAD(slotName, argTypes)

#define QT7_CS_SIGNAL_1(access, ...)                              Q_DECL_IMPORT __VA_ARGS__;
#define QT7_CS_SIGNAL_2(signalName, ...)
#define QT7_CS_SIGNAL_OVERLOAD(signalName, argTypes, ...)

#endif


#endif // CSOBJECT_MACRO_H
