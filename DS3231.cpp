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
#include "DS3231.hpp"


#include "../hal/BCD.hpp"


namespace lr {


/// @internal
/// A struct to store all time related registers in one block.
///
struct DS3231::DateTimeRegister {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t dayOfWeek;
    uint8_t day;
    uint8_t month;
    uint8_t year;
};

/// @internal
/// A struct to store all alarm related registers in one block.
///
struct DS3231::AlarmRegister {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day; // or day of week.
};

/// @internal
/// A struct with the temperature registers.
///
struct DS3231::TemperatureRegister {
	int8_t high;
	uint8_t low;
};


DS3231::DS3231(WireMaster *bus, uint16_t yearBase)
    : _bus(bus, cChipAddress), _yearBase(yearBase)
{
}


DS3231::Status DS3231::getDateTime(DateTime &dateTime)
{
    // Use the struct to read all registers in one batch.
    DateTimeRegister data;
    const auto status = _bus.readRegisterData(Register::Seconds, reinterpret_cast<uint8_t*>(&data), sizeof(DateTimeRegister));
    if (isSuccessful(status)) {
        // Convert these values into a date object.
        dateTime = DateTime::fromUncheckedValues(
            static_cast<uint16_t>(BCD::convertBcdToBin(data.year))+((data.month&(1<<7))!=0?(_yearBase+100):_yearBase),
            BCD::convertBcdToBin(data.month&0x1f),
            BCD::convertBcdToBin(data.day&0x3f),
            BCD::convertBcdToBin(data.hours&0x3f),
            BCD::convertBcdToBin(data.minutes&0x7f),
            BCD::convertBcdToBin(data.seconds&0x7f),
            data.dayOfWeek&0x7);
        return Status::Success;
    } else {
        return Status::Error;
    }
}

    
DS3231::Status DS3231::setDateTime(const DateTime &dateTime)
{
    // Basic year check
    const uint16_t newYear = dateTime.getYear();
    if (newYear < _yearBase || newYear >= (_yearBase+200)) {
        return Status::Error; // The date is outside of the valid range.
    }
    // Use a struct to write all registers in one batch.
    DateTimeRegister data;
	// Prepare all values.
    data.seconds = BCD::convertBinToBcd(dateTime.getSecond());
    data.minutes = BCD::convertBinToBcd(dateTime.getMinute());
    data.hours = BCD::convertBinToBcd(dateTime.getHour());
    data.dayOfWeek = dateTime.getDayOfWeek();
    data.day = BCD::convertBinToBcd(dateTime.getDay());
    data.month = BCD::convertBinToBcd(dateTime.getMonth()) |
        (dateTime.getYear()>=(_yearBase+100)?(1<<7):0);
    data.year = BCD::convertBinToBcd(dateTime.getYear()%100);
    // Write all registers.
    return statusFromBus(
        _bus.writeRegisterData(Register::Seconds, reinterpret_cast<uint8_t*>(&data), sizeof(DateTimeRegister)));
}

    
DS3231::Status DS3231::isRunning(bool &isRunning)
{
    WireMaster::BitResult bitResult;
    auto status = _bus.testBits(Register::Status, static_cast<uint8_t>(StatusFlag::OSF), bitResult);
    if (hasError(status)) {
        return statusFromBus(status);
    }
    if (bitResult == WireMaster::BitResult::Set) {
        isRunning = false;
        return statusFromBus(status);
    }
    status = _bus.testBits(Register::Control, static_cast<uint8_t>(ControlFlag::EOSC), bitResult);
    if (hasError(status)) {
        return statusFromBus(status);
    }
    isRunning = (bitResult == WireMaster::BitResult::Zero);
    return statusFromBus(status);
}

    
DS3231::Status DS3231::enableOscillator()
{
    // Start the oscillator in the control register.
    auto status = _bus.changeBits(Register::Control, static_cast<uint8_t>(ControlFlag::EOSC), WireMaster::BitOperation::Clear);
    if (hasError(status)) {
        return statusFromBus(status);
    }
    // Reset the flag.
    return statusFromBus(_bus.changeBits(Register::Status, static_cast<uint8_t>(StatusFlag::OSF), WireMaster::BitOperation::Clear));
}

    
inline void DS3231::fillAlarmRegister(const AlarmMode alarmMode, const DateTime &dateTime, AlarmRegister &data)
{
    data.seconds = BCD::convertBinToBcd(dateTime.getSecond())|((static_cast<uint8_t>(alarmMode)&0b00001)!=0?0x80:0x00);
    data.minutes = BCD::convertBinToBcd(dateTime.getMinute())|((static_cast<uint8_t>(alarmMode)&0b00010)!=0?0x80:0x00);
    data.hours = BCD::convertBinToBcd(dateTime.getHour())|((static_cast<uint8_t>(alarmMode)&0b00100)!=0?0x80:0x00);
    if (alarmMode == AlarmMode::DayHoursMinutesSeconds) {
        data.day = BCD::convertBinToBcd(dateTime.getDayOfWeek()+1)|0b01000000;
    } else {
        data.day = BCD::convertBinToBcd(dateTime.getDay());
    }
    data.day |= ((static_cast<uint8_t>(alarmMode)&0b01000)!=0?0x80:0x00);
}
    

DS3231::Status DS3231::setAlarm1(const AlarmMode alarmMode, const lr::DateTime &dateTime)
{
    AlarmRegister data;
    fillAlarmRegister(alarmMode, dateTime, data);
    // Write all registers.
    return statusFromBus(
         _bus.writeRegisterData(Register::Alarm1Seconds, reinterpret_cast<uint8_t*>(&data), sizeof(AlarmRegister)));
}

    
DS3231::Status DS3231::setAlarm2(const AlarmMode alarmMode, const lr::DateTime &dateTime)
{
    AlarmRegister data;
    fillAlarmRegister(alarmMode, dateTime, data);
    // Write all registers.
    return statusFromBus(
         _bus.writeRegisterData(Register::Alarm2Minutes, reinterpret_cast<uint8_t*>(&data)+1, sizeof(AlarmRegister)-1));
}

    
DS3231::Status DS3231::isAlarm1Set(bool &isSet)
{
    WireMaster::BitResult bitResult;
    auto status = _bus.testBits(Register::Status, static_cast<uint8_t>(StatusFlag::A1F), bitResult);
    if (hasError(status)) {
        return statusFromBus(status);
    }
    isSet = (bitResult == WireMaster::BitResult::Set);
    // If the bit is set, clear the bit.
    if (isSet) {
        _bus.changeBits(Register::Status, static_cast<uint8_t>(StatusFlag::A1F), WireMaster::BitOperation::Clear);
    }
    return statusFromBus(status);
}

    
DS3231::Status DS3231::isAlarm2Set(bool &isSet)
{
    WireMaster::BitResult bitResult;
    auto status = _bus.testBits(Register::Status, static_cast<uint8_t>(StatusFlag::A2F), bitResult);
    if (hasError(status)) {
        return statusFromBus(status);
    }
    isSet = (bitResult == WireMaster::BitResult::Set);
    // If the bit is set, clear the bit.
    if (isSet) {
        _bus.changeBits(Register::Status, static_cast<uint8_t>(StatusFlag::A2F), WireMaster::BitOperation::Clear);
    }
    return statusFromBus(status);
}

    
DS3231::Status DS3231::setIntPinMode(const IntPinMode mode)
{
    return statusFromBus(_bus.writeBits(Register::Control, static_cast<uint8_t>(0b00011111), static_cast<uint8_t>(mode)));
}


DS3231::Status DS3231::getTemperature(float &temperature)
{
    // Read all temperature registers.
	TemperatureRegister data;
	const auto status = _bus.readRegisterData(Register::TemperatureHigh, reinterpret_cast<uint8_t*>(&data), sizeof(TemperatureRegister));
    if (status == WireMaster::Status::Success) {
        // Create a float from this values.
        float result = static_cast<float>(data.high);
        const float fraction = static_cast<float>(data.low >> 6) * 0.25f;
        if (result < 0) {
            result -= fraction;
        } else {
            result += fraction;
        }
        temperature = result;
        return Status::Success;
    } else {
        return Status::Error;
    }
}


DS3231::Status DS3231::getAllRegisterValuesAsString(String &str)
{
    String result;
    const auto registerCount = getRegisterCount();
    uint8_t rtcRegister[registerCount];
    const auto status = _bus.readRegisterData(Register::Seconds, rtcRegister, registerCount);
    if (status == WireMaster::Status::Success) {
        for (uint8_t i = 0; i < registerCount; ++i) {
            result.appendHex(i);
            result.append(':');
            const uint8_t value = rtcRegister[i];
            result.appendHex(value);
            result.append(':');
            result.appendBin(value);
            result.append('\n');
        }
        str = result;
        return Status::Success;
    } else {
        return Status::Error;
    }
}


}


