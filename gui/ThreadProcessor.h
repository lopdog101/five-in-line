#pragma once
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <vector>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/signal.hpp>

// CThreadProcessor

class CThreadProcessor : public CWnd
{
	DECLARE_DYNAMIC(CThreadProcessor)
public:
	typedef boost::shared_ptr<boost::thread> thrd_ptr;
	typedef boost::recursive_mutex::scoped_lock lkt;

	struct error_t
	{
		std::string message;
		std::string context;
	};
	typedef std::vector<error_t> errors_t;

	enum State{st_stopped,st_wait_task,st_new_task,st_execute_task,st_cancelling,st_stopping};

	static const DWORD WM_COMPLETE=WM_USER+1;
private:
	thrd_ptr thrd;
	mutable boost::recursive_mutex mtx;
	boost::condition cond;
	bool window_created;
	State state;
	errors_t messages;

	void execute();
    bool wait_for_new_job();
void add_message(const error_t& mess);
	void create_window();
public:
	CThreadProcessor();
	virtual ~CThreadProcessor();

	void start();
	void stop();
	bool is_started() const{return thrd;}
	void start_job();
	void cancel_job();
	bool is_cancelling() const;

	boost::signal<void ()> OnExecute;
	boost::signal<void ()> OnComplete;
	boost::signal<void (const errors_t& vals)> OnErrors;

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnCompleteHandler(WPARAM,LPARAM);
};


