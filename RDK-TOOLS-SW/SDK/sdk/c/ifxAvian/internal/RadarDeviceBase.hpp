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

/**
 * @internal
 * @file RadarDeviceBase.hpp
 *
 * @brief Defines the structure for the Radar Device Controller Module.
*/

#ifndef IFX_RADAR_INTERNAL_RADAR_DEVICE_BASE_HPP
#define IFX_RADAR_INTERNAL_RADAR_DEVICE_BASE_HPP

#include <memory>
#include <atomic>
#include <iterator>

#include "ifxBase/internal/NonCopyable.hpp"
#include "ifxAvian/internal/DeviceControlAvian.hpp"

#include "ifxBase/Error.h"
#include "ifxBase/Cube.h"
#include "ifxAvian/DeviceConfig.h"
#include "ifxAvian/Metrics.h"

#include "ifxRadarDeviceCommon/internal/AcquisitionState.hpp"

#include "platform/BoardInstance.hpp"
#include <ifxAvian_CwController.hpp>

// libAvian
#include "ifxAvian_Driver.hpp"
#include "ports/ifxAvian_StrataPort.hpp"

using namespace Infineon;

/*
==============================================================================
   2. DEFINITIONS
==============================================================================
*/

/*
==============================================================================
   3. TYPES
==============================================================================
*/


/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/
struct RadarDeviceBase: public DeviceControlAvian
{
    NONCOPYABLE(RadarDeviceBase);

    static constexpr auto UUID_INVALID_STR = "00000000-0000-0000-0000-000000000000"; /**< uuid of shield; if all bytes are 0 then the uuid is invalid */

    virtual ~RadarDeviceBase() { ifx_avian_calc_destroy(m_calc);  };

    ifx_Float_t get_device_calc_chirp_time(const ifx_Avian_Config_t* config) const;
    ifx_Avian_Calc_t* get_device_calc() const;

    void set_config(const ifx_Avian_Config_t* config);
    void get_config(ifx_Avian_Config_t* config);

    void get_default_metrics(ifx_Avian_Metrics_t* metrics);

    void configure_shapes(ifx_Avian_Shape_Set_t* shape_set);

    void configure_adc(const ifx_Avian_ADC_Config_t* config);

    uint32_t export_register_list(bool set_trigger_bit, uint32_t* register_list);

    uint16_t get_slice_size() const;

    const ifx_Radar_Sensor_Info_t* get_sensor_info() const;

    ifx_Radar_Sensor_t get_sensor_type() const;

    const char* get_board_uuid() const;

    const ifx_Firmware_Info_t* get_firmware_info() const;

public:
    /* These abstract virtual members must be implemented by a derived class */
    virtual void get_temperature(ifx_Float_t* temperature_celsius) = 0;

    virtual ifx_Cube_R_t* get_next_frame(ifx_Cube_R_t* frame, uint16_t timeout_ms) = 0;

    virtual ifx_Float_t get_tx_power(uint8_t tx_antenna) = 0;

    virtual BoardInstance* get_strata_avian_board() const = 0;

    virtual Avian::StrataPort* get_strata_avian_port() const = 0;

    virtual bool start_acquisition() = 0;
    virtual bool stop_acquisition() = 0;

    virtual Avian::Constant_Wave_Controller* get_constant_wave_controller() = 0;

protected:
    RadarDeviceBase() = default;

    void initialize();
    bool stop_and_reset();

private:
    // private methods
    virtual ifx_Error_t send_to_device() = 0;

    ifx_Avian_Shape_Set_t get_shape_set_from_config() const;

protected:
    ifx_Error_t initialize_baseband_config(Avian::Baseband_Configuration* baseband_config) const;

    // attributes
    std::string m_board_uuid = UUID_INVALID_STR; /**< UUID of board */
    ifx_Radar_Sensor_Info_t m_sensor_info = {};  /**< Sensor information */
    ifx_Firmware_Info_t m_firmware_info = {};    /**< Firmware information */
    Atomic_Acquisition_State_t m_acquisition_state{ Acquisition_State_t::Stopped };
};

#endif /* IFX_RADAR_INTERNAL_RADAR_DEVICE_BASE_HPP */
