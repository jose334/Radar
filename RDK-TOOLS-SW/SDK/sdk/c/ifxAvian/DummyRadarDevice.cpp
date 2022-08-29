/* ===========================================================================
** Copyright (C) 2021 - 2022 Infineon Technologies AG
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
** 1. Redistributions of source code must retain the above copyright notice,
**    this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. Neither the name of the copyright holder nor the names of its
**    contributors may be used to endorse or promote products derived from
**    this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
** ===========================================================================
*/

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "internal/DummyRadarDevice.hpp"
#include "ifxAvian/internal/DummyControlPort.hpp"

#include "ifxBase/Exception.hpp"

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

ifx_Dummy_Radar_Device_s::ifx_Dummy_Radar_Device_s(int sensor_type)
{
    m_driver = std::make_unique<Avian::Driver>(m_avian_dummy_port, static_cast<Infineon::Avian::Device_Type>(sensor_type));
    if (!m_driver)
    {
        throw rdk::exception::device_not_supported();
    }

    initialize();
}

BoardInstance* ifx_Dummy_Radar_Device_s::get_strata_avian_board() const
{
    return nullptr;
};

Avian::StrataPort* ifx_Dummy_Radar_Device_s::get_strata_avian_port() const
{
  return nullptr;
}

bool ifx_Dummy_Radar_Device_s::start_acquisition()
{
    return true;
}

bool ifx_Dummy_Radar_Device_s::stop_acquisition()
{
    return true;
}

Avian::Constant_Wave_Controller* ifx_Dummy_Radar_Device_s::get_constant_wave_controller()
{
  return nullptr;
}

void ifx_Dummy_Radar_Device_s::get_temperature(ifx_Float_t* temperature_celsius)
{
    throw rdk::exception::not_supported();
}

ifx_Cube_R_t* ifx_Dummy_Radar_Device_s::get_next_frame(ifx_Cube_R_t* frame, uint16_t timeout_ms)
{
    throw rdk::exception::not_supported();
    return nullptr;
}

ifx_Float_t ifx_Dummy_Radar_Device_s::get_tx_power(uint8_t tx_antenna)
{
    throw rdk::exception::not_supported();
}

ifx_Error_t ifx_Dummy_Radar_Device_s::send_to_device()
{
    return IFX_OK;
}