/*

  Copyright (C) 2014  Lee Wiggins <lee@wigweb.com.au>
                      F1RMB, Daniel Caujolle-Bert <f1rmb.daniel@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
/**
******************************************************************************************************
*****************                         BIG FAT WARNING                         ********************
*****************                 Should be compiled with -Os flag                ********************
****************                     (use the included Makefile)                  ********************
******************************************************************************************************
******************************************************************************************************
**/
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <EEPROM.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

#include "aDCLoad.h"


static const uint8_t _glyphs[8][8] PROGMEM =
{
    { // . ..1
        B00000,
        B00000,
        B00001,
        B00001,
        B00001,
        B10101,
        B00000,
        B00000,
    },
    { // . .1.
        B00000,
        B00000,
        B00100,
        B00100,
        B00100,
        B10101,
        B00000,
        B00000,
    },
    { // . 1..
        B00000,
        B00000,
        B10000,
        B10000,
        B10000,
        B10101,
        B00000,
        B00000,
    },
    { // . 1k
        B00000,
        B00000,
        B10000,
        B10000,
        B10000,
        B10101,
        B00110,
        B00101,
    },
    { // USB
        B00100,
        B01110,
        B00101,
        B10101,
        B10110,
        B01100,
        B00100,
        B01110,
    },
    { // LOCK
        B00110,
        B00100,
        B00110,
        B00100,
        B00100,
        B01110,
        B10001,
        B01110,
    },
    { // CHECKBOX UNTICKED
        B10101,
        B00000,
        B10001,
        B00000,
        B10001,
        B00000,
        B10101,
        B00000,
    },
    { // CHECKBOX TICKED
        B11110,
        B10001,
        B00011,
        B10110,
        B11101,
        B01001,
        B10011,
        B00000
    }
};

/**
*** Implement our serial print function to save ~300ko
**/
// Prototype
#if 1
void serialPrint(unsigned long, int = DEC);

void serialWrite(char c)
{
    Serial.print(c);
}
void serialPrint(const __FlashStringHelper *ifsh)
{
    const char *__attribute__((progmem)) p = (const char * ) ifsh;
    while (1)
    {
        unsigned char c = pgm_read_byte(p++);
        if (c == 0)
            break;
        serialWrite(c);
    }
}
void serialPrint(char c)
{
    serialWrite(c);
}
void serialPrint(int n, int16_t base = DEC)
{
   serialPrint((unsigned long) n, base);
}
void serialPrintln(void)
{
    serialPrint('\r');
    serialPrint('\n');
}
void serialPrint(unsigned long n, int base)
{
    Serial.print(n, base);
}
void serialPrint(double n, int digits)
{
    Serial.print(n, digits);
}
void serialPrintln(double n, int digits)
{
    Serial.print(n, digits);
    serialPrintln();
}
void serialPrintln(const __FlashStringHelper *ifsh)
{
    serialPrint(ifsh);
    serialPrintln();
}
void serialPrintln(char c)
{
    serialWrite(c);
    serialPrintln();
}
#endif
#if 0
/**
*** Numerical utils
**/
int8_t getNumericalLength(int16_t n)
{
    char buf[16];

    return (static_cast<int8_t>(snprintf(buf, sizeof(buf) - 1, "%d", n)));
}
int8_t getNumericalLength(uint16_t n)
{
    char buf[16];

    return (static_cast<int8_t>(snprintf(buf, sizeof(buf) - 1, "%u", n)));
}
#endif
/**
*** Our float to string format function
**/
int8_t getNumericalLength(float n, uint8_t dLen = 3)
{
    char buf[32];

    if (dLen > 11)
	    dLen = 11;

    char *p = dtostrf(n, 1, dLen, buf);

    return strlen(p);
}

/**
*** Our float rounding function
**/
float floatRounding(float f, float p = 0.001f)
{
	return (floor(f * (1.0f / p) + 0.5) / (1.0f / p));
}

/**
*** Macro used in aDCSettings class setters (some of them)
**/
#define RETURN_IF_INVALID(type, value) \
    do { \
        if (value < 0) \
            return SETTING_UNDERSIZED;\
        else if (value > type ## _MAXIMUM)\
            return SETTING_OVERSIZED;\
    } while(0)


aDCSettings::aDCSettings() :
#ifdef SIMU
                    m_readVoltage(24.000),
#else
                    m_readVoltage(0),
#endif
                    m_dispVoltage(-1), m_setsCurrent(0), m_readCurrent(0), m_dispCurrent(-1),
#ifdef RESISTANCE
                    m_setsResistance(0), m_readResistance(0), m_dispResistance(-1),
#endif
                    m_setsPower(0), m_readPower(0), m_dispPower(-1),
#ifdef SIMU
                    m_readTemperature(20),
#else
                    m_readTemperature(0),
#endif
                    m_dispTemperature(1),
                    m_fanSpeed(1),
                    m_operationMode(OPERATION_READ), m_prevOperationMode(OPERATION_UNKNOWN),
                    m_Mode(SELECTION_CURRENT), m_prevMode(SELECTION_UNKNOWN),
                    m_EncoderPos(0), m_prevEncoderPos(-1),
                    m_dispMode(DISPLAY_VALUES), m_prevDispMode(DISPLAY_UNKNOWN),
                    m_lockTick(0), m_operationTick(0),
                    m_features(0x0)
{
    _eepromRestore();
}

aDCSettings::~aDCSettings()
{
}

// Voltage
aDCSettings::SettingError aDCSettings::setVoltageRead(float v)
{
    RETURN_IF_INVALID(VOLTAGE, v);
    m_readVoltage =  v;
    return SETTING_VALID;
}
float aDCSettings::getVoltageRead()
{
    return m_readVoltage;
}
void aDCSettings::syncVoltageDisp()
{
    m_dispVoltage = m_readVoltage;
}
bool aDCSettings::isVoltageAlreadyDisplayed()
{
    return (_isEqual(m_readVoltage, m_dispVoltage));
}

// Current
aDCSettings::SettingError aDCSettings::setCurrentSets(float v)
{
    RETURN_IF_INVALID(CURRENT, v);
    m_setsCurrent = v;
    return SETTING_VALID;
}
float aDCSettings::getCurrentSets()
{
    return m_setsCurrent;
}
aDCSettings::SettingError aDCSettings::setCurrentRead(float v)
{
    RETURN_IF_INVALID(CURRENT, v);
    m_readCurrent = v;
    return SETTING_VALID;
}
float aDCSettings::getCurrentRead()
{
    return m_readCurrent;
}
void aDCSettings::syncCurrentDisp()
{
    m_dispCurrent = (m_operationMode == OPERATION_READ) ? m_readCurrent : m_setsCurrent;
}
bool aDCSettings::isCurrentAlreadyDisplayed()
{
    return (_isEqual((m_operationMode == OPERATION_READ) ? m_readCurrent : m_setsCurrent, m_dispCurrent));
}

#ifdef RESISTANCE
// Resistance
aDCSettings::SettingError aDCSettings::setResistanceSets(float v)
{
    RETURN_IF_INVALID(RESISTANCE, v);
    m_setsResistance = v;
    return SETTING_VALID;
}
float aDCSettings::getResistanceSets()
{
    return m_setsResistance;
}
void aDCSettings::setResistanceRead(float v)
{
    m_readResistance = v;
}
float aDCSettings::getResistanceRead()
{
    return m_readResistance;
}
void aDCSettings::syncResistanceDisp()
{
    m_dispResistance = (m_operationMode == OPERATION_READ) ? m_readResistance : m_setsResistance;
}
bool aDCSettings::isResistanceAlreadyDisplayed()
{
    return (_isEqual((m_operationMode == OPERATION_READ) ? m_readResistance :  m_setsResistance, m_dispResistance));
}
#endif
// Power
aDCSettings::SettingError aDCSettings::setPowerSets(float v)
{
    RETURN_IF_INVALID(POWER, v);
    m_setsPower = v;
    return SETTING_VALID;
}
float aDCSettings::getPowerSets()
{
    return m_setsPower;
}
aDCSettings::SettingError aDCSettings::setPowerRead(float v)
{
    RETURN_IF_INVALID(POWER, v);
    m_readPower = v;
    return SETTING_VALID;
}
float aDCSettings::getPowerRead()
{
    return m_readPower;
}
void aDCSettings::syncPowerDisp()
{
    m_dispPower = (m_operationMode == OPERATION_READ) ? m_readPower : m_setsPower;
}
bool aDCSettings::isPowerAlreadyDisplayed()
{
    return (_isEqual((m_operationMode == OPERATION_READ) ? m_readPower : m_setsPower, m_dispPower));
}

void aDCSettings::updateValuesFromMode(float v, SelectionMode mode)
{
    switch (mode)
    {
        case SELECTION_CURRENT:
            switch (setCurrentSets(v))
            {
                case aDCSettings::SETTING_UNDERSIZED:
                    setCurrentSets(0.0);
                    setEncoderPosition(0);
                    break;

                case aDCSettings::SETTING_OVERSIZED:
                    setCurrentSets(CURRENT_MAXIMUM);
                    setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(CURRENT_MAXIMUM) * floatRounding(500.000))));
                    break;

                default:
                    break;
            }

            switch (setPowerSets(floatRounding(floatRounding(getVoltageRead()) * floatRounding(getCurrentSets()))))
            {
                case aDCSettings::SETTING_OVERSIZED:
                    setCurrentSets(floatRounding(floatRounding(POWER_MAXIMUM) / floatRounding(getVoltageRead())));
#ifdef RESISTANCE
                    setResistanceSets((getCurrentSets() > 0.0) ? floatRounding(floatRounding(POWER_MAXIMUM) / floatRounding(floatRounding(getCurrentSets()) * floatRounding(getCurrentSets()))) : -1);
#endif
                    setPowerSets(floatRounding(floatRounding(getVoltageRead()) * floatRounding(getCurrentSets())));
                    setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(getCurrentSets()) * floatRounding(500.000))));
                    break;

                case aDCSettings::SETTING_UNDERSIZED:
                    break;

                case aDCSettings::SETTING_VALID:
