/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

// include first, do not move
#include <qmetaobject.h>

#include <qcoreapplication.h>
#include <qthread_p.h>
#include <qstringlist.h>
#include <qstringparser.h>

int QMetaObject::classInfoOffset() const
{
   int retval = 0;
   const QMetaObject *obj = this->superClass();

   if (obj) {
      retval = obj->classInfoCount();
   }

   return retval;
}

bool QMetaObject::checkConnectArgs(const QString &signal, const QString &method)
{
   bool retval = false;

   QString::const_iterator s1 = signal.begin();
   QString::const_iterator s2 = method.begin();

   while (*s1++ != '(') {
      // scan to first '('
   }

   while (*s2++ != '(') {
   }

   if (*s2 == ')' || QStringView8(s1, signal.end()) == QStringView8(s2, method.end()) )   {
      // method has no args or exact match
      retval = true;

   } else  {
      QStringView8 x1 = QStringView8(s1, signal.end());
      QStringView8 x2 = QStringView8(s2, method.end() - 1);

      if (x1.startsWith(x2) && x1[x2.size()] == ',') {
         // method has less args
         retval = true;
      }
   }

   return retval;
}

bool QMetaObject::checkConnectArgs(const QMetaMethod &signal, const QMetaMethod &slot)
{
   bool retval = true;

   QList<QString> typesSignal = signal.parameterTypes();
   QList<QString> typesSlot   = slot.parameterTypes();

   if (typesSignal.count() < typesSlot.count() )  {
      retval = false;

   } else {

      for (int index = 0; index != typesSlot.count(); ++index)   {

         if (typesSignal.at(index) != typesSlot.at(index)) {
            // unable to test if typeDefs are used
            retval = false;
            break;
         }
      }
   }

   return retval;
}

void QMetaObject::connectSlotsByName(QObject *receiver)
{
   if (! receiver)  {
      return;
   }

   const QMetaObject *metaObj = receiver->metaObject();

   // list of elements or children
   const QList<QObject *> list = receiver->findChildren<QObject *>();

   for (int slotIndex = 0; slotIndex < metaObj->methodCount(); ++slotIndex) {

      QMetaMethod slotMethod = metaObj->method(slotIndex);
      const QString &slotName = slotMethod.methodSignature();

      if (! slotName.startsWith("on_")) {
         continue;
      }

      QStringView8 realSlotName(slotName.midView(3));
      bool isConnected = false;

      // walk element list
      for (int j = 0; j < list.count(); ++j) {

         const QObject *element = list.at(j);

         // name of the element
         const QString &objName = element->objectName();
         int len = objName.length();

         // on_myButton_clicked
         if (objName.isEmpty() || ! realSlotName.startsWith(objName) || realSlotName[len] != '_') {
            continue;
         }

         // get the signal name from the slot name
         const QStringView8 desiredSignal = realSlotName.mid(len + 1);

         // does this element have a corresponding signal
         const QMetaObject *elem_metaObj = element->metaObject();

         for (int x = 0; x < elem_metaObj->methodCount(); ++x) {
            QMetaMethod signalMethod = elem_metaObj->method(x);

            if (signalMethod.methodType() != QMetaMethod::Signal) {
               continue;
            }

            // get signal name for the widget/element
            const QString &testSignal = signalMethod.methodSignature();

            if (desiredSignal == testSignal)  {
               // found a matching signal for our slot
               QMetaMethod slotMethod = metaObj->method(slotIndex);

               isConnected = QObject::connect(element, signalMethod, receiver, slotMethod);

               if (isConnected) {
                  // connection set
                  break;
               }
            }
         }

      }

      if (isConnected) {
         // found a slot, skip all clones (defaults)
         int slotCount = metaObj->methodCount();

         while (slotIndex < slotCount - 1)  {

            if ( ! (metaObj->method(slotIndex + 1).attributes() & QMetaMethod::Cloned) ) {
               break;
            }

            // cloned Slot ignored
            ++slotIndex;
         }

      } else if (! (metaObj->method(slotIndex).attributes() & QMetaMethod::Cloned)) {
         qWarning("QMetaObject::connectSlotsByName() No matching signal for: %s::%s",
                  csPrintable(metaObj->className()), csPrintable(slotName));

      }
   }
}

int QMetaObject::enumeratorOffset() const
{
   int retval = 0;
   const QMetaObject *obj = this->superClass();

   if (obj) {
      retval = obj->enumeratorCount();
   }

   return retval;
}

int QMetaObject::indexOfClassInfo(const QString &name) const
{
   int retval = -1;

   for (int index = 0; index < this->classInfoCount(); ++index) {
      QMetaClassInfo testClass = this->classInfo(index);

      if (testClass.name() == name)  {
         retval = index;
         break;
      }
   }

   return retval;
}

int QMetaObject::indexOfConstructor(const QString &constructor) const
{
   int retval = -1;

   // adjust spacing in the passed method name
   QString tValue = constructor;
   tValue.remove(char(32));

   for (int index = 0; index < this->constructorCount(); ++index) {
      const QMetaMethod &testMethod = this->constructor(index);

      if (testMethod.methodSignature() == tValue)  {

         if (testMethod.methodType() == QMetaMethod::Constructor) {
            retval = index;
            break;
         }
      }
   }

   return retval;
}

