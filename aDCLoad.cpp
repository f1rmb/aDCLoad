/**
 *
 * \copyright Copyright (C) 2014-2015  F1RMB, Daniel Caujolle-Bert <f1rmb.daniel@gmail.com>
 * \copyright Copyright (C) 2014       Lee Wiggins <lee@wigweb.com.au>
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
 * \warning
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
    \author F1RMB, Daniel Caujolle-Bert <f1rmb.daniel@gmail.com>
    \author Lee Wiggins <lee@wigweb.com.au>
*/

// A bit of user's manual
/**
 * \page fuses ATmega32U4 fuses settings
 *
 * Unlike the Arduino Leornardo board, the ATmega32U4 MCU used in this DC load needs some special fuses settings.
 *
 * The following command line defines them to the correct values:
 *
 * \code avrdude -F -p atmega32u4 -C /etc/avrdude.conf -v -e -V -c usbasp -P usb -U lfuse:w:0xFF:m -U hfuse:w:0xD1:m -U efuse:w:0xCB:m -F lfuse:w:0xE1:m \endcode
 *
 *
 */

/**
 * \page gui User interface overview
 * \todo complete GUI section
 *
 * - The DC load control is done using a simple rotary encoder, which integrates a push button.
 * There are two modes, <b>values reading</b> and <b>values settings</b>.
 * When you rotate the encoder, the controler automatically switches to settings mode. You just need to rotate the encoder to set up the desired value.
 * In both modes, a double click changes the focus (delimited by '[' and ']' symbols) to the next value parameter.
 * Also, a simple click changes the tuning step. Next to the ']' delimiter symbol, an icon displays the tuning step, as following:
 *  Multiplier | Glyph
 *  -----------|-------
 *  x1 | \image html x1.png "" \image latex x1.png ""
 *  x10 | \image html x10.png "" \image latex x10.png ""
 *  x100 |  \image html x100.png "" \image latex x100.png ""
 *  x1000 | \image html x1k.png "" \image latex x1k.png ""
 *
 * <br>
 *
 * - According to the status of the DC Load, some icons may be shown:
 *  Feature | Glyph
 *  --------|------
 *  Logging | \image html Logging.png "" \image latex Logging.png ""
 *  Locked | \image html Lock.png "" \image latex Lock.png ""
 *  USB remote control | \image html USB.png "" \image latex USB.png ""
 *
 * <br>
 *
 * - To access to the options, you need to press the button for more than 3 seconds. In this <i>window</i>,
 * you can enable or disable the <b>backlight's auto-dimmer</b> and the <b>rotary encoder's auto-lock</b> features.
 * A double click changes the option focus, a simple click changes the option status and a long press exits this <i>window</i>.
 *
 * <br>
 *
 * - When auto-lock is turned on, and triggered, a double click unlocks the rotary encoder.
 *
 * <br>
 *
 * - There are 3 differents alarms: <b>OC</b> for over-current, <b>OV</b> for over-voltage and <b>OT</b> the over-termperature.
 *
 * <br>
 *
 * - When auto-dimmer is turned on, and triggered, any rotary encoder action will turn the backlight on, without any change to
 * the defined settings.
 *
 * <br>
 *
 * - The DC Load can be remotely controlled, see \ref remote
 */

/**
 * \page remote Remote Commands
 * See also \ref logging
 *
 * \note Serial port configuration: <b>57600</b>,<b>8</b>,<b>N</b>,<b>1</b>
 * \warning Commands and arguments are <b>case sensitive</b>, <b>ALL</b> in <b>UPCASE</b>
 *
 * \section idn Get Identification
 * * <b>:*IDN?:</b>
 *      - Returns firmware informations
 *
 * See \ref retval
 *
 * \section curget Current setting getter
 * * <b>:ISET?:</b>
 *      - Returns current setting (in <b>mA</b>)
 *
 * See \ref retval.
 *
 * \section curset Current setting setter
 * * <b>:ISET:<i>value</i></b>
 *      - Set current <b><i>value</i></b> (in <b>mA</b>)
 *
 * See \ref retval.
 *
 * \section cal Calibration
 * * <b>:CAL:<i>toggle</i></b>
 *      - Turns <b><i>ON</i></b> or <b><i>OFF</i></b> the logging feature.<br>
 * * <b>:CAL:<i>section</i>:<i>slope</i>,<i>offset</i></b>
 *      - <b><i>section</i></b> could be <b>V</b>, <b>C</b>, <b>D</b> or <b>VD</b>, standing for <b>V</b><i>oltage</i>, <b>C</b><i>urrent</i>, <b>D</b><i>AC</i> and <b>V</b><i>oltage</i> <b>D</b><i>rop</i>.
        - <b><i>slope</i></b> and <b><i>offset</i></b> are floating point values, with US period decimal separator ('.'). These values could be calculated using the <i>LibreOffice</i>'s spreadsheet file <i>aDCLoadCalibration.ods</i>.
 * * <b>:CAL:SAVE</b>
 *      - Backup calibation datas into EEPROM.
 *
 * See \ref retval
 * <br> See \ref calibration
 *
 * \section dac DAC value setter (calibration purpose)
 * * <b>:DAC:<i>value</i></b>
 *      - Set DAC value (from <b>0</b> to <b>4095</b>).
 *
 * \note This command has no effect outside calibration mode
 *
 * See \ref cal
 * <br> See \ref calibration
 *
 * \section curread Current readed getter
 * * <b>:I?:</b>
 *      - Returns current readed from the load (in <b>mA</b>)
 *
 * See \ref retval.
 *
 * \section volread Voltage readed getter
 * * <b>:U?:</b>
 *      - Returns voltage readed from the load (in <b>mV</b>)
 *
 * See \ref retval.
 *
 * \section logsingle Logging enability
 * * <b>:LOG?:</b>
 *      - Printout if logging is <b><i>ON</i></b> or <b><i>OFF</i></b>.
 *
 * See \ref retval.
 *
 * \section logrun Logging enability
 * * <b>:LOG:<i>toggle</i></b>
 *      - Turns <b><i>ON</i></b> or <b><i>OFF</i></b> the logging feature.<br>

 * \note If <b><i>toggle</i></b> value is not specified, a single logging line is returned.

 * See \ref retval.
 *
 *
 * \section retval Return value
 * <b>:<i>value</i>:<i>status</i>:</b>
 * - Where:
 *  * <b><i>value</i></b> if any expected. <b>INVALID</b> on unknown command.
 *  * <b><i>status</i></b> could be <b>OK</b> on success or <b>ERR</b> on failure.
 *
 */

