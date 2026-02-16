#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace PyEngine {

// ═══════════════════════════════════════════════════════════════
// Math Utilities
// ═══════════════════════════════════════════════════════════════
namespace Math {

constexpr float PI = 3.14159265358979323846f;
constexpr float TAU = PI * 2.0f;
constexpr float E = 2.71828182845904523536f;
constexpr float DEG2RAD = PI / 180.0f;
constexpr float RAD2DEG = 180.0f / PI;
constexpr float EPSILON = 1e-6f;
constexpr float GOLDEN_RATIO = 1.6180339887f;

// ── Basic math ───────────────────────────────────────────
inline float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}
inline float InverseLerp(float a, float b, float value) {
    return (value - a) / (b - a);
}
inline float Remap(float value, float fromMin, float fromMax, float toMin, float toMax) {
    return Lerp(toMin, toMax, InverseLerp(fromMin, fromMax, value));
}

inline float SmoothStep(float edge0, float edge1, float x) {
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

inline float SmootherStep(float edge0, float edge1, float x) {
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f);
}

inline float MoveTowards(float current, float target, float maxDelta) {
    float diff = target - current;
    if (std::abs(diff) <= maxDelta)
        return target;
    return current + std::copysign(maxDelta, diff);
}

inline float SmoothDamp(float current, float target, float& velocity, float smoothTime, float deltaTime) {
    float omega = 2.0f / std::max(smoothTime, 0.0001f);
    float x = omega * deltaTime;
    float exp = 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
    float change = current - target;
    float temp = (velocity + omega * change) * deltaTime;
    velocity = (velocity - omega * temp) * exp;
    return target + (change + temp) * exp;
}

inline float PingPong(float t, float length) {
    t = std::fmod(t, length * 2.0f);
    return length - std::abs(t - length);
}

inline float Repeat(float t, float length) {
    return std::clamp(t - std::floor(t / length) * length, 0.0f, length);
}

inline float DampedOscillation(float t, float frequency, float damping) {
    return std::exp(-damping * t) * std::cos(TAU * frequency * t);
}

inline int Sign(float value) {
    return (value > 0) - (value < 0);
}
inline bool Approximately(float a, float b, float eps = EPSILON) {
    return std::abs(a - b) < eps;
}

// ── Angle operations ─────────────────────────────────────
inline float DeltaAngle(float current, float target) {
    float delta = std::fmod(target - current + 180.0f, 360.0f) - 180.0f;
    return delta < -180.0f ? delta + 360.0f : delta;
}

inline float LerpAngle(float a, float b, float t) {
    float delta = DeltaAngle(a, b);
    return a + delta * t;
}

inline float MoveTowardsAngle(float current, float target, float maxDelta) {
    float delta = DeltaAngle(current, target);
    if (std::abs(delta) <= maxDelta)
        return target;
    return current + std::copysign(maxDelta, delta);
}

// ── Vector operations ────────────────────────────────────
inline glm::vec3 MoveTowards(const glm::vec3& current, const glm::vec3& target, float maxDist) {
    glm::vec3 diff = target - current;
    float dist = glm::length(diff);
    if (dist <= maxDist || dist < EPSILON)
        return target;
    return current + diff / dist * maxDist;
}

inline glm::vec3 SmoothDamp(const glm::vec3& current, const glm::vec3& target, glm::vec3& velocity, float smoothTime,
                            float deltaTime) {
    glm::vec3 result;
    result.x = SmoothDamp(current.x, target.x, velocity.x, smoothTime, deltaTime);
    result.y = SmoothDamp(current.y, target.y, velocity.y, smoothTime, deltaTime);
    result.z = SmoothDamp(current.z, target.z, velocity.z, smoothTime, deltaTime);
    return result;
}

inline glm::vec3 ProjectOnPlane(const glm::vec3& vector, const glm::vec3& planeNormal) {
    return vector - planeNormal * glm::dot(vector, planeNormal);
}

inline glm::vec3 Reflect(const glm::vec3& direction, const glm::vec3& normal) {
    return direction - 2.0f * glm::dot(direction, normal) * normal;
}

