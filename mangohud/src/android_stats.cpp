#include "android_stats.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <stdint.h>

namespace {

MangoHudAndroidStatsFn g_cb = nullptr;

// State of the GPU temperature estimate.
float s_last_headroom = 0.f;
int64_t s_last_headroom_ms = 0;
bool s_have_30s = false;
float s_batt_temp_30s_ago = 0.f;
int64_t s_batt_temp_30s_ms = 0;
bool s_have_prev_gpu = false;
float s_prev_gpu = 0.f;

// Cache of the last result, so battery and GPU temp share one JNI round trip.
AndroidStats s_cached;
bool s_cached_ok = false;
int64_t s_cached_ms = -1;

int64_t now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(v, hi));
}

} // namespace

void AndroidStats_SetCallback(MangoHudAndroidStatsFn fn) {
    g_cb = fn;
}

bool AndroidStats_Available() {
    return g_cb != nullptr;
}

bool AndroidStats_Get(AndroidStats& out) {
    if (!g_cb)
        return false;

    const int64_t now = now_ms();
    if (s_cached_ms >= 0 && now - s_cached_ms < 1000) {
        out = s_cached;
        return s_cached_ok;
    }
    s_cached_ms = now;

    // Thermal headroom is cached and fetched at most every 5 seconds.
    const bool query_headroom = (now - s_last_headroom_ms >= 5000);

    MangoHudAndroidRaw raw;
    raw.level = raw.scale = raw.voltage = raw.temperature = raw.status = -1;
    raw.api_level = 0;
    raw.thermal_headroom = std::numeric_limits<float>::quiet_NaN();

    if (!g_cb(&raw, query_headroom ? 1 : 0) || raw.scale <= 0 || raw.level < 0) {
        s_cached_ok = false;
        return false;
    }

    AndroidStats r;

    // 1. Battery percent
    r.battery_percent = (raw.scale > 0) ? (raw.level * 100.0f) / raw.scale : 0.0f;

    // 2. Battery voltage (V)
    r.battery_volt = (raw.voltage > 100) ? (raw.voltage / 1000.0f) : (float)raw.voltage;

    // 3. GPU temperature estimate
    const float t_batt = raw.temperature / 10.0f;
    r.charging = (raw.status == 2) || (raw.status == 5);

    float h;
    if (raw.api_level >= 30) {
        if (query_headroom) {
            if (raw.thermal_headroom == raw.thermal_headroom) // not NaN (old NDK lacks std::isnan)
                s_last_headroom = clampf(raw.thermal_headroom, 0.0f, 1.2f);
            s_last_headroom_ms = now;
        }
        h = s_last_headroom;
    } else {
        h = 0.25f; // fixed average thermal load assumption
    }

    if (!s_have_30s) {
        s_have_30s = true;
        s_batt_temp_30s_ago = t_batt;
        s_batt_temp_30s_ms = now;
    }

    const float h_mult = r.charging ? 14.0f : 26.0f;
    const float heating = std::max(0.0f, t_batt - s_batt_temp_30s_ago);

    if (now - s_batt_temp_30s_ms >= 30000) {
        s_batt_temp_30s_ago = t_batt;
        s_batt_temp_30s_ms = now;
    }

    const float t_gpu_raw = t_batt + 3.0f + (h * h_mult) + (heating * 6.0f);

    float t_gpu_smooth;
    if (!s_have_prev_gpu)
        t_gpu_smooth = t_gpu_raw;
    else
        t_gpu_smooth = (0.2f * t_gpu_raw) + (0.8f * s_prev_gpu);
    s_have_prev_gpu = true;
    s_prev_gpu = t_gpu_smooth;

    r.gpu_temp = std::max(t_batt + 2.0f, std::min(t_gpu_smooth, 95.0f));

    s_cached = r;
    s_cached_ok = true;
    out = r;
    return true;
}
