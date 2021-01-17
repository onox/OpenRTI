/***********************************************************************
   The IEEE hereby grants a general, royalty-free license to copy, distribute,
   display and make derivative works from this material, for all purposes,
   provided that any use of the material contains the following
   attribution: "Reprinted with permission from IEEE 1516.1(TM)-2010".
   Should you require additional information, contact the Manager, Standards
   Intellectual Property, IEEE Standards Association (stds-ipr@ieee.org).
***********************************************************************/
/***********************************************************************
   IEEE 1516.1 High Level Architecture Interface Specification C++ API
   File: RTI/time/HLAfloat64TimeFactory.h
***********************************************************************/

#ifndef RTI_HLAfloat64TimeFactory_H_
#define RTI_HLAfloat64TimeFactory_H_

#include <RTI/LogicalTimeFactory.h>

namespace rti1516e
{
   class HLAfloat64Time;
   class HLAfloat64Interval;

   // Defines interface for HLAfloat64TimeFactory which presents a
   // floating point-based time/interval representation in the range 0 - 2^63-1.

   const std::wstring HLAfloat64TimeName(L"HLAfloat64Time");

   class RTI_EXPORT HLAfloat64TimeFactory : public rti1516e::LogicalTimeFactory
   {
   public:
      HLAfloat64TimeFactory ();

      virtual ~HLAfloat64TimeFactory ()
         RTI_NOEXCEPT;

      // Return a LogicalTime with the given value
      virtual RTI_UNIQUE_PTR< HLAfloat64Time > makeLogicalTime (
         double value)
         RTI_THROW ((rti1516e::InternalError));

      // Return a LogicalTime with a value of "initial"
      virtual RTI_UNIQUE_PTR< LogicalTime > makeInitial()
         RTI_THROW ((InternalError));

      // Return a LogicalTime with a value of "final"
      virtual RTI_UNIQUE_PTR< LogicalTime > makeFinal()
         RTI_THROW ((InternalError));

      // Return a LogicalTimeInterval with the given value
      virtual RTI_UNIQUE_PTR< HLAfloat64Interval > makeLogicalTimeInterval (
         double value)
         RTI_THROW ((rti1516e::InternalError));

      // Return a LogicalTimeInterval with a value of "zero"
      virtual RTI_UNIQUE_PTR< LogicalTimeInterval > makeZero()
         RTI_THROW ((InternalError));

      // Return a LogicalTimeInterval with a value of "epsilon"
      virtual RTI_UNIQUE_PTR< LogicalTimeInterval > makeEpsilon()
         RTI_THROW ((InternalError));

      // LogicalTime decode from an encoded LogicalTime
      virtual RTI_UNIQUE_PTR< LogicalTime > decodeLogicalTime (
         VariableLengthData const & encodedLogicalTime)
         RTI_THROW ((InternalError,
         CouldNotDecode));

      // Alternate LogicalTime decode that reads directly from a buffer
      virtual RTI_UNIQUE_PTR< LogicalTime > decodeLogicalTime (
         void* buffer,
         size_t bufferSize)
         RTI_THROW ((InternalError,
         CouldNotDecode));

      // LogicalTimeInterval decode from an encoded LogicalTimeInterval
      virtual RTI_UNIQUE_PTR< LogicalTimeInterval > decodeLogicalTimeInterval (
         VariableLengthData const & encodedValue)
         RTI_THROW ((InternalError,
         CouldNotDecode));

      // Alternate LogicalTimeInterval decode that reads directly from a buffer
      virtual RTI_UNIQUE_PTR< LogicalTimeInterval > decodeLogicalTimeInterval (
         void* buffer,
         size_t bufferSize)
         RTI_THROW ((InternalError,
         CouldNotDecode));

      virtual std::wstring getName () const;
   };
}

#endif // RTI_HLAfloat64TimeFactory_H_

