#include "PyEngine/Network/NetworkSystem.hpp"

#include <chrono>

#include "PyEngine/Core/Log.hpp"

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// Mock Socket Implementations
// ═══════════════════════════════════════════════════════════════

bool TCPSocket::Open(const std::string& address, uint16_t port) {
    // In a real implementation: socket(), connect()
    PYENGINE_CORE_INFO("TCPSocket connecting to {}:{}", address, port);
    m_IsOpen = true;
    return true;
}

void TCPSocket::Close() {
    PYENGINE_CORE_INFO("TCPSocket closing");
    m_IsOpen = false;
}

bool TCPSocket::Send(const Packet& packet) {
    if (!m_IsOpen)
        return false;
    // send()
    return true;
}

bool TCPSocket::Receive(Packet& outPacket) {
    if (!m_IsOpen)
        return false;
    // recv()
    // For demo, pretend we didn't receive anything
    return false;
}

bool UDPSocket::Open(const std::string& address, uint16_t port) {
    PYENGINE_CORE_INFO("UDPSocket bound to port {}", port);
    m_IsOpen = true;
    return true;
}

void UDPSocket::Close() {
    m_IsOpen = false;
}

bool UDPSocket::Send(const Packet& packet) {
    if (!m_IsOpen)
        return false;
    return true;
}

bool UDPSocket::Receive(Packet& outPacket) {
    return false;
}

// ═══════════════════════════════════════════════════════════════
// Network Manager
// ═══════════════════════════════════════════════════════════════

void NetworkManager::Init() {
    PYENGINE_CORE_INFO("NetworkManager Initialized");
    m_Socket = std::make_shared<TCPSocket>();
}

void NetworkManager::Shutdown() {
    Disconnect();
}

void NetworkManager::Connect(const std::string& address, uint16_t port) {
    if (m_Status == ConnectionStatus::Connected || m_Status == ConnectionStatus::Connecting)
        return;

    m_Status = ConnectionStatus::Connecting;
    m_Running = true;

    // Launch network thread
    m_NetworkThread = std::thread([this, address, port]() {
        if (m_Socket->Open(address, port)) {
            m_Status = ConnectionStatus::Connected;
            PYENGINE_CORE_INFO("Connected to server");

            while (m_Running && m_Socket->IsOpen()) {
                Packet p;
                if (m_Socket->Receive(p)) {
                    std::lock_guard<std::mutex> lock(m_PacketMutex);
                    m_IncomingPackets.push(p);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        } else {
            m_Status = ConnectionStatus::Failed;
            PYENGINE_CORE_ERROR("Failed to connect");
        }
    });
    m_NetworkThread.detach();
}

void NetworkManager::Disconnect() {
    m_Running = false;
    if (m_Socket)
        m_Socket->Close();
    m_Status = ConnectionStatus::Disconnected;
}

void NetworkManager::SendPacket(const Packet& packet) {
    if (m_Socket)
        m_Socket->Send(packet);
}

void NetworkManager::Update() {
    std::lock_guard<std::mutex> lock(m_PacketMutex);
    while (!m_IncomingPackets.empty()) {
        Packet p = m_IncomingPackets.front();
        m_IncomingPackets.pop();

        if (m_Handler)
            m_Handler(p);
    }
}

void NetworkManager::SetPacketHandler(std::function<void(const Packet&)> handler) {
    m_Handler = handler;
}

}  // namespace PyEngine
