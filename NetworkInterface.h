#pragma once
#include "Packet.h"
#include "Graphics.h"
#include "QTcpSocket"
#include <QThread>
#include "DoOnThread.h"

class NetworkInterface : public QThread
{
	Q_OBJECT

public:

	explicit NetworkInterface(Graphics& g, QObject* parent, NetworkInterface** r) : QThread(parent), _graphics(g), _myRef(r) { }
	NetworkInterface() = delete;
	virtual ~NetworkInterface();
	NetworkInterface(const NetworkInterface&) = delete;
	NetworkInterface(NetworkInterface&&) = delete;
	NetworkInterface& operator=(const NetworkInterface&) = delete;
	NetworkInterface& operator=(NetworkInterface&&) = delete;

	[[nodiscard]] std::optional<QString> SendHit();

	std::function<void()> f2;
	
	void Close()// ����� ������� ��� ������ � �������� �����
	{
		if (this == nullptr)
			return;
		*_myRef = nullptr;

		const auto f = [this]//������ �������
		{
			IntClose();//������ ����� ������� ������ �� ����������
		};

		emit SendToThread(f);//��������, ������� ������������������� � ���, ��� � ������� ��������� ����� �� ������ ������
		emit Do();

		//const std::function<void()> f1 = f;
		//f1();
		//
		f2 = f;
		//test(f);
		//test1();
	}

	static void test(std::function<void()> f)
	{
		f();
	}

	void test1()
	{
		f2();
	}
	
protected:

	enum class STATE : quint8
	{
		PUSHMAP,
		WAITMAP,
		WAITHIT,
		HIT
	};

	STATE _currentState = STATE::PUSHMAP;
	Graphics& _graphics;
	NetworkInterface** const _myRef = nullptr;

	[[nodiscard]] static QString GetErrorDescr(QAbstractSocket::SocketError err);
	[[nodiscard]] std::optional<Packet> CreateHitPacket();
	virtual void Send(const Packet& packet) = 0;
	virtual void Run() = 0;

protected slots:
	
	virtual void IntClose() = 0;
	
private:

	void run() override;

signals:

	void SignalReceive(Packet);
	void Update();
	void SendToThread(std::function<void()>);
	void Do();
};
