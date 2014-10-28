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
#ifndef ADCLOAD_H
#define ADCLOAD_H

#include <float.h>
#include <LiquidCrystal.h>
#include <ClickEncoder.h>

/** \file aDCLoad.h
    \author Lee Wiggins <lee@wigweb.com.au>
    \author F1RMB, Daniel Caujolle-Bert <f1rmb.daniel@gmail.com>
*/

//#define SIMU 1
#define RESISTANCE 1                                        ///< Define this if you want to display Resistance settings

// Set Constants
const uint8_t       ADC_CHIPSELECT_PIN          = 8;        ///< set pin 8 as the chip select for the ADC:
const uint8_t       ADC_INPUTVOLTAGE_CHAN       = 0;        ///< set the ADC channel that reads the input voltage.
const uint8_t       ADC_MEASUREDCURRENT_CHAN    = 1;        ///< set the ADC channel that reads the input current by measuring the voltage on the input side of the sense resistors.
const uint8_t       ADC_TEMPSENSE1_CHAN         = 2;        ///< set the ADC channel that reads the temprature sensor 1 under the heatsink.
const uint8_t       ADC_TEMPSENSE2_CHAN         = 3;        ///< set the ADC channel that reads the temprature sensor 2 under the heatsink.

const uint8_t       DAC_CHIPSELECT_PIN          = 9;        ///< set pin 9 as the chip select for the DAC:
const uint16_t      DAC_CURRENT_CHAN            = 0;        ///< set The DAC channel that sets the constant current.
const uint16_t      DAC_FAN_CHAN                = 1;        ///< set The DAC channel that sets the fan speed.

const uint8_t       LCD_RS_PIN                  = 10;       ///< LCD RS pin.
const uint8_t       LCD_ENABLE_PIN              = 12;       ///< LCD ENABLE pin.
const uint8_t       LCD_D0_PIN                  = 4;        ///< LCD d0 pin.
const uint8_t       LCD_D1_PIN                  = 13;       ///< LCD d1 pin.
const uint8_t       LCD_D2_PIN                  = 6;        ///< LCD d2 pin.
const uint8_t       LCD_D3_PIN                  = 5;        ///< LCD d3 pin.
const uint8_t       LCD_COLS_NUM                = 20;       ///< LCD columns size
const uint8_t       LCD_ROWS_NUM                = 4;        ///< LCD rows size

const uint8_t       ENCODER_A_PIN               = 3;        ///< set pin 3 as the channel A for encoder 1, int.0:
const uint8_t       ENCODER_B_PIN               = 2;        ///< set pin 2 as the channel B for encoder 1, int.1:
#ifdef SIMU
const uint8_t       ENCODER_PB_PIN              = 21;       ///< set pin 21 as the push button for encoder 1
#else
const uint8_t       ENCODER_PB_PIN              = 0;        ///< set pin 0 as the push button for encoder 1
#endif // SIMU
const uint8_t       ENCODER_STEPS_PER_NOTCH     = 4;        ///< Depending on the type of your encoder, you can define use the constructors parameter `stepsPerNotch` an set it to either `1`, `2` or `4` steps per notch, with `1` being the default.

const uint8_t       LED_BACKLIGHT_PIN           = 11;       ///< LCD backlight pin.

// Values offsets
const uint8_t       OFFSET_UNIT                 = 1;        ///< Unit column LCD offset
const uint8_t       OFFSET_VALUE                = 4;        ///< Value column LCD offset
const uint8_t       OFFSET_TEMP                 = 12;       ///< Temperature column LCD offset
const uint8_t       OFFSET_MARKER_LEFT          = 0;        ///< Column LCD offset for left marker '['
const uint8_t       OFFSET_MARKER_RIGHT         = 14;       ///< Column LCD offset for right marker ']'
const uint8_t       OFFSET_SETUP_MARKER_LEFT    = 1;        ///< Column LCD offset for left marker '[' in setup mode
const uint8_t       OFFSET_SETUP_MARKER_RIGHT   = 18;       ///< Column LCD offset for right marker ']' in setup mode