inline glm::vec3 RotateAround(const glm::vec3& point, const glm::vec3& pivot, const glm::vec3& axis, float angle) {
    glm::quat rotation = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
    return pivot + rotation * (point - pivot);
}

inline glm::vec3 RandomInSphere(std::mt19937& rng, float radius = 1.0f) {
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    glm::vec3 v;
    do {
        v = glm::vec3(dist(rng), dist(rng), dist(rng));
    } while (glm::dot(v, v) > 1.0f);
    return v * radius;
}

inline glm::vec3 RandomOnSphere(std::mt19937& rng, float radius = 1.0f) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float theta = dist(rng) * TAU;
    float phi = std::acos(2.0f * dist(rng) - 1.0f);
    return glm::vec3(std::sin(phi) * std::cos(theta), std::cos(phi), std::sin(phi) * std::sin(theta)) * radius;
}

// ── Easing functions ─────────────────────────────────────
namespace Easing {
inline float InQuad(float t) {
    return t * t;
}
inline float OutQuad(float t) {
    return t * (2.0f - t);
}
inline float InOutQuad(float t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

inline float InCubic(float t) {
    return t * t * t;
}
inline float OutCubic(float t) {
    float f = t - 1.0f;
    return f * f * f + 1.0f;
}
inline float InOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
}

inline float InQuart(float t) {
    return t * t * t * t;
}
inline float OutQuart(float t) {
    float f = t - 1.0f;
    return 1.0f - f * f * f * f;
}

inline float InElastic(float t) {
    return std::sin(13.0f * PI * 0.5f * t) * std::pow(2.0f, 10.0f * (t - 1.0f));
}
inline float OutElastic(float t) {
    return std::sin(-13.0f * PI * 0.5f * (t + 1.0f)) * std::pow(2.0f, -10.0f * t) + 1.0f;
}

inline float OutBounce(float t) {
    if (t < 1.0f / 2.75f)
        return 7.5625f * t * t;
    if (t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    }
    if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    }
    t -= 2.625f / 2.75f;
    return 7.5625f * t * t + 0.984375f;
}
inline float InBounce(float t) {
    return 1.0f - OutBounce(1.0f - t);
}

inline float InBack(float t) {
    return t * t * (2.70158f * t - 1.70158f);
}
inline float OutBack(float t) {
    float f = t - 1.0f;
    return f * f * (2.70158f * f + 1.70158f) + 1.0f;
}

inline float InExpo(float t) {
    return (t == 0.0f) ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f));
}
inline float OutExpo(float t) {
    return (t == 1.0f) ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
}
inline float InOutExpo(float t) {
    if (t == 0.0f || t == 1.0f)
        return t;
    return t < 0.5f ? 0.5f * std::pow(2.0f, 20.0f * t - 10.0f) : 0.5f * (2.0f - std::pow(2.0f, -20.0f * t + 10.0f));
}

inline float InCirc(float t) {
    return 1.0f - std::sqrt(1.0f - t * t);
}
inline float OutCirc(float t) {
    return std::sqrt(1.0f - (t - 1.0f) * (t - 1.0f));
}
inline float InSine(float t) {
    return 1.0f - std::cos(t * PI * 0.5f);
}
inline float OutSine(float t) {
    return std::sin(t * PI * 0.5f);
}
inline float InOutSine(float t) {
    return 0.5f * (1.0f - std::cos(PI * t));
}
}  // namespace Easing
}  // namespace Math

