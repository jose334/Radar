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

#include <array>
#include <atomic>
#include <chrono>
#include <cctype>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "ifxBase/Defines.h"
#include "ifxBase/Error.h"
#include "ifxBase/Exception.hpp"
#include "ifxBase/internal/Util.h"
#include "ifxBase/List.h"
#include "ifxBase/internal/List.hpp"
#include "ifxBase/Log.h"
#include "ifxBase/Mem.h"
#include "ifxBase/Matrix.h"
#include "ifxBase/Uuid.h"

#include "ifxAvian/internal/DeviceCalc.h"
#include "ifxAvian/internal/RadarDevice.hpp"
#include "ifxAvian/internal/DummyRadarDevice.hpp"
#include "ifxAvian/DeviceControl.h"
#include "ifxAvian/DeviceConfig.h"
#include "ifxAvian/Shapes.h"

#include "ifxRadarDeviceCommon/internal/RadarDeviceCommon.hpp"

// libAvian
#include <ifxAvian_Driver.hpp>

// Strata
#include <common/Logger.hpp>
#include <platform/BoardManager.hpp>
#include <platform/serial/BoardSerial.hpp>
#include <platform/exception/EConnection.hpp>
#include <platform/exception/EAlreadyOpened.hpp>
#include <platform/exception/EProtocol.hpp>

// DeviceControlHelper functions
#include "DeviceControlHelper.hpp"

using namespace Infineon;

/*
==============================================================================
   2. LOCAL DEFINITIONS
==============================================================================
*/

// Log level for Strata.
#ifdef NDEBUG
    // Release build: set log level to ERROR only
    #define STRATA_LOG_LEVEL (LOG_ERROR)
#else
    // Debug build: Set log level to error, warn, debug
    #define STRATA_LOG_LEVEL (LOG_ERROR | LOG_WARN | LOG_DEBUG)
#endif

/*
==============================================================================
   3. LOCAL TYPES
==============================================================================
*/

/**
 * @brief Defines the structure for device register list access object.
 *        When created, retrieves the register list and stores it in integer and string
 *        format.
 */
typedef struct {
    uint32_t num_registers;
    uint32_t* register_list;
} ifx_Avian_Register_List_t;

/*
==============================================================================
   5. LOCAL FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @brief Exports device register list to device control.
 *
 * @param [in]  handle              A handle to the radar device object.
 * @param [in]  set_trigger_bit     boolean for wrapped libAvian function.
 * @return      pointer to initialized ifx_Avian_Register_List_t handle. The caller is responsible to run \ref ifx_avian_register_list_destroy within the current scope.
 *
 */
static ifx_Avian_Register_List_t* register_list_create(ifx_Avian_Device_t* handle, bool set_trigger_bit);

/**
 * @brief destroys ifx_Avian_Register_List_t object.
 *
 * @param [in]  register_list_handle       handle to ifx_Avian_Register_List_t object
 *
 */
static void register_list_destroy(ifx_Avian_Register_List_t* register_list);


/**
 * @brief Translate register list in uint32_t array format to encapsulated string
 */
static char* register_list_to_string(uint32_t* register_list, uint32_t num_reg);

/*
==============================================================================
   6. LOCAL FUNCTIONS
==============================================================================
*/

static ifx_Avian_Register_List_t* register_list_create(ifx_Avian_Device_t* handle, bool set_trigger_bit)
{
    IFX_ERR_BRV_NULL(handle, nullptr);

    auto* device_register_list_handle = static_cast<ifx_Avian_Register_List_t*>(ifx_mem_alloc(sizeof(ifx_Avian_Register_List_t)));
    IFX_ERR_BRV_MEMALLOC(device_register_list_handle, nullptr);

    uint32_t num_reg = 0;

    try
    {
        num_reg = handle->export_register_list(set_trigger_bit, nullptr);
    }
    catch (const EConnection&)
    {
        ifx_error_set(IFX_ERROR_COMMUNICATION_ERROR);
    }

    // allocate memory for register_list based on number of registers
    device_register_list_handle->register_list = static_cast<uint32_t*>(ifx_mem_calloc(num_reg, sizeof(uint32_t)));
    IFX_ERR_BRF_MEMALLOC(device_register_list_handle->register_list);

    //populate device_register_list_handle->register_list
    try
    {
        device_register_list_handle->num_registers = handle->export_register_list(set_trigger_bit, device_register_list_handle->register_list);
    }
    catch (const EConnection&)
    {
        ifx_error_set(IFX_ERROR_COMMUNICATION_ERROR);
    }
    return device_register_list_handle;

fail:
    ifx_mem_free(device_register_list_handle);
    return nullptr;
}