int QMetaObject::indexOfEnumerator(const QString &name) const
{
   int retval = -1;

   for (int index = 0; index < this->enumeratorCount(); ++index) {
      QMetaEnum testEnum = this->enumerator(index);

      if (testEnum.name() == name)  {
         retval = index;
         break;
      }
   }

   return retval;
}

int QMetaObject::indexOfMethod(const QString &method) const
{
   int retval = -1;

   // adjust spacing in the passed method name
   QString tValue = method;
   tValue.remove(' ');

   for (int index = 0; index < this->methodCount(); ++index) {
      const QMetaMethod &testMethod = this->method(index);

      if (testMethod.methodSignature() == tValue)  {
         retval = index;
         break;
      }
   }

   return retval;
}

int QMetaObject::indexOfMethod(const CsSignal::Internal::BentoAbstract &temp) const
{
   int retval = -1;

   for (int index = 0; index < this->methodCount(); ++index)  {
      QMetaMethod metaMethod = this->method(index);

      bool ok = metaMethod.compare(temp);

      if (ok) {
         retval = index;
         break;
      }
   }

   return retval;
}

int QMetaObject::indexOfProperty(const QString &name) const
{
   int retval = -1;

   for (int x = 0; x < this->propertyCount(); ++x) {
      QMetaProperty testProperty = this->property(x);

      if (testProperty.name() == name)  {
         retval = x;
         break;
      }
   }

   return retval;
}

int QMetaObject::indexOfSignal(const QString &signal) const
{
   int retval = -1;

   // adjust spacing in the passed method name
   QString tValue = signal;
   tValue.remove(' ');
   tValue.chop(1);

   for (int index = 0; index < this->methodCount(); ++index) {
      const QMetaMethod &testMethod = this->method(index);

      // NOTE: will match a method with the same or less arguments than signal

      if (testMethod.methodSignature().startsWith(tValue))  {

         if (testMethod.methodType() == QMetaMethod::Signal) {
            retval = index;
            break;
         }
      }
   }

   return retval;
}

int QMetaObject::indexOfSlot(const QString &slot) const
{
   int retval = -1;

   // adjust spacing in the passed method name
   QString tValue = slot;
   tValue.remove(' ');

   for (int index = 0; index < this->methodCount(); ++index) {
      const QMetaMethod &testMethod = this->method(index);

      if (testMethod.methodSignature() == tValue)  {

         if (testMethod.methodType() == QMetaMethod::Slot) {
            retval = index;
            break;
         }
      }
   }

   return retval;
}

// internal method wrapping the global enum map
QMap<std::type_index, std::pair<QMetaObject *, QString> > &QMetaObject::m_enumsAll()
{
   static QMap<std::type_index, std::pair<QMetaObject *, QString> > enums_All;
   return enums_All;
}

QMetaMethod QMetaObject::method(const CSBentoAbstract &temp) const
{
   QMetaMethod retval;
   const int count = methodCount();

   for (int index = 0; index < count; ++index)  {
      QMetaMethod metaMethod = method(index);

      bool ok = metaMethod.compare(temp);

      if (ok) {
         // found QMetaMethod match
         retval = metaMethod;
         break;
      }
   }

   return retval;
}

int QMetaObject::methodOffset() const
{
   int retval = 0;
   const QMetaObject *obj = this->superClass();

   if (obj) {
      retval = obj->methodCount();
   }

   return retval;
}

QString QMetaObject::normalizedSignature(const QString &method)
{
   QString result;

   if (method.isEmpty()) {
      return result;
   }

   std::vector<QString> signatures;
   QString typeReturn;
   std::vector<QString> paramNames;

   // no return type was passed
   QString fullMethod = "void " + method;

   std::tie(signatures, typeReturn, paramNames) = getSignatures(fullMethod);
   result = signatures[signatures.size() - 1];

   return result;
}

QString QMetaObject::normalizedType(const QString &type)
{
   QString result;

   if (type.isEmpty()) {
      return result;
   }

   result = getType(type);

   return result;
}

int QMetaObject::propertyOffset() const
{
   int retval = 0;
   const QMetaObject *obj = this->superClass();

   if (obj) {
      retval = obj->propertyCount();
   }

   return retval;
}

QString QMetaObject::tr(const char *text, const char *notes, int n) const
{
   const char *context = csPrintable(className());
   return QCoreApplication::translate(context, text, notes, QCoreApplication::CodecForTr, n);
}

QString QMetaObject::trUtf8(const char *text, const char *notes, int n) const
{
   const char *context = csPrintable(className());
   return QCoreApplication::translate(context, text, notes, QCoreApplication::UnicodeUTF8, n);
}

QMetaProperty QMetaObject::userProperty() const
{
   QMetaProperty retval = QMetaProperty();

   const int lastIndex = this->propertyCount() - 1;

   for (int index = lastIndex; index >= 0; --index) {
      const QMetaProperty prop = property(index);

      if (prop.isUser()) {
         retval = prop;
         break;
      }
   }

   return retval;
}

// ***************

