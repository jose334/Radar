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
 * @file RangeAngleImage_Util.h
 * 
 * Internal helper functions used by the RangeAngleImage* modules
 * 
 */
#ifndef IFX_RADAR_RANGE_ANGLE_UTIL_H
#define IFX_RADAR_RANGE_ANGLE_UTIL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    /* On Windows symbols (functions) in a DLL (shared library) are not exported by
     * default. You have to tell the linker to export the symbol when you build the
     * library and you have to tell the linker (?) to import the symbol when using
     * the library. This is done with __declspec:
     *     Compiling the library: __declspec(dllexport) void foo(void);
     *     Using the library:     __declspec(dllimport) void foo(void);
     * More information can be found here:
     * https://stackoverflow.com/questions/33062728/cmake-link-shared-library-on-windows/41618677
     *
     * In contrast, on Linux the default is to export all symbols. For gcc
     * (probably also clang), see https://gcc.gnu.org/wiki/Visibility.
     *
     * The build system has to set correct preprocessor defines (cmake works).
     * - If the library is build as a static library, no preprocessor name is
     *   needed.
     * - If the library is build as a dynamic library:
     *      - Windows: radar_sdk_EXPORTS must be set when compiling the library
     *      - Windows: radar_sdk_EXPORTS must not be set when linking
     *      - Linux: No preprocessor defines are needed
     */
#ifdef RADAR_SDK_BUILD_STATIC
     // build as static library; no visibility
#define RAI_DLL_PUBLIC
#define RAI_DLL_HIDDEN
#elif defined(_MSC_VER) || defined(__MINGW64__) || defined(__WIN32__)
     // default visibility is hidden, so RAI_DLL_HIDDEN is a noop
#define RAI_DLL_HIDDEN

#ifndef RAI_DLL_PUBLIC
#ifdef radar_sdk_EXPORTS
    // We are building this library
#define RAI_DLL_PUBLIC __declspec(dllexport)
#else
    // We are using this library
#define RAI_DLL_PUBLIC __declspec(dllimport)
#endif
#endif
#elif (__GNUC__ >= 4) || (__clang_major__ >= 5)
     // see https://gcc.gnu.org/wiki/Visibility
#define RAI_DLL_PUBLIC __attribute__ ((visibility ("default")))
#define RAI_DLL_HIDDEN __attribute__ ((visibility ("hidden")))
#else
#define RAI_DLL_PUBLIC
#define RAI_DLL_HIDDEN
#endif

#ifdef _MSC_VER
#define RAI_NOINLINE __declspec(noinline)
#else
#define RAI_NOINLINE __attribute__((noinline))
#endif

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

/* type for complex numbers used by this module */
typedef struct {
    float real, imag;
} rai_cmplx_t;

RAI_DLL_HIDDEN
void rai_window_init_blackman(float* win, uint32_t length);

RAI_DLL_HIDDEN
void rai_window_init_chebyshev(
    float* win,
    uint32_t length,
    float at_dB);


#endif /* IFX_RADAR_RANGE_ANGLE_IMAGE_H */