/**
 * \page logging Logging data format
 * See also \ref remote
 * \note fields are comma separated
 *
 * \section logform CSV logging format
 * <b><i>timestamp</i></b>,<b><i>voltage</i></b>,<b><i>current sets</i></b>,<b><i>current read</i></b>,<b><i>temperature</i></b><b><i>\\r\\n</i></b>
 * - Where:
 *  + <b><i>timestamp</i></b> in <b>hundred of milliseconds</b>,
 *  + <b><i>voltage</i></b> in <b>mV</b>,
 *  + <b><i>current sets</i></b> in <b>mA</b>,
 *  + <b><i>current read</i></b> in <b>mA</b>,
 *  + <b><i>temperature</i></b> in <b>Celcius degrees</b>.
 */

/**
 * \page calibration Calibration Process
 *
 *
 * + __Prerequisites__:<br><br>
 *
 *      - __Hardware__:
 *        - Amp-meter,
 *        - Volt-meter,
 *        - Power supply (<b>0..24V</b>, <b>8A</b>)
 * <br><br>
 *      - __Software__:
 *        * A serial terminal emulator (eg: “<i>HyperTerminal</i>” or “<i>Tera Term</i>” on Windows, “<i>minicom</i>” or “<i>cutecom</i>” on Linux).
 *
 *         The communication settings are: <b>57600</b>, <b>8</b>, <b>N</b>, <b>1</b>
 *
 * <br>
 * + __Process Description__:<br><br>
 *
 *      - __Step 1: Maximum Current__
 *
 *          Select “<i>Step 1</i>” tab in the calibration spreadsheet file.
 *
 *          Connect the amp-meter and the power supply to the DC load, for current measurements.
 *          Open your serial terminal emulator, connect the DC load, then type:
 *          \code :DAC:4095 \endcode
 *          Write down the <b>mA</b> value readed on the amp-meter to the “<b><i>mA<sub>max</sub></i></b>” column.
 *          Now, type:
 *          \code :DAC:0 \endcode
 *          Edit the aDCLoad.h source file, browse down the file, looking for the following line:
 *          \code static const float         CURRENT_MAXIMUM             = 7.845;    ///< Maximum value of load current (A) \endcode
 *          If necessary, change the 7.845 value to the one you've got on your amp-meter (don't forget to convert it from <b>mA</b>
 *          to <b>A</b>), then reflash the board with new code (using <i>Code::Blocks IDE</i> or the provided <i>Makefile</i>,
 *          running “<i>make burn</i>” command).
 *
 *          Remember, if you have to reflash the board, that could be only done using ICSP programming. There is no bootloader flashed on the MCU, due to flash space restriction.
 *
 *          Calibration step 1 is now done.<br><br>
 *
 *      - __Step 2: Voltage__
 *
 *          Select “<i>Step 2</i>” tab in the calibration spreadsheet file.
 *
 *          Connect your power supply to the DC load, sets to <b>0V</b>. The DC Load should the sets to <b>0mA</b>.
 *          In the serial terminal emulator, type:
 *          \code :CAL:ON \endcode
 *          Set your power supply voltage output for each value in “<b><i>V<sub>set</sub></i></b>” column, and write down the readed
 *          value in “<b><i>V<sub>read</sub></i></b>” column.
 *
 *          Once you went through the whole array, the calibration string should be entered into the serial terminal emulator, like:
 *          \code :CAL:V:x.xxx,y.yyy \endcode
 *          Please note that the decimal separator <b>HAS TO BE</b> a period ('.'), as in US format.
 *
 *          Calibration step 2 is now done.<br><br>
 *
 *      - __Step 3: Current__
 *
 *          Select “<i>Step 3</i>” tab in the calibration spreadsheet file.
 *
 *         Connect the amp-meter and the power supply to the DC load, for current measurements.
 *         Sets the output voltage to <b>5V</b>.
 *
 *         Using the DAC command, try to adjust its value to match each value in the “<b><i>A Amp-Meter</i></b>” column,
 *         and write down the readed value, on the LCD or serial terminal emulator output, into the “<b><i>A LCD/Term.</i></b>” column.
 *
 *         You can change the values in the “<b><i>A Amp-Meter</i></b>” column to strictly match the ones you're reading on the amp-meter.
 *
 *         The DAC command syntax is :
 *         \code :DAC:value \endcode where value is an integer from 0 to 4095.<br><br>
 *         Once you went through the whole array, set DAC value to <b>0</b>:
 *         \code :DAC:0 \endcode
 *         The calibration string should be entered into the serial terminal emulator, like:
 *         \code :CAL:C:x.xxx,y.yyy \endcode
 *         Please note that the decimal separator <b>HAS TO BE</b> a period ('.'), as in US format.
 *
 *         Calibration step 3 is now done.<br><br>
 *
 *      - __Step 4: DAC__
 *
 *         Select “<i>Step 4</i>” tab in the calibration spreadsheet file.
 *
 *         Connect the amp-meter and the power supply to the DC load, for current measurements.
 *         Sets the output voltage to <b>5V</b>.
 *
 *         Set the DAC value for each value in “<b><i>Steps</i></b>” column, and write down the readed value on the
 *         amp-meter into the “<b><i>mA<sub>read</sub></i></b>” column.
 *
 *         The DAC command syntax is :
 *         \code :DAC:value \endcode where value is an integer from 0 to 4095.<br><br>
 *         Once you went through the whole array, set DAC value to <b>0</b>:
 *         \code :DAC:0 \endcode
 *         The calibration string should be entered into the serial terminal emulator, like:
 *         \code :CAL:D:x.xxx,y.yyy \endcode
 *         Please note that the decimal separator <b>HAS TO BE</b> a period ('.'), as in US format.
 *
 *         Calibration step 4 is now done.<br><br>
 *
 *      - __Step 5: Voltage Drop__
 *
 *          Select “<i>Step 5</i>” tab in the calibration spreadsheet file.
 *
 *          Connect the amp-meter, the volt-meter and the power supply to the DC load, for current <b>AND</b> voltage measurements.
 *          Sets the output voltage to <b>5V</b> (the last entry in the array should be set around <b>12V</b>).
 *
 *          Using the DAC command, try to adjust the its value to match each value in the “<b><i>mA</i></b>” column on the amp-meter,
 *          and write down the voltage readed value on the volt-meter into the “<b><i>mV meter</i></b>” column, and the readed value
 *          on the LCD and/or serial terminal emulator to the “<b><i>mV LCD/Term.</i></b>” column.
 *
 *          The DAC command syntax is :
 *          \code :DAC:value \endcode where value is an integer from 0 to 4095.<br><br>
 *          You can change the values in the “<b><i>mA</i></b>” column to strictly match the ones you're reading on the amp-meter.
 *
 *          For the last row on the array, set the output voltage to around <b>12V</b>.
 *
 *          Once you went through the whole array, set DAC value to <b>0</b>:
 *          \code :DAC:0 \endcode
 *          The calibration string should be entered into the serial terminal emulator, like:
 *          \code :CAL:VD:x.xxx,y.yyy \endcode
 *          Please note that the decimal separator <b>HAS TO BE</b> a period ('.'), as in US format.
 *
 *          Calibration step 5 is now done.<br><br>
 *
 *      - __Last Step: Backup__
 *
 *          Once the full calibration is done, you <b>HAVE</b> to save the values into the EEPROM, using the following command:
 *          \code :CAL:SAVE \endcode <br><br>
 *          ### Now, the calibration is done. You can use your DC load.
 *
 *
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

/** \brief Flush serial buffer (TX)
 *
 * \return void
 *
 */
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
 * \return uint8_t
 *
 */
