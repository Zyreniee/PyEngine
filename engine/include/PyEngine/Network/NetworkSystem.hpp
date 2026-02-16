#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace PyEngine {

enum class ConnectionStatus { Disconnected, Connecting, Connected, Failed };

struct Packet {
    uint32_t ID;
    std::vector<uint8_t> Data;
};

class Socket {
public:
    virtual ~Socket() = default;

    virtual bool Open(const std::string& address, uint16_t port) = 0;
    virtual void Close() = 0;
    virtual bool Send(const Packet& packet) = 0;
    virtual bool Receive(Packet& outPacket) = 0;

    bool IsOpen() const { return m_IsOpen; }

protected:
    bool m_IsOpen = false;
};

class TCPSocket : public Socket {
public:
    bool Open(const std::string& address, uint16_t port) override;
    void Close() override;
    bool Send(const Packet& packet) override;
    bool Receive(Packet& outPacket) override;
};

class UDPSocket : public Socket {
public:
    bool Open(const std::string& address, uint16_t port) override;
    void Close() override;
    bool Send(const Packet& packet) override;
    bool Receive(Packet& outPacket) override;
};

class NetworkManager {
public:
    static NetworkManager& Get() {
        static NetworkManager instance;
        return instance;
    }

    void Init();
    void Shutdown();

    void Connect(const std::string& address, uint16_t port);
    void Disconnect();

    void SendPacket(const Packet& packet);

    // Process queue
    void Update();

    void SetPacketHandler(std::function<void(const Packet&)> handler);
    ConnectionStatus GetStatus() const { return m_Status; }

private:
    NetworkManager() = default;

    std::shared_ptr<Socket> m_Socket;
    ConnectionStatus m_Status = ConnectionStatus::Disconnected;

    std::queue<Packet> m_IncomingPackets;
    std::mutex m_PacketMutex;

    std::function<void(const Packet&)> m_Handler;

    std::thread m_NetworkThread;
    bool m_Running = false;
};

}  // namespace PyEngine