std::tuple<std::vector<QString>, QString, std::vector<QString> > QMetaObject::getSignatures(const QString &fullName)
{
   try {

      std::vector<QString> sigList;
      QString returnType;
      std::vector<QString> paramNames;

      QList<QString> tokens;
      QString item;

      // part 1, decipher tokens from fullName
      for (auto letter : fullName) {

         if (letter.isSpace()) {
            // store the token

            if (! item.isEmpty()) {
               tokens.append(item);
               item.clear();
            }

         } else if (letter == '*' || letter == '&' || letter == '=' || letter == ',' ||
                    letter == '[' || letter == ']' || letter == '(' || letter == ')' ||
                    letter == '{' || letter == '}' || letter == '<' || letter == '>'  )   {

            // store the token
            if (! item.isEmpty()) {
               tokens.append(item);
               item.clear();
            }

            tokens.append(letter);

         } else {
            // tack letter onto the current item
            item += letter;
         }
      }

      if (! item.isEmpty()) {
         tokens.append(item);
      }

      // part 2, parse return type from tokens
      int index = 0;

      QString typeReturn;

      QString word;
      QString nextWord;

      bool isStar = false;

      int angleLevel   = 0;
      int bracketLevel = 0;
      int parenLevel   = 0;

      while (true)  {
         word = tokens[index];

         // A
         if (word == "<") {
            ++angleLevel;

         } else if (word == ">")  {
            --angleLevel;

         } else if (word == "[")  {
            ++bracketLevel;

         } else if (word == "]")  {
            --bracketLevel;

         } else if (word == "(")  {
            ++parenLevel;

         } else if (word == ")")  {
            --parenLevel;

         }


         // B
         if (word == "const")  {
            nextWord = tokens[index + 1];

            if (isStar && nextWord != "*") {
               typeReturn += word;
               ++index;
               break;
            }

         } else if (word == "enum")  {
            // discard

            ++index;
            continue;

         } else if (word == "inline")  {
            // discard

            ++index;
            continue;

         } else if (word == "static")  {
            // discard

            ++index;
            continue;

         } else if (word == "virtual")  {
            // discard

            ++index;
            continue;

         } else if (word == "volatile")  {
            // nothing for now

            typeReturn = typeReturn + word + " ";
            ++index;
            continue;

         } else if (word == "*") {
            isStar   = true;
            nextWord = tokens[index + 1];

            if (nextWord == "*" || nextWord == "const") {
               // do nothing

            } else {
               typeReturn += word;
               ++index;
               break;
            }

         } else if (angleLevel != 0 && word == ">")   {
            typeReturn += word;
            ++index;
            continue;

         } else if (angleLevel != 0 && word == "<")   {
            typeReturn += word;
            ++index;
            continue;

         } else if (bracketLevel != 0 && word == "]") {
            typeReturn += word;
            ++index;
            continue;

         } else if (bracketLevel != 0 && word == "[") {
            typeReturn += word;
            ++index;
            continue;

         } else if (parenLevel != 0 && word == ",") {
            isStar = false;

            typeReturn += word;
            ++index;
            continue;

         } else if (parenLevel != 0 && word == ")") {
            isStar = false;

            typeReturn += word;
            ++index;
            continue;

         } else if (parenLevel != 0 && word == "(") {
            typeReturn += word;
            ++index;
            continue;

         } else {
            nextWord = tokens[index + 1];

            if (nextWord == "*" || nextWord == "&" || nextWord == "<" || nextWord == "[" || nextWord == "(") {
               typeReturn += word;
               ++index;
               continue;

            }  else {
               typeReturn += word;
               ++index;

               if (angleLevel == 0 && bracketLevel == 0 && parenLevel == 0) {
                  break;

               } else {
                  continue;

               }
            }
         }

         typeReturn = typeReturn + word + " ";
         ++index;
      }

      // convert return type to "char *" data type
      returnType = strdup(csPrintable(typeReturn));

      // part 3 parse method name
      QString signature = tokens[index++];

      // part 4, parse signature from tokens
      QString typeArg;

      bool found_funcPtrVar;
      bool isDefaultArg;

      // parse open paren
      QString leftParn = tokens[index++];

      if (leftParn != "(")   {
         qWarning("QMetaObject:getSignature() Unable to parse signature: %s", csPrintable(fullName));
      }

      signature += leftParn;

      bool break_loop  = false;
      bool all_done    = false;
      bool bigArg      = false;
      bool bigArg_test = true;

      for (int k = index; k < tokens.size(); ++k) {
         word = tokens[k];

         if (word == ")")  {
            signature += word;
            break;

         } else {
            found_funcPtrVar = false;

            bigArg        = false;
            bigArg_test   = true;

            isDefaultArg  = false;
            isStar        = false;
            typeArg       = "";

            angleLevel    = 0;
            bracketLevel  = 0;
            parenLevel    = 0;

            while (true)  {
               word = tokens[k];

               // A
               if (word == "<") {
                  ++angleLevel;

               } else if (word == ">")  {
                  --angleLevel;

               } else if (word == "[")  {
                  ++bracketLevel;

               } else if (word == "]")  {
                  --bracketLevel;

               } else if (word == "(")  {
                  ++parenLevel;

               } else if (word == ")")  {
                  --parenLevel;

               }

               // B
               int tokenMax = tokens.size() - 1;

               if (word == "const")  {

                  if (k < tokenMax)  {
                     nextWord = tokens[k + 1];

                  } else {
                     // all done
                     break_loop = true;
                     break ;
                  }

                  if (isStar && nextWord != "*") {
                     typeArg += word;
                     ++k;
                     break;
                  }

               } else if (word == "enum")  {
                  // discard
                  ++k;
                  continue;

               } else if (word == "restrict")  {
                  // discard
                  ++k;
                  continue;

               } else if (word == "volatile")  {
                  // keep

                  typeArg = typeArg + word + " ";
                  ++k;
                  continue;

               } else if (bigArg_test && (word == "signed" || word == "unsigned" || word == "short" || word == "long"))  {

                  bool firstLoop = true;
                  int index = k + 1;

#ifdef CS_Debug
                  qDebug("Debug (bigArg):  Passed full name %s",    fullName );
#endif

                  while (k < tokenMax)  {
                     bool found = false;

                     nextWord = tokens[index];

                     if ((word == "long") && (nextWord == "double" || nextWord == "int" || nextWord == "long")) {
                        found = true;

                     } else if ((word == "short") && (nextWord == "int")) {
                        found = true;

                     } else if (nextWord == "char" || nextWord == "int" || nextWord == "long" || nextWord == "short") {
                        // signed & unsigned
                        found = true;

                     }

                     //
                     if (found) {

                        if (firstLoop) {
                           typeArg += word;
                           ++k;

                           firstLoop   = false;
                           bigArg      = true;

#ifdef CS_Debug
                           qDebug("Debug (bigArg):  Inside 'first loop'  %s", csPrintable(typeArg));
#endif

                        }

                        typeArg += " " + nextWord;
                        ++k;

                        // used for nextWord
                        index = k;

                     }  else {
                        // all done

                        if (firstLoop) {
                           // standard, no doubled type name

                           bigArg_test = false;
                           break;
                        }

                        break;
                     }

                  }

                  if (bigArg) {
                     typeArg += " ";
                  }

                  continue;

               } else if (word == "*") {
                  isStar = true;

                  if (k < tokenMax)  {
                     nextWord = tokens[k + 1];
                  }

                  //
                  if (nextWord == "*" || nextWord == "const") {
                     typeArg += word;
                     ++k;
                     continue;

                  }  else if (parenLevel != 0 && nextWord == ")")  {
                     // function pointer with no variable name
                     typeArg += word;
                     ++k;
                     continue;

                  }  else if (parenLevel != 0 && ! found_funcPtrVar)  {
                     // function pointer, save the var name, then keep going
                     found_funcPtrVar = true;
                     paramNames.push_back(nextWord);

                     typeArg += word;
                     k += 2;
                     continue;

                  } else  {
                     typeArg += word;
                     ++k;

                     if (parenLevel == 0) {
                        break;
                     } else {
                        continue;
                     }

                  }

               } else if (angleLevel != 0 && word == ">")   {
                  typeArg += word;
                  ++k;
                  continue;

               } else if (angleLevel != 0 && word == "<")   {
                  typeArg += word;
                  ++k;
                  continue;

               } else if (bracketLevel != 0 && word == "]") {
                  typeArg += word;
                  ++k;
                  continue;

               } else if (bracketLevel != 0 && word == "[") {
                  typeArg += word;
                  ++k;
                  continue;

               } else if (parenLevel != 0 && word == ",") {
                  isStar = false;

                  typeArg += word;
                  ++k;
                  continue;

               } else if (parenLevel != 0 && word == ")") {
                  isStar = false;

                  typeArg += word;
                  ++k;
                  continue;

               } else if (parenLevel != 0 && word == "(") {
                  typeArg += word;
                  ++k;
                  continue;

               } else {
                  nextWord = tokens[k + 1];

                  if (bigArg)  {
                     // we are on a comma, var name, right paren, star, ampersand

#ifdef CS_Debug
                     // parse the following:   &  *  (  )
                     qDebug("Debug (bigArg):  Args:     %s", csPrintable(typeArg) );
                     qDebug("Debug (bigArg):  NextWord  %s", csPrintable(nextWord) );
#endif

                     break;

                  } else if (nextWord == "*" || nextWord == "&" || nextWord == "<" || nextWord == "[" || nextWord == "(") {
                     typeArg += word;
                     ++k;
                     continue;

                  }  else {
                     typeArg += word;
                     ++k;

                     if (angleLevel == 0 && bracketLevel == 0 && parenLevel == 0) {
                        break;

                     } else {
                        continue;

                     }
                  }
               }

               typeArg = typeArg + word + " ";
               ++k;
            }

            if (break_loop)  {
               break;
            }


            // C save var name
            word = tokens[k];


            // D
            if (found_funcPtrVar) {
               // fall thru

            } else if (word == "=" || word == ")" || word == ",")  {
               // default name
               paramNames.push_back("un_named_arg");

            } else {
               // token is a variable
               paramNames.push_back(word);
               ++k;
            }

            // E next token could be equal, close paren, comma, left square bracket(array)
            int parenLevel_2 = 0;

            while (true) {
               word = tokens[k];

               if (word == "=") {
                  isDefaultArg = true;

               } else if (word == "(" && isDefaultArg)  {
                  ++parenLevel_2;

               } else if (word == ")")  {

                  if (isDefaultArg) {

                     if (parenLevel_2 <= 0) {
                        break;
                     }

                     --parenLevel_2;

                  } else {
                     all_done = true;
                     break;

                  }

               } else if (word == ",")  {
                  if (parenLevel_2 <= 0) {
                     break;
                  }

               } else if (! isDefaultArg) {
                  // add to signature
                  typeArg += word;

               }

               ++k;
            }

            if (isDefaultArg)  {
               // add overloaded signature to the vector
               QString tmp = signature;

               if (tmp.endsWith(",")) {
                  tmp.chop(1);
               }

               tmp += ")";
               sigList.push_back(std::move(tmp));
            }

            signature += typeArg;
         }

         signature += tokens[k];

         if (all_done) {
            break;
         }
      }

#ifdef CS_Debug
      const char *space = "                      ";
      qDebug("QObject:getSignature()  Passed name: %s\n %s Signature: %s \n", fullName, space, csPrintable(signature));
#endif

      // add sig to the vector
      sigList.push_back(std::move(signature));

      return std::make_tuple(sigList, returnType, paramNames);

   }  catch (std::exception &e) {
      // rethrow
      std::string msg = "QObject::getSignature() Exception when processing " + std::string(csPrintable(fullName));
      std::throw_with_nested(std::logic_error(msg));

   }
}


