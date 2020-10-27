////////////////////////////////////////////////
// Generated by SmartState C++ Code Generator //
//                 DO NOT EDIT				  //
////////////////////////////////////////////////

#pragma warning(disable: 4786)
#pragma warning(disable: 4290)

//Additional Includes
#include "AtomicCOUT.h"
//#include <iostream>
#include <stdlib.h>


#include "ReceiverSS.h"
#include "ReceiverX.h"

/*Messages
Define user specific messages in a file and
include that file in the additional includes section in 
the model.
-- FOLLOWING MESSAGES ARE USED --
SER
CONT
*/

//Additional Declarations
#define c wParam



namespace Receiver_SS
{
using namespace std;
using namespace smartstate;

//State Mgr
//--------------------------------------------------------------------
ReceiverSS::ReceiverSS(ReceiverX* ctx, bool startMachine/*=true*/)
 : StateMgr("ReceiverSS"),
   myCtx(ctx)
{
	myConcStateList.push_back(new Receiver_TopLevel_ReceiverSS("Receiver_TopLevel_ReceiverSS", 0, this));

	if(startMachine)
		start();
}

ReceiverX& ReceiverSS::getCtx() const
{
	return *myCtx;
}

//Base State
//--------------------------------------------------------------------
ReceiverBaseState::ReceiverBaseState(const string& name, BaseState* parent, ReceiverSS* mgr)
 : BaseState(name, parent, mgr)
{
}

//--------------------------------------------------------------------
Receiver_TopLevel_ReceiverSS::Receiver_TopLevel_ReceiverSS(const string& name, BaseState* parent, ReceiverSS* mgr)
 : ReceiverBaseState(name, parent, mgr)
{
	myHistory = false;
	mySubStates.push_back(new FirstByte_Receiver_TopLevel("FirstByte_Receiver_TopLevel", this, mgr));
	mySubStates.push_back(new EOT_Receiver_TopLevel("EOT_Receiver_TopLevel", this, mgr));
	mySubStates.push_back(new ConditionalTransient_Receiver_TopLevel("ConditionalTransient_Receiver_TopLevel", this, mgr));
	mySubStates.push_back(new CAN_Receiver_TopLevel("CAN_Receiver_TopLevel", this, mgr));
	setType(eSuper);
}

void Receiver_TopLevel_ReceiverSS::onEntry()
{
	/* -g option specified while compilation. */
	myMgr->debugLog("> Receiver_TopLevel_ReceiverSS <onEntry>");

	ReceiverX& ctx = getMgr()->getCtx();

	// Code from Model here
	if(ctx.openFileForTransfer() == -1)
	    POST("*",CONT);
	else {
	    ctx.NCGbyte = ctx.Crcflg ? 'C' : NAK; 
	    ctx.sendByte(ctx.NCGbyte); 
	    ctx.errCnt=0;
	}
}

void Receiver_TopLevel_ReceiverSS::onExit()
{
	/* -g option specified while compilation. */
	myMgr->debugLog("< Receiver_TopLevel_ReceiverSS <onExit>");

}

void Receiver_TopLevel_ReceiverSS::onMessage(const Mesg& mesg)
{
	if(mesg.message == SER)
		onSERMessage(mesg);
	else if(mesg.message == CONT)
		onCONTMessage(mesg);
	else 
		super::onMessage(mesg);
}

void Receiver_TopLevel_ReceiverSS::onSERMessage(const Mesg& mesg)
{
	int wParam = mesg.wParam;
	int lParam = mesg.lParam;
	ReceiverX& ctx = getMgr()->getCtx();

		/* -g option specified while compilation. */
		myMgr->debugLog("Receiver_TopLevel_ReceiverSS SER <message trapped>");

	if(true)
	{
		/* -g option specified while compilation. */
		myMgr->debugLog("Receiver_TopLevel_ReceiverSS SER <executing exit>");

		const BaseState* root = getMgr()->executeExit("Receiver_TopLevel_ReceiverSS", "FinalState");
		/* -g option specified while compilation. */
		myMgr->debugLog("Receiver_TopLevel_ReceiverSS SER <executing effect>");


		//User specified effect begin
		COUT << "Receiver received totally unexpected char #" << c << ": " << (char) c << endl;
		exit(EXIT_FAILURE);
		//User specified effect end

		/* -g option specified while compilation. */
		myMgr->debugLog("Receiver_TopLevel_ReceiverSS SER <executing entry>");

		getMgr()->executeEntry(root, "FinalState");
		return;
	}

	super::onMessage(mesg);
}

void Receiver_TopLevel_ReceiverSS::onCONTMessage(const Mesg& mesg)
{
	int wParam = mesg.wParam;
	int lParam = mesg.lParam;
	ReceiverX& ctx = getMgr()->getCtx();

		/* -g option specified while compilation. */
		myMgr->debugLog("Receiver_TopLevel_ReceiverSS CONT <message trapped>");

	if(true)
	{
		/* -g option specified while compilation. */
		myMgr->debugLog("Receiver_TopLevel_ReceiverSS CONT <executing exit>");

		const BaseState* root = getMgr()->executeExit("Receiver_TopLevel_ReceiverSS", "FinalState");
		/* -g option specified while compilation. */
		myMgr->debugLog("Receiver_TopLevel_ReceiverSS CONT <executing effect>");


		//User specified effect begin
		ctx.can8();
		ctx.result="CreatError";
		//User specified effect end

		/* -g option specified while compilation. */
		myMgr->debugLog("Receiver_TopLevel_ReceiverSS CONT <executing entry>");

		getMgr()->executeEntry(root, "FinalState");
		return;
	}

	super::onMessage(mesg);
}

//--------------------------------------------------------------------
FirstByte_Receiver_TopLevel::FirstByte_Receiver_TopLevel(const string& name, BaseState* parent, ReceiverSS* mgr)
 : ReceiverBaseState(name, parent, mgr)
{
	myHistory = false;
}

void FirstByte_Receiver_TopLevel::onEntry()
{
	/* -g option specified while compilation. */
	myMgr->debugLog("> FirstByte_Receiver_TopLevel <onEntry>");

}

void FirstByte_Receiver_TopLevel::onExit()
{
	/* -g option specified while compilation. */
	myMgr->debugLog("< FirstByte_Receiver_TopLevel <onExit>");

}

void FirstByte_Receiver_TopLevel::onMessage(const Mesg& mesg)
{
	if(mesg.message == SER)
		onSERMessage(mesg);
	else 
		super::onMessage(mesg);
}

void FirstByte_Receiver_TopLevel::onSERMessage(const Mesg& mesg)
{
	int wParam = mesg.wParam;
	int lParam = mesg.lParam;
	ReceiverX& ctx = getMgr()->getCtx();

		/* -g option specified while compilation. */
		myMgr->debugLog("FirstByte_Receiver_TopLevel SER <message trapped>");

	if(c == CAN)
	{
		/* -g option specified while compilation. */
		myMgr->debugLog("FirstByte_Receiver_TopLevel SER <executing exit>");

		const BaseState* root = getMgr()->executeExit("FirstByte_Receiver_TopLevel", "CAN_Receiver_TopLevel");
		/* -g option specified while compilation. */
		myMgr->debugLog("FirstByte_Receiver_TopLevel SER <executing effect>");


		//User specified effect begin
		//nil
		//User specified effect end

		/* -g option specified while compilation. */
		myMgr->debugLog("FirstByte_Receiver_TopLevel SER <executing entry>");

		getMgr()->executeEntry(root, "CAN_Receiver_TopLevel");
		return;
	}
	else
	if(c == EOT)
	{
		/* -g option specified while compilation. */
		myMgr->debugLog("FirstByte_Receiver_TopLevel SER <executing exit>");

		const BaseState* root = getMgr()->executeExit("FirstByte_Receiver_TopLevel", "EOT_Receiver_TopLevel");
		/* -g option specified while compilation. */
		myMgr->debugLog("FirstByte_Receiver_TopLevel SER <executing effect>");


		//User specified effect begin
		ctx.sendByte(NAK);
		//User specified effect end

		/* -g option specified while compilation. */
		myMgr->debugLog("FirstByte_Receiver_TopLevel SER <executing entry>");

		getMgr()->executeEntry(root, "EOT_Receiver_TopLevel");
		return;
	}
	else
	if(c==SOH)
	{
		/* -g option specified while compilation. */
		myMgr->debugLog("FirstByte_Receiver_TopLevel SER <executing exit>");

		const BaseState* root = getMgr()->executeExit("FirstByte_Receiver_TopLevel", "ConditionalTransient_Receiver_TopLevel");
		/* -g option specified while compilation. */
		myMgr->debugLog("FirstByte_Receiver_TopLevel SER <executing effect>");


		//User specified effect begin
		ctx.getRestBlk();
		if (ctx.goodBlk1st) 
		     ctx.errCnt=0;
		else ctx.errCnt++;
		//User specified effect end

		/* -g option specified while compilation. */
		myMgr->debugLog("FirstByte_Receiver_TopLevel SER <executing entry>");

		getMgr()->executeEntry(root, "ConditionalTransient_Receiver_TopLevel");
		return;
	}

	super::onMessage(mesg);
}

//--------------------------------------------------------------------
EOT_Receiver_TopLevel::EOT_Receiver_TopLevel(const string& name, BaseState* parent, ReceiverSS* mgr)
 : ReceiverBaseState(name, parent, mgr)
{
	myHistory = false;
}

void EOT_Receiver_TopLevel::onEntry()
{
	/* -g option specified while compilation. */
	myMgr->debugLog("> EOT_Receiver_TopLevel <onEntry>");

}

void EOT_Receiver_TopLevel::onExit()
{
	/* -g option specified while compilation. */
	myMgr->debugLog("< EOT_Receiver_TopLevel <onExit>");

}

void EOT_Receiver_TopLevel::onMessage(const Mesg& mesg)
{
	if(mesg.message == SER)
		onSERMessage(mesg);
	else 
		super::onMessage(mesg);
}

void EOT_Receiver_TopLevel::onSERMessage(const Mesg& mesg)
{
	int wParam = mesg.wParam;
	int lParam = mesg.lParam;
	ReceiverX& ctx = getMgr()->getCtx();

		/* -g option specified while compilation. */
		myMgr->debugLog("EOT_Receiver_TopLevel SER <message trapped>");

	if(c==CAN)
	{
		/* -g option specified while compilation. */
		myMgr->debugLog("EOT_Receiver_TopLevel SER <executing exit>");

		const BaseState* root = getMgr()->executeExit("EOT_Receiver_TopLevel", "CAN_Receiver_TopLevel");
		/* -g option specified while compilation. */
		myMgr->debugLog("EOT_Receiver_TopLevel SER <executing effect>");


		//User specified effect begin
		//nil
		//User specified effect end

		/* -g option specified while compilation. */
		myMgr->debugLog("EOT_Receiver_TopLevel SER <executing entry>");

		getMgr()->executeEntry(root, "CAN_Receiver_TopLevel");
		return;
	}
	else
	if(c==EOT)
	{
		/* -g option specified while compilation. */
		myMgr->debugLog("EOT_Receiver_TopLevel SER <executing exit>");

		const BaseState* root = getMgr()->executeExit("EOT_Receiver_TopLevel", "FinalState");
		/* -g option specified while compilation. */
		myMgr->debugLog("EOT_Receiver_TopLevel SER <executing effect>");


		//User specified effect begin
		if (ctx.closeTransferredFile()) {
		     ctx.can8();  
		     ctx.result="CloseError";
		}
		else {
		     ctx.sendByte(ACK);
		     ctx.result="Done";
		}
		//User specified effect end

		/* -g option specified while compilation. */
		myMgr->debugLog("EOT_Receiver_TopLevel SER <executing entry>");

		getMgr()->executeEntry(root, "FinalState");
		return;
	}

	super::onMessage(mesg);
}

//--------------------------------------------------------------------
ConditionalTransient_Receiver_TopLevel::ConditionalTransient_Receiver_TopLevel(const string& name, BaseState* parent, ReceiverSS* mgr)
 : ReceiverBaseState(name, parent, mgr)
{
	myHistory = false;
}

void ConditionalTransient_Receiver_TopLevel::onEntry()
{
	/* -g option specified while compilation. */
	myMgr->debugLog("> ConditionalTransient_Receiver_TopLevel <onEntry>");

	ReceiverX& ctx = getMgr()->getCtx();

	// Code from Model here
	POST("*",CONT);
}

void ConditionalTransient_Receiver_TopLevel::onExit()
{
	/* -g option specified while compilation. */
	myMgr->debugLog("< ConditionalTransient_Receiver_TopLevel <onExit>");

}

void ConditionalTransient_Receiver_TopLevel::onMessage(const Mesg& mesg)
{
	if(mesg.message == CONT)
		onCONTMessage(mesg);
	else 
		super::onMessage(mesg);
}

void ConditionalTransient_Receiver_TopLevel::onCONTMessage(const Mesg& mesg)
{
	int wParam = mesg.wParam;
	int lParam = mesg.lParam;
	ReceiverX& ctx = getMgr()->getCtx();

		/* -g option specified while compilation. */
		myMgr->debugLog("ConditionalTransient_Receiver_TopLevel CONT <message trapped>");

	if(!ctx.syncLoss && (ctx.errCnt < errB))
	{
		/* -g option specified while compilation. */
		myMgr->debugLog("ConditionalTransient_Receiver_TopLevel CONT <executing exit>");

		const BaseState* root = getMgr()->executeExit("ConditionalTransient_Receiver_TopLevel", "FirstByte_Receiver_TopLevel");
		/* -g option specified while compilation. */
		myMgr->debugLog("ConditionalTransient_Receiver_TopLevel CONT <executing effect>");


		//User specified effect begin
		if (ctx.goodBlk)  ctx.sendByte(ACK);
		else  ctx.sendByte(NAK);
		if (ctx.goodBlk1st) 
		     ctx.writeChunk();
		
		//User specified effect end

		/* -g option specified while compilation. */
		myMgr->debugLog("ConditionalTransient_Receiver_TopLevel CONT <executing entry>");

		getMgr()->executeEntry(root, "FirstByte_Receiver_TopLevel");
		return;
	}
	else
	if(ctx.syncLoss || ctx.errCnt >= errB)
	{
		/* -g option specified while compilation. */
		myMgr->debugLog("ConditionalTransient_Receiver_TopLevel CONT <executing exit>");

		const BaseState* root = getMgr()->executeExit("ConditionalTransient_Receiver_TopLevel", "FinalState");
		/* -g option specified while compilation. */
		myMgr->debugLog("ConditionalTransient_Receiver_TopLevel CONT <executing effect>");


		//User specified effect begin
		ctx.can8();
		if (ctx.syncLoss)
		     ctx.result="LossOfSyncronization";
		else
		     ctx.result="ExcessiveErrors";
		//User specified effect end

		/* -g option specified while compilation. */
		myMgr->debugLog("ConditionalTransient_Receiver_TopLevel CONT <executing entry>");

		getMgr()->executeEntry(root, "FinalState");
		return;
	}

	super::onMessage(mesg);
}

//--------------------------------------------------------------------
CAN_Receiver_TopLevel::CAN_Receiver_TopLevel(const string& name, BaseState* parent, ReceiverSS* mgr)
 : ReceiverBaseState(name, parent, mgr)
{
	myHistory = false;
}

void CAN_Receiver_TopLevel::onEntry()
{
	/* -g option specified while compilation. */
	myMgr->debugLog("> CAN_Receiver_TopLevel <onEntry>");

}

void CAN_Receiver_TopLevel::onExit()
{
	/* -g option specified while compilation. */
	myMgr->debugLog("< CAN_Receiver_TopLevel <onExit>");

}

void CAN_Receiver_TopLevel::onMessage(const Mesg& mesg)
{
	if(mesg.message == SER)
		onSERMessage(mesg);
	else 
		super::onMessage(mesg);
}

void CAN_Receiver_TopLevel::onSERMessage(const Mesg& mesg)
{
	int wParam = mesg.wParam;
	int lParam = mesg.lParam;
	ReceiverX& ctx = getMgr()->getCtx();

		/* -g option specified while compilation. */
		myMgr->debugLog("CAN_Receiver_TopLevel SER <message trapped>");

	if(c == CAN)
	{
		/* -g option specified while compilation. */
		myMgr->debugLog("CAN_Receiver_TopLevel SER <executing exit>");

		const BaseState* root = getMgr()->executeExit("CAN_Receiver_TopLevel", "FinalState");
		/* -g option specified while compilation. */
		myMgr->debugLog("CAN_Receiver_TopLevel SER <executing effect>");


		//User specified effect begin
		ctx.result="SndCancelled";
		/*ctx.clearCan();*/
		//User specified effect end

		/* -g option specified while compilation. */
		myMgr->debugLog("CAN_Receiver_TopLevel SER <executing entry>");

		getMgr()->executeEntry(root, "FinalState");
		return;
	}

	super::onMessage(mesg);
}


} /*end namespace*/

//___________________________________vv^^vv___________________________________