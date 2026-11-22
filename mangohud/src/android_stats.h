#pragma once
#ifndef MANGOHUD_ANDROID_STATS_H
#define MANGOHUD_ANDROID_STATS_H

#include "mangohud/mangohud.h"

// Port: values derived from Android's BatteryManager / PowerManager.
struct AndroidStats {
    float battery_percent = 0.f;  // 0..100
    float battery_volt = 0.f;     // V
    bool charging = false;        // charging or full
    float gpu_temp = 0.f;         // estimated, degrees C
};

void AndroidStats_SetCallback(MangoHudAndroidStatsFn fn);
bool AndroidStats_Available();

// Refreshes at most once per second (shared by battery and GPU temperature).
// Returns false if the Android side could not provide values.
bool AndroidStats_Get(AndroidStats& out);

#endif
