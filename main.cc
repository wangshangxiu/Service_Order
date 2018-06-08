/*************************************************************************
	> File Name: main.cc
	> Author: xyz
	> Mail: xiao13149920@foxmail.com 
	> Created Time: Fri 03 Feb 2017 11:26:30 AM CST
 ************************************************************************/
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"tnode_adapter.h"
#include"ThreadPool.h"
#include"object_pool.h"
#include"redisop.h"
#include"mysqlop.h"
#include"object_pool.h"
#include"solution_config.h"
#include"ordersvr_subscribe.h"
#include <math.h>
#include <map>
#include <time.h>

using namespace snetwork_xservice_xflagger;
using namespace snetwork_xservice_tnode ;
 
TNodeAministrator* g_tnode = nullptr; 
ThreadPool* g_NewOrderthreadPool = nullptr;
ThreadPool* g_CheckOrderthreadPool = nullptr;
ThreadPool* g_OrderPool = nullptr;
ThreadPool* g_OrderRevokePool = nullptr;
ThreadPool* g_UpdatethreadPool = nullptr;
using RedisPool =  ObjectPool<CRedis,15>;
RedisPool* g_redisPool = nullptr;
std::map<int,std::string> g_pwdMap;
std::map<int,std::string> g_webInfo ;
ConnectionPool* g_connectionPool = nullptr;
//CRedisLock redisLock("uniqueredislock");
 
#define SAFE_LIST_ELAPSED_SECONDS 60
void RunList(void) {
    std::chrono::system_clock::time_point end;
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds;
    int sleepTime = 0;

    sleep(SAFE_LIST_ELAPSED_SECONDS);
    while (1) {
        end = std::chrono::system_clock::now();
        CRedis* redis = g_redisPool->GetObject();
        int cur = 0;
        while(true) {
            shared_ptr<redisReply> ry = redis->Excute("SCAN %d count %d", cur, 20);
            if(ry.get() != nullptr) {
                cur = atoi(ry.get()->element[0]->str);
                for (int i =0; i < ry.get()->element[1]->elements; i++) {
                    char* orderid = ry.get()->element[1]->element[i]->str;
                    std::string balancechecked = redis->HGet(orderid, "BalanceChecked");
                    std::string orderack =  redis->HGet(orderid, "OrderAck");

                    if (!balancechecked.empty() && !orderack.empty() &&
                            (balancechecked.compare("1") == 0) &&
                            (orderack.compare("0") == 0)) {
                        std::string Symbol = redis->HGet(orderid,"Symbol");
                        std::string OrderType = redis->HGet(orderid,"OrderType");
                        std::string TransType =redis->HGet(orderid,"TransType");
                        std::string OrderNumber = redis->HGet(orderid,"OrderNumber");
                        std::string Price = redis->HGet(orderid,"Price");

                        char buffer [200] = { 0 };
                        snprintf(buffer,sizeof(buffer),"{\"Tag\":%d,\"OrderID\":%ld,\"Symbol\":\"%s\",\"OrderType\":%s,\"Side\":%s,\"OrderQty\":%s,\"Price\":%s}",
                                                                                ETag::ORDERREVOKE_CHANGEBALANCE,
                                                                                atol(orderid),
                                                                                Symbol.c_str(),
                                                                                OrderType.c_str(),
                                                                                TransType.c_str(),
                                                                                OrderNumber.c_str(),
                                                                                Price.c_str());
                        size_t len = strlen(buffer) + 1;
                        TNodeAministrator::GetInstance()->PublishToMQ(ROUTINGKEY_ORDERME,buffer,len);
                        XINFO("OrderSrv --> OrderAPI, Resend Order : %s\n",buffer);
                    }

                    //delete invaluable order accured by managersvr stoped and no assert result respone
                    //here we consider that manager maybe crash or network blocked
//                    if(!balancechecked.empty() && balancechecked.compare("0") == 0) {
//                        std::string ordertime = redis->HGet(orderid, "OrderTime");
//                        if (atol(ordertime.c_str()) + 15 < time(NULL)) {
//                           redis->Del(orderid);
//                        }
//                    }
                }

                if (cur ==0) {
                    break;
                }
            }
        }
        g_redisPool->ReleaseObject(redis);

        elapsed_seconds = end - start;
        sleepTime = SAFE_LIST_ELAPSED_SECONDS - elapsed_seconds.count();
        start = std::move(end);

        if (sleepTime > 0) {
            sleep(sleepTime);
        }
    }
}

