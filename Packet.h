#pragma once
#include "Ship.h"
#include "ClientServer.h"

class Packet
{
	std::vector<quint8> _massive{ 0 };
	bool _isCorrect = true;

public:

	using DOIT = ClientServer::DOIT;

	Packet() = default;

	explicit Packet(const bool isCorrect) : _isCorrect(isCorrect) { }

	~Packet() = default;
	Packet(const Packet&) = delete;
	Packet(Packet&& packet) noexcept;
	Packet& operator=(const Packet&) = delete;
	Packet& operator=(Packet&& packet) noexcept;

	void WriteData(ClientServer::DOIT doit, quint8 param);
	void WriteData(const std::vector<Ship>& mas);
	void WriteData(ClientServer::DOIT doit);
	[[nodiscard]] bool ReadData(ClientServer::DOIT& doit, quint8& param) const;
	[[nodiscard]] bool ReadData(std::vector<Ship>& mas) const;
	[[nodiscard]] bool ReadData(ClientServer::DOIT& doit) const;

	[[nodiscard]] bool IsCorrect() const noexcept
	{
		return _isCorrect;
	}
};
