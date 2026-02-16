#pragma once

#include <cstdint>
#include <functional>

namespace PyEngine {

class UUID {
public:
    UUID();
    explicit UUID(uint64_t uuid);

    operator uint64_t() const { return m_UUID; }

    bool operator==(const UUID& other) const { return m_UUID == other.m_UUID; }
    bool operator!=(const UUID& other) const { return m_UUID != other.m_UUID; }

private:
    uint64_t m_UUID;
};

}  // namespace PyEngine

// Hash specialization for use in unordered containers
namespace std {
template <>
struct hash<PyEngine::UUID> {
    size_t operator()(const PyEngine::UUID& uuid) const { return hash<uint64_t>{}(static_cast<uint64_t>(uuid)); }
};
}  // namespace std
