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

/*

  This file exists only to make internal all the classes you see
  below. they are all in the QPatternist namespace, but they don't
  have qdoc documentation because they are all declared in xxx_p.h files. Without these \internal declarations, the class names
  appear on the Inheritance Hierarchy page, which is bad because
  clicking on them brings up the "File not found" page.
 */

#include "qitem_p.h"
#include "qbuiltintypes_p.h"
#include "qitem_p.h"
#include "qschematypefactory_p.h"
#include "qxmlname.h"
#include "qatomictype_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

/*!
  \class AtomicCaster
  \internal
 */

/*!
  \class AtomicComparator
  \internal
 */

/*!
  \class AtomicMathematician
  \internal
 */

/*!
  \class AtomicValue
  \internal
 */

/*!
  \class AbstractDateTime
  \internal
 */

/*!
  \class AbstractDuration
  \internal
 */

/*!
  \class AbstractFloatComparator
  \internal
 */

/*!
  \class AtomicString
  \internal
 */

/*!
  \class Base64Binary
  \internal
 */

/*!
  \class FunctionCall
  \internal
 */

/*!
  \class AddingAggregate
  \internal
 */

/*!
  \class AdjustTimezone
  \internal
 */

/*!
  \class Aggregator
  \internal
 */

/*!
  \class ComparesCaseAware
  \internal
 */

/*!
  \class EncodeString
  \internal
 */

/*!
  \class ExtractFromDateTimeFN
  \internal
 */

/*!
  \class ExtractFromDurationFN
  \internal
 */

/*!
  \class FunctionFactory
  \internal
 */

/*!
  \class Numeric
  \internal
 */

/*!
  \class ReportContext
  \internal
 */

/*!
  \class AbstractFunctionFactory
  \internal
 */

/*!
  \class DynamicContext
  \internal
 */

/*!
  \class IdFN
  \internal
 */

/*!
  \class IntegerType
  \internal
 */

/*!
  \class NamespaceResolver
  \internal
 */

/*!
  \class PatternPlatform
  \internal
 */

/*! \class DocumentProjector
  \internal */
/*! \class NodeBuilder
  \internal */
/*! \class AccelTreeBuilder
  \internal */
/*! \class OutputValidator
  \internal */
/*! \class NetworkLoop
  \internal */
/*! \class QIODeviceDelegate
  \internal */
/*! \class URILoader
  \internal */
/*! \class AbsFN
  \internal */
/*! \class AbstractDateTimeComparator
  \internal */
/*! \class AbstractDateTimeMathematician
  \internal */
/*! \class AbstractDateTimeToDateCaster
  \internal */
/*! \class AbstractDateTimeToDateTimeCaster
  \internal */
/*! \class AbstractDateTimeToGDayCaster
  \internal */
/*! \class AbstractDateTimeToGMonthCaster
  \internal */
/*! \class AbstractDateTimeToGMonthDayCaster
  \internal */
/*! \class AbstractDateTimeToGYearCaster
  \internal */
/*! \class AbstractDateTimeToGYearMonthCaster
  \internal */
/*! \class AbstractDateTimeToTimeCaster
  \internal */
/*! \class AbstractDurationComparator
  \internal */
/*! \class AbstractDurationToDayTimeDurationCaster
  \internal */
/*! \class AbstractDurationToDurationCaster
  \internal */
/*! \class AbstractDurationToYearMonthDurationCaster
  \internal */
/*! \class AbstractFloat
  \internal */
/*! \class AbstractFloatMathematician
  \internal */
/*! \class AbstractFloatSortComparator
  \internal */
/*! \class ConstructorFunctionsFactory
  \internal */
/*! \class XPath10CoreFunctions
  \internal */
/*! \class XPath20CoreFunctions
  \internal */
/*! \class AdjustDateTimeToTimezoneFN
  \internal */
/*! \class AdjustDateToTimezoneFN
  \internal */
/*! \class AdjustTimeToTimezoneFN
  \internal */
