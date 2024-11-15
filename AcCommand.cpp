#include "AcCommand.h"
#include <Arduino.h>

#define BIT_MARK 420      // Độ dài tín hiệu thấp
#define ONE_SPACE 1330    // Độ dài tín hiệu cao
#define ZERO_SPACE 470    // Sai lệch cho phép
#define HEADER_MARK 3588  // Giá trị đầu
#define HEADER_SPACE 1790 // Giá trị tiếp theo
#define FOOTER 490
#define MAX_CODE_LENGTH 307

const uint8_t kPanasonicTemp16 = 0b00000100;
const uint8_t kPanasonicTemp17 = 0b01000100;
const uint8_t kPanasonicTemp18 = 0b00100100;
const uint8_t kPanasonicTemp19 = 0b01100100;
const uint8_t kPanasonicTemp20 = 0b00010100;
const uint8_t kPanasonicTemp21 = 0b01010100;
const uint8_t kPanasonicTemp22 = 0b00110100;
const uint8_t kPanasonicTemp23 = 0b01110100;
const uint8_t kPanasonicTemp24 = 0b00001100;
const uint8_t kPanasonicTemp25 = 0b01001100;
const uint8_t kPanasonicTemp26 = 0b00101100;
const uint8_t kPanasonicTemp27 = 0b01101100;
const uint8_t kPanasonicTemp28 = 0b00011100;
const uint8_t kPanasonicTemp29 = 0b01011100;
const uint8_t kPanasonicTemp30 = 0b00111100;

const uint8_t kPanasonicAuto = 0b0000;
const uint8_t kPanasonicCool = 0b1100;
const uint8_t kPanasonicHeat = 0b0010;
const uint8_t kPanasonicDry = 0b0100;

const uint8_t kPanasonicFanAuto = 0b0101;
const uint8_t kPanasonicFanLow = 0b1100;
const uint8_t kPanasonicFanMed = 0b1010;
const uint8_t kPanasonicFanHigh = 0b1110;

const uint8_t kPanasonicSwingAuto = 0b1111;
const uint8_t kPanasonicSwingUMax = 0b1000;
const uint8_t kPanasonicSwingMax = 0b0100;
const uint8_t kPanasonicSwingHigh = 0b1100;
const uint8_t kPanasonicSwingMed = 0b0010;
const uint8_t kPanasonicSwingLow = 0b1010;


const uint8_t kPanasonicOn = 0b0001;
const uint8_t kPanasonicOff = 0b1001;

const uint8_t byte1 = 0b01000000;
const uint8_t byte2 = 0b00000100;
const uint8_t byte3 = 0b00000111;
const uint8_t byte4 = 0b00100000;
const uint8_t byte5 = 0b00000000;
const uint8_t byte8 = 0b00000001;
const uint8_t byte10 = 0b00000000;
const uint8_t byte11 = 0b00000000;
const uint8_t byte12 = 0b01110000;
const uint8_t byte13 = 0b00000111;
const uint8_t byte14 = 0b00000000;
const uint8_t byte15 = 0b00000000;
const uint8_t byte16 = 0b10000001;
const uint8_t byte17 = 0b00000000;
const uint8_t byte18 = 0b00000000;

uint8_t encodeTemp(uint8_t temp)
{
    switch (temp)
    {
    case 16:
        return kPanasonicTemp16;
    case 17:
        return kPanasonicTemp17;
    case 18:
        return kPanasonicTemp18;
    case 19:
        return kPanasonicTemp19;
    case 20:
        return kPanasonicTemp20;
    case 21:
        return kPanasonicTemp21;
    case 22:
        return kPanasonicTemp22;
    case 23:
        return kPanasonicTemp23;
    case 24:
        return kPanasonicTemp24;
    case 25:
        return kPanasonicTemp25;
    case 26:
        return kPanasonicTemp26;
    case 27:
        return kPanasonicTemp27;
    case 28:
        return kPanasonicTemp28;
    case 29:
        return kPanasonicTemp29;
    case 30:
        return kPanasonicTemp30;
    default:
        return kPanasonicTemp28;
    }
}

uint8_t encodeMode(uint8_t mode)
{
    switch (mode)
    {
    case 1:
        return kPanasonicCool;
    case 2:
        return kPanasonicHeat;
    case 3:
        return kPanasonicDry;
    default:
        return kPanasonicAuto;
    }
}

