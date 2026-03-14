#pragma once

#ifndef BMI160_ACCEL_FPS
#define BMI160_ACCEL_FPS 60
#endif

#include <stdint.h>

#include "Vector3.h"

class BMI160Accel {
    public:
    static constexpr uint8_t DEFAULT_I2C_ADDRESS = 0x68;

    BMI160Accel() = default;
    BMI160Accel(uint8_t sdaPin, uint8_t sclPin,
                uint8_t i2cAddress = DEFAULT_I2C_ADDRESS);

    void attach(uint8_t sdaPin, uint8_t sclPin,
                uint8_t i2cAddress = DEFAULT_I2C_ADDRESS);
    bool begin();
    void update(float delta);
    const Vector3f& getAcceleration() const;

    private:
    uint8_t sdaPin;
    uint8_t sclPin;
    uint8_t i2cAddress;
    bool initialized = false;
    float updateAccumulator = 0.0f;
    Vector3f acceleration;

    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegisters(uint8_t startReg, uint8_t* data, uint8_t size);
    bool readAcceleration();
};