/*! \class AnyToDerivedStringCaster
  \internal */
/*! \class AnyURI
  \internal */
/*! \class Atomizer
  \internal */
/*! \class AvgFN
  \internal */
/*! \class Base64BinaryComparatorLocator
  \internal */
/*! \class Base64BinaryToHexBinaryCaster
  \internal */
/*! \class Base64BinaryType
  \internal */
/*! \class BaseURIFN
  \internal */
/*! \class BinaryDataComparator
  \internal */
/*! \class Boolean
  \internal */
/*! \class BooleanComparator
  \internal */
/*! \class BooleanComparatorLocator
  \internal */
/*! \class BooleanFN
  \internal */
/*! \class BooleanToAbstractFloatCaster
  \internal */
/*! \class BooleanToDecimalCaster
  \internal */
/*! \class BooleanToDerivedIntegerCaster
  \internal */
/*! \class BooleanToIntegerCaster
  \internal */
/*! \class BooleanType
  \internal */
/*! \class BuiltinNodeType
  \internal */
/*! \class CardinalityVerifier
  \internal */
/*! \class CaseInsensitiveStringComparator
  \internal */
/*! \class CeilingFN
  \internal */
/*! \class CodepointEqualFN
  \internal */
/*! \class CodepointsToStringFN
  \internal */
/*! \class CollectionFN
  \internal */
/*! \class CompareFN
  \internal */
/*! \class ComparingAggregator
  \internal */
/*! \class ConcatFN
  \internal */
/*! \class ContainsFN
  \internal */
/*! \class CountFN
  \internal */
/*! \class CurrentDateFN
  \internal */
/*! \class CurrentDateTimeFN
  \internal */
/*! \class CurrentTimeFN
  \internal */
/*! \class Date
  \internal */
/*! \class DateComparatorLocator
  \internal */
/*! \class DateMathematicianLocator
  \internal */
/*! \class DateTime
  \internal */
/*! \class DateTimeComparatorLocator
  \internal */
/*! \class DateTimeDurationMathematician
  \internal */
/*! \class DateTimeFN
  \internal */
/*! \class DateTimeMathematicianLocator
  \internal */
/*! \class DateTimeType
  \internal */
/*! \class DateType
  \internal */
/*! \class DayFromAbstractDateTimeFN
  \internal */
/*! \class DayTimeDuration
  \internal */
/*! \class DayTimeDurationComparatorLocator
  \internal */
/*! \class DayTimeDurationMathematicianLocator
  \internal */
/*! \class DayTimeDurationType
  \internal */
/*! \class DaysFromDurationFN
  \internal */
/*! \class Decimal
  \internal */
/*! \class DecimalComparator
  \internal */
/*! \class DecimalComparatorLocator
  \internal */
/*! \class DecimalMathematician
  \internal */
/*! \class DecimalMathematicianLocator
  \internal */
/*! \class DecimalType
  \internal */
/*! \class DeduplicateIterator
  \internal */
/*! \class DeepEqualFN
  \internal */
/*! \class DefaultCollationFN
  \internal */
/*! \class DerivedInteger
  \internal */
/*! \class DerivedString
  \internal */
/*! \class DistinctValuesFN
  \internal */
/*! \class DocAvailableFN
  \internal */
/*! \class DocFN
  \internal */
/*! \class DocumentURIFN
  \internal */
/*! \class DoubleComparatorLocator
  \internal */
/*! \class DoubleMathematicianLocator
  \internal */
/*! \class DoubleType
  \internal */
/*! \class Duration
  \internal */
/*! \class DurationComparatorLocator
  \internal */
/*! \class DurationDurationDivisor
  \internal */
/*! \class DurationDurationMathematician
  \internal */
/*! \class DurationNumericMathematician
  \internal */
/*! \class DurationType
  \internal */
/*! \class DelegatingDynamicContext
  \internal */
/*! \class Focus
  \internal */
