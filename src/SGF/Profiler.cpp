#include "Profiler.h"

Profiler::Profiler(const char* name, Slot* slots, uint8_t slotCapacity)
    : profilerName(name), slots(slots), slotCapacity(slotCapacity) {}

void Profiler::setName(const char* name) { profilerName = name; }

const char* Profiler::name() const { return profilerName; }

uint8_t Profiler::capacity() const { return slotCapacity; }

uint8_t Profiler::usedSlots() const { return usedSlotCount; }

bool Profiler::isUsed(int slot) const {
    const Slot* slotData = slotAt(slot);
    if (!slotData) {
        return false;
    }
    return slotData->used;
}

bool Profiler::hasData() const {
    for (uint8_t i = 0; i < slotCapacity; ++i) {
        const Slot& slotData = slots[i];
        if (!slotData.used) {
            continue;
        }
        if (slotData.value != 0 || slotData.samples != 0 || slotData.maxValue != 0) {
            return true;
        }
    }
    return false;
}

void Profiler::setLabel(int slot, const char* label) {
    Slot* slotData = slotAt(slot);
    if (!slotData) {
        return;
    }
    markUsed(*slotData);
    slotData->label = label;
}

void Profiler::setMode(int slot, SlotMode mode) {
    Slot* slotData = slotAt(slot);
    if (!slotData) {
        return;
    }
    markUsed(*slotData);
    slotData->mode = mode;
}

const char* Profiler::label(int slot) const {
    const Slot* slotData = slotAt(slot);
    if (!slotData) {
        return nullptr;
    }
    return slotData->label;
}

Profiler::SlotMode Profiler::mode(int slot) const {
    const Slot* slotData = slotAt(slot);
    if (!slotData) {
        return CounterSlot;
    }
    return slotData->mode;
}

void Profiler::increment(int slot, uint32_t amount) {
    record(slot, amount, CounterSlot);
}

void Profiler::probe(int slot, uint32_t amount) {
    record(slot, amount, SampleSlot);
}

void Profiler::reset() {
    for (uint8_t i = 0; i < slotCapacity; ++i) {
        slots[i].value = 0;
        slots[i].samples = 0;
        slots[i].maxValue = 0;
    }
}

uint32_t Profiler::value(int slot) const {
    const Slot* slotData = slotAt(slot);
    if (!slotData) {
        return 0;
    }
    return slotData->value;
}

uint32_t Profiler::samples(int slot) const {
    const Slot* slotData = slotAt(slot);
    if (!slotData) {
        return 0;
    }
    return slotData->samples;
}

uint32_t Profiler::average(int slot) const {
    const Slot* slotData = slotAt(slot);
    if (!slotData || slotData->samples == 0) {
        return 0;
    }
    return slotData->value / slotData->samples;
}

uint32_t Profiler::maxValue(int slot) const {
    const Slot* slotData = slotAt(slot);
    if (!slotData) {
        return 0;
    }
    return slotData->maxValue;
}

Profiler::Slot* Profiler::slotAt(int slot) {
    if (slot < 0 || slot >= slotCapacity || !slots) {
        return nullptr;
    }
    return &slots[slot];
}

const Profiler::Slot* Profiler::slotAt(int slot) const {
    if (slot < 0 || slot >= slotCapacity || !slots) {
        return nullptr;
    }
    return &slots[slot];
}

void Profiler::markUsed(Slot& slotData) {
    if (slotData.used) {
        return;
    }
    slotData.used = true;
    usedSlotCount++;
}

void Profiler::record(int slot, uint32_t amount, SlotMode mode) {
    Slot* slotData = slotAt(slot);
    if (!slotData) {
        return;
    }

    markUsed(*slotData);
    slotData->mode = mode;
    slotData->value += amount;
    slotData->samples++;
    if (amount > slotData->maxValue) {
        slotData->maxValue = amount;
    }
}
