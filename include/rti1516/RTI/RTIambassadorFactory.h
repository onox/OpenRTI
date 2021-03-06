/***********************************************************************
  IEEE 1516.1 High Level Architecture Interface Specification C++ API
  File: RTI/RTIambassadorFactory.h
***********************************************************************/

#ifndef RTI_RTIambassadorFactory_h
#define RTI_RTIambassadorFactory_h

namespace rti1516
{
  class RTIambassador;
}

#include <RTI/SpecificConfig.h>
#include <RTI/Exception.h>
#include <vector>
#include <string>
#include <memory>

namespace rti1516
{
  class RTI_EXPORT RTIambassadorFactory
  {
  public:
    RTIambassadorFactory();
    
    virtual
    ~RTIambassadorFactory()
      RTI_NOEXCEPT;
    
    // 10.35
    RTI_UNIQUE_PTR< RTIambassador >
    createRTIambassador(std::vector< std::wstring > & args)
      RTI_THROW ((BadInitializationParameter,
             RTIinternalError));
  };
}

#endif // RTI_RTIambassadorFactory_h
