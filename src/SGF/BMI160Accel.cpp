#include "BMI160Accel.h"

#include <Arduino.h>
#include <Wire.h>

namespace {
constexpr uint8_t BMI160_CHIP_ID_REGISTER = 0x00;
constexpr uint8_t BMI160_CHIP_ID = 0xD1;
constexpr uint8_t BMI160_ACCEL_DATA_REGISTER = 0x12;
constexpr uint8_t BMI160_ACCEL_CONFIG_REGISTER = 0x40;
constexpr uint8_t BMI160_ACCEL_RANGE_REGISTER = 0x41;
constexpr uint8_t BMI160_COMMAND_REGISTER = 0x7E;
constexpr uint8_t BMI160_SOFT_RESET_COMMAND = 0xB6;
constexpr uint8_t BMI160_ACCEL_NORMAL_MODE_COMMAND = 0x11;
constexpr uint8_t BMI160_ACCEL_RANGE_2G = 0x03;
constexpr uint8_t BMI160_ACCEL_CONFIG_100HZ = 0x28;
constexpr float BMI160_ACCEL_LSB_PER_G = 16384.0f;

float updateIntervalSeconds() {
    if (BMI160_ACCEL_FPS == 0) {
        return 0.0f;
    }
    return 1.0f / (float)BMI160_ACCEL_FPS;
}
}  // namespace

BMI160Accel::BMI160Accel(uint8_t sdaPin, uint8_t sclPin,
                                         uint8_t i2cAddress)
    : sdaPin(sdaPin), sclPin(sclPin), i2cAddress(i2cAddress) {}

void BMI160Accel::attach(uint8_t sdaPin, uint8_t sclPin,
                         uint8_t i2cAddress) {
    this->sdaPin = sdaPin;
    this->sclPin = sclPin;
    this->i2cAddress = i2cAddress;
    initialized = false;
    updateAccumulator = 0.0f;
    acceleration = Vector3f{};
}

bool BMI160Accel::begin() {
    Wire.begin();

    if (!writeRegister(BMI160_COMMAND_REGISTER, BMI160_SOFT_RESET_COMMAND)) {
        return false;
    }
    delay(20);

    uint8_t chipId = 0;
    if (!readRegisters(BMI160_CHIP_ID_REGISTER, &chipId, 1)) {
        return false;
    }
    if (chipId != BMI160_CHIP_ID) {
        return false;
    }

    if (!writeRegister(BMI160_COMMAND_REGISTER, BMI160_ACCEL_NORMAL_MODE_COMMAND)) {
        return false;
    }
    delay(10);

    if (!writeRegister(BMI160_ACCEL_CONFIG_REGISTER, BMI160_ACCEL_CONFIG_100HZ)) {
        return false;
    }
    if (!writeRegister(BMI160_ACCEL_RANGE_REGISTER, BMI160_ACCEL_RANGE_2G)) {
        return false;
    }

    initialized = readAcceleration();
    updateAccumulator = 0.0f;
    return initialized;
}

void BMI160Accel::update(float delta) {
    if (!initialized) {
        return;
    }

    float interval = updateIntervalSeconds();
    if (interval <= 0.0f) {
        readAcceleration();
        return;
    }

    updateAccumulator += delta;
    if (updateAccumulator < interval) {
        return;
    }

    updateAccumulator -= interval;
    readAcceleration();
}

const Vector3f& BMI160Accel::getAcceleration() const {
    return acceleration;
}

bool BMI160Accel::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(i2cAddress);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool BMI160Accel::readRegisters(uint8_t startReg, uint8_t* data, uint8_t size) {
    Wire.beginTransmission(i2cAddress);
    Wire.write(startReg);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    uint8_t readCount = Wire.requestFrom((int)i2cAddress, (int)size);
    if (readCount != size) {
        return false;
    }

    for (uint8_t i = 0; i < size; ++i) {
        data[i] = (uint8_t)Wire.read();
    }
    return true;
}

bool BMI160Accel::readAcceleration() {
    uint8_t data[6];
    if (!readRegisters(BMI160_ACCEL_DATA_REGISTER, data, sizeof(data))) {
        return false;
    }

    int16_t rawX = (int16_t)((uint16_t)data[1] << 8 | data[0]);
    int16_t rawY = (int16_t)((uint16_t)data[3] << 8 | data[2]);
    int16_t rawZ = (int16_t)((uint16_t)data[5] << 8 | data[4]);

    acceleration.x = (float)rawX / BMI160_ACCEL_LSB_PER_G;
    acceleration.y = (float)rawY / BMI160_ACCEL_LSB_PER_G;
    acceleration.z = (float)rawZ / BMI160_ACCEL_LSB_PER_G;
    return true;
}