// ═══════════════════════════════════════════════════════════════
// Color Utilities
// ═══════════════════════════════════════════════════════════════
namespace ColorUtils {

inline glm::vec4 FromHex(const std::string& hex) {
    unsigned int r = 0, g = 0, b = 0, a = 255;
    std::string h = hex;
    if (h[0] == '#')
        h = h.substr(1);

    if (h.length() == 6) {
        sscanf(h.c_str(), "%02x%02x%02x", &r, &g, &b);
    } else if (h.length() == 8) {
        sscanf(h.c_str(), "%02x%02x%02x%02x", &r, &g, &b, &a);
    }

    return glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

inline std::string ToHex(const glm::vec4& color) {
    char buf[10];
    snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", static_cast<int>(color.r * 255), static_cast<int>(color.g * 255),
             static_cast<int>(color.b * 255), static_cast<int>(color.a * 255));
    return std::string(buf);
}

inline glm::vec3 RGBToHSV(const glm::vec3& rgb) {
    float maxC = std::max({rgb.r, rgb.g, rgb.b});
    float minC = std::min({rgb.r, rgb.g, rgb.b});
    float delta = maxC - minC;

    float h = 0.0f, s = 0.0f, v = maxC;
    if (delta > Math::EPSILON) {
        s = delta / maxC;
        if (rgb.r >= maxC)
            h = (rgb.g - rgb.b) / delta;
        else if (rgb.g >= maxC)
            h = 2.0f + (rgb.b - rgb.r) / delta;
        else
            h = 4.0f + (rgb.r - rgb.g) / delta;
        h *= 60.0f;
        if (h < 0.0f)
            h += 360.0f;
    }
    return {h, s, v};
}

inline glm::vec3 HSVToRGB(const glm::vec3& hsv) {
    float h = hsv.x, s = hsv.y, v = hsv.z;
    float c = v * s;
    float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    glm::vec3 rgb;
    if (h < 60)
        rgb = {c, x, 0};
    else if (h < 120)
        rgb = {x, c, 0};
    else if (h < 180)
        rgb = {0, c, x};
    else if (h < 240)
        rgb = {0, x, c};
    else if (h < 300)
        rgb = {x, 0, c};
    else
        rgb = {c, 0, x};

    return rgb + m;
}

inline float Luminance(const glm::vec3& color) {
    return glm::dot(color, glm::vec3(0.2126f, 0.7152f, 0.0722f));
}

inline glm::vec3 GammaToLinear(const glm::vec3& color) {
    return glm::pow(color, glm::vec3(2.2f));
}

inline glm::vec3 LinearToGamma(const glm::vec3& color) {
    return glm::pow(color, glm::vec3(1.0f / 2.2f));
}

inline glm::vec4 LerpColor(const glm::vec4& a, const glm::vec4& b, float t) {
    return glm::mix(a, b, t);
}

// Predefined colors
inline glm::vec4 Red() {
    return {1.0f, 0.0f, 0.0f, 1.0f};
}
inline glm::vec4 Green() {
    return {0.0f, 1.0f, 0.0f, 1.0f};
}
inline glm::vec4 Blue() {
    return {0.0f, 0.0f, 1.0f, 1.0f};
}
inline glm::vec4 White() {
    return {1.0f, 1.0f, 1.0f, 1.0f};
}
inline glm::vec4 Black() {
    return {0.0f, 0.0f, 0.0f, 1.0f};
}
inline glm::vec4 Yellow() {
    return {1.0f, 1.0f, 0.0f, 1.0f};
}
inline glm::vec4 Cyan() {
    return {0.0f, 1.0f, 1.0f, 1.0f};
}
inline glm::vec4 Magenta() {
    return {1.0f, 0.0f, 1.0f, 1.0f};
}
inline glm::vec4 Orange() {
    return {1.0f, 0.5f, 0.0f, 1.0f};
}
inline glm::vec4 Purple() {
    return {0.5f, 0.0f, 0.5f, 1.0f};
}
inline glm::vec4 Gray() {
    return {0.5f, 0.5f, 0.5f, 1.0f};
}
inline glm::vec4 Clear() {
    return {0.0f, 0.0f, 0.0f, 0.0f};
}
}  // namespace ColorUtils