uint8_t serialRead()
{
    return static_cast<uint8_t>(Serial.read());
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
 * \return int8_t : <b> numeric value length </b>
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
 * \return float : <b> rounded value </b>
 *
 */
float floatRounding(float f)
{
	return ((f / 1000.000) * 1000.000);
}

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
                    m_fanSpeed(0xFF),
                    m_currentDAC(0xFF),
                    m_operationMode(OPERATION_MODE_READ),
                    m_mode(SELECTION_MODE_CURRENT),
                    m_encoderPos(0),
                    m_dispMode(DISPLAY_MODE_VALUES),
                    m_lockTick(0), m_operationTick(0),
                    m_features(0x0),
                    m_datas(0xFFFF)
{
    // Calibration zeroing
    for (uint8_t i = 0; i < CALIBRATION_MAX; i++)
    {
        m_calibrationValues[i].slope = 1.0;
        m_calibrationValues[i].offset = 0.0;
    }

    _eepromRestore();
    syncData(DATA_IN_CALIBRATION); // Reset Calibration Bit (all bits are set to 1 on startup)
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
    SettingError_t err = _setValue(aDCSettings::OPERATION_MODE_READ, DATA_VOLTAGE, v, m_readVoltage, m_readVoltage, VOLTAGE_MAXIMUM);

    // Over-Voltage protection triggered
    if (err == SETTING_ERROR_OVERSIZED)
        enableAlarm(FEATURE_OVP);

    return err;

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
 * \param v float : <b> current </b>
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
 * \return float : <b> current </b>
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
 * \param v float : <b> resistance </b>
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
 * \return float : <b> resistance </b>
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
 * \param v float : <b> power </b>
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
 * \return float : <b> power </b>
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
            if ((voltage == 0.0) && (v > 0.0))
                v = 0.0;

            switch (setPower(v, aDCSettings::OPERATION_MODE_SET))
            {
                case aDCSettings::SETTING_ERROR_OVERSIZED:
                    setPower(POWER_MAXIMUM, aDCSettings::OPERATION_MODE_SET);
                    setCurrent((voltage > 0.0) ? floatRounding(floatRounding(getPower(aDCSettings::OPERATION_MODE_SET)) / floatRounding(voltage)) : 0.0, aDCSettings::OPERATION_MODE_SET);
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
                    setCurrent((voltage > 0.0) ? floatRounding(floatRounding(getPower(aDCSettings::OPERATION_MODE_SET)) / floatRounding(voltage)) : 0.0, aDCSettings::OPERATION_MODE_SET);
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
 * \param v uint16_t : <b> temperature </b>
 * \return void
 *
 */
void aDCSettings::setTemperature(uint16_t v)
{
    uint16_t p = m_readTemperature;
    m_readTemperature = v;

    _enableDataCheck(DATA_TEMPERATURE, (p != m_readTemperature));

    // Turn over-temperature alarm, set current to zero;
    if (m_readTemperature > TEMPERATURE_MAXIMUM)
        enableAlarm(FEATURE_OTP);
    else if ((m_readTemperature <= TEMPERATURE_MAXIMUM) && isFeatureEnabled(FEATURE_OTP))
        enableFeature(FEATURE_OTP, false);
}

/** \brief Temperature readed getter
 *
 * \return uint16_t : <b> temperature </b>
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

// DAC (current)
/** \brief Current DAC value setter
 *
 * \param dac uint16_t : <b> current DAC value </b>
 * \return void
 *
 */
void aDCSettings::setCurrentDAC(uint16_t dac)
{
    m_currentDAC = dac;
}

/** \brief Current DAC value getter
 *
 * \return uint16_t : <b> current DAC value </b>
 *
 */
uint16_t aDCSettings::getCurrentDAC()
{
    return m_currentDAC;
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

// Calibation
/** \brief Calibration mode enability setter
 *
 * \param enable bool : <b> enability </b>
 * \return void
 *
 */
void aDCSettings::setCalibationMode(bool enable)
{
    _enableData(DATA_IN_CALIBRATION, enable);
}

/** \brief Calibration mode enability getter
 *
 * \return bool : <b> enability </b>
 *
 */
bool aDCSettings::getCalibrationMode()
{
    return isDataEnabled(DATA_IN_CALIBRATION);
}

/** \brief Calibration data getter, according to <i>calsection</i> argument.
 *
 * \param calsection CalibrationValues_t : <b> calibration parameter </b>
 * \param data CalibrationData_t& : <b> calibration data </b>
 * \return void
 *
 */
void aDCSettings::getCalibrationValues(CalibrationValues_t calsection, CalibrationData_t &data)
{
    data.slope = m_calibrationValues[calsection].slope;
    data.offset = m_calibrationValues[calsection].offset;
}

/** \brief Calibration data setter, according to <i>calsection</i> argument
 *
 * \param calsection CalibrationValues_t : <b> calibration parameter </b>
 * \param data CalibrationData_t& : <b> calibration data </b>
 * \return void
 *
 */
void aDCSettings::setCalibrationValues(CalibrationValues_t calsection, CalibrationData_t data)
{
    m_calibrationValues[calsection].slope = data.slope;
    m_calibrationValues[calsection].offset = data.offset;
}

/** \brief Backup calibration data into EEPROM
 *
 * \return void
 *
 */
void aDCSettings::backupCalibration()
{
    int16_t start = EEPROM_ADDR_CALIBRATION_VOLTAGE;

    for (uint8_t i = CALIBRATION_VOLTAGE; i < CALIBRATION_MAX; i++, start += EEPROM_CALIBRATION_SIZE)
        _eepromCalibrationBackup(start, m_calibrationValues[i]);
}

/** \brief Restore calibration data from EEPROM
 *
 * \return void
 *
 */
void aDCSettings::restoreCalibration()
{
    int16_t start = EEPROM_ADDR_CALIBRATION_VOLTAGE;

    for (uint8_t i = CALIBRATION_VOLTAGE; i < CALIBRATION_MAX; i++, start += EEPROM_CALIBRATION_SIZE)
        _eepromCalibrationRestore(start, m_calibrationValues[i]);
}

/** \brief Turn alarm (OVP, OCP or OTP) on, sets output current to zero
 *
 * \param aBit uint16_t : <b> alarm bit to set </b>
 * \return void
 *
 */
void aDCSettings::enableAlarm(uint16_t aBit)
{
    if (!isFeatureEnabled(aBit))
        enableFeature(aBit);

    setCurrent(0.0, OPERATION_MODE_SET);
    setPower(0.0, OPERATION_MODE_SET);
#ifdef RESISTANCE
    setResistance(0.0, OPERATION_MODE_SET);
#endif
    setEncoderPosition(0);
    syncData(aDCSettings::DATA_ENCODER);
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

// EEPROM functions
/** \brief Check for EEPROM magic numbers
 *
 * \return bool
 *
 * Used to check if some data has already been wrote in the EEPROM.
 */
bool aDCSettings::_eepromCheckForMagicNumbers()
{
    return ((EEPROM.read(EEPROM_ADDR_MAGIC) == 0xF) && (EEPROM.read(EEPROM_ADDR_MAGIC + 1) == 0xE) &&
            (EEPROM.read(EEPROM_ADDR_MAGIC + 2) == 0xE) && (EEPROM.read(EEPROM_ADDR_MAGIC + 3) == 0xD));
}

/** \brief Write magic numbers into EEPROM
 *
 * \return void
 *
 */
void aDCSettings::_eepromWriteMagicNumbers()
{
    // Magic numbers
    EEPROM.write(EEPROM_ADDR_MAGIC,     0xF);
    EEPROM.write(EEPROM_ADDR_MAGIC + 1, 0xE);
    EEPROM.write(EEPROM_ADDR_MAGIC + 2, 0xE);
    EEPROM.write(EEPROM_ADDR_MAGIC + 3, 0xD);
}

/** \brief Reset all stored parameters into EEPROM
 *
 * \return void
 *
 */
void aDCSettings::_eepromReset()
{
    _eepromWriteMagicNumbers();

    EEPROM.write(EEPROM_ADDR_AUTODIM, 1);
    EEPROM.write(EEPROM_ADDR_AUTOLOCK, 1);

    backupCalibration();
}

/** \brief Restore value from EEPROM
 *
 * \return void
 *
 */
void aDCSettings::_eepromRestore()
{
    if (!_eepromCheckForMagicNumbers())
        _eepromReset();

    enableFeature(FEATURE_AUTODIM, (EEPROM.read(EEPROM_ADDR_AUTODIM) == 1));
    enableFeature(FEATURE_AUTOLOCK, (EEPROM.read(EEPROM_ADDR_AUTOLOCK) == 1));
    restoreCalibration();
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

//! \brief CRC8 computation
//!
//! Code took from http://www.pjrc.com/teensy/td_libs_OneWire.html
//!
//! \param addr const uint8_t* : <b> Data source </b>
//! \param len uint8_t : <b> Data source length </b>
//! \return uint8_t : <b> CRC </b>
//!
//!
uint8_t aDCSettings::_crc8(const uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0;

	while (len--)
    {
		uint8_t inbyte = *addr++;

		for (uint8_t i = 8; i; i--)
        {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;

			if (mix)
                crc ^= 0x8C;

			inbyte >>= 1;
		}
	}

	return crc;
}

/** \brief Read data from EEPROM, used to restore calibration data
 *
 * \param addr int16_t : <b> start address location </b>
 * \param cal CalibrationData_t& : <b> destination </b>
 * \return void
 *
 */
void aDCSettings::_eepromCalibrationRestore(int16_t addr, CalibrationData_t &cal)
{
    int16_t                     start = addr;
    _eepromCalibrationValue_t   sl, off;
    uint8_t                     crc;
    uint8_t                     crcData[sizeof(float) * 2];
    uint8_t                     crcOffset = 0;

    for (uint8_t i = 0; i < sizeof(float); i++)
    {
        sl.c[i] = EEPROM.read(start++);
        crcData[crcOffset++] = sl.c[i];
    }

    for (uint8_t i = 0; i < sizeof(float); i++)
    {
        off.c[i] = EEPROM.read(start++);
        crcData[crcOffset++] = off.c[i];
    }

    crc = EEPROM.read(start++);

    if (crc == _crc8(crcData, crcOffset))
    {
        cal.slope = sl.v;
        cal.offset = off.v;
    }
}

/** \brief Save data to EEPROM, used to restore calibration data
 *
 * \param addr int16_t : <b> start address location </b>
 * \param cal CalibrationData_t& : <b> destination </b>
 * \return void
 *
 */
void aDCSettings::_eepromCalibrationBackup(int16_t addr, CalibrationData_t cal)
{
    int16_t                     start = addr;
    _eepromCalibrationValue_t   sl, off;
    uint8_t                     crcData[sizeof(float) * 2];
    uint8_t                     crcOffset = 0;

    sl.v = cal.slope;
    off.v = cal.offset;

    if (!_eepromCheckForMagicNumbers())
        return;

    for (uint8_t i = 0; i < sizeof(float); i++)
    {
        EEPROM.write(start++, sl.c[i]);
        crcData[crcOffset++] = sl.c[i];
    }

    for (uint8_t i = 0; i < sizeof(float); i++)
    {
        EEPROM.write(start++, off.c[i]);
        crcData[crcOffset++] = off.c[i];
    }

    EEPROM.write(start++, (_crc8(crcData, crcOffset)));
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
void aStepper::increment()
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
uint8_t aStepper::getValue()
{
    return m_inc;
}

/** \brief Reset value
 *
 * \return void
 *
 */
void aStepper::reset()
{
    m_inc = 0;
}

/** \brief Value getter, according to multiple.
 *
 * \return int16_t
 *
 */
int16_t aStepper::getMult()
{
    return (_pow(10, m_inc));
}

/** \brief Get value according to selection mode
 *
 * \param mode uint16_t : <b> Selection mode (will be typecasted to aDCSettings::SelectionMode_t)</b>
 * \return int16_t
 *
 */
int16_t aStepper::getValueFromMode(uint8_t mode)
{
    switch (static_cast<aDCSettings::SelectionMode_t>(mode))
    {
        case aDCSettings::SELECTION_MODE_CURRENT:
            {
                switch (getMult())
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
            return getMult();
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
bool aStepper::isSynced()
{
    return (m_inc == m_incPrev);
}

/** \brief Check if value is synchronized
 *
 * \return void
 *
 */
void aStepper::sync()
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
 * \param d4 uint8_t : <b> LCD d4 pin </b>
 * \param d5 uint8_t : <b> LCD d5 pin </b>
 * \param d6 uint8_t : <b> LCD d6 pin </b>
 * \param d7 uint8_t : <b> LCD d7 pin </b>
 * \param cols uint8_t : <b> LCD columns number </b>
 * \param rows uint8_t : <b> LCD rows number </b>
 *
 */
aLCD::aLCD(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t cols, uint8_t rows) :
        LiquidCrystal(rs, enable, d0, d1, d2, d3, d4, d5, d6, d7),
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
#if 1
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
#endif

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
 * \param d4 uint8_t : <b> LCD d4 pin </b>
 * \param d5 uint8_t : <b> LCD d5 pin </b>
 * \param d6 uint8_t : <b> LCD d6 pin </b>
 * \param d7 uint8_t : <b> LCD d7 pin </b>
 * \param cols uint8_t : <b> LCD columns </b>
 * \param rows uint8_t : <b> LCD rows </b>
 *
 */
aDCDisplay::aDCDisplay(aDCEngine *parent, uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t cols, uint8_t rows) :
        aLCD(rs, enable, d0, d1, d2, d3, d4, d5, d6, d7, cols, rows),
        m_Parent(parent), m_dimmed(false), m_dimmerTick(0), m_nextUpdate(0)
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
#ifdef SIMU
    aLCD::setCursor(0, 0);
    aLCD::printCenter("*SIMULATION*");
#endif // SIMU

    aLCD::setCursor(0, 1);
    aLCD::printCenter("DC Electronic Load");

    char buffer[LCD_COLS_NUM + 1];
    snprintf(buffer, LCD_COLS_NUM, "v %d.%d", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR);

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
    unsigned long                   m = millis();

    // Update the display each DISPLAY_UPDATE_RATE ms
    if ((m - m_nextUpdate) > DISPLAY_UPDATE_RATE)
    {
        m_nextUpdate = m;

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
                if (!d->isSynced() || fullRedraw)
                {
                    aLCD::setCursor(OFFSET_MARKER_RIGHT + 1, static_cast<uint8_t>(d->getSelectionMode()) + 1);
                    aLCD::write(d->getValue());
                    d->sync();
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

                // OTP
                if (d->isFeatureEnabled(FEATURE_OTP) && (!d->isFeatureEnabled(FEATURE_OTP_VISIBLE) || fullRedraw))
                {
                    d->enableFeature(FEATURE_OTP_VISIBLE);

                    aLCD::setCursor(OFFSET_UNIT + OFFSET_TEMP, 0);
                    aLCD::print("OT");
                }
                else if (!d->isFeatureEnabled(FEATURE_OTP) && (d->isFeatureEnabled(FEATURE_OTP_VISIBLE) || fullRedraw))
                {
                    d->enableFeature(FEATURE_OTP_VISIBLE, false);

                    aLCD::setCursor(OFFSET_UNIT + OFFSET_TEMP, 0);
                    aLCD::print(char(0xDF)); // °
                    aLCD::print("C");
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
 * \param d4 uint8_t : <b> LCD d4 pin </b>
 * \param d5 uint8_t : <b> LCD d5 pin </b>
 * \param d6 uint8_t : <b> LCD d6 pin </b>
 * \param d7 uint8_t : <b> LCD d7 pin </b>
 * \param cols uint8_t : <b> LCD Columns number </b>
 * \param rows uint8_t : <b> LCD Rows number </b>
 * \param enca uint8_t : <b> Encoder A pin </b>
 * \param encb uint8_t : <b> Encoder B pin </b>
 * \param encpb uint8_t : <b> Encoder push button pin </b>
 * \param encsteps uint8_t : <b> Encoder steps per notch </b>
 *
 */
aDCEngine::aDCEngine(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                     uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t cols, uint8_t rows,
                     uint8_t enca, uint8_t encb, uint8_t encpb, uint8_t encsteps)
                     : aDCDisplay(this, rs, enable, d0, d1, d2, d3, d4, d5, d6, d7, cols, rows),
                       m_encoder(new ClickEncoder(enca, encb, encpb, encsteps)),
                       m_RXoffset(0)
{
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
void aDCEngine::_handleLoggingAndRemote()
{
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

                if ((m_RXbuffer[m_RXoffset] == 0xA) || (m_RXoffset == (RXBUFFER_MAXLEN - 1) /* Overflow checking */))
                {
                    EOL = true;
                    break;
                }

                m_RXoffset++;
            }

            if (EOL)
            {
                bool valid = false;

                m_RXbuffer[m_RXoffset] = '\0';

                if (m_RXoffset >= 3)
                {
                    /*
                     *** Format is: :CMD:<ARG> or :CMD:SUB:ARG
                     ***         CMD is command
                     ***         SUB is sub-command
                     ***         ARG is (<>: optional) argument
                     */

                    uint8_t *cmdStart, *cmdEnd;
                    if (((cmdStart = (uint8_t *)strchr((const char *)&m_RXbuffer[0], ':')) != NULL) &&
                        ((cmdEnd = (uint8_t *)strchr((const char *)cmdStart + 1, ':')) != NULL) )
                    {
                        uint8_t cmd[(cmdEnd - cmdStart)];
                        uint8_t *arg = cmdEnd + 1;

                        memcpy(&cmd[0], cmdStart + 1, sizeof(cmd) - 1);
                        cmd[sizeof(cmd) - 1] = '\0';

                        serialPrint(':');
#if 1
                        if (!strcmp((const char *)cmd, "*IDN?")) // Get Identification
                        {
                            char buf[64];

                            snprintf(buf, sizeof(buf), "FW=%d.%d, Built=<%s>@%s, GNU c++=%d.%d.%d",
                                     SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, __DATE__, __TIME__,
                                     __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);

                            serialPrint(&buf[0]);
                            valid = true;
                        }
                        else
#endif
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
                                m_Data.enableAlarm(FEATURE_OVP);

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
                        else if (!strcmp((const char *)cmd, "T")) // Temperature Read Setting
                        {
                            uint8_t v = static_cast<uint8_t>(atoi((const char *)arg));
                            m_Data.setTemperature(v);

                            serialPrint(int(m_Data.getTemperature()));
                            valid = true;
                        }
                        else if (!strcmp((const char *)cmd, "F")) // Fan speed (DAC value)
                        {
                            _setDAC((static_cast<uint16_t>(atoi((const char *)arg))), DAC_FAN_CHAN);
                            valid = true;
                        }
#endif
                        else if (!strcmp((const char *)cmd, "CAL")) // Calibration
                        {
                            valid = true;

                            // Turn ON calibation mode, DAC won't be automatically updated.
                            if (!strcmp((const char *)arg, "ON"))
                            {
                                aDCSettings::CalibrationData_t cal;
                                cal.slope = 1.0;
                                cal.offset = 0.0;

                                m_Data.setCalibationMode(true);

                                // Set all calibration values to zero
                                for (uint8_t i = aDCSettings::CALIBRATION_VOLTAGE; i < aDCSettings::CALIBRATION_MAX; i++)
                                    m_Data.setCalibrationValues(static_cast<aDCSettings::CalibrationValues_t>(i), cal);
                            }
                            else if (!strcmp((const char *)arg, "OFF"))
                            {
                                m_Data.setCalibationMode(false);
                                m_Data.restoreCalibration();
                            }
                            else if (!strcmp((const char *)arg, "SAVE"))
                            {
                                m_Data.backupCalibration();
                                m_Data.setCalibationMode(false);
                            }
#if 0
                            else if (!strcmp((const char *)arg, "DUMP"))
                            {
                                m_Data.dumpCalibration();
                            }
#endif
                            else
                            {
                                uint8_t *pV1; uint8_t *pV2;

                                if (((pV1 = (uint8_t *)strchr((const char *)arg, ':')) != NULL)
                                    && ((pV2 = (uint8_t *)strchr((const char *)pV1, ',')) != NULL))
                                {
                                    pV1++;

                                    aDCSettings::CalibrationData_t cal;
                                    uint8_t cmd2[(pV1 - arg)];

                                    memcpy(&cmd2[0], arg, sizeof(cmd2) - 1);
                                    cmd2[sizeof(cmd2) - 1] = '\0';

                                    *pV2++ = '\0';

                                    cal.slope = atof((char *)pV1);
                                    cal.offset = atof((char *)pV2);

                                    if (!strcmp((const char *)cmd2, "VD"))
                                        m_Data.setCalibrationValues(aDCSettings::CALIBRATION_VOLTAGE_DROP, cal);
                                    else if (!strcmp((const char *)cmd2, "V"))
                                        m_Data.setCalibrationValues(aDCSettings::CALIBRATION_VOLTAGE, cal);
                                    else if (!strcmp((const char *)cmd2, "C"))
                                        m_Data.setCalibrationValues(aDCSettings::CALIBRATION_READ_CURRENT, cal);
                                    else if (!strcmp((const char *)cmd2, "D"))
                                        m_Data.setCalibrationValues(aDCSettings::CALIBRATION_DAC_CURRENT, cal);
                                    else
                                        valid = false;
                                }
                                else
                                    valid = false;
                            }
                        }
                        else if (!strcmp((const char *)cmd, "DAC") && m_Data.getCalibrationMode())
                        {
                            uint16_t v = atoi((const char *)arg);

                            _setDAC(v, DAC_CURRENT_CHAN);
                            valid = true;
                        }
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
                        else if (!strcmp((const char *)cmd, "LOG?")) // Logging Ask
                        {
                            serialPrint(m_Data.isFeatureEnabled(FEATURE_LOGGING) ? "ON" : "OFF");
                            valid = true;
                        }
                        else if (!strcmp((const char *)cmd, "LOG")) // Logging ON/OFF
                        {
                            if (!strcmp((const char *)arg, "OFF"))
                                m_Data.enableFeature(FEATURE_LOGGING, false);
                            else if (!strcmp((const char *)arg, "ON"))
                                m_Data.enableFeature(FEATURE_LOGGING, true);
                            else
                                single = true;

                            valid = true;
                        }
                        else
                            serialPrint("INVALID");

                    }
                }

                // Clear buffer for next command
                m_RXoffset = 0;

                serialPrint(valid ? ":OK:\r\n" : ":ERR:\r\n");
            }
        }

        // Logging
        unsigned long m = millis();
        static unsigned long nextLogging = 0;

        if ((m_Data.isFeatureEnabled(FEATURE_LOGGING) && ((m - nextLogging) > LOGGING_RATE)) || single)
        {
            char buf[64];

            nextLogging = m;

            snprintf(buf, sizeof(buf) - 1, "%lu,%u,%u,%u,%u\r\n",
                    m / 100,
                    static_cast<uint16_t>(floatRounding(floatRounding(m_Data.getVoltage()) * 1000.000)),
                    static_cast<uint16_t>(floatRounding(floatRounding(m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET)) * 1000.000)),
                    static_cast<uint16_t>(floatRounding(floatRounding(m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ)) * 1000.000)),
                    m_Data.getTemperature());
            serialPrint(buf);
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
}

/** \brief Setup function, should be called before any other member
 *
 * \param isr ISRCallback : <b> pointer to callback function that may call service() </b>
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
    SPI.setClockDivider(SPI_CLOCK_DIV32); // WAS 16

    // Initialise timer
    Timer1.initialize(1000);
    Timer1.attachInterrupt(isr);

    aDCDisplay::setup();
    aDCDisplay::showBanner();

    // Data Logging and Remote Control
    Serial.begin(57600);

    // Flushing buffered char.
     if (Serial)
        while (serialAvailable() > 0)
            serialRead();

    // Workaround TX LED indicator
    TXLED0;
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
            int16_t mult = m_Data.getValueFromMode(m_Data.getSelectionMode());

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
                m_Data.increment();

            // Change selection mode according to double click.
            _handleButtonEvent(b);
        }

        // Reads input voltage from the load source. ****MAXIMUM 24V INPUT****
        m_Data.setVoltage(_getInputVoltage());

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
        _handleLoggingAndRemote();
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
float aDCEngine::_getInputVoltage()
{
#ifdef SIMU
    float v = m_Data.getVoltage();
#else
    aDCSettings::CalibrationData_t cal;

    // Retrieve Calibration data
    m_Data.getCalibrationValues(aDCSettings::CALIBRATION_VOLTAGE, cal);

    float v = (_getADC(ADC_INPUTVOLTAGE_CHAN) * cal.slope);
    if (v > 0.000)
        v += cal.offset;

    if (!m_Data.getCalibrationMode())
    {
        // Compensate voltage drop
        float current;
        if ((current = m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ)) > 0.000)
        {
            m_Data.getCalibrationValues(aDCSettings::CALIBRATION_VOLTAGE_DROP, cal);
            v += (current * cal.slope); // (0.11525 for single, 0.013555 for twin) is an average value of the voltage drop per mA
        }
    }
    else
    {
        serialPrint("U: ");
        serialPrint(v, 5);
    }
#endif

#warning DOUBLE CHECK THIS
    return (v < 0.018 ? 0.000 : v);
}

/** \brief Function to measure the actual load current.
 *
 * \return float : <b> readed current </b>
 *
 */
float aDCEngine::_getMeasuredCurrent()
{
#ifdef SIMU
    return m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ);
#else
    float                           current = 0.0;
    aDCSettings::CalibrationData_t  cal;

    // Retrieve Calibration data
    m_Data.getCalibrationValues(aDCSettings::CALIBRATION_READ_CURRENT, cal);

    // Averaging
    for (uint8_t i = 0; i < 5; i++)
        current += _getADC(ADC_MEASUREDCURRENT_CHAN) / 0.1000;

    current /= 5;

    current = current * cal.slope;

    if (current > 0.0)
        current += cal.offset;

    if (m_Data.getCalibrationMode())
    {
        serialPrint(", I: ");
        serialPrintln(current, 5);
    }

    return current;
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
                break;

#ifdef RESISTANCE
            case aDCSettings::SELECTION_MODE_RESISTANCE:
                m_Data.updateValuesFromMode(m_Data.getEncoderPosition() / 1000.000, aDCSettings::SELECTION_MODE_RESISTANCE);
                break;
#endif
            case aDCSettings::SELECTION_MODE_POWER:
                m_Data.updateValuesFromMode(m_Data.getEncoderPosition() / 1000.000, aDCSettings::SELECTION_MODE_POWER);
                break;

            default:
                break;
        }
    }

    // Convert the set current into an voltage to be sent to the DAC
    switch (m_Data.setCurrent(_getMeasuredCurrent(), aDCSettings::OPERATION_MODE_READ))
    {
        case aDCSettings::SETTING_ERROR_OVERSIZED:
            if (m_Data.getCalibrationMode())
            {
                _adjustLoadCurrent();
                break;
            }

            // Overcurrent alarm
            if (encoderChanged)
                m_Data.enableFeature(FEATURE_OCP);

            // Set to MAX
            switch (m_Data.getSelectionMode())
            {
                case aDCSettings::SELECTION_MODE_CURRENT:
                    m_Data.setEncoderPosition(CURRENT_MAXIMUM * 500.000);
                    break;
#ifdef RESISTANCE
                case aDCSettings::SELECTION_MODE_RESISTANCE:
                    m_Data.setEncoderPosition(RESISTANCE_MAXIMUM * 1000.000);
                    break;
#endif
                case aDCSettings::SELECTION_MODE_POWER:
                    m_Data.setEncoderPosition(POWER_MAXIMUM * 1000.000);
                    break;

                default:
                    break;
            }
            _updateLoadCurrent();
            break;

        case aDCSettings::SETTING_ERROR_UNDERSIZED:
        case aDCSettings::SETTING_ERROR_VALID:
            _adjustLoadCurrent();
            break;
    }
}

/** \brief Adjust current settings
 *
 * \return void
 *
 */
void aDCEngine::_adjustLoadCurrent()
{
    float currentMeasured = floatRounding(m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ));
    float currentSets     = floatRounding(m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET));

    // Only adjust the current if the set and measured currents are different.
    if (currentMeasured != currentSets)
    {
        aDCSettings::CalibrationData_t cal;

        // Retrieve Calibration data
        m_Data.getCalibrationValues(aDCSettings::CALIBRATION_DAC_CURRENT, cal);

        uint16_t dacCurrent = static_cast<uint16_t>(((currentSets * 1000.000) * cal.slope) + cal.offset);

        // Send the value to the DAC.
        if (!m_Data.getCalibrationMode() && (dacCurrent != m_Data.getCurrentDAC()))
        {
            if (dacCurrent > 4095)
                dacCurrent = 4095;

            _setDAC(dacCurrent, DAC_CURRENT_CHAN);

            m_Data.setCurrentDAC(dacCurrent);
        }

        // Read current value again
#ifdef SIMU
        m_Data.setCurrent(m_Data.getCurrent(aDCSettings::OPERATION_MODE_SET), aDCSettings::OPERATION_MODE_READ);
#else
        m_Data.setCurrent(_getMeasuredCurrent(), aDCSettings::OPERATION_MODE_READ);
#endif
    }

#ifdef RESISTANCE
    m_Data.setResistance((m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ) > 0.0) ? m_Data.getVoltage() / m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ) : 0, aDCSettings::OPERATION_MODE_READ);
#endif // RESISTANCE

    switch (m_Data.setPower(floatRounding(m_Data.getVoltage() * m_Data.getCurrent(aDCSettings::OPERATION_MODE_READ)), aDCSettings::OPERATION_MODE_READ))
    {
        case aDCSettings::SETTING_ERROR_OVERSIZED:
            if (!m_Data.getCalibrationMode())
            {
                m_Data.enableAlarm(FEATURE_OCP);
                _adjustLoadCurrent();
            }
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
float aDCEngine::_getADC(uint8_t channel)
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
int16_t aDCEngine::_getTemp()
{
#ifdef SIMU
    return m_Data.getTemperature();
#else
    float tempVoltage = ((_getADC(ADC_TEMPSENSE1_CHAN) + _getADC(ADC_TEMPSENSE2_CHAN)) / 2) * 1000;   // This takes an average of both temp sensors and converts the
                                                                                                        // value to millivolts

    return static_cast<int16_t>(((tempVoltage - 1885) / -11.2692307) + 20); // This comes from the datasheet to calculate the temp from the voltage given.
#endif // SIMU
}

/** \brief Function to set the fan speed depending on the temperature sensors value.
 *
 * \return void
 *
 */
void aDCEngine::_updateFanSpeed()
{
    uint16_t heatSinkTemp;
    static const struct
    {
        uint16_t temp;
        uint16_t speed;
    } fanThresholds[] PROGMEM =
    {
        { 60, 4095 },
        { 50, 3000 },
        { 40, 2400 },
        { 30, 2000 },
        { 20, 1800 }
    };

    m_Data.setTemperature((heatSinkTemp = _getTemp()));

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
