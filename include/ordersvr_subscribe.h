/*************************************************************************
	> File Name: solution_subscribe.h
	> Author: xyz
	> Mail: xiao13149920@foxmail.com 
	> Created Time: Tue 28 Feb 2017 07:36:02 PM CST
 ************************************************************************/
#ifndef SOLUTIONGATEWAY_SOLUTION_SUBSCRIBE_H_
#define SOLUTIONGATEWAY_SOLUTION_SUBSCRIBE_H_

#include<mutex>
#include<string.h>
#include<type_traits>
//#include"tnode_adapter.h"
#include"ThreadPool.h"
#include<rapidjson/document.h>
#include<rapidjson/writer.h>
#include<rapidjson/rapidjson.h>
#include<rapidjson/stringbuffer.h>

/*Subscribe   begin ****/
 
class Subscribe {
public:
	Subscribe();
	virtual ~Subscribe() =default;

	 
	//Subscribe(const Subscribe& r) =default;
	//Subscribe& operator=(const Subscribe& r) = delete;
	//Subscribe(Subscribe&& r) =default;
	//Subscribe& operator=(Subscribe&& r) = delete;

	 
	virtual void Done(const char* event, unsigned int eventLen) = 0;

	 
};
/*Subscribe   end ****/

/*NewOrderScribe   begin ****/
class NewOrderScribe : public Subscribe {
public:
	NewOrderScribe();
	virtual ~NewOrderScribe() =default;

	//Subscribe(const Subscribe& r) = default;
	//NewOrderScribe(const NewOrderScribe& r)=default;
	//NewOrderScribe& operator=(const NewOrderScribe& r) = delete;
	//NewOrderScribe(NewOrderScribe&& r) =default;
	//NewOrderScribe& operator=(NewOrderScribe&& r) = delete;

	/* parse by rapidson */
	virtual void Done(const char* event, unsigned int eventLen);
	void CreateOrderID(std::string& orderid);
};  
/*NewOrderScribe end ****/

/*OrderACKScribe begin ****/
class OrderACKScribe : public Subscribe {
	public:
		OrderACKScribe();
		virtual ~OrderACKScribe()=default ;

		//Subscribe(const Subscribe& r) = default;
		//OrderACKScribe(const OrderACKScribe& r)=default;
		//OrderACKScribe& operator=(const OrderACKScribe& r) = delete;
		//OrderACKScribe(OrderACKScribe&& r) =default;
		//OrderACKScribe& operator=(OrderACKScribe&& r) = delete;

		/* parse by rapidson */
		virtual void Done(const char* event, unsigned int eventLen);
};
/*OrderACKScribe end ****/


 
/*OrderReplyScribe begin ****/
class OrderReplyScribe : public Subscribe {
	public:
		OrderReplyScribe();
		virtual ~OrderReplyScribe()= default;

		//Subscribe(const Subscribe& r) = default;
		//OrderReplyScribe(const OrderReplyScribe& r) = default;
		//OrderReplyScribe& operator=(const OrderReplyScribe& r) = delete;
		//OrderReplyScribe(OrderReplyScribe&& r) = default;
		//OrderReplyScribe& operator=(OrderReplyScribe&& r) = delete;

		/* parse by rapidson */
		virtual void Done(const char* event, unsigned int eventLen);
};
/*OrderReplyScribe end ****/

/*CheckOrderScribe begin ****/
class CheckOrderScribe : public Subscribe  {
	public:
		CheckOrderScribe();
		virtual ~CheckOrderScribe()=default;

		//Subscribe(const Subscribe& r) = default;
		//CheckOrderScribe(const CheckOrderScribe& r)=default;
		///CheckOrderScribe& operator=(const CheckOrderScribe& r) = delete;
		//CheckOrderScribe(CheckOrderScribe&& r)= default;
		//CheckOrderScribe& operator=(CheckOrderScribe&& r) = delete;

		/* parse by rapidson */
		virtual void Done(const char* event, unsigned int eventLen);
};

/*CheckOrderScribe end ****/

/*RevokeOrderScribe begin ****/
class RevokeOrderScribe : public Subscribe {
	public:
		RevokeOrderScribe();
		virtual ~RevokeOrderScribe()=default ;

		//Subscribe(const Subscribe& r) = default;
		//RevokeOrderScribe(const RevokeOrderScribe& r)= default;
		//RevokeOrderScribe& operator=(const RevokeOrderScribe& r) = delete;
		//RevokeOrderScribe(RevokeOrderScribe&& r)=default;
		//RevokeOrderScribe& operator=(RevokeOrderScribe&& r) = delete;

		/* parse by rapidson */
		virtual void Done(const char* event, unsigned int eventLen);
};
/*RevokeOrderScribe end ****/

/*RevokeACKScribe begin ****/
class RevokeACKScribe : public Subscribe{
	public:
		RevokeACKScribe();
		virtual ~RevokeACKScribe() =default ;

		//Subscribe(const Subscribe& r) = default;
		//RevokeACKScribe(const RevokeACKScribe& r) =default;
		//RevokeACKScribe& operator=(const RevokeACKScribe& r) = delete;
		//RevokeACKScribe(RevokeACKScribe&& r)=default;
		//RevokeACKScribe& operator=(RevokeACKScribe&& r) = delete;

		/* parse by rapidson */
		virtual void Done(const char* event, unsigned int eventLen);
};

/*RevokeRelpyScribe end ****/

/*RevokeRelpyScribe begin ****/
class RevokeRelpyScribe : public Subscribe{
	public:
		RevokeRelpyScribe();
		virtual ~RevokeRelpyScribe() =default;

		//Subscribe(const Subscribe& r) = default;
		//RevokeRelpyScribe(const RevokeRelpyScribe& r) = default;
		//RevokeRelpyScribe& operator=(const RevokeRelpyScribe& r) = delete;
		//RevokeRelpyScribe(RevokeRelpyScribe&& r) =default;
		//RevokeRelpyScribe& operator=(RevokeRelpyScribe&& r) = delete;

		/* parse by rapidson */
		virtual void Done(const char* event, unsigned int eventLen);
};
/*RevokeRelpyScribe end ****/

/*UpdateTradePwdScribe begin ****/
class UpdateTradePwdScribe : public Subscribe{
	public:
		UpdateTradePwdScribe();
		virtual ~UpdateTradePwdScribe() =default ;

		//Subscribe(const Subscribe& r) = default;
		//UpdateTradePwdScribe(const UpdateTradePwdScribe& r) =default;
		//UpdateTradePwdScribe& operator=(const UpdateTradePwdScribe& r) = delete;
		//UpdateTradePwdScribe(UpdateTradePwdScribe&& r)=default;
		//UpdateTradePwdScribe& operator=(UpdateTradePwdScribe&& r) = delete;

		/* parse by rapidson */
		virtual void Done(const char* event, unsigned int eventLen);
};
/*UpdateTradePwdScribe end ****/



#endif

