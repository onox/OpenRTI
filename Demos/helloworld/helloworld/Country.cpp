#include <RTI.hh>
#include <fedtime.hh>

#include <string.h>
#include <windows.h>

// the following are for ntohl and cvt_ftof
#if defined(_X86_) && !defined(WIN32)
#include <arpa/inet.h>
#endif 

#if defined(__alpha)
#include <arpa/inet.h>
#include <cvt.h>
#endif  // __alpha

#include <stdlib.h>
#include "Country.h"

 //-----------------------------------------------------------------
// Static variable definition
//-----------------------------------------------------------------
RTI::RTIambassador*          Country::ms_rtiAmb        = NULL;
RTI::ObjectClassHandle       Country::ms_countryTypeId = 0;
RTI::AttributeHandle         Country::ms_nameTypeId    = 0;
RTI::AttributeHandle         Country::ms_popTypeId     = 0;
RTI::InteractionClassHandle  Country::ms_commTypeId    = 0;
RTI::ParameterHandle         Country::ms_commMsgTypeId = 0;

RTIfedTime                   Country::ms_lookahead(1.0);	//????????????

RTI::Boolean Country:: ms_sendCommInteractions       = RTI::RTI_TRUE;
RTI::Boolean Country:: ms_doRegistry                 = RTI::RTI_TRUE;

char* const  Country::ms_countryTypeStr = "Country";       
char* const  Country::ms_nameTypeStr = "Name" ;
char* const  Country::ms_popTypeStr = "Population" ;       

char* const  Country::ms_commTypeStr = "Communication";
char* const  Country::ms_commMsgTypeStr = "Message";

const double Country::ms_growthRate(0.001); 

