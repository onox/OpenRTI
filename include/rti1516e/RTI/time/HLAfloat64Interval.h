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
   File: RTI/time/HLAfloat64Interval.h
***********************************************************************/

#ifndef RTI_HLAfloat64Interval_H_
#define RTI_HLAfloat64Interval_H_

#include <RTI/LogicalTimeInterval.h>


namespace rti1516e
{
   class HLAfloat64IntervalImpl;

   class RTI_EXPORT HLAfloat64Interval : public rti1516e::LogicalTimeInterval
   {
   public:

      // Constructors
      HLAfloat64Interval ();

      HLAfloat64Interval (
         double);

      HLAfloat64Interval (
         rti1516e::LogicalTimeInterval const &);

      HLAfloat64Interval (
         const HLAfloat64Interval&);

      // Destructor
      virtual ~HLAfloat64Interval ()
         RTI_NOEXCEPT;

      // Basic accessors/mutators

      virtual void setZero ();

      virtual bool isZero () const;

      virtual void setEpsilon ();

      virtual bool isEpsilon () const;

      // Operators

      virtual rti1516e::LogicalTimeInterval& operator= (
         rti1516e::LogicalTimeInterval const & value)
         RTI_THROW ((rti1516e::InvalidLogicalTimeInterval));

      virtual rti1516e::LogicalTimeInterval& operator+= (
         rti1516e::LogicalTimeInterval const & addend)
         RTI_THROW ((rti1516e::IllegalTimeArithmetic,
                rti1516e::InvalidLogicalTimeInterval));

      virtual rti1516e::LogicalTimeInterval& operator-= (
         rti1516e::LogicalTimeInterval const & subtrahend)
         RTI_THROW ((rti1516e::IllegalTimeArithmetic,
                rti1516e::InvalidLogicalTimeInterval));

      virtual bool operator> (
         rti1516e::LogicalTimeInterval const & value) const
         RTI_THROW ((rti1516e::InvalidLogicalTimeInterval));

      virtual bool operator< (
         rti1516e::LogicalTimeInterval const & value) const
         RTI_THROW ((rti1516e::InvalidLogicalTimeInterval));

      virtual bool operator== (
         rti1516e::LogicalTimeInterval const & value) const
         RTI_THROW ((rti1516e::InvalidLogicalTimeInterval));

      virtual bool operator>= (
         rti1516e::LogicalTimeInterval const & value) const
         RTI_THROW ((rti1516e::InvalidLogicalTimeInterval));

      virtual bool operator<= (
         rti1516e::LogicalTimeInterval const & value) const
         RTI_THROW ((rti1516e::InvalidLogicalTimeInterval));

      // Set self to the difference between two LogicalTimes
      virtual void setToDifference (
         rti1516e::LogicalTime const & minuend,
         rti1516e::LogicalTime const& subtrahend)
         RTI_THROW ((rti1516e::IllegalTimeArithmetic,
                rti1516e::InvalidLogicalTime));

      // Generates an encoded value that can be used to send
      // LogicalTimeIntervals to other federates in updates or interactions
      virtual rti1516e::VariableLengthData encode () const;

      // Alternate encode for directly filling a buffer
      virtual size_t encode (
         void* buffer,
         size_t bufferSize) const
         RTI_THROW ((rti1516e::CouldNotEncode));

      // The length of the encoded data
      virtual size_t encodedLength () const;

      // Decode encodedValue into self
      virtual void decode (
         rti1516e::VariableLengthData const & encodedValue)
         RTI_THROW ((rti1516e::InternalError,
                rti1516e::CouldNotDecode));

      // Alternate decode that reads directly from a buffer
      virtual void decode (
         void* buffer,
         size_t bufferSize)
         RTI_THROW ((rti1516e::InternalError,
                rti1516e::CouldNotDecode));

      // Diagnostic string representation of time
      virtual std::wstring toString () const;

      // Return the name of the Implementation, as needed by
      // createFederationExecution.
      virtual std::wstring implementationName() const;

   public:
      //-----------------------------------------------------------------
      // Implementation functions
      //-----------------------------------------------------------------

      virtual double getInterval () const;

      virtual void setInterval (
         double value);

      //-----------------------------------------------------------------
      // Implementation operators
      //-----------------------------------------------------------------
      virtual HLAfloat64Interval& operator= (
         const HLAfloat64Interval& value)
         RTI_THROW ((rti1516e::InvalidLogicalTimeInterval));

      operator double () const;


   private:

      //-----------------------------------------------------------------
      // Pointer to internal implementation
      //-----------------------------------------------------------------
      HLAfloat64IntervalImpl* _impl;
   };
}

#endif // RTI_HLAfloat64Interval_H_

