/**
 ******************************************************************************
 * @file           : tmp117.h
 * @brief          : Header for tmp117.c file.
 *                   This file contains the common defines of tmp117's driver.
 ******************************************************************************
 */

// #pragma once // non standard I guess?

#ifndef __TMP117_H__
#define __TMP117_H__

#include "stm32l0xx_hal.h"
#include <stdint.h>

// addresses
#define TMP117_ADDR_GND 0x48
#define TMP117_ADDR_VDD 0x49
#define TMP117_ADDR_SDA 0x4A
#define TMP117_ADDR_SCL 0x4B

/**
 * @brief TMP117 I2C address.
 *
 * @note If TMP117_I2C_ADDR is not defined at compile time,
 *       the default address is TMP117_ADDR_GND (0x48).
 *       you can use the -D directive to override it.
 *       This would not work with multiple TMP117s but
 *       it's good enough for now.
 */
#ifndef TMP117_ADDR
#define TMP117_ADDR TMP117_ADDR_GND
#endif

// sensor resolution
#define TMP117_RESOLUTION 7.8125e-3f

/**
 * sensor registers
 */
// config register
#define TMP117_CONFIG_REG	   0x01

// temp result register
#define TMP117_TEMP_RES_REG    0x00
// temp high limit register
#define TMP117_THIGH_LIM_REG   0x02
// temp low limit register
#define TMP117_TLOW_LIM_REG	   0x03

// EEPROM unlock register
#define TMP117_EEPROM_UL_REG   0x04
// EEPROM1 register
#define TMP117_EEPROM1_REG	   0x05
// EEPROM2 register
#define TMP117_EEPROM2_REG	   0x06
// EEPROM3 register
#define TMP117_EEPROM3_REG	   0x08

// device ID register
#define TMP117_DEVICE_ID_REG   0x0F
// temp offset register
#define TMP117_TEMP_OFFSET_REG 0x07

#define TMP117_OK 0x00
#define TMP117_ERR 0xFF // generic error

// config
/**
 * @brief set config register
 *
 * @param i2c I2C handler
 * @param config config reg value
 * @return TMP117_OK on success, otherwise an error code
 */
uint8_t setConfig(I2C_HandleTypeDef* i2c, uint16_t config);

/**
 * @brief get config register
 *
 * @param i2c I2C handler
 * @param[out] config config reg value
 * @return TMP117_OK on success, otherwise an error code
 */
uint8_t getConfig(I2C_HandleTypeDef* i2c, uint16_t* config);

/**
 * @brief execute software reset: set reset bit to 1, when it reads back 0
 * 		  the software reset has been completed
 *
 * @param i2c I2C handler
 * @return TMP117_OK on success, otherwise an error code
 */
uint8_t softReset(I2C_HandleTypeDef* i2c);

/**
 * @brief get chip id
 *
 * @param i2c I2C handler
 * @param[out] id chip ID
 * @return TMP117_OK on success, otherwise an error code
 */
uint8_t getDeviceID(I2C_HandleTypeDef* i2c, uint16_t* id);

// temp measurement / temp config
/**
 * @brief get temperature
 *
 * @param i2c I2C handler
 * @param[out] temp measured temp
 * @return TMP117_OK on success, otherwise an error code
 */
uint8_t getMeasuredTemp(I2C_HandleTypeDef* i2c, float* temp);

/**
 * @brief set the high limit temperature for system:
 * 						- following power-up or a general-call reset, the high-limit register is loaded with the
 * 						  stored value from the EEPROM. The factory default reset value is 6000h.
 *
 * @param i2c I2C handler
 * @param temp temp high limit
 * @return TMP117_OK on success, otherwise an error code'
 */
uint8_t setHighLimitTemp(I2C_HandleTypeDef* i2c, float temp);

/**
 * @brief  set the low limit temperature for system:
 * 						- following power-up or a general-call reset, the low-limit register is loaded with the
 * 						stored value from the EEPROM. The factory default reset value is 8000h.
 *
 * @param i2c I2C handler
 * @param temp temp low limit
 * @return TMP117_OK on success, otherwise an error code'
 */
uint8_t setLowLimitTemp(I2C_HandleTypeDef* i2c, float temp);

/**
 * @brief set the temp offset:
 * 						- target temperature for calibration (the difference between this value
 * 						and the result temperature value from the sensor) will be passed as an offset
 * 						this is used as a form of calibration
 *
 * @param i2c I2C handler
 * @param target_temp target temperature for calibration
 * @return TMP117_OK on success, otherwise an error code'
 */
uint8_t setOffsetTemp(I2C_HandleTypeDef* i2c, float target_temp);

/**
 * @brief get offset temperature
 *
 * @param i2c I2C handler
 * @param[out] offset offset register value
 * @return TMP117_OK on success, otherwise an error code
 */
uint8_t getOffsetTemp(I2C_HandleTypeDef* i2c, float* offset);

/**
 * @brief get high limit temperature
 *
 * @param i2c I2C handler
 * @param[out] high_lim high limit temperature register value
 * @return TMP117_OK on success, otherwise an error code
 */
uint8_t getHighLimitTemp(I2C_HandleTypeDef* i2c, float* high_lim);

/**
 * @brief get low limit temperature
 *
 * @param i2c I2C handler
 * @param[out] low_lim high limit temperature register value
 * @return TMP117_OK on success, otherwise an error code
 */
uint8_t getLowLimitTemp(I2C_HandleTypeDef* i2c, float* low_lim);

// EEPROM
/**
 * @brief  read EEPROM
 *
 * @param i2c I2C handler
 * @param eeprom_idx EEPROM location's number
 * @param data[out] data read from the EEPROM location (TMP117_EEPROM[1:3]_REG)
 * @return TMP117_OK on success, otherwise an error code
 *
 * @note if this return TMP117_ERR is most likely due to the EEPROM being busy
 */
uint8_t readEEPROM (I2C_HandleTypeDef* i2c, uint8_t eeprom_idx, uint16_t* data);

/**
 * @brief  write EEPROM
 *
 * @param i2c I2C handler
 * @param eeprom_idx EEPROM location's number
 * @param data data written to the EEPROM location (TMP117_EEPROM[1:3]_REG)
 * @return TMP117_OK on success, otherwise an error code
 */
uint8_t writeEEPROM (I2C_HandleTypeDef* i2c, uint8_t eeprom_idx, uint16_t data);

#endif