//----------------------------------------------------------------------------

static void register_list_destroy(ifx_Avian_Register_List_t* device_register_list_handle)
{
    if (!device_register_list_handle)
    {
        return;
    }
    ifx_mem_free(device_register_list_handle->register_list);
    ifx_mem_free(device_register_list_handle);
}

//----------------------------------------------------------------------------

static char* register_list_to_string(uint32_t* register_list, uint32_t num_reg)
{
    IFX_ERR_BRV_NULL(register_list, nullptr);

    std::stringstream string_cpp;

    string_cpp << "#define XENSIV_BGT60TRXX_CONF_NUM_REGS (" << num_reg << ")" << std::endl;

    string_cpp << std::endl;

    string_cpp << "#if defined(XENSIV_BGT60TRXX_CONF_IMPL)" << std::endl;

    string_cpp << "const uint32_t register_list[] = { " << std::endl;
    for (uint32_t i = 0; i < (num_reg - 1); i++)
    {
        string_cpp << "    0x" << std::hex << register_list[i] << "UL, " << std::endl;
    }
    string_cpp << "    0x" << std::hex << register_list[num_reg - 1] << "UL" << std::endl << "};" << std::endl;

    string_cpp << "#endif" << std::endl;

    // size()+1 to include the string delimiter
    auto* outstring_c = static_cast<char*>(ifx_mem_calloc(string_cpp.str().size() + 1, sizeof(char)));
    IFX_ERR_BRV_MEMALLOC(outstring_c, nullptr);

    std::memcpy(outstring_c, string_cpp.str().data(), string_cpp.str().size()+1);

    return(outstring_c);
}

//----------------------------------------------------------------------------

static ifx_Avian_Device_t* open_dummy_device(ifx_Radar_Sensor_t sensor_type)
{
  try
  {
      return new ifx_Dummy_Radar_Device_s(sensor_type);
  }
  catch (const rdk::exception::exception& e)
  {
      ifx_Error_t error_code = e.error_code();
      IFX_LOG_WARNING("Cannot open radar device: %s", ifx_error_to_string(error_code));
      ifx_error_set(error_code);
  }
  catch (const std::bad_alloc&)
  {
      IFX_LOG_WARNING("Cannot open radar device: Memory allocation failed");
      ifx_error_set(IFX_ERROR_MEMORY_ALLOCATION_FAILED);
  }

  return nullptr;
}

ifx_Avian_Device_t* open_device(std::unique_ptr<BoardInstance> board)
{
    try
    {
        return new ifx_Radar_Device_s(std::move(board));
    }
    catch (const EConnection&)
    {
        ifx_error_set(IFX_ERROR_NO_DEVICE);
    }
    catch (const EException&)
    {
        ifx_error_set(IFX_ERROR_NO_DEVICE);
    }
    catch (const rdk::exception::exception& e)
    {
        ifx_error_set(e.error_code());
    }
    catch (const std::bad_alloc& err)
    {
        (void)err;
        ifx_error_set(IFX_ERROR_MEMORY_ALLOCATION_FAILED);
    }
    catch (...)
    {
        ifx_error_set(IFX_ERROR);
    }

    return nullptr;
}

/*
==============================================================================
   7. EXPORTED FUNCTIONS
==============================================================================
*/

ifx_List_t* ifx_avian_get_list_by_sensor_type(ifx_Radar_Sensor_t sensor_type)
{
    auto selector = [&sensor_type](const ifx_Radar_Sensor_List_Entry_t& entry) {
        return entry.sensor_type == sensor_type;
    };

    auto list = rdk::RadarDeviceCommon::get_list(selector);

    return ifx_list_from_vector(list);
}

//----------------------------------------------------------------------------

ifx_List_t* ifx_avian_get_list()
{
    auto selector = [](const ifx_Radar_Sensor_List_Entry_t& entry) {
        return rdk::RadarDeviceCommon::sensor_is_avian(entry.sensor_type);
    };

    auto list = rdk::RadarDeviceCommon::get_list(selector);
    return ifx_list_from_vector(list);
}

//----------------------------------------------------------------------------

ifx_Avian_Device_t* ifx_avian_create_by_port(const char* port)
{
    // if port is NULL call ifx_avian_create. This ensures the previous behavior.
    if (port == nullptr)
        return ifx_avian_create();

    try
    {
        auto board = BoardSerial::createBoardInstance(port);
        return open_device(std::move(board));
    }
    catch (const EConnection&)
    {
        ifx_error_set(IFX_ERROR_NO_DEVICE);
    }
    catch (const EException&)
    {
        ifx_error_set(IFX_ERROR_NO_DEVICE);
    }

    return nullptr;
}