#ifdef RESISTANCE
                    setResistanceSets((getCurrentSets() > 0.0) ? floatRounding(floatRounding(getVoltageRead()) / floatRounding(getCurrentSets())) : 0);
#endif
                    break;
            }
            break;

#ifdef RESISTANCE
        case SELECTION_RESISTANCE:
            switch (setResistanceSets(v))
            {
                case aDCSettings::SETTING_UNDERSIZED:
                    setResistanceSets(0.0);
                    setEncoderPosition(0);
                    break;

                default:
                    break;
            }

            switch (setCurrentSets((getResistanceSets() > 0.0) ? floatRounding(floatRounding(getVoltageRead()) / floatRounding(getResistanceSets())) : 0))
            {
                case aDCSettings::SETTING_OVERSIZED:
                    setCurrentSets(floatRounding(CURRENT_MAXIMUM));
                    setResistanceSets(floatRounding(floatRounding(getVoltageRead()) / floatRounding(CURRENT_MAXIMUM)));
                    setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(getResistanceSets()) * floatRounding(1000.000))));
                    break;

                case aDCSettings::SETTING_UNDERSIZED:
                    break;

                default:
                    break;
            }

            switch (setPowerSets(floatRounding(floatRounding(getVoltageRead()) * floatRounding(getCurrentSets()))))
            {
                case aDCSettings::SETTING_OVERSIZED:
                    setCurrentSets(floatRounding(floatRounding(POWER_MAXIMUM) / floatRounding(getVoltageRead())));
                    setResistanceSets((getCurrentSets() > 0.0) ? floatRounding(floatRounding(POWER_MAXIMUM) / floatRounding(floatRounding(getCurrentSets()) * floatRounding(getCurrentSets()))) : 0);
                    setPowerSets(floatRounding(floatRounding(getVoltageRead()) * floatRounding(getCurrentSets())));
                    setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(getResistanceSets()) * floatRounding(1000.000))));
                    break;

                case aDCSettings::SETTING_UNDERSIZED:
                    break;

                default:
                    break;
            }
            break;
#endif

        case SELECTION_POWER:
            switch (setPowerSets(v))
            {
                case aDCSettings::SETTING_OVERSIZED:
                    setPowerSets(floatRounding(POWER_MAXIMUM));
                    setCurrentSets(floatRounding(floatRounding(getPowerSets()) / floatRounding(getVoltageRead())));
#ifdef RESISTANCE
                    setResistanceSets((getCurrentSets() > 0.0) ? floatRounding(floatRounding(POWER_MAXIMUM) / floatRounding(floatRounding(getCurrentSets()) * floatRounding(getCurrentSets()))) : 0);
#endif
                    setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(getPowerSets()) * floatRounding(1000.000))));
                    break;

                case aDCSettings::SETTING_UNDERSIZED:
                    setPowerSets(0.0);
                    setEncoderPosition(0);

                default:
                    setCurrentSets(floatRounding(floatRounding(getPowerSets()) / floatRounding(getVoltageRead())));
#ifdef RESISTANCE
                    setResistanceSets((getCurrentSets() > 0.0) ? floatRounding(floatRounding(getVoltageRead()) / floatRounding(getCurrentSets())) : 0);
#endif
                    break;
            }
            break;

        default:
            break;
    }
}

// Temperature
void aDCSettings::setTemperatureRead(uint8_t v)
{
    m_readTemperature = v;
}
uint8_t aDCSettings::getTemperatureRead()
{
    return m_readTemperature;
}
void aDCSettings::syncTemperatureDisp()
{
    m_dispTemperature = m_readTemperature;
}
bool aDCSettings::isTemperatureAlreadyDisplayed()
{
    return (_isEqual(static_cast<float>(m_readTemperature), static_cast<float>(m_dispTemperature)));
}

// Fan
void aDCSettings::setFanSpeed(uint16_t v)
{
    m_fanSpeed = v;
}
uint16_t aDCSettings::getFanSpeed()
{
    return m_fanSpeed;
}

// Selection Mode
void aDCSettings::setSelectionMode(SelectionMode m)
{
    m_prevMode = m_Mode;
    m_Mode = m;
}
SelectionMode aDCSettings::getSelectionMode()
{
    return m_Mode;
}
SelectionMode aDCSettings::getNextMode(SelectionMode origin)
{
    if (m_dispMode == DISPLAY_SETUP)
        return (static_cast<SelectionMode>(((origin != SELECTION_UNKNOWN) ? !origin : !m_Mode)));

    uint8_t m = static_cast<uint8_t>(origin != SELECTION_UNKNOWN ? origin : m_Mode);

    if ((static_cast<uint8_t>(origin != SELECTION_UNKNOWN ? origin : m_Mode) + 1) < static_cast<uint8_t>(SELECTION_UNKNOWN))
        return static_cast<SelectionMode>(m + 1);

    return SELECTION_CURRENT;
}
SelectionMode aDCSettings::getPrevMode(SelectionMode origin)
{
    if (m_dispMode == DISPLAY_SETUP)
        return (static_cast<SelectionMode>(((origin != SELECTION_UNKNOWN) ? !origin : !m_Mode)));

    uint8_t m = static_cast<uint8_t>(origin != SELECTION_UNKNOWN ? origin : m_Mode);

    if ((static_cast<uint8_t>(origin != SELECTION_UNKNOWN ? origin : m_Mode) - 1) >= static_cast<uint8_t>(SELECTION_CURRENT))
        return static_cast<SelectionMode>(m - 1);

    return SELECTION_POWER;
}

void aDCSettings::syncSelectionMode()
{
    m_prevMode = m_Mode;
}
bool aDCSettings::isSelectionModeChanged()
{
    return (!_isEqual(static_cast<float>(m_Mode), static_cast<float>(m_prevMode)));
}

// Display Mode
void aDCSettings::setDisplayMode(DisplayMode d)
{
    m_prevDispMode = m_dispMode;
    m_dispMode = d;
}
DisplayMode aDCSettings::getDisplayMode()
{
    return m_dispMode;
}
void aDCSettings::syncDisplayMode()
{
    m_prevDispMode = m_dispMode;
}
bool aDCSettings::isDisplayModeChanged()
{
    return (!_isEqual(static_cast<float>(m_dispMode), static_cast<float>(m_prevDispMode)));
}

