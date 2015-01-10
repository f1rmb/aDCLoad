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
#ifndef ADCLOAD_H
#define ADCLOAD_H

#include <float.h>
#include <LiquidCrystal.h>
#include <ClickEncoder.h>
#include <EEPROM.h>

/** \file aDCLoad.h
    \author F1RMB, Daniel Caujolle-Bert <f1rmb.daniel@gmail.com>
    \author Lee Wiggins <lee@wigweb.com.au>
*/

/*
    Calibrations:

    RMB:
        :CAL:V:12.0164739328,0.0341809174
        :CAL:C:0.9597601497,0.0211903263
        :CAL:D:0.522215448,7.4469088399

      twin:
        :CAL:V:12.0165258544,0.029649335
        :CAL:C:0.9602825049,0.0367962659
        :CAL:D:0.5218646521,3.9098266577
        :CAL:VD:0.013555,0.000

    JKY:
        :CAL:V:
        :CAL:C:
        :CAL:D:

    AZZ:
        :CAL:V:
        :CAL:C:
        :CAL:D:

*/

#define MAX_POWER                                 1                ///< Define this if you want 192W support (otherwise 50W)
//#define SIMU 1                                              ///< Define this if you want to simulate ADC/DAC/whatever (debug mode)

/**
 * Define this if you want to display Resistance settings.
 * Undefine this to enable PULSE feature.
 */
#undef RESISTANCE

// Set Constants
static const uint8_t       ADC_CHIPSELECT_PIN          = 8;        ///< set pin 8 as the chip select for the ADC:
static const uint8_t       ADC_INPUTVOLTAGE_CHAN       = 0;        ///< set the ADC channel that reads the input voltage.
static const uint8_t       ADC_MEASUREDCURRENT_CHAN    = 1;        ///< set the ADC channel that reads the input current by measuring the voltage on the input side of the sense resistors.
static const uint8_t       ADC_TEMPSENSE1_CHAN         = 2;        ///< set the ADC channel that reads the temperature sensor 1 under the heatsink.
static const uint8_t       ADC_TEMPSENSE2_CHAN         = 3;        ///< set the ADC channel that reads the temperature sensor 2 under the heatsink.

static const uint8_t       DAC_CHIPSELECT_PIN          = 9;        ///< set pin 9 as the chip select for the DAC:
static const uint8_t       DAC_CURRENT_CHAN            = 0;        ///< set The DAC channel that sets the constant current.
static const uint8_t       DAC_FAN_CHAN                = 1;        ///< set The DAC channel that sets the fan speed.

static const uint8_t       LCD_RS_PIN                  = 10;       ///< LCD RS pin.
static const uint8_t       LCD_ENABLE_PIN              = 12;       ///< LCD ENABLE pin.

/** \warning pin 5 shouldn't be used, since Timer3 use it
*/
static const uint8_t       LCD_D4_PIN                  = A0;       ///< LCD d4 pin.
/** \warning pin 5 shouldn't be used, since Timer3 use it
*/
static const uint8_t       LCD_D5_PIN                  = A1;       ///< LCD d5 pin.
/** \warning pin 5 shouldn't be used, since Timer3 use it
*/
static const uint8_t       LCD_D6_PIN                  = A2;       ///< LCD d6 pin.
/** \warning pin 5 shouldn't be used, since Timer3 use it
*/
static const uint8_t       LCD_D7_PIN                  = A3;       ///< LCD d7 pin.

static const uint8_t       LCD_COLS_NUM                = 20;       ///< LCD columns size
static const uint8_t       LCD_ROWS_NUM                = 4;        ///< LCD rows size

static const uint8_t       ENCODER_A_PIN               = 3;        ///< Encoder Channel A pin, INT 0
static const uint8_t       ENCODER_B_PIN               = 2;        ///< Encoder Channel B pin, INT 1
static const uint8_t       ENCODER_PB_PIN              = 0;        ///< Encoder push button pin, INT 2
static const uint8_t       ENCODER_STEPS_PER_NOTCH     = 4;        ///< Depending on the type of your encoder, you can define use the constructors parameter `stepsPerNotch` an set it to either `1`, `2` or `4` steps per notch, with `1` being the default.

static const uint8_t       LED_BACKLIGHT_PIN           = 11;       ///< LCD backlight pin.