//----------------------------------------------------------------------------

ifx_Avian_Device_t* ifx_avian_create_dummy(ifx_Radar_Sensor_t sensor_type)
{
    return open_dummy_device(sensor_type);
}

ifx_Avian_Device_t* ifx_avian_create(void)
{
    auto selector = [](const ifx_Radar_Sensor_List_Entry_t& entry) {
        return rdk::RadarDeviceCommon::sensor_is_avian(entry.sensor_type);
    };

    auto board = rdk::RadarDeviceCommon::open(selector);
    return open_device(std::move(board));
}

//----------------------------------------------------------------------------

ifx_Avian_Device_t* ifx_avian_create_by_uuid(const char* uuid)
{
    IFX_ERR_BRV_NULL(uuid, nullptr);

    uint8_t uuid_array[16];
    if (!ifx_uuid_from_string(uuid, uuid_array))
    {
        ifx_error_set(IFX_ERROR_ARGUMENT_INVALID);
        return nullptr;
    }

    auto board = rdk::RadarDeviceCommon::open_by_uuid(uuid_array);
    return open_device(std::move(board));
}

//----------------------------------------------------------------------------

char* ifx_avian_configuration_string(ifx_Avian_Device_t* handle)
{
    IFX_ERR_BRV_NULL(handle, nullptr);

    ifx_Avian_Config_t config;
    try
    {
        handle->get_config(&config);
    }
    catch (const rdk::exception::exception& e)
    {
        ifx_Error_t error_code = e.error_code();
        ifx_error_set(error_code);
    }
    catch (const EConnection&)
    {
        ifx_error_set(IFX_ERROR_COMMUNICATION_ERROR);
    }
    catch (const EException&)
    {
        ifx_error_set(IFX_ERROR);
    }  

    std::stringstream string_cpp;

    const uint32_t num_samples_per_fifo_word = 2;
    const uint32_t num_rx_antennas = ifx_util_popcount(config.rx_mask);
    const uint32_t num_tx_antennas = ifx_util_popcount(config.tx_mask);
    const uint32_t frame_size = config.num_samples_per_chirp * config.num_chirps_per_frame * num_rx_antennas;

    string_cpp << "#define XENSIV_BGT60TRXX_CONF_START_FREQ_HZ (" << config.start_frequency_Hz << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_END_FREQ_HZ (" << config.end_frequency_Hz << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_NUM_SAMPLES_PER_CHIRP (" << config.num_samples_per_chirp << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_NUM_CHIRPS_PER_FRAME (" << config.num_chirps_per_frame << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_NUM_RX_ANTENNAS (" << num_rx_antennas << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_NUM_TX_ANTENNAS (" << num_tx_antennas << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_SAMPLE_RATE (" << config.sample_rate_Hz << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_CHIRP_REPETION_TIME_S (" << config.chirp_repetition_time_s << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_FRAME_REPETION_TIME_S (" << config.frame_repetition_time_s << ")" << std::endl;
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_FRAME_XFER_SIZE_WORDS (" << (frame_size / num_samples_per_fifo_word) << ")" << std::endl;    
    string_cpp << "#define XENSIV_BGT60TRXX_CONF_BUFFER_SIZE_BYTES ((XENSIV_BGT60TRXX_CONF_FRAME_XFER_SIZE_WORDS * XENSIV_BGT60TRXX_FIFO_WORD_SIZE_BYTES) + XENSIV_BGT60TRXX_SPI_BURST_HEADER_SIZE_BYTES)" << std::endl;    

    // size()+1 to include the string delimiter
    auto* outstring_c = static_cast<char*>(ifx_mem_calloc(string_cpp.str().size() + 1, sizeof(char)));
    IFX_ERR_BRV_MEMALLOC(outstring_c, nullptr);

    std::memcpy(outstring_c, string_cpp.str().data(), string_cpp.str().size()+1);

    return(outstring_c);
}

//----------------------------------------------------------------------------

char* ifx_avian_get_register_list_string(ifx_Avian_Device_t* handle, bool set_trigger_bit)
{
    IFX_ERR_BRV_NULL(handle, nullptr);
    ifx_Avian_Register_List_t* device_register_list = register_list_create(handle, set_trigger_bit);
    IFX_ERR_BRV_NULL(device_register_list, nullptr);

    char* register_list_string = register_list_to_string(device_register_list->register_list, device_register_list->num_registers); // allocates memory
    IFX_ERR_BRV_NULL(register_list_string, nullptr);
    register_list_destroy(device_register_list);

    return register_list_string;
}

//----------------------------------------------------------------------------

void ifx_avian_config_get_limits(const ifx_Avian_Device_t* handle, ifx_Avian_Config_t* config_lower, ifx_Avian_Config_t* config_upper)
{
    IFX_ERR_BRK_NULL(handle);
    IFX_ERR_BRK_NULL(config_lower);
    IFX_ERR_BRK_NULL(config_upper);

    const auto* sensor_info = handle->get_sensor_info();

    config_lower->sample_rate_Hz = 500'000u; // 500kHz, 80kHz could be also, because of default ADC configs (38.6MHz clock sets 80kHz)
    config_lower->rx_mask = 1; // one rx antenna activated
    config_lower->tx_mask = 1; // one tx antenna activated
    config_lower->tx_power_level = TX_POWER_LEVEL_LOWER;
    config_lower->if_gain_dB = IF_GAIN_DB_LOWER;
    config_lower->start_frequency_Hz = 57'500'000'000u; // 57.5GHz
    config_lower->end_frequency_Hz = 57'500'000'000u; // 57.5GHz
    config_lower->num_samples_per_chirp = 2;  //Strata has an issue to not pack odd 12-bit samples to even number of bytes, CW supports the same lower limit
    config_lower->num_chirps_per_frame = 1;
    config_lower->chirp_repetition_time_s = 10e-6f; // 10µs
    config_lower->frame_repetition_time_s = 0.02f; // frame rate of 50Hz
    config_lower->mimo_mode = IFX_MIMO_OFF;

    config_upper->sample_rate_Hz = 2'500'000u; // 2.5MHz
    config_upper->rx_mask = (1 << sensor_info->num_rx_antennas) - 1; // all rx antennas activated
    config_upper->tx_mask = 1 << (sensor_info->num_tx_antennas - 1); // all tx antennas activated
    config_upper->tx_power_level = TX_POWER_LEVEL_UPPER;
    config_upper->if_gain_dB = IF_GAIN_DB_UPPER;
    config_upper->start_frequency_Hz = 64'300'000'000u; // 64.3GHz
    config_upper->end_frequency_Hz = 64'300'000'000u; // 64.3GHz
    config_upper->num_samples_per_chirp = 1024;
    config_upper->num_chirps_per_frame = 512;
    config_upper->chirp_repetition_time_s = 0.01f; // 10ms
    config_upper->frame_repetition_time_s = 10; // frame rate of 0.1Hz
    config_upper->mimo_mode = sensor_info->num_tx_antennas > 1 ? IFX_MIMO_TDM : IFX_MIMO_OFF; // IFX_MIMO_TDM if device has more than one RX antenna
}


//----------------------------------------------------------------------------

ifx_Radar_Sensor_t ifx_avian_get_sensor_type(const ifx_Avian_Device_t* handle)
{
    IFX_ERR_BRV_NULL(handle, IFX_RADAR_SENSOR_UNKNOWN);
    return handle->get_sensor_type();
}

//----------------------------------------------------------------------------

Infineon::Avian::Driver* ifx_avian_get_avian_driver(const ifx_Avian_Device_t* handle)
{
    IFX_ERR_BRV_NULL(handle, nullptr);
    return handle->get_avian_driver();
}

//----------------------------------------------------------------------------
BoardInstance* ifx_avian_get_strata_avian_board(const ifx_Avian_Device_t* handle)
{
    IFX_ERR_BRV_NULL(handle, nullptr);
    return handle->get_strata_avian_board();
}

//----------------------------------------------------------------------------

Infineon::Avian::StrataPort* ifx_avian_get_strata_avian_port(const ifx_Avian_Device_t* handle)
{
    IFX_ERR_BRV_NULL(handle, nullptr);
    return handle->get_strata_avian_port();
}

Infineon::Avian::Constant_Wave_Controller* ifx_avian_get_constant_wave_controller(ifx_Avian_Device_t* handle)
{
    IFX_ERR_BRV_NULL(handle, nullptr);
    return handle->get_constant_wave_controller();
}

Infineon::Avian::Constant_Wave_Controller* ifx_Radar_Device_s::get_constant_wave_controller()
{
    if (!m_cw_controller)
    {
        auto* avian_port = get_strata_avian_port();
        m_cw_controller = std::make_unique<Avian::Constant_Wave_Controller>(*avian_port);
    }

    return m_cw_controller.get();
}
