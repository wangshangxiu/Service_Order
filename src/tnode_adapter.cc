/*************************************************************************
	> File Name: tnode_adapter.cc
	> Author: xyz
	> Mail: xiao13149920@foxmail.com 
	> Created Time: Wed 08 Feb 2017 11:17:29 AM CST
 ************************************************************************/
#include<mutex>
#include<new>
#include<string.h>
#include<type_traits>
#include"tnode_adapter.h"
//#include"define.h"
#include"ThreadPool.h"
#include<rapidjson/document.h>
#include<rapidjson/writer.h>
#include<rapidjson/rapidjson.h>
#include<rapidjson/stringbuffer.h>


#if defined(DPOSITION)
#include"ordersvr_subscribe.h"
#include"solution_config.h"
#endif

#if defined(DSOLUTION)
#include"solution_config.h"
#include"ordersvr_subscribe.h"
#endif
 
TNodeAministrator* TNodeAministrator::m_instance = nullptr;
std::mutex TNodeAministrator::m_mutex;
extern ThreadPool* g_NewOrderthreadPool;
extern ThreadPool* g_CheckOrderthreadPool ;
extern ThreadPool* g_OrderPool ;
extern ThreadPool* g_OrderRevokePool;
extern ThreadPool* g_UpdatethreadPool ;
using namespace snetwork_xservice_xflagger;
TNodeAministrator* TNodeAministrator::GetInstance(void) {
	if (m_instance == nullptr) {
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_instance == nullptr) {
			m_instance = new TNodeAministrator();
		}
	}

	return m_instance;
}

TNodeAministrator::TNodeAministrator() { 
	 
	SXConfig* sxconfig = dynamic_cast<SXConfig*>(SXFlagger::GetInstance()->GetConfig());
	STTnodeConfig tnodeconfig ;
	tnodeconfig.mq_vhost= sxconfig->VHost() ;
	tnodeconfig.mq_exchange_group = sxconfig->ExchangeGroup();
	tnodeconfig.mq_host = sxconfig->Address();
	tnodeconfig.mq_port= atoi(sxconfig->Port().c_str());
	tnodeconfig.mq_user = sxconfig->User() ;
	tnodeconfig.mq_passwd= sxconfig->Password() ;

	m_consumertnode = new TNode(tnodeconfig);

	m_publishtnode = new TNode(tnodeconfig);

	if(-1 == m_publishtnode->Init()  || -1 == m_consumertnode->Init())
	{
		XERROR("tnode init fail , pls check ! \n"); 
	}
	m_atomic = 0;
}

TNodeAministrator::~TNodeAministrator() {
 
	if(nullptr == m_consumertnode)
		delete m_consumertnode;
	if(nullptr == m_publishtnode)
		delete m_publishtnode;

}
 
void TNodeAministrator::PublishToMQ(const char* proutingkey, char* msg, unsigned int len) {
	std::unique_lock<std::mutex> lk(m_mutex);
	m_publishtnode->PublishToMQ( proutingkey, msg,len);
#if defined(DEBUG)
    XINFO("publish routingkey:%s msg:%s\n",proutingkey,msg );
#endif
}

bool TNodeAministrator::AddConsumer(TNodeConsumer* pConsumer){
	if(-1 == m_consumertnode->AddTNodeConsumer(pConsumer) )
	{
		XERROR("AddTNodeConsumer  ERROR \n");
	 	m_consumertnode->Close();
		return false;
	}
	return true;
}

void TNodeAministrator::Run(){
	m_consumertnode->RunConsumer();
}
/* TNodeAministrator end ****/




/* NewOrderConsumer  begin **/
NewOrderConsumer::NewOrderConsumer(const char *bindingkey, const char *queuename) :
	m_bindingkey(bindingkey),
	m_queuename(queuename) {
	m_strategy =  nullptr;
	m_NewOrder = new NewOrderScribe();
	m_revokeOrder = new RevokeOrderScribe();
}