// This is bad form - The size of the array is based on a compile
// directive but the initialization of the array is hard coded for
// a specific number of elements.  This will do - it's not worth
// going through the hassle of creating a dynamic array of pointers.
CountryPtr   Country::ms_countryExtent[MAX_FEDERATE_SIZE + 1] =
{ NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
  
unsigned int Country::ms_extentCardinality = 0;

// Non-Class Constant
const double countryDefaultPopulation = 100;

//-----------------------------------------------------------------
//
// METHOD:
//     Country::Country( const char* name, const char* populationStr )
//
// PURPOSE:
//     Constructor.  The constructor initializes the member data
//     with the values passed in and adds this Country instance to
//     the Country extenet (collection of all elements of a type).
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
Country::Country( const char* name, const char* populationStr )
   : m_name(NULL),
     m_lastTime(0.0),
     m_sendNameAttrUpdates(RTI::RTI_TRUE),
     m_sendPopulationAttrUpdates(RTI::RTI_TRUE)
{
   //-----------------------------------------------------------------
   // Add this new object to the extent (collection) of all
   // Country instances.
   //-----------------------------------------------------------------
   Country::ms_countryExtent[ Country::ms_extentCardinality++ ] = this;

   this->SetName( name );

   if ( populationStr && strlen( populationStr ) > 0 )
   {
      this->SetPopulation( atof( populationStr ) );
   }
   else
   {
      this->SetPopulation( countryDefaultPopulation ); // default population
   }
}

//-----------------------------------------------------------------
//
// METHOD:
//     Country::Country( const char* name, const double& population )
//
// PURPOSE:
//     Constructor.  The constructor initializes the member data
//     with the values passed in and adds this Country instance to
//     the Country extenet (collection of all elements of a type).
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
Country::Country( const char* name, const double& population )
   : m_name(NULL),
     m_lastTime(0.0),
     m_sendNameAttrUpdates(RTI::RTI_TRUE),
     m_sendPopulationAttrUpdates(RTI::RTI_TRUE)
{
   //-----------------------------------------------------------------
   // Add this new object to the extent (collection) of all
   // Country instances.
   //-----------------------------------------------------------------
   Country::ms_countryExtent[ Country::ms_extentCardinality++ ] = this;

   this->SetName( name );
   this->SetPopulation( population );
}

//-----------------------------------------------------------------
//
// METHOD:
//     Country::Country()
//
// PURPOSE:
//     Constructor.  The constructor initializes the member data
//     with the default values and adds this Country instance to
//     the Country extent (collection of all elements of a type).
//
//     This constructor is used when a remote object is discovered.
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
Country::Country( RTI::ObjectHandle  id)
   : m_instanceId(id),
     m_name(NULL),
     m_lastTime(0.0),
     m_sendNameAttrUpdates(RTI::RTI_TRUE),
     m_sendPopulationAttrUpdates(RTI::RTI_TRUE)
{
   //-----------------------------------------------------------------
   // Add this new object to the extent (collection) of all
   // Country instances.
   //-----------------------------------------------------------------
   Country::ms_countryExtent[ Country::ms_extentCardinality++ ] = this;

   this->SetName( NULL );
   this->SetPopulation( countryDefaultPopulation ); // default population
}

//-----------------------------------------------------------------
//
// METHOD:
//     Country::Country()
//
// PURPOSE:
//     Constructor.  The constructor initializes the member data
//     with the default values and adds this Country instance to
//     the Country extenet (collection of all elements of a type).
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
Country::Country()
   : m_name(NULL),
     m_lastTime(0.0)
{
   //-----------------------------------------------------------------
   // Add this new object to the extent (collection) of all
   // Country instances.
   //-----------------------------------------------------------------
   Country::ms_countryExtent[ Country::ms_extentCardinality++ ] = this;

   this->SetName( NULL );
   this->SetPopulation( countryDefaultPopulation ); // default population
}

//-----------------------------------------------------------------
//
// METHOD:
//     Country::~Country()
//
// PURPOSE:
//     Virtual destructor. Frees memory allocated for m_name and
//     removes this instance from the extent.
//
//     Note: Removing an element from the extent causes the array
//           to be collapsed so as to not have any gaps.  Some
//           elements will have new indices within the extent
//           after this occurs.
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
Country::~Country()
{
   Country *pCountry = NULL;
   unsigned int ndx = 0;

   //-----------------------------------------------------------------
   // Find the position in the extent for this instance.  The
   // zero'th position always hold the local instance.
   //-----------------------------------------------------------------
   for ( ndx = 0; ndx < Country::ms_extentCardinality; ndx++ )
   {
      pCountry = Country::ms_countryExtent[ ndx ];

      if ( pCountry && pCountry->GetInstanceId() == this->GetInstanceId() )
      {
         break;
      }
   }

   if ( pCountry )
   {
      //-----------------------------------------------------------------
      // First thing to do is move the rest of the Country objects
      // one position up in the collection and then reduce the
      // cardinality by one.
      //-----------------------------------------------------------------
      for ( unsigned int i=ndx; (i < Country::ms_extentCardinality)
                       && (Country::ms_countryExtent[ i ] != NULL); i++ )
      {
         Country::ms_countryExtent[ i ] = Country::ms_countryExtent[ i+1 ];
      }

      Country::ms_extentCardinality = Country::ms_extentCardinality - 1;

      //-----------------------------------------------------------------
      // If the instance was found in the 0th position then this is the
      // local Country instance so we should delete it from the federation
      // execution.
      //-----------------------------------------------------------------
      if ( ms_rtiAmb && ndx==0 )
      {
         //-----------------------------------------------------------------
         // this call returns an event retraction handle but we don't
         // support event retraction so no need to store it.
         //-----------------------------------------------------------------
         (void) ms_rtiAmb->deleteObjectInstance( this->GetInstanceId(),
                                                 this->GetLastTimePlusLookahead(),
                                                 NULL );
      }
      else
      {
         //-----------------------------------------------------------------
         // Otherwise, this is a remote object that removeObject was called
         // on.
         //-----------------------------------------------------------------
         //We don't need to do anything here 
      }
   }
 
   if ( m_name )
   {
      delete m_name;
   }
}

//-----------------------------------------------------------------
//
// METHOD:
//     Country* Country::Find( RTI::ObjectHandle objectId )
//
// PURPOSE:
//     Looks through the extent to find the Country instance
//     with the passed in object Id.
//
// RETURN VALUES:
//     Pointer to country object that has the specified
//     ObjectHandle. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
Country* Country::Find( RTI::ObjectHandle objectId )
{
   Country *pCountry = NULL;

   for ( unsigned int i = 0; i < Country::ms_extentCardinality; i++ )
   {
      pCountry = Country::ms_countryExtent[ i ];

      if ( pCountry && pCountry->GetInstanceId() == objectId )
         break;
   }

   return pCountry;
}

//-----------------------------------------------------------------
//
// METHOD:
//     Country::Init( const RTI::RTIambassador* rtiAmb )
//
// PURPOSE:
//     Sets the member data containing the RTI run-time type
//     ids for Country's class and attributes.  Stores the
//     rtiAmb pointer for updating state etc.
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
void Country::Init( RTI::RTIambassador* rtiAmb )
{
   ms_rtiAmb = rtiAmb;

   if ( ms_rtiAmb )
   {
      //------------------------------------------------------
      // Get the RTTI (Meta-Object Protocol MOP) handles
      //
      // Since the 1.0 RTI does not know anything about your data
      // and thus uses Run-Time Type Identification we must ask the
      // RTI what to call each of our data elements.
      //
      //------------------------------------------------------
      ms_countryTypeId  = ms_rtiAmb->getObjectClassHandle(ms_countryTypeStr);
      ms_nameTypeId     = ms_rtiAmb->getAttributeHandle( ms_nameTypeStr,
                                                         ms_countryTypeId);
      ms_popTypeId      = ms_rtiAmb->getAttributeHandle( ms_popTypeStr,
                                                         ms_countryTypeId);

   }
}

//-----------------------------------------------------------------
//
// METHOD:
//     RTIfedTime Country::GetLastTimePlusLookahead( )
//
// PURPOSE:
//     Gets the last time plus the current lookahead.
//
// RETURN VALUES:
//     RTIfedTime 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
RTIfedTime Country::GetLastTimePlusLookahead()
{
  // Make sure that the lookahead is current
  try 
  {
    ms_rtiAmb->queryLookahead(ms_lookahead);
  }
  catch ( RTI::Exception& e )
  {
    cerr << "FED_HW: Error:" << &e << endl;
  }

  return m_lastTime + ms_lookahead;
}

//-----------------------------------------------------------------
//
// METHOD:
//     void Country::PublishAndSubscribe()
//
// PURPOSE:
//     Publishes and Subscribes to Object & Interaction classes
//     and their member data.
//
//     Note: In most reasonable applications this would be broken
//           up into 2 different methods.
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
void Country::PublishAndSubscribe()
{
   if ( ms_rtiAmb )
   {
      //------------------------------------------------------
      // To actually subscribe and publish we need to build
      // an AttributeHandleSet that contains a list of
      // attribute type ids (AttributeHandle).
      //------------------------------------------------------
      RTI::AttributeHandleSet *countryAttributes;
      countryAttributes = RTI::AttributeHandleSetFactory::create(2);

      countryAttributes->add( ms_nameTypeId );
      countryAttributes->add( ms_popTypeId );

      ms_rtiAmb->subscribeObjectClassAttributes( ms_countryTypeId,
                                                *countryAttributes );
      ms_rtiAmb->publishObjectClass( ms_countryTypeId,
                                     *countryAttributes);

      countryAttributes->empty();

      delete countryAttributes;   // Deallocate the memory

      //------------------------------------------------------
      // Same as above for interactions
      //------------------------------------------------------

      // Get RTTI info
      ms_commTypeId    = ms_rtiAmb->getInteractionClassHandle( ms_commTypeStr );

      ms_commMsgTypeId = ms_rtiAmb->getParameterHandle( ms_commMsgTypeStr,
                                                        ms_commTypeId);

      // Declare my Interaction interests
      ms_rtiAmb->subscribeInteractionClass( ms_commTypeId );
      ms_rtiAmb->publishInteractionClass( ms_commTypeId );
   }
}

//-----------------------------------------------------------------
//
// METHOD:
//     void Country::Register()
//
// PURPOSE:
//     Creates an HLA object instance and registers it with the RTI.
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
void Country::Register()
{
   if ( ms_rtiAmb )
   {
      //---------------------------------------------------------
      // Register my country object with the RTI.  Registering
      // an object with the RTI allows the object to be 
      // discovered by other federates in the federation
      // execution.
      //
      // Note: Discovery happens after an object is registered
      //       not after the initial update like in the 1.0 RTI.
      //---------------------------------------------------------
      m_instanceId =
         ms_rtiAmb->registerObjectInstance( this->GetCountryRtiId() );
   }
}

//-----------------------------------------------------------------
//
// METHOD:
//     void Country::SetInteractionControl( RTI::Boolean status,
//                      RTI::InteractionClassHandle theClass )
//
// PURPOSE:
//     Sets flag so that application knows whether to send
//     interactions for the specified class to the RTI.
//     The Local RTI Component (LRC) will tell my federate to
//     send interactions of a particular class when other
//     federates are subscribed to them. The LRC invokes the
//     FederateAmbassador::turnInteractionsOn service which
//     is what calls this method.
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
void Country::SetInteractionControl( RTI::Boolean status,
                 RTI::InteractionClassHandle theClass )
{
   if ( theClass == Country::GetCommRtiId() )
   {
      // Set a flag here so that I can tell whether I
      // need to send an interaction of this type.
      ms_sendCommInteractions = status;
          
      char *pStr = ms_sendCommInteractions ? "ON" : "OFF";
          
      cout << "FED_HW: Turning Communication Interactions "
           << pStr << "." << endl;
   }
   else
   {
      // If it gets this far I don't know this type of interaction
      // better let someone know.
      char *pStr = status ? "Start" : "Stop";
      cerr << pStr
           << " interaction for unknown class: " << theClass << endl;
   }
}

//-----------------------------------------------------------------
//
// METHOD:
//     void Country::SetUpdateControl( RTI::Boolean    status,
//                      const RTI::AttributeHandleSet& theAttributes )
//
// PURPOSE:
//     Sets flag so that application knows whether to send updates
//     for class Country to RTI. The Local RTI Component (LRC) will
//     tell my federate to update the attributes of a particular
//     object when other federates are subscribed to them. The LRC
//     invokes the FederateAmbassador::turnUpdatesOnForObjectInstance
//     service which is what calls this method.
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
void Country::SetUpdateControl( RTI::Boolean status,
                 const RTI::AttributeHandleSet& theAttrHandles )
{
   RTI::AttributeHandle attrHandle;

   //-----------------------------------------------------------------
   // We need to iterate through the AttributeHandleSet
   // to extract each AttributeHandle.  Based on the type
   // specified ( the value returned by getHandle() ) we need to
   // set the status of whether we should send this attribute when
   // its value changes.
   //-----------------------------------------------------------------
   for ( unsigned int i = 0; i < theAttrHandles.size(); i++ )
   {
      attrHandle = theAttrHandles.getHandle( i );
      if ( attrHandle == Country::GetPopulationRtiId() )
      {
         // Turn population updates on/off
         m_sendPopulationAttrUpdates = status;

         char *pStr = m_sendPopulationAttrUpdates ? "ON" : "OFF";

         cout << "FED_HW: Turning Country.Population Updates "
              << pStr << " for object " << this->GetInstanceId()
              << " ." << endl;
      }
      else if ( attrHandle == Country::GetNameRtiId() )
      {
         // Turn name updates on/off
         m_sendNameAttrUpdates = status;

         char *pStr = m_sendNameAttrUpdates ? "ON" : "OFF";
          
         cout << "FED_HW: Turning Country.Name Updates "
              << pStr << " for object " << this->GetInstanceId()
              << " ." << endl;
      }
   }
}

//-----------------------------------------------------------------
//
// METHOD:
//     void Country::Update( RTIfedTime& newTime )
//
// PURPOSE:
//     Update the state of the Country's population based on
//     the new time value.  The deltaTime is calculated based
//     on the last time the Country object was updated and
//     the newTime passed in.  The deltaTime is multiplied by
//     the growth rate and current population to determine the
//     number of births in the deltaTime.  The population is
//     increased by the number of births.
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
void Country::Update( RTIfedTime& newTime )
{
   //------------------------------------------------------
   // we have advanced in time so calculate my next state.
   //------------------------------------------------------
//   double deltaTime = newTime - this->GetLastTime();
    double deltaTime = newTime.getTime()  - this->GetLastTime().getTime() ;
   // Set last time to new time
   this->SetLastTime( newTime );

   if ( deltaTime > 0.0 )
   {
      SetPopulation( GetPopulation() + 
                     (GetPopulation()*ms_growthRate*deltaTime) );
   }

   if ( ms_rtiAmb )
   {
      //------------------------------------------------------
      // Update state of country
      //------------------------------------------------------
      try
      {
         //------------------------------------------------------
         // In order to send the values of our attributes, we must
         // construct an AttributeHandleValuePairSet (AHVPS) which
         // is a set comprised of attribute handles, values, and
         // the size of the values. CreateNVPSet() is a method 
         // defined on the Country class - it is not part of the RTI.
         // Look inside the method to see how to construct an AHVPS
         //------------------------------------------------------
         RTI::AttributeHandleValuePairSet* pNvpSet = this->CreateNVPSet();
         
         //------------------------------------------------------
         // Send the AHVPS to the federation.
         //
         // this call returns an event retraction handle but we 
         // don't support event retraction so no need to store it.
         //------------------------------------------------------
         (void) ms_rtiAmb->updateAttributeValues( this->GetInstanceId(),
                                                  *pNvpSet,
                                                  this->GetLastTimePlusLookahead(), 
                                                  NULL );
         // Must free the memory
         pNvpSet->empty();
         delete pNvpSet;
      }
      catch ( RTI::Exception& e )
      {
         cerr << "FED_HW: Error:" << &e << endl;
      }
      
      // Periodically send an interaction to tell everyone Hello
      static int periodicMessage = 0;

      if ( (periodicMessage++%100) == 0 )
      {
         RTI::ParameterHandleValuePairSet* pParams = NULL;

                 //------------------------------------------------------
                 // Periodically stimulate an update of the "Name" 
                 // attribute for the benefit of late-arriving federates.
                 // It would be more correct to use 
                 // "requestClassAttributeValueUpdate" and 
                 // "provideAttributeValueUpdate", but let's keep things
                 // simple.
                 //------------------------------------------------------
         hasNameChanged = RTI::RTI_TRUE;

         //------------------------------------------------------
         // Set up the data structure required to push this
         // objects's state to the RTI.  The
         // ParameterHandleValuePairSet is similar to the AHVPS
         // except it contains ParameterHandles instead of
         // AttributeHandles.
         //------------------------------------------------------
         pParams = RTI::ParameterSetFactory::create( 1 );

         char *pMessage = "Hello World!";
         
         pParams->add( this->GetMessageRtiId(),
                       (char*) pMessage,
                       ((strlen(pMessage)+1)*sizeof(char)) );
         try
         {
            //------------------------------------------------------
            // this call returns an event retraction handle but we 
            // don't support event retraction so no need to store it.
            //------------------------------------------------------
			 class RTIfedTime tm = this->GetLastTimePlusLookahead();
            (void) ms_rtiAmb->sendInteraction( GetCommRtiId(), *pParams,
                                               tm,
                                               NULL );
         }
         catch ( RTI::Exception& e )
         {
            cerr << "FED_HW: Error:" << &e << endl;
         }

         //------------------------------------------------------
         // Must free the memory:
         //    ParameterSetFactory::create() allocates memory on
         //    the heap.
         //------------------------------------------------------
         delete pParams;
      }
   }
}

//-----------------------------------------------------------------
//
// METHOD:
//     void Country::Update( const AttributeHandleValuePairSet& theAttributes )
//
// PURPOSE:
//     Update the new attribute values.  If this is being called
//     then this object is either a remote object that was
//     discovered or it has some attributes that are owned by
//     another federate.
//
//     Note: This version does not implement any ownership
//           transfer - the above was a generic statement.
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
void Country::Update( const RTI::AttributeHandleValuePairSet& theAttributes )
{
   RTI::AttributeHandle attrHandle;
   RTI::ULong           valueLength;

   // We need to iterate through the AttributeHandleValuePairSet
   // to extract each AttributeHandleValuePair.  Based on the type
   // specified ( the value returned by getHandle() ) we need to
   // extract the data from the buffer that is returned by 
   // getValue().
   for ( unsigned int i = 0; i < theAttributes.size(); i++ )
   {
      attrHandle = theAttributes.getHandle( i );

      if ( attrHandle == Country::GetPopulationRtiId() )
      {
         // When we run this over multiple platforms we will have
         // a problem with different endian-ness of platforms. Either
         // we need to encode the data using something like XDR or
         // provide another mechanism.
         double population;
         theAttributes.getValue( i, (char*)&population, valueLength );

// #if defined(_X86_) || defined(i386)
//          long x = ntohl(*(long*)&population);
//          *(long*)&population = ntohl(* (((long*)&population) + 1));
//          *(((long*)&population)+1) = x;
// #elif defined(__alpha)
//          double x;
//          cvt_ftof(&population, CVT_IEEE_T, &x, CVT_BIG_ENDIAN_IEEE_T, 0);
//          population = x;
// #endif  // __alpha

         SetPopulation( (double)population );
      }
      else if ( attrHandle == Country::GetNameRtiId() )
      {
        // Same as above goes here...
         char name[ 1024 ];
         theAttributes.getValue( i, (char*)name, valueLength );
         SetName( (const char*)name );
      }
   }
}

//-----------------------------------------------------------------
//
// METHOD:
//     void Country::Update( RTI::InteractionClassHandle theInteraction,
//                    const RTI::ParameterHandleValuePairSet& theParameters )
//
// PURPOSE:
//     Process an interaction.
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
void Country::Update( RTI::InteractionClassHandle theInteraction,
             const RTI::ParameterHandleValuePairSet& theParameters )
{
   if ( theInteraction == Country::GetCommRtiId() )
   {
      RTI::ParameterHandle paramHandle;
      RTI::ULong           valueLength;

      // We need to iterate through the AttributeHandleValuePairSet
      // to extract each AttributeHandleValuePair.  Based on the type
      // specified ( the value returned by getHandle() ) we need to
      // extract the data frlom the buffer that is returned by 
      // getValue().
      for ( unsigned int i = 0; i < theParameters.size(); i++ )
      {
         paramHandle = theParameters.getHandle( i );
         if ( paramHandle == Country::GetMessageRtiId() )
         {
            // When we run this over multiple platforms we will have
            // a problem with different endian-ness of platforms. Either
            // we need to encode the data using something like XDR or
            // provide another mechanism.
            char msg[ 1024 ];
            theParameters.getValue( i, (char*)msg, valueLength );
            cout << "FED_HW: Interaction Received: " << msg << endl;
         }
         else
         {
            // There must be an error since there should only be
            // one parameter to Communication.
            cerr << "FED_HW: Error: I seem to have received a parameter for "
                 << "interaction class Communication that I don't "
                 << "know about." << endl;
         }
      }
   }
   else
   {
      cerr << "FED_HW: Recieved an interaction class I don't know about." << endl;   
   }
}
             
 
//-----------------------------------------------------------------
//
// METHOD:
//     void Country::SetName( const char* )
//
// PURPOSE:
//     Sets the new value of the name attribute and sets the
//     changed flag to true.
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
void Country::SetName( const char* name)
{
   // Delete the existing memory
   if ( m_name )
   {
      delete m_name;
   }

   // Allocate appropriate size string and copy data
   if ( name && strlen( name ) > 0 )
   {
      m_name = new char[ strlen(name) + 1 ];
      strcpy( m_name, name );
   }
   else
   {
      m_name = NULL;
   }

   // Set flag so that when Update( FederateTime ) is called
   // we send this new value to the RTI.
   hasNameChanged = RTI::RTI_TRUE;
}

//-----------------------------------------------------------------
//
// METHOD:
//     void Country::SetPopulation( const double& population)
//
// PURPOSE:
//     Sets the new value of the population attribute and sets the
//     changed flag to true.
//
// RETURN VALUES:
//     None. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
void Country::SetPopulation( const double& population )
{
   m_population = population;

   // Set flag so that when Update( FederateTime ) is called
   // we send this new value to the RTI.
   hasPopulationChanged = RTI::RTI_TRUE;
}

//-----------------------------------------------------------------
//
// METHOD:
//     RTI::AttributeHandleValuePairSet* Country::CreateNVPSet()
//
// PURPOSE:
//     Create a name value pair set (aka handle value pair) for
//     the changed attributes of this country object.
//
// RETURN VALUES:
//     RTI::AttributeHandleValuePairSet* containing the
//     attributes that have changed in the instance of Country. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
RTI::AttributeHandleValuePairSet* Country::CreateNVPSet()
{
   RTI::AttributeHandleValuePairSet* pCountryAttributes = NULL;

   // Make sure the RTI Ambassador is set.
   if ( ms_rtiAmb )
   {
      //------------------------------------------------------
      // Set up the data structure required to push this
      // object's state to the RTI.
      //------------------------------------------------------
      pCountryAttributes = RTI::AttributeSetFactory::create( 2 );

      if ( ( hasNameChanged == RTI::RTI_TRUE ) &&
           ( m_sendNameAttrUpdates == RTI::RTI_TRUE ) )
      {
         // We don't do any encoding here since the name type
         // is a string.
         pCountryAttributes->add( this->GetNameRtiId(),
                                  (char*) this->GetName(),
                                  ((strlen(this->GetName())+1)*sizeof(char)) );
      }
      
      if ( ( hasPopulationChanged == RTI::RTI_TRUE ) && 
                  ( m_sendPopulationAttrUpdates == RTI::RTI_TRUE ) )
      {
         // Here we are encoding the double so that it is in a
         // common format so that federates on other platforms
         // know how to read it.

         pCountryAttributes->add( this->GetPopulationRtiId(),
                                  (char*) &this->GetPopulation(),
                                  sizeof(double) );
      }
   }

   // pCountryAttributes is allocated on the heap and must be
   // deallocated by the federate.
   return pCountryAttributes;
}

//-----------------------------------------------------------------
//
// METHOD:
//     ostream &operator << ( ostream &s, Country &v )
//
// PURPOSE:
//     Overloaded stream operator for outputing objects contents in
//     a readable format.
//
// RETURN VALUES:
//     Returns the stream. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
ostream &operator << ( ostream &s, Country &v )
{
   const char* name = v.GetName();
   if (name == 0)
           name = "(unknown)";

   s << "Name: " << name
     << " Population: " << v.GetPopulation()
     << " Time: " << v.GetLastTime().getTime();

   return s;
}

//-----------------------------------------------------------------
//
// METHOD:
//     ostream &operator << ( ostream &s, Country *v )
//
// PURPOSE:
//     Overloaded stream operator for outputing objects contents in
//     a readable format.
//
// RETURN VALUES:
//     Returns the stream. 
//
// HISTORY:
//     1) Created 11/6/96
//     2) Updated to RTI 1.3 3/26/98
//
//-----------------------------------------------------------------
ostream &operator << ( ostream &s, Country *v ) 
{
   if ( !v )
      return s;
   else
   {
      s << *v;
   }

   return s;
}
