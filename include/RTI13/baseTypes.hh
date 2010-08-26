// HLA 1.3 Header "baseTypes.hh"

typedef unsigned short UShort;
typedef short Short;
typedef unsigned int ULong;
typedef int Long;
typedef double Double;
typedef float Float;

enum Boolean {
    RTI_FALSE = 0,
    RTI_TRUE
};

class RTI_EXPORT Exception
{
public:
    ULong _serial;
    const char* _reason;
    const char* _name;
    Exception(const char* reason);
    Exception(ULong serial, const char* reason);
    Exception(const Exception& toCopy);
    virtual ~Exception();
    Exception& operator=(const Exception&);
};

#define RTI_EXCEPT(A) \
class A : public Exception { \
public: \
    A (const char *reason) : Exception(reason) { _name = #A; } \
    A (ULong serial, const char *reason = 0) \
        : Exception(serial, reason) { _name = #A; } \
    A (A const &toCopy) : Exception(toCopy) { _name = #A; } \
};