/*! \class ReceiverDynamicContext
  \internal */
/*! \class EmptyIterator
  \internal */
/*! \class EncodeForURIFN
  \internal */
/*! \class EndsWithFN
  \internal */
/*! \class ErrorFN
  \internal */
/*! \class EscapeHtmlURIFN
  \internal */
/*! \class EvaluationCache
  \internal */
/*! \class Existence
  \internal */
/*! \class ExpressionVisitor
  \internal */
/*! \class ExpressionVisitorResult
  \internal */
/*! \class FalseFN
  \internal */
/*! \class FloatComparatorLocator
  \internal */
/*! \class FloatMathematicianLocator
  \internal */
/*! \class FloatType
  \internal */
/*! \class FloorFN
  \internal */
/*! \class FunctionArgument
  \internal */
/*! \class FunctionAvailableFN
  \internal */
/*! \class FunctionFactoryCollection
  \internal */
/*! \class FunctionSignature
  \internal */
/*! \class GDay
  \internal */
/*! \class GDayComparatorLocator
  \internal */
/*! \class GDayType
  \internal */
/*! \class GMonth
  \internal */
/*! \class GMonthComparatorLocator
  \internal */
/*! \class GMonthDay
  \internal */
/*! \class GMonthDayComparatorLocator
  \internal */
/*! \class GMonthDayType
  \internal */
/*! \class GMonthType
  \internal */
/*! \class GYear
  \internal */
/*! \class GYearComparatorLocator
  \internal */
/*! \class GYearMonth
  \internal */
/*! \class GYearMonthComparatorLocator
  \internal */
/*! \class GYearMonthType
  \internal */
/*! \class GYearType
  \internal */
/*! \class HexBinary
  \internal */
/*! \class HexBinaryComparatorLocator
  \internal */
/*! \class HexBinaryToBase64BinaryCaster
  \internal */
/*! \class HexBinaryType
  \internal */
/*! \class HoursFromAbstractDateTimeFN
  \internal */
/*! \class HoursFromDurationFN
  \internal */
/*! \class IdrefFN
  \internal */
/*! \class ImplicitTimezoneFN
  \internal */
/*! \class InScopePrefixesFN
  \internal */
/*! \class IndexOfFN
  \internal */
/*! \class InsertBeforeFN
  \internal */
/*! \class Integer
  \internal */
/*! \class IntegerComparator
  \internal */
/*! \class IntegerComparatorLocator
  \internal */
/*! \class IntegerMathematician
  \internal */
/*! \class IntegerMathematicianLocator
  \internal */
/*! \class DerivedIntegerType
  \internal */
/*! \class IriToURIFN
  \internal */
/*! \class ItemMappingIterator
  \internal */
/*! \class ItemVerifier
  \internal */
/*! \class LangFN
  \internal */
/*! \class LastFN
  \internal */
/*! \class LocalNameFN
  \internal */
/*! \class LocalNameFromQNameFN
  \internal */
/*! \class LowerCaseFN
  \internal */
/*! \class MinutesFromAbstractDateTimeFN
  \internal */
/*! \class MinutesFromDurationFN
  \internal */
/*! \class MonthFromAbstractDateTimeFN
  \internal */
/*! \class MonthsFromDurationFN
  \internal */
/*! \class NOTATIONType
  \internal */
/*! \class NameFN
  \internal */
/*! \class NamePool
  \internal */
/*! \class DelegatingNamespaceResolver
  \internal */
/*! \class GenericNamespaceResolver
  \internal */
/*! \class NodeNamespaceResolver
  \internal */
/*! \class NamespaceURIFN
  \internal */
/*! \class NamespaceURIForPrefixFN
  \internal */
/*! \class NamespaceURIFromQNameFN
  \internal */
/*! \class NilledFN
  \internal */
/*! \class NodeNameFN
  \internal */
/*! \class NormalizeSpaceFN
  \internal */
/*! \class NormalizeUnicodeFN
  \internal */