// Encoder
void aDCSettings::setEncoderPosition(int32_t p)
{
    m_prevEncoderPos = m_EncoderPos;
    m_EncoderPos = p;

    pingAutolock();
}
void aDCSettings::incEncoderPosition(int32_t p)
{
    m_prevEncoderPos = m_EncoderPos;
    m_EncoderPos += p;

    pingAutolock();
}
int32_t aDCSettings::getEncoderPosition()
{
    return m_EncoderPos;
}
void aDCSettings::syncEncoderPosition()
{
    m_prevEncoderPos = m_EncoderPos;
}
bool aDCSettings::isEncoderPositionChanged()
{
    return (!_isEqual(m_EncoderPos, m_prevEncoderPos));
}

// Operation Mode
void aDCSettings::setOperationMode(OperationMode m)
{
    m_prevOperationMode = m_operationMode;
    m_operationMode = m;

    if (m_operationMode == OPERATION_SET)
        m_operationTick = millis();
    else
        m_operationTick = 0;
}
OperationMode aDCSettings::getOperationMode()
{
    return m_operationMode;
}
void aDCSettings::updateOperationMode()
{
    if (m_operationMode == OPERATION_SET)
    {
        if ((millis() - m_operationTick) > OPERATION_SET_TIMEOUT)
        {
            m_operationMode = OPERATION_READ;
            m_operationTick = 0;
        }
    }
}
void aDCSettings::pingOperationMode()
{
    if (m_operationMode == OPERATION_SET)
        m_operationTick = millis();
}
bool aDCSettings::isOperationModeChanged()
{
    return (m_operationMode != m_prevOperationMode);
}
void aDCSettings::syncOperationMode()
{
    m_prevOperationMode = m_operationMode;
}

// Autolock
void aDCSettings::pingAutolock()
{
    m_lockTick = millis();
}
bool aDCSettings::isAutolocked()
{
    if (!(m_features & FEATURE_AUTOLOCK))
        return false;

    if ((m_features & FEATURE_LOCKED) == false)
    {
        if ((millis() - m_lockTick) > AUTOLOCK_TIMEOUT)
            m_features |= FEATURE_LOCKED;
    }

    return (m_features & FEATURE_LOCKED);
}

// Features bitfield
void aDCSettings::enableFeature(uint16_t feature, bool enable)
{
    if (enable)
        m_features |= feature;
    else
        m_features &= (0xFFFF ^ feature);

    // Save to eeprom
    if ((feature == FEATURE_AUTODIM) || (feature == FEATURE_AUTOLOCK))
        EEPROM.write(((feature == FEATURE_AUTODIM) ? EEPROM_ADDR_AUTODIM : EEPROM_ADDR_AUTOLOCK), (m_features & feature) ? 1 : 0);
}
bool aDCSettings::isFeatureEnabled(uint16_t feature)
{
    return (m_features & feature);
}

bool aDCSettings::_isEqual(float v1, float v2)
{
    return (v1 == v2);
}

bool aDCSettings::_isEqual(int32_t v1, int32_t v2)
{
    return (v1 == v2);
}
/**
*** EEPROM functions
**/
//
// Check magic numbers in EEPROM
//
bool aDCSettings::_eepromCheckMagic()
{
    return ((EEPROM.read(EEPROM_ADDR_MAGIC) == 0xD) && (EEPROM.read(EEPROM_ADDR_MAGIC + 1) == 0xE) &&
            (EEPROM.read(EEPROM_ADDR_MAGIC + 2) == 0xA) && (EEPROM.read(EEPROM_ADDR_MAGIC + 3) == 0xD));
}
//
// Write magic numbers in EEPROM
//
void aDCSettings::_eepromWriteMagic()
{
    // Magic numbers
    EEPROM.write(EEPROM_ADDR_MAGIC,     0xD);
    EEPROM.write(EEPROM_ADDR_MAGIC + 1, 0xE);
    EEPROM.write(EEPROM_ADDR_MAGIC + 2, 0xA);
    EEPROM.write(EEPROM_ADDR_MAGIC + 3, 0xD);
}
//
// Reset config into EEPROM
//
void aDCSettings::_eepromReset()
{
    _eepromWriteMagic();

    EEPROM.write(EEPROM_ADDR_AUTODIM, 1);
    EEPROM.write(EEPROM_ADDR_AUTOLOCK, 1);
}
//
// Restore config from EEPROM
//
void aDCSettings::_eepromRestore()
{
    if (!_eepromCheckMagic())
        _eepromReset();

    enableFeature(FEATURE_AUTODIM, (EEPROM.read(EEPROM_ADDR_AUTODIM) == 1));
    enableFeature(FEATURE_AUTOLOCK, (EEPROM.read(EEPROM_ADDR_AUTOLOCK) == 1));
}

aStepper::aStepper() : m_inc(0), m_incPrev(255)
{
}
aStepper::~aStepper()
{
}

void aStepper::incIncrement()
{
    if((m_inc + 1) <= MAX_VALUE)
        m_inc++;
    else
        m_inc = 0;
}

uint8_t aStepper::incGetValue()
{
    return m_inc;
}

void aStepper::incReset()
{
    m_inc = 0;
}

int16_t aStepper::incGetMult()
{
    return (_pow(10, m_inc));
}

int16_t aStepper::incGetValueFromMode(SelectionMode mode)
{
    switch (mode)
    {
        case SELECTION_CURRENT:
            {
                switch (incGetMult())
                {
                    case 10:
                        return 5;
                        break;
                    case 100:
                        return 50;
                        break;
                    case 1000:
                        return 500;
                }
                return 1;
            }
            break;

#ifdef RESISTANCE
        case SELECTION_RESISTANCE:
#endif
        case SELECTION_POWER:
            return incGetMult();
            break;

        default:
            break;
    }

    return m_inc;
}

bool aStepper::incIsSynced()
{
    return (m_inc == m_incPrev);
}

void aStepper::incSync()
{
    m_incPrev = m_inc;
}

int16_t aStepper::_pow(int base, int exp)
{
    if(exp < 0)
        return -1;

    int16_t result = 1;

    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }

    return result;
}

/**
*** LiquidDisplay extension
**/
aLCD::aLCD(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t cols, uint8_t rows) :
        LiquidCrystal(rs, enable, d0, d1, d2, d3),
        m_cols(0), m_rows(0), m_curCol(0), m_curRow(0)
{
    begin(cols, rows);
}

aLCD::~aLCD()
{
}

void aLCD::begin(uint8_t cols, uint8_t lines)
{
    m_cols = cols;
    m_rows = lines;
    LiquidCrystal::begin(m_cols, m_rows);
}

void aLCD::setCursor(uint8_t col, uint8_t row)
{
    m_curCol = col;
    m_curRow = row;
    LiquidCrystal::setCursor(col, row);
}

void aLCD::printCenter(const char *str)
{
    if (str)
    {
        uint8_t len = strlen(str);

        // Scrolling is disabled due to memory footprint
        if (len <= m_cols)
        {
            uint8_t x = (m_cols - len) >> 1;

            setCursor(x, m_curRow);
        }
        else
            setCursor(0, m_curRow);

        LiquidCrystal::print(str);

#if 0
        else
        {
            char buf[m_cols + 1];
            char *p = (char *)str + m_cols;
            char *pp = p;

            strcat(buf, str);

            setCursor(0, m_curRow);
            LiquidCrystal::print(buf);

            delay(SCROLL_DELAY);

            setCursor(m_cols, m_curRow);
            LiquidCrystal::autoscroll();

            while (*p != '\0')
            {
                LiquidCrystal::print(*p);
                delay(SCROLL_DELAY);
                p++;
            }

            LiquidCrystal::rightToLeft();

            do
            {
                LiquidCrystal::print(*p);
                delay(SCROLL_DELAY);
                p--;
            } while (p != pp);

            LiquidCrystal::leftToRight();
            LiquidCrystal::noAutoscroll();
        }
#endif
    }
}

void aLCD::printCenter(const __FlashStringHelper *ifsh)
{
    const char * __attribute__((progmem)) p = (const char *)ifsh;
    size_t                                n = 0;

    while (1)
    {
        unsigned char c = pgm_read_byte(p++);
        if (c == 0)
            break;
        n++;
    }

    if (!n)
        return;

    char     buf[n + 1];
    char    *pp = buf;

    p = (const char *) ifsh;

    while (1)
    {
        unsigned char c = pgm_read_byte(p++);
        *pp = c;
        pp++;

        if (c == 0)
            break;
    }

    printCenter(buf);
}

