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
#ifndef ADCLOAD_H
#define ADCLOAD_H

#include <float.h>
#include <LiquidCrystal.h>
#include <ClickEncoder.h>

//#define SIMU 1
#define RESISTANCE 1

// Set Constants
const uint8_t       ADC_CHIPSELECT_PIN          = 8;        // set pin 8 as the chip select for the ADC:
const uint8_t       ADC_INPUTVOLTAGE_CHAN       = 0;        // set the ADC channel that reads the input voltage.
const uint8_t       ADC_MEASUREDCURRENT_CHAN    = 1;        // set the ADC channel that reads the input current by measuring the voltage on the input side of the sense resistors.
const uint8_t       ADC_TEMPSENSE1_CHAN         = 2;        // set the ADC channel that reads the temprature sensor 1 under the heatsink.
const uint8_t       ADC_TEMPSENSE2_CHAN         = 3;        // set the ADC channel that reads the temprature sensor 2 under the heatsink.

const uint8_t       DAC_CHIPSELECT_PIN          = 9;        // set pin 9 as the chip select for the DAC:
const uint16_t      DAC_CURRENT_CHAN            = 0;        // set The DAC channel that sets the constant current.
const uint16_t      DAC_FAN_CHAN                = 1;        // set The DAC channel that sets the fan speed.

const uint8_t       LCD_RS_PIN                  = 10;
const uint8_t       LCD_ENABLE_PIN              = 12;
const uint8_t       LCD_D0_PIN                  = 4;
const uint8_t       LCD_D1_PIN                  = 13;
const uint8_t       LCD_D2_PIN                  = 6;
const uint8_t       LCD_D3_PIN                  = 5;
const uint8_t       LCD_COLS_NUM                = 20;
const uint8_t       LCD_ROWS_NUM                = 4;

const uint8_t       ENCODER_A_PIN               = 3;        // set pin 3 as the channel A for encoder 1, int.0:
const uint8_t       ENCODER_B_PIN               = 2;        // set pin 2 as the channel B for encoder 1, int.1:
#ifdef SIMU
const uint8_t       ENCODER_PB_PIN              = 21;       // set pin 21 as the push button for encoder 1
#else
const uint8_t       ENCODER_PB_PIN              = 0;        // set pin 0 as the push button for encoder 1
#endif // SIMU
const uint8_t       ENCODER_STEPS_PER_NOTCH     = 4;        // Depending on the type of your encoder, you can define use the
                                                            // constructors parameter `stepsPerNotch` an set it to
                                                            // either `1`, `2` or `4` steps per notch, with `1` being the default.

const uint8_t       LED_BACKLIGHT_PIN           = 11;       //LCD Backlight

// Values offsets
const uint8_t       OFFSET_UNIT                 = 1;
const uint8_t       OFFSET_VALUE                = 4;
const uint8_t       OFFSET_TEMP                 = 12;
const uint8_t       OFFSET_MARKER_LEFT          = 0;
const uint8_t       OFFSET_MARKER_RIGHT         = 14;
const uint8_t       OFFSET_SETUP_MARKER_LEFT    = 1;
const uint8_t       OFFSET_SETUP_MARKER_RIGHT   = 18;

// Icons and alarm coords
const uint8_t       LOGGING_ICON_X_COORD        = 17;
const uint8_t       LOGGING_ICON_Y_COORD        = 3;
const uint8_t       USB_ICON_X_COORD            = 18;
const uint8_t       USB_ICON_Y_COORD            = 3;
const uint8_t       LOCK_ICON_X_COORD           = 19;
const uint8_t       LOCK_ICON_Y_COORD           = 3;
const uint8_t       ALARM_OV_X_COORD            = 18;
const uint8_t       ALARM_OV_Y_COORD            = 1;
const uint8_t       ALARM_OC_X_COORD            = 18;
const uint8_t       ALARM_OC_Y_COORD            = 2;

// Autolock
const unsigned long AUTOLOCK_TIMEOUT            = 60000;    // 60s timeout

// Settings to Operation mode switch timeout
const unsigned long OPERATION_SET_TIMEOUT       = 3000;     // 3s timeout

// Backlight
const unsigned long BACKLIGHT_TIMEOUT           = 300000;   // 5m timeout

// Set maximum values.
const float         VOLTAGE_MAXIMUM             = 24.000;   // Maximum handled voltage (V)
const float         CURRENT_MAXIMUM             = 8.000;    // Maximum Value of load current (A)
const float         POWER_MAXIMUM               = 50.000;   // Maximum power dissipated (W)
#ifdef RESISTANCE
const float         RESISTANCE_MAXIMUM          = FLT_MAX;
#endif

// Software version
const int8_t        SOFTWARE_VERSION_MAJOR      = 2;        // Used for the current software version
const int8_t        SOFTWARE_VERSION_MINOR      = 0;

