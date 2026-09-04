#include <Arduino.h>
#include <string>
#include "RPC.h"
#include "electrical_measurement.hpp"

using EEChannel = ElectricalMeasurement::Channel;

int result; 
void setup() {

    RPC.begin();

}

void loop() {

    /*result = RPC.call("add", result, 1).as<int>();
    RPC.print("M4 Add Result: ");
    RPC.println(result);*/
    
    std::string electrical_param = RPC.call("getElectricalMeasurement", EEChannel::ToUint8(EEChannel::MES_I_AUX)).as<std::string>();
    RPC.print("M4 Electrical Measurement channel MES_I_AUX: ");
    RPC.println(electrical_param.c_str());

    electrical_param = RPC.call("getElectricalMeasurement", EEChannel::ToUint8(EEChannel::MES_V_AUX)).as<std::string>();
    RPC.print("M4 Electrical Measurement channel MES_V_AUX: ");
    RPC.println(electrical_param.c_str());

    electrical_param = RPC.call("getElectricalMeasurement", EEChannel::ToUint8(EEChannel::MES_V_BAT)).as<std::string>();
    RPC.print("M4 Electrical Measurement channel MES_V_BAT: ");
    RPC.println(electrical_param.c_str());

    delay(1000);

}