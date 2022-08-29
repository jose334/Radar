/* ===========================================================================
** Copyright (C) 2021 Infineon Technologies AG
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
 * @file Segmentation.h
 *
 * \brief \copybrief gr_segmentation
 *
 * For details refer to \ref gr_segmentation
 */

#ifndef IFX_RADAR_SEGMENTATION_H
#define IFX_RADAR_SEGMENTATION_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include "ifxBase/Types.h"
#include "ifxBase/Cube.h"
#include "ifxBase/Matrix.h"
#include "ifxBase/Vector.h"

#include "ifxAvian/DeviceConfig.h"

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

/** @addtogroup gr_cat_RadarSegmentation
* @{
*/

/** @defgroup gr_segmentation Segmentation
  * @brief API for Segmentation algorithm
  *
  * Calculates presence and absence in each segment, frame by frame.
  *
  * A recommended height of sensor for optimum performance for human target
  * tracking (sitting on couch, standing, walking) is 75cm-150cm (30-60 inches)
  * from ground with no tilt. A description of the orientation of the sensor can
  * be found here: \ref ifx_Orientation_t
  *
  * @{
  */

/**
 * \brief This enumeration type lists the detection sensitivity levels.
 * @brief Orientation of radar sensor
 *
 * @image html orientation_sensor.png "Portrait and landscape orientation of sensor" width=400px
 */
typedef enum {
    IFX_ORIENTATION_LANDSCAPE = 0U, /**< Sensor is oriented in landscape format (default) */
    IFX_ORIENTATION_PORTRAIT  = 1U, /**< Sensor is oriented in portrait format */
    IFX_ORIENTATION_LANDSCAPE_FLIPPED = 2U,  /**< Sensor is oriented in landscape format but flipped 180 degrees */
    IFX_ORIENTATION_PORTRAIT_FLIPPED = 3U, /**< Sensor is oriented in portrait format  but flipped 180 degrees */
} ifx_Orientation_t;

/* Check if orientation is in portrait mode independent of being flipped */
#define IFX_ORIENTATION_IS_PORTRAIT(orientation) (((orientation) & 1) != 0)
/* Check if orientation is flipped independent of landscape or portrait mode */
#define IFX_ORIENTATION_IS_FLIPPED(orientation) (((orientation) & 2) != 0)

/**
 * @brief A handle for an instance of Segmentation module, see Segmentation.h
 */
typedef struct ifx_Segmentation_s ifx_Segmentation_t;

IFX_TYPEDEF_DEPRECATED("ifx_Segmentation_Handle_t is deprecated and will be removed in RDK 3.3, use ifx_Segmentation_t instead.", ifx_Segmentation_t*, ifx_Segmentation_Handle_t);

/**
 * @brief Segmentation modes
 */
typedef enum {
    IFX_SEGMENTATION_500MHZ_LANDSCAPE = 0u, /**< bandwidth of 500MHz, landscape orientation */
    IFX_SEGMENTATION_500MHZ_PORTRAIT = 1u, /**< bandwidth of 500MHz, portrait orientation*/
    IFX_SEGMENTATION_1GHZ_LANDSCAPE = 2u, /**< bandwidth of 1GHz, landscape orientation*/
    IFX_SEGMENTATION_1GHZ_PORTRAIT = 3u  /**< bandwidth of 1GHz, portrait orientation*/
} ifx_Segmentation_Mode_t;

/**
 * @brief Defines the structure for Segmentation module related settings.
 */
typedef struct
{
    uint32_t        num_samples_per_chirp;      /**< Number of samples per chirp. */
    uint32_t        num_chirps_per_frame;       /**< Number of chirps per frame. */
    ifx_Float_t     bandwidth_Hz;               /**< Frequency bandwidth in Hz. */
    ifx_Float_t     center_frequency_Hz;        /**< Center frequency in Hz. */
    ifx_Orientation_t orientation;              /**< Orientation of the sensor \ref ifx_Orientation_t */
} ifx_Segmentation_Config_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @brief Creates a Segmentation handle (object), based on the input parameters.
 *
 * \attention It is currently not possible to run two instances of segmentation
 *            in parallel. There must not be more than one segmentation object
 *            at the same time.
 *
 * @param [in]     config    Segmentation configurations defined by \ref ifx_Segmentation_Config_t.
 * @return Handle to the newly created instance or NULL in case of failure.
 */