uint8_t encodeFanMode(uint8_t fan_mode)
{
    switch (fan_mode)
    {
    case 1:
        return kPanasonicFanLow;
    case 2:
        return kPanasonicFanMed;
    case 3:
        return kPanasonicFanHigh;
    default:
        return kPanasonicFanAuto;
    }
}

uint8_t encodeSwingMode(uint8_t swing_mode)
{
    switch (swing_mode)
    {
    case 1:
        return kPanasonicSwingLow;
    case 2:
        return kPanasonicSwingMed;
    case 3:
        return kPanasonicSwingHigh;
    case 4:
        return kPanasonicSwingMax;
    case 5:
        return kPanasonicSwingUMax;
    default:
        return kPanasonicSwingAuto;
    }
}

uint8_t encodePower(bool power)
{
    return power ? 0b1001 : 0b0001;
}

uint8_t reverseBit(uint8_t data)
{
    int result;
    for (int i = 0; i < 8; i++)
    {
        result <<= 1;              // 101
        result |= (data >> i) & 1; // 001001001 -> 001001001 & 00000001 -> 1
    }
    return result;
}
uint8_t sumAllByte(uint8_t *bitData, uint8_t length)
{
    uint8_t sum = 0;
    for (int i = 0; i < length; ++i)
    {
        sum += bitData[i];
        // std::cout << "Sum " << i <<": " << std::bitset<8>(sum) << std::endl;
    }
    return sum;
}
uint8_t calculateCheckSum(uint8_t *originalData)
{
    uint8_t reversedData[18];
    for (int i = 0; i < sizeof(reversedData); ++i)
    {
        reversedData[i] = reverseBit(originalData[i]);
        // std::cout << std::bitset<16>(bitData2[i]) << std::endl;
    }

    uint8_t checkSum = sumAllByte(reversedData, sizeof(reversedData));
    checkSum = reverseBit(checkSum);
    return checkSum;
}
// encodePanasonicIR(data, 30, 1, 2, 1, 2, 0);  0111                             001         1             100                 1100
void encodePanasonicIR(uint8 *result, uint8_t temp, uint8_t mode, bool power, uint8_t fan_mode, uint8_t swing_mode)
{
    uint8_t eTemp = encodeTemp(temp);
    uint8_t eMode = encodeMode(mode);
    uint8_t ePower = encodePower(power);
    uint8_t eFan_mode = encodeFanMode(fan_mode);
    uint8_t eSwing_mode = encodeSwingMode(swing_mode);

    result[0] = (uint16_t)byte1;
    result[1] = (uint16_t)byte2;
    //Serial.print("result[0]: ");
    //Serial.println(String(result[0], BIN));
    //Serial.print("result[1]: ");
    //Serial.println(String(result[1], BIN));

    result[2] = (uint16_t)byte3;
    result[3] = (uint16_t)byte4;
    //Serial.print("result[2]: ");
    //Serial.println(String(result[2], BIN));
    //Serial.print("result[3]: ");
    //Serial.println(String(result[3], BIN));

    result[4] = ((uint16_t)byte5);
    result[5] = (uint16_t)ePower << 4 | (uint16_t)eMode;
    //Serial.print("result[4]: ");
    //Serial.println(String(result[4], BIN));
    //Serial.print("result[5]: ");
    //Serial.println(String(result[5], BIN));

    result[6] = ((uint16_t)eTemp);
    result[7] = (uint16_t)byte8;
    //Serial.print("result[6]: ");
    //Serial.println(String(result[6], BIN));
    //Serial.print("result[7]: ");
    //Serial.println(String(result[7], BIN));

    result[8] = ((uint16_t)eSwing_mode << 4) | (uint16_t)eFan_mode;
    result[9] = (uint16_t)byte10;
    //Serial.print("result[8]: ");
    //Serial.println(String(result[8], BIN));
    //Serial.print("result[9]: ");
    //Serial.println(String(result[9], BIN));

    result[10] = (uint16_t)byte11;
    result[11] = (uint16_t)byte12;
    //Serial.print("result[10]: ");
    //Serial.println(String(result[10], BIN));
    //Serial.print("result[11]: ");
    //Serial.println(String(result[11], BIN));

    result[12] = (uint16_t)byte13;
    result[13] = (uint16_t)byte14;
    //Serial.print("result[12]: ");
    //Serial.println(String(result[12], BIN));
    //Serial.print("result[13]: ");
    //Serial.println(String(result[13], BIN));

    result[14] = (uint16_t)byte15;
    result[15] = (uint16_t)byte16;
    //Serial.print("result[14]: ");
    //Serial.println(String(result[14], BIN));
    //Serial.print("result[15]: ");
    //Serial.println(String(result[15], BIN));

    result[16] = (uint16_t)byte17;
    result[17] = (uint16_t)byte18;
    //Serial.print("result[16]: ");
    //Serial.println(String(result[16], BIN));
    //Serial.print("result[17]: ");
    //Serial.println(String(result[17], BIN));

    uint8_t checkSum = calculateCheckSum(result);
    //Serial.print("CheckSum: ");
    //Serial.println(String(checkSum, BIN));

    result[18] = (uint8_t)checkSum;
    //Serial.print("result[18]: ");
    //Serial.println(String(result[18], BIN));

    // for (int i = 0; i < 19; ++i)
    // {
    //     //Serial.print(String(result[i], BIN));
    //     //Serial.println();
    // }
}

