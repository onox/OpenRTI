// HLA 1.3 Header "RTI.hh"

#ifndef RTI_hh
#define RTI_hh

#if defined(_WIN32)
// Since the standard intrinsically relies on std::auto_ptr
# if __cplusplus < 201703L && !defined(_HAS_AUTO_PTR_ETC)
#  define _HAS_AUTO_PTR_ETC 1
# endif
# pragma warning(disable: 4290)
# pragma warning(disable: 4275)
# pragma warning(disable: 4251)
# pragma warning(disable: 4273)
# pragma warning(disable: 4996)
# if defined(RTI_EXPORTS)
#  define RTI_EXPORT __declspec(dllexport)
# else
#  define RTI_EXPORT __declspec(dllimport)
# endif
# if defined(FedTime_EXPORTS)
#  define FEDTIME_EXPORT __declspec(dllexport)
# else
#  define FEDTIME_EXPORT __declspec(dllimport)
# endif
#else
# if __cplusplus < 201703L && defined(__GNUC__) && (4 <= __GNUC__)
#  pragma GCC diagnostic ignored "-Wdeprecated"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
# endif
# define RTI_EXPORT
# define FEDTIME_EXPORT
#endif

#if __cplusplus < 201703L
#define RTI_NOEXCEPT throw()
#define RTI_THROW(e) throw e
#define RTI_UNIQUE_PTR std::auto_ptr
#else
#define RTI_NOEXCEPT noexcept
#define RTI_THROW(e)
#define RTI_UNIQUE_PTR std::unique_ptr
#endif

#include <iosfwd>
#include <memory>

class RTIambPrivateRefs;
struct RTIambPrivateData;

/**
 * @defgroup libRTI RTI library (normative API).
 * @ingroup HLA_Libraries
 * The API comes directly from HLA specifications.
 * @{
 */

// May be RTI13 like portico does???
class RTI_EXPORT RTI {
public:
#include "baseTypes.hh"
#include "RTItypes.hh"
  
  class RTI_EXPORT RTIambassador {
  public:
#include "RTIambServices.hh"
    RTIambPrivateData* privateData;
  private:
    RTIambPrivateRefs* privateRefs;
  };
  
  class RTI_EXPORT FederateAmbassador {
  public:
#include "federateAmbServices.hh"
  };
};

std::ostream RTI_EXPORT& 
operator<<(std::ostream& os, const RTI::Exception* ex);

std::ostream RTI_EXPORT& 
operator<<(std::ostream& os, const RTI::Exception& ex);

/** @} */

#endif
