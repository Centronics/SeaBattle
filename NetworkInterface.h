#pragma once
#include "Packet.h"
#include "Graphics.h"
#include <QThread>
#include "ServerThread.h"

class NetworkInterface : public QThread
{
	Q_OBJECT

public:

	explicit NetworkInterface(Graphics& g, QObject* parent, NetworkInterface** r) : QThread(parent), _graphics(g), _myRef(r)
	{
		//«¿œ»—¿“‹ »ƒ∆≈Õ“»‘» ¿“Œ– œŒ“Œ ¿
	}
	
	NetworkInterface() = delete;
	virtual ~NetworkInterface();
	NetworkInterface(const NetworkInterface&) = delete;
	NetworkInterface(NetworkInterface&&) = delete;
	NetworkInterface& operator=(const NetworkInterface&) = delete;
	NetworkInterface& operator=(NetworkInterface&&) = delete;

	[[nodiscard]] std::optional<QString> SendHit();

	void Close()
	{//Õ≈Œ¡’Œƒ»ÃŒ »«¡≈∆¿“‹ ¬«¿»ÃŒ¡ÀŒ »–Œ¬ » œ–» ”ƒ¿À≈Õ»» —¿ÃŒ√Œ —≈¡ﬂ!!
		//—»Õ’–ŒÕ»«»–Œ¬¿“‹ œŒ“Œ »
		if (this == nullptr || !(*_myRef))
			return;
		*_myRef = nullptr;
		deleteLater();
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

	[[nodiscard]] static QString GetErrorDescr(QAbstractSocket::SocketError err);
	virtual void Send(const Packet&) = 0;
	virtual void Run() = 0;

private:

	NetworkInterface** const _myRef = nullptr;

	[[nodiscard]] std::optional<Packet> CreateHitPacket();
	void run() override;

signals:

	void SignalReceive(Packet);
	void Update();
};