/*! \class NotFN
  \internal */
/*! \class NumberFN
  \internal */
/*! \class NumericToAbstractFloatCaster
  \internal */
/*! \class NumericToBooleanCaster
  \internal */
/*! \class NumericToDecimalCaster
  \internal */
/*! \class NumericToDerivedIntegerCaster
  \internal */
/*! \class OperandSwitcherMathematician
  \internal */
/*! \class ParserContext
  \internal */
/*! \class MatchesFN
  \internal */
/*! \class ReplaceFN
  \internal */
/*! \class TokenizeFN
  \internal */
/*! \class PositionFN
  \internal */
/*! \class PrefixFromQNameFN
  \internal */
/*! \class AccelTree
  \internal */
/*! \class QNameComparator
  \internal */
/*! \class QNameComparatorLocator
  \internal */
/*! \class QNameFN
  \internal */
/*! \class QNameType
  \internal */
/*! \class QNameValue
  \internal */
/*! \class RemoveFN
  \internal */
/*! \class ResolveQNameFN
  \internal */
/*! \class ResolveURIFN
  \internal */
/*! \class ResourceLoader
  \internal */
/*! \class AccelTreeResourceLoader
  \internal */
/*! \class ReverseFN
  \internal */
/*! \class RootFN
  \internal */
/*! \class RoundFN
  \internal */
/*! \class RoundHalfToEvenFN
  \internal */
/*! \class SchemaTime
  \internal */
/*! \class SchemaTimeComparatorLocator
  \internal */
/*! \class SchemaTimeMathematicianLocator
  \internal */
/*! \class SchemaTimeType
  \internal */
/*! \class SecondsFromAbstractDateTimeFN
  \internal */
/*! \class SecondsFromDurationFN
  \internal */
/*! \class SelfToSelfCaster
  \internal */
/*! \class SequenceMappingIterator
  \internal */
/*! \class SingletonIterator
  \internal */
/*! \class SortTuple
  \internal */
/*! \class StartsWithFN
  \internal */
/*! \class StaticBaseURIFN
  \internal */
/*! \class StaticContext
  \internal */
/*! \class DelegatingStaticContext
  \internal */
/*! \class StaticFocusContext
  \internal */
/*! \class StaticNamespaceContext
  \internal */
/*! \class GenericStaticContext
  \internal */
/*! \class StringComparator
  \internal */
/*! \class StringComparatorLocator
  \internal */
/*! \class StringFN
  \internal */
/*! \class StringJoinFN
  \internal */
/*! \class StringLengthFN
  \internal */
/*! \class StringToAbstractFloatCaster
  \internal */
/*! \class StringToBase64BinaryCaster
  \internal */
/*! \class StringToBooleanCaster
  \internal */
/*! \class StringToCodepointsFN
  \internal */
/*! \class StringToDateCaster
  \internal */
/*! \class StringToDateTimeCaster
  \internal */
/*! \class StringToDayTimeDurationCaster
  \internal */
/*! \class StringToDecimalCaster
  \internal */
/*! \class StringToDerivedIntegerCaster
  \internal */
/*! \class StringToDurationCaster
  \internal */
/*! \class StringToGDayCaster
  \internal */
/*! \class StringToGMonthCaster
  \internal */
/*! \class StringToGMonthDayCaster
  \internal */
/*! \class StringToGYearCaster
  \internal */
/*! \class StringToGYearMonthCaster
  \internal */
/*! \class StringToHexBinaryCaster
  \internal */
/*! \class StringToIntegerCaster
  \internal */
/*! \class StringToTimeCaster
  \internal */
/*! \class StringToYearMonthDurationCaster
  \internal */
/*! \class StringType
  \internal */
/*! \class DerivedStringType
  \internal */
/*! \class SubsequenceFN
  \internal */
/*! \class SubstringAfterFN
  \internal */
/*! \class SubstringBeforeFN
  \internal */
/*! \class SubstringFN
  \internal */