// Values offsets
static const uint8_t       OFFSET_UNIT                 = 1;        ///< Unit column LCD offset
static const uint8_t       OFFSET_VALUE                = 4;        ///< Value column LCD offset
static const uint8_t       OFFSET_TEMP                 = 12;       ///< Temperature column LCD offset
static const uint8_t       OFFSET_MARKER_LEFT          = 0;        ///< Column LCD offset for left marker '['
static const uint8_t       OFFSET_MARKER_RIGHT         = 14;       ///< Column LCD offset for right marker ']'
static const uint8_t       OFFSET_SETUP_MARKER_LEFT    = 1;        ///< Column LCD offset for left marker '[' in setup mode
static const uint8_t       OFFSET_SETUP_MARKER_RIGHT   = 18;       ///< Column LCD offset for right marker ']' in setup mode

// Icons and alarm coords
static const uint8_t       LOGGING_ICON_X_COORD        = 17;       ///< Logging icon LCD X coord
static const uint8_t       LOGGING_ICON_Y_COORD        = 3;        ///< Logging icon LCD Y coord
static const uint8_t       USB_ICON_X_COORD            = 18;       ///< USB icon LCD X coord
static const uint8_t       USB_ICON_Y_COORD            = 3;        ///< USB icon LCD Y coord
static const uint8_t       LOCK_ICON_X_COORD           = 19;       ///< LOCK icon LCD X coord
static const uint8_t       LOCK_ICON_Y_COORD           = 3;        ///< LOCK icon LCD Y coord
static const uint8_t       ALARM_OV_X_COORD            = 18;       ///< Over-voltage 'OV' text LCD X coord
static const uint8_t       ALARM_OV_Y_COORD            = 1;        ///< Over-voltage 'OV' text LCD Y coord
static const uint8_t       ALARM_OC_X_COORD            = 18;       ///< Over-current 'OC' text LCD X coord
static const uint8_t       ALARM_OC_Y_COORD            = 2;        ///< Over-current 'OC' text LCD Y coord
static const uint8_t       ALARM_OT_X_COORD            = 18;       ///< Over-temperature 'OT' text LCD X coord
static const uint8_t       ALARM_OT_Y_COORD            = 2;        ///< Over-temperature 'OT' text LCD Y coord

// Autolock
static const unsigned long AUTOLOCK_TIMEOUT            = 60000;    ///< Autolock timeout value (60 seconds)

// Settings to Operation mode switch timeout
static const unsigned long OPERATION_SET_TIMEOUT       = 3000;     ///< Automatic toggle settings->reading timeout (3 seconds)

// Backlight
static const unsigned long BACKLIGHT_TIMEOUT           = 600000;   ///< Backlight dimmer timeout (10 minutes)

// Logging rate
static const unsigned long LOGGING_RATE                = 100;      ///< CSV data-logging rate (ms)

// Display update rate
static const unsigned long DISPLAY_UPDATE_RATE         = 200;      ///< Display update rate (ms)

// Set maximum values.
static const float         VOLTAGE_MAXIMUM             = 24.000;   ///< Maximum handled voltage (V)
static const float         CURRENT_MAXIMUM             = 7.845;    ///< Maximum value of load current (A)

#ifdef MAX_POWER
static const float         POWER_MAXIMUM               = VOLTAGE_MAXIMUM * CURRENT_MAXIMUM;   ///< Maximum power dissipated (W)
#else
static const float         POWER_MAXIMUM               = 50.000;   ///< Maximum power dissipated (W)
#endif // MAX_POWER
#ifdef RESISTANCE
static const float         RESISTANCE_MAXIMUM          = FLT_MAX;  ///< Maximum resistance value (R)
#else
static const float         PULSE_MAXIMUM               = 8.192;   ///< Maximum pulse value (ms) (unsigned 16-bit)
#endif // RESISTANCE
static const uint16_t      TEMPERATURE_MAXIMUM         = 80;      ///< Over-temperature threshold

// Software version
static const int8_t        SOFTWARE_VERSION_MAJOR      = 2;        ///< Software major version
static const int8_t        SOFTWARE_VERSION_MINOR      = 6;        ///< Software minor version

