/*************************************************************************
	> File Name: tnode_adapter.h
	> Author: xyz
	> Mail: xiao13149920@foxmail.com 
	> Created Time: Mon 06 Feb 2017 06:04:58 PM CST
 ************************************************************************/
#ifndef SOLUTIONGATEWAY_TNODE_ADAPTER_H_
#define SOLUTIONGATEWAY_TNODE_ADAPTER_H_
#include<atomic>
#include<vector>
#include<mutex>
#include<memory>

#include"tnode.h"
#include"define.h"
#include "ordersvr_subscribe.h"

using namespace snetwork_xservice_tnode;
//namespace snetwork_xservice_solutiongateway {

/*TNodeAministrator begin ****/

class TNodeAministrator {
	public:
		~TNodeAministrator();
	private:
		TNodeAministrator();

		TNodeAministrator(const TNodeAministrator& r) = delete;
		TNodeAministrator& operator=(const TNodeAministrator& r) = delete;
		TNodeAministrator(TNodeAministrator&& r) = delete;
		TNodeAministrator& operator=(TNodeAministrator&& r) = delete;

	public:
		static TNodeAministrator* GetInstance(void);

	public:
#if defined(SIGNALENGINE)
		//snetwork_signalengine_marketdata::MarketDataAgent* GetMarketDataAgent(void) const {
			//return m_agent;
		}
#endif
		//snetwork_xservice_tnode::TNodeChannel* GetChannel(void);
		//snetwork_xservice_tnode::TNodeChannel GetChannel(void);
		void PublishToMQ(const char* proutingkey,char* msg, unsigned int len);
		bool AddConsumer(TNodeConsumer* pConsumer);
		void Run();
		TNode* GetConsumerTnode(void) const {
			return m_consumertnode;
		}
		TNode* GetPublishTnode(void) const {
			return m_publishtnode;
		}
	private:
		//const std::string m_sendOrderGroupName;
		//const std::string m_sendOrderGroupEvent;
		static TNodeAministrator* m_instance;
		static std::mutex m_mutex;
		//std::vector<snetwork_xservice_tnode::TNodeChannel*> m_channel;
		std::atomic<int> m_atomic;
		TNode *m_consumertnode;
		TNode *m_publishtnode;
		//std::mutex m_mutex;
#if defined(SIGNALENGINE)
		//snetwork_signalengine_marketdata::MarketDataAgent* m_agent;
#endif
};
/*TNodeAministrator end ****/


 class Subscribe;

 

/*NewOrderConsumer begin ****/
class NewOrderConsumer : public snetwork_xservice_tnode::TNodeConsumer {
	public:
		explicit NewOrderConsumer(const char *bindingkey=ORDERSVRBINDINGKEY_NEWORDER, const char *queuename=QUEUENAME_NEWORDER);
		virtual ~NewOrderConsumer();

		NewOrderConsumer(const NewOrderConsumer& r) = delete;
		NewOrderConsumer& operator=(const NewOrderConsumer& r) = delete;
		NewOrderConsumer(NewOrderConsumer&& r) = delete;
		NewOrderConsumer& operator=(NewOrderConsumer&& r) = delete;

	public:
		virtual unsigned int ConsumerData( char *pMsg,int nMsgLen) override;
		std::string GetBindingkey(){ return m_bindingkey ; }
		std::string GetQueueName(){ return m_queuename ; }
	private:
		std::string	m_bindingkey ;
		std::string	m_queuename  ;
		Subscribe* m_strategy; 
		NewOrderScribe* m_NewOrder ;
		RevokeOrderScribe* m_revokeOrder ;
};
/*NewOrderConsumer end ****/

/*CheckOrderConsumer begin ***/
class CheckOrderConsumer : public snetwork_xservice_tnode::TNodeConsumer {
	public:
		explicit CheckOrderConsumer(const char *bindingkey=ORDERSVRBINDINGKEY_CHECK, const char *queuename=QUEUENMAE_CHECKORDER);
		virtual ~CheckOrderConsumer();

		CheckOrderConsumer(const CheckOrderConsumer& r) = delete;
		CheckOrderConsumer& operator=(const CheckOrderConsumer& r) = delete;
		CheckOrderConsumer(CheckOrderConsumer&& r) = delete;
		CheckOrderConsumer& operator=(CheckOrderConsumer&& r) = delete;

	public:
		virtual unsigned int ConsumerData( char *pMsg,int nMsgLen) override;
		std::string GetBindingkey(){ return m_bindingkey ; }
		std::string GetQueueName(){ return m_queuename ; }
	private:
		std::string	m_bindingkey ;
		std::string	m_queuename  ;
		Subscribe* m_strategy;
		CheckOrderScribe* m_check;

};
/*CheckOrderConsumer end ***/

 

/*TransactionConsumer begin***/
class TransactionConsumer : public snetwork_xservice_tnode::TNodeConsumer {
	public:
		explicit TransactionConsumer(const char *bindingkey=ORDERSVRBINDINGKEY_TRANSACTION, const char *queuename=QUEUENMAE_TRANSACTION);
		virtual ~TransactionConsumer();

		TransactionConsumer(const TransactionConsumer& r) = delete;
		TransactionConsumer& operator=(const TransactionConsumer& r) = delete;
		TransactionConsumer(TransactionConsumer&& r) = delete;
		TransactionConsumer& operator=(TransactionConsumer&& r) = delete;

	public:
		virtual unsigned int ConsumerData( char *pMsg,int nMsgLen) override;
		std::string GetBindingkey(){ return m_bindingkey ; }
		std::string GetQueueName(){ return m_queuename ; }
	private:
		std::string	m_bindingkey ;
		std::string	m_queuename  ;
		Subscribe* m_strategy;
		OrderACKScribe* m_orderACK;
		RevokeACKScribe* m_revokeACK ;
		RevokeRelpyScribe* m_revokeReply;
		OrderReplyScribe* m_orderReply;
};
/*TransactionConsumer end***/

 

 
 

/*UpdateUseInfoConsumer begin***/
class UpdateUseInfoConsumer : public snetwork_xservice_tnode::TNodeConsumer {
	public:
		explicit UpdateUseInfoConsumer(const char *bindingkey=ORDERSVRBINDINGKEY_UPDATE, const char *queuename=QUEUENAME_UPDATEUSERINFO);
		virtual ~UpdateUseInfoConsumer();

		UpdateUseInfoConsumer(const UpdateUseInfoConsumer& r) = delete;
		UpdateUseInfoConsumer& operator=(const UpdateUseInfoConsumer& r) = delete;
		UpdateUseInfoConsumer(UpdateUseInfoConsumer&& r) = delete;
		UpdateUseInfoConsumer& operator=(UpdateUseInfoConsumer&& r) = delete;

	public:
		virtual unsigned int ConsumerData( char *pMsg,int nMsgLen) override;
		std::string GetBindingkey(){ return m_bindingkey ; }
		std::string GetQueueName(){ return m_queuename ; }
	private:
		std::string	m_bindingkey ;
		std::string	m_queuename  ;
		Subscribe* m_strategy;
		//RevokeOrderScribe* m_revokeOrder;
		UpdateTradePwdScribe* m_update;
};
/*UpdateTradePwdConsumer end***/



#endif // end of SOLUTIONGATEWAY_TNODE_ADAPTER_H_