// Features bitfield (max 16)
const uint16_t      FEATURE_LOGGING             = 1;
const uint16_t      FEATURE_LOGGING_VISIBLE     = 1 << 1;
const uint16_t      FEATURE_USB                 = 1 << 2;
const uint16_t      FEATURE_USB_VISIBLE         = 1 << 3;
const uint16_t      FEATURE_LOCKED              = 1 << 4;
const uint16_t      FEATURE_LOCKED_VISIBLE      = 1 << 5;
const uint16_t      FEATURE_AUTOLOCK            = 1 << 6;
const uint16_t      FEATURE_AUTOLOCK_VISIBLE    = 1 << 7;
const uint16_t      FEATURE_AUTODIM             = 1 << 8;
const uint16_t      FEATURE_AUTODIM_VISIBLE     = 1 << 9;
const uint16_t      FEATURE_OVP                 = 1 << 10;
const uint16_t      FEATURE_OVP_VISIBLE         = 1 << 11;
const uint16_t      FEATURE_OCP                 = 1 << 12;
const uint16_t      FEATURE_OCP_VISIBLE         = 1 << 13;

// Glyph offsets
const uint8_t       GLYPH_X1                    = 0;
const uint8_t       GLYPH_X10                   = 1;
const uint8_t       GLYPH_X100                  = 2;
const uint8_t       GLYPH_X1000                 = 3;
const uint8_t       GLYPH_USB                   = 4;
const uint8_t       GLYPH_LOCK                  = 5;
const uint8_t       GLYPH_CHECKBOX_UNTICKED     = 6;
const uint8_t       GLYPH_CHECKBOX_TICKED       = 7;

// EEPROM
const int16_t       EEPROM_ADDR_MAGIC           = 0; // 0xDEAD
const int16_t       EEPROM_ADDR_AUTODIM         = 4;
const int16_t       EEPROM_ADDR_AUTOLOCK        = 5;

typedef void (*ISRCallback)();

// Display Modes
typedef enum
{
    DISPLAY_VALUES,
    DISPLAY_SETUP,
    DISPLAY_UNKNOWN
} DisplayMode;

// Selection Modes
typedef enum
{
    SELECTION_CURRENT,
#ifdef RESISTANCE
    SELECTION_RESISTANCE,
#endif
    SELECTION_POWER,
    SELECTION_UNKNOWN
} SelectionMode;

// Operation Modes
typedef enum
{
    OPERATION_READ,
    OPERATION_SET,
    OPERATION_UNKNOWN
} OperationMode;


/**
*** Class declarations
**/

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

class aDCSettings : public aStepper
{
    public:
        typedef enum
        {
            SETTING_OVERSIZED,
            SETTING_UNDERSIZED,
            SETTING_VALID
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


class aLCD : public LiquidCrystal
{
    private:
        static const unsigned long SCROLL_DELAY = 300;

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

class aDCEngine : public aDCDisplay
{
    friend class aDCDisplay;

    public:
        aDCEngine(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t = 1);
        ~aDCEngine();

        /**
        *** This initialize the engine, and must be called before anything else.
        **/
        void                setup(ISRCallback);

        /**
        *** This is the main loop.
        **/
        void                run();

        /**
        *** This should be called inside the ISR passed in ::setup() argument.
        **/
        void                service();

    private:
        /**
        *** Handle button event, according display mode
        **/
        void                _handleButtonEvent(ClickEncoder::Button);

        /**
        *** Function to read the input voltage and return a float number represention volts.
        **/
        float               _readInputVoltage();

        /**
        *** Function to read the ADC, accepts the channel to be read.
        **/
        float               _readADC(uint8_t);

        /**
        *** Function to set the DAC, Accepts the Value to be sent and the channel of the DAC to be used.
        **/
        void                _setDAC(uint16_t, uint8_t);

        /**
        *** Function to read heat sink temp
        **/
        int8_t              _readTemp();

        /**
        *** Function to set the fan speed depending on the heatsink temprature.
        **/
        void                _updateFanSpeed();

        /**
        *** Function to measure the actual load current.
        **/
        float               _readMeasuredCurrent();

        /**
        *** Function to calculate and set the required load current. Accepts the mode variable to determine if the constant current,
        *** resistance or power mode is to be used.
        **/
        void                _updateLoadCurrent();

        /**
        *** Adjust DAC to match current's setting
        **/
        void                _adjustCurrent();

        /**
        *** Returns pointer to current settings class.
        **/
        const aDCSettings  *_getSettings() const;

        /**
        *** Catch and handle RX and TX USB events.
        **/
        void                _updateLoggingAndRemote();

    private:
        aDCSettings         m_Data;
        ClickEncoder       *m_encoder;
        uint8_t             m_RXbuffer[64 + 1];
        uint8_t             m_RXoffset;
};

#endif // ADCLOAD_H