// Features bitfield (max 16)
static const uint16_t      FEATURE_LOGGING             = 1;        ///< Bitfield logging feature
static const uint16_t      FEATURE_LOGGING_VISIBLE     = 1 << 1;   ///< Bitfield logging icon feature
static const uint16_t      FEATURE_USB                 = 1 << 2;   ///< Bitfield USB feature
static const uint16_t      FEATURE_USB_VISIBLE         = 1 << 3;   ///< Bitfield USB icon feature
static const uint16_t      FEATURE_LOCKED              = 1 << 4;   ///< Bitfield locking feature
static const uint16_t      FEATURE_LOCKED_VISIBLE      = 1 << 5;   ///< Bitfield locking icon feature
static const uint16_t      FEATURE_AUTOLOCK            = 1 << 6;   ///< Bitfield auto-lock setting feature
static const uint16_t      FEATURE_AUTOLOCK_VISIBLE    = 1 << 7;   ///< Bitfield auto-lock setting icon feature
static const uint16_t      FEATURE_AUTODIM             = 1 << 8;   ///< Bitfield auto-dimmer setting feature
static const uint16_t      FEATURE_AUTODIM_VISIBLE     = 1 << 9;   ///< Bitfield auto-dimmer setting icon feature
static const uint16_t      FEATURE_OVP                 = 1 << 10;  ///< Bitfield over-voltage protection feature
static const uint16_t      FEATURE_OVP_VISIBLE         = 1 << 11;  ///< Bitfield over-voltage protection icon feature
static const uint16_t      FEATURE_OCP                 = 1 << 12;  ///< Bitfield over-current protection feature
static const uint16_t      FEATURE_OCP_VISIBLE         = 1 << 13;  ///< Bitfield over-current protection icon feature
static const uint16_t      FEATURE_OTP                 = 1 << 14;  ///< Bitfield over-temperature protection feature
static const uint16_t      FEATURE_OTP_VISIBLE         = 1 << 15;  ///< Bitfield over-temperature protection icon feature

// Glyph offsets
static const uint8_t       GLYPH_X1                    = 0;        ///< Offset of *1 icon in LCD HD44780 controller memory
static const uint8_t       GLYPH_X10                   = 1;        ///< Offset of *10 icon in LCD HD44780 controller memory
static const uint8_t       GLYPH_X100                  = 2;        ///< Offset of *100 icon in LCD HD44780 controller memory
static const uint8_t       GLYPH_X1000                 = 3;        ///< Offset of *1000 icon in LCD HD44780 controller memory
static const uint8_t       GLYPH_USB                   = 4;        ///< Offset of USB icon in LCD HD44780 controller memory
static const uint8_t       GLYPH_LOCK                  = 5;        ///< Offset of LOCK icon in LCD HD44780 controller memory
static const uint8_t       GLYPH_CHECKBOX_UNTICKED     = 6;        ///< Offset of unticked checkbox icon in LCD HD44780 controller memory
static const uint8_t       GLYPH_CHECKBOX_TICKED       = 7;        ///< Offset of ticked checkbox icon in LCD HD44780 controller memory

// EEPROM
static const int16_t       EEPROM_ADDR_MAGIC               = 0;        ///< EEPROM start offset to magic numbers (0xDEAD)
static const int16_t       EEPROM_ADDR_AUTODIM             = 4;        ///< EEPROM start offset for autodimming setting
static const int16_t       EEPROM_ADDR_AUTOLOCK            = 5;        ///< EEPROM start offset for autolocking setting
static const int16_t       EEPROM_CALIBRATION_SIZE              = (sizeof(float) * 2) + sizeof(uint8_t); ///< EEPROM calibration size: 2 float (slope & offset), and one uint8_t for crc
static const int16_t       EEPROM_ADDR_CALIBRATION_VOLTAGE      = EEPROM_ADDR_AUTOLOCK + 1; ///< EEPROM start offset for voltage calibration values
static const int16_t       EEPROM_ADDR_CALIBRATION_READ_CURRENT = EEPROM_ADDR_CALIBRATION_VOLTAGE + EEPROM_CALIBRATION_SIZE; ///< EEPROM start offset for current calibration values
static const int16_t       EEPROM_ADDR_CALIBRATION_DAC_CURRENT  = EEPROM_ADDR_CALIBRATION_READ_CURRENT + EEPROM_CALIBRATION_SIZE; ///< EEPROM start offset for DAC calibration values
static const int16_t       EEPROM_ADDR_CALIBRATION_VOLTAGE_DROP = EEPROM_ADDR_CALIBRATION_DAC_CURRENT + EEPROM_CALIBRATION_SIZE; ///< EEPROM start offset for DAC calibration values


