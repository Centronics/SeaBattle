#pragma once
#include "QTcpSocket"

class SocketDoOnThread : public QTcpSocket
{
	Q_OBJECT

public:

	SocketDoOnThread() = default;
	SocketDoOnThread(const SocketDoOnThread&) = delete;
	SocketDoOnThread(SocketDoOnThread&&) = delete;
	SocketDoOnThread& operator=(const SocketDoOnThread&) = delete;
	SocketDoOnThread& operator=(SocketDoOnThread&&) = delete;
	virtual ~SocketDoOnThread() = default;

public slots:

	// ReSharper disable once CppMemberFunctionMayBeStatic
	void DoThis(const std::function<void()> f)
	{
		f();//ме бшонкмъеряъ
	}

	void Do()
{
	
}
};