/*! \class SumFN
  \internal */
/*! \class SystemPropertyFN
  \internal */
/*! \class TimezoneFromAbstractDateTimeFN
  \internal */
/*! \class ToAnyURICaster
  \internal */
/*! \class ToAnyURICasterLocator
  \internal */
/*! \class ToBase64BinaryCasterLocator
  \internal */
/*! \class ToBooleanCasterLocator
  \internal */
/*! \class ToDateCasterLocator
  \internal */
/*! \class ToDateTimeCasterLocator
  \internal */
/*! \class ToDayTimeDurationCasterLocator
  \internal */
/*! \class ToDecimalCasterLocator
  \internal */
/*! \class ToDoubleCasterLocator
  \internal */
/*! \class ToDurationCasterLocator
  \internal */
/*! \class ToFloatCasterLocator
  \internal */
/*! \class ToGDayCasterLocator
  \internal */
/*! \class ToGMonthCasterLocator
  \internal */
/*! \class ToGMonthDayCasterLocator
  \internal */
/*! \class ToGYearCasterLocator
  \internal */
/*! \class ToGYearMonthCasterLocator
  \internal */
/*! \class ToHexBinaryCasterLocator
  \internal */
/*! \class ToIntegerCasterLocator
  \internal */
/*! \class ToDerivedIntegerCasterLocator
  \internal */
/*! \class ToQNameCasterLocator
  \internal */
/*! \class ToSchemaTimeCasterLocator
  \internal */
/*! \class ToStringCaster
  \internal */
/*! \class ToStringCasterLocator
  \internal */
/*! \class ToDerivedStringCasterLocator
  \internal */
/*! \class ToUntypedAtomicCaster
  \internal */
/*! \class ToUntypedAtomicCasterLocator
  \internal */
/*! \class ToYearMonthDurationCasterLocator
  \internal */
/*! \class Tokenizer
  \internal */
/*! \class XQueryTokenizer
  \internal */
/*! \class TraceFN
  \internal */
/*! \class TranslateFN
  \internal */
/*! \class TrueFN
  \internal */
/*! \class UntypedAtomic
  \internal */
/*! \class UntypedAtomicConverter
  \internal */
/*! \class ArgumentConverter
  \internal */
/*! \class UntypedAtomicType
  \internal */
/*! \class UpperCaseFN
  \internal */
/*! \class ValidationError
  \internal */
/*! \class VariableDeclaration
  \internal */
/*! \class VariableLoader
  \internal */
/*! \class YearFromAbstractDateTimeFN
  \internal */
/*! \class YearMonthDuration
  \internal */
/*! \class YearMonthDurationComparatorLocator
  \internal */
/*! \class YearMonthDurationMathematicianLocator
  \internal */
/*! \class YearMonthDurationType
  \internal */
/*! \class YearsFromDurationFN
  \internal
 */
/*! \class DocumentContentValidator
  \internal */
/*! \class AtomicTypeVisitor
  \internal */
/*! \class AtomicCasterLocator
  \internal */
/*! \class AtomicTypeVisitorResult
  \internal */
/*! \class Expression
  \internal */
/*! \class EmptyContainer
  \internal */
/*! \class AxisStep
  \internal */
/*! \class ContextItem
  \internal */
/*! \class EmptySequence
  \internal */
/*! \class ExternalVariableReference
  \internal */
/*! \class Literal
  \internal */
/*! \class LiteralSequence
  \internal */
/*! \class NamespaceConstructor
  \internal */
/*! \class ParentNodeAxis
  \internal */
/*! \class VariableReference
  \internal */
/*! \class ArgumentReference
  \internal */
/*! \class ExpressionVariableReference
  \internal */
/*! \class PositionalVariableReference
  \internal */
/*! \class RangeVariableReference
  \internal */
/*! \class PairContainer
  \internal */
/*! \class AndExpression
  \internal */
/*! \class OrExpression
  \internal */
