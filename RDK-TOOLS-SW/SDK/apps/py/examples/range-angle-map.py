# ===========================================================================
# Copyright (C) 2022 Infineon Technologies AG
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# ===========================================================================

import pprint
import matplotlib.pyplot as plt
import numpy as np
from scipy import signal

from ifxRadarSDK import *
from internal.fft_spectrum import *
from DBF import DBF
from doppler import DopplerAlgo

def num_rx_antennas_from_config(config):
    rx_mask = config["rx_mask"]

    # popcount for rx_mask
    c = 0
    for i in range(32):
        if rx_mask & (1 << i):
            c += 1
    return c


class LivePlot:
    def __init__(self, max_angle_degrees : float, max_range_m : float):
        # max_angle_degrees: maximum supported speed
        # max_range_m:   maximum supported range
        self.h = None
        self.max_angle_degrees = max_angle_degrees
        self.max_range_m = max_range_m

        self.fig, self._ax = plt.subplots(nrows=1, ncols=1)

        self.fig.canvas.manager.set_window_title("Range-Angle-Map using DBF")

    def _draw_first_time(self, data : np.ndarray):
        # First time draw

        minmin = -60
        maxmax = 0

        self.h = self._ax.imshow(
                   data,
                   vmin=minmin, vmax=maxmax,
                   cmap='viridis',
                   extent=(-self.max_angle_degrees,
                           self.max_angle_degrees,
                           0,
                           self.max_range_m),
                   origin='lower')

        self._ax.set_xlabel("angle (degrees)")
        self._ax.set_ylabel("distance (m)")
        self._ax.set_aspect("auto")

        self.fig.subplots_adjust(right=0.8)
        cbar_ax = self.fig.add_axes([0.85, 0.0, 0.03, 1])

        cbar = self.fig.colorbar(self.h, cax=cbar_ax)
        cbar.ax.set_ylabel("magnitude (a.u.)")

    def _draw_next_time(self, data : np.ndarray):
        # Update data for each antenna

        self.h.set_data(data)

    def draw(self, data : np.ndarray, title : str):
        if self.h:
            self._draw_next_time(data)
        else:
            self._draw_first_time(data)
        self._ax.set_title(title)
        plt.draw()
        plt.pause(1e-3)


if __name__ == '__main__':
    num_beams = 27         # number of beams
    max_angle_degrees = 40 # maximum angle, angle ranges from -40 to +40 degrees

    config = {
        'sample_rate_Hz': 1_000_000,       # 1MHZ
        'rx_mask': 5,                      # activate RX1 and RX3
        'tx_mask': 1,                      # activate TX1
        'if_gain_dB': 33,                  # gain of 33dB
        'tx_power_level': 31,              # TX power level of 31
        'start_frequency_Hz': 60e9,        # 60GHz 
        'end_frequency_Hz': 61.5e9,        # 61.5GHz
        'num_chirps_per_frame': 128,       # 128 chirps per frame
        'num_samples_per_chirp': 64,       # 64 samples per chirp
        'chirp_repetition_time_s': 0.0005, # 0.5ms
        'frame_repetition_time_s': 0.15,   # 0.15s, frame_Rate = 6.667Hz
        'mimo_mode': 'off'                 # MIMO disabled
    }

    with Device() as device:
        # set configuration
        device.set_config(**config)

        # get metrics and print them
        metrics = device.metrics_from_config(**config)
        pprint.pprint(metrics)

        # get maximum range
        max_range_m = metrics["max_range_m"]

        # Create frame handle
        num_rx_antennas = num_rx_antennas_from_config(config)

        # Create objects for Rangle-Doppler, DBF, and plotting.
        doppler = DopplerAlgo(config, num_rx_antennas)
        dbf = DBF(num_rx_antennas, num_beams = num_beams, max_angle_degrees = max_angle_degrees)
        plot = LivePlot(max_angle_degrees, max_range_m)

        while True:
            # frame has dimension num_rx_antennas x num_samples_per_chirp x num_chirps_per_frame
            frame = device.get_next_frame()

            rd_spectrum = np.zeros((config["num_samples_per_chirp"], 2*config["num_chirps_per_frame"], num_rx_antennas), dtype=complex)

            beam_range_energy = np.zeros((config["num_samples_per_chirp"], num_beams))

            for i_ant in range(num_rx_antennas): #For each antenna
                mat = frame[i_ant, :, :]

                # Compute Doppler spectrum
                dfft_dbfs = doppler.compute_doppler_map(mat, i_ant)
                rd_spectrum[:,:,i_ant] = dfft_dbfs

            # Compute Range-Angle map
            rd_beam_formed = dbf.run(rd_spectrum)
            for i_beam in range(num_beams):
                doppler_i = rd_beam_formed[:,:,i_beam]
                beam_range_energy[:,i_beam] += np.linalg.norm(doppler_i, axis=1) / np.sqrt(num_beams)

            # Maximum energy in Range-Angle map
            max_energy = np.max(beam_range_energy)

            # Rescale map to better capture the peak
            scale = 150
            beam_range_energy = scale*(beam_range_energy/max_energy - 1)

            # Find dominant angle of target
            _, idx = np.unravel_index(beam_range_energy.argmax(), beam_range_energy.shape)
            angle_degrees = np.linspace(-max_angle_degrees, max_angle_degrees, num_beams)[idx]

            # And plot...
            plot.draw(beam_range_energy, f"Range-Angle map using DBF, angle={angle_degrees:+02.0f} degrees")
