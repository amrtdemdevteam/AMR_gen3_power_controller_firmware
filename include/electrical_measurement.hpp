#pragma once

#include <Arduino.h>

class ElectricalMeasurement {
public:
	enum class Unit : uint8_t {
		CURRENT = 0,
		VOLTAGE
	};

	enum class Channel : uint8_t {
		MES_I_AUX = 0,
		MES_V_AUX,
		MES_I_CON,
		MES_V_CON,
		MES_I_MOT,
		MES_V_MOT,
		MES_I_CHR,
		MES_V_CHR,
		MES_I_BAT,
		MES_V_BUS,
		MES_V_BAT,
		MES_V_GNDC,
		MES_V_GNDD,
		CHANNEL_COUNT
	};

	struct ChannelConfig {
		const char* name;
		float adc_gain;
		int adc_offset;
		Unit unit;
	};

	static constexpr uint8_t CHANNEL_COUNT = static_cast<uint8_t>(Channel::CHANNEL_COUNT);

    /** 
     * @brief Initialize the electrical measurement system.
     * 
     * @param adc_resolution_bits The resolution of the ADC in bits. Default is 12.
     */
	void begin(uint8_t adc_resolution_bits = 12) {
		analogReadResolution(adc_resolution_bits);
		initialized_ = true;
	}

    /**
     * @brief Sample all electrical measurement channels.
     * 
     * If the system is not initialized, it will automatically initialize with the default ADC resolution.
     */
	void sampleAll() {
		if (!initialized_) {
			begin();
		}

		const ChannelConfig* channel_configs = channelConfigs();

		for (uint8_t i = 0; i < CHANNEL_COUNT; ++i) {
			const int raw_value = readRawChannel(static_cast<Channel>(i));
			raw_values_[i] = raw_value;
			physical_values_[i] = (static_cast<float>(raw_value - channel_configs[i].adc_offset) *
								   channel_configs[i].adc_gain);
		}
	}

    /**
     * @brief Get the raw ADC value for a specific channel.
     * 
     * @param channel The channel to read.
     * @return The raw ADC value.
     */
	int rawValue(Channel channel) const {
		return raw_values_[toIndex(channel)];
	}

    /**
     * @brief Get the physical value (converted from raw ADC) for a specific channel.
     * 
     * @param channel The channel to read.
     * @return The physical value.
     */
	float physicalValue(Channel channel) const {
		return physical_values_[toIndex(channel)];
	}

    /**
     * @brief Convert a raw ADC value to its corresponding physical value for a specific channel.
     * 
     * @param channel The channel to read.
     * @param raw_value The raw ADC value to convert.
     * @return The physical value.
     */
	float convertRawToPhysical(Channel channel, int raw_value) const {
		const ChannelConfig* channel_configs = channelConfigs();
		const uint8_t index = toIndex(channel);
		return static_cast<float>(raw_value - channel_configs[index].adc_offset) *
			   channel_configs[index].adc_gain;
	}

    /**
     * @brief Get the name of a specific channel.
     * 
     * @param channel The channel to read.
     * @return The name of the channel.
     */
	const char* channelName(Channel channel) const {
		return channelConfigs()[toIndex(channel)].name;
	}

    /**
     * @brief Get the unit of a specific channel.
     * 
     * @param channel The channel to read.
     * @return The unit of the channel.
     */
	Unit channelUnit(Channel channel) const {
		return channelConfigs()[toIndex(channel)].unit;
	}

    /**
     * @brief Get the symbol of the unit for a specific channel.
     * 
     * @param channel The channel to read.
     * @return The symbol of the unit.
     */
	char unitSymbol(Channel channel) const {
		return unitToSymbol(channelUnit(channel));
	}
    /**
     * @brief Get the full configuration of a specific channel.
     * 
     * @param channel The channel to read.
     * @return The configuration of the channel.
     */
	const ChannelConfig& config(Channel channel) const {
		return channelConfigs()[toIndex(channel)];
	}

private:
	static uint8_t toIndex(Channel channel) {
		return static_cast<uint8_t>(channel);
	}

	static char unitToSymbol(Unit unit) {
		switch (unit) {
			case Unit::CURRENT:
				return 'I';
			case Unit::VOLTAGE:
				return 'V';
			default:
				return '?';
		}
	}

	static int readRawChannel(Channel channel) {
		switch (channel) {
			case Channel::MES_I_AUX:
				return analogRead(A0);
			case Channel::MES_V_AUX:
				return analogRead(A1);
			case Channel::MES_I_CON:
				return analogRead(A2);
			case Channel::MES_V_CON:
				return analogRead(A3);
			case Channel::MES_I_MOT:
				return analogRead(A4);
			case Channel::MES_V_MOT:
				return analogRead(A5);
			case Channel::MES_I_CHR:
				return analogRead(A6);
			case Channel::MES_V_CHR:
				return analogRead(A7);
			case Channel::MES_I_BAT:
				return analogRead(A8);
			case Channel::MES_V_BUS:
				return analogRead(A9);
			case Channel::MES_V_BAT:
				return analogRead(A10);
			case Channel::MES_V_GNDC:
				return analogRead(A11);
			case Channel::MES_V_GNDD:
				return analogRead(A12);
			case Channel::CHANNEL_COUNT:
			default:
				return 0;
		}
	}

	static const ChannelConfig* channelConfigs() {
		static const ChannelConfig channel_configs[CHANNEL_COUNT] = {
			{"MES_I_AUX", 0.02260f, 292, Unit::CURRENT},
			{"MES_V_AUX", 0.01605f, 0, Unit::VOLTAGE},
			{"MES_I_CON", 0.02260f, 292, Unit::CURRENT},
			{"MES_V_CON", 0.01605f, 0, Unit::VOLTAGE},
			{"MES_I_MOT", 0.02260f, 1460, Unit::CURRENT},
			{"MES_V_MOT", 0.01605f, 0, Unit::VOLTAGE},
			{"MES_I_CHR", 0.02260f, 1460, Unit::CURRENT},
			{"MES_V_CHR", 0.01605f, 0, Unit::VOLTAGE},
			{"MES_I_BAT", 0.02260f, 1460, Unit::CURRENT},
			{"MES_V_BUS", 0.01605f, 0, Unit::VOLTAGE},
			{"MES_V_BAT", 0.01605f, 0, Unit::VOLTAGE},
			{"MES_V_GNDC", 0.01605f, 0, Unit::VOLTAGE},
			{"MES_V_GNDD", 0.01605f, 0, Unit::VOLTAGE},
		};

		return channel_configs;
	}

	bool initialized_ = false;
	int raw_values_[CHANNEL_COUNT] = {0};
	float physical_values_[CHANNEL_COUNT] = {0.0f};
};