// Icons and alarm coords
const uint8_t       LOGGING_ICON_X_COORD        = 17;       ///< Logging icon LCD X coord
const uint8_t       LOGGING_ICON_Y_COORD        = 3;        ///< Logging icon LCD Y coord
const uint8_t       USB_ICON_X_COORD            = 18;       ///< USB icon LCD X coord
const uint8_t       USB_ICON_Y_COORD            = 3;        ///< USB icon LCD Y coord
const uint8_t       LOCK_ICON_X_COORD           = 19;       ///< LOCK icon LCD X coord
const uint8_t       LOCK_ICON_Y_COORD           = 3;        ///< LOCK icon LCD Y coord
const uint8_t       ALARM_OV_X_COORD            = 18;       ///< Overvoltage 'OV' text LCD X coord
const uint8_t       ALARM_OV_Y_COORD            = 1;        ///< Overvoltage 'OV' text LCD Y coord
const uint8_t       ALARM_OC_X_COORD            = 18;       ///< Overcurrent 'OC' text LCD X coord
const uint8_t       ALARM_OC_Y_COORD            = 2;        ///< Overcurrent 'OC' text LCD Y coord

// Autolock
const unsigned long AUTOLOCK_TIMEOUT            = 60000;    ///< Autolock timeout value (60 seconds)

// Settings to Operation mode switch timeout
const unsigned long OPERATION_SET_TIMEOUT       = 3000;     ///< Automatic toggle settings->reading timeout (3 seconds)

// Backlight
const unsigned long BACKLIGHT_TIMEOUT           = 300000;   ///< Backlight dimmer timeout (5 minutes)

// Set maximum values.
const float         VOLTAGE_MAXIMUM             = 24.000;   ///< Maximum handled voltage (V)
const float         CURRENT_MAXIMUM             = 8.000;    ///< Maximum value of load current (A)
const float         POWER_MAXIMUM               = 50.000;   ///< Maximum power dissipated (W)
#ifdef RESISTANCE
const float         RESISTANCE_MAXIMUM          = FLT_MAX;  ///< Maximum resistance value (R)
#endif

// Software version
const int8_t        SOFTWARE_VERSION_MAJOR      = 2;        ///< Software major version
const int8_t        SOFTWARE_VERSION_MINOR      = 0;        ///< Software minor version

// Features bitfield (max 16)
const uint16_t      FEATURE_LOGGING             = 1;        ///< Bitfield logging feature
const uint16_t      FEATURE_LOGGING_VISIBLE     = 1 << 1;   ///< Bitfield logging icon feature
const uint16_t      FEATURE_USB                 = 1 << 2;   ///< Bitfield USB feature
const uint16_t      FEATURE_USB_VISIBLE         = 1 << 3;   ///< Bitfield USB icon feature
const uint16_t      FEATURE_LOCKED              = 1 << 4;   ///< Bitfield locking feature
const uint16_t      FEATURE_LOCKED_VISIBLE      = 1 << 5;   ///< Bitfield locking icon feature
const uint16_t      FEATURE_AUTOLOCK            = 1 << 6;   ///< Bitfield autolock setting feature
const uint16_t      FEATURE_AUTOLOCK_VISIBLE    = 1 << 7;   ///< Bitfield autolock setting icon feature
const uint16_t      FEATURE_AUTODIM             = 1 << 8;   ///< Bitfield autodimmer setting feature
const uint16_t      FEATURE_AUTODIM_VISIBLE     = 1 << 9;   ///< Bitfield autodimmer setting icon feature
const uint16_t      FEATURE_OVP                 = 1 << 10;  ///< Bitfield overvoltage protection feature
const uint16_t      FEATURE_OVP_VISIBLE         = 1 << 11;  ///< Bitfield overvoltage protection icon feature
const uint16_t      FEATURE_OCP                 = 1 << 12;  ///< Bitfield overcurrent protection feature
const uint16_t      FEATURE_OCP_VISIBLE         = 1 << 13;  ///< Bitfield overcurrent protection icon feature