void aLCD::clearLine(uint8_t row)
{
    setCursor(0, row);
    for (uint8_t i = 0; i < 40; i++)
        LiquidCrystal::write(char(0x20));
}

void aLCD::clearValue(uint8_t row, int destMinus)
{
    setCursor(OFFSET_VALUE, row);
    for (uint8_t i = OFFSET_VALUE; i < OFFSET_MARKER_RIGHT + destMinus; i++)
        LiquidCrystal::write(char(0x20));
}

/**
*** Class to manage display output
**/
aDCDisplay::aDCDisplay(aDCEngine *parent, uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t cols, uint8_t rows) :
        aLCD(rs, enable, d0, d1, d2, d3, cols, rows),
        m_Parent(parent), m_dimmed(false), m_dimmerTick(0)
{
}

aDCDisplay::~aDCDisplay()
{
}

void aDCDisplay::setup()
{
    pinMode(LED_BACKLIGHT_PIN, OUTPUT);

    // set the LCD Backlight high
    digitalWrite(LED_BACKLIGHT_PIN, HIGH);

    for (uint8_t i = 0; i < sizeof(_glyphs) / sizeof(_glyphs[0]); i++)
    {
        uint8_t gl[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

        for (uint8_t j = 0 ; j < 8; j++)
            gl[j] = pgm_read_byte(&_glyphs[i][j]);

        aLCD::createChar(i, gl);
    }
}

void aDCDisplay::showBanner()
{
    aLCD::clear();
    aLCD::setCursor(0, 1);
    aLCD::printCenter(F("DC Electronic Load"));

    char buffer[LCD_COLS_NUM + 1];
    snprintf(buffer, LCD_COLS_NUM, "Version %d.%d", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR);

    aLCD::setCursor(0, 3);
    aLCD::printCenter(buffer);

    delay(3000);
}

void aDCDisplay::updateField(OperationMode opMode, float vSet, float vRead, uint8_t row, uint8_t unit)
{
    aLCD::clearValue(row);
    aLCD::setCursor(((opMode == OPERATION_READ) ? OFFSET_VALUE : (OFFSET_MARKER_RIGHT - getNumericalLength(vSet)) - 1), row);
    aLCD::print((opMode == OPERATION_READ) ? vRead : vSet, 3);
    aLCD::print(char(unit));
}

void aDCDisplay::updateDisplay()
{
    aDCSettings    *d           = (aDCSettings *)m_Parent->_getSettings();
    bool            fullRedraw  = false;
    OperationMode   opMode;

    switch (d->getDisplayMode())
    {
        case DISPLAY_VALUES:
            if (d->isDisplayModeChanged())
            {
                fullRedraw = true;

                aLCD::clear();
                aLCD::setCursor(OFFSET_UNIT, 0);
                aLCD::print(F("U: "));
                aLCD::setCursor(OFFSET_UNIT + OFFSET_TEMP, 0);
                aLCD::print(char(0xDF)); // °
                aLCD::print(F("C: "));
                aLCD::setCursor(OFFSET_UNIT, SELECTION_CURRENT + 1);
                aLCD::print(F("I: "));
#ifdef RESISTANCE
                aLCD::setCursor(OFFSET_UNIT, SELECTION_RESISTANCE + 1);
                aLCD::print(F("R: "));
#endif
                aLCD::setCursor(OFFSET_UNIT, SELECTION_POWER + 1);
                aLCD::print(F("P: "));
            }

            if (!d->isVoltageAlreadyDisplayed() || fullRedraw)
            {
                aLCD::clearValue(0, -2);
                aLCD::setCursor(OFFSET_VALUE, 0);
                aLCD::print(d->getVoltageRead(), 3);
                aLCD::print('V');
                d->syncVoltageDisp();
            }

            if (!d->isTemperatureAlreadyDisplayed() || fullRedraw)
            {
                aLCD::setCursor(OFFSET_VALUE + OFFSET_TEMP + 1, 0);
                aLCD::print(F("   "));
                aLCD::setCursor(OFFSET_VALUE + OFFSET_TEMP + 1, 0);
                aLCD::print(d->getTemperatureRead(), DEC);
                d->syncTemperatureDisp();
            }

            // Display Current Set/Read
            opMode = d->getOperationMode();

            if (d->isOperationModeChanged())
            {
                fullRedraw = true;
                d->syncOperationMode();
            }

            if (!d->isCurrentAlreadyDisplayed() || fullRedraw)
            {
                updateField(opMode, d->getCurrentSets(), d->getCurrentRead(), SELECTION_CURRENT + 1, 'A');
                d->syncCurrentDisp();
            }

#ifdef RESISTANCE
            if (!d->isResistanceAlreadyDisplayed() || fullRedraw)
            {
                updateField(opMode, d->getResistanceSets(), d->getResistanceRead(), SELECTION_RESISTANCE + 1, char(0xF4));
                d->syncResistanceDisp();
            }
#endif // RESISTANCE

            if (!d->isPowerAlreadyDisplayed() || fullRedraw)
            {
                updateField(opMode, d->getPowerSets(), d->getPowerRead(), SELECTION_POWER + 1, 'W');
                d->syncPowerDisp();
            }

            if (d->isSelectionModeChanged() || d->isDisplayModeChanged() || fullRedraw)
            {
                uint8_t prevMode = static_cast<uint8_t>(d->getPrevMode(d->getSelectionMode())) + 1;
                uint8_t mode = static_cast<uint8_t>(d->getSelectionMode()) + 1;

                // Clear previous mode selection markers
                aLCD::setCursor(OFFSET_MARKER_LEFT, prevMode);
                aLCD::print(' ');
                aLCD::setCursor(OFFSET_MARKER_RIGHT, prevMode);
                aLCD::print(F("  ")); // Marker + stepper

                // Force stepper redraw
                fullRedraw = true;

                aLCD::setCursor(OFFSET_MARKER_LEFT, mode);
                aLCD::print('[');
                aLCD::setCursor(OFFSET_MARKER_RIGHT, mode);
                aLCD::print(']');

                d->syncSelectionMode();
            }

            // Update Stepper icon
            if (!d->incIsSynced() || fullRedraw)
            {
                aLCD::setCursor(OFFSET_MARKER_RIGHT + 1, static_cast<uint8_t>(d->getSelectionMode()) + 1);
                aLCD::write(d->incGetValue());
                d->incSync();
            }

            // Features icons
            // LOGGING
            if (d->isFeatureEnabled(FEATURE_LOGGING) && (!d->isFeatureEnabled(FEATURE_LOGGING_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_LOGGING_VISIBLE);
                aLCD::setCursor(LOGGING_ICON_X_COORD, LOGGING_ICON_Y_COORD);
                aLCD::write(char(0xD0));
            }
            else if (!d->isFeatureEnabled(FEATURE_LOGGING) && (d->isFeatureEnabled(FEATURE_LOGGING_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_LOGGING_VISIBLE, false);
                aLCD::setCursor(LOGGING_ICON_X_COORD, LOGGING_ICON_Y_COORD);
                aLCD::write(char(0x20));
            }
            // USB
            if (d->isFeatureEnabled(FEATURE_USB) && (!d->isFeatureEnabled(FEATURE_USB_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_USB_VISIBLE);
                aLCD::setCursor(USB_ICON_X_COORD, USB_ICON_Y_COORD);
                aLCD::write(GLYPH_USB);
            }
            else if (!d->isFeatureEnabled(FEATURE_USB) && (d->isFeatureEnabled(FEATURE_USB_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_USB_VISIBLE, false);
                aLCD::setCursor(USB_ICON_X_COORD, USB_ICON_Y_COORD);
                aLCD::write(char(0x20));
            }
            // LOCK
            if (d->isAutolocked() && (!d->isFeatureEnabled(FEATURE_LOCKED_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_LOCKED_VISIBLE);
                aLCD::setCursor(LOCK_ICON_X_COORD, LOCK_ICON_Y_COORD);
                aLCD::write(GLYPH_LOCK);
            }
            else if (!d->isFeatureEnabled(FEATURE_LOCKED) && (d->isFeatureEnabled(FEATURE_LOCKED_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_LOCKED_VISIBLE, false);
                aLCD::setCursor(LOCK_ICON_X_COORD, LOCK_ICON_Y_COORD);
                aLCD::write(char(0x20));
            }

            // OVP
            if (d->isFeatureEnabled(FEATURE_OVP) && (!d->isFeatureEnabled(FEATURE_OVP_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_OVP_VISIBLE);

                aLCD::setCursor(ALARM_OV_X_COORD, ALARM_OV_Y_COORD);
                aLCD::print(F("OV"));
            }
            else if (!d->isFeatureEnabled(FEATURE_OVP) && (d->isFeatureEnabled(FEATURE_OVP_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_OVP_VISIBLE, false);

                aLCD::setCursor(ALARM_OV_X_COORD, ALARM_OV_Y_COORD);
                aLCD::print(F("  "));
            }

            // OCP
            if (d->isFeatureEnabled(FEATURE_OCP) && (!d->isFeatureEnabled(FEATURE_OCP_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_OCP_VISIBLE);

                aLCD::setCursor(ALARM_OC_X_COORD, ALARM_OC_Y_COORD);
                aLCD::print(F("OC"));
            }
            else if (!d->isFeatureEnabled(FEATURE_OCP) && (d->isFeatureEnabled(FEATURE_OCP_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_OCP_VISIBLE, false);

                aLCD::setCursor(ALARM_OC_X_COORD, ALARM_OC_Y_COORD);
                aLCD::print(F("  "));
            }
            break;

      case DISPLAY_SETUP:
            pingBacklight();
            d->pingAutolock();
            if (d->isDisplayModeChanged())
            {
                aLCD::clear();
                aLCD::setCursor(0, 0);
                aLCD::print("Options:");
                d->syncDisplayMode();

                aLCD::setCursor(0, 1);
                aLCD::printCenter("Auto Dimmer");
                aLCD::setCursor(0, 2);
                aLCD::printCenter("Auto Lock");
            }

            if (d->isSelectionModeChanged() || d->isDisplayModeChanged() || fullRedraw)
            {
                uint8_t prevMode = static_cast<uint8_t>(d->getPrevMode(d->getSelectionMode())) + 1;
                uint8_t mode = static_cast<uint8_t>(d->getSelectionMode()) + 1;

                // Clear previous mode selection markers
                aLCD::setCursor(OFFSET_SETUP_MARKER_LEFT, prevMode);
                aLCD::print(' ');
                aLCD::setCursor(OFFSET_SETUP_MARKER_RIGHT, prevMode);
                aLCD::print(' ');

                // Force stepper redraw
                fullRedraw = true;

                aLCD::setCursor(OFFSET_SETUP_MARKER_LEFT, mode);
                aLCD::print('[');
                aLCD::setCursor(OFFSET_SETUP_MARKER_RIGHT, mode);
                aLCD::print(']');

                d->syncSelectionMode();
            }

            if (d->isFeatureEnabled(FEATURE_AUTODIM) && (!d->isFeatureEnabled(FEATURE_AUTODIM_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_AUTODIM_VISIBLE);
                aLCD::setCursor(OFFSET_SETUP_MARKER_LEFT + 1, 1);
                aLCD::write(GLYPH_CHECKBOX_TICKED);
            }
            else if (!d->isFeatureEnabled(FEATURE_AUTODIM) && (d->isFeatureEnabled(FEATURE_AUTODIM_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_AUTODIM_VISIBLE, false);
                aLCD::setCursor(OFFSET_SETUP_MARKER_LEFT + 1, 1);
                aLCD::write(GLYPH_CHECKBOX_UNTICKED);
            }

            if (d->isFeatureEnabled(FEATURE_AUTOLOCK) && (!d->isFeatureEnabled(FEATURE_AUTOLOCK_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_AUTOLOCK_VISIBLE);
                aLCD::setCursor(OFFSET_SETUP_MARKER_LEFT + 1, 2);
                aLCD::write(GLYPH_CHECKBOX_TICKED);
            }
            else if (!d->isFeatureEnabled(FEATURE_AUTOLOCK) && (d->isFeatureEnabled(FEATURE_AUTOLOCK_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_AUTOLOCK_VISIBLE, false);
                aLCD::setCursor(OFFSET_SETUP_MARKER_LEFT + 1, 2);
                aLCD::write(GLYPH_CHECKBOX_UNTICKED);
            }
            break;

      default:
        break;
    }

    // Backlight dimming
    if (d->isFeatureEnabled(FEATURE_AUTODIM) && (!m_dimmed && ((millis() - m_dimmerTick) >= BACKLIGHT_TIMEOUT)))
        _dimmingBacklight();
}

void aDCDisplay::_dimBacklight(bool up)
{
    uint16_t n = up ? 2 : 254;

    while (up ? n < 255 : n != 0)
    {
        analogWrite(LED_BACKLIGHT_PIN, n);
        delay(2);
        n = up ? n + 2 : n - 2;
    }
    digitalWrite(LED_BACKLIGHT_PIN, (up ? HIGH : LOW));

    m_dimmed = (up ? false : true);
}

void aDCDisplay::_dimmingBacklight()
{
    _dimBacklight(false);
}

void aDCDisplay::_wakeupBacklight()
{
    _dimBacklight(true);
}

void aDCDisplay::pingBacklight()
{
    m_dimmerTick = millis();

    if (m_dimmed)
        _wakeupBacklight();
}

bool aDCDisplay::isBacklightDimmed()
{
    return m_dimmed;
}

/**
*** Class to manage settings
**/
aDCEngine::aDCEngine(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t cols, uint8_t rows,
                     uint8_t enca, uint8_t encb, uint8_t encpb, uint8_t encsteps) : aDCDisplay(this, rs, enable, d0, d1, d2, d3, cols, rows),
                    m_encoder(new ClickEncoder(enca, encb, encpb, encsteps)), m_RXoffset(0)
{
    memset(&m_RXbuffer, '\0', sizeof(m_RXbuffer));
}

aDCEngine::~aDCEngine()
{
}

void aDCEngine::_updateLoggingAndRemote()
{
    if (Serial)
    {
        bool single = false;

        if (Serial.available() > 0)
        {
            bool EOL = false;

            if (!m_Data.isFeatureEnabled(FEATURE_USB))
                m_Data.enableFeature(FEATURE_USB);

            while (Serial.available() > 0)
            {
                m_RXbuffer[m_RXoffset] = Serial.read();

                if (m_RXbuffer[m_RXoffset] == 0xA)
                {
                    EOL = true;
                    break;
                }

                m_RXoffset++;
            }

            m_RXbuffer[m_RXoffset] = '\0';

            if (EOL)
            {
                bool valid = false;

                if (m_RXoffset >= 3)
                {
                    uint8_t *cmdStart, *cmdEnd;
                    if (((cmdStart = (uint8_t *)strchr((const char *)&m_RXbuffer[0], ':')) != NULL) &&
                        ((cmdEnd = (uint8_t *)strchr((const char *)cmdStart + 1, ':')) != NULL))
                    {
                        uint8_t cmd[(cmdEnd - cmdStart)];
                        uint8_t *arg = cmdEnd + 1;

                        memcpy(&cmd[0], cmdStart + 1, sizeof(cmd) - 1);
                        cmd[sizeof(cmd) - 1] = '\0';

                        serialPrint(':');

                        if (!strcasecmp((const char *)cmd, "ISET?")) // Get Current Settings
                        {
                            serialPrint(floatRounding(floatRounding(m_Data.getCurrentSets()) * floatRounding(1000.000)), 0);
                            valid = true;
                        }
                        else if (!strcasecmp((const char *)cmd, "ISET")) // Current Setting
                        {
                            float v = atof((const char *)arg);
                            float nv = floatRounding(floatRounding(v) / floatRounding(1000.000));

                            m_Data.updateValuesFromMode(nv, SELECTION_CURRENT);

                            serialPrint(floatRounding(floatRounding(m_Data.getCurrentSets()) * floatRounding(1000.000)), 0);

                            switch (m_Data.getSelectionMode())
                            {
                                case SELECTION_CURRENT:
                                    m_Data.setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(m_Data.getCurrentSets()) * floatRounding(500.000))));
                                    break;
#ifdef RESISTANCE
                                case SELECTION_RESISTANCE:
                                    m_Data.setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(m_Data.getResistanceSets()) * floatRounding(1000.000))));
                                    break;
#endif
                                case SELECTION_POWER:
                                    m_Data.setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(m_Data.getPowerSets()) * floatRounding(1000.000))));
                                    break;
                                default:
                                    break;
                            }

                            m_Data.syncEncoderPosition();

                            valid = (m_Data.getCurrentSets() == nv);
                        }
#ifdef SIMU
                        else if (!strcasecmp((const char *)cmd, "USET")) // Voltage Read Setting
                        {
                            float v = atof((const char *)arg);
                            if (m_Data.setVoltageRead(floatRounding(floatRounding(v) / floatRounding(1000.000))) == aDCSettings::SETTING_OVERSIZED)
                            {
                                m_Data.enableFeature(FEATURE_OVP);
                                m_Data.setCurrentSets(0.0);
                                m_Data.setEncoderPosition(0);
                                m_Data.syncEncoderPosition();
                            }

                            serialPrint(floatRounding(floatRounding(m_Data.getVoltageRead()) * floatRounding(1000.000)), 0);
                            valid = true;
                        }
                        else if (!strcasecmp((const char *)cmd, "IS")) // Set Current Read
                        {
                            float v = atof((const char *)arg);
                            float nv = floatRounding(floatRounding(v) / floatRounding(1000.000));

                            m_Data.setCurrentRead(nv);

                            serialPrint(floatRounding(floatRounding(m_Data.getCurrentRead()) * floatRounding(1000.000)), 0);

                            valid = (m_Data.getCurrentRead() == nv);
                        }
#endif
                        else if (!strcasecmp((const char *)cmd, "I?")) // Report Current Read
                        {
                            serialPrint(floatRounding(floatRounding(m_Data.getCurrentRead()) * floatRounding(1000.000)), 0);
                            valid = true;
                        }
                        else if (!strcasecmp((const char *)cmd, "U?")) // Report Voltage Read
                        {
                            serialPrint(floatRounding(floatRounding(m_Data.getVoltageRead()) * floatRounding(1000.000)), 0);
                            valid = true;
                        }
                        else if (!strcasecmp((const char *)cmd, "L?")) // Logging Ask
                        {
                            serialPrint(m_Data.isFeatureEnabled(FEATURE_LOGGING) ? F("ON") : F("OFF"));
                            valid = true;
                        }
                        else if (!strcasecmp((const char *)cmd, "L")) // Logging ON/OFF
                        {
                            if (!strcasecmp((const char *)arg, "OFF"))
                                m_Data.enableFeature(FEATURE_LOGGING, false);
                            else if (!strcasecmp((const char *)arg, "ON"))
                                m_Data.enableFeature(FEATURE_LOGGING, true);
                            else
                                single = true;

                            valid = true;
                        }
#ifdef SIMU
                        else if (!strcasecmp((const char *)cmd, "T")) // Temperature Read Setting
                        {
                            uint8_t v = static_cast<uint8_t>(atoi((const char *)arg));
                            m_Data.setTemperatureRead(v);

                            serialPrint(m_Data.getTemperatureRead());
                            valid = true;
                        }
#endif
#if 0
                        else if (!strcasecmp((const char *)cmd, "?")) // Temperature Read Setting
                        {
                            serialPrintln(F("Usage :CMD:ARG"));
                            serialPrintln(F("  CMD are:"));
                            serialPrintln(F("    I?      Get current."));
                            serialPrintln(F("    I       Set max current."));
                            serialPrintln(F("    ISET    Set current setting."));
                            serialPrintln(F("    ISET?   Get current setting."));
                            serialPrintln(F("    U?      Get Voltage."));
                            serialPrintln(F("    L       Turn logging ON*, OFF* or SINGLE*."));
                            serialPrintln(F("    ?       This help."));
#ifdef SIMU
                            serialPrintln(F("IS,USET,T (simulation)."));
#endif
                            serialPrintln(F(""));
                            serialPrintln(F("* is ARG"));
                            serialPrintln(F("returns value (is needed) + ':OK:' or ':ERR:'"));
                            valid = true;
                        }
#endif
                        else
                            serialPrint(F("INVALID"));
                    }

                    // Clear buffer for next command
                    memset(&m_RXbuffer, '\0', sizeof(m_RXbuffer));
                    m_RXoffset = 0;
                }

                serialPrint(':');
                serialPrint(valid ? F("OK") : F("ERR"));
                serialPrintln(':');
            }
        }

        // Logging
        /**
        *** Format is: :L:timestamp:voltage:current:temperature::
        ***         timestamp in milliseconds
        ***         voltage in mV
        ***         current in mA
        ***         temperature in Celcius
        **/
        unsigned long m = millis();
        if ((m_Data.isFeatureEnabled(FEATURE_LOGGING) && !(m % 100)) || single)
        {
            serialPrint(m / 100);
            serialPrint(',');
            serialPrint(floatRounding(floatRounding(m_Data.getVoltageRead()) * floatRounding(1000.000)), 0);
            serialPrint(',');
            serialPrint(floatRounding(floatRounding(m_Data.getCurrentRead()) * floatRounding(1000.000)), 0);
            serialPrint(',');
            serialPrint(m_Data.getTemperatureRead());
            serialPrintln();
        }

        Serial.flush();
    }
    else
    {
        if (m_Data.isFeatureEnabled(FEATURE_LOGGING))
            m_Data.enableFeature(FEATURE_LOGGING, false);

        if (m_Data.isFeatureEnabled(FEATURE_USB))
            m_Data.enableFeature(FEATURE_USB, false);

    }
}

void aDCEngine::setup(ISRCallback isr)
{
    // set outputs:
    pinMode(ADC_CHIPSELECT_PIN, OUTPUT);
    pinMode(DAC_CHIPSELECT_PIN, OUTPUT);

    // set the ChipSelectPins high initially:
    digitalWrite(ADC_CHIPSELECT_PIN, HIGH);
    digitalWrite(DAC_CHIPSELECT_PIN, HIGH);

    // initialise SPI:
    SPI.begin();
    SPI.setBitOrder(MSBFIRST);         // Not strictly needed but just to be sure.
    SPI.setDataMode(SPI_MODE0);        // Not strictly needed but just to be sure.
    // Set SPI clock divider to 16, therfore a 1 MhZ signal due to the maximum frequency of the ADC.
    SPI.setClockDivider(SPI_CLOCK_DIV16);

    // Initialise timer
    Timer1.initialize(1000);
    Timer1.attachInterrupt(isr);

    aDCDisplay::setup();
    aDCDisplay::showBanner();

    // Data Logging and Remote Control
    Serial.begin(9600);
    Serial.flush();
    // Flushing buffered char.
     if (Serial)
    {
        while (Serial.available() > 0)
            Serial.read();
    }
}

void aDCEngine::run()
{
    m_Data.pingAutolock();
    pingBacklight();
    m_Data.pingOperationMode();

    while(true)
    {
        int32_t v = m_encoder->getValue();
        float oldPos = m_Data.getEncoderPosition();

        m_Data.updateOperationMode();

        if ((v != 0) && (!m_Data.isFeatureEnabled(FEATURE_LOCKED)) && (m_Data.getDisplayMode() == DISPLAY_VALUES))
        {
            int16_t mult = m_Data.incGetValueFromMode(m_Data.getSelectionMode());

            m_Data.incEncoderPosition((v * mult));
        }


        // Is USB control is enabled and the encoder has been rotated, turn OFF USB icon
        if (m_Data.isEncoderPositionChanged())
        {
            // Is dimmed, wake up the backlight
            if (isBacklightDimmed())
            {
                m_Data.setEncoderPosition(oldPos);
                m_Data.syncEncoderPosition();

                pingBacklight();
                continue;
            }
/*
            // If autolock is enabled, ignore encoder new position
            if (m_Data.isFeatureEnabled(FEATURE_AUTOLOCK))
            {
                if (m_Data.isFeatureEnabled(FEATURE_LOCKED))
                    continue;
            }
*/
            // Remote is enable, disable it now
            if (m_Data.isFeatureEnabled(FEATURE_USB))
                m_Data.enableFeature(FEATURE_USB, false);

            // If current operation mode is READ, set display to SET mode, handling next encoder event
            if (m_Data.getOperationMode() == OPERATION_READ)
            {
                // Ignore new encoder position
                m_Data.setEncoderPosition(oldPos);
                m_Data.syncEncoderPosition();
                // Turn on settings adjustments
                m_Data.setOperationMode(OPERATION_SET);
                m_Data.pingOperationMode();
                continue;
            }

            // Reset backlight auto dimming timer
            pingBacklight();
            m_Data.pingOperationMode();

            if (m_Data.isFeatureEnabled(FEATURE_OVP))
                m_Data.enableFeature(FEATURE_OVP, false);

            if (m_Data.isFeatureEnabled(FEATURE_OCP))
                m_Data.enableFeature(FEATURE_OCP, false);
        }
        else
        {
            // Is dimmed and locked, wake up the backlight
            if ((v != 0) && m_Data.isFeatureEnabled(FEATURE_AUTOLOCK) && isBacklightDimmed())
            {
                pingBacklight();
                continue;
            }
        }

        ClickEncoder::Button b = m_encoder->getButton();
        if (b != ClickEncoder::Open)
        {
            if ((b == ClickEncoder::Clicked) || (b == ClickEncoder::DoubleClicked))
            {
                // Is dimmed, wake up the backlight
                if (isBacklightDimmed())
                {
                    pingBacklight();
                    continue;
                }

                // If autolock feature is enabled
                if (m_Data.isFeatureEnabled(FEATURE_AUTOLOCK))
                {
                    // Unlocked, reset auto lock timer
                    if (!m_Data.isFeatureEnabled(FEATURE_LOCKED))
                        m_Data.pingAutolock();
                    else
                    {
                        // Unlock
                        if (b == ClickEncoder::DoubleClicked)
                        {
                            m_Data.enableFeature(FEATURE_LOCKED, false);
                            m_Data.pingAutolock();
                        }

                        continue;
                    }
                }

                // Reset backlight auto dimming timer
                pingBacklight();
            }

            if (!m_Data.isFeatureEnabled(FEATURE_LOCKED) && (b == ClickEncoder::Held && !m_Data.isDisplayModeChanged()))
            {
                do
                {
                    m_Data.pingAutolock();
                } while (m_encoder->getButton() != ClickEncoder::Released);

                m_Data.setSelectionMode(SELECTION_UNKNOWN);
                m_Data.setSelectionMode(SELECTION_CURRENT);
                m_Data.enableFeature(FEATURE_LOCKED, false);
                m_Data.setDisplayMode(static_cast<DisplayMode>(!m_Data.getDisplayMode()));
            }

            // Increment encoder step (0.001, 0.01, 0.1, 1.0)
            if ((b == ClickEncoder::Clicked) && (m_Data.getDisplayMode() == DISPLAY_VALUES))
                m_Data.incIncrement();

            // Change selection mode according to double click.
            _handleButtonEvent(b);
        }

        // Reads input voltage from the load source. ****MAXIMUM 24V INPUT****
        if (m_Data.setVoltageRead(_readInputVoltage()) == aDCSettings::SETTING_OVERSIZED)
        {
            // Over-Voltage protection triggered
            m_Data.enableFeature(FEATURE_OVP);
            m_Data.setCurrentSets(0.0);
            m_Data.setEncoderPosition(0);
        }

        // Calculates and sets required load current. Accepts the mode variable which defines the mode of the unit,
        // ie. Constant current, resistance or power.
        _updateLoadCurrent();

        // Calculates heatsink temprature and sets fan speed accordingly.
        _updateFanSpeed();

        // Updates the LCD display. Accepts the lcdDisplay variable which defines if the values or menu is to be displayed.
        updateDisplay();

        if (m_Data.isDisplayModeChanged())
            m_Data.syncDisplayMode();

        if (m_Data.isEncoderPositionChanged())
            m_Data.syncEncoderPosition();

         // Data logging and USB controlling
        _updateLoggingAndRemote();
    }
}

void aDCEngine::_handleButtonEvent(ClickEncoder::Button button)
{
    switch (m_Data.getDisplayMode())
    {
        case DISPLAY_VALUES:
            if (button == ClickEncoder::DoubleClicked)
            {
                m_Data.setSelectionMode(m_Data.getNextMode());
                // Force redraw
                m_Data.incEncoderPosition(-1);

                switch (m_Data.getSelectionMode())
                {
                    case SELECTION_CURRENT:
                        m_Data.setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(m_Data.getCurrentSets()) * floatRounding(500.000))));
                        break;
#ifdef RESISTANCE
                    case SELECTION_RESISTANCE:
                        m_Data.setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(m_Data.getResistanceSets()) * floatRounding(1000.000))));
                        break;
#endif
                    case SELECTION_POWER:
                        m_Data.setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(m_Data.getPowerSets()) * floatRounding(1000.000))));
                        break;
                    default:
                        break;
                }
            }
            break;

        case DISPLAY_SETUP:
            switch (button)
            {
                case ClickEncoder::DoubleClicked:
                    m_Data.setSelectionMode(m_Data.getNextMode());
                    break;

                case ClickEncoder::Clicked:
                    switch(m_Data.getSelectionMode())
                    {
                        case SELECTION_CURRENT:
                            m_Data.enableFeature(FEATURE_AUTODIM, !m_Data.isFeatureEnabled(FEATURE_AUTODIM));
                            break;

#ifdef RESISTANCE
                        case SELECTION_RESISTANCE:
#else
                        case SELECTION_POWER:
#endif
                            m_Data.enableFeature(FEATURE_AUTOLOCK, !m_Data.isFeatureEnabled(FEATURE_AUTOLOCK));
                            break;

                        default:
                            break;
                    }
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
}