/*! \class ArithmeticExpression
  \internal */
/*! \class UnaryExpression
  \internal */
/*! \class AttributeConstructor
  \internal */
/*! \class CombineNodes
  \internal */
/*! \class ElementConstructor
  \internal */
/*! \class ForClause
  \internal */
/*! \class GeneralComparison
  \internal */
/*! \class GenericPredicate
  \internal */
/*! \class TruthPredicate
  \internal */
/*! \class LetClause
  \internal */
/*! \class NodeComparison
  \internal */
/*! \class Path
  \internal */
/*! \class ProcessingInstructionConstructor
  \internal */
/*! \class QuantifiedExpression
  \internal */
/*! \class RangeExpression
  \internal */
/*! \class ValueComparison
  \internal */
/*! \class SingleContainer
  \internal */
/*! \class AttributeNameValidator
  \internal */
/*! \class CastAs
  \internal */
/*! \class CastableAs
  \internal */
/*! \class CollationChecker
  \internal */
/*! \class CommentConstructor
  \internal */
/*! \class CopyOf
  \internal */
/*! \class DocumentConstructor
  \internal */
/*! \class DynamicContextStore
  \internal */
/*! \class FirstItemPredicate
  \internal */
/*! \class InstanceOf
  \internal */
/*! \class NCNameConstructor
  \internal */
/*! \class NodeSortExpression
  \internal */
/*! \class OrderBy
  \internal */
/*! \class QNameConstructor
  \internal */
/*! \class SimpleContentConstructor
  \internal */
/*! \class TextNodeConstructor
  \internal */
/*! \class TreatAs
  \internal */
/*! \class TripleContainer
  \internal */
/*! \class IfThenClause
  \internal */
/*! \class UnlimitedContainer
  \internal */
/*! \class ExpressionSequence
  \internal */
/*! \class ReturnOrderBy
  \internal */
/*! \class UserFunctionCallsite
  \internal */
/*! \class ExpressionCreator
  \internal */
/*! \class ByIDCreator
  \internal */
/*! \class ExpressionFactory
  \internal */
/*! \class ExpressionIdentifier
  \internal */
/*! \class BooleanIdentifier
  \internal */
/*! \class ByIDIdentifier
  \internal */
/*! \class BySequenceTypeIdentifier
  \internal */
/*! \class ComparisonIdentifier
  \internal */
/*! \class IntegerIdentifier
  \internal */
/*! \class ExternalVariableLoader
  \internal */
/*! \class ItemType
  \internal */
/*! \class AnyItemType
  \internal */
/*! \class AnyNodeType
  \internal */
/*! \class AbstractNodeTest
  \internal */
/*! \class LocalNameTest
  \internal */
/*! \class NamespaceNameTest
  \internal */
/*! \class QNameTest
  \internal */
/*! \class AtomicType
  \internal */
/*! \class BuiltinAtomicType
  \internal */
/*! \class AnyAtomicType
  \internal */
/*! \class AnyURIType
  \internal */
/*! \class NumericType
  \internal */
/*! \class EBVType
  \internal */
/*! \class EmptySequenceType
  \internal */
/*! \class MultiItemType
  \internal */
/*! \class NoneType
  \internal */
/*! \class OptimizationPass
  \internal */
/*! \class ParameterizedAtomicTypeVisitor
  \internal */
/*! \class AtomicComparatorLocator
  \internal */
/*! \class AtomicMathematicianLocator
  \internal */
/*! \class SchemaComponent
  \internal */
/*! \class SchemaType
  \internal */
/*! \class AnyType
  \internal */
/*! \class AnySimpleType
  \internal */
/*! \class Untyped
  \internal */
/*! \class SchemaTypeFactory
  \internal */
/*! \class BasicTypesFactory
  \internal */
/*! \class SequenceType
  \internal */
/*! \class GenericSequenceType
  \internal */
/*! \class UserFunction
  \internal
*/
QT_END_NAMESPACE