typedef void (*ISRCallback)();                                     ///< Function prototype for ISR callback


/**
*** Classes declarations
**/

/** \brief Class that handle increase/decrease step multiplier
 */
class aStepper
{
    private:
        static const uint8_t MAX_VALUE = 3;

    public:
        aStepper();
        ~aStepper();

        void                increment();
        uint8_t             getValue();
        void                reset();
        int16_t             getMult();
        int16_t             getValueFromMode(uint8_t);
        bool                isSynced();
        void                sync();

    private:
        inline int16_t      _pow(int, int);

    private:
        uint8_t             m_inc, m_incPrev; ///< Stepper counters
};


/** \brief Class that handle settings.
 */
class aDCSettings : public aStepper
{
    public:
        // Operation Modes
        /** \brief Operation mode enumeration
         *
         */
        typedef enum
        {
            OPERATION_MODE_READ,        ///< Reading values
            OPERATION_MODE_SET,         ///< Settings values
            OPERATION_MODE_UNKNOWN      ///< Unset (internal)
        } OperationMode_t;

        // Selection Modes
        /** \brief Selection settings mode enumeration
         *
         *
         */
        typedef enum
        {
            SELECTION_MODE_CURRENT,     ///< Current selected
            SELECTION_MODE_POWER,       ///< Power selected
#ifdef RESISTANCE
            SELECTION_MODE_RESISTANCE,  ///< Resistance selected
#else
            SELECTION_MODE_PULSE,       ///< Pulse selected
#endif
            SELECTION_MODE_UNKNOWN      ///< Nothing selected (internal)
        } SelectionMode_t;

        // Display Modes
        /** \brief Display mode enumeration
         *
         */
        typedef enum
        {
            DISPLAY_MODE_VALUES,        ///< Display read/set values mode
            DISPLAY_MODE_SETUP,         ///< Display settings mode
            DISPLAY_MODE_UNKNOWN        ///< Display in undefined (internal)
        } DisplayMode_t;


        /** \brief Setting error enumeration
         *
         */
        typedef enum
        {
            SETTING_ERROR_OVERSIZED,    ///< Setting value is oversized
            SETTING_ERROR_UNDERSIZED,   ///< Setting value is undersized
            SETTING_ERROR_VALID         ///< Setting value is valid
        } SettingError_t;

        /** \brief Calibration values
        *
        * Contains Slope and Offset float values
        *
        */
        typedef struct
        {
            float slope;                ///< Slope value
            float offset;               ///< Offset value
        } CalibrationData_t;

        /** \brief Calibration offset enumeration
        *
        * Used to get/set calibration values
        *
        */
        typedef enum
        {
            CALIBRATION_VOLTAGE,        ///< Voltage calibration offset value
            CALIBRATION_READ_CURRENT,   ///< Readed Current calibration offset value
            CALIBRATION_DAC_CURRENT,    ///< DAC Current calibration offset value
            CALIBRATION_VOLTAGE_DROP,    ///< Voltage drop calibration offset value
            CALIBRATION_MAX             ///< Maximum offset in calibration array
        } CalibrationValues_t;