// Glyph offsets
const uint8_t       GLYPH_X1                    = 0;        ///< Offset of *1 icon in LCD HD44780 controller memory
const uint8_t       GLYPH_X10                   = 1;        ///< Offset of *10 icon in LCD HD44780 controller memory
const uint8_t       GLYPH_X100                  = 2;        ///< Offset of *100 icon in LCD HD44780 controller memory
const uint8_t       GLYPH_X1000                 = 3;        ///< Offset of *1000 icon in LCD HD44780 controller memory
const uint8_t       GLYPH_USB                   = 4;        ///< Offset of USB icon in LCD HD44780 controller memory
const uint8_t       GLYPH_LOCK                  = 5;        ///< Offset of LOCK icon in LCD HD44780 controller memory
const uint8_t       GLYPH_CHECKBOX_UNTICKED     = 6;        ///< Offset of unticked checkbox icon in LCD HD44780 controller memory
const uint8_t       GLYPH_CHECKBOX_TICKED       = 7;        ///< Offset of ticked checkbox icon in LCD HD44780 controller memory

// EEPROM
const int16_t       EEPROM_ADDR_MAGIC           = 0;        ///< EEPROM start offset to magic numbers (0xDEAD)
const int16_t       EEPROM_ADDR_AUTODIM         = 4;        ///< EEPROM start offset for autodimming setting
const int16_t       EEPROM_ADDR_AUTOLOCK        = 5;        ///< EEPROM start offset for autolocking setting

typedef void (*ISRCallback)();                              ///< Function prototype for ISR callback

// Display Modes
/** \brief Display mode enumeration
 *
 */
typedef enum
{
    DISPLAY_VALUES, /**< Display read/set values mode */
    DISPLAY_SETUP,  /**< Display settings mode */
    DISPLAY_UNKNOWN /**< Display in undefined (internal) */
} DisplayMode;

// Selection Modes
/** \brief Selection settings mode enumeration
 *
 *
 */
typedef enum
{
    SELECTION_CURRENT, /**< Current selected */
#ifdef RESISTANCE
    SELECTION_RESISTANCE, /**< Resistance selected */
#endif
    SELECTION_POWER, /**< Power selected */
    SELECTION_UNKNOWN /**< Nothing selected (internal) */
} SelectionMode;

// Operation Modes
/** \brief Operation mode enumeration
 *
 */
typedef enum
{
    OPERATION_READ, /**< Reading values */
    OPERATION_SET, /**< Settings values */
    OPERATION_UNKNOWN /**< Unset (internal) */
} OperationMode;


/**
*** Class declarations
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

        void                incIncrement();
        uint8_t             incGetValue();
        void                incReset();
        int16_t             incGetMult();
        int16_t             incGetValueFromMode(SelectionMode);
        bool                incIsSynced();
        void                incSync();

    private:
        int16_t             _pow(int, int);

    private:
        uint8_t             m_inc, m_incPrev;
};

/** \brief Class that handle settings.
 */
class aDCSettings : public aStepper
{
    public:
        /** \brief Setting error enumeration
         *
         */

        typedef enum
        {
            SETTING_OVERSIZED, ///< Setting value is oversized
            SETTING_UNDERSIZED, ///< Setting value is undersized
            SETTING_VALID ///< Setting value is valid
        } SettingError;

    public:
        aDCSettings();
        ~aDCSettings();

        // Voltage
        SettingError        setVoltageRead(float);
        float               getVoltageRead();
        void                syncVoltageDisp();
        bool                isVoltageAlreadyDisplayed();

        // Current
        SettingError        setCurrentSets(float);
        float               getCurrentSets();
        SettingError        setCurrentRead(float);
        float               getCurrentRead();
        void                syncCurrentDisp();
        bool                isCurrentAlreadyDisplayed();

#ifdef RESISTANCE
        // Resistance
        SettingError        setResistanceSets(float);
        float               getResistanceSets();
        void                setResistanceRead(float);
        float               getResistanceRead();
        void                syncResistanceDisp();
        bool                isResistanceAlreadyDisplayed();
#endif

        // Power
        SettingError        setPowerSets(float);
        float               getPowerSets();
        SettingError        setPowerRead(float);
        float               getPowerRead();
        void                syncPowerDisp();
        bool                isPowerAlreadyDisplayed();

        void                updateValuesFromMode(float, SelectionMode);

        // Temperature
        void                setTemperatureRead(uint8_t);
        uint8_t             getTemperatureRead();
        void                syncTemperatureDisp();
        bool                isTemperatureAlreadyDisplayed();

        // Fan
        void                setFanSpeed(uint16_t);
        uint16_t            getFanSpeed();

