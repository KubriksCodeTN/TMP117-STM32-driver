/**
 ******************************************************************************
 * @file           : tmp117.c
 * @brief          : This file contains the definitions of tmp117's
 *                   driver functions.
 ******************************************************************************
 */

#include "tmp117.h"

// SW reset bit
#define SW_RESET (1UL << 1)
// I2C W bit
#define W 0x00
// I2C R bit
#define R 0x01

// PRIVATE
/**
 * @brief write a 16-bit register
 *
 * @param i2c I2C handler
 * @param reg_addr register address
 * @param reg_value register value to write
 * @return TMP117_OK on success, otherwise an error code
 */
static uint8_t writeI2CReg(I2C_HandleTypeDef* i2c, uint8_t reg_addr, uint16_t reg_val) {
	// buffer to write
	uint8_t buff[3];
	buff[0] = reg_addr;
	// MSB
	buff[1] = (reg_val >> 8);
	// LSB
	buff[2] = (reg_val & 0xFF);

	// transmit the data to the sensor
	uint8_t ret = HAL_I2C_Master_Transmit(i2c, (TMP117_ADDR << 1) | W, buff, 3, HAL_MAX_DELAY);
	return ret;
}

/**
 * @brief read a 16-bit register value
 *
 * @param i2c I2C handler
 * @param reg_addr register address
 * @param reg_value[out] read register value
 * @return TMP117_OK on success, otherwise an error code
*/
static uint8_t readI2CReg(I2C_HandleTypeDef* i2c, uint8_t reg_addr, uint16_t* reg_val) {
	// I think this is needed due to endianness but might be useless
	// better safe than sorry
	uint8_t buff[2];
	uint8_t ret_val = HAL_I2C_Mem_Read(i2c, TMP117_ADDR << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, buff, 2, HAL_MAX_DELAY);
	*reg_val = (buff[0] << 8) | buff[1];

	return ret_val;
}

/**
 * @brief get a temperature value
 *
 * @param i2c I2C handler
 * @param reg_addr register address
 * @param temp[out] read temp value
 * @return TMP117_OK on success, otherwise an error code
*/
static uint8_t getTemperature(I2C_HandleTypeDef* i2c, uint8_t reg_addr, float* temp) {
	uint8_t ret_val;
	uint16_t raw_temp;

	if ((ret_val = readI2CReg(i2c, reg_addr, &raw_temp)))
	    return ret_val;

	*temp = (int16_t)raw_temp * TMP117_RESOLUTION;

	return TMP117_OK;
}

/**
 * @brief lock EEPROM, setting bit 15 to 0
 *
 * @param i2c I2C handler
 * @return TMP117_OK on success, otherwise an error code
 */
static uint8_t lockEEPROM(I2C_HandleTypeDef* i2c) {
	return writeI2CReg(i2c, TMP117_EEPROM_UL_REG, 0);
}

/**
 * @brief unlock EEPROM, setting bit 15 to 1
 *
 * @param i2c I2C handler
 * @return TMP117_OK on success, otherwise an error code
 */
static uint8_t unlockEEPROM(I2C_HandleTypeDef* i2c) {
	return writeI2CReg(i2c, TMP117_EEPROM_UL_REG, 0 | (1UL << 15));
}

/*
 * @brief  	check whether EEPROM is busy:
 *
 * @param 	i2c					I2C handler
 * @param		buffer			buffer for the I2C data exchange
*
* @return 1 if the EEPROM is busy, otherwise 0
*
* @note false might also been that an I2C error has occurred
*/
static uint8_t isEEPROMbusy(I2C_HandleTypeDef* i2c) {
	uint16_t code = 0;

	if (readI2CReg(i2c, TMP117_EEPROM_UL_REG, &code))
		return 0;

	return ((code >> 14) & 0x01);
}

// PUBLIC
uint8_t setConfig(I2C_HandleTypeDef* i2c, uint16_t config) {
	// set configuration register
	return writeI2CReg(i2c, TMP117_CONFIG_REG, config);
}

uint8_t getConfig(I2C_HandleTypeDef* i2c, uint16_t* config) {
	// get configuration register
	return readI2CReg(i2c, TMP117_CONFIG_REG, config);
}