        static const uint16_t   DATA_VOLTAGE            = 1;        ///< bit-field storage: Voltage readed
        static const uint16_t   DATA_CURRENT_SETS       = 1 << 1;   ///< bit-field storage: Current sets
        static const uint16_t   DATA_CURRENT_READ       = 1 << 2;   ///< bit-field storage: Current readed
#ifdef RESISTANCE
        static const uint16_t   DATA_RESISTANCE_SETS    = 1 << 3;   ///< bit-field storage: Resistance sets
        static const uint16_t   DATA_RESISTANCE_READ    = 1 << 4;   ///< bit-field storage: Resistance readed
#else
        static const uint16_t   DATA_PULSE_SETS         = 1 << 3;
#endif
        static const uint16_t   DATA_POWER_SETS         = 1 << 5;   ///< bit-field storage: Power sets
        static const uint16_t   DATA_POWER_READ         = 1 << 6;   ///< bit-field storage: Power readed
        static const uint16_t   DATA_TEMPERATURE        = 1 << 7;   ///< bit-field storage: Temperature readed
        static const uint16_t   DATA_SELECTION          = 1 << 8;   ///< bit-field storage: Selection mode sets
        static const uint16_t   DATA_DISPLAY            = 1 << 9;   ///< bit-field storage: Display mode sets
        static const uint16_t   DATA_ENCODER            = 1 << 10;  ///< bit-field storage: Encoder position sets
        static const uint16_t   DATA_OPERATION          = 1 << 11;  ///< bit-field storage: Operation mode sets
        static const uint16_t   DATA_IN_CALIBRATION     = 1 << 12;  ///< bit-field storage: In calibration mode

    private:
        /** \brief Union to manipulate float/uint8_t [] calibration values
         *
         */
        union _eepromCalibrationValue_t
        {
            float v;
            uint8_t c[sizeof(float)];
        };

    public:
        aDCSettings();
        ~aDCSettings();

        // Voltage
        SettingError_t      setVoltage(float);
        float               getVoltage();

        // Current
        SettingError_t      setCurrent(float, OperationMode_t);
        float               getCurrent(OperationMode_t);

#ifdef RESISTANCE
        // Resistance
        SettingError_t      setResistance(float, OperationMode_t);
        float               getResistance(OperationMode_t);
#else
        // Pulse
        SettingError_t      setPulse(float);
        float               getPulse();
        bool                isPulseEnabled() { return m_pulseEnabled; }
        void                enablePulse(bool enable) { m_pulseEnabled = enable; }
        bool                isPulseHigh() { return m_pulseHigh; }
        void                setPulseHigh(bool high) { m_pulseHigh = high; }
#endif

        // Power
        SettingError_t      setPower(float, OperationMode_t);
        float               getPower(OperationMode_t);

        void                updateValuesFromMode(float, SelectionMode_t);

        // Temperature
        void                setTemperature(uint16_t);
        uint16_t            getTemperature();

        // Fan
        void                setFanSpeed(uint16_t);
        uint16_t            getFanSpeed();

        // DAC (current)
        void                setCurrentDAC(uint16_t);
        uint16_t            getCurrentDAC();

        // Mode
        void                setSelectionMode(SelectionMode_t, bool = false);
        SelectionMode_t     getSelectionMode();
        SelectionMode_t     getPrevNextMode(SelectionMode_t = SELECTION_MODE_UNKNOWN, bool = true);

        // Display Mode
        void                setDisplayMode(DisplayMode_t);
        DisplayMode_t       getDisplayMode();

        // Encoder
        void                setEncoderPosition(int32_t);
        void                incEncoderPosition(int32_t = 1);
        int32_t             getEncoderPosition();

        // Operation Mode
        void                setOperationMode(OperationMode_t);
        OperationMode_t     getOperationMode();
        void                updateOperationMode();
        void                pingOperationMode();

        // Autolock
        void                pingAutolock();
        bool                isAutolocked();

        // Calibation
        void                setCalibationMode(bool = true);
        bool                getCalibrationMode();

        void                getCalibrationValues(CalibrationValues_t, CalibrationData_t &);
        void                setCalibrationValues(CalibrationValues_t, CalibrationData_t);
        void                backupCalibration();
        void                restoreCalibration();

        // Alarm
        void                enableAlarm(uint16_t bit);

        // Features
        void                enableFeature(uint16_t, bool = true);
        bool                isFeatureEnabled(uint16_t);

        bool                isDataEnabled(uint16_t);
        void                syncData(uint16_t);

    private:
        SettingError_t      _setValue(OperationMode_t, uint16_t, float, float &, float &, float);
        uint8_t             _crc8(const uint8_t *, uint8_t);
        void                _eepromCalibrationRestore(int16_t, CalibrationData_t &);
        void                _eepromCalibrationBackup(int16_t, CalibrationData_t);
        bool                _eepromCheckForMagicNumbers();
        void                _eepromWriteMagicNumbers();
        void                _eepromReset();
        void                _eepromRestore();
        void                _enableData(uint16_t, bool);
        void                _enableDataCheck(uint16_t, bool);

