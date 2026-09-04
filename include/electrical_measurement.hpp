#pragma once

#include <Arduino.h>
#include <list>
#include <string>
#include <memory>

class ElectricalMeasurement {
public:

	class Unit {
		public:
			enum Value  : uint8_t {
				CURRENT = 0,
				VOLTAGE
			};

			static std::string ToString(const Value unit) {
				switch (unit) {
					case CURRENT:
						return "CURRENT";
					case VOLTAGE:
						return "VOLTAGE";
					default:
						return "UNKNOWN";
				}
			}

			static char ToSymbol(const Value unit) {
			switch (unit) {
				case CURRENT:
					return 'I';
				case VOLTAGE:
					return 'V';
				default:
					return '?';
			}
		}
	};


	class Channel {
		public:
		enum Value : uint8_t {
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

		static std::string ToString(const Value channel){
			switch (channel) {
				case MES_I_AUX:
					return "MES_I_AUX";
				case MES_V_AUX:
					return "MES_V_AUX";
				case MES_I_CON:
					return "MES_I_CON";
				case MES_V_CON:
					return "MES_V_CON";
				case MES_I_MOT:
					return "MES_I_MOT";
				case MES_V_MOT:
					return "MES_V_MOT";
				case MES_I_CHR:
					return "MES_I_CHR";
				case MES_V_CHR:
					return "MES_V_CHR";
				case MES_I_BAT:
					return "MES_I_BAT";
				case MES_V_BUS:
					return "MES_V_BUS";
				case MES_V_BAT:
					return "MES_V_BAT";
				case MES_V_GNDC:
					return "MES_V_GNDC";
				case MES_V_GNDD:
					return "MES_V_GNDD";
				case CHANNEL_COUNT:
					return "CHANNEL_COUNT";
				default:
					return "UNKNOWN";
			}
		}
	};

	struct ChannelData {
		Channel::Value channel;
		int raw;
		float physical;
		float adc_gain;
		int adc_offset;
		Unit::Value unit;
	};

	static constexpr uint8_t CHANNEL_COUNT = static_cast<uint8_t>(Channel::Value::CHANNEL_COUNT);

    /** 
     * @brief Initialize the electrical measurement system.
     * 
     * @param adc_resolution_bits The resolution of the ADC in bits. Default is 12.
     */
	void begin(uint8_t adc_resolution_bits = 12) {
		initChannelData();
		analogReadResolution(adc_resolution_bits);
		initialized_ = true;
	}

    /**
     * @brief Sample all electrical measurement channels.
     * 
     * If the system is not initialized, it will automatically initialize with the default ADC resolution.
     */
	std::shared_ptr<const std::list<ChannelData>> readAll() {
		sampleAll();
		return std::make_shared<const std::list<ChannelData>>(channel_data_list_);
	}

	void sampleAll() {
		if (!initialized_) {
			begin();
		}

		for (auto& channel_data : channel_data_list_) {
			const int raw_value = readRawChannel(channel_data.channel);
			channel_data.raw = raw_value;
			channel_data.physical = static_cast<float>(raw_value - channel_data.adc_offset) *
							  channel_data.adc_gain;
		}
	}



    /**
     * @brief Get the raw ADC value for a specific channel.
     * 
     * @param channel The channel to read.
     * @return The raw ADC value.
     */
	int rawValue(Channel::Value channel) const {
		const ChannelData* channel_data = findChannelData(channel);
		return channel_data != nullptr ? channel_data->raw : 0;
	}

    /**
     * @brief Get the physical value (converted from raw ADC) for a specific channel.
     * 
     * @param channel The channel to read.
     * @return The physical value.
     */
	float physicalValue(Channel::Value channel) const {
		const ChannelData* channel_data = findChannelData(channel);
		return channel_data != nullptr ? channel_data->physical : 0.0f;
	}

    /**
     * @brief Convert a raw ADC value to its corresponding physical value for a specific channel.
     * 
     * @param channel The channel to read.
     * @param raw_value The raw ADC value to convert.
     * @return The physical value.
     */
	float convertRawToPhysical(Channel::Value channel, int raw_value) const {
		const ChannelData* channel_data = findChannelData(channel);
		if (channel_data == nullptr) {
			return 0.0f;
		}

		return static_cast<float>(raw_value - channel_data->adc_offset) * channel_data->adc_gain;
	}



private:
	void initChannelData() {
		if (!channel_data_list_.empty()) {
			return;
		}

		channel_data_list_.push_back({Channel::MES_I_AUX, 0, 0.0f, 0.02260f, 292, Unit::CURRENT});
		channel_data_list_.push_back({Channel::MES_V_AUX, 0, 0.0f, 0.01605f, 0, Unit::VOLTAGE});
		channel_data_list_.push_back({Channel::MES_I_CON, 0, 0.0f, 0.02260f, 292, Unit::CURRENT});
		channel_data_list_.push_back({Channel::MES_V_CON, 0, 0.0f, 0.01605f, 0, Unit::VOLTAGE});
		channel_data_list_.push_back({Channel::MES_I_MOT, 0, 0.0f, 0.02260f, 1460, Unit::CURRENT});
		channel_data_list_.push_back({Channel::MES_V_MOT, 0, 0.0f, 0.01605f, 0, Unit::VOLTAGE});
		channel_data_list_.push_back({Channel::MES_I_CHR, 0, 0.0f, 0.02260f, 1460, Unit::CURRENT});
		channel_data_list_.push_back({Channel::MES_V_CHR, 0, 0.0f, 0.01605f, 0, Unit::VOLTAGE});
		channel_data_list_.push_back({Channel::MES_I_BAT, 0, 0.0f, 0.02260f, 1460, Unit::CURRENT});
		channel_data_list_.push_back({Channel::MES_V_BUS, 0, 0.0f, 0.01605f, 0, Unit::VOLTAGE});
		channel_data_list_.push_back({Channel::MES_V_BAT, 0, 0.0f, 0.01605f, 0, Unit::VOLTAGE});
		channel_data_list_.push_back({Channel::MES_V_GNDC, 0, 0.0f, 0.01605f, 0, Unit::VOLTAGE});
		channel_data_list_.push_back({Channel::MES_V_GNDD, 0, 0.0f, 0.01605f, 0, Unit::VOLTAGE});
	}

	ChannelData* findChannelData(Channel::Value channel) {
		for (auto& channel_data : channel_data_list_) {
			if (channel_data.channel == channel) {
				return &channel_data;
			}
		}

		return nullptr;
	}

	const ChannelData* findChannelData(Channel::Value channel) const {
		for (const auto& channel_data : channel_data_list_) {
			if (channel_data.channel == channel) {
				return &channel_data;
			}
		}

		return nullptr;
	}

	static const ChannelData& unknownChannelData() {
		static const ChannelData unknown_channel_data = {
			Channel::CHANNEL_COUNT,
			0,
			0.0f,
			0.0f,
			0,
			Unit::VOLTAGE
		};

		return unknown_channel_data;
	}



	static int readRawChannel(Channel::Value channel) {
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

	bool initialized_ = false;
	std::list<ChannelData> channel_data_list_;
	
};