/**
***
**/
void aDCEngine::service()
{
    m_encoder->service();
}

/**
*** Function to read the input voltage and return a float number represention volts.
**/
float aDCEngine::_readInputVoltage()
{
#ifdef SIMU
    float v = m_Data.getVoltageRead();// 1.000;
#else
    float v = (_readADC(ADC_INPUTVOLTAGE_CHAN)) * 12.03;
#endif
    return (v < 0.018 ? 0 : v);
}

/**
*** Function to measure the actual load current.
**/
float aDCEngine::_readMeasuredCurrent()
{
#ifdef SIMU
    return m_Data.getCurrentRead();
#else
    return ((_readADC(ADC_MEASUREDCURRENT_CHAN)) / 0.1000);
#endif // SIMU
}
/**
*** Function to calculate and set the required load current. Accepts the mode variable to determine if the constant current,
*** resistance or power mode is to be used.
**/
void aDCEngine::_updateLoadCurrent()
{
    if ((m_Data.getDisplayMode() == DISPLAY_VALUES) && m_Data.isEncoderPositionChanged())
    {
        switch (m_Data.getSelectionMode())
        {
            case SELECTION_CURRENT:
                m_Data.updateValuesFromMode(static_cast<float>(floatRounding(floatRounding(float(m_Data.getEncoderPosition())) / floatRounding(500.000))), SELECTION_CURRENT);
                break;

#ifdef RESISTANCE
            case SELECTION_RESISTANCE:
                m_Data.updateValuesFromMode(static_cast<float>(floatRounding(floatRounding(float(m_Data.getEncoderPosition())) / floatRounding(1000.000))), SELECTION_RESISTANCE);
                break;
#endif
            case SELECTION_POWER:
                m_Data.updateValuesFromMode(static_cast<float>(floatRounding(floatRounding(float(m_Data.getEncoderPosition())) / floatRounding(1000.000))), SELECTION_POWER);
                break;

            default:
                break;
        }
    }

    // Convert the set current into an voltage to be sent to the DAC
    switch (m_Data.setCurrentRead(_readMeasuredCurrent()))
    {
        case aDCSettings::SETTING_OVERSIZED:
            // Overcurrent alarm
            m_Data.enableFeature(FEATURE_OCP);
            m_Data.setCurrentSets(0.0);
            m_Data.setEncoderPosition(0);
            break;

        case aDCSettings::SETTING_UNDERSIZED:
        case aDCSettings::SETTING_VALID:
            //m_Data.setOverCurrent(false);
            _adjustCurrent();
            break;
    }
}
#warning check if currentread <= currentset
void aDCEngine::_adjustCurrent()
{
    float roundedMeasuredCurrent = round(m_Data.getCurrentRead() * 1000) / 1000.000; // This the best way I can think of rounding a floating
                                                                                     // point number to 3 decimal places.
    //only adjust the current of the set and measured currents are different.
    if (roundedMeasuredCurrent != m_Data.getCurrentSets())
    {
        float adjustedCurrent = 0.0;

        // To ensure we are not dividing by 0.
        if (m_Data.getCurrentRead() != 0.0)                                                                  // Turn the current error between set and measured
            adjustedCurrent = (m_Data.getCurrentSets() / m_Data.getCurrentRead()) * m_Data.getCurrentSets(); // into a percentage so it can be adjusted
        else
            adjustedCurrent = m_Data.getCurrentSets();

        int16_t dacCurrent = ((adjustedCurrent * 0.1 * 2.5) / 2.048) * 4096;

        // Send the value to the DAC.
        _setDAC(dacCurrent, DAC_CURRENT_CHAN);

        // Read current value again
#ifdef SIMU
        m_Data.setCurrentRead(m_Data.getCurrentSets());
#else
        m_Data.setCurrentRead(_readMeasuredCurrent());
#endif
    }

#ifdef RESISTANCE
    m_Data.setResistanceRead((m_Data.getCurrentRead() > 0.0) ? m_Data.getVoltageRead() / m_Data.getCurrentRead() : 0);
#endif // RESISTANCE

    switch (m_Data.setPowerRead(m_Data.getVoltageRead() * m_Data.getCurrentRead()))
    {
        case aDCSettings::SETTING_OVERSIZED:
            m_Data.enableFeature(FEATURE_OCP);
            m_Data.setCurrentSets(0.0);
#ifdef SIMU
            m_Data.setCurrentRead(0.0);
#endif // SIMU
            m_Data.setEncoderPosition(0);
            _adjustCurrent();
            break;

        default:
            break;
    }
}

/**
*** Function to read the ADC, accepts the channel to be read.
**/
float aDCEngine::_readADC(uint8_t channel)
{
    uint8_t adcPrimaryRegister = 0b00000110;    // Sets default Primary ADC Address register B00000110, This is a default address
                                                // setting, the third LSB is set to one to start the ADC, the second LSB is to set
                                                // the ADC to single ended mode, the LSB is for D2 address bit, for this ADC its
                                                // a "Don't Care" bit.
    uint8_t adcPrimaryRegisterMask = 0b00000111;  // b00000111 Isolates the three LSB.
    uint8_t adcPrimaryConfig = adcPrimaryRegister & adcPrimaryRegisterMask; // ensures the adc register is limited to the mask and
                                                                            // assembles the configuration byte to send to ADC.
    uint8_t adcSecondaryConfig = channel << 6;

    noInterrupts(); // disable interrupts to prepare to send address data to the ADC.

    digitalWrite(ADC_CHIPSELECT_PIN, LOW); // take the Chip Select pin low to select the ADC.

    SPI.transfer(adcPrimaryConfig); //  send in the primary configuration address byte to the ADC.

    uint8_t adcPrimaryByte = SPI.transfer(adcSecondaryConfig); // read the primary byte, also sending in the secondary address byte.
    uint8_t adcSecondaryByte = SPI.transfer(0x00); // read the secondary byte, also sending 0 as this doesn't matter.

    digitalWrite(ADC_CHIPSELECT_PIN, HIGH); // take the Chip Select pin high to de-select the ADC.

    interrupts(); // Enable interrupts.

    uint8_t adcPrimaryByteMask = 0b00001111;      // b00001111 isolates the 4 LSB for the value returned.

    adcPrimaryByte &= adcPrimaryByteMask; // Limits the value of the primary byte to the 4 LSB:

    uint16_t digitalValue = (adcPrimaryByte << 8) | adcSecondaryByte;   // Shifts the 4 LSB of the primary byte to become the 4 MSB
                                                                        // of the 12 bit digital value, this is then ORed to the
                                                                        // secondary byte value that holds the 8 LSB of the digital value.

    return ((float(digitalValue) * 2.048) / 4096.000); // The digital value is converted to an analogue voltage using a VREF of 2.048V.
}