IFX_DLL_PUBLIC
ifx_Segmentation_t* ifx_segmentation_create(const ifx_Segmentation_Config_t* config);

/**
 * @brief Creates a Segmentation handle from a specific configuration
 *
 * The function creates a segmentation handle for a specific scenario described
 * by mode. The associated device configuration is written to device_config.
 * The caller is responsible for setting the device configuration.
 *
 * \attention It is currently not possible to run two instances of segmentation
 *            in parallel. There must not be more than one segmentation object
 *            at the same time.
 *
 * @param [in]     mode             Segmentation configurations defined by \ref ifx_Segmentation_Config_t.
 * @param [out]    device_config    Device configuration
 * @return Handle to the newly created instance or NULL in case of failure.
 */
IFX_DLL_PUBLIC
ifx_Segmentation_t* ifx_segmentation_create_from_mode(ifx_Segmentation_Mode_t mode, ifx_Avian_Config_t* device_config);

/**
 * @brief Performs the segmentation algorithm on given raw data.
 *
 * The input is a cube with dimensions num_rx_antennas x num_chirps_per_frame x num_samples_per_chirp.
 *
 * The function outputs 6 segments. The vector segments must have a length of
 * 6. A value of 0 means that no target was detected, while 1 means that a
 * target was detected in the respective segment. Each segment covers 20
 * degrees, the total field of view is 120 degrees (from -60 to +60 degrees).
 *
 * The function outputs the information about the tracks to tracks. The matrix
 * tracks must have dimension 5 x 4. Each row contains one track. The track is
 * valid if the first element, the track_id, is a positive number. Each row
 * consists of the track id (starting from one), the range (in meters), the
 * angle (in degrees), and the speed (in m/s):
 * - IFX_MAT_AT(tracks,row,0): track_id (positive if a valid track)
 * - IFX_MAT_AT(tracks,row,1): range (in m)
 * - IFX_MAT_AT(tracks,row,2): angle (in degrees)
 * - IFX_MAT_AT(tracks,row,3): speed (in m/s)
 *
 * The maxmimum number of valid tracks is 5. The maximum range is 5 meters.
 *
 * @param [in]     handle              A handle to the Segmentation object.
 * @param [in]     frame               Input raw data (num_rx_antennas x num_chirps_per_frame x num_samples_per_chirp).
 * @param [out]    segments            Result of segments occupation.
 * @param [out]    tracks              Tracks consisting of track_id, range (in m), angle (in degrees), and speed (in m/s)
 */
IFX_DLL_PUBLIC
void ifx_segmentation_run(ifx_Segmentation_t* handle,
                          const ifx_Cube_R_t* frame,
                          ifx_Vector_R_t* segments,
                          ifx_Matrix_R_t* tracks);

/**
 * @brief Destroys Segmentation handle (object) to clear internal states and memories.
 *
 * @param [in]  handle      A handle to the Segmentation object.
 */
IFX_DLL_PUBLIC
void ifx_segmentation_destroy(ifx_Segmentation_t* handle);

/**
 * @brief By default the orientation of the sensor is set to IFX_ORIENTATION_LANDSCAPE.
 *        This setter can be used to change the orientation.
 * 
 * @param [in]    handle       A handle to the Segmentation object.
 * @param [in]    orientation  New orientation to use see \ref ifx_Orientation_t.
 */
IFX_DLL_PUBLIC
void ifx_segmentation_set_orientation(ifx_Segmentation_t* handle, ifx_Orientation_t orientation);

/**
 * @brief 
 * 
 * @param [in]    handle       A handle to the Segmentation object.
 * @return ifx_Orientation_t   Current sensor orientation \ref ifx_Orientation_t.
 */
IFX_DLL_PUBLIC
ifx_Orientation_t ifx_segmentation_get_orientation(ifx_Segmentation_t* handle);

/**
  * @}
  */
 
/**
  * @}
  */ 

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* IFX_RADAR_SEGMENTATION_H */
