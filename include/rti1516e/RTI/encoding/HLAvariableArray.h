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
   File: HLAvariableArray.h
***********************************************************************/
#ifndef RTI_HLAvariableArray_h_
#define RTI_HLAvariableArray_h_

#include <RTI/encoding/DataElement.h>
#include <RTI/encoding/EncodingConfig.h>
#include <RTI/SpecificConfig.h>

namespace rti1516e
{
   // Forward Declarations
   class VariableLengthData;
   class HLAvariableArrayImplementation;

   // Interface for the HLAvariableArray complex data element
   class RTI_EXPORT HLAvariableArray : public rti1516e::DataElement
   {
   public:

      // Constructor which accepts a prototype element
      // that specifies the type of elements to be stored in the array.
      // A clone of the given element works as a seed.
      explicit HLAvariableArray (
         const DataElement& prototype );

      // Copy Constructor
      HLAvariableArray (
         HLAvariableArray const & rhs);

      // Destructor
      virtual ~HLAvariableArray ();

      // Return a new copy of the DataElement
      virtual RTI_UNIQUE_PTR<DataElement> clone () const;

      // Encode this element into a new VariableLengthData
      virtual VariableLengthData encode () const
         RTI_THROW ((EncoderException));

      // Encode this element into an existing VariableLengthData
      virtual void encode (
         VariableLengthData& inData) const
         RTI_THROW ((EncoderException));

      // Encode this element and append it to a buffer
      virtual void encodeInto (
         std::vector<Octet>& buffer) const
         RTI_THROW ((EncoderException));

      // Decode this element from the RTI's VariableLengthData.
      virtual void decode (
         VariableLengthData const & inData)
         RTI_THROW ((EncoderException));

      // Decode this element starting at the index in the provided buffer
      virtual size_t decodeFrom (
         std::vector<Octet> const & buffer,
         size_t index)
         RTI_THROW ((EncoderException));

      // Return the size in bytes of this element's encoding.
      virtual size_t getEncodedLength () const
         RTI_THROW ((EncoderException));

      // Return the octet boundary of this element.
      virtual unsigned int getOctetBoundary () const;

      // Return the number of elements in this variable array.
      virtual size_t size () const;

      // Return true if given element is same type as this; otherwise, false.
      virtual bool isSameTypeAs(
         DataElement const& inData ) const;

      // Return true if given element matches prototype of array.
      virtual bool hasPrototypeSameTypeAs(DataElement const& dataElement ) const;

      // Adds a copy of the given element instance to this array
      // Element must match prototype.
      virtual void addElement (
         const DataElement& dataElement)
         RTI_THROW ((EncoderException));

      // Adds the given element instance to this variable array
      // Element must match prototype.
      // Caller is responsible for ensuring that the external memory is
      // valid for the lifetime of this object or until the indexed element
      // acquires new memory through the set method.
      // Null pointer results in an exception.
      virtual void addElementPointer (
         DataElement* dataElement)
         RTI_THROW ((EncoderException));

      // Sets the indexed element to a copy of the given element instance.
      // Element must match prototype.
      // If indexed element uses external memory, the memory will be modified.
      virtual void set (
         size_t index,
         const DataElement& dataElement)
         RTI_THROW ((EncoderException));

      // Sets the indexed element to the given element instance.
      // Element must match prototype.
      // Null pointer results in an exception.
      // Caller is responsible for ensuring that the external memory is
      // valid for the lifetime of this object or until the indexed element
      // acquires new memory through this call.
      virtual void setElementPointer (
         size_t index,
         DataElement* dataElement)
         RTI_THROW ((EncoderException));

      // Return a const reference to the element instance at the specified index.
      // Must use set to change element.
      virtual const DataElement& get (
         size_t index) const
         RTI_THROW ((EncoderException));

      // Return a const reference to the element instance at the specified index.
      // Must use set to change element.
      DataElement const& operator [](size_t index) const
         RTI_THROW ((EncoderException));
   private:

      // Default Constructor not allowed
      HLAvariableArray ();

      // Assignment Operator not allowed
      HLAvariableArray& operator=(
         HLAvariableArray const & rhs);

   protected:

      HLAvariableArrayImplementation* _impl;
   };
}

#endif // RTI_HLAvariableArray_h_

