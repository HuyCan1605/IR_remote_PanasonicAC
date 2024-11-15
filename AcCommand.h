#ifndef ACCOMMAND_H
#define ACCOMMAND_H
#include <Arduino.h>

#define MAX_CODE_LENGTH 307
#define SHORT_CODE_LENGTH 131
#define MAX_CODE_BIT_LENGTH 152

const char* returnTempForMQTT(String resultData);
const char* returnModeForMQTT(String resultData);
const char* returnSwingModeForMQTT(String resultData);
const char* returnPowerForMQTT(String resultData);
const char* returnFanModeForMQTT(String resultData);
String turnRawSignalToBinary(String rawData);
void encodePanasonicIR(uint8 *result, uint8_t temp, uint8_t mode, bool power, uint8_t fan_mode, uint8_t swing_mode);
void binaryToRawSignal(uint8_t *binaryData, uint16_t *rawSignal);

#endif // ACCOMMAND_H