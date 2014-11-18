/**
 *
 * \copyright Copyright (C) 2014  Lee Wiggins <lee@wigweb.com.au>
 *  \copyright Copyright (C) 2014  F1RMB, Daniel Caujolle-Bert <f1rmb.daniel@gmail.com>
 *
 * \license
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.<br><br>
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.<br><br>
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**
 *
 * \note
<h1><center> BIG FAT WARNING </center></h1>
<center>Should be compiled with  <b> "-Os" </b> flag.</center>
<center>Bootloader <b><span style="text-decoration:underline;color:red;">couldn't</span></b> be flashed, <b>ISP programming ONLY</b></center>
<center> -=- use <i>Code::Blocks</i> or the included <i>Makefile</i> to compile -=- </center>
*/
#warning !!! BIG FAT WARNING !!!
#warning !!! Should be compiled with "-Os" !!!

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <LiquidCrystal.h>
#include <SPI.h>
#include <EEPROM.h>
#include <ClickEncoder.h>
#include <TimerOne.h>

#include "aDCLoad.h"

/** \file aDCLoad.cpp
    \author Lee Wiggins <lee@wigweb.com.au>
    \author F1RMB, Daniel Caujolle-Bert <f1rmb.daniel@gmail.com>
*/


/**
*** Implement our serial print function to save ~300ko
**/
// Prototype
#if 1
void serialPrint(unsigned long, int = DEC);

/** \brief Serial printing
 *
 * \param c char
 * \return void
 *
 */
void serialWrite(char c)
{
    Serial.print(c);
}

/** \brief Serial printing
 *
 * \param str char[]
 * \return void
 *
 */
void serialPrint(const char str[])
{
#if 1
    Serial.print(str);
#else
    char *p = (char *)str;

    while(*p != '\0')
    {
        serialWrite(*p);
        p++;
    }
#endif
}

#if 0
/** \brief Serial printing
 *
 * \param ifsh const __FlashStringHelper*
 * \return void
 *
 */
void serialPrint(const __FlashStringHelper *ifsh)
{
    Serial.print(ifsh);
#if 0
    const char *__attribute__((progmem)) p = (const char * ) ifsh;
    while (1)
    {
        unsigned char c = pgm_read_byte(p++);
        if (c == 0)
            break;
        serialWrite(c);
    }
#endif
}
#endif

/** \brief Serial printing
 *
 * \param c char
 * \return void
 *
 */
void serialPrint(char c)
{
    serialWrite(c);
}

/** \brief Serial printing
 *
 * \param n int
 * \param base int16_t
 * \return void
 *
 */
void serialPrint(int n, int16_t base = DEC)
{
   serialPrint((unsigned long) n, base);
}

void serialFlush()
{
    Serial.flush();
}

/** \brief Serial printing
 *
 * \return void
 *
 */
void serialPrintln()
{
    serialPrint("\r\n");
    serialFlush();
}

/** \brief Serial printing
 *
 * \param n unsigned long
 * \param base int
 * \return void
 *
 */
void serialPrint(unsigned long n, int base)
{
    Serial.print(n, base);
}

/** \brief Serial printing
 *
 * \param n double
 * \param digits int
 * \return void
 *
 */
void serialPrint(double n, int digits)
{
    Serial.print(n, digits);
}

/** \brief Serial printing
 *
 * \param n double
 * \param digits int
 * \return void
 *
 */
void serialPrintln(double n, int digits)
{
    Serial.println(n, digits);
#if 0
    serialPrint(n, digits);
    serialPrintln();
#endif
}

#if 0
/** \brief Serial printing
 *
 * \param ifsh const __FlashStringHelper*
 * \return void
 *
 */
void serialPrintln(const __FlashStringHelper *ifsh)
{
    serialPrint(ifsh);
    serialPrintln();
}
#endif

/** \brief Serial printing
 *
 * \param c char
 * \return void
 *
 */
void serialPrintln(char c)
{
    serialWrite(c);
    serialPrintln();
}

/** \brief Get the number of bytes (characters) available for reading from the serial port
 *
 * \return int16_t
 *
 */
int16_t serialAvailable()
{
    return Serial.available();
}

/** \brief Reads incoming serial data
 *
 * \return int16_t
 *
 */
int16_t serialRead()
{
    return Serial.read();
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

/** \brief Get float string length, according to decimal man length
 *
 * \param n float : <b> Float number to analyse </b>
 * \param len uint8_t : <b> decimal max length (3 as default) </b>
 * \return int8_t
 *
 */
int8_t getNumericalLength(float n, uint8_t len = 3)
{
    char buf[32];

    if (len > 11)
	    len = 11;

    return strlen(dtostrf(n, 1, len, buf));
}

/**
*** Float rounding function
**/
/** \brief Float number rounding, extensively used
 *
 * We just get rid of +4 decimal values
 *
 * \param f float : <b> Float to rounding </b>
 * \return float
 *
 */
float floatRounding(float f)
{
	return ((f / 1000.000) * 1000.000);
}

#if 0
/** \brief Macro used for checking "value" boundaries, using "type"_MAXIMUM constant. Returns SETTING_UNDERSIZED or SETTING_OVERSIZED if "value" is out of boundaries.
 *
 * Macro used in aDCSettings class setters (some of them)
 *
 * \param type : <b> CURRENT, VOLTAGE or POWER </b>
 * \param value : <b> value to check </b>
 *
 */
#define RETURN_IF_INVALID(type, value)\
    do {\
        if (value < 0)\
            return SETTING_ERROR_UNDERSIZED;\
        else if (value > type ## _MAXIMUM)\
            return SETTING_ERROR_OVERSIZED;\
    } while(0)
#endif

/** \brief Constructor
 */
aDCSettings::aDCSettings() :
#ifdef SIMU
                    m_readVoltage(24.000),
#else
                    m_readVoltage(0),
#endif
                    m_setsCurrent(0), m_readCurrent(0),
#ifdef RESISTANCE
                    m_setsResistance(0), m_readResistance(0),
#endif
                    m_setsPower(0), m_readPower(0),
#ifdef SIMU
                    m_readTemperature(20),
#else
                    m_readTemperature(0),
#endif
                    m_fanSpeed(1),
                    m_operationMode(OPERATION_MODE_READ),
                    m_mode(SELECTION_MODE_CURRENT),
                    m_encoderPos(0),
                    m_dispMode(DISPLAY_MODE_VALUES),
                    m_lockTick(0), m_operationTick(0),
                    m_features(0x0),
                    m_datas(0xFFFF)
{
    _eepromRestore();
}

/** \brief Destructor
 */
aDCSettings::~aDCSettings()
{
}

// Voltage
/** \brief Voltage setter
 *
 * \param v float : <b> voltage </b>
 * \return aDCSettings::SettingError_t
 *
 */
aDCSettings::SettingError_t aDCSettings::setVoltage(float v)
{
    return _setValue(aDCSettings::OPERATION_MODE_READ, DATA_VOLTAGE, v, m_readVoltage, m_readVoltage, VOLTAGE_MAXIMUM);
}

/** \brief Voltage getter
 *
 * \return float : <b> voltage </b>
 *
 */
float aDCSettings::getVoltage()
{
    return m_readVoltage;
}

// Current
/** \brief Current setter
 *
 * \param v float : <b> Current </b>
 * \param mode OperationMode_t : <b> READ/SET mode storage access </b>
 * \return aDCSettings::SettingError_t
 *
 */
aDCSettings::SettingError_t aDCSettings::setCurrent(float v, OperationMode_t mode)
{
    return _setValue(mode, (mode == OPERATION_MODE_SET) ? DATA_CURRENT_SETS : DATA_CURRENT_READ, v, m_setsCurrent, m_readCurrent, CURRENT_MAXIMUM);
}

/** \brief Current getter
 *
 * \param mode OperationMode_t : <b> READ/SET mode storage access </b>
 * \return float : <b> Current </b>
 *
 */
float aDCSettings::getCurrent(OperationMode_t mode)
{
    return (mode == OPERATION_MODE_SET) ? m_setsCurrent : m_readCurrent;
}


// Resistance
#ifdef RESISTANCE
/** \brief Resistance setter
 *
 * \param v float : <b> Resistance </b>
 * \param mode OperationMode_t : <b> READ/SET mode storage access </b>
 * \return aDCSettings::SettingError_t
 *
 */
aDCSettings::SettingError_t aDCSettings::setResistance(float v, OperationMode_t mode)
{
    return _setValue(mode, (mode == OPERATION_MODE_SET) ? DATA_RESISTANCE_SETS : DATA_RESISTANCE_READ, v, m_setsResistance, m_readResistance, RESISTANCE_MAXIMUM);

}

/** \brief Resistance getter
 *
 * \param mode OperationMode_t : <b> READ/SET mode storage access </b>
 * \return float : <b> Resistance </b>
 *
 */
float aDCSettings::getResistance(OperationMode_t mode)
{
    return (mode == OPERATION_MODE_SET) ? m_setsResistance : m_readResistance;
}

#endif

// Power
/** \brief Power setter
 *
 * \param v float : <b> Power </b>
 * \param mode OperationMode_t : <b> READ/SET mode storage access </b>
 * \return aDCSettings::SettingError_t
 *
 */
aDCSettings::SettingError_t aDCSettings::setPower(float v, OperationMode_t mode)
{
    return _setValue(mode, (mode == OPERATION_MODE_SET) ? DATA_POWER_SETS : DATA_POWER_READ, v, m_setsPower, m_readPower, POWER_MAXIMUM);

}

/** \brief Power getter
 *
 * \param mode OperationMode_t : <b> READ/SET mode storage access </b>
 * \return float : <b> Power </b>
 *
 */
float aDCSettings::getPower(OperationMode_t mode)
{
    return (mode == OPERATION_MODE_SET) ? m_setsPower : m_readPower;
}

/** \brief Update values setting (Current, Resistance, Power) according to selection mode. Sanity checking is also performed.
 *
 * \param v float : <b> updated value </b>
 * \param mode SelectionMode_t : <b> selection mode (CURRENT, RESISTANCE, POWER) </b>
 * \return void
 *
 */
void aDCSettings::updateValuesFromMode(float v, SelectionMode_t mode)
{
    float voltage = getVoltage();
    float currentSets = getCurrent(aDCSettings::OPERATION_MODE_SET);

    switch (mode)
    {
        case SELECTION_MODE_CURRENT:
            switch (setCurrent(v, aDCSettings::OPERATION_MODE_SET))
            {
                case aDCSettings::SETTING_ERROR_UNDERSIZED:
                    setCurrent(0.0, aDCSettings::OPERATION_MODE_SET);
                    setEncoderPosition(0);
                    break;

                case aDCSettings::SETTING_ERROR_OVERSIZED:
                    setCurrent(CURRENT_MAXIMUM, aDCSettings::OPERATION_MODE_SET);
                    setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(CURRENT_MAXIMUM) * 500.000)));
                    break;

                default:
                    break;
            }

            currentSets = getCurrent(aDCSettings::OPERATION_MODE_SET);

            switch (setPower(floatRounding(floatRounding(voltage) * floatRounding(currentSets)), aDCSettings::OPERATION_MODE_SET))
            {
                case aDCSettings::SETTING_ERROR_OVERSIZED:
                    setCurrent(floatRounding(floatRounding(POWER_MAXIMUM) / floatRounding(voltage)), aDCSettings::OPERATION_MODE_SET);
                    currentSets = getCurrent(aDCSettings::OPERATION_MODE_SET);
#ifdef RESISTANCE
                    setResistance((currentSets > 0.0) ? floatRounding(floatRounding(POWER_MAXIMUM) / floatRounding(floatRounding(getCurrent(aDCSettings::OPERATION_MODE_SET)) * floatRounding(getCurrent(aDCSettings::OPERATION_MODE_SET)))) : -1, OPERATION_MODE_SET);
#endif
                    setPower(floatRounding(floatRounding(voltage) * floatRounding(currentSets)), aDCSettings::OPERATION_MODE_SET);
                    setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(currentSets) * 500.000)));
                    break;

                case aDCSettings::SETTING_ERROR_UNDERSIZED:
                    break;

                case aDCSettings::SETTING_ERROR_VALID:
#ifdef RESISTANCE
                    currentSets = getCurrent(aDCSettings::OPERATION_MODE_SET);
                    setResistance((currentSets > 0.0) ? floatRounding(floatRounding(voltage) / floatRounding(currentSets)) : 0, OPERATION_MODE_SET);
#endif
                    break;
            }
            break;

#ifdef RESISTANCE
        case SELECTION_MODE_RESISTANCE:
            if (voltage == 0.0)
            {
                setEncoderPosition(0);
                break;
            }

            switch (setResistance(v, aDCSettings::OPERATION_MODE_SET))
            {
                case aDCSettings::SETTING_ERROR_UNDERSIZED:
                    setResistance(0.0, aDCSettings::OPERATION_MODE_SET);
                    setEncoderPosition(0);
                    break;

                default:
                    break;
            }

            switch (setCurrent((getResistance(aDCSettings::OPERATION_MODE_SET) > 0.0) ? floatRounding(floatRounding(voltage) / floatRounding(getResistance(OPERATION_MODE_SET))) : 0, aDCSettings::OPERATION_MODE_SET))
            {
                case aDCSettings::SETTING_ERROR_OVERSIZED:
                    setCurrent(CURRENT_MAXIMUM, aDCSettings::OPERATION_MODE_SET);
                    setResistance(floatRounding(floatRounding(voltage) / floatRounding(CURRENT_MAXIMUM)), aDCSettings::OPERATION_MODE_SET);
                    setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(getResistance(OPERATION_MODE_SET)) * 1000.000)));
                    break;

                case aDCSettings::SETTING_ERROR_UNDERSIZED:
                    break;

                default:
                    break;
            }

            currentSets = getCurrent(aDCSettings::OPERATION_MODE_SET);

            switch (setPower(floatRounding(floatRounding(voltage) * floatRounding(currentSets)), aDCSettings::OPERATION_MODE_SET))
            {
                case aDCSettings::SETTING_ERROR_OVERSIZED:
                    setCurrent(floatRounding(floatRounding(POWER_MAXIMUM) / floatRounding(voltage)), aDCSettings::OPERATION_MODE_SET);
                    currentSets = getCurrent(aDCSettings::OPERATION_MODE_SET);
                    setResistance((currentSets > 0.0) ? floatRounding(floatRounding(POWER_MAXIMUM) / floatRounding(floatRounding(currentSets) * floatRounding(currentSets))) : 0, OPERATION_MODE_SET);
                    setPower(floatRounding(floatRounding(voltage) * floatRounding(currentSets)), aDCSettings::OPERATION_MODE_SET);
                    setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(getResistance(aDCSettings::OPERATION_MODE_SET)) * 1000.000)));
                    break;

                case aDCSettings::SETTING_ERROR_UNDERSIZED:
                    break;

                default:
                    break;
            }
            break;
#endif

        case SELECTION_MODE_POWER:
            if (voltage == 0.0)
            {
                setEncoderPosition(0);
                break;
            }

            switch (setPower(v, aDCSettings::OPERATION_MODE_SET))
            {
                case aDCSettings::SETTING_ERROR_OVERSIZED:
                    setPower(POWER_MAXIMUM, aDCSettings::OPERATION_MODE_SET);
                    setCurrent(floatRounding(floatRounding(getPower(aDCSettings::OPERATION_MODE_SET)) / floatRounding(voltage)), aDCSettings::OPERATION_MODE_SET);
#ifdef RESISTANCE
                    currentSets = getCurrent(aDCSettings::OPERATION_MODE_SET);
                    setResistance((currentSets > 0.0) ? floatRounding(floatRounding(POWER_MAXIMUM) / floatRounding(floatRounding(currentSets) * floatRounding(currentSets))) : 0, aDCSettings::OPERATION_MODE_SET);
#endif
                    setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(getPower(aDCSettings::OPERATION_MODE_SET)) * 1000.000)));
                    break;

                case aDCSettings::SETTING_ERROR_UNDERSIZED:
                    setPower(0.0, aDCSettings::OPERATION_MODE_SET);
                    setEncoderPosition(0);

                default:
                    setCurrent(floatRounding(floatRounding(getPower(aDCSettings::OPERATION_MODE_SET)) / floatRounding(voltage)), aDCSettings::OPERATION_MODE_SET);
#ifdef RESISTANCE
                    currentSets = getCurrent(aDCSettings::OPERATION_MODE_SET);
                    setResistance((currentSets > 0.0) ? floatRounding(floatRounding(voltage) / floatRounding(currentSets)) : 0.0, aDCSettings::OPERATION_MODE_SET);
#endif
                    break;
            }
            break;

        default:
            break;
    }
}

// Temperature
/** \brief Temperature readed setter
 *
 * \param v uint16_t : <b> Temperature </b>
 * \return void
 *
 */
void aDCSettings::setTemperature(uint16_t v)
{
    uint16_t p = m_readTemperature;
    m_readTemperature = v;

    _enableDataCheck(DATA_TEMPERATURE, (p != m_readTemperature));
}

/** \brief Temperature readed getter
 *
 * \return uint16_t : <b> Temperature </b>
 *
 */
uint16_t aDCSettings::getTemperature()
{
    return m_readTemperature;
}

// Fan
/** \brief Fan speed setter
 *
 * \param v uint16_t : <b> DAC speed value </b>
 * \return void
 *
 */
void aDCSettings::setFanSpeed(uint16_t v)
{
    m_fanSpeed = v;
}

/** \brief Fan speed getter
 *
 * \return uint16_t : <b> DAC speed value </b>
 *
 */
uint16_t aDCSettings::getFanSpeed()
{
    return m_fanSpeed;
}

// Selection Mode
/** \brief Selection mode setter
 *
 * \param m SelectionMode_t : <b> new selection mode </b>
 * \param force bool : <b> force the bit setting in m_datas to be set (default : false) </b>
 * \return void
 *
 */
void aDCSettings::setSelectionMode(SelectionMode_t m, bool force)
{
    SelectionMode_t p = m_mode;
    m_mode = m;

    _enableDataCheck(DATA_SELECTION, (force || (p != m_mode)));
}

/** \brief Selection mode getter
 *
 * \return SelectionMode_t : <b> current selection mode </b>
 *
 */
aDCSettings::SelectionMode_t aDCSettings::getSelectionMode()
{
    return m_mode;
}

/** \brief Get the next selection mode, according to "origin", if any provided.
 *
 * \param origin SelectionMode_t : <b> origin starter selection mode </b>
 * \param next bool : <b> Next or Previous mode </b>
 * \return SelectionMode_t : <b> next selection mode </b>
 *
 */
aDCSettings::SelectionMode_t aDCSettings::getPrevNextMode(aDCSettings::SelectionMode_t origin, bool next)
{
    if (m_dispMode == DISPLAY_MODE_SETUP)
        return (static_cast<SelectionMode_t>(((origin != SELECTION_MODE_UNKNOWN) ? !origin : !m_mode)));

    uint8_t m = static_cast<uint8_t>(origin != SELECTION_MODE_UNKNOWN ? origin : m_mode);

    if (next)
    {
        if ((static_cast<uint8_t>(origin != SELECTION_MODE_UNKNOWN ? origin : m_mode) + 1) < static_cast<uint8_t>(SELECTION_MODE_UNKNOWN))
            return static_cast<SelectionMode_t>(m + 1);
    }
    else
    {
        if ((static_cast<uint8_t>(origin != SELECTION_MODE_UNKNOWN ? origin : m_mode) - 1) >= static_cast<uint8_t>(SELECTION_MODE_CURRENT))
            return static_cast<SelectionMode_t>(m - 1);
    }

    return (next ? SELECTION_MODE_CURRENT : SELECTION_MODE_POWER);
}

// Display Mode
/** \brief Display mode setter
 *
 * \param d DisplayMode_t : <b> display mode </b>
 * \return void
 *
 */
void aDCSettings::setDisplayMode(DisplayMode_t d)
{
    DisplayMode_t p = m_dispMode;
    m_dispMode = d;

    _enableDataCheck(DATA_DISPLAY, (p != m_dispMode));
}

/** \brief Display mode getter
 *
 * \return DisplayMode_t : <b> Display mode </b>
 *
 */
aDCSettings::DisplayMode_t aDCSettings::getDisplayMode()
{
    return m_dispMode;
}

// Encoder
/** \brief Encoder position setter
 *
 * \param v int32_t : <b> encoder position </b>
 * \return void
 *
 */
void aDCSettings::setEncoderPosition(int32_t v)
{
    int32_t p = m_encoderPos;
    m_encoderPos = v;

    _enableDataCheck(DATA_ENCODER, (p != m_encoderPos));

    pingAutolock();
}

/** \brief Increment stored encoder position by "p" (default = 1)
 *
 * \param v int32_t : <b> increment value, 1 by default </b>
 * \return void
 *
 */
void aDCSettings::incEncoderPosition(int32_t v)
{
    int32_t p = m_encoderPos;
    m_encoderPos += v;

    _enableDataCheck(DATA_ENCODER, (p != m_encoderPos));

    pingAutolock();
}

/** \brief Encoder position getter
 *
 * \return int32_t : <b> encoder position </b>
 *
 */
int32_t aDCSettings::getEncoderPosition()
{
    return m_encoderPos;
}

// Operation Mode
/** \brief Operation mode setter
 *
 * \param m OperationMode_t : <b> new operation mode </b>
 * \return void
 *
 */
void aDCSettings::setOperationMode(OperationMode_t m)
{
    OperationMode_t p = m_operationMode;
    m_operationMode = m;

    _enableDataCheck(DATA_OPERATION, (p != m_operationMode));

    m_operationTick = (m_operationMode == OPERATION_MODE_SET) ? millis() : 0;
}

/** \brief Operation mode getter
 *
 * \return OperationMode_t : <b> operation mode </b>
 *
 */
aDCSettings::OperationMode_t aDCSettings::getOperationMode()
{
    return m_operationMode;
}

/** \brief Automatic timeouted toggle between OPERATION_SET and OPERATION_READ
 *
 * \return void
 *
 */
void aDCSettings::updateOperationMode()
{
    if (m_operationMode == OPERATION_MODE_SET)
    {
        if ((millis() - m_operationTick) > OPERATION_SET_TIMEOUT)
            setOperationMode(OPERATION_MODE_READ);
    }
}

/** \brief Reset timeout while in OPERATION_SET mode
 *
 * \return void
 *
 */
void aDCSettings::pingOperationMode()
{
    if (m_operationMode == OPERATION_MODE_SET)
        m_operationTick = millis();
}

// Autolock
/** \brief Reset autolock timeout
 *
 * \return void
 *
 */
void aDCSettings::pingAutolock()
{
    m_lockTick = millis();
}

/** \brief Check if autolock is enabled and performs.
 *
 * \return bool
 *
 */
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
/** \brief Helper function to manage bit-field features
 *
 * \param feature uint16_t : <b> FEATURE to enable/disable </b>
 * \param enable bool : <b> FEATURE enability (default = enable) </b>
 * \return void
 *
 */
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

/** \brief Check if FEATURE is enabled
 *
 * \param feature uint16_t : <b> FEATURE to check against </b>
 * \return bool
 *
 */
bool aDCSettings::isFeatureEnabled(uint16_t feature)
{
    return (m_features & feature);
}

/**
*** EEPROM functions
**/
//
// Check magic numbers in EEPROM
//
/** \brief Check for EEPROM magic numbers
 *
 * \return bool
 *
 * Used to check if some data has already been wrote in the EEPROM.
 */
bool aDCSettings::_eepromCheckMagic()
{
    return ((EEPROM.read(EEPROM_ADDR_MAGIC) == 0xD) && (EEPROM.read(EEPROM_ADDR_MAGIC + 1) == 0xE) &&
            (EEPROM.read(EEPROM_ADDR_MAGIC + 2) == 0xA) && (EEPROM.read(EEPROM_ADDR_MAGIC + 3) == 0xD));
}
//
// Write magic numbers in EEPROM
//
/** \brief Write magic numbers into EEPROM
 *
 * \return void
 *
 */
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
/** \brief Reset all stored parameters into EEPROM
 *
 * \return void
 *
 */
void aDCSettings::_eepromReset()
{
    _eepromWriteMagic();

    EEPROM.write(EEPROM_ADDR_AUTODIM, 1);
    EEPROM.write(EEPROM_ADDR_AUTOLOCK, 1);
}
//
// Restore config from EEPROM
//
/** \brief Restore value from EEPROM
 *
 * \return void
 *
 */
void aDCSettings::_eepromRestore()
{
    if (!_eepromCheckMagic())
        _eepromReset();

    enableFeature(FEATURE_AUTODIM, (EEPROM.read(EEPROM_ADDR_AUTODIM) == 1));
    enableFeature(FEATURE_AUTOLOCK, (EEPROM.read(EEPROM_ADDR_AUTOLOCK) == 1));
}

/** \brief Enable a bit in the m_datas bit-field storage
 *
 * \param bit uint16_t : <b> Bit to set </b>
 * \param enable bool : <b> Bit enability </b>
 * \return void
 *
 */
void aDCSettings::_enableData(uint16_t bit, bool enable)
{
    if (enable)
        m_datas |= bit;
    else
        m_datas &= (0xFFFF ^ bit);

}

/** \brief Enable a bit in the m_datas bit-field storage, checking for previous state.
 *
 * If the bit is already sets to TRUE, we don't touch his state, syncData() should be called for this.
 *
 * \param bit uint16_t : <b> Bit to set </b>
 * \param enable bool : <b> Bit enability </b>
 * \return void
 *
 */
void aDCSettings::_enableDataCheck(uint16_t bit, bool enable)
{
    if (m_datas & bit)
        return;

    if (enable)
        m_datas |= bit;
    else
        m_datas &= (0xFFFF ^ bit);
}

/** \brief Get bit enability in m_datas bit-field storage
 *
 * \param bit uint16_t : <b> Bit to check </b>
 * \return bool
 *
 */
bool aDCSettings::isDataEnabled(uint16_t bit)
{
    return (m_datas & bit);
}

/** \brief Clear a bit in m_datas bit-field storage
 *
 * \param bit uint16_t
 * \return void
 *
 */
void aDCSettings::syncData(uint16_t bit)
{
    _enableData(bit, false);
}

/** \brief Constructor
 */
aStepper::aStepper() : m_inc(0), m_incPrev(255)
{
}

/** \brief Destructor
 */
aStepper::~aStepper()
{
}

/** \brief Increment value, check for boundaries.
 *
 * \return void
 *
 */
void aStepper::incIncrement()
{
    if((m_inc + 1) <= MAX_VALUE)
        m_inc++;
    else
        m_inc = 0;
}

/** \brief Value getter
 *
 * \return uint8_t
 *
 */
uint8_t aStepper::incGetValue()
{
    return m_inc;
}

/** \brief Reset value
 *
 * \return void
 *
 */
void aStepper::incReset()
{
    m_inc = 0;
}

/** \brief Value getter, according to multiple.
 *
 * \return int16_t
 *
 */
int16_t aStepper::incGetMult()
{
    return (_pow(10, m_inc));
}

/** \brief Get value according to selection mode
 *
 * \param mode uint16_t : <b> Selection mode (will be typecasted to aDCSettings::SelectionMode_t)</b>
 * \return int16_t
 *
 */
int16_t aStepper::incGetValueFromMode(uint8_t mode)
{
    switch (static_cast<aDCSettings::SelectionMode_t>(mode))
    {
        case aDCSettings::SELECTION_MODE_CURRENT:
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
                        break;
                }
                return 1;
            }
            break;

#ifdef RESISTANCE
        case aDCSettings::SELECTION_MODE_RESISTANCE:
#endif
        case aDCSettings::SELECTION_MODE_POWER:
            return incGetMult();
            break;

        default:
            break;
    }

    return m_inc;
}

/** \brief Synchronize value
 *
 * \return bool
 *
 */
bool aStepper::incIsSynced()
{
    return (m_inc == m_incPrev);
}

/** \brief Check if value is synchronized
 *
 * \return void
 *
 */
void aStepper::incSync()
{
    m_incPrev = m_inc;
}

/** \brief Small implementation of pow() math function
 *
 * \param base int : <b> base radix </b>
 * \param exp int : <b> exponent value </b>
 * \return int16_t : <b> result </b>
 *
 */
inline int16_t aStepper::_pow(int base, int exp)
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
/** \brief Constructor
 *
 * \param rs uint8_t : <b> LCD RS pin </b>
 * \param enable uint8_t : <b> LCD ENABLE pin </b>
 * \param d0 uint8_t : <b> LCD d0 pin </b>
 * \param d1 uint8_t : <b> LCD d1 pin </b>
 * \param d2 uint8_t : <b> LCD d2 pin </b>
 * \param d3 uint8_t : <b> LCD d3 pin </b>
 * \param cols uint8_t : <b> LCD columns number </b>
 * \param rows uint8_t : <b> LCD rows number </b>
 *
 */
aLCD::aLCD(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t cols, uint8_t rows) :
        LiquidCrystal(rs, enable, d0, d1, d2, d3),
        m_cols(0), m_rows(0), m_curCol(0), m_curRow(0)
{
    begin(cols, rows);
}

/** \brief Destructor
 */
aLCD::~aLCD()
{
}

/** \brief Initialize LCD screen size
 *
 * \param cols uint8_t : <b> Columns </b>
 * \param lines uint8_t : <b> Rows </b>
 * \return void
 *
 */
void aLCD::begin(uint8_t cols, uint8_t lines)
{
    m_cols = cols;
    m_rows = lines;
    LiquidCrystal::begin(m_cols, m_rows);
}

/** \brief Set cursor positon
 *
 * \param col uint8_t : <b> Column </b>
 * \param row uint8_t : <b> Row </b>
 * \return void
 *
 */
void aLCD::setCursor(uint8_t col, uint8_t row)
{
    m_curCol = col;
    m_curRow = row;
    LiquidCrystal::setCursor(col, row);
}

/** \brief Print centered string to current row
 *
 * \param str const char* : <b> String to display </b>
 * \return void
 *
 */
void aLCD::printCenter(const char *str)
{
    if (str)
    {
        uint8_t len = strlen(str);

        // Scrolling is disabled due to memory footprint
        uint8_t x = 0;
        if (len <= m_cols)
            x = (m_cols - len) >> 1;

        setCursor(x, m_curRow);

        LiquidCrystal::print(str);
        m_curCol = x + strlen(str);
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

/** \brief Print centered string to current row
 *
 * \param ifsh const __FlashStringHelper* : <b> string to display </b>
 * \return void
 *
 */
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

/** \brief Clear displayed value, from given row, stopping at value end field - destMinus
 *
 * \param row uint8_t : <b> field row </b>
 * \param destMinus int : <b> minus end field position (default = 0) </b>
 * \return void
 *
 */
void aLCD::clearValue(uint8_t row, int destMinus)
{
    setCursor(OFFSET_VALUE, row);
    for (uint8_t i = OFFSET_VALUE; i < OFFSET_MARKER_RIGHT + destMinus; i++)
        LiquidCrystal::write(char(0x20));
}

/**
*** Class to manage display output
**/
/** \brief Constructor
 *
 * \param parent aDCEngine* : <b> Parent engine </b>
 * \param rs uint8_t : <b> LCD RS pin </b>
 * \param enable uint8_t : <b> LCD ENABLE pin </b>
 * \param d0 uint8_t : <b> LCD d0 pin </b>
 * \param d1 uint8_t : <b> LCD d1 pin </b>
 * \param d2 uint8_t : <b> LCD d2 pin </b>
 * \param d3 uint8_t : <b> LCD d3 pin </b>
 * \param cols uint8_t : <b> LCD columns </b>
 * \param rows uint8_t : <b> LCD rows </b>
 *
 */
aDCDisplay::aDCDisplay(aDCEngine *parent, uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t cols, uint8_t rows) :
        aLCD(rs, enable, d0, d1, d2, d3, cols, rows),
        m_Parent(parent), m_dimmed(false), m_dimmerTick(0)
{
}

/** \brief Destructor
 */
aDCDisplay::~aDCDisplay()
{
}

/** \brief Setup function. Should be called before any other member.
 *
 * \return void
 *
 */
void aDCDisplay::setup()
{
    static const uint8_t _glyphs[8][8] =
    {
        { // . ..1
            B00000,
            B00000,
            B00001,
            B00001,
            B00001,
            B10101,
            B00000,
            B00000
        },
        { // . .1.
            B00000,
            B00000,
            B00100,
            B00100,
            B00100,
            B10101,
            B00000,
            B00000
        },
        { // . 1..
            B00000,
            B00000,
            B10000,
            B10000,
            B10000,
            B10101,
            B00000,
            B00000
        },
        { // . 1k
            B00000,
            B00000,
            B10000,
            B10000,
            B10000,
            B10101,
            B00110,
            B00101
        },
        { // USB
            B00100,
            B01110,
            B00101,
            B10101,
            B10110,
            B01100,
            B00100,
            B01110
        },
        { // LOCK
            B00110,
            B00100,
            B00110,
            B00100,
            B00100,
            B01110,
            B10001,
            B01110
        },
        { // CHECKBOX UNTICKED
            B10101,
            B00000,
            B10001,
            B00000,
            B10001,
            B00000,
            B10101,
            B00000
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

    pinMode(LED_BACKLIGHT_PIN, OUTPUT);

    // set the LCD Backlight high
    digitalWrite(LED_BACKLIGHT_PIN, HIGH);

    for (uint8_t i = 0; i < sizeof(_glyphs) / sizeof(_glyphs[0]); i++)
        aLCD::createChar(i, (uint8_t *)_glyphs[i]);
}

/** \brief Display small banner
 *
 * \return void
 *
 */
void aDCDisplay::showBanner()
{
    aLCD::clear();
    aLCD::setCursor(0, 1);
    aLCD::printCenter("DC Electronic Load");

    char buffer[LCD_COLS_NUM + 1];
    snprintf(buffer, LCD_COLS_NUM, "Version %d.%d", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR);

    aLCD::setCursor(0, 3);
    aLCD::printCenter(buffer);

    delay(3000);
}

/** \brief Update displayed field according to operation mode
 *
 * \param opMode OperationMode_t : <b> Operation mode </b>
 * \param vSet float : <b> Settings value </b>
 * \param vRead float : <b> Readed value </b>
 * \param row uint8_t : <b> LCD row position </b>
 * \param unit uint8_t : <b> Unit character </b>
 * \return void
 *
 */
void aDCDisplay::updateField(aDCSettings::OperationMode_t opMode, float vSet, float vRead, uint8_t row, uint8_t unit)
{
    aLCD::clearValue(row);
    aLCD::setCursor(((opMode == aDCSettings::OPERATION_MODE_READ) ? OFFSET_VALUE : (OFFSET_MARKER_RIGHT - getNumericalLength(vSet)) - 1), row);
    aLCD::print((opMode == aDCSettings::OPERATION_MODE_READ) ? vRead : vSet, 3);
    aLCD::print(char(unit));
}

/** \brief Update LCD display management function
 *
 * Draw/Redraw on screen datas
 *
 * \return void
 *
 */
void aDCDisplay::updateDisplay()
{
    aDCSettings                    *d           = (aDCSettings *)m_Parent->_getSettings();
    bool                            fullRedraw  = false;
    aDCSettings::OperationMode_t    opMode;

    switch (d->getDisplayMode())
    {
        case aDCSettings::DISPLAY_MODE_VALUES:
            if (d->isDataEnabled(aDCSettings::DATA_DISPLAY))
            {
                fullRedraw = true;

                aLCD::clear();
                aLCD::setCursor(OFFSET_UNIT, 0);
                aLCD::print("U: ");
                aLCD::setCursor(OFFSET_UNIT + OFFSET_TEMP, 0);
                aLCD::print(char(0xDF)); // °
                aLCD::print("C: ");
                aLCD::setCursor(OFFSET_UNIT, aDCSettings::SELECTION_MODE_CURRENT + 1);
                aLCD::print("I: ");
#ifdef RESISTANCE
                aLCD::setCursor(OFFSET_UNIT, aDCSettings::SELECTION_MODE_RESISTANCE + 1);
                aLCD::print("R: ");
#endif
                aLCD::setCursor(OFFSET_UNIT, aDCSettings::SELECTION_MODE_POWER + 1);
                aLCD::print("P: ");
            }

            if (d->isDataEnabled(aDCSettings::DATA_VOLTAGE) || fullRedraw)
            {
                aLCD::clearValue(0, -2);
                aLCD::setCursor(OFFSET_VALUE, 0);
                aLCD::print(d->getVoltage(), 3);
                aLCD::print('V');
                d->syncData(aDCSettings::DATA_VOLTAGE);
            }

            if (d->isDataEnabled(aDCSettings::DATA_TEMPERATURE) || fullRedraw)
            {
                aLCD::setCursor(OFFSET_VALUE + OFFSET_TEMP + 1, 0);
                aLCD::print("   ");
                aLCD::setCursor(OFFSET_VALUE + OFFSET_TEMP + 1, 0);
                aLCD::print(d->getTemperature(), DEC);
                d->syncData(aDCSettings::DATA_TEMPERATURE);
            }

            // Display Current Set/Read
            opMode = d->getOperationMode();

            if (d->isDataEnabled(aDCSettings::DATA_OPERATION))
            {
                fullRedraw = true;
                d->syncData(aDCSettings::DATA_OPERATION);
            }

            if (d->isDataEnabled((opMode == aDCSettings::OPERATION_MODE_READ) ? aDCSettings::DATA_CURRENT_READ : aDCSettings::DATA_CURRENT_SETS) || fullRedraw)
            {
                updateField(opMode, d->getCurrent(aDCSettings::OPERATION_MODE_SET), d->getCurrent(aDCSettings::OPERATION_MODE_READ), aDCSettings::SELECTION_MODE_CURRENT + 1, 'A');
                d->syncData((opMode == aDCSettings::OPERATION_MODE_READ) ? aDCSettings::DATA_CURRENT_READ : aDCSettings::DATA_CURRENT_SETS);
            }

#ifdef RESISTANCE
            if (d->isDataEnabled((opMode == aDCSettings::OPERATION_MODE_READ) ? aDCSettings::DATA_RESISTANCE_READ : aDCSettings::DATA_RESISTANCE_SETS) || fullRedraw)
            {
                updateField(opMode, d->getResistance(aDCSettings::OPERATION_MODE_SET), d->getResistance(aDCSettings::OPERATION_MODE_READ), aDCSettings::SELECTION_MODE_RESISTANCE + 1, char(0xF4));
                d->syncData((opMode == aDCSettings::OPERATION_MODE_READ) ? aDCSettings::DATA_RESISTANCE_READ : aDCSettings::DATA_RESISTANCE_SETS);
            }
#endif // RESISTANCE

            if (d->isDataEnabled((opMode == aDCSettings::OPERATION_MODE_READ) ? aDCSettings::DATA_POWER_READ : aDCSettings::DATA_POWER_SETS) || fullRedraw)
            {
                updateField(opMode, d->getPower(aDCSettings::OPERATION_MODE_SET), d->getPower(aDCSettings::OPERATION_MODE_READ), aDCSettings::SELECTION_MODE_POWER + 1, 'W');
                d->syncData((opMode == aDCSettings::OPERATION_MODE_READ) ? aDCSettings::DATA_POWER_READ : aDCSettings::DATA_POWER_SETS);
            }

            if (d->isDataEnabled(aDCSettings::DATA_SELECTION) || d->isDataEnabled(aDCSettings::DATA_DISPLAY) || fullRedraw)
            {
                uint8_t mode = static_cast<uint8_t>(d->getSelectionMode()) + 1;
                uint8_t prevMode = static_cast<uint8_t>(d->getPrevNextMode(static_cast<aDCSettings::SelectionMode_t>(mode - 1), false)) + 1;

                // Clear previous mode selection markers
                aLCD::setCursor(OFFSET_MARKER_LEFT, prevMode);
                aLCD::print(' ');
                aLCD::setCursor(OFFSET_MARKER_RIGHT, prevMode);
                aLCD::print("  "); // Marker + stepper

                // Force stepper redraw
                fullRedraw = true;

                aLCD::setCursor(OFFSET_MARKER_LEFT, mode);
                aLCD::print('[');
                aLCD::setCursor(OFFSET_MARKER_RIGHT, mode);
                aLCD::print(']');

                d->syncData(aDCSettings::DATA_SELECTION);
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
                aLCD::print("OV");
            }
            else if (!d->isFeatureEnabled(FEATURE_OVP) && (d->isFeatureEnabled(FEATURE_OVP_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_OVP_VISIBLE, false);

                aLCD::setCursor(ALARM_OV_X_COORD, ALARM_OV_Y_COORD);
                aLCD::print("  ");
            }

            // OCP
            if (d->isFeatureEnabled(FEATURE_OCP) && (!d->isFeatureEnabled(FEATURE_OCP_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_OCP_VISIBLE);

                aLCD::setCursor(ALARM_OC_X_COORD, ALARM_OC_Y_COORD);
                aLCD::print("OC");
            }
            else if (!d->isFeatureEnabled(FEATURE_OCP) && (d->isFeatureEnabled(FEATURE_OCP_VISIBLE) || fullRedraw))
            {
                d->enableFeature(FEATURE_OCP_VISIBLE, false);

                aLCD::setCursor(ALARM_OC_X_COORD, ALARM_OC_Y_COORD);
                aLCD::print("  ");
            }
            break;

      case aDCSettings::DISPLAY_MODE_SETUP:
            pingBacklight();
            d->pingAutolock();
            if (d->isDataEnabled(aDCSettings::DATA_DISPLAY))
            {
                aLCD::clear();
                aLCD::setCursor(0, 0);
                aLCD::print("Options:");
                d->syncData(aDCSettings::DATA_DISPLAY);

                aLCD::setCursor(0, 1);
                aLCD::printCenter("Auto Dimmer");
                aLCD::setCursor(0, 2);
                aLCD::printCenter("Auto Lock");
            }

            if (d->isDataEnabled(aDCSettings::DATA_SELECTION) || d->isDataEnabled(aDCSettings::DATA_DISPLAY) || fullRedraw)
            {
                uint8_t mode = static_cast<uint8_t>(d->getSelectionMode()) + 1;
                uint8_t prevMode = static_cast<uint8_t>(d->getPrevNextMode(static_cast<aDCSettings::SelectionMode_t>(mode - 1), false)) + 1;

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

                d->syncData(aDCSettings::DATA_SELECTION);
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

/** \brief Dim/undim backlight helper function
 *
 * \param up bool : <b> dimmer direction </b>
 * \return void
 *
 */
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

/** \brief Backlight dimming
 *
 * \return void
 *
 */
void aDCDisplay::_dimmingBacklight()
{
    _dimBacklight(false);
}

/** \brief Backlight waker
 *
 * \return void
 *
 */
void aDCDisplay::_wakeupBacklight()
{
    _dimBacklight(true);
}

/** \brief Reset backlight dimmer.
 *
 * \return void
 *
 */
void aDCDisplay::pingBacklight()
{
    m_dimmerTick = millis();

    if (m_dimmed)
        _wakeupBacklight();
}

/** \brief Check if backlight is dimmed
 *
 * \return bool
 *
 */
bool aDCDisplay::isBacklightDimmed()
{
    return m_dimmed;
}

/**
*** Class to manage settings
**/
/** \brief
 *
 * \param rs uint8_t : <b> LCD RS pin </b>
 * \param enable uint8_t : <b> LCD ENABLE pin </b>
 * \param d0 uint8_t : <b> LCD d0 pin </b>
 * \param d1 uint8_t : <b> LCD d1 pin </b>
 * \param d2 uint8_t : <b> LCD d2 pin </b>
 * \param d3 uint8_t : <b> LCD d3 pin </b>
 * \param cols uint8_t : <b> LCD Columns number </b>
 * \param rows uint8_t : <b> LCD Rows number </b>
 * \param enca uint8_t : <b> Encoder A pin </b>
 * \param encb uint8_t : <b> Encoder B pin </b>
 * \param encpb uint8_t : <b> Encoder push button bin </b>
 * \param encsteps uint8_t : <b> Encoder steps per notch </b>
 *
 */
aDCEngine::aDCEngine(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t cols, uint8_t rows,
                     uint8_t enca, uint8_t encb, uint8_t encpb, uint8_t encsteps)
                     : aDCDisplay(this, rs, enable, d0, d1, d2, d3, cols, rows),
                       m_encoder(new ClickEncoder(enca, encb, encpb, encsteps)),
                       m_RXoffset(0)
{
    m_RXbuffer[0] = '\0';
}

/** \brief Destructor
 */
aDCEngine::~aDCEngine()
{
}

/** \brief Check and handle remote control and data logging
 *
 * \return void
 *
 */
void aDCEngine::_updateLoggingAndRemote()
{
#if 1
    if (Serial)
    {
        bool single = false;

        if (serialAvailable() > 0)
        {
            bool EOL = false;

            if (!m_Data.isFeatureEnabled(FEATURE_USB))
                m_Data.enableFeature(FEATURE_USB);

            while (serialAvailable() > 0)
            {
                m_RXbuffer[m_RXoffset] = serialRead();

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
                    /*
                     *** Format is: :CMD:<ARG>
                     ***         CMD is command
                     ***         ARG is optional argument
                     */

                    uint8_t *cmdStart, *cmdEnd;
                    if (((cmdStart = (uint8_t *)strchr((const char *)&m_RXbuffer[0], ':')) != NULL) &&
                        ((cmdEnd = (uint8_t *)strchr((const char *)cmdStart + 1, ':')) != NULL))
                    {
                        uint8_t cmd[(cmdEnd - cmdStart)];
                        uint8_t *arg = cmdEnd + 1;

                        memcpy(&cmd[0], cmdStart + 1, sizeof(cmd) - 1);
                        cmd[sizeof(cmd) - 1] = '\0';

                        serialPrint(':');

                        if (!strcmp((const char *)cmd, "ISET?")) // Get Current Settings
                        {
                            serialPrint(floatRounding(floatRounding(m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET)) * 1000.000), 0);
                            valid = true;
                        }
                        else if (!strcmp((const char *)cmd, "ISET")) // Current Setting
                        {
                            float v = atof((const char *)arg);
                            float nv = floatRounding(floatRounding(v) / 1000.000);

                            m_Data.updateValuesFromMode(nv, aDCSettings::SELECTION_MODE_CURRENT);

                            serialPrint(floatRounding(floatRounding(m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET)) * 1000.000), 0);

                            switch (m_Data.getSelectionMode())
                            {
                                case aDCSettings::SELECTION_MODE_CURRENT:
                                    m_Data.setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET)) * 500.000)));
                                    break;
#ifdef RESISTANCE
                                case aDCSettings::SELECTION_MODE_RESISTANCE:
                                    m_Data.setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(m_Data.getResistance(aDCSettings::OPERATION_MODE_SET)) * 1000.000)));
                                    break;
#endif
                                case aDCSettings::SELECTION_MODE_POWER:
                                    m_Data.setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(m_Data.getPower(aDCSettings::OPERATION_MODE_SET)) * 1000.000)));
                                    break;
                                default:
                                    break;
                            }

                            m_Data.syncData(aDCSettings::DATA_ENCODER);

                            valid = (m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET) == nv);
                        }
#ifdef SIMU
                        else if (!strcmp((const char *)cmd, "USET")) // Voltage Read Setting
                        {
                            float v = atof((const char *)arg);
                            if (m_Data.setVoltage(floatRounding(floatRounding(v) / 1000.000)) == aDCSettings::SETTING_ERROR_OVERSIZED)
                            {
                                m_Data.enableFeature(FEATURE_OVP);
                                m_Data.setCurrent(0.0, aDCSettings::OPERATION_MODE_SET);
                                m_Data.setEncoderPosition(0);
                                m_Data.syncData(aDCSettings::DATA_ENCODER);
                            }

                            serialPrint(floatRounding(floatRounding(m_Data.getVoltage()) * 1000.000), 0);
                            valid = true;
                        }
                        else if (!strcmp((const char *)cmd, "IS")) // Set Current Read
                        {
                            float v = atof((const char *)arg);
                            float nv = floatRounding(floatRounding(v) / 1000.000);

                            m_Data.setCurrent(nv, aDCSettings::OPERATION_MODE_READ);

                            serialPrint(floatRounding(floatRounding(m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ)) * 1000.000), 0);

                            valid = (m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ) == nv);
                        }
#endif
                        else if (!strcmp((const char *)cmd, "I?")) // Report Current Read
                        {
                            serialPrint(floatRounding(floatRounding(m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ)) * 1000.000), 0);
                            valid = true;
                        }
                        else if (!strcmp((const char *)cmd, "U?")) // Report Voltage Read
                        {
                            serialPrint(floatRounding(floatRounding(m_Data.getVoltage()) * 1000.000), 0);
                            valid = true;
                        }
                        else if (!strcmp((const char *)cmd, "L?")) // Logging Ask
                        {
                            serialPrint(m_Data.isFeatureEnabled(FEATURE_LOGGING) ? "ON" : "OFF");
                            valid = true;
                        }
                        else if (!strcmp((const char *)cmd, "L")) // Logging ON/OFF
                        {
                            if (!strcmp((const char *)arg, "OFF"))
                                m_Data.enableFeature(FEATURE_LOGGING, false);
                            else if (!strcmp((const char *)arg, "ON"))
                                m_Data.enableFeature(FEATURE_LOGGING, true);
                            else
                                single = true;

                            valid = true;
                        }
#ifdef SIMU
                        else if (!strcmp((const char *)cmd, "T")) // Temperature Read Setting
                        {
                            uint8_t v = static_cast<uint8_t>(atoi((const char *)arg));
                            m_Data.setTemperature(v);

                            serialPrint(int(m_Data.getTemperature()));
                            valid = true;
                        }
#endif
#if 0
                        else if (!strcmp((const char *)cmd, "?")) // Temperature Read Setting
                        {
                            serialPrintln("Usage :CMD:ARG");
                            serialPrintln("  CMD are:");
                            serialPrintln("    I?      Get current.");
                            serialPrintln("    I       Set max current.");
                            serialPrintln("    ISET    Set current setting.");
                            serialPrintln("    ISET?   Get current setting.");
                            serialPrintln("    U?      Get Voltage.");
                            serialPrintln("    L       Turn logging ON*, OFF* or SINGLE*.");
                            serialPrintln("    ?       This help.");
#ifdef SIMU
                            serialPrintln("IS,USET,T (simulation).");
#endif
                            serialPrintln("");
                            serialPrintln("* is ARG");
                            serialPrintln("returns value (is needed) + ':OK:' or ':ERR:'");
                            valid = true;
                        }
#endif
                        else
                            serialPrint("INVALID");
                    }

                    // Clear buffer for next command
                    m_RXbuffer[0] = '\0';
                    m_RXoffset = 0;
                }

                serialPrint(':');
                serialPrint(valid ? "OK" : "ERR");
                serialPrintln(':');
            }
        }

        // Logging
        /*
         *** Format is: timestamp,voltage,current,temperature
         ***         timestamp in hundred of milliseconds
         ***         voltage in mV
         ***         current in mA
         ***         temperature in Celcius
         */
        unsigned long m = millis();
        if ((m_Data.isFeatureEnabled(FEATURE_LOGGING) && !(m % 200)) || single)
        {
            serialPrint(m / 100);
            serialPrint(',');
            serialPrint(floatRounding(floatRounding(m_Data.getVoltage()) * 1000.000), 0);
            serialPrint(',');
            serialPrint(floatRounding(floatRounding(m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ)) * 1000.000), 0);
            serialPrint(',');
            serialPrint(int(m_Data.getTemperature()));
            serialPrintln();
        }

        serialFlush();
    }
    else
    {
        if (m_Data.isFeatureEnabled(FEATURE_LOGGING))
            m_Data.enableFeature(FEATURE_LOGGING, false);

        if (m_Data.isFeatureEnabled(FEATURE_USB))
            m_Data.enableFeature(FEATURE_USB, false);

    }
#endif
}

/** \brief Setup function, should be called before any other member
 *
 * \param isr ISRCallback : <b> callback function pointer that may call service() </b>
 * \return void
 *
 */
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
    Serial.begin(57600);
    serialFlush();
    // Flushing buffered char.
     if (Serial)
        while (serialAvailable() > 0)
            serialRead();
}

/** \brief Main loop function.
 *
 * \return void
 *
 */
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

        if ((v != 0) && (!m_Data.isFeatureEnabled(FEATURE_LOCKED)) && (m_Data.getDisplayMode() == aDCSettings::DISPLAY_MODE_VALUES))
        {
            int16_t mult = m_Data.incGetValueFromMode(m_Data.getSelectionMode());

            m_Data.incEncoderPosition((v * mult));
        }


        // Is USB control is enabled and the encoder has been rotated, turn OFF USB icon
        if (m_Data.isDataEnabled(aDCSettings::DATA_ENCODER))
        {
            // Is dimmed, wake up the backlight
            if (isBacklightDimmed())
            {
                m_Data.setEncoderPosition(oldPos);
                m_Data.syncData(aDCSettings::DATA_ENCODER);

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
            if (m_Data.getOperationMode() == aDCSettings::OPERATION_MODE_READ)
            {
                // Ignore new encoder position
                m_Data.setEncoderPosition(oldPos);
                m_Data.syncData(aDCSettings::DATA_ENCODER);
                // Turn on settings adjustments
                m_Data.setOperationMode(aDCSettings::OPERATION_MODE_SET);
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

            if (!m_Data.isFeatureEnabled(FEATURE_LOCKED) && (b == ClickEncoder::Held && !m_Data.isDataEnabled(aDCSettings::DATA_DISPLAY)))
            {
                do
                {
                    m_Data.pingAutolock();
                } while (m_encoder->getButton() != ClickEncoder::Released);

                m_Data.setSelectionMode(aDCSettings::SELECTION_MODE_CURRENT, true);
                m_Data.enableFeature(FEATURE_LOCKED, false);
                m_Data.setDisplayMode(static_cast<aDCSettings::DisplayMode_t>(!m_Data.getDisplayMode()));
            }

            // Increment encoder step (0.001, 0.01, 0.1, 1.0)
            if ((b == ClickEncoder::Clicked) && (m_Data.getDisplayMode() == aDCSettings::DISPLAY_MODE_VALUES))
                m_Data.incIncrement();

            // Change selection mode according to double click.
            _handleButtonEvent(b);
        }

        // Reads input voltage from the load source. ****MAXIMUM 24V INPUT****
        if (m_Data.setVoltage(_readInputVoltage()) == aDCSettings::SETTING_ERROR_OVERSIZED)
        {
            // Over-Voltage protection triggered
            m_Data.enableFeature(FEATURE_OVP);
            m_Data.setCurrent(0.0, aDCSettings::OPERATION_MODE_SET);
            m_Data.setEncoderPosition(0);
        }

        // Calculates and sets required load current. Accepts the mode variable which defines the mode of the unit,
        // ie. Constant current, resistance or power.
        _updateLoadCurrent();

        // Calculates heatsink temprature and sets fan speed accordingly.
        _updateFanSpeed();

        // Updates the LCD display. Accepts the lcdDisplay variable which defines if the values or menu is to be displayed.
        updateDisplay();

        if (m_Data.isDataEnabled(aDCSettings::DATA_DISPLAY))
            m_Data.syncData(aDCSettings::DATA_DISPLAY);

        if (m_Data.isDataEnabled(aDCSettings::DATA_ENCODER))
            m_Data.syncData(aDCSettings::DATA_ENCODER);

         // Data logging and USB controlling
        _updateLoggingAndRemote();
    }
}

/** \brief Function that handles button clicking
 *
 * \param button ClickEncoder::Button
 * \return void
 *
 */
void aDCEngine::_handleButtonEvent(ClickEncoder::Button button)
{
    switch (m_Data.getDisplayMode())
    {
        case aDCSettings::DISPLAY_MODE_VALUES:
            if (button == ClickEncoder::DoubleClicked)
            {
                m_Data.setSelectionMode(m_Data.getPrevNextMode());
                // Force redraw
                m_Data.incEncoderPosition(-1);

                switch (m_Data.getSelectionMode())
                {
                    case aDCSettings::SELECTION_MODE_CURRENT:
                        m_Data.setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET)) * 500.000)));
                        break;
#ifdef RESISTANCE
                    case aDCSettings::SELECTION_MODE_RESISTANCE:
                        m_Data.setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(m_Data.getResistance(aDCSettings::OPERATION_MODE_SET)) * 1000.000)));
                        break;
