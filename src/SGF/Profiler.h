#pragma once

#include <stdint.h>

class Profiler {
   public:
    enum SlotMode {
        CounterSlot,
        SampleSlot
    };

    struct Slot {
        const char* label = nullptr;
        uint32_t value = 0;
        uint32_t samples = 0;
        uint32_t maxValue = 0;
        bool used = false;
        SlotMode mode = CounterSlot;
    };

    Profiler(const char* name, Slot* slots, uint8_t slotCapacity);

    void setName(const char* name);
    const char* name() const;

    uint8_t capacity() const;
    uint8_t usedSlots() const;
    bool isUsed(int slot) const;
    bool hasData() const;

    void setLabel(int slot, const char* label);
    void setMode(int slot, SlotMode mode);
    const char* label(int slot) const;
    SlotMode mode(int slot) const;

    void increment(int slot, uint32_t amount = 1);
    void probe(int slot, uint32_t amount);

    void reset();
    uint32_t value(int slot) const;
    uint32_t samples(int slot) const;
    uint32_t average(int slot) const;
    uint32_t maxValue(int slot) const;

   private:
    const char* profilerName;
    Slot* slots;
    uint8_t slotCapacity;
    uint8_t usedSlotCount = 0;

    Slot* slotAt(int slot);
    const Slot* slotAt(int slot) const;
    void markUsed(Slot& slotData);
    void record(int slot, uint32_t amount, SlotMode mode);
};