        // Mode
        void                setSelectionMode(SelectionMode);
        SelectionMode       getSelectionMode();
        SelectionMode       getNextMode(SelectionMode = SELECTION_UNKNOWN);
        SelectionMode       getPrevMode(SelectionMode = SELECTION_UNKNOWN);
        void                syncSelectionMode();
        bool                isSelectionModeChanged();

        // Display Mode
        void                setDisplayMode(DisplayMode);
        DisplayMode         getDisplayMode();
        void                syncDisplayMode();
        bool                isDisplayModeChanged();

        // Encoder
        void                setEncoderPosition(int32_t);
        void                incEncoderPosition(int32_t = 1);
        int32_t             getEncoderPosition();
        void                syncEncoderPosition();
        bool                isEncoderPositionChanged();

        // Operation Mode
        void                setOperationMode(OperationMode);
        OperationMode       getOperationMode();
        void                updateOperationMode();
        void                pingOperationMode();
        bool                isOperationModeChanged();
        void                syncOperationMode();

        // Autolock
        void                pingAutolock();
        bool                isAutolocked();

        // Features
        void                enableFeature(uint16_t, bool = true);
        bool                isFeatureEnabled(uint16_t);

    private:
        bool                _isEqual(float, float);
        bool                _isEqual(int32_t, int32_t);
        bool                _eepromCheckMagic();
        void                _eepromWriteMagic();
        void                _eepromReset();
        void                _eepromRestore();

    private:
        float               m_readVoltage, m_dispVoltage;                           // voltage storage
        float               m_setsCurrent, m_readCurrent, m_dispCurrent;            // current storage
#ifdef RESISTANCE
        float               m_setsResistance, m_readResistance, m_dispResistance;   // resistance storage
#endif
        float               m_setsPower, m_readPower, m_dispPower;                  // power storage
        uint8_t             m_readTemperature, m_dispTemperature;                   // temperature storage

        uint16_t            m_fanSpeed;                                             // fan speed storage

        OperationMode       m_operationMode, m_prevOperationMode;                   // operation mode (see OperationMode enum)

        SelectionMode       m_Mode, m_prevMode;                                     // selection mode (see SelectionMode enum)
        int32_t             m_EncoderPos, m_prevEncoderPos;                         // encoder position (yeah, 32bits, due to resistance max val (12000 * 1000);

        DisplayMode         m_dispMode, m_prevDispMode;                             // display mode (see DisplayMode enum)

        unsigned long       m_lockTick;                                             // tick count storage, for autolock feature
        unsigned long       m_operationTick;                                        // tick count storage, for SET/READ operation

        uint16_t            m_features;                                             // boolean features storage
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
        void                clearLine(uint8_t);
        void                clearValue(uint8_t, int = 0);

    private:
        uint8_t             m_cols, m_rows;
        uint8_t             m_curCol, m_curRow;
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
        void                updateField(OperationMode, float, float, uint8_t, uint8_t);

        void                updateDisplay();
        void                pingBacklight();
        bool                isBacklightDimmed();

    private:
        void                _dimBacklight(bool);
        void                _dimmingBacklight();
        void                _wakeupBacklight();

    private:
        aDCEngine          *m_Parent;
        bool                m_dimmed;
        unsigned long       m_dimmerTick;
};

/** \brief Main class
 */
class aDCEngine : public aDCDisplay
{
    friend class aDCDisplay;

    public:
        aDCEngine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t = 1);
        ~aDCEngine();

        void                setup(ISRCallback);
        void                run();
        void                service();

    private:
        void                _handleButtonEvent(ClickEncoder::Button);
        float               _readInputVoltage();
        float               _readADC(uint8_t);
        void                _setDAC(uint16_t, uint8_t);
        int8_t              _readTemp();
        void                _updateFanSpeed();
        float               _readMeasuredCurrent();
        void                _updateLoadCurrent();
        void                _adjustCurrent();
        const aDCSettings  *_getSettings() const;
        void                _updateLoggingAndRemote();

    private:
        aDCSettings         m_Data;
        ClickEncoder       *m_encoder;
        uint8_t             m_RXbuffer[63 + 1];
        uint8_t             m_RXoffset;
};

#endif // ADCLOAD_H
