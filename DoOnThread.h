#pragma once
#include "QTcpServer"

class DoOnThread : public QTcpServer
{
	Q_OBJECT

public:

	DoOnThread() = default;
	DoOnThread(const DoOnThread&) = delete;
	DoOnThread(DoOnThread&&) = delete;
	DoOnThread& operator=(const DoOnThread&) = delete;
	DoOnThread& operator=(DoOnThread&&) = delete;
	virtual ~DoOnThread() = default;

public slots:

	// ReSharper disable once CppMemberFunctionMayBeStatic
	void DoThis(const std::function<void()> f)
	{
		f();//ÍÅ ÂÛÏÎËÍßÅÒÑß
	}

	void Do()
	{//ÂÛÏÎËÍßÅÒÑß
		close();
		QThread::currentThread()->quit();
	}
};
