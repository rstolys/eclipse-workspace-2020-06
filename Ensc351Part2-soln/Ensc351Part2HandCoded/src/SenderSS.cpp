// Solution to ENSC 351 Project Part 2.  2020  Prepared by:
//      Copyright (C) 2020 Craig Scratchley, Simon Fraser University

#include <stdlib.h> // for EXIT_FAILURE
#include "AtomicCOUT.h"

#include "SenderSS.h"
#include "SenderX.h"

#define c wParam

#define FinalState 99

namespace Sender_SS
{
using namespace std;

enum {InitialState,
	STATE_START,
	STATE_ACKNAK,
	STATE_EOT1,
	STATE_EOTEOT,
	STATE_CAN,
	STATE_CommonTransient,
};

//State Mgr
//--------------------------------------------------------------------
SenderSS::SenderSS(SenderX* senderCtx, bool startMachine/*=true*/)
: StateMgr("SenderSS"),
  myCtx(senderCtx)
{
	SenderX& ctx = *myCtx;
	
	//Entry code for TopLevel and NON_CAN States
	if (!(ctx.openErr = (ctx.openFileToTransfer() == -1)))  {
	    ctx.Crcflg=true; ctx.prep1stBlk();
	    ctx.errCnt=0; ctx.firstCrcBlk=true;
	}

	state = STATE_START;
//	state = InitialState;
//	postEvent(CONT);//,0,0);
}

/***************************************************************************/
bool SenderSS::isRunning() const
{
    return (state != FinalState);
}

void SenderSS::postEvent(unsigned int event, int /*wParam*/ c, int lParam) throw (std::string)
{
    // do exit code
    handleTransition(event, c, lParam);
    doEntryCode();
}

void SenderSS::doEntryCode()
{
    SenderX& ctx = *myCtx;
    if (state == STATE_CommonTransient) {
        ctx.closeTransferredFile();
        // post CONT event to exit STATE_CommonTransient
        state = FinalState;
    }
}

void SenderSS::handleTransition(unsigned int event, int /*wParam*/ c, int lParam) throw (std::string)
{
	SenderX& ctx = *myCtx;
    // for Assignment 2, (event == SER) always
    if (state == STATE_CAN) {
        if (c == CAN) {
            ctx.result="RcvCancelled";
            //ctx.clearCan();
            state = STATE_CommonTransient;
            return;
        };
    }
    else { // a state in the NON_CAN state
        switch(state) {
        /*
            case InitialState:
                // event is CONT
                state = STATE_START;
                return;
            */
            case STATE_START:
                // for Assignment 2, (event == SER) always
                if ((c == NAK) || (c == 'C')) {
                    if (ctx.openErr){
                        ctx.can8();
                        ctx.result="OpenError";
                        state = FinalState;
                    }
                    else if (!ctx.bytesRd) {
                        if (c==NAK) {ctx.firstCrcBlk=false;}
                        ctx.sendByte(EOT);
                        state = STATE_EOT1;		// at end of file being transferred
                    }
                    else {
                        if (c==NAK) {
                            ctx.Crcflg=false;
                            ctx.cs1stBlk();
                            ctx.firstCrcBlk=false;
                        }
                        ctx.sendBlkPrepNext();
                        state = STATE_ACKNAK;
                    }
                    return;
                }
                break;

            case STATE_ACKNAK:
                // for Assignment 2, (event == SER) always
                if (c == ACK) {
                    if (!ctx.bytesRd) {
                        ctx.sendByte(EOT);
                        ctx.errCnt=0;
                        ctx.firstCrcBlk=false;
                        state = STATE_EOT1;		// at end of file
                    }
                    else {
                        ctx.sendBlkPrepNext();
                        ctx.errCnt = 0;
                        ctx.firstCrcBlk=false;
                    }
                }
                else if ((c == NAK) && (ctx.errCnt < (errB))) { // (errB - 1) ??
                        ctx.resendBlk();
                        ctx.errCnt++;
                }
                else
                    break;
                return;

            case STATE_EOT1:
                // for Assignment 2, (event == SER) always
                if (c == NAK){
                    ctx.sendByte(EOT);
                    state = STATE_EOTEOT;
                }
                else if (c == ACK) {
                    ctx.result="1st EOT ACK'd";
                    state = STATE_CommonTransient;
                }
                else
                    break;
                return;

            case STATE_EOTEOT:
                // for Assignment 2, (event == SER) always
                if (c == ACK) {
                    ctx.result="Done";
                    state = STATE_CommonTransient;
                }
                else if (c=='C') {
                    ctx.can8();
                    ctx.result = "UnexpectedC";
                    state = STATE_CommonTransient;
                }
                else if (c==NAK && ctx.errCnt < errB) {
                    ctx.sendByte(EOT);
                    ctx.errCnt++;
                }
                else
                    break;
                return;

            case FinalState:
                COUT << "Event should not be posted when sender in final state" << endl;
                exit(EXIT_FAILURE);

            default:
                COUT << "Sender in invalid state!" << endl; // COUT ?
                exit(EXIT_FAILURE);
        }

        // process transitions from state NON_CAN
        if ((c == NAK) /* && (ctx.errCnt >= (errB)) */ ) { // (errB - 1) ??
            ctx.can8();
            ctx.result="ExcessiveNAKs";
            state = STATE_CommonTransient;
            return;
        }
        else if (c == CAN) {
            // this is a mystery.  The other 7 CANs are going missing. **
            state = STATE_CAN;
            return;
        }
    };

    // process transition from state Sender_TopLevel
	COUT << "In state " << state << " sender received totally unexpected char #" << c << ": " << (char) c << endl;
	exit(EXIT_FAILURE);
	state = FinalState;
}
		
} // namespace Sender_SS
