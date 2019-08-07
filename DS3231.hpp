#pragma once
//
// The DS3231 driver for the WireMaster system
// ---------------------------------------------------------------------------
// (c)2018 by Lucky Resistor. See LICENSE for details.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//


#include "hal-common/BitTools.hpp"
#include "hal-common/DateTime.hpp"
#include "hal-common/Flags.hpp"
#include "hal-common/StatusTools.hpp"
#include "hal-common/String.hpp"
#include "hal-common/WireMasterChipRegister.hpp"


namespace lr {


/// A simple driver for the DS3231 real time clock.
///
class DS3231
{
public:
    /// The status of function calls
    ///
    using Status = CallStatus;

    /// The alarm mode.
    ///
    /// The modes `OncePerSecond` and `SecondsMatch` are only valid for alarm 1.
    /// The mode `OncePerMinute` is only valid for alarm 2 and is triggered if the seconds match 00.
    /// All other mothes are valid for both alarms, but for alarm 2 the second part is ignored.
    ///
    /// For the `DayHoursMinutesSeconds`, the function `Date::getDayOfWeek()` is used, instead of
    /// `DateTime::getDay()`. You can create a `DateTime` value using the
    /// `DateTime::fromUncheckedValues()` value for this use case.
    ///
    enum class AlarmMode : uint8_t {
        OncePerSecond = 0b01111, ///< Alarm once per second (only for alarm 1).
        SecondsMatch = 0b01110, ///< Alarm when seconds match (only for alarm 1).
        OncePerMinute = SecondsMatch, ///< Alarm once per minute (only for alarm 2).
        MinutesSeconds = 0b01100, ///< Alarm when minutes and seconds match.
        HoursMinutesSeconds = 0b01000, ///< Alarm when hours, minutes and seconds match.
        DateHoursMinutesSeconds = 0b00000, ///< Alarm when the date of the month, hours, minutes and seconds match.
        DayHoursMinutesSeconds = 0b10000, ///< Alarm when the day of the week, hours minutes and seconds match.
    };
    
    /// The mode of the INT/SQW pin on the chip.
    ///
    enum class IntPinMode : uint8_t {
        Disabled = 0b00000, ///< The pin is disabled.
        Alarm1 = 0b00101, ///< The pin is driven low if alarm 1 matches.
        Alarm2 = 0b00101, ///< The pin is driven low if alarm 2 matches.
        Alarm12 = 0b00101, ///< The pin is driven low if alarm 1 or 2 matches.
        SquareWave1Hz = 0b00000, ///< The pin outputs a 1Hz square wave signal.
        SquareWave1024Hz = 0b01000, ///< The pin outputs a 1.024kHz square wave signal.
        SquareWave4096Hz = 0b10000, ///< The pin outputs a 4.096kHz square wave signal.
        SquareWave8192Hz = 0b11000, ///< The pin outputs a 8.192kHz square wave signal.
    };
    
public:
    /// Initialize the real time clock driver.
    ///
    /// @param[in] bus The bus to use to communicate with the chip.
    /// @param[in] yearBase The year base which is used for the RTC.
    ///    The RTC stores the year only with two digits, plus one
    ///    additional bit for the next century. If you set the
    ///    year base to `2000`, the RTC will hold the correct time
    ///    for 200 years, starting from `2000-01-01 00:00:00`.
    ///
    DS3231(WireMaster *bus, uint16_t yearBase = 2000);

public:
    /// Get the current date/time.
    ///
    /// @param[out] dateTime The variable where the read date/time is stored.
    /// @return `Success` or `Error` if there was a communication problem with the chip.
    ///
    Status getDateTime(DateTime &dateTime);

    /// Set the date/time.
    ///
    /// @param[in] dateTime The new date/time to write to the chip.
    /// @return `Success` or `Error` if there was a communication problem with the chip.
    ///
    Status setDateTime(const DateTime &dateTime);

    /// Check if the RTC is running.
    ///
    /// If this method returns `false`, the RTC lost its power and you have to
    /// initialize a new date/time and configuration. It was most likely reset to
    /// the default values.
    ///
    /// @param[out] isRunning A variable where the running state of the chip is stored.
    /// @return `Success` or `Error` if there was a communication problem with the chip.
    ///
    Status isRunning(bool &isRunning);

    /// Enable the oscillator.
    ///
    /// In case the `isRunning` method reports `false`, the chip had most likely a
    /// complete power loss and needs to be initialized with a date and time.
    /// After settings the date/time using `setDateTime()` make sure to call this
    /// method, to start the oscillator.
    ///
    Status enableOscillator();
    