NewOrderConsumer::~NewOrderConsumer() {
	 if(nullptr != m_NewOrder)
		 delete m_NewOrder;
	if(nullptr != m_revokeOrder)
		delete m_revokeOrder;
}
unsigned int NewOrderConsumer::ConsumerData( char *pMsg,int nMsgLen) {
	if(pMsg == NULL){
		XERROR("MQ Message is NULL");
		return -1 ;
	}
	//XINFO("Msg inforation:[%s] len[%d]",pMsg,nMsgLen);
	unsigned int eventLen = nMsgLen+1;
	//std::shared_ptr<void> msg (malloc(nMsgLen), [](void* p) {
	//	free(p);
	//});
//	char *msg = (char*)malloc(eventLen);
//	memset(msg,'\0',eventLen) ;
//	memcpy(msg, pMsg, eventLen-1);
	//XINFO("Msg inforation:[%s] len[%d]",msg,eventLen);
	 /* Parse Json */
	rapidjson::Document d;
	d.Parse(pMsg, nMsgLen);
	if (d.HasParseError() ||!d.IsObject()) {
		XERROR("parser json error|event=%s", pMsg);

		return -1;
	}

	rapidjson::Value::ConstMemberIterator it;

	if(((it = d.FindMember("Tag")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return -1;
	}
	int Tag = it->value.GetInt();
	Subscribe *strategy = nullptr;
	switch(Tag){
		case ETag::NEWORDER :
			strategy = m_NewOrder;
			break;
		case ETag::ORDERREVOKE_REQUEST :
			strategy = m_revokeOrder;
			break;
		default :
#if defined(DEBUG)	
            XINFO("tag is not match NEW ORDER: Msg:%s\n",pMsg);
#endif
			return -1 ;
			 

	}
	//Subscribe *strategy = m_strategy;
    char *msg = (char*)malloc(eventLen);
    memset(msg,'\0',eventLen) ;
    memcpy(msg, pMsg, eventLen-1);

	g_NewOrderthreadPool->Enqueue([strategy, msg, nMsgLen] {
		strategy->Done(msg, nMsgLen);
		free(msg);
		//delete strategy;
	}); 
	
	
	return 0;
}
/* NewOrderConsumer  end **/

/*  CheckOrderConsumer  begin **/
CheckOrderConsumer::CheckOrderConsumer(const char *bindingkey, const char *queuename) :
	m_bindingkey(bindingkey),
	m_queuename(queuename) {
	m_strategy = nullptr;
	m_check = new CheckOrderScribe();

}

CheckOrderConsumer::~CheckOrderConsumer() {
	 if(nullptr != m_check)
	 	delete m_check;
}
unsigned int CheckOrderConsumer::ConsumerData( char *pMsg,int nMsgLen) {
	if(pMsg == NULL){
		XERROR("MQ Message is NULL");
		return -1 ;
	}

	unsigned int eventLen = nMsgLen+1;
	//std::shared_ptr<void> msg (malloc(nMsgLen), [](void* p) {
	//	free(p);
	//});
//	char *msg = (char*)malloc(eventLen);
//	memset(msg,'\0',eventLen) ;
//	memcpy(msg, pMsg, nMsgLen);
	 /* Parse Json */
	rapidjson::Document d;
	d.Parse(pMsg, nMsgLen);
	if (d.HasParseError() ||!d.IsObject()) {
		XERROR("parser json error|event=%s", pMsg);

		return -1;
	}

	rapidjson::Value::ConstMemberIterator it;

	if(((it = d.FindMember("Tag")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return -1;
	}
	Subscribe *strategy = nullptr;
	int Tag = it->value.GetInt();
	switch(Tag){
		case ETag::ORDERCHECK_BALANCEREPLY:
			strategy = m_check;
			break;
		default :
#if defined(DEBUG)	
            XINFO("tag is not match Current Request: Msg:%s\n",pMsg);
#endif
			return -1 ;
			 

	}
	//Subscribe *strategy = m_strategy;
    char *msg = (char*)malloc(eventLen);
    memset(msg,'\0',eventLen) ;
    memcpy(msg, pMsg, nMsgLen);

	g_CheckOrderthreadPool->Enqueue([strategy, msg, nMsgLen] {
		strategy->Done(msg, nMsgLen);
		free(msg);
		//delete strategy;
	}); 
	
	
	return 0;
 
}
/* CheckOrderConsumer  end **/

 

/*TransactionConsumer  begin **/
TransactionConsumer::TransactionConsumer(const char *bindingkey, const char *queuename) :
	m_bindingkey(bindingkey),
	m_queuename(queuename),
	m_strategy(nullptr) {
	m_orderACK = new OrderACKScribe();
	m_revokeACK = new RevokeACKScribe();
	m_revokeReply = new RevokeRelpyScribe();
	m_orderReply = new OrderReplyScribe() ;

}

TransactionConsumer::~TransactionConsumer() {
	if(nullptr != m_orderACK)
		delete m_orderACK;
	if(nullptr != m_revokeACK)
		delete m_revokeACK;
	if(nullptr != m_revokeReply)
		delete m_revokeReply;
	if(nullptr != m_orderReply)
		delete m_orderReply;
}
unsigned int TransactionConsumer::ConsumerData( char *pMsg,int nMsgLen) {
	if(pMsg == NULL){
		XERROR("MQ Message is NULL");
		return 0 ;
	}
	unsigned int eventLen = nMsgLen+1;
	//std::shared_ptr<void> msg (malloc(nMsgLen), [](void* p) {
	//	free(p);
	//});
//	char *msg = (char*)malloc(eventLen);
//	memset(msg,'\0',eventLen) ;
//	memcpy(msg, pMsg, nMsgLen);
	 /* Parse Json */
	rapidjson::Document d;
	d.Parse(pMsg, nMsgLen);
	if (d.HasParseError() ||!d.IsObject()) {
		XERROR("parser json error|event=%s", pMsg);

		return -1;
	}

	rapidjson::Value::ConstMemberIterator it;

	if(((it = d.FindMember("Tag")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return -1;
	}
	int Tag = it->value.GetInt();
	Subscribe *strategy = nullptr;
	switch(Tag){
		case ETag::ORDERACK_MANAGEMENTSRV :
			strategy = m_orderACK;
			break;
        case ETag::ORDERTRANSACTION_REPLY:
            strategy = m_orderReply;
            break;
		case ETag::ORDERREVOKE_ACK :
			strategy = m_revokeACK;
			break;
		case ETag::ORDERREVOKE_REPLY :
			strategy = m_revokeReply;
			break;
		default :
#if defined(DEBUG)	
            XINFO("tag is not match Current Request: Msg:%s\n",pMsg);
#endif

            return -1 ;
			 

	}
	//Subscribe *strategy = m_strategy;

    char *msg = (char*)malloc(eventLen);
    memset(msg,'\0',eventLen) ;
    memcpy(msg, pMsg, nMsgLen);

    if (ETag::ORDERACK_MANAGEMENTSRV == Tag || ETag::ORDERTRANSACTION_REPLY == Tag) {
        g_OrderPool->Enqueue([strategy, msg, nMsgLen] {
            strategy->Done(msg, nMsgLen);
            free(msg);
        });
    } else {

        g_OrderRevokePool->Enqueue([strategy, msg, nMsgLen] {
            strategy->Done(msg, nMsgLen);
            free(msg);
        });
    }

	return 0;
}
/* TransactionConsumer  end **/

/*UpdateUseInfoConsumer  begin **/
UpdateUseInfoConsumer::UpdateUseInfoConsumer(const char *bindingkey, const char *queuename) :
	m_bindingkey(bindingkey),
	m_queuename(queuename) {
	//m_revokeOrder = new RevokeOrderScribe();
	m_update = new UpdateTradePwdScribe();
}

UpdateUseInfoConsumer::~UpdateUseInfoConsumer() {
	//if(nullptr != m_revokeOrder)
		//delete m_revokeOrder;
	if(nullptr != m_update)
		delete m_update; 
}
unsigned int UpdateUseInfoConsumer::ConsumerData( char *pMsg,int nMsgLen) {
	if(pMsg == NULL){
		XERROR("MQ Message is NULL");
		return 0 ;
	}
	unsigned int eventLen = nMsgLen+1;
	//std::shared_ptr<void> msg (malloc(nMsgLen), [](void* p) {
	//	free(p);
	//});
//	char *msg = (char*)malloc(eventLen);
//	memset( msg,0x00,eventLen) ;
//	memcpy(msg, pMsg, nMsgLen);
	 /* Parse Json */
	rapidjson::Document d;
	d.Parse(pMsg, nMsgLen);
	if (d.HasParseError() ||!d.IsObject()) {
		XERROR("parser json error|event=%s", pMsg);

		return -1;
	}

	rapidjson::Value::ConstMemberIterator it;

	if(((it = d.FindMember("Tag")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return -1;
	}
	int Tag = it->value.GetInt();
	switch(Tag){
		//case ETag::ORDERREVOKE_REQUEST :
			//m_strategy = m_revokeOrder;
			//break;
		case ETag::TRADEPWD_UPDATE:
			m_strategy = m_update;
		default :
#if defined(DEBUG)	
            XINFO("tag is not match Current Request: Msg:%s\n",pMsg);
#endif
			return -1 ;
			 

	}
	Subscribe *strategy = m_strategy;
    char *msg = (char*)malloc(eventLen);
    memset( msg,0x00,eventLen) ;
    memcpy(msg, pMsg, nMsgLen);

	g_UpdatethreadPool->Enqueue([strategy, msg, eventLen] {
		strategy->Done(msg, eventLen);
		free(msg);
		delete strategy;
	}); 
	
	return 0;
}
/* UpdateUseInfoConsumer  end **/