/**
*** Function to set the DAC, Accepts the Value to be sent and the channel of the DAC to be used.
**/
void aDCEngine::_setDAC(uint16_t value, uint8_t channel)
{
    uint8_t dacRegister = 0b00110000;   // Sets default DAC registers B00110000, 1st bit choses DAC, A=0 B=1, 2nd Bit bypasses input
                                        // Buffer, 3rd bit sets output gain to 1x, 4th bit controls active low shutdown. LSB are
                                        // insignifigant here.
    uint8_t dacSecondaryByteMask = 0xFF; // Isolates the last 8 bits of the 12 bit value, B0000000011111111.
    uint8_t dacPrimaryByte = (value >> 8) | dacRegister;    // Value is a maximum 12 Bit value, it is shifted to the right by 8 bytes
                                                            // to get the first 4 MSB out of the value for entry into th Primary Byte,
                                                            // then ORed with the dacRegister
    uint8_t dacSecondaryByte = value & dacSecondaryByteMask; // compares the 12 bit value to isolate the 8 LSB and reduce it to a single byte.

    // Sets the MSB in the primaryByte to determine the DAC to be set, DAC A=0, DAC B=1
    switch (channel)
    {
        case ADC_INPUTVOLTAGE_CHAN:
            dacPrimaryByte &= ~(1 << 7);
            break;

        case ADC_MEASUREDCURRENT_CHAN:
            dacPrimaryByte |= (1 << 7);
            break;
    }

    noInterrupts(); // disable interupts to prepare to send data to the DAC

    digitalWrite(DAC_CHIPSELECT_PIN, LOW); // take the Chip Select pin low to select the DAC:

    SPI.transfer(dacPrimaryByte); //  send in the Primary Byte:
    SPI.transfer(dacSecondaryByte);// send in the Secondary Byte

    digitalWrite(DAC_CHIPSELECT_PIN, HIGH);// take the Chip Select pin high to de-select the DAC:

    interrupts(); // Enable interupts
}

