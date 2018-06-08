/*************************************************************************
	> File Name: solution_subscribe.cc
	> Author: xyz
	> Mail: xiao13149920@foxmail.com 
	> Created Time: Tue 28 Feb 2017 07:56:30 PM CST
 ************************************************************************/
#include"ordersvr_subscribe.h"
#include<mutex>
#include<string.h>
#include<type_traits>
#include"tnode_adapter.h"
#include"define.h"
#include"ThreadPool.h"
#include<rapidjson/document.h>
#include<rapidjson/writer.h>
#include<rapidjson/rapidjson.h>
#include<rapidjson/stringbuffer.h>
 

 
#include"mysqlop.h"
#include"redisop.h"
 
#include"solution_config.h"
#include"xdatetime.h"
#include"object_pool.h"
#include <sys/types.h>
 
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <map>
#include <thread>
using namespace snetwork_xservice_xflagger;
//using namespace snetwork_xservice_solutiongateway;
//using MySqlRecordSet = snetwork_xservice_db::MySqlRecordSet;
//using MySqlDB = MySqlDB;
using RedisPool = ObjectPool<CRedis,15>; //ObjectPool<snetwork_xservice_db::Redis, 5>;

extern SXConfig* sxconfig;
extern std::map<int,std::string> g_pwdMap;
extern RedisPool* g_redisPool;
extern std::map<int,std::string> g_webInfo ;
//extern CRedisLock redisLock;

Subscribe::Subscribe(){
	
}
 

/*NewOrderScribe   begin ****/
NewOrderScribe::NewOrderScribe(){
	
}