// **
QString QMetaObject::getType(const QString &fullName)
{
   QList<QString> tokens;
   QString    item;

   // part 1, decipher tokens from fullName
   for (auto letter : fullName) {

      if (letter.isSpace()) {
         // store the token

         if (! item.isEmpty()) {
            tokens.append(item);
            item.clear();
         }

      } else if (letter == '*' || letter == '&' || letter == '=' || letter == ',' ||
                 letter == '[' || letter == ']' || letter == '(' || letter == ')' ||
                 letter == '{' || letter == '}' || letter == '<' || letter == '>'  )   {

         // store the token
         if (! item.isEmpty()) {
            tokens.append(item);
            item.clear();
         }

         tokens.append(letter);

      } else {
         // tack letter onto the current item
         item = item + letter;

      }
   }

   if (! item.isEmpty()) {
      tokens.append(item);
   }


   // part 2, parse return type from tokens
   int index = 0;

   QString typeReturn;

   QString word;
   QString nextWord;

   bool isStar = false;

   int angleLevel   = 0;
   int bracketLevel = 0;
   int parenLevel   = 0;

   while (index < tokens.size()) {
      word = tokens[index];

      // A
      if (word == "<") {
         ++angleLevel;

      } else if (word == ">")  {
         --angleLevel;

      } else if (word == "[")  {
         ++bracketLevel;

      } else if (word == "]")  {
         --bracketLevel;

      } else if (word == "(")  {
         ++parenLevel;

      } else if (word == ")")  {
         --parenLevel;

      }


      // B
      if (word == "const")  {

         if (index + 1 >= tokens.size()) {
            typeReturn += word;
            ++index;
            break;
         }

         nextWord = tokens[index + 1];

         if (isStar && nextWord != "*") {
            typeReturn += word;
            ++index;
            break;
         }

      } else if (word == "volatile")  {
         // nothing for now

         typeReturn = typeReturn + word + " ";
         ++index;
         continue;

      } else if (word == "*") {
         isStar = true;

         if (index + 1 >= tokens.size()) {
            typeReturn += word;
            ++index;
            break;
         }

         nextWord = tokens[index + 1];

         if (nextWord == "*" || nextWord == "const") {
            // do nothing

         } else {
            typeReturn += word;
            ++index;
            break;
         }

      } else if (angleLevel != 0 && word == ">")   {
         typeReturn += word;
         ++index;
         continue;

      } else if (angleLevel != 0 && word == "<")   {
         typeReturn += word;
         ++index;
         continue;

      } else if (bracketLevel != 0 && word == "]") {
         typeReturn += word;
         ++index;
         continue;

      } else if (bracketLevel != 0 && word == "[") {
         typeReturn += word;
         ++index;
         continue;

      } else if (parenLevel != 0 && word == ",") {
         isStar = false;

         typeReturn += word;
         ++index;
         continue;

      } else if (parenLevel != 0 && word == ")") {
         isStar = false;

         typeReturn += word;
         ++index;
         continue;

      } else if (parenLevel != 0 && word == "(") {
         typeReturn += word;
         ++index;
         continue;

      } else {
         if (index + 1 >= tokens.size()) {
            nextWord = "";

         } else {
            nextWord = tokens[index + 1];

         }

         if (nextWord == "*" || nextWord == "&" || nextWord == "<" || nextWord == "[" || nextWord == "(") {
            typeReturn += word;
            ++index;
            continue;

         }  else {
            typeReturn += word;
            ++index;

            if (angleLevel == 0 && bracketLevel == 0 && parenLevel == 0) {
               break;

            } else {
               continue;

            }
         }
      }

      typeReturn = typeReturn + word + " ";
      ++index;
   }

   return typeReturn;
}

