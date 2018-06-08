/*************************************************************************
	> File Name: define.h
	> Author: xyz
	> Mail: xiao13149920@foxmail.com 
	> Created Time: Fri 03 Feb 2017 11:33:28 AM CST
 ************************************************************************/
#ifndef SOLUTIONGATEWAY_DEFINE_H_
#define SOLUTIONGATEWAY_DEFINE_H_

 
#include<chrono>
#include<string.h>

 
#include"xlogger.h"
#include<chrono>
#include<sys/time.h>

 
 struct NewOrderInfo {
	 int m_UserID;
	 char m_RequestID[20]  ;
	 char m_TradePwd[20];
	 char m_Symbol[10];
	  
 };

 enum ETag {
	 NEWORDER = 0x3001,
     NEWORDERANS = 0x3002,
	 ORDERCHECK_BALANCE = 0x3003 ,
	 ORDERCHECK_BALANCEREPLY = 0x3004,
	 ORDERREVOKE_CHANGEBALANCE = 0x3005,
	 ORDERACK_MANAGEMENTSRV = 0x3006,
	 ORDERACK_PINDING = 0x3007,
	 ORDERTRANSACTION_REPLY = 0x3008,
	 ORDERUNLOCKFUNDS = 0x3009,
	 ORDER_FROZENFUNDSFAILED = 0x30041,
	 ORDERASKTRANSACTION2WEB = 0x30101,
	 ORDERBIDTRANSACTION2WEB = 0x30102,

	 ORDERREVOKE_REQUEST = 0x4001,
	 ORDERREVOKEME_REQUEST = 0x4002,
	 ORDERREVOKE_ACK = 0x4003,
	 ORDERREVOKE_REPLY = 0x4004,

     ORDERCHECK_BALANCE_UNFREEZE = 0x4005,

	 ORDERREVOKE_WEB = 0x4006,
	 TRADEPWD_UPDATE = 0x5001
	 
 } ;

enum O_Status{
    //for redis
	ORDER_READY = 0,
    ORDER_PINDING,
	ORDER_UNFINISH,
    //for mysql
	ORDER_REVOKE,
    ORDER_TRADED,
} ;

enum Order_Type {
	ORDER_BUY = 0,
	ORDER_SELL,
	
};

enum STATUS_CODE {
    MEREFUSE = -3,
    TRADINGCODERROR = -2,
    BALANCELESS = -1,
    PENDINGOK = 0,
    //0/-1/-2/-3   // 0 挂单成功, -1 余额不足, -2交易密错误,-3 其他错误}
};

#define TOWEBSERVER webserver

#define ORDERSVRBINDINGKEY_NEWORDER "order"
#define ORDERSVRBINDINGKEY_CHECK  "manger2order_bkey"
#define ORDERSVRBINDINGKEY_TRANSACTION "orderapi2order"  
#define ORDERSVRBINDINGKEY_UPDATE "update"
#define QUEUENAME_NEWORDER "NEW_ORDER_QUEUE"
#define QUEUENMAE_CHECKORDER "frommanger_queue"
#define QUEUENMAE_TRANSACTION "fromorderapi_queue" 
#define QUEUENAME_UPDATEUSERINFO "UPDATE_USER_INFO"


#define TRADEPWDERROR "the trade error ,the order is failed"

#define ROUTINGKEY_WEB "wsid" 
#define ROUTINGKEY_MANAGEMENTSRV "manger_bkey"
#define ROUTINGKEY_ORDERME "order2orderapi"

#define ACCOUNTIDME "dsfdfdfd" 

#define REDIS_USERDB 4
#endif 