void binaryToRawSignal(uint8_t *binaryData, uint16_t *rawSignal)
{
    int maxCodeLength = MAX_CODE_LENGTH;
    rawSignal[0] = HEADER_MARK;
    rawSignal[1] = HEADER_SPACE;
    rawSignal[306] = FOOTER;
    int index = 0;
    const int numBytes = 19;   // Độ dài của chuỗi binaryData
    const int bitsPerByte = 8; // Số bit trong mỗi byte

    for (int i = 0; i < numBytes; i++)
    {
        for (int j = bitsPerByte - 1; j >= 0; --j)
        {
            if (((binaryData[i] >> j) & 1) == 1)
            {
                rawSignal[2 + index * 2] = BIT_MARK;
                rawSignal[2 + index * 2 + 1] = ONE_SPACE;
            }
            else
            {
                rawSignal[2 + index * 2] = BIT_MARK;
                rawSignal[2 + index * 2 + 1] = ZERO_SPACE;
            }
            index++;
        }
    }

    //Serial.println();
    for (int i = 0; i < maxCodeLength; i++)
    {
        //Serial.print(rawSignal[i]);
        if (i < maxCodeLength - 1)
        {
            //Serial.print(", ");
        }
    }
}

/**
 * Extracts the Temperature bits from the received IR data and converts them to human-readable format.
 * @param resultData: The string representing the raw bit data received from the IR signal.
 * @return uint8_t: The parsed temperature value in binary.
 */
uint8_t getTemp(String resultData)
{
    String result = resultData.substring(48, 56);
    return (uint8_t)strtol(result.c_str(), NULL, 2);
}
// Hàm getMode trích xuất đoạn bit dài của IRremote và chuyển đổi thông tin từ chuỗi bit liên quan đến Mode
uint8_t getMode(String resultData)
{
    String result = resultData.substring(44, 48);
    return (uint8_t)strtol(result.c_str(), NULL, 2);
}

// Hàm getPower trích xuất đoạn bit dài của IRremote và chuyển đổi thông tin từ chuỗi bit liên quan đến bật/tắt
uint8_t getPower(String resultData)
{
    String result = resultData.substring(40, 44);
    return (uint8_t)strtol(result.c_str(), NULL, 2);
}
// Hàm getSwingMode trích xuất đoạn bit dài của IRremote và chuyển đổi thông tin từ chuỗi bit liên quan đến độ mở cách gió
uint8_t getSwingMode(String resultData)
{
    String result = resultData.substring(64, 68);
    return (uint8_t)strtol(result.c_str(), NULL, 2);
}

// Hàm getFanMode trích xuất đoạn bit dài của IRremote và chuyển đổi thông tin từ chuỗi bit liên quan đến độ mạnh của quạt
uint8_t getFanMode(String resultData)
{
    String result = resultData.substring(68, 72);
    return (uint8_t)strtol(result.c_str(), NULL, 2);
}

int abs(int val)
{
    return (val < 0) ? -val : val;
}

