#include "SerialMonitor.h"

#include <Arduino.h>
#include <string.h>

#include "Profiler.h"

namespace {
void printSlotLabel(const Profiler& profiler, int slot) {
    const char* label = profiler.label(slot);
    if (label) {
        Serial.print(label);
        return;
    }

    Serial.print("slot");
    Serial.print(slot);
}

int findSlotByLabel(const Profiler& profiler, const char* label) {
    for (int slot = 0; slot < profiler.capacity(); ++slot) {
        const char* slotLabel = profiler.label(slot);
        if (!slotLabel) {
            continue;
        }
        if (strcmp(slotLabel, label) == 0) {
            return slot;
        }
    }
    return -1;
}
}  // namespace

SerialMonitor::SerialMonitor(uint32_t intervalMs, uint32_t baudRate)
    : intervalMs(intervalMs), baudRate(baudRate) {}

void SerialMonitor::begin() {
    if (started) {
        return;
    }

    Serial.begin(baudRate);
    started = true;
    lastDumpMs = millis();
}

void SerialMonitor::tick() {
    if (!started) {
        begin();
    }

    uint32_t nowMs = millis();
    uint32_t elapsedMs = nowMs - lastDumpMs;
    if (elapsedMs < intervalMs) {
        return;
    }

    lastDumpElapsedMs = elapsedMs;
    dumpProfilers();
    lastDumpMs = nowMs;
}

bool SerialMonitor::attachProfiler(Profiler& profiler) {
    if (isAttached(profiler)) {
        return true;
    }
    if (profilerCount >= MAX_PROFILERS) {
        return false;
    }

    profilers[profilerCount] = &profiler;
    profilerCount++;
    return true;
}

void SerialMonitor::dumpProfilers() {
    for (uint8_t i = 0; i < profilerCount; ++i) {
        if (!profilers[i]) {
            continue;
        }
        dumpProfiler(*profilers[i]);
        profilers[i]->reset();
    }
}

bool SerialMonitor::isAttached(const Profiler& profiler) const {
    for (uint8_t i = 0; i < profilerCount; ++i) {
        if (profilers[i] == &profiler) {
            return true;
        }
    }
    return false;
}

void SerialMonitor::dumpProfiler(const Profiler& profiler) {
    if (!profiler.hasData()) {
        return;
    }

    Serial.print("[");
    Serial.print(profiler.name() ? profiler.name() : "unnamed");
    Serial.print("]");

    const char* profilerName = profiler.name();
    if (profilerName && strcmp(profilerName, "Renderer2D") == 0 && lastDumpElapsedMs != 0) {
        int renderCallsSlot = findSlotByLabel(profiler, "render_calls");
        if (renderCallsSlot >= 0 && profiler.mode(renderCallsSlot) == Profiler::CounterSlot) {
            uint32_t fps = (profiler.value(renderCallsSlot) * 1000u) / lastDumpElapsedMs;
            Serial.print(" fps=");
            Serial.print(fps);
        }
    }

    for (int slot = 0; slot < profiler.capacity(); ++slot) {
        if (!profiler.isUsed(slot)) {
            continue;
        }

        uint32_t value = profiler.value(slot);
        Profiler::SlotMode mode = profiler.mode(slot);

        if (mode == Profiler::CounterSlot && lastDumpElapsedMs != 0) {
            value = (value * 1000u) / lastDumpElapsedMs;
        }

        Serial.print(' ');
        printSlotLabel(profiler, slot);
        Serial.print('=');
        if (mode == Profiler::SampleSlot) {
            uint32_t avgUs = profiler.average(slot);
            Serial.print(avgUs / 1000u);
            Serial.print('.');
            Serial.print((avgUs % 1000u) / 100u);
            Serial.print("ms");
        } else {
            Serial.print(value);
        }
    }

    Serial.println();
}
