/****************************************************************************\
* Copyright (C) 2021 Infineon Technologies
*
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
*
\****************************************************************************/
#ifndef DATA_SETTINGS_BGT_RADAR
#define DATA_SETTINGS_BGT_RADAR 1


#define DATA_SETTINGS_BGT_RADAR_SIZE(count, aggregation) (2 * sizeof(uint16_t) * ((count) + (((aggregation) > 1) ? 1 : 0)))


#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
#else
#include <stdint.h>
#endif


static inline void DataSettingsBgtRadar_initialize(uint8_t *settings, const uint16_t readouts[][2], uint16_t count, uint16_t aggregation)
{
    for (uint16_t i = 0; i < count; i++)
    {
        const uint16_t readoutAddress = readouts[i][0];
        const uint16_t readoutCount   = readouts[i][1];

        *settings++ = readoutAddress & 0xFF;
        *settings++ = readoutAddress >> 8;
        *settings++ = readoutCount & 0xFF;
        *settings++ = readoutCount >> 8;
    }
    if (aggregation > 1)
    {
        aggregation--;
        *settings++ = aggregation & 0xFF;
        *settings++ = aggregation >> 8;
        *settings++ = 0;
        *settings   = 0;
    }
}

#ifdef __cplusplus

struct DataSettingsBgtRadar_t
{
    DataSettingsBgtRadar_t() :
        settings {nullptr},
        settingsSize {0}
    {}

    DataSettingsBgtRadar_t(const uint16_t readouts[][2], uint16_t count, uint16_t aggregation = 0) :
        DataSettingsBgtRadar_t()
    {
        initialize(readouts, count, aggregation);
    }

    template <size_t N>
    DataSettingsBgtRadar_t(const uint16_t (*readouts)[N][2], uint16_t aggregation = 0) :
        DataSettingsBgtRadar_t(*readouts, N, aggregation)
    {}

    ~DataSettingsBgtRadar_t()
    {
        if (settings)
        {
            delete[] settings;
        }
    }

    inline void initialize(const uint16_t readouts[][2], uint16_t count, uint16_t aggregation = 0)
    {
        if (settings)
        {
            delete[] settings;
        }

        settingsSize = DATA_SETTINGS_BGT_RADAR_SIZE(count, aggregation);
        if (settingsSize == 0)
        {
            return;
        }

        settings = new uint8_t[settingsSize];
        DataSettingsBgtRadar_initialize(settings, readouts, count, aggregation);
    }

    uint8_t *settings;
    uint16_t settingsSize;
};

#endif

#endif /* DATA_SETTINGS_BGT_RADAR H */