char decode(int val1, int val2)
{
    if (val1 != HEADER_MARK && val2 != HEADER_SPACE)
    {
        if (abs(val1 - BIT_MARK) < ZERO_SPACE && abs(val2 - BIT_MARK) < ZERO_SPACE)
        {
            return '0';
        }
        if (abs(val1 - BIT_MARK) < ZERO_SPACE && abs(val2 - ONE_SPACE) < ZERO_SPACE)
        {
            return '1';
        }
    }
    return '\0';
}

String turnRawSignalToBinary(String rawData)
{
    char rawDataArray[rawData.length()]; // Thêm ký tự null cuối
    const char *delimiters = ", ";
    int firstValue, secondValue;

    strcpy(rawDataArray, rawData.c_str());
    char *token = strtok(rawDataArray, delimiters);
    String binaryResult = ""; // Sử dụng `String` thay cho mảng ký tự
    int tokenIndex = 0;

    while (token != NULL)
    {
        if (tokenIndex & 1)
        {
            secondValue = atoi(token);
            char decodedChar = decode(firstValue, secondValue);
            if (decodedChar != '\0')
            {
                binaryResult += decodedChar; // Nối thẳng vào `String`
            }
        }
        else
        {
            firstValue = atoi(token);
        }
        token = strtok(NULL, delimiters);
        tokenIndex++;
    }
    //Serial.println();
    //Serial.print(binaryResult);
    return binaryResult; // Trả về kết quả chuỗi nhị phân
}

/**
 * Publishes the parsed Auto Swing and Temperature data to Home Assistant via MQTT.
 */
const char *returnTempForMQTT(String resultData)
{
    // //Serial.print("Temperature: "); //Serial.println(getTemp(resultData), BIN);
    switch (getTemp(resultData))
    {
    case kPanasonicTemp16:
        return "16";
    case kPanasonicTemp17:
        return "17";
    case kPanasonicTemp18:
        return "18";
    case kPanasonicTemp19:
        return "19";
    case kPanasonicTemp20:
        return "20";
    case kPanasonicTemp21:
        return "21";
    case kPanasonicTemp22:
        return "22";
    case kPanasonicTemp23:
        return "23";
    case kPanasonicTemp24:
        return "24";
    case kPanasonicTemp25:
        return "25";
    case kPanasonicTemp26:
        return "26";
    case kPanasonicTemp27:
        return "27";
    case kPanasonicTemp28:
        return "28";
    case kPanasonicTemp29:
        return "29";
    case kPanasonicTemp30:
        return "30";
    default:
        return "";
    }
}

const char *returnModeForMQTT(String resultData)
{
    // //Serial.print("Mode: "); //Serial.println(getMode(resultData), BIN);
    switch (getMode(resultData))
    {
    case kPanasonicAuto:
        return "auto";
    case kPanasonicCool:
        return "cool";
    case kPanasonicHeat:
        return "heat";
    case kPanasonicDry:
        return "dry";
    default:
        return "";
    }
}
const char *returnSwingModeForMQTT(String resultData)
{
    //Serial.print("Swing: "); //Serial.println(getSwingMode(resultData), BIN);
    switch (getSwingMode(resultData))
    {
    case kPanasonicSwingHigh:
        return "High";
    case kPanasonicSwingMed:
        return "Medium";
    case kPanasonicSwingMax:
        return "Max";
    case kPanasonicSwingLow:
        return "Low";
    case kPanasonicSwingAuto:
        return "Auto";
    case kPanasonicSwingUMax:
        return "UMax";
    default:
        return "";
    }
}
const char *returnPowerForMQTT(String resultData)
{
    // //Serial.print("Power: "); //Serial.println(getPower(resultData), BIN);
    switch (getPower(resultData))
    {
    case kPanasonicOff:
        return "off";
    default:
        return "";
    }
}

const char *returnFanModeForMQTT(String resultData)
{
    // //Serial.print("Fan: "); //Serial.println(getFanMode(resultData), BIN);
    switch (getFanMode(resultData))
    {
    case kPanasonicFanAuto:
        return "auto";
    case kPanasonicFanHigh:
        return "high";
    case kPanasonicFanMed:
        return "medium";
    case kPanasonicFanLow:
        return "low";
    default:
        return "";
    }
}
