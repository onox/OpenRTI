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
   File: RTI/time/HLAinteger64TimeFactory.h
***********************************************************************/

#ifndef RTI_HLAinteger64TimeFactory_H_
#define RTI_HLAinteger64TimeFactory_H_

#include <RTI/LogicalTimeFactory.h>
#include <RTI/encoding/EncodingConfig.h>


// Defines interface for HLAinteger64TimeFactory which presents an integer-based
// time /interval representation in the range 0 - 2^63-1.

namespace rti1516e
{
   const std::wstring HLAinteger64TimeName(L"HLAinteger64Time");

   class HLAinteger64Time;
   class HLAinteger64Interval;

   class RTI_EXPORT HLAinteger64TimeFactory : public rti1516e::LogicalTimeFactory
   {
   public:
      HLAinteger64TimeFactory ();

      virtual ~HLAinteger64TimeFactory ()
         RTI_NOEXCEPT;

      // Return a LogicalTime with a value of "initial"
      virtual RTI_UNIQUE_PTR< LogicalTime > makeInitial()
         RTI_THROW ((InternalError));

      // Return a LogicalTime with a value of "final"
      virtual RTI_UNIQUE_PTR< LogicalTime > makeFinal()
         RTI_THROW ((InternalError));

      // Return a LogicalTimeInterval with a value of "zero"
      virtual RTI_UNIQUE_PTR< LogicalTimeInterval > makeZero()
         RTI_THROW ((InternalError));

      // Return a LogicalTimeInterval with a value of "epsilon"
      virtual RTI_UNIQUE_PTR< LogicalTimeInterval > makeEpsilon()
         RTI_THROW ((InternalError));

      virtual RTI_UNIQUE_PTR< HLAinteger64Time > makeLogicalTime (
         Integer64 value)
         RTI_THROW ((rti1516e::InternalError));

      virtual RTI_UNIQUE_PTR< HLAinteger64Interval > makeLogicalTimeInterval (
         Integer64 value)
         RTI_THROW ((rti1516e::InternalError));

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

#endif // RTI_HLAinteger64TimeFactory_H_

