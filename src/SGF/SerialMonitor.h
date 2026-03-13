#pragma once

#include <stdint.h>

class Profiler;

class SerialMonitor {
   public:
    static constexpr uint8_t MAX_PROFILERS = 8;

    explicit SerialMonitor(uint32_t intervalMs = 1000, uint32_t baudRate = 115200);

    void begin();
    void tick();

    bool attachProfiler(Profiler& profiler);
    void dumpProfilers();

   private:
    uint32_t intervalMs;
    uint32_t baudRate;
    uint32_t lastDumpMs = 0;
    uint32_t lastDumpElapsedMs = 0;
    bool started = false;
    Profiler* profilers[MAX_PROFILERS]{};
    uint8_t profilerCount = 0;

    bool isAttached(const Profiler& profiler) const;
    void dumpProfiler(const Profiler& profiler);
};