    private:
        float               m_readVoltage;                          ///< voltage storage
        float               m_setsCurrent, m_readCurrent;           ///< current storage
#ifdef RESISTANCE
        float               m_setsResistance, m_readResistance;     ///< resistance storage
#else
        float               m_setsPulse;                            ///< Pulse time length (ms)
#endif
        float               m_setsPower, m_readPower;               ///< power storage
        uint16_t            m_readTemperature;                      ///< temperature storage

        uint16_t            m_fanSpeed;                             ///< fan speed storage

        uint16_t            m_currentDAC;                           ///< current DAC value

        OperationMode_t     m_operationMode;                        ///< operation mode @see OperationMode

        SelectionMode_t     m_mode;                                 ///< selection mode @see SelectionMode
        int32_t             m_encoderPos;                           ///< encoder position (yeah, 32bits, due to resistance max val (12000 * 1000);

        DisplayMode_t       m_dispMode;                             ///< display mode @see DisplayMode

        unsigned long       m_lockTick;                             ///< tick count storage, for autolock feature
        unsigned long       m_operationTick;                        ///< tick count storage, for SET/READ operation

        uint16_t            m_features;                             ///< boolean features storage
        uint16_t            m_datas;                                ///< boolean displayes data storage

        CalibrationData_t   m_calibrationValues[CALIBRATION_MAX];   ///< Calibration datas, restored from EEPROM

#ifndef RESISTANCE
        volatile bool       m_pulseEnabled;                         ///< Pulse is enabled
        volatile bool       m_pulseHigh;                            ///< Pulse is high ?
#endif
};


/** \brief LiquidDisplay extension class
 */
class aLCD : public LiquidCrystal
{
    private:
        //static const unsigned long SCROLL_DELAY = 300; // Disabled due to memory footprint

    public:
        aLCD(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
        ~aLCD();

        void                begin(uint8_t, uint8_t);
        void                setCursor(uint8_t, uint8_t);
        void                printCenter(const char *);
        void                printCenter(const __FlashStringHelper *);
        void                clearValue(uint8_t, int = 0);

    private:
        uint8_t             m_cols, m_rows;     ///< LCD sizes
        uint8_t             m_curCol, m_curRow; ///< Current cursor position
};

class aDCEngine;

/** \brief Class that handle LCD displaying
 */
class aDCDisplay : public aLCD
{
    public:
        aDCDisplay(aDCEngine *, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
        ~aDCDisplay();

        void                setup();
        void                showBanner();
        void                updateField(aDCSettings::OperationMode_t, float, float, uint8_t, uint8_t);

        void                updateDisplay();
        void                pingBacklight();
        bool                isBacklightDimmed();

    private:
        void                _dimBacklight(bool);
        void                _dimmingBacklight();
        void                _wakeupBacklight();

    private:
        aDCEngine          *m_Parent;       ///< Pointer to aDCEngine parent
        bool                m_dimmed;       ///< Dimmed state storage
        unsigned long       m_dimmerTick;   ///< Dimmer timeout tick counter
        unsigned long       m_nextUpdate;
};

/** \brief Main class
 */
class aDCEngine : public aDCDisplay
{
    friend class aDCDisplay;

    private:
        static const uint8_t RXBUFFER_MAXLEN = 64;

    public:
        aDCEngine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t = 1);
        ~aDCEngine();

        void                setup(ISRCallback);
        void                run();
        void                service();
        const aDCSettings  *getSettings() const;

    private:
        void                _handleButtonEvent(ClickEncoder::Button);
        float               _getInputVoltage();
        float               _getADC(uint8_t);
        void                _setDAC(uint16_t, uint8_t);
        int16_t             _getTemp();
        void                _updateFanSpeed();
        float               _getMeasuredCurrent();
        void                _updateLoadCurrent();
        void                _adjustLoadCurrent();
        void                _handleLoggingAndRemote();

    private:
        aDCSettings         m_Data;                         ///< Settings object
        ClickEncoder       *m_encoder;                      ///< Encoder object
        uint8_t             m_RXbuffer[RXBUFFER_MAXLEN];    ///< USB rx buffer
        uint8_t             m_RXoffset;                     ///< USB rx buffer offset counter
};

#endif // ADCLOAD_H
