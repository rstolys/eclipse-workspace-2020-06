////////////////////////////////////////////////
// Generated by SmartState C++ Code Generator //
//                 DO NOT EDIT				  //
////////////////////////////////////////////////

#ifndef Sender_SS_H
#define Sender_SS_H

#include <ss_api.hxx>

/*Context*/
class SenderX;

namespace Sender_SS
{
	using namespace smartstate;
	//State Mgr
	class SenderSS : public StateMgr
	{
		public:
			SenderSS(SenderX* ctx, bool startMachine=true);

			SenderX& getCtx() const;

		private:
			SenderX* myCtx;
	};

	//Base State
	class SenderBaseState : public BaseState
	{
		protected:
			SenderBaseState(){};
			SenderBaseState(const string& name, BaseState* parent, SenderSS* mgr);

		protected:
			SenderSS* getMgr(){return static_cast<SenderSS*>(myMgr);}
	};

	//States
	//------------------------------------------------------------------------
	class CompleteSender_TopLevel_SenderSS : public virtual SenderBaseState
	{
			typedef SenderBaseState super;

		public:
			CompleteSender_TopLevel_SenderSS(){};
			CompleteSender_TopLevel_SenderSS(const string& name, BaseState* parent, SenderSS* mgr);

			virtual void onMessage(const Mesg& mesg);

			virtual void onEntry();
			virtual void onExit();

		//Transitions

		private:
			void onSERMessage(const Mesg& mesg);
			void onTMMessage(const Mesg& mesg);
	};

	class SERcancelable_CompleteSender_TopLevel : public virtual CompleteSender_TopLevel_SenderSS
	{
			typedef CompleteSender_TopLevel_SenderSS super;

		public:
			SERcancelable_CompleteSender_TopLevel(){};
			SERcancelable_CompleteSender_TopLevel(const string& name, BaseState* parent, SenderSS* mgr);

			virtual void onMessage(const Mesg& mesg);

			virtual void onEntry();
			virtual void onExit();

		//Transitions

		private:
			void onKB_CMessage(const Mesg& mesg);
			void onSERMessage(const Mesg& mesg);
	};

	class EOT1_SERcancelable : public virtual SERcancelable_CompleteSender_TopLevel
	{
			typedef SERcancelable_CompleteSender_TopLevel super;

		public:
			EOT1_SERcancelable(){};
			EOT1_SERcancelable(const string& name, BaseState* parent, SenderSS* mgr);

			virtual void onMessage(const Mesg& mesg);

			virtual void onEntry();
			virtual void onExit();

		//Transitions

		private:
			void onSERMessage(const Mesg& mesg);
	};

	class EOTEOT_SERcancelable : public virtual SERcancelable_CompleteSender_TopLevel
	{
			typedef SERcancelable_CompleteSender_TopLevel super;

		public:
			EOTEOT_SERcancelable(){};
			EOTEOT_SERcancelable(const string& name, BaseState* parent, SenderSS* mgr);

			virtual void onMessage(const Mesg& mesg);

			virtual void onEntry();
			virtual void onExit();

		//Transitions

		private:
			void onSERMessage(const Mesg& mesg);
			void onTMMessage(const Mesg& mesg);
	};

	class ACKNAK_SERcancelable : public virtual SERcancelable_CompleteSender_TopLevel
	{
			typedef SERcancelable_CompleteSender_TopLevel super;

		public:
			ACKNAK_SERcancelable(){};
			ACKNAK_SERcancelable(const string& name, BaseState* parent, SenderSS* mgr);

			virtual void onMessage(const Mesg& mesg);

			virtual void onEntry();
			virtual void onExit();

		//Transitions

		private:
			void onSERMessage(const Mesg& mesg);
	};

	class START_SERcancelable : public virtual SERcancelable_CompleteSender_TopLevel
	{
			typedef SERcancelable_CompleteSender_TopLevel super;

		public:
			START_SERcancelable(){};
			START_SERcancelable(const string& name, BaseState* parent, SenderSS* mgr);

			virtual void onMessage(const Mesg& mesg);

			virtual void onEntry();
			virtual void onExit();

		//Transitions

		private:
			void onSERMessage(const Mesg& mesg);
			void onKB_CMessage(const Mesg& mesg);
	};

	class CAN_CompleteSender_TopLevel : public virtual CompleteSender_TopLevel_SenderSS
	{
			typedef CompleteSender_TopLevel_SenderSS super;

		public:
			CAN_CompleteSender_TopLevel(){};
			CAN_CompleteSender_TopLevel(const string& name, BaseState* parent, SenderSS* mgr);

			virtual void onMessage(const Mesg& mesg);

			virtual void onEntry();
			virtual void onExit();

		//Transitions

		private:
			void onTMMessage(const Mesg& mesg);
			void onSERMessage(const Mesg& mesg);
			void onKB_CMessage(const Mesg& mesg);
	};

};

#endif

//___________________________________vv^^vv___________________________________
