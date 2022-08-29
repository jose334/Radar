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
 * @file RangeAngleImage7.h
 *
 */

#ifndef IFX_RADAR_RANGE_ANGLE_IMAGE7_H
#define IFX_RADAR_RANGE_ANGLE_IMAGE7_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/*
==============================================================================
   1. INCLUDE FILES
==============================================================================
*/

#include <stdint.h>
#include "RangeAngleImage_Util.h"

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

/**
* @brief A handle for an instance of Range Angle Image module, see RangeAngleImage.h.
*/
typedef struct ifx_RAI_Opt_s* ifx_RAI_Opt_t;

/**
    * @brief Defines the structure for Range Angle Image module related settings.
    */
typedef struct ifx_RAI_Opt_Config_s
{
    uint32_t  samples_per_chirp;  /**< number of samples per chirp */
    uint32_t  chirps_per_frame;   /**< number of chirps per frame */
    float     alpha_mti_filter;   /**< 2D MTI filter coefficient.*/
    uint8_t   num_beams;          /**< Number of beams.*/
    float     min_angle;          /**< Minimum angle on left side of FoV.*/
    float     max_angle;          /**< Maximum angle on right side of FoV.*/
    float     d_by_lambda;        /**< Ratio between antenna spacing 'd' and wavelength.*/
    uint32_t  num_of_images;      /**< Number of images (responses) for Range Angle Image.*/
    uint32_t  num_antennas;  /**< Number of virtual antennas.*/
} ifx_RAI_Opt_Config_t;

/*
==============================================================================
   4. FUNCTION PROTOTYPES
==============================================================================
*/

/**
 * @brief Creates a Range Angle Image handle (object), based on the input parameters
 *
 * @param [in] config        Range Angle Image configurations defined by \ref ifx_RAI_Opt_Config_t.
 *
 * @return Handle to the newly created instance or NULL in case of failure.
 *
 */
RAI_DLL_PUBLIC
ifx_RAI_Opt_t ifx_rai7_create(ifx_RAI_Opt_Config_t* config);

/**
 * @brief Calculates range angle image from real input raw data.
 *
 * @param [in]     handle    A handle to the Range Angle Image object
 * @param [in]     input     An array of real 2D raw ADC data matrices for all Rx channels of
 *                           dimension (nAntennas x nChirps x nSamples) in row-major format
 *
 * @param [out]    output    An array of real 2D matrices for range doppler image beams in of
 *                           dimension(nImages x nSamples x nBeams) in row-major format
 */
RAI_DLL_PUBLIC RAI_NOINLINE
void ifx_rai7_run_r(ifx_RAI_Opt_t handle,
                   const float* input,
                   float* output);


/**
 * @brief Performs destruction of Range Angle Image handle (object) to clear internal states and memories.
 *
 * @param [in]     handle    A handle to the Range Angle Image object
 *
 */
RAI_DLL_PUBLIC
void ifx_rai7_destroy(ifx_RAI_Opt_t handle);

/**
 * @brief Getter function to access SNR result
 *
 * @param [in]     handle    A handle to the Range Angle Image object
 * @return Pointer to SNR result
 */
RAI_DLL_PUBLIC
const float* ifx_rai7_get_snr(ifx_RAI_Opt_t handle);

/**
 * @brief Getter function to access RX spectrum result
 *
 * @param [in]     handle    A handle to the Range Angle Image object
 * @return Pointer to RX spectrum result
 */
RAI_DLL_PUBLIC
rai_cmplx_t* ifx_rai7_get_rx_spectrum(ifx_RAI_Opt_t handle);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* IFX_RADAR_RANGE_ANGLE_IMAGE_H */
