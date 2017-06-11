#ifndef Country_HEADER
#define Country_HEADER

#include <RTI.hh>
#include <fedtime.hh>
#include <iostream>

#define MAX_FEDERATE_SIZE 512
using namespace std;
//#include <fedtime.h>

//-----------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------
class Country;

//-----------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------
typedef Country* CountryPtr;

//-----------------------------------------------------------------
//
// CLASS:
//     Country
//
// PURPOSE:
//     The purpose of instances of class Country is to demonstrate
//     the proper usage of the RTI C++ API.  Instances can update
//     their state on the local machine (actually simulate the 
//     population) or from reflections provided by the RTI
//     (ghosting).
//
//     Note: Neither the interface nor the implementation of
//           Country is intended to be a tutorial on
//           Object-Oriented Analysis & Design.
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
class Country
{
public:
   Country();
   Country( const char* name, const char*   populationStr );
   Country( const char* name, const double& population );
   Country( RTI::ObjectHandle  id);
   virtual ~Country();

   //-----------------------------------------------------------------
   // Methods acting on the RTI
   //-----------------------------------------------------------------
   static Country*  Find( RTI::ObjectHandle       objectId );
   static void      Init( RTI::RTIambassador* rtiAmb );
   void             PublishAndSubscribe();
   void             Register();
   void             Update( RTIfedTime& newTime );
   void             Update( const RTI::AttributeHandleValuePairSet& theAttributes );
   static void      Update( RTI::InteractionClassHandle theInteraction,
                            const RTI::ParameterHandleValuePairSet& theParameters );
   
   //-----------------------------------------------------------------
   // Friends of Country
   //-----------------------------------------------------------------
   friend ostream& operator << ( ostream &s, Country &v );
   friend ostream& operator << ( ostream &s, Country *v );

   //-----------------------------------------------------------------
   // Accessor Methods
   //-----------------------------------------------------------------
   RTIfedTime                          GetLastTime()
                                          { return m_lastTime; };
   RTIfedTime                          GetLastTimePlusLookahead();

   const char*                         GetName()
                                          { return m_name; };
   double&                             GetPopulation()
                                          { return m_population; };
   RTI::ObjectHandle&                      GetInstanceId()
                                          { return m_instanceId; };

   // Static Accessor Methods
   static RTI::ObjectClassHandle       GetCountryRtiId()
                                          { return ms_countryTypeId; };
   static RTI::AttributeHandle         GetNameRtiId() 
                                          { return ms_nameTypeId; };
   static RTI::AttributeHandle         GetPopulationRtiId()
                                          { return ms_popTypeId; };
   static RTI::InteractionClassHandle  GetCommRtiId()
                                          { return ms_commTypeId; };
   static RTI::ParameterHandle         GetMessageRtiId()
                                          { return ms_commMsgTypeId; };
   static RTIfedTime                   GetLookahead()
                                          { return ms_lookahead;};

   //-----------------------------------------------------------------
   // Mutator Methods
   //-----------------------------------------------------------------
   void                                SetLastTime( RTIfedTime& time )
                                          { m_lastTime = (RTI::FedTime&) time;};
   void                                SetName( const char* );
   void                                SetPopulation( const double& );
   void                                SetUpdateControl( RTI::Boolean status,
                                       const RTI::AttributeHandleSet& attrs );

   // Static Mutator Methods
   static void SetInteractionControl( RTI::Boolean status,
                           RTI::InteractionClassHandle theClass );
   static void SetRegistration( RTI::Boolean status )
                  { ms_doRegistry = status; };
   static void SetLookahead( RTIfedTime& time )
                  { ms_lookahead = (RTI::FedTime&) time;};

   //-----------------------------------------------------------------
   // Static Members
   //-----------------------------------------------------------------
   static const double                 ms_growthRate; 

   // Extent data memebers
   static CountryPtr                   ms_countryExtent[MAX_FEDERATE_SIZE + 1];
   static unsigned int                 ms_extentCardinality;    

protected:
   RTI::AttributeHandleValuePairSet*   CreateNVPSet();

private:
   char*                               m_name;
   double                              m_population;
   RTI::ObjectHandle                   m_instanceId;
   RTIfedTime                          m_lastTime;

   static RTI::RTIambassador*          ms_rtiAmb;

   // Change flags for attribute values (dirty bits)
   RTI::Boolean                        hasNameChanged;
   RTI::Boolean                        hasPopulationChanged;

   // Update Control flags for instances
   RTI::Boolean                        m_sendNameAttrUpdates;
   RTI::Boolean                        m_sendPopulationAttrUpdates;

   //-----------------------------------------------------------------
   // Static Member Data
   //-----------------------------------------------------------------

   // Registration Control flags for class
   static RTI::Boolean                 ms_doRegistry;
   static RTI::Boolean                 ms_sendCommInteractions;

   // Run-Time Type Identification data
   static RTI::ObjectClassHandle       ms_countryTypeId;
   static RTI::AttributeHandle         ms_nameTypeId;
   static RTI::AttributeHandle         ms_popTypeId;
   static RTI::InteractionClassHandle  ms_commTypeId;
   static RTI::ParameterHandle         ms_commMsgTypeId;

   // Names for querying RTTI values
   static char* const                  ms_countryTypeStr;       
   static char* const                  ms_nameTypeStr;       
   static char* const                  ms_popTypeStr;       
   static char* const                  ms_commTypeStr;       
   static char* const                  ms_commMsgTypeStr;

   // Lookahead Time value
   static RTIfedTime                   ms_lookahead;

};

#endif