/**
*** Function to read heat sink temp
**/
int8_t aDCEngine::_readTemp()
{
#ifdef SIMU
    return m_Data.getTemperatureRead();
#else
    float tempSensor1 = _readADC(ADC_TEMPSENSE1_CHAN);
    float tempSensor2 = _readADC(ADC_TEMPSENSE2_CHAN);
    float tempVoltage = ((tempSensor1 + tempSensor2) / 2) * 1000; // This takes an average of bothe temp sensors and converts the
                                                                  // value to millivolts

    return int8_t(((tempVoltage - 1885) / -11.2692307) + 20); //This comes from the datasheet to calculate the temp from the voltage given.
#endif // SIMU
}

/**
*** Function to set the fan speed depending on the heatsink temprature.
**/
void aDCEngine::_updateFanSpeed()
{
    m_Data.setTemperatureRead(_readTemp());

    uint8_t heatSinkTemp = m_Data.getTemperatureRead();
    static const struct
    {
        uint16_t temp;
        uint16_t speed;
    } fanThresholds[] PROGMEM =
    {
        { 70, 3000 },
        { 60, 2750 },
        { 50, 2500 },
        { 40, 2250 },
        { 30, 2000 },
        { 0,  0}
    };

    for (uint8_t i = 0; i < (sizeof(fanThresholds) / sizeof(fanThresholds[0])); i++)
    {
        if (heatSinkTemp >= static_cast<uint16_t>(pgm_read_word(&fanThresholds[i].temp)))
        {
            if (m_Data.getFanSpeed() != static_cast<uint16_t>(pgm_read_word(&fanThresholds[i].speed)))
            {
                m_Data.setFanSpeed(static_cast<uint16_t>(pgm_read_word(&fanThresholds[i].speed)));
                _setDAC(m_Data.getFanSpeed(), DAC_FAN_CHAN);
            }
            break;
        }
    }
}

const aDCSettings *aDCEngine::_getSettings() const
{
    return &m_Data;
}