    /// Set the first alarm.
    ///
    /// @param[in] alarmMode The alarm mode to set.
    /// @param[in] dateTime The date/time for the alarm to set. Seconds, Minutes, Hours
    ///     and either the day of the month, or day of the week are used from the date.
    /// @return `Success` or `Error` if there was a communication problem with the chip.
    ///
    Status setAlarm1(const AlarmMode alarmMode, const DateTime &dateTime = DateTime());
    
    /// Set the second alarm.
    ///
    /// @param[in] alarmMode The alarm mode to set.
    /// @param[in] dateTime The date/time for the alarm to set. Minutes, Hours
    ///     and either the day of the month, or day of the week are used from the date.
    /// @return `Success` or `Error` if there was a communication problem with the chip.
    ///
    Status setAlarm2(const AlarmMode alarmMode, const DateTime &dateTime = DateTime());
    
    /// Check if alarm 1 is set and clear the alarm.
    ///
    /// @param[out] isSet If the alarm is set.
    /// @return `Success` or `Error` if there was a communication problem with the chip.
    ///
    Status isAlarm1Set(bool &isSet);
    
    /// Check if alarm 2 is set and clear the alarm.
    ///
    /// @param[out] isSet If the alarm is set.
    /// @return `Success` or `Error` if there was a communication problem with the chip.
    ///
    Status isAlarm2Set(bool &isSet);

    /// Set the mode for the Int/Sqw pin of the chip.
    ///
    Status setIntPinMode(const IntPinMode mode);
    
    /// Get the temperature in degrees celsius.
    ///
    /// @param temperature A variable where the read temperature is stored.
    /// @return `Success` or `Error` if there was a communication problem with the chip.
    ///
    Status getTemperature(float &temperature);

public:
    /// @name Low Level Functions
    /// Low level functions to directly access all registers of the chip or
    /// to print useful information for debugging.
    /// @{

    /// Get the status of all register values for debugging.
    ///
    /// @param str The string with all register values formatted for serial output.
    /// @return `Success` or `Error` if there was a communication problem with the chip.
    ///
    Status getAllRegisterValuesAsString(String &str);

    /// All registers available in the chip.
    ///
    enum class Register : uint8_t {
        Seconds = 0x00,
        Minutes = 0x01,
        Hours = 0x02,
        DayOfWeek = 0x03,
        Day = 0x04,
        MonthCentury = 0x05,
        Year = 0x06,
        Alarm1Seconds = 0x07,
        Alarm1Minutes = 0x08,
        Alarm1Hours = 0x09,
        Alarm1DayDate = 0x0a,
        Alarm2Minutes = 0x0b,
        Alarm2Hours = 0x0c,
        Alarm2DayDate = 0x0d,
        Control = 0x0e,
        Status = 0x0f,
        AgingOffset = 0x10,
        TemperatureHigh = 0x11,
        TemperatureLow = 0x12
    };

    /// The number of registers in the chip.
    ///
    constexpr inline static uint8_t getRegisterCount() {
        return 0x13;
    };

    /// The chip address.
    ///
    constexpr static const uint8_t cChipAddress = 0x68;
    
    /// Directly access the low-level bus functions
    ///
    inline const WireMasterRegisterChip<Register>& bus() const {
        return _bus;
    }

    /// All flags for the control register
    ///
    enum class ControlFlag : uint8_t {
        A1IE = oneBit8(0),
        A2IE = oneBit8(1),
        INTCN = oneBit8(2),
        RS1 = oneBit8(3),
        RS2 = oneBit8(4),
        CONV = oneBit8(5),
        BBSQW = oneBit8(6),
        EOSC = oneBit8(7)
    };
    LR_DECLARE_FLAGS(ControlFlag, ControlFlags);

    /// All flags for the status register
    ///
    enum class StatusFlag : uint8_t {
        A1F = oneBit8(0),
        A2F = oneBit8(1),
        BSY = oneBit8(2),
        EN32kHz = oneBit8(3),
        OSF = oneBit8(7)
    };
    LR_DECLARE_FLAGS(StatusFlag, StatusFlags);

    /// Convert a WireMaster status into a status of this driver.
    ///
    constexpr static inline Status statusFromBus(WireMaster::Status busStatus) {
        return (isSuccessful(busStatus)?(Status::Success):(Status::Error));
    }

    /// @}
    
private:
    struct DateTimeRegister;
    struct AlarmRegister;
    struct TemperatureRegister;
    
private:
    void fillAlarmRegister(const AlarmMode alarmMode, const lr::DateTime &dateTime, AlarmRegister &data);
    
private:
    const WireMasterRegisterChip<Register> _bus; ///< The bus for the communication.
    const uint16_t _yearBase; ///< The base for the year.
};


}


LR_DECLARE_OPERATORS_FOR_FLAGS(::lr::DS3231::ControlFlags);
LR_DECLARE_OPERATORS_FOR_FLAGS(::lr::DS3231::StatusFlags);