void NewOrderScribe::CreateOrderID(std::string& orderid){
	long currtime = time(NULL);
	static long old_time = currtime;
	SXConfig* sxconfig = dynamic_cast<SXConfig*>(SXFlagger::GetInstance()->GetConfig());
	long svrID = atoi(sxconfig->SolutionID().c_str());
	static atomic_long seq(1);
	if(currtime != old_time){
		seq = 1 ;
	}
	else{
		seq++;
	}
	old_time = currtime;
	//long seq = this_thread::get_id();
	long ordertmpID = 0 ;
	ordertmpID |= svrID<<53;
	ordertmpID |= currtime<<16;
	ordertmpID |= seq ;
	orderid = to_string(ordertmpID);
	

} 
 void NewOrderScribe::Done(const char* event, unsigned int eventLen){
	  /* Parse Json */
    XINFO("WS --> OrderSrv:len:%u, json[%s]\n",eventLen,event);
	  
	rapidjson::Document d;
	d.Parse(event, eventLen);
	if (d.HasParseError() ||
		!d.IsObject()) {
		XERROR("parser json error|event=%s", event);

		return;
	}

	
	
	rapidjson::Value::ConstMemberIterator it;

	if(((it = d.FindMember("UserID")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	int userid = it->value.GetInt();

	if(((it = d.FindMember("TradePwd")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	const char* TradPWD = it->value.GetString();

	if(((it = d.FindMember("WSID")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	int WebID = it->value.GetInt();
	 
	 if(((it = d.FindMember("RequestID")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif
	
		return;
	}
	const char* RequestID = it->value.GetString(); 

	if(((it = d.FindMember("Symbol")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif
	
		return;
	}
	const char* Symbol = it->value.GetString(); 

	if(((it = d.FindMember("TransType")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif
	
		return;
	}
	int TransType = it->value.GetInt(); 
	//
	if(((it = d.FindMember("OrderType")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif	
		return;
	}
	int OrderType = it->value.GetInt();

	if(((it = d.FindMember("OrderNumber")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif
	
		return;
	}
	double OrderNumber = atof(it->value.GetString());  

	if(((it = d.FindMember("Price")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif
	
		return;
	}

	double Price = atof(it->value.GetString()); 

	if(((it = d.FindMember("SessionID")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif
			
				return;
	}

	
	
	const char* sessionId = it->value.GetString();
	// check trade password 
	char sSQL[200]={0};
    sprintf(sSQL,"select TradePassword from User where ID = %ld;\n",userid);
   	MySqlDB db ;
	MySqlRecordSet rs ;
  	db.Open();
	db.SelectDB(dynamic_cast<SXConfig*>(SXFlagger::GetInstance()->GetConfig())->MySqlDBName().c_str());

	rs = db.QuerySql(sSQL);
	if (db.IsError()) {
#if defined(DEBUG)
		XERROR("err=%s, con=%d", db.GetMySqlErrMsg(), db.IsConnect());
#endif
		db.Close();
	} else {
		db.Close();
	}
	size_t rows = rs.GetRows();
   	if(rows <= 0 || rows != 1)
   	{
	   	XERROR("the UserID = %ld has more than one or none trade pwd",userid);
	   	return  ; 
   	} 
	std::string tradepwd = rs.GetFieldByID(0,0);
    std::cout << tradepwd << std::endl;
	//std::string tradepwd = redis->Excute("get",to_string(userid).c_str()).get()->str;
	if(tradepwd.compare(TradPWD) != 0)  //  must change  
	{
		char routingkey[50] = { 0 };
		//  trade pwd is error 
		snprintf(routingkey,sizeof(routingkey),"%s.%d",ROUTINGKEY_WEB,WebID);
		char buffer[200] = { 0 };
        snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"RequestID\":%s,\"SessionID\":\"%s\",\"Status\":%d}",
                 ETag::NEWORDERANS,RequestID,sessionId, STATUS_CODE::TRADINGCODERROR) ;
		size_t len = strlen(buffer) +1;	
		TNodeAministrator::GetInstance()->PublishToMQ(routingkey,buffer,len);
        XINFO("trade pwd error buff :%s\n",buffer);
		return ;
	}

	/*  Note by savin on 20180425
	std::map<int,std::string>::iterator iter =  g_pwdMap.find(userid);
	bool isCheck = true;
	//bool isCheck = false;
	if(iter != g_pwdMap.end()){
		if (iter->second.compare(TradPWD) == 0 ){
			isCheck = true;
		}
	}
	*/
	
	//rapidjson::Document doc;  
    //doc.SetObject();  
    //rapidjson::Document::AllocatorType &allocator=doc.GetAllocator();     
	//doc.AddMember("author","tashaxing",allocator);  
	
	// create Order ID 
	std::string OrderID  ; 
	CreateOrderID(OrderID);
    XINFO("OrderID[%s]\n",OrderID.c_str());
	// write order and web info  to redis
	CRedis* redis = g_redisPool->GetObject();
	redis->HSet(OrderID.c_str(), "wsId", to_string(WebID).c_str());
	redis->HSet(OrderID.c_str(),"sessionId",sessionId);
	redis->HSet(OrderID.c_str(), "UserID", to_string(userid).c_str());
	redis->HSet(OrderID.c_str(),"Symbol",Symbol);
	redis->HSet(OrderID.c_str(),"OrderType",to_string(OrderType).c_str());
	redis->HSet(OrderID.c_str(),"TransType",to_string(TransType).c_str());
	redis->HSet(OrderID.c_str(),"OrderNumber",to_string(OrderNumber).c_str());
	redis->HSet(OrderID.c_str(),"Price",to_string(Price).c_str());
	time_t now = time(NULL);
    cout << "now:" << now << endl;
	redis->HSet(OrderID.c_str(),"OrderTime",to_string(now).c_str());
	redis->HSet(OrderID.c_str(),"OrderStatus",to_string(O_Status::ORDER_READY).c_str());
    redis->HSet(OrderID.c_str(),"BalanceChecked","0");
    redis->HSet(OrderID.c_str(),"OrderAck","0");

    redis->HSet(OrderID.c_str(),"RemainNum", to_string(OrderNumber).c_str());
    redis->HSet(OrderID.c_str(),"Total","0");



// publish msg to ManageMent
	char buffer [500] = { 0 };
    snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"UserID\":%d,\"OrderID\":%s,\"Symbol\":\"%s\","
                                   "\"OrderType\":%d,\"TransType\":%d,\"OrderNumber\":%.9f,"
                                   "\"Price\":%.9f,\"SrvIdentify\":\"%s\"}",
															ETag::ORDERCHECK_BALANCE,
															userid,
															OrderID.c_str(),
															Symbol,
															OrderType,
															TransType,
															OrderNumber,
															Price,
															ORDERSVRBINDINGKEY_CHECK);
	 
	size_t len = strlen(buffer) +1;	
	TNodeAministrator::GetInstance()->PublishToMQ(ROUTINGKEY_MANAGEMENTSRV,buffer,len);
    XINFO("OrderSrv-->ManagerSrv Assets Freeze:%s\n",buffer);
	//return ;

	/*
	std::string SessionId = redis->HGet(OrderID.c_str(),"sessionId");
	std::string UserID = redis->HGet(OrderID.c_str(),"UserID");
	std::string Type = redis->HGet(OrderID.c_str(),"OrderType");
	std::string Number = redis->HGet(OrderID.c_str(),"OrderNumber");
	std::string price = redis->HGet(OrderID.c_str(),"Price");
	std::string symbol = redis->HGet(OrderID.c_str(),"Symbol");
	//std::string TransType = redis->HGet(OrderID.c_str(),"TransType");
	std::string wsId = redis->HGet(OrderID.c_str(),"wsId");
	 
	if (SessionId.empty() || UserID.empty() || Type.empty() || Number.empty() || price.empty() || symbol.empty() || wsId.empty())
	{
		XERROR("the OrderID = %s insert failed ",OrderID.c_str());
		return ;
	}*/
	g_redisPool->ReleaseObject(redis);

 }

 /*NewOrderScribe   end ****/


/*OrderACKScribe begin ****/
 OrderACKScribe::OrderACKScribe(){

 }

 

 void OrderACKScribe::Done(const char* event, unsigned int eventLen){//two statue:0 successfully, 1 ME refuse
	/* Parse Json */
    XINFO("OrderAPI --> OrderSrv  New Order ACK : [%s]\n",event);
	rapidjson::Document d;
	d.Parse(event, eventLen);
	if (d.HasParseError() ||
		!d.IsObject()) {
		XERROR("parser json error|event=%s", event);

		return;
	}

	rapidjson::Value::ConstMemberIterator it;

	if(((it = d.FindMember("OrderID")) == d.MemberEnd()) || !it->value.IsInt64()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	long OrderID = it->value.GetInt64();

	if(((it = d.FindMember("ExcelID")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	const char* ExcelID = it->value.GetString(); 

	if(((it = d.FindMember("OrderIDFrME")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif
		return;
	}
	const char* OrderIDFrME = it->value.GetString(); 

	if(((it = d.FindMember("OrderStatus")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	int OrderStatus = it->value.GetInt();

	if(((it = d.FindMember("TransactTime")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}

	const char* TransactTime = it->value.GetString();

	if(((it = d.FindMember("Text")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}

	const char* Text = it->value.GetString();

    if (OrderStatus  == 1)//0 sussessfully, 1 ME refuse
	{
		CRedis* redis = g_redisPool->GetObject();
		std::string UserID = redis->HGet(to_string(OrderID).c_str(),"UserID");
		std::string OrderType = redis->HGet(to_string(OrderID).c_str(),"OrderType");
		std::string ExcelID = redis->HGet(to_string(OrderID).c_str(),"ExcelID");
		std::string OrderIDFrME = redis->HGet(to_string(OrderID).c_str(),"OrderIDFrME");
		std::string OrderNumber = redis->HGet(to_string(OrderID).c_str(),"OrderNumber");
		std::string Price = redis->HGet(to_string(OrderID).c_str(),"Price");
		std::string Symbol = redis->HGet(to_string(OrderID).c_str(),"Symbol");
		std::string TransType = redis->HGet(to_string(OrderID).c_str(),"TransType");
		char buffer [500] = { 0 };
        snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"UserID\":%s,\"OrderID\":\"%ld\",\"Symbol\":"
                                       "\"%s\",\"OrderType\":%s,\"TransType\":%s,\"OrderNumber\":%s,"
                                       "\"Price\":%s,\"SrvIdentify\":\"%s\"}",
                                                                ETag::ORDERCHECK_BALANCE_UNFREEZE,
																UserID.c_str(),
																OrderID,
																Symbol.c_str(),
																OrderType.c_str(),
																TransType.c_str(),
																OrderNumber.c_str(),
																Price.c_str(),
																ORDERSVRBINDINGKEY_CHECK);
		size_t len = strlen(buffer) +1;	
        TNodeAministrator::GetInstance()->PublishToMQ(ROUTINGKEY_MANAGEMENTSRV,buffer,len);
        XINFO("OrderSrv -->ManagerSrv:ME refuse,unfreeze msg, b: %s\n",buffer);

		memset(buffer,0x00,sizeof(buffer));
		char routingkey[50] = { 0 };
		 
		std::string wsId = redis->HGet(to_string(OrderID).c_str(),"wsId");
		std::string sessionId = redis->HGet(to_string(OrderID).c_str(),"sessionId");
		snprintf(routingkey,sizeof(routingkey),"%s,%s",ROUTINGKEY_WEB,wsId.c_str());
        snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"SessionId\":\"%s\",\"UserID\":%s,\"OrderID\":"
                                       "\"%ld\",\"Status\":%d}", ETag::NEWORDERANS,
                                        sessionId.c_str(),UserID.c_str(),OrderID,STATUS_CODE::MEREFUSE);//ME refuse
		//char *buffer = const_cast<char*>(event) ;
		len = strlen(buffer) +1;	
		TNodeAministrator::GetInstance()->PublishToMQ(routingkey,buffer,len);
		redis->Del(to_string(OrderID).c_str()) ;	
        XINFO("OrderSrv -->WS :ME refuse order b: %s\n",buffer);
		g_redisPool->ReleaseObject(redis);


		// the order has been refused 
		//TNodeAministrator::GetInstance()->PublishToMQ();
		//XINFO("the order [%ld] is been refused ",OrderID);
		return ;
	}
	
	CRedis* redis = g_redisPool->GetObject();
	std::string UserID = redis->HGet(to_string(OrderID).c_str(),"UserID");
	std::string OrderType = redis->HGet(to_string(OrderID).c_str(),"OrderType");
	std::string OrderNumber = redis->HGet(to_string(OrderID).c_str(),"OrderNumber");
	std::string Price = redis->HGet(to_string(OrderID).c_str(),"Price");
	std::string Symbol = redis->HGet(to_string(OrderID).c_str(),"Symbol");
	std::string TransType = redis->HGet(to_string(OrderID).c_str(),"TransType");
	std::string wsId = redis->HGet(to_string(OrderID).c_str(),"wsId");
	std::string sessionId = redis->HGet(to_string(OrderID).c_str(),"sessionId");

	redis->HSet(to_string(OrderID).c_str(),"ExcelID",ExcelID);
	redis->HSet(to_string(OrderID).c_str(),"OrderIDFrME",OrderIDFrME);
	redis->HSet(to_string(OrderID).c_str(),"TransactTime",TransactTime);
    redis->HSet(to_string(OrderID).c_str(),"OrderStatus",to_string(O_Status::ORDER_PINDING).c_str());
    redis->HSet(to_string(OrderID).c_str(),"OrderAck","1");

    if(redis->SelectDB(2)) {
        redis->Excute("sadd %s %ld",UserID.c_str(),OrderID);//add to set of userid
    }

	// public to web  the order is penging 
	char buffer[500] =  { 0 };
    snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"OrderID\":\"%ld\",\"SessionID\":\"%s\",\"UserID\":%s,"
                                   "\"Symbol\":\"%s\",\"OrderType\":%s,\"TransType\":%s,\"OrderNumber\":%s,"
                                   "\"Price\":%s,\"Status\":%d}",
									ETag::ORDERACK_PINDING,
									OrderID,
									sessionId.c_str(),
									UserID.c_str(),
									Symbol.c_str(),
									OrderType.c_str(),
									TransType.c_str(),
									OrderNumber.c_str(),
                                    Price.c_str(),
                                    OrderStatus //pending successfully
	);
	char routingkey[50] = { 0 };
	snprintf(routingkey,sizeof(routingkey),"%s.%s",ROUTINGKEY_WEB,wsId.c_str());
	size_t len = strlen(buffer) +1;	
	TNodeAministrator::GetInstance()->PublishToMQ(routingkey,buffer,len);
    XINFO("OrderSrv --> WS new order ack msg: %s\n",buffer);

    redis->SelectDB(0);
	g_redisPool->ReleaseObject(redis);
 }
 /*OrderACKScribe end ****/

 /*OrderReplyScribe begin ****/
 OrderReplyScribe::OrderReplyScribe(){

 }
 
  

void OrderReplyScribe::Done(const char* event, unsigned int eventLen){
	/* Parse Json */
	XINFO("MarketAPI --> OrderSrv Response msg:%s\n",event);
	rapidjson::Document d;
	d.Parse(event, eventLen);
	if (d.HasParseError() ||
		!d.IsObject()) {
		XERROR("parser json error|event=%s", event);

		return;
	}

	rapidjson::Value::ConstMemberIterator it;

	if(((it = d.FindMember("AskOrderID")) == d.MemberEnd()) || !it->value.IsInt64()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	long AskOrderID = it->value.GetInt64();

	if(((it = d.FindMember("BidOrderID")) == d.MemberEnd()) || !it->value.IsInt64()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	long BidOrderID = it->value.GetInt64();

	if(((it = d.FindMember("Symbol")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	const char* Symbol = it->value.GetString();

	if(((it = d.FindMember("TransType")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	int TransType = it->value.GetInt();

	if(((it = d.FindMember("DealNum")) == d.MemberEnd()) || !it->value.IsDouble()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	double DealNum = it->value.GetDouble();

	if(((it = d.FindMember("DealPrice")) == d.MemberEnd()) || !it->value.IsDouble()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	double DealPrice = it->value.GetDouble();

	if(((it = d.FindMember("TransactTime")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	const char* TransactTime = it->value.GetString();
	
	CRedis* redis = g_redisPool->GetObject();

	std::string AskUserID = redis->HGet(to_string(AskOrderID).c_str(),"UserID");
	std::string AskOrderType = redis->HGet(to_string(AskOrderID).c_str(),"OrderType");
	std::string AskOrderNumber = redis->HGet(to_string(AskOrderID).c_str(),"OrderNumber");
	std::string AskPrice = redis->HGet(to_string(AskOrderID).c_str(),"Price");
    std::string AskTransType = redis->HGet(to_string(AskOrderID).c_str(), "TransType");
	std::string AskOrderTime = redis->HGet(to_string(AskOrderID).c_str(),"OrderTime");
	std::string AskOrderStatus = redis->HGet(to_string(AskOrderID).c_str(),"OrderStatus");
    std::string AskOrderIDFrME = redis->HGet(to_string(AskOrderID).c_str(),"OrderIDFrME");
    std::string AskExcelID = redis->HGet(to_string(AskOrderID).c_str(),"ExcelID");
    std::string AskSessionID = redis->HGet(to_string(AskOrderID).c_str(),"sessionId");
    double AskRemainNum = atof((redis->HGet(to_string(AskOrderID).c_str(),"RemainNum")).c_str());  // atof -> double or float ?
    AskRemainNum = AskRemainNum - DealNum;
    double AskTotal = atof((redis->HGet(to_string(AskOrderID).c_str(),"Total")).c_str());
    AskTotal = AskTotal + DealNum*DealPrice;
    double AskAverPrice =  AskTotal / (atof(AskOrderNumber.c_str())-AskRemainNum) ;

	std::string BidUserID = redis->HGet(to_string(BidOrderID).c_str(),"UserID");
	std::string BidOrderType = redis->HGet(to_string(BidOrderID).c_str(),"OrderType");
	std::string BidOrderNumber = redis->HGet(to_string(BidOrderID).c_str(),"OrderNumber");
	std::string BidPrice = redis->HGet(to_string(BidOrderID).c_str(),"Price");
    std::string BigTransType = redis->HGet(to_string(BidOrderID).c_str(), "TransType");
	std::string BidOrderTime = redis->HGet(to_string(BidOrderID).c_str(),"OrderTime");
	std::string BidOrderStatus = redis->HGet(to_string(BidOrderID).c_str(),"OrderStatus");
	std::string BidOrderIDFrME = redis->HGet(to_string(BidOrderID).c_str(),"OrderIDFrME");
	std::string BidExcelID = redis->HGet(to_string(BidOrderID).c_str(),"ExcelID");
    std::string BidSessionID = redis->HGet(to_string(BidOrderID).c_str(),"sessionId");
    double BidRemainNum = atof((redis->HGet(to_string(BidOrderID).c_str(),"RemainNum")).c_str());
    BidRemainNum = BidRemainNum - DealNum;
    double BidTotal = atof((redis->HGet(to_string(BidOrderID).c_str(),"Total")).c_str());
    BidTotal += DealNum*DealPrice;
    double BidAverPrice = BidTotal/(atof(BidOrderNumber.c_str())-BidRemainNum) ;

//    while (redis->Lock(redisLock) == 0) {
//        usleep(3000);
//    }
//    while (!redis->Lock(redisLock)) {
//        usleep(3000);
//    }


	
	 
	
	//  Record to mysql 
	//MySqlRecordSet rs_position;
	MySqlDB db;
	db.Open();
	db.SelectDB(dynamic_cast<SXConfig *>(SXFlagger::GetInstance()->GetConfig())->MySqlDBName().c_str());
	db.StartTransaction();
    char sqlbuf[1024] = { 0 };
	
	snprintf(sqlbuf,sizeof(sqlbuf),"insert into token_order(ID,UserID,DelegateType,Type,Symbol,Price,Amount,TradeAmount,AveragePrice,OrderTime, \
                                    LastTraderTime,OrderIDFrME,ExcelID,Status) values (%ld,%s,%s,%s,'%s',%s,%s,%.9f,%.9f,%s,%s,'%s','%s',%d) ON DUPLICATE KEY\
                                    UPDATE AveragePrice = %.9f,TradeAmount = TradeAmount + %.9f,LastTraderTime = %s;",
																AskOrderID,AskUserID.c_str(),AskOrderType.c_str(),
                                                                AskTransType.c_str(),Symbol,AskPrice.c_str(),
																AskOrderNumber.c_str(),DealNum,AskAverPrice,
																AskOrderTime.c_str(), TransactTime,AskOrderIDFrME.c_str(),
                                                                AskExcelID.c_str(),O_Status::ORDER_TRADED,AskAverPrice,DealNum,TransactTime);
//    XINFO("%s is %d", sqlbuf ,strlen(sqlbuf));
	ssize_t rows = db.ExecSql(sqlbuf);
	if (rows == -1)
	{
        XERROR("%s,con=%d|sql=%s\n", db.GetMySqlErrMsg(), db.IsConnect(), sqlbuf);
		db.Commint();
		db.Close();
		return ;
	}
    XINFO("Ask sql:%s\n",sqlbuf);

	memset(sqlbuf,0x00,sizeof(sqlbuf));
	snprintf(sqlbuf,sizeof(sqlbuf),"insert into token_order(ID,UserID,DelegateType,Type,Symbol,Price,Amount,TradeAmount,AveragePrice,OrderTime, \
        LastTraderTime,OrderIDFrME,ExcelID,Status)  \
        values(%ld,%s,%s,%s,'%s',%s,%s,%.9f,%.9f, %s, %s,'%s','%s',%d) ON DUPLICATE KEY UPDATE AveragePrice = %.9f,\
        TradeAmount = TradeAmount + %.9f,LastTraderTime = %s;",
																BidOrderID,BidUserID.c_str(),BidOrderType.c_str(),
                                                                BigTransType.c_str(),Symbol,BidPrice.c_str(),
																BidOrderNumber.c_str(),DealNum,DealPrice,
																BidOrderTime.c_str(), TransactTime,BidOrderIDFrME.c_str(),
                                                                BidExcelID.c_str(),O_Status::ORDER_TRADED,BidAverPrice,DealNum,
																TransactTime
    );
	rows = db.ExecSql(sqlbuf);
	if (rows == -1)
	{
        XERROR("%s,con=%d|sql=%s\n", db.GetMySqlErrMsg(), db.IsConnect(), sqlbuf);
	}
	
    XINFO("Bid sql:%s\n",sqlbuf);
	memset(sqlbuf,0x00,sizeof(sqlbuf));
    snprintf(sqlbuf,sizeof(sqlbuf),"insert into Order_Child(OrderID,PeerOrderID,TradeAmount,Price,tradetime) values(%ld,%ld,%.9f,%.9f,%s)",
																AskOrderID,
																BidOrderID,
																DealNum,
																DealPrice,
																TransactTime
	);
	rows = db.ExecSql(sqlbuf);
	if(rows == -1)
	{
        XERROR("%s,con=%d|sql=%s\n", db.GetMySqlErrMsg(), db.IsConnect(), sqlbuf);
	}
	db.Commint();

	db.Close();

    XINFO("order_child sql:%s\n",sqlbuf);
	
	//return ;  // need debug  
	//redis->SelectDB(0);
    cout << "AskRemainNum: " << AskRemainNum <<endl;
    cout << "BidRemainNum: " << BidRemainNum << endl;
    cout << endl;
	if(AskRemainNum < 0.0000000001 && AskRemainNum > -0.0000000001)
	{
        redis->Del(to_string(AskOrderID).c_str()) ;
        if (redis->SelectDB(2)) {
            redis->Excute("srem %s %ld",AskUserID.c_str(),AskOrderID);
            //redis->SelectDB(0);
        }
			//  Set 集合未删除	 
	}
	else{
			redis->HSet(to_string(AskOrderID).c_str(),"OrderStatus",to_string(O_Status::ORDER_UNFINISH).c_str()) ;
			redis->HSet(to_string(AskOrderID).c_str(),"RemainNum",to_string(AskRemainNum).c_str()) ;
			redis->HSet(to_string(AskOrderID).c_str(),"Total",to_string(AskTotal).c_str());
    }

	if(BidRemainNum < 0.0000000001 && BidRemainNum > -0.0000000001)
	{
        redis->Del(to_string(BidOrderID).c_str()) ;
        if (redis->SelectDB(2)) {
            redis->Excute("srem %s %ld",BidUserID.c_str(),BidOrderID);
            //redis->SelectDB(0);
        }
			//  Set 集合未删除	 
	}
	else
    {
		redis->HSet(to_string(BidOrderID).c_str(),"OrderStatus",to_string(O_Status::ORDER_UNFINISH).c_str()) ;
		redis->HSet(to_string(BidOrderID).c_str(),"RemainNum",to_string(BidRemainNum).c_str()) ;
		redis->HSet(to_string(BidOrderID).c_str(),"Total",to_string(BidTotal).c_str());
	}

//    redis->UnLock(redisLock);

	char buffer [500] = { 0 };
    snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"Symbol\":\"%s\",\"TransType\":%d,\"AskOrderID\":%ld,"
                                   "\"AskUserID\":%s,\"BidOrderID\":%ld,\"BidUserID\":%s,"
                                   "\"DealNum\":%.9f,\"DealPrice\":%.9f,\"TransactTime\":\"%s\",\"SrvIdentify\":\"%s\"}",
															ETag::ORDERUNLOCKFUNDS,
															Symbol,
															TransType,
															AskOrderID,
															AskUserID.c_str(),
															BidOrderID,
															BidUserID.c_str(),
															DealNum,
															DealPrice,
															TransactTime,
															ORDERSVRBINDINGKEY_CHECK

	);
	size_t len = strlen(buffer) +1;	
	TNodeAministrator::GetInstance()->PublishToMQ(ROUTINGKEY_MANAGEMENTSRV,buffer,len);
    XINFO("OrderSrv --> ManagerSrv Tradesucess asserts operater: %s\n",buffer);

	memset(buffer,0x00,sizeof(buffer));
	snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"UserID\":\"%s\",\"Symbol\":\"%s\",\"OrderID\":\"%ld\",\"DealNum\":%.9f,\"DealPrice\":%.9f,\"TransactTime\":\"%s\",\"SessionID\":\"%s\"}",
															ETag::ORDERASKTRANSACTION2WEB,
															AskUserID.c_str(),
															Symbol,
															AskOrderID,
															DealNum,
															DealPrice,
															TransactTime,
															AskSessionID.c_str()

	);
	char rountkey[100] = { 0 };
	redis->SelectDB(4);
    std::string wsid = redis->HGet(AskUserID.c_str(),"WSID");
//    redis->SelectDB(0);
	sprintf(rountkey,"%s.%s",ROUTINGKEY_WEB,wsid.c_str());
	len = strlen(buffer) +1;	
	TNodeAministrator::GetInstance()->PublishToMQ(rountkey,buffer,len);
    XINFO("OrderSrv --> WS  Ask order reply to web buff: %s  routingkey: %s\n",buffer,rountkey);

	memset(buffer,0x00,sizeof(buffer));
	snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"UserID\":\"%s\",\"Symbol\":\"%s\",\"OrderID\":\"%ld\",\"DealNum\":%.9f,\"DealPrice\":%.9f,\"TransactTime\":\"%s\",\"SessionID\":\"%s\"}",
															ETag::ORDERASKTRANSACTION2WEB,
															BidUserID.c_str(),
															Symbol,
															BidOrderID,
															DealNum,
															DealPrice,
															TransactTime,
															BidSessionID.c_str()


	);
	memset(rountkey,0x00,sizeof(rountkey));
    wsid = redis->HGet(BidUserID.c_str(),"WSID");
	sprintf(rountkey,"%s.%s",ROUTINGKEY_WEB,wsid.c_str());
	len = strlen(buffer) +1;	
	TNodeAministrator::GetInstance()->PublishToMQ(rountkey,buffer,len);
    XINFO("OrderSrv --> WS Bid order reply to web buff: %s routingkey :%s\n",buffer,rountkey);

    redis->SelectDB(0);
	g_redisPool->ReleaseObject(redis);

}
/*OrderReplyScribe end ****/

 /*CheckOrderScribe   begin ****/
 CheckOrderScribe::CheckOrderScribe(){

 }

  
 
 void CheckOrderScribe::Done(const char* event, unsigned int eventLen){
	 /* Parse Json */

	XINFO("ManagerSrv --> OrderSrv: %s\n", event);
	 
	rapidjson::Document d;
	d.Parse(event, eventLen);
	if (d.HasParseError() ||
		!d.IsObject()) {
		XERROR("parser json error|event=%s", event);

		return;
	}

	rapidjson::Value::ConstMemberIterator it;

	if(((it = d.FindMember("UserID")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	int UserID = it->value.GetInt();

	if(((it = d.FindMember("OrderID")) == d.MemberEnd()) || !it->value.IsInt64()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	long OrderID = it->value.GetInt64();

	if(((it = d.FindMember("Status")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	int Status = it->value.GetInt();
/*
if(((it = d.FindMember("Text")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
		XINFO("field no exit or type error");
#endif

		return;
	}
	const char* Text = it->value.GetString();
*/
	
    if (Status != 0){
    //if (Status == 1) {
		 //  return to web server abount the reason of order failed 
		 // if or not write to mysql  ?
		char buffer[200] = { 0 };
		char routingkey[50] = { 0 };
		CRedis* redis = g_redisPool->GetObject();
		std::string wsId = redis->HGet(to_string(OrderID).c_str(),"wsId");
		std::string sessionId = redis->HGet(to_string(OrderID).c_str(),"sessionId");
		snprintf(routingkey,sizeof(routingkey),"%s.%s",ROUTINGKEY_WEB,wsId.c_str());
        snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"SessionID\":\"%s\",\"UserID\":%ld,\"OrderID\":"
                                       "\"%ld\",\"Status\":%d}", ETag::NEWORDERANS,
                                        sessionId.c_str(),UserID,OrderID,STATUS_CODE::BALANCELESS) ;
		//char *buffer = const_cast<char*>(event) ;
		size_t len = strlen(buffer) +1 ;
		redis->Del(to_string(OrderID).c_str()) ;
		TNodeAministrator::GetInstance()->PublishToMQ(routingkey,buffer,len);
//		redis->Del(to_string(OrderID).c_str()) ;
        XINFO("OrderSrv --> WS  New order fail msg,routingkey[%s]  buff: %s  len[%d]\n",routingkey,buffer,len);
		g_redisPool->ReleaseObject(redis);
		return ;
		 //TNodeAministrator::GetInstance()->PublishToMQ();
	}

	// send the order to ORDERME
	CRedis* redis = g_redisPool->GetObject();
	
	std::string Symbol = redis->HGet(to_string(OrderID).c_str(),"Symbol");
	std::string OrderType = redis->HGet(to_string(OrderID).c_str (),"OrderType");
	std::string TransType =redis->HGet(to_string(OrderID).c_str(),"TransType");
	std::string OrderNumber = redis->HGet(to_string(OrderID).c_str(),"OrderNumber");
	std::string Price = redis->HGet(to_string(OrderID).c_str(),"Price");

//    redis->HSet(OrderID.c_str(),"Symbol",Symbol);
//	redis->HSet(OrderID.c_str(),"OrderType",to_string(OrderType).c_str());
//	redis->HSet(OrderID.c_str(),"TransType",to_string(TransType).c_str());
//	redis->HSet(OrderID.c_str(),"OrderNumber",to_string(OrderNumber).c_str());
//	redis->HSet(OrderID.c_str(),"Price",to_string(Price).c_str());
	/*std::string strOrderID = to_string(OrderID);
	std::string Symbol = redis->HGet(strOrderID.c_str(),"Symbol");
	std::string OrderType = redis->HGet(strOrderID.c_str(),"OrderType");
	std::string TransType =redis->HGet(strOrderID.c_str(),"TransType");
	std::string OrderNumber = redis->HGet(strOrderID.c_str(),"OrderNumber");
	std::string Price = redis->HGet(strOrderID.c_str(),"Price");*/
    redis->HSet(to_string(OrderID).c_str(), "BalanceChecked", "1");
    redis->HSet(to_string(OrderID).c_str(),"OrderStatus",to_string(O_Status::ORDER_READY).c_str()) ;
	 
	if(Symbol.empty() || OrderType.empty() || TransType.empty() || OrderNumber.empty() || Price.empty())
	{
        XERROR("New Order Inner error from redis, the OrderID = %ld has error  message buff is : %s\n",OrderID,event);
		return ;
	}

	char buffer [200] = { 0 };
	snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"OrderID\":%ld,\"Symbol\":\"%s\",\"OrderType\":%s,\"Side\":%s,\"OrderQty\":%s,\"Price\":%s}",
															ETag::ORDERREVOKE_CHANGEBALANCE,
															OrderID,
															Symbol.c_str(),
															OrderType.c_str(),
															TransType.c_str(),
															OrderNumber.c_str(),
															Price.c_str());
	size_t len = strlen(buffer) + 1;
	TNodeAministrator::GetInstance()->PublishToMQ(ROUTINGKEY_ORDERME,buffer,len);
    XINFO("OrderSrv --> OrderAPI, New Order : %s\n",buffer);
	

	g_redisPool->ReleaseObject(redis);
 }

 /*CheckOrderScribe   end ****/




 /*RevokeOrderScribe begin ****/
 RevokeOrderScribe::RevokeOrderScribe(){

 }

  

 void RevokeOrderScribe::Done(const char* event, unsigned int eventLen){
	 /* Parse Json */

    XINFO("WS --> OrderSrv Cancel order: %s\n",event);
	 
	rapidjson::Document d;
	d.Parse(event, eventLen);
	if (d.HasParseError() ||
		!d.IsObject()) {
		XERROR("parser json error|event=%s", event);

		return;
	}
	
	rapidjson::Value::ConstMemberIterator it;

	if(((it = d.FindMember("UserID")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	int UserID = it->value.GetInt();

	if(((it = d.FindMember("OrderID")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
    long OrderID = atol(it->value.GetString());

	CRedis* redis = g_redisPool->GetObject();
	int redis_UserID = atol(redis->HGet(to_string(OrderID).c_str(),"UserID").c_str());
    if(redis_UserID != UserID)
	{
        XERROR("Request UserID [%d], OrderID [%ld] is not match Redis UserID [%d]\n",UserID,OrderID, redis_UserID);
		return ;
	}

	// publish to ORDERME
	//snetwork_xservice_db::Redis* redis = g_redisPool->GetObject();
	std::string Symbol = redis->HGet(to_string(OrderID).c_str(),"Symbol");
	std::string OrderType = redis->HGet(to_string(OrderID).c_str(),"OrderType");
    std::string TransType = redis->HGet(to_string(OrderID).c_str(),"TransType");
	std::string OrderIDFrME = redis->HGet(to_string(OrderID).c_str(),"OrderIDFrME");
	std::string ExcelID = redis->HGet(to_string(OrderID).c_str(),"ExcelID");
	std::string Price = redis->HGet(to_string(OrderID).c_str(),"Price");

	g_redisPool->ReleaseObject(redis);

	char buffer [200] = { 0 };
    snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"OrderID\":%ld,\"OrderIDFrME\":\"%s\","
                                   "\"ExcelID\":\"%s\",\"Symbol\":\"%s\",\"Side\":%s,\"Price\":%s}",
															ETag::ORDERREVOKEME_REQUEST,
															OrderID,
															OrderIDFrME.c_str(),
															ExcelID.c_str(),
															Symbol.c_str(),
															TransType.c_str(),
                                                            Price.c_str());

	
	size_t len = strlen(buffer)+1;														
	TNodeAministrator::GetInstance()->PublishToMQ(ROUTINGKEY_ORDERME,buffer,len);

    XINFO("OrderSrv -->OrderAPI Cancel Order: %s\n",buffer);
 }
 /*RevokeOrderScribe end ****/


 /*RevokeACKScribe begin***/
RevokeACKScribe::RevokeACKScribe(){

 }
 
 void RevokeACKScribe::Done(const char* event, unsigned int eventLen){

    XINFO("OrderAPI --> OrderSrv Cancel order ack: %s\n",event);

 
	/* Parse Json */
	rapidjson::Document d;
	d.Parse(event, eventLen);
	if (d.HasParseError() ||
		!d.IsObject()) {
        XERROR("parser json error|event=%s\n", event);

		return;
	}

	rapidjson::Value::ConstMemberIterator it;

	if(((it = d.FindMember("OrderID")) == d.MemberEnd()) || !it->value.IsInt64()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	long OrderID = it->value.GetInt64();

	if(((it = d.FindMember("OrderStatus")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	int OrderStatus = it->value.GetInt();

	if(((it = d.FindMember("TransactTime")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	const char* TransactTime = it->value.GetString();

	if(((it = d.FindMember("Text")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	const char* Text = it->value.GetString();

	if(OrderStatus == 1){
        //need to tell web what happend
        XINFO("revoke order[%s] has been refused\n",OrderID);

//        memset(buffer,0,sizeof(buffer));


//        snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"UserID\":%s,\"OrderID\":\"%ld\",\"Symbol\":\"%s\",\"OrderType\":%s,        \
//        \"TransType\":%d,\"OrderNumber\":%.9f,\"Price\":%.9f,\"TransactTime\":\"%s\",\"SessionID\":\"%s\"}",
//                                                                ETag::ORDERREVOKE_WEB,
//                                                                UserID.c_str(),
//                                                                OrderID,
//                                                                Symbol,
//                                                                OrderType.c_str(),
//                                                                TransType,
//                                                                DealNum,
//                                                                DealPrice,
//                                                                TransactTime,
//                                                                SessionID.c_str());
//        char rountkey[100] = { 0 };
//        redis->SelectDB(4);
//        std::string wsid = redis->HGet(UserID.c_str(),"WSID");
//        sprintf(rountkey,"%s.%s",ROUTINGKEY_WEB,wsid.c_str());
//        len = strlen(buffer) +1;
//        TNodeAministrator::GetInstance()->PublishToMQ(rountkey,buffer,len);
//        XINFO("OrderSrv --> WS Cancel order response: %s",buffer);

		return ;
	}

	CRedis* redis = g_redisPool->GetObject();
	 
    //redis->HSet(to_string(OrderID).c_str(),"OrderStatus",to_string(O_Status::ORDER_REVOKE).c_str()) ;

    XINFO("Cancel Order ACK has received,the OrderID = %ld\n",OrderID);
	g_redisPool->ReleaseObject(redis);
	 
 }



  
 

 /*RevokeACKScribe end***/

 /*RevokeRelpyScribe begin***/
 RevokeRelpyScribe::RevokeRelpyScribe(){

 }

  

 void RevokeRelpyScribe::Done(const char* event, unsigned int eventLen){

    XINFO("MarketAPI --> OrderSrv Cancel response :%s\n",event);
	
 	/* Parse Json */
	rapidjson::Document d;
	d.Parse(event, eventLen);
	if (d.HasParseError() ||
		!d.IsObject()) {
        XERROR("parser json error|event=%s\n", event);

		return;
	}
	rapidjson::Value::ConstMemberIterator it;
	if(((it = d.FindMember("TransType")) == d.MemberEnd()) || !it->value.IsInt()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	int TransType = it->value.GetInt();
	long OrderID = -1 ;
	 
	if(((it = d.FindMember("OrderID")) == d.MemberEnd()) || !it->value.IsInt64()) {
#if defined(DEBUG)	
            XINFO("field no exit or type error\n");
#endif
			return;
		}
	OrderID = it->value.GetInt64();

	if(((it = d.FindMember("Symbol")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	const char* Symbol = it->value.GetString();

	

	if(((it = d.FindMember("DealNum")) == d.MemberEnd()) || !it->value.IsDouble()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	double DealNum = it->value.GetDouble();

	if(((it = d.FindMember("DealPrice")) == d.MemberEnd()) || !it->value.IsDouble()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	double DealPrice = it->value.GetDouble();

	if(((it = d.FindMember("TransactTime")) == d.MemberEnd()) || !it->value.IsString()) {
#if defined(DEBUG)	
        XINFO("field no exit or type error\n");
#endif

		return;
	}
	const char* TransactTime = it->value.GetString();

	 
	CRedis* redis = g_redisPool->GetObject();
	std::string UserID = redis->HGet(to_string(OrderID).c_str(),"UserID");
	std::string OrderType = redis->HGet(to_string(OrderID).c_str(),"OrderType");
	std::string ExcelID = redis->HGet(to_string(OrderID).c_str(),"ExcelID");
	std::string OrderIDFrME = redis->HGet(to_string(OrderID).c_str(),"OrderIDFrME");
	std::string OrderNumber = redis->HGet(to_string(OrderID).c_str(),"OrderNumber");
    std::string RemainNumber = redis->HGet(to_string(OrderID).c_str(), "RemainNum");
	std::string Price = redis->HGet(to_string(OrderID).c_str(),"Price");
    std::string SessionID = redis->HGet(to_string(OrderID).c_str(),"sessionId");
    std::string OrderTime = redis->HGet(to_string(OrderID).c_str(), "OrderTime");
    std::string OrderStatus = redis->HGet(to_string(OrderID).c_str(), "OrderStatus");
	
	//return ;
	// mysql 
	MySqlDB db;
	db.Open();
	db.SelectDB(dynamic_cast<SXConfig *>(SXFlagger::GetInstance()->GetConfig())->MySqlDBName().c_str());
	db.StartTransaction();
	char sqlbuf[500] = { 0 }; 
	/*Symbol*/
    snprintf(sqlbuf,sizeof(sqlbuf),"insert into token_order(ID,UserID,DelegateType,Type,Symbol,ExcelID,OrderIDFrME,Status,Price,Amount,OrderTime,LastTraderTime)  \
                                        values(%ld,%s,%d, %d,'%s','%s', '%s',%d,%s, %s, %s,%s)  \
                                        ON DUPLICATE KEY UPDATE Status = %d and LastTraderTime = %s" ,
										OrderID,UserID.c_str(),0,
										TransType,Symbol,ExcelID.c_str(),
                                        OrderIDFrME.c_str(),O_Status::ORDER_REVOKE,Price.c_str(),
                                        RemainNumber.c_str(),OrderTime.c_str(),
                                        TransactTime,O_Status::ORDER_REVOKE,TransactTime
	);

	ssize_t rows = db.ExecSql(sqlbuf);
	if (rows == -1)
	{
        XERROR("%s,con=%d|sql=%s\n", db.GetMySqlErrMsg(), db.IsConnect(), sqlbuf);
		db.Commint();
		return ;
	}
	db.Commint();

	db.Close();
	//return ;
//	redis->SelectDB(0);
    redis->Del(to_string(OrderID).c_str()) ;
    if (redis->SelectDB(2)) {
        redis->Excute("srem %s %ld",UserID.c_str(),OrderID);
    }
    //g_redisPool->ReleaseObject(redis);

	char buffer [500] = { 0 };
    snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"UserID\":%s,\"OrderID\":%ld,\"Symbol\":\"%s\","
                                   "\"OrderType\":%s,\"TransType\":%d,\"OrderNumber\":%.9f,"
                                   "\"Price\":%.9f,\"SrvIdentify\":\"%s\"}",
                                                            ETag::ORDERCHECK_BALANCE_UNFREEZE,
															UserID.c_str(),
															OrderID,
															Symbol,
															OrderType.c_str(),
															TransType,
															DealNum,
															DealPrice,
															ORDERSVRBINDINGKEY_CHECK);
	size_t len = strlen(buffer) +1;													
	TNodeAministrator::GetInstance()->PublishToMQ(ROUTINGKEY_MANAGEMENTSRV,buffer,len);
    XINFO("OrderSrv --> ManagerSrv Cancel order unfreeze msg: %s\n",buffer);

	memset(buffer,0,sizeof(buffer));
	 
 
    snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"UserID\":%s,\"OrderID\":\"%ld\",\"Symbol\":\"%s\",\"OrderType\":%s,"
                                   "\"TransType\":%d,\"OrderNumber\":%.9f,\"Price\":%.9f,\"TransactTime\":\"%s\",\"SessionID\":\"%s\"}",
															ETag::ORDERREVOKE_WEB,
															UserID.c_str(),
															OrderID,
															Symbol,
															OrderType.c_str(),
															TransType,
															DealNum,
															DealPrice,
															TransactTime,
															SessionID.c_str());
	char rountkey[100] = { 0 };
	redis->SelectDB(4);
    std::string wsid = redis->HGet(UserID.c_str(),"WSID");
    redis->SelectDB(0);
	sprintf(rountkey,"%s.%s",ROUTINGKEY_WEB,wsid.c_str());
	len = strlen(buffer) +1;	
	TNodeAministrator::GetInstance()->PublishToMQ(rountkey,buffer,len);
    XINFO("OrderSrv --> WS Cancel order response: %s\n",buffer);

    g_redisPool->ReleaseObject(redis);
 }
 
 /*RevokeRelpyScribe end***/

 /*UpdateTradePwdScribe begin ****/
 UpdateTradePwdScribe::UpdateTradePwdScribe(){

 }

  

 void UpdateTradePwdScribe::Done(const char* event, unsigned int eventLen){

 }
 /*UpdateTradePwdScribe end ****/

 


 



 