// ═══════════════════════════════════════════════════════════════
// String Utilities
// ═══════════════════════════════════════════════════════════════
namespace StringUtils {

inline std::vector<std::string> Split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream stream(str);
    std::string token;
    while (std::getline(stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

inline std::string Trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\n\r");
    auto end = str.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

inline std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

inline std::string ToUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

inline bool StartsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

inline bool EndsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

inline bool Contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

inline std::string Replace(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

inline std::string Join(const std::vector<std::string>& parts, const std::string& separator) {
    std::string result;
    for (size_t i = 0; i < parts.size(); i++) {
        if (i > 0)
            result += separator;
        result += parts[i];
    }
    return result;
}

inline std::string FormatBytes(size_t bytes) {
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    int i = 0;
    double size = static_cast<double>(bytes);
    while (size >= 1024.0 && i < 4) {
        size /= 1024.0;
        i++;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(i > 0 ? 2 : 0) << size << " " << suffixes[i];
    return oss.str();
}

inline std::string FormatDuration(float seconds) {
    int min = static_cast<int>(seconds) / 60;
    float sec = seconds - min * 60;
    std::ostringstream oss;
    oss << min << ":" << std::setfill('0') << std::setw(5) << std::fixed << std::setprecision(2) << sec;
    return oss.str();
}

inline uint32_t Hash(const std::string& str) {
    uint32_t hash = 5381;
    for (char c : str)
        hash = ((hash << 5) + hash) + static_cast<uint32_t>(c);
    return hash;
}
}  // namespace StringUtils

// ═══════════════════════════════════════════════════════════════
// Timer & Stopwatch
// ═══════════════════════════════════════════════════════════════
class Timer {
public:
    Timer() : m_StartTime(std::chrono::high_resolution_clock::now()) {}

    void Reset() { m_StartTime = std::chrono::high_resolution_clock::now(); }

    float ElapsedSeconds() const {
        return std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - m_StartTime).count();
    }
    float ElapsedMilliseconds() const { return ElapsedSeconds() * 1000.0f; }
    float ElapsedMicroseconds() const { return ElapsedSeconds() * 1000000.0f; }

private:
    std::chrono::high_resolution_clock::time_point m_StartTime;
};

class Stopwatch {
public:
    void Start() {
        m_Running = true;
        m_Start = std::chrono::high_resolution_clock::now();
    }
    void Stop() {
        m_Running = false;
        m_End = std::chrono::high_resolution_clock::now();
    }
    void Reset() {
        m_Running = false;
        m_Start = m_End = std::chrono::high_resolution_clock::now();
        m_Laps.clear();
    }

    void Lap() {
        if (m_Running) {
            auto now = std::chrono::high_resolution_clock::now();
            m_Laps.push_back(std::chrono::duration<float, std::milli>(now - m_Start).count());
        }
    }

    float ElapsedMs() const {
        auto end = m_Running ? std::chrono::high_resolution_clock::now() : m_End;
        return std::chrono::duration<float, std::milli>(end - m_Start).count();
    }

    const std::vector<float>& GetLaps() const { return m_Laps; }
    bool IsRunning() const { return m_Running; }

private:
    bool m_Running = false;
    std::chrono::high_resolution_clock::time_point m_Start;
    std::chrono::high_resolution_clock::time_point m_End;
    std::vector<float> m_Laps;
};

// ═══════════════════════════════════════════════════════════════
// UUID Generator
// ═══════════════════════════════════════════════════════════════
class UUID {
public:
    UUID() : m_UUID(Generate()) {}
    UUID(uint64_t uuid) : m_UUID(uuid) {}

    uint64_t GetValue() const { return m_UUID; }
    operator uint64_t() const { return m_UUID; }

    bool operator==(const UUID& other) const { return m_UUID == other.m_UUID; }
    bool operator!=(const UUID& other) const { return m_UUID != other.m_UUID; }
    bool operator<(const UUID& other) const { return m_UUID < other.m_UUID; }

    std::string ToString() const {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0') << std::setw(16) << m_UUID;
        return oss.str();
    }

    static uint64_t Generate() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dist;
        return dist(gen);
    }

private:
    uint64_t m_UUID;
};

}  // namespace PyEngine

namespace std {
template <>
struct hash<PyEngine::UUID> {
    size_t operator()(const PyEngine::UUID& uuid) const { return std::hash<uint64_t>()(uuid.GetValue()); }
};
}  // namespace std