int QMetaObject::enum_calculate(QString enumData, QMap<QString, int> valueMap)
{
   int retval = 0;

   QList<QString> tokens;
   QString item;

   QString::const_iterator iter = enumData.begin();

   // part 1, decipher tokens from enumData
   while (iter != enumData.end()) {
      QChar32 letter = *iter;

      if (letter.isSpace()) {
         // store the token

         if (! item.isEmpty()) {
            tokens.append(item);
            item.clear();
         }

      } else if (letter == '&' || letter == '|' || letter == '<' || letter == '~' ||
                 letter == '(' || letter == ')' )   {

         if (! item.isEmpty()) {
            // store the token
            tokens.append(item);
            item.clear();
         }

         if (letter == '<' && *(iter + 1) == '<' )  {
            // store the token "<<"
            tokens.append("<<");
            ++iter;

         } else {
            // store the token
            tokens.append(letter);
         }

      } else {
         // tack letter onto the current item
         item = item + letter;

      }

      ++iter;
   }

   if (! item.isEmpty()) {
      tokens.append(item);
   }


   // part 2, convert to rpn
   int tokensSize = tokens.size();
   int index = 0;

   QString word;

   QList<QString> opList;
   QList<QString> vList;

   while (index < tokensSize)  {
      word = tokens[index];

      if (word == "&" || word  == "|" || word == "<<" || word  == "~")  {
         opList.prepend(word);

      } else  {
         vList.append(word);

      }

      ++index;
   }

   tokens.clear();
   tokens.append(vList);
   tokens.append(opList);


   // part 3, decipher tokens from word
   bool ok;
   index = 0;

   QList<int> valueStack;

   while (index < tokensSize)  {
      word = tokens[index];

      QChar32 firstChar = word.at(0);

      // A
      if (firstChar == QChar(39) )  {
         // single quote

         int len = word.size();
         QStringView8 tStr = word.midView(1, len - 2);

         if (tStr.startsWith("\\u"))  {

            tStr = tStr.mid(2);
            int t_value = QStringParser::toInteger<int>(tStr, &ok, 16);

            if (ok) {
               valueStack.append(t_value);
            } else {
               return -1;
            }

         } else if (tStr.startsWith("\\0")) {

            tStr = tStr.mid(2);
            int t_value = QStringParser::toInteger<int>(tStr, &ok, 0);

            if (ok) {
               valueStack.append(t_value);
            } else {
               return -1;
            }

         }  else {
            // single letter
            valueStack.append(tStr.at(0).unicode());

         }

      } else if ( word.contains("::") )        {
         // Ginger::MetaModifier

         int pos = word.indexOf("::");
         QString className = word.mid(0, pos);
         QString enumKey   = word.mid(pos + 2);

         for (auto index = m_enumsAll().begin(); index != m_enumsAll().end(); ++index  )  {

            QMetaObject *obj = index.value().first;

            if ( obj->className() == className )  {
               QString enumName = index.value().second;

               // obtain the enum object
               int x = obj->indexOfEnumerator(enumName);
               QMetaEnum enumObj = obj->enumerator(x);

               int answer = enumObj.keyToValue(enumKey);

               if (answer != -1 ) {
                  valueStack.append(answer);
                  break;
               }

            }
         }

      } else {

         if (firstChar >= '0' && firstChar <= '9')  {
            // strip 8l or 8u to a value of 8
            word = word.remove('l');
            word = word.remove('u');
         }

         int t_value = QStringParser::toInteger<int>(word, &ok, 0);

         if (ok) {
            // token value is an int
            valueStack.append(t_value);

         } else  {
            // look up the value for the enum
            auto index = valueMap.find(word);

            if (index != valueMap.end() )  {
               valueStack.append(index.value());
            }
         }
      }


      // B
      if (word  == "|")  {
         if (valueStack.size() < 2) {
            throw std::logic_error("Unable to parse enum data, check enum macros");
         }

         int right = valueStack.takeLast();
         int left  = valueStack.takeLast();
         valueStack.append(left | right);


      } else if (word  == "<<")  {
         if (valueStack.size() < 2) {
            throw std::logic_error("Unable to parse enum data, check enum macros");
         }

         int right = valueStack.takeLast();
         int left  = valueStack.takeLast();
         valueStack.append(left << right);

      } else if (word  == "~")  {
         if (valueStack.size() < 1) {
            throw std::logic_error("Unable to parse enum data, check enum macros");
         }

         int right = valueStack.takeLast();
         valueStack.append(~right);
      }

      ++index;
   }

   // return the last value
   if (valueStack.count() == 1) {
      retval = valueStack.at(0);
   }

   return retval;
}

