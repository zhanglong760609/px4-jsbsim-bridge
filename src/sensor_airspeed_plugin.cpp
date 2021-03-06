/****************************************************************************
 *
 *   Copyright (c) 2020 Auterion AG. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/
/**
 * @brief JSBSim Airspeed Plugin
 *
 * This is a plugin modeling a airspeed sensor for JSBSim
 *
 * @author Jaeyoung Lim <jaeyoung@auterion.com>
 * @author Roman Bapst <roman@auterion.com>
 */

#include "sensor_airspeed_plugin.h"

SensorAirspeedPlugin::SensorAirspeedPlugin(JSBSim::FGFDMExec* jsbsim) : SensorPlugin(jsbsim) {}

SensorAirspeedPlugin::~SensorAirspeedPlugin() {}

void SensorAirspeedPlugin::setSensorConfigs(const TiXmlElement& configs) {
  GetConfigElement<double>(configs, "diff_pressure_stddev", _diff_pressure_stddev);
}

SensorData::Airspeed SensorAirspeedPlugin::getData() {
  double sim_time = _sim_ptr->GetSimTime();
  double dt = sim_time - _last_sim_time;

  const double temperature_msl = 288.0f;  // temperature at MSL (Kelvin)
  double temperature_local = getAirTemperature() + 273.0f;
  const double density_ratio = powf((temperature_msl / temperature_local), 4.256f);
  float rho = 1.225f / density_ratio;

  const double diff_pressure_noise = standard_normal_distribution_(_random_generator) * _diff_pressure_stddev;

  double vel_a = getAirspeed();

  double diff_pressure = 0.005f * rho * vel_a * vel_a + diff_pressure_noise;

  SensorData::Airspeed data;

  data.diff_pressure = diff_pressure;

  _last_sim_time = sim_time;
  return data;
}

double SensorAirspeedPlugin::getAirspeed() { return ftToM(_sim_ptr->GetPropertyValue("velocities/vc-fps")); }

double SensorAirspeedPlugin::getAirTemperature() {
  return rankineToCelsius(_sim_ptr->GetPropertyValue("atmosphere/T-R"));
}