uint8_t softwareReset(I2C_HandleTypeDef* i2c) {
	uint8_t ret_val = 0;
	uint16_t config_val = 0;

	// set reset bit to reset the sensor
	if ((ret_val = writeI2CReg(i2c, TMP117_CONFIG_REG, SW_RESET)))
		return ret_val;

	// wait 2 ms reset
	HAL_Delay(2);

	if ((ret_val = getConfig(i2c, &config_val)))
		return ret_val;

	return (config_val & SW_RESET) ? TMP117_ERR : TMP117_OK;
}

uint8_t getDeviceID(I2C_HandleTypeDef* i2c, uint16_t* id) {
	return readI2CReg(i2c, TMP117_DEVICE_ID_REG, id);
}

uint8_t getMeasuredTemp(I2C_HandleTypeDef* i2c, float* temp) {
	return getTemperature(i2c, TMP117_TEMP_RES_REG, temp);
}

uint8_t setHighLimitTemp(I2C_HandleTypeDef* i2c, float temp) {
	// sensor works in 2's complement
	int16_t temp_raw = (int16_t)(temp / TMP117_RESOLUTION);

	return writeI2CReg(i2c, TMP117_THIGH_LIM_REG, (uint16_t)temp_raw);
}

uint8_t setLowLimitTemp(I2C_HandleTypeDef* i2c, float temp) {
	// sensor works in 2's complement
	int16_t temp_raw = (int16_t)(temp / TMP117_RESOLUTION);

	// write to the offset temperature register
	return writeI2CReg(i2c, TMP117_TLOW_LIM_REG, (uint16_t)temp_raw);
}

uint8_t setOffsetTemp(I2C_HandleTypeDef* i2c, float target_temp) {
	float actual_temp = 0;
	uint8_t ret_val = 0;

	if ((ret_val = getMeasuredTemp(i2c, &actual_temp)))
		return ret_val;

	float delta_temp = target_temp - actual_temp;

	// convert to two's complement
	int16_t delta_int = (int16_t)(delta_temp / TMP117_RESOLUTION);

	// write to the offset temperature register
	return writeI2CReg(i2c, TMP117_TEMP_OFFSET_REG, (uint16_t)delta_int);
}

uint8_t getOffsetTemp(I2C_HandleTypeDef* i2c, float* offset) {
	return getTemperature(i2c, TMP117_TEMP_OFFSET_REG, offset);
}

uint8_t getHighLimitTemp(I2C_HandleTypeDef* i2c, float* high_lim) {
	return getTemperature(i2c, TMP117_THIGH_LIM_REG, high_lim);
}

uint8_t getLowLimitTemperature(I2C_HandleTypeDef* i2c, float* low_lim) {
	return getTemperature(i2c, TMP117_TLOW_LIM_REG, low_lim);
}

uint8_t readEEPROM (I2C_HandleTypeDef* i2c, uint8_t eeprom_idx, uint16_t* data) {
	uint8_t ret_val = TMP117_ERR;

	if (!(eeprom_idx == TMP117_EEPROM1_REG || eeprom_idx == TMP117_EEPROM2_REG
			|| eeprom_idx == TMP117_EEPROM3_REG))
		return ret_val;

	if (!isEEPROMbusy(i2c))
		ret_val = readI2CReg(i2c, eeprom_idx, data);

	return ret_val;
}

uint8_t writeEEPROM (I2C_HandleTypeDef* i2c, uint8_t eeprom_idx, uint16_t data) {
	uint8_t ret_val = TMP117_ERR;

	if (!(eeprom_idx == TMP117_EEPROM1_REG || eeprom_idx == TMP117_EEPROM2_REG
				|| eeprom_idx == TMP117_EEPROM3_REG))
			return ret_val;

	if (isEEPROMbusy(i2c))
		return ret_val;

	// unlock EEPROM
	ret_val = unlockEEPROM(i2c);
	// if EEPROM unlocked
	if (ret_val)
		return ret_val;

	// attempt to write a EEPROM
	ret_val = readI2CReg(i2c, eeprom_idx, &data);
	HAL_Delay(7);

	if (ret_val)
		return ret_val;

	// wait for EEPROM to become available
	uint32_t i = 0;
	while (isEEPROMbusy(i2c) && i < 10000);

	// lock EEPROM
	lockEEPROM(i2c);

	return TMP117_OK;
}