// ** internal
void QMetaObject_X::register_classInfo(const QString &name, const QString &value)
{
   if (name.isEmpty()) {
      return;
   }

   QMetaClassInfo data(name, value);
   m_classInfo.insert(name, data);
}

QMetaClassInfo QMetaObject_X::classInfo(int index) const
{
   const int count = m_classInfo.size();

   if (index >= count) {
      // index is out of bounds, look in parent class
      return superClass()->classInfo(index - count);

   }  else {
      auto elem = m_classInfo.end();

      // decrement iterator (end() is really plus 1, sub it out)
      elem -= index + 1;
      return elem.value();
   }
}

int QMetaObject_X::classInfoCount() const
{
   int count = m_classInfo.size();

   if (superClass()) {
      count += superClass()->classInfoCount();
   }

   return count;
}

QMetaMethod QMetaObject_X::constructor(int index) const
{
   const int count = m_constructor.size();

   if (index >= count) {
      // not sure

   }  else {
      auto elem = m_constructor.end();

      // decrement iterator (end() is really plus 1, sub it out)
      elem -= index + 1;
      return elem.value();
   }

   return QMetaMethod {};
}

int QMetaObject_X::constructorCount() const
{
   // number of constructors in this class
   int count = m_constructor.size();

   return count;
}