#endif
                    case aDCSettings::SELECTION_MODE_POWER:
                        m_Data.setEncoderPosition(static_cast<int32_t>(floatRounding(floatRounding(m_Data.getPower(aDCSettings::OPERATION_MODE_SET)) * 1000.000)));
                        break;
                    default:
                        break;
                }
            }
            break;

        case aDCSettings::DISPLAY_MODE_SETUP:
            switch (button)
            {
                case ClickEncoder::DoubleClicked:
                    m_Data.setSelectionMode(m_Data.getPrevNextMode());
                    break;

                case ClickEncoder::Clicked:
                    switch(m_Data.getSelectionMode())
                    {
                        case aDCSettings::SELECTION_MODE_CURRENT:
                            m_Data.enableFeature(FEATURE_AUTODIM, !m_Data.isFeatureEnabled(FEATURE_AUTODIM));
                            break;

#ifdef RESISTANCE
                        case aDCSettings::SELECTION_MODE_RESISTANCE:
#else
                        case aDCSettings::SELECTION_MODE_POWER:
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

/** \brief Caller callback function that manages encoder clicking and so on.
 *
 * \return void
 *
 */
void aDCEngine::service()
{
    m_encoder->service();
}

/** \brief Function to read the input voltage and return a float number represention volts.
 *
 * \return float : <b> readed input voltage </b>
 *
 */
float aDCEngine::_readInputVoltage()
{
#ifdef SIMU
    float v = m_Data.getVoltage();// 1.000;
#else
    float v = (_readADC(ADC_INPUTVOLTAGE_CHAN)) * 12.03;
#endif
    return (v < 0.018 ? 0 : v);
}

/** \brief Function to measure the actual load current.
 *
 * \return float : <b> readed current </b>
 *
 */
float aDCEngine::_readMeasuredCurrent()
{
#ifdef SIMU
    return m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ);
#else
    return ((_readADC(ADC_MEASUREDCURRENT_CHAN)) / 0.1000);
#endif // SIMU
}

/** \brief Function to calculate and set the required load current. Accepts the mode variable to determine if the constant current, resistance or power mode is to be used.
 *
 * \return void
 *
 */
void aDCEngine::_updateLoadCurrent()
{
    bool encoderChanged = m_Data.isDataEnabled(aDCSettings::DATA_ENCODER);

    if ((m_Data.getDisplayMode() == aDCSettings::DISPLAY_MODE_VALUES) && encoderChanged)
    {
        switch (m_Data.getSelectionMode())
        {
            case aDCSettings::SELECTION_MODE_CURRENT:
                m_Data.updateValuesFromMode(m_Data.getEncoderPosition() / 500.000, aDCSettings::SELECTION_MODE_CURRENT);
                //m_Data.updateValuesFromMode(static_cast<float>(floatRounding(floatRounding(float(m_Data.getEncoderPosition())) / 500.000)), aDCSettings::SELECTION_MODE_CURRENT);
                break;

#ifdef RESISTANCE
            case aDCSettings::SELECTION_MODE_RESISTANCE:
                m_Data.updateValuesFromMode(m_Data.getEncoderPosition() / 1000.000, aDCSettings::SELECTION_MODE_RESISTANCE);
                //m_Data.updateValuesFromMode(static_cast<float>(floatRounding(floatRounding(float(m_Data.getEncoderPosition())) / 1000.000)), aDCSettings::SELECTION_MODE_RESISTANCE);
                break;
#endif
            case aDCSettings::SELECTION_MODE_POWER:
                m_Data.updateValuesFromMode(m_Data.getEncoderPosition() / 1000.000, aDCSettings::SELECTION_MODE_POWER);
//                m_Data.updateValuesFromMode(static_cast<float>(floatRounding(floatRounding(float(m_Data.getEncoderPosition())) / 1000.000)), aDCSettings::SELECTION_MODE_POWER);
                break;

            default:
                break;
        }
    }

    // Convert the set current into an voltage to be sent to the DAC
    switch (m_Data.setCurrent(_readMeasuredCurrent(), aDCSettings::OPERATION_MODE_READ))
    {
        case aDCSettings::SETTING_ERROR_OVERSIZED:
            // Overcurrent alarm
            if (encoderChanged)
                m_Data.enableFeature(FEATURE_OCP);
            m_Data.setCurrent(encoderChanged ? 1 : 0.0, aDCSettings::OPERATION_MODE_SET);
            m_Data.setEncoderPosition(encoderChanged ? 0 : 0);
            break;

        case aDCSettings::SETTING_ERROR_UNDERSIZED:
        case aDCSettings::SETTING_ERROR_VALID:
            //m_Data.setOverCurrent(false);
            _adjustCurrent();
            break;
    }
}

/** \brief Adjust current settings
 *
 * \return void
 *
 */
void aDCEngine::_adjustCurrent()
{
    float roundedMeasuredCurrent = round(m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ) * 1000) / 1000.000; // This the best way I can think of rounding a floating
                                                                                     // point number to 3 decimal places.
    //only adjust the current of the set and measured currents are different.
    if (roundedMeasuredCurrent != m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET))
    {
        float adjustedCurrent = 0.0;

        // To ensure we are not dividing by 0.
        if (m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ) != 0.0)                                                                  // Turn the current error between set and measured
            adjustedCurrent = (m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET) / m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ)) * m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET); // into a percentage so it can be adjusted
        else
            adjustedCurrent = m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET);

        int16_t dacCurrent = ((adjustedCurrent * 0.1 * 2.5) / 2.048) * 4096;

        // Send the value to the DAC.
        _setDAC(dacCurrent, DAC_CURRENT_CHAN);

        // Read current value again
#ifdef SIMU
        m_Data.setCurrent(m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET), aDCSettings::OPERATION_MODE_READ);
#else
        m_Data.setCurrent(_readMeasuredCurrent(), aDCSettings::OPERATION_MODE_READ);
#endif
    }

#ifdef RESISTANCE
    m_Data.setResistance((m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ) > 0.0) ? m_Data.getVoltage() / m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ) : 0, aDCSettings::OPERATION_MODE_READ);
#endif // RESISTANCE

    switch (m_Data.setPower(floatRounding(m_Data.getVoltage() * m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ)), aDCSettings::OPERATION_MODE_READ))
    {
        case aDCSettings::SETTING_ERROR_OVERSIZED:
            m_Data.enableFeature(FEATURE_OCP);
            m_Data.setCurrent(0.0, aDCSettings::OPERATION_MODE_SET);
            m_Data.setPower(0.0, aDCSettings::OPERATION_MODE_SET);
#ifdef RESISTANCE
            m_Data.setResistance(0.0, aDCSettings::OPERATION_MODE_SET);
#endif
            m_Data.setEncoderPosition(0);
            _adjustCurrent();
            break;

        default:
            break;
    }
}

/** \brief Function to read the ADC, accepts the channel to be read.
 *
 * \param channel uint8_t : <b> ADC channel </b>
 * \return float : <b> readed value </b>
 *
 */
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

/** \brief Function to set the DAC, Accepts the Value to be sent and the channel of the DAC to be used.
 *
 * \param value uint16_t : <b> value to send to DAC </b>
 * \param channel uint8_t : <b> DAC channel </b>
 * \return void
 *
 */
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

/** \brief Function that read temperature from ADC channels.
 *
 * \return int16_t : <b> temperature </b>
 *
 */
int16_t aDCEngine::_readTemp()
{
#ifdef SIMU
    return m_Data.getTemperature();
#else
    float tempSensor1 = _readADC(ADC_TEMPSENSE1_CHAN);
    float tempSensor2 = _readADC(ADC_TEMPSENSE2_CHAN);
    float tempVoltage = ((tempSensor1 + tempSensor2) / 2) * 1000; // This takes an average of bothe temp sensors and converts the
                                                                  // value to millivolts

    return static_cast<int16_t>(((tempVoltage - 1885) / -11.2692307) + 20); // This comes from the datasheet to calculate the temp from the voltage given.
#endif // SIMU
}

/** \brief Function to set the fan speed depending on the heatsink temperature.
 *
 * \return void
 *
 */
void aDCEngine::_updateFanSpeed()
{
    m_Data.setTemperature(_readTemp());

    uint16_t heatSinkTemp = m_Data.getTemperature();
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
#warning handle over temp

    for (uint8_t i = 0; i < (sizeof(fanThresholds) / sizeof(fanThresholds[0])); i++)
    {
        if (heatSinkTemp >= static_cast<uint16_t>(pgm_read_word(&fanThresholds[i].temp)))
        {
            uint16_t speed;

            if (m_Data.getFanSpeed() != (speed = static_cast<uint16_t>(pgm_read_word(&fanThresholds[i].speed))))
            {
                m_Data.setFanSpeed(speed);
                _setDAC(speed, DAC_FAN_CHAN);
            }
            break;
        }
    }
}

/** \brief Return a pointer to settings instancied class
 *
 * \return const aDCSettings*
 *
 */
const aDCSettings *aDCEngine::_getSettings() const
{
    return &m_Data;
}