char intTohexChar(int x) {
    static const char HEX[16] = {
		 '0', '1', '2', '3', '4', '5', '6', '7',
		 '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };
    return HEX[x];
}

int main(int argc, char* argv[]) {
	XConfig* cfg = new SXConfig;
	XFlagger* flagger = SXFlagger::GetInstance();
	flagger->ParseCommandLine(&argc, &argv, false);
	flagger->FlagXConfig(cfg);
	SXConfig* sxconfig = dynamic_cast<SXConfig*>(flagger->GetConfig());
	if (sxconfig == nullptr) {
		fprintf(stderr, "config error");

		exit(1);
	}
	std::cout<<*sxconfig<<std::endl;
	/* logger setting */

	std::string fileName (sxconfig->LoggerFile());
	if (!sxconfig->Screen()) {
		fileName = sxconfig->LoggerFile();
	}
	(*XLogger::GetObject(fileName.c_str())).StdErr(sxconfig->Screen()).Colour(sxconfig->Color()).Dir(sxconfig->LoggerDir());

	g_redisPool = RedisPool::GetInstance(sxconfig->RedisHost().c_str(),sxconfig->RedisPort(),
																			sxconfig->RedisDB(),
																			sxconfig->RedisAuth().c_str());

    g_connectionPool = ConnectionPool::GetInstance(sxconfig->MySqlHost().c_str(),
																		 sxconfig->MySqlUser().c_str(),
																		 sxconfig->MySqlPassword().c_str(),
																		 sxconfig->MySqlPort(),
																		 sxconfig->ConnSize()); 
	g_tnode = TNodeAministrator::GetInstance();
	g_NewOrderthreadPool = new ThreadPool(1);
	g_CheckOrderthreadPool = new ThreadPool(1) ;
    g_OrderPool = new ThreadPool(1);
    g_OrderRevokePool = new ThreadPool(1);
	g_UpdatethreadPool = new ThreadPool(1);

	char bingingkey[100] = { 0 };
	snprintf(bingingkey,sizeof(bingingkey),"%s.%s",ORDERSVRBINDINGKEY_NEWORDER,sxconfig->SolutionID().c_str());

	/*we->ordersvr  */
	TNodeConsumer* psendorder = new NewOrderConsumer();
	g_tnode->AddConsumer(psendorder);

	memset(bingingkey,0x00,sizeof(bingingkey));
	snprintf(bingingkey,sizeof(bingingkey),"%s.%s",ORDERSVRBINDINGKEY_CHECK,sxconfig->SolutionID().c_str());
	TNodeConsumer* pcheckOrder = new CheckOrderConsumer();
	g_tnode->AddConsumer(pcheckOrder);

	memset(bingingkey,0x00,sizeof(bingingkey));
	snprintf(bingingkey,sizeof(bingingkey),"%s.%s",ORDERSVRBINDINGKEY_TRANSACTION,sxconfig->SolutionID().c_str());
	TNodeConsumer* porderME = new TransactionConsumer();
	g_tnode->AddConsumer(porderME);

	memset(bingingkey,0x00,sizeof(bingingkey));
	snprintf(bingingkey,sizeof(bingingkey),"%s.%s",ORDERSVRBINDINGKEY_UPDATE,sxconfig->SolutionID().c_str());
	TNodeConsumer* puser = new UpdateUseInfoConsumer();
	g_tnode->AddConsumer(puser);

	g_tnode->Run();

    std::thread t(RunList);

	while(1)
	{
		sleep(3600) ;
	}
}