QMetaEnum QMetaObject_X::enumerator(int index) const
{
   const int count = m_enums.size();

   if (index >= count) {
      // index is out of bounds, look in parent class
      return superClass()->enumerator(index - count);

   }  else {
      auto elem = m_enums.end();

      // decrement iterator (end() is really plus 1, sub it out)
      elem -= index + 1;
      return elem.value();
   }
}

int QMetaObject_X::enumeratorCount() const
{
   int count = m_enums.size();

   if (superClass()) {
      count += superClass()->enumeratorCount();
   }

   return count;
}

QMetaMethod QMetaObject_X::method(int index) const
{
   const int count = m_methods.size();

   if (index >= count) {
      // index is out of bounds, look in parent class
      return superClass()->method(index - count);

   }  else {
      auto elem = m_methods.end();

      // decrement iterator (end() is really plus 1, sub it out)
      elem -= index + 1;
      return elem.value();
   }
}

int QMetaObject_X::methodCount() const
{
   // includes signals, slots, and methods declared with CS_INVOKABLE
   int count = m_methods.size();

   if (superClass()) {
      count += superClass()->methodCount();
   }

   return count;
}

QMetaProperty QMetaObject_X::property(int index) const
{
   const int count = m_properties.size();

   if (index >= count) {
      // index is out of bounds, look in parent class
      return superClass()->property(index - count);

   }  else {
      auto elem = m_properties.end();

      // decrement iterator (end() is really plus 1, sub it out)
      elem -= index + 1;
      return elem.value();
   }
}

int QMetaObject_X::propertyCount() const
{
   int count = m_properties.size();

   if (superClass()) {
      count += superClass()->propertyCount();
   }

   return count;
}

int QMetaObject_X::register_enum(const QString &name, std::type_index id, const QString &scope)
{
   if (name.isEmpty() || scope.isEmpty()) {
      return 0;
   }

   QMetaEnum data(name, scope, false);
   m_enums.insert(name, data);

   // used in findEnum()
   m_enumsAll().insert(id, std::make_pair(this, name) );

   return 0;
}

int QMetaObject_X::register_flag(const QString &enumName, const QString &scope, const QString &flagName, std::type_index id)
{
   if (enumName.isEmpty() || scope.isEmpty() || flagName.isEmpty()) {
      return 0;
   }

   // save for look up with properties
   m_flag.insertMulti(enumName, flagName);

   QMetaEnum data(flagName, scope, true);
   m_enums.insert(flagName, data);

   // used in findEnum()
   m_enumsAll().insert(id, std::make_pair(this, flagName) );

   return 0;
}

void QMetaObject_X::register_enum_data(const QString &args)
{
   if (args.isEmpty()) {
      return;
   }

   QString word;
   bool isName = false;

   QString::const_iterator iter = args.begin();

   // part 1, get the enum name
   while (iter != args.end()) {
      QChar32 letter = *iter;

      if (letter.isSpace()) {

         if (isName)   {
            break;
         }

      } else {
         word = word + letter;

      }

     ++iter;

      // may not handle enum class
      if (word == "enum") {
         isName = true;
         word   = "";

         while (iter->isSpace()) {
            ++iter;
         }
      }
   }

   QString enumName = word;

   // part 2, get the enum data
   QMap<QString, int> valueMap;

   int value = 0;
   word      = "";

   while (iter != args.end()) {
      QChar32 letter = *iter;

      if ((letter.isSpace() || letter == '{')) {
         // move on

      } else if (letter == '}' || letter == ',') {

         int index = word.indexOf("=");

         if (index > 0)  {
            // convert enum value
            QString t_word = word.mid(index + 1);

            bool ok;
            int t_value = QStringParser::toInteger<int>(t_word, &ok, 0);

            if (ok) {
               // value is an int
               value = t_value;

            } else  {
               value = this->enum_calculate(t_word, valueMap);

            }

            word = word.left(index);
         }

         QString key = word;
         valueMap.insert(key, value);
         value++;

         if (letter == '}') {
            break;
         }

         word = "";

      } else {
         word = word + letter;

      }

      ++iter;
   }


   // 3 look up enumName to test, is a flag?
   std::vector<QString> tempName(0);

   auto iter_flag = m_flag.find(enumName);

   if (iter_flag == m_flag.end()) {
      // save the enumName in the QVector
      tempName.push_back(enumName);

   } else {
      while (iter_flag != m_flag.end() && iter_flag.key() == enumName) {
         tempName.push_back(iter_flag.value());
         ++iter_flag;
      }
   }

   // save enum data in QMap
   for (int i = 0; i < tempName.size(); ++i) {
      auto iter_enum = m_enums.find(tempName.at(i));

      if (iter_enum == m_enums.end()) {
         throw std::logic_error("Unable to register enum data, verify enum macros");

      } else  {
         QMetaEnum enumObj = iter_enum.value();
         enumObj.setData(valueMap);

         // update QMetaEnum
         m_enums.insert( tempName.at(i), enumObj);
      }
   }
}

