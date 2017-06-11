#include <QtCore/QCoreApplication>

//#include <stdlib.h>
#include <string.h>

#include <RTI.hh>
#include <fedtime.hh>

#include <windows.h>   // for "Sleep"
#include <sys/timeb.h> // for "struct _timeb"

#include "HwFederateAmbassador.h"//成员代理头文件
#include "Country.h"//成员头文件
//#include "fedtime.hh"

RTI::Boolean        timeAdvGrant = RTI::RTI_FALSE;		//时间推进许可
RTIfedTime          grantTime(0.0);

//打印可执行程序的使用方法
void printUsage(const char* pExeName)
{
	cout << "FED_HW: usage: "
		<< pExeName
		<< " <Country Name> <Initial Population> [<Number of Ticks>]"
		<< endl;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	const char* exeName = argv[0];					//可执行程序的名称
	char* const fedExecName = "HelloWorld";	//联邦执行的名称
	Country*    myCountry = NULL;					// Pointer to Federate's Country
	int         numberOfTicks(100);

	//------------------------------------------------------
	// Make sure executable process is provided with correct number
	// of arguments. 确保提供给可执行程序的名称是正确的
	//------------------------------------------------------
	if (argc < 3)
	{
		printUsage(exeName);
		system("pause");
		return -1;
	}
	else if (!argv[1] && !argv[2] && (argc == 3 || !argv[3]))
	{
		printUsage(exeName);
		return -1;
	}
	else
	{
		myCountry = new Country(argv[1], argv[2]);

		if (argc > 3)
		{
			numberOfTicks = atoi(argv[3]);
		}
	}

	try
	{
		//------------------------------------------------------
		// Create RTI objects
		//
		// The federate communicates to the RTI through the RTIambassador
		// object and the RTI communicates back to the federate through
		// the FederateAmbassador object.
		// 联邦通过RTIambassador对象与RTI进行通信，RTI通过FederateAmbassador
		// 对象与联邦进行通信
		//------------------------------------------------------
		RTI::RTIambassador       rtiAmb;         // libRTI provided，RTI代理
		HwFederateAmbassador     fedAmb;         // User-defined，成员代理

		// Named value placeholder for the federates handle -
		// we don't really use this in HelloWorld but some
		// services and MOM might need it - if we were to use them.
		RTI::FederateHandle      federateId;//联邦成员句柄

		//------------------------------------------------------
		// Create federation execution. 创建联邦执行
		//
		// The HJ_RTI_CONFIG environment variable must be set in the 
		// shell's environment to the directory that contains
		// the RTI.rid file and the HelloWorld.fed
		//      
		// In RTI 1.3, when a federate creates the federation 
		// execution the $RTI_HOME/bin/$RTI_ARCH/fedex.sh process
		// is executed on the localhost. Therefore, the RTI_HOME 
		// environment variable must be set to the root of the 
		// RTI 1.0 distribution tree in the federate environment.
		//------------------------------------------------------
		try
		{
			//------------------------------------------------------
			// A successful createFederationExecution will cause
			// the FedExec process to be executed on this machine.
			//
			// A "HelloWorld.fed" file must exist in the
			// RTI_CONFIG directory. This file specifies the FOM
			// object and interaction class structures as well as 
			// default/initial transport and ordering information for
			// object attributes and interaction classes. In RTI 1.0
			// the name of the file was required to be the same as 
			// the name of the federation execution but in 1.3 a 2nd
			// argument to createFederationExecution exists to specify
			// the FED filename.
			//------------------------------------------------------
			cout << "FED_HW: CREATING FEDERATION EXECUTION" << endl;
			//创建联邦执行，需要联邦执行名和Fed文件名作为参数
			rtiAmb.createFederationExecution(fedExecName, "HelloWorld.fed");
			cout << "FED_HW: SUCCESSFUL CREATE FEDERATION EXECUTION" << endl;
		}
		catch (RTI::FederationExecutionAlreadyExists& e)
		{
			cerr << "FED_HW: Note: Federation execution already exists." << &e << endl;
		}
		catch (RTI::Exception& e)
		{
			cerr << "FED_HW: ERROR:" << &e << endl;
		}

		struct _timeb tb;
		_ftime(&tb);

		cout << myCountry->GetName() << " " << tb.time << " " <<
			tb.millitm * 1000 << " START" << endl;

		RTI::Boolean Joined = RTI::RTI_FALSE;
		int          numTries = 0;

		//------------------------------------------------------
		// Here we loop around the joinFederationExecution call
		// until we try to many times or the Join is successful.
		// 
		// The federate that successfully CREATES the federation
		// execution will get to the join call before the 
		// FedExec is initializes and will be unsuccessful at
		// JOIN call.  In this loop we catch the
		// FederationExecutionDoesNotExist exception to
		// determine that the FedExec is not initialized and to
		// keep trying. If the JOIN call does not throw an
		// exception then we set Joined to TRUE and that will
		// cause us to exit the loop anf proceed in the execution.
		//------------------------------------------------------
		while (!Joined && (numTries++ < 20))
		{

			//------------------------------------------------------
			// Join the named federation execution as the named
			// federate type.  Federate types (2nd argument to
			// joinFederationExecution) does not have to be unique
			// in a federation execution; however, the save/restore
			// services use this information but we are not doing
			// save/restore in HelloWorld so we won't worry about it
			// here (best to make the names unique if you do
			// save/restore unless you understand how save/restore
			// will use the information
			//
			//------------------------------------------------------
			try
			{
				cout << "FED_HW: JOINING FEDERATION EXECUTION: " << exeName << endl;
				//加入联邦运行
				federateId = rtiAmb.joinFederationExecution((char* const)
					myCountry->GetName(),//成员名
					fedExecName, //联邦执行名
					&fedAmb);//成员代理
				Joined = RTI::RTI_TRUE;
			}
			catch (RTI::FederateAlreadyExecutionMember& e)
			{
				cerr << "FED_HW: ERROR: " << myCountry->GetName()
					<< " already exists in the Federation Execution "
					<< fedExecName << "." << endl;
				cerr << &e << endl;
				return -1;
			}
			catch (RTI::FederationExecutionDoesNotExist&)
			{
				cerr << "FED_HW: ERROR: " << fedExecName << " Federation Execution "
					<< "does not exists." << endl;

				Sleep(2000);
			}
			catch (RTI::Exception& e)
			{
				cerr << "FED_HW: ERROR:" << &e << endl;
			}
		} // end of while

		cout << "FED_HW: JOINED SUCCESSFULLY: " << exeName
			<< ": Federate Handle = " << federateId << endl;

		//------------------------------------------------------
		// Must enable attribute relevance advisories in order to 
		// receive turnUpdatesOnForObjectInstance callbacks.
		//------------------------------------------------------
		try
		{
			rtiAmb.enableAttributeRelevanceAdvisorySwitch();
		}
		catch (RTI::Exception& e)
		{
			cerr << "FED_HW: ERROR:" << &e << endl;
		}

		//------------------------------------------------------
		// The Country class needs to determine what the RTI is
		// going to call its class type and its attribute's types.
		//
		// This is stored globally even though it is
		// theoretically possible for a federate to join more
		// than one Federation Execution and thus
		// possibly have more than one Run-Time mapping.
		//
		// Note: This has not been tested and may not work.
		//       This would require having an RTIambassador for
		//       each FederationExecution.
		//------------------------------------------------------
		Country::Init(&rtiAmb);//成员初始化

		//------------------------------------------------------
		// Publication/Subscription
		//
		// Declare my interests to the RTI for the object and
		// interaction data types I want to receive.  Also tell
		// the RTI the types of data I can produce.
		//
		// Note: In publication I am telling the RTI the type
		// of data I CAN produce not that I necessarily will.
		// In this program we will create all data types that
		// are published however in more advance applications
		// this convention allows migration of object &
		// attributes to other simulations as neccessary
		// through the Ownership Management services.
		//
		// NOTE: Each time an object or interaction class is
		//       subscribed or published it replaces the previous
		//       subscription/publication for that class.
		//------------------------------------------------------
		myCountry->PublishAndSubscribe();//公布定购

		//------------------------------------------------------
		// Register my object with the federation execution.
		// This requires invoking the registerObject service 
		// which returns an HLA object handle. 
		//------------------------------------------------------
		myCountry->Register();//实体注册

		// Set time step to 1
		const RTIfedTime timeStep(10.0);

		timeAdvGrant = RTI::RTI_FALSE;

		//------------------------------------------------------
		// Set the Time Management parameters
		//
		// This version of HelloWorld operates as a time-stepped
		// simulation.  This means that it should be constrained
		// and regulating.
		//
		// In the 1.0 version of HelloWorld this section of code
		// was before the publication and subscription.  In 1.3
		// enableTimeConstrained and enableTimeRegulation 
		//------------------------------------------------------
		try
		{
			//选择时间管理方式
			cout << "FED_HW: ENABLING TIME CONTRAINT" << endl;
			//------------------------------------------------------
			// Turn on constrained status so that regulating
			// federates will control our advancement in time.
			//
			// If we are constrained and sending fderates specify
			// the Country attributes and Communication interaction
			// with timestamp in the HelloWorld.fed file we will
			// receive TimeStamp Ordered messages.
			//------------------------------------------------------
			timeAdvGrant = RTI::RTI_FALSE;
			rtiAmb.enableTimeConstrained();

			//------------------------------------------------------
			// Tick the RTI until we get the timeConstrainedEnabled
			// callback with my current time.
			//------------------------------------------------------
			while (!timeAdvGrant)
			{
				rtiAmb.tick(0.01, 1.0);
			}
		}
		catch (RTI::Exception& e)
		{
			cerr << "FED_HW: ERROR:" << &e << endl;
		}

		try
		{
			cout << "FED_HW: ENABLING TIME REGULATION WITH LOOKAHEAD = "
				<< Country::GetLookahead().getTime() << endl;
			//------------------------------------------------------
			// Turn on regulating status so that constrained
			// federates will be controlled by our time.
			//
			//------------------------------------------------------
			// enableTimeRegulation is an implicit timeAdvanceRequest
			// so set timeAdvGrant to TRUE since we will get a
			// timeRegulationEnabled which is an implicit 
			// timeAdvanceGrant
			//------------------------------------------------------
			timeAdvGrant = RTI::RTI_FALSE;

			// If we are regulating and our Country attributes and
			// Communication interaction are specified with timestamp
			// in the HelloWorld.fed file we will send TimeStamp
			// Ordered messages.
			//------------------------------------------------------
			rtiAmb.enableTimeRegulation(grantTime, Country::GetLookahead());


			//------------------------------------------------------
			// Tick the RTI until we gwt the timeRegulationEnabled
			// callback with my current time.
			//------------------------------------------------------
			while (!timeAdvGrant)
			{
				rtiAmb.tick(0.01, 1.0);
			}
		}
		catch (RTI::Exception& e)
		{
			cerr << "FED_HW: ERROR:" << &e << endl;
		}

		try
		{
			cout << "FED_HW: ENABLING ASYNCHRONOUS DELIVERY" << endl;
			//------------------------------------------------------
			// Turn on asynchronous delivery of receive ordered
			// messages. This will allow us to receive messages that
			// are not TimeStamp Ordered outside of a time
			// advancement.
			//------------------------------------------------------
			rtiAmb.enableAsynchronousDelivery();
		}
		catch (RTI::Exception& e)
		{
			cerr << "FED_HW: ERROR:" << &e << endl;
		}

		//------------------------------------------------------
		// Event Loop
		// ----------
		// 
		// 1.) Calculate current state and update to RTI.
		// 2.) Ask for a time advance.
		// 3.) Tick the RTI waiting for the grant and process all
		//     RTI initiated services (especially reflections).
		// 4.) Repeat.
		//------------------------------------------------------
		int counter = 0;

		while (counter++ < numberOfTicks - 1)
		{
			cout << "FED_HW: " << endl;
			cout << "FED_HW: HelloWorld Event Loop Iteration #: " << counter << endl;

			//------------------------------------------------------
			// 1.) - Update current state
			//------------------------------------------------------
			myCountry->Update(grantTime);

			// Print state of all countries
			Country* pCountry = NULL;
			for (unsigned int i = 0; i < Country::ms_extentCardinality; i++)
			{
				pCountry = Country::ms_countryExtent[i];

				if (pCountry)
				{
					cout << "FED_HW: Country[" << i << "] " << pCountry << endl;
				}
			}

			//------------------------------------------------------
			// 2.) - Ask for a time advance.
			//------------------------------------------------------
			try
			{
				RTIfedTime requestTime(timeStep);
				requestTime += grantTime;
				timeAdvGrant = RTI::RTI_FALSE;

				rtiAmb.timeAdvanceRequest(requestTime);
			}
			catch (RTI::Exception& e)
			{
				cerr << "FED_HW: ERROR: " << &e << endl;
			}

			//------------------------------------------------------
			// 3.) Tick the RTI waiting for the grant and process all
			//     RTI initiated services (especially reflections).
			//------------------------------------------------------
			while (timeAdvGrant != RTI::RTI_TRUE)
			{
				//------------------------------------------------------
				// Tick will turn control over to the RTI so that it can
				// process an event.  This will cause an invocation of one
				// of the federateAmbassadorServices methods.
				//
				// Be sure not to invoke the RTIambassadorServices from the
				// federateAmbassadorServices; otherwise, a ConcurrentAccess
				// exception will be thrown.
				//------------------------------------------------------
				rtiAmb.tick(0.1, 1.0);
			}
		} // 4.) - Repeat

		if (myCountry)
		{
			// Perform last update - this is necessary to give the
			// Country instance the current granted time otherwise
			// the deleteObjectInstance call that happens in the
			// destructor will have an invalid time since it is in
			// the past. This is a problem with HelloWorld not RTI.
			myCountry->Update(grantTime);
			delete myCountry;
		}

		//------------------------------------------------------
		// Resign from the federation execution to remove this
		// federate from participation.  The flag provided
		// will instruct the RTI to call deleteObjectInstance
		// for all objects this federate has privilegeToDelete
		// for (which by default is all objects that this federate
		// registered) and to release ownership of any attributes
		// that this federate owns but does not own the
		// privilefeToDelete for.
		//------------------------------------------------------
		try
		{
			system("pause");
			cout << "FED_HW: RESIGN FEDERATION EXECUTION CALLED" << endl;
			rtiAmb.resignFederationExecution(
				RTI::DELETE_OBJECTS_AND_RELEASE_ATTRIBUTES);
			cout << "FED_HW: SUCCESSFUL RESIGN FEDERATION EXECUTION CALLED" << endl;
			system("pause");
		}
		catch (RTI::Exception& e)
		{
			cerr << "FED_HW: ERROR:" << &e << endl;
			return -1;
		}

		//------------------------------------------------------
		// Destroy the federation execution in case we are the
		// last federate. This will not do anything bad if there
		// other federates joined.  The RTI will throw us an
		// exception telling us that other federates are joined
		// and we can just ignore that.
		//------------------------------------------------------
		try
		{
			cout << "FED_HW: DESTROY FEDERATION EXECUTION CALLED" << endl;
			rtiAmb.destroyFederationExecution(fedExecName);
			cout << "FED_HW: SUCCESSFUL DESTROY FEDERATION EXECUTION CALLED" << endl;
		}
		catch (RTI::Exception& e)
		{
			cerr << "FED_HW: ERROR:" << &e << endl;
			return -1;
		}

	}
	catch (RTI::ConcurrentAccessAttempted& e)
	{
		cerr << "FED_HW: ERROR: Concurrent access to the RTI was attemted.\n"
			<< "       Exception caught in main() - PROGRAM EXITING.\n"
			<< "\n"
			<< "Note:  Concurrent access will result from invoking\n"
			<< "       RTIambassadorServices within the scope of\n"
			<< "       federateAmbassadorService invocations.\n"
			<< "\n"
			<< "       e.g. RTI calls provideAttributeValueUpdate() and\n"
			<< "       within that method you invoke updateAttributeValues\n"
			<< endl;
		cerr << &e << endl;
		return -1;
	}
	catch (RTI::Exception& e)
	{
		cerr << "FED_HW: ERROR:" << &e << endl;
		return -1;
	}

// 	struct _timeb tb;
// 	_ftime(&tb);

	//   cout << argv[2] << " " << tb.time << " " << tb.millitm * 1000 << 
	//           " END" << endl;

	cout << "FED_HW: Exiting " << exeName << "." << endl;

	//return 0;
	return a.exec();
}