void QMetaObject_X::register_method_s1(const QString &name, QMetaMethod::Access access, QMetaMethod::MethodType kind)
{
   if (name.isEmpty()) {
      return;
   }

   // declare first
   std::vector<QString> signatures;
   QString typeReturn;
   std::vector<QString> paramNames;

   std::tie(signatures, typeReturn, paramNames) = this->getSignatures(name);

   //
   QMetaMethod::Attributes attr = QMetaMethod::Attributes();
   auto count = signatures.size();                              // base method plus number of defaulted parameters

   std::vector<QString> tmpArgNames = paramNames;

   for (std::vector<QString>::size_type k = 0; k < count; ++k)  {

      if (count > 1) {
         // remove defaulted parameter names
         int howMany = paramNames.size() - ((count - 1) - k);
         tmpArgNames = std::vector<QString>(paramNames.begin(), paramNames.begin() + howMany);

         if (k == count - 1) {
            // base method
            attr = QMetaMethod::Attributes();

         }  else {
            attr = QMetaMethod::Cloned;
         }
      }

      // remove spacing from the key
      QString tokenKey = signatures[k];
      tokenKey.remove(' ');

      // save the key/value into the master map
      QMetaMethod data(typeReturn, tokenKey, tmpArgNames, access, kind, attr, this);

      if (kind == QMetaMethod::Constructor) {
         m_constructor.insert(tokenKey, data);
      } else  {
         m_methods.insert(tokenKey, data);
      }
   }
}

void QMetaObject_X::register_tag(const QString &name, const QString &method)
{
   if (name.isEmpty()) {
      return;
   }

   auto item = m_methods.find(method);

   if ( item == m_methods.end() )  {
      // entry not found
      throw std::logic_error("Unable to register method tag, verify signal/slot macros");


   } else {
      // retrieve existing obj
      QMetaMethod obj = item.value();
      obj.setTag(name);

      // update QMetaMethod
      m_methods.insert(method, obj);
   }
}

// ** internal properties
int QMetaObject_X::register_property_read(const QString &name, const QString &dataType, JarReadAbstract *readJar)
{
   if (name.isEmpty() ) {
      return 0;
   }

   QMetaProperty data;
   auto item = m_properties.find(name);

   if ( item == m_properties.end() )  {
      // entry not found in QMap, construct new obj then insert

      data = QMetaProperty {name, this};
      m_properties.insert(name, data);

   } else {
      // retrieve existing obj
      data = item.value();

   }

   //
   data.setReadMethod(dataType, readJar);

   // update QMetaProperty
   m_properties.insert(name, data);

   return 0;
}

int QMetaObject_X::register_property_write(const QString &name, JarWriteAbstract *method)
{
   if (name.isEmpty()) {
      return 0;
   }

   QMetaProperty data;
   auto item = m_properties.find(name);

   if ( item == m_properties.end() )  {
      // entry not found in QMap, construct new obj then insert

      data = QMetaProperty {name, this};
      m_properties.insert(name, data);

   } else {
      // retrieve existing obj
      data = item.value();

   }

   //
   data.setWriteMethod(method);

   // update QMetaProperty
   m_properties.insert(name, data);

   return 0;
}

int QMetaObject_X::register_property_bool(const QString &name, JarReadAbstract *method, QMetaProperty::Kind kind)
{
   if (name.isEmpty()) {
      return 0;
   }

   QMetaProperty data;
   auto item = m_properties.find(name);

   if ( item == m_properties.end() )  {
      // entry not found in QMap, construct new obj then insert

      data = QMetaProperty {name, this};
      m_properties.insert(name, data);

   } else {
      // retrieve existing obj
      data = item.value();

   }

   if (kind == QMetaProperty::DESIGNABLE) {
      data.setDesignable(method);

   } else if (kind == QMetaProperty::SCRIPTABLE) {
      data.setScriptable(method);

   } else if (kind == QMetaProperty::STORED) {
      data.setStored(method);

   } else if (kind == QMetaProperty::USER) {
      data.setUser(method);

   }

   // update QMetaProperty
   m_properties.insert(name, data);

   return 0;
}

void QMetaObject_X::register_property_int(const QString &name, int value, QMetaProperty::Kind kind)
{
   if (name.isEmpty()) {
      return;
   }

   QMetaProperty data;
   auto item = m_properties.find(name);

   if ( item == m_properties.end() )  {
      // entry not found in QMap, construct new obj then insert

      data = QMetaProperty {name, this};
      m_properties.insert(name, data);

   } else {
      // retrieve existing obj
      data = item.value();

   }

   //
   if (kind == QMetaProperty::REVISION) {
      // int value
      data.setRevision(value);

   }
   if (kind == QMetaProperty::CONSTANT) {
      data.setConstant();

   }
   if (kind == QMetaProperty::FINAL) {
      data.setFinal();

   }

   // update QMetaProperty
   m_properties.insert(name, data);
}

