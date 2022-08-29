% ===========================================================================
% Copyright (C) 2021 Infineon Technologies AG
%
% Redistribution and use in source and binary forms, with or without
% modification, are permitted provided that the following conditions are met:
%
% 1. Redistributions of source code must retain the above copyright notice,
%    this list of conditions and the following disclaimer.
% 2. Redistributions in binary form must reproduce the above copyright
%    notice, this list of conditions and the following disclaimer in the
%    documentation and/or other materials provided with the distribution.
% 3. Neither the name of the copyright holder nor the names of its
%    contributors may be used to endorse or promote products derived from
%    this software without specific prior written permission.
%
% THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
% AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
% IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
% ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
% LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
% CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
% SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
% INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
% CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
% ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
% POSSIBILITY OF SUCH DAMAGE.
% ===========================================================================

classdef RadarDevice < handle
    %RADARDEVICE Class for a radar device.
    %   An object of this class helps to
    %   - configure a radar device and connect to it
    %   - retrieve radar data from it
    %   - disconnect from it.
    %   This object may be used as a Radar SDK wrapper in matlab,
    %   as it performs the above tasks similar to Radar SDK


    % TODO: When 'connect' failed, the hardware device needs to be reset. But the
    % 'dev_handle' is invalid

    properties
        oDevConf;   % DeviceConfig object
        cw_control_handle; % Reference to Constant Wave Control Object handler. It protects from destroying CW when the device handle is no longer available.
    end

    properties (Access = private)
        dll_ver;            % sdk dll version
        device_handle;      % device handle given by radar sdk
    end

    methods
        function obj = RadarDevice()
            %RADARDEVICE Constructor for an instance of RadarDevice
            % and connects to available board.
            obj.oDevConf = 0;
            obj.cw_control_handle = 0;
            try
                [ec, ver] = DeviceControlM('get_version');
                obj.check_error_code(ec);
                obj.dll_ver = ver;
                [ec, handle] = DeviceControlM('create');
                obj.check_error_code(ec);
                obj.device_handle = handle;
                disp('Radar SDK Matlab MEX Wrapper: Radar handle created and device connected');
            catch ME
                rethrow(ME)
            end
        end

        function set_config(obj, oDevConf)
            %SET_CONFIG Configures the board instance with configuration
            % object parameter
            obj.oDevConf = oDevConf;
            try
                ec = DeviceControlM('set_config', obj.device_handle, obj.oDevConf);
                obj.check_error_code(ec);
                disp('Radar SDK Matlab MEX Wrapper: Radar Device configuration updated');
            catch ME
                rethrow(ME);
            end
        end

        function config = get_config(obj)
            config = obj.oDevConf;
        end

        function disconnect(obj)
            %DISCONNECT disconnect from a Radar Device attached via USB
            %   This method disconnects from the port where the radar device is connected
            if obj.device_handle ~= 0
                if(obj.cw_control_handle ~= 0)
                    obj.cw_control_handle.delete();
                end
                ec = DeviceControlM('destroy', obj.device_handle);
                obj.check_error_code(ec);
                obj.device_handle = 0;
                disp('Radar SDK Matlab MEX Wrapper: Radar handle released and device disconnected.');
            end
        end

        function constant_wave_control = create_cw_control(obj)
            %CREATE_CW_CONTROL Creates and initializes the constant wave mode controller instance for a connected device
            
            obj.cw_control_handle = ConstantWaveControl(obj.device_handle);
            constant_wave_control = obj.cw_control_handle;
        end

        function reg_list_string = get_register_list_string(obj, trigger)
            %GET_REG_LIST_STRING get register contents from a Radar Device attached via USB
            %   This method returns the device register list in hexadecimal string format
            try
                [err_code, reg_list_string] = DeviceControlM('get_register_list_string',obj.device_handle, trigger);
                obj.check_error_code(err_code);
            catch ME
                rethrow(ME);
            end
        end

        function RxFrame = get_next_frame(obj, timeout_ms_opt)
            %GET_NEXT_FRAME_DATA method to fetch radar data from radar device
            %   This method fetches data from the radar device in terms of one
            %   frame (all samples of all chirps from all enabled antennas)
            %   and returns a Frame object
            RxFrame = [];
            if(isa(obj.get_config(),'DeviceConfig'))
                if exist('timeout_ms_opt','var')
                    [ec, num_rx, num_samples_per_chirp, num_chirps_per_frame, Total_samples] = DeviceControlM('get_next_frame_timeout', obj.device_handle, timeout_ms_opt);
                else
                    [ec, num_rx, num_samples_per_chirp, num_chirps_per_frame, Total_samples] = DeviceControlM('get_next_frame', obj.device_handle);
                end
                obj.check_error_code(ec);
                RxFrame = Frame(num_rx, num_chirps_per_frame, num_samples_per_chirp);
                RxFrame.set_data_all_antenna(Total_samples);
            else
                disp('Radar not configured. Please Configure Radar and try again');
            end
        end

        function Started = start_acquisition(obj)
            %START_ACQUISITION starts the acquisition of raw data
            %   This method starts the acquisition of raw data when the radar device is connected
            if obj.device_handle ~= 0
                if (isa(obj.get_config(),'DeviceConfig'))
                    [ec, Started] = DeviceControlM('start_acquisition', obj.device_handle);
                    obj.check_error_code(ec);
                    if(Started)
                        disp('Radar SDK Matlab MEX Wrapper: acquisition of raw data has started.');
                    end
                else
                    Started = false;
                    disp('Radar not configured. Please Configure Radar and try again');
                end
            end
        end

        function Stopped = stop_acquisition(obj)
            %STOP_ACQUISITION stops the acquisition of raw data
            %   This method stops the acquisition of raw data when the radar device is connected
            if obj.device_handle ~= 0
                [ec, Stopped] = DeviceControlM('stop_acquisition', obj.device_handle);
                obj.check_error_code(ec);
                if(Stopped)
                  disp('Radar SDK Matlab MEX Wrapper: acquisition of raw data has stopped.');
                end
            end
        end

        function sensor_info = get_sensor_information(obj)
            %GET_SENSOR_INFORMATION Gets information about the connected device
            try
                oSensorInfo = SensorInfo();
                [err_code, sensor_info] = DeviceControlM('get_sensor_information', obj.device_handle, oSensorInfo);
                obj.check_error_code(err_code);
            catch ME
                rethrow(ME);
            end
        end

        function fw_info = get_firmware_information(obj)
            %GET_FIRMWARE_INFORMATION Gets information about the firmware of a connected device
            try
                oFwInfo = FirmwareInfo();
                [err_code, fw_info] = DeviceControlM('get_firmware_information', obj.device_handle, oFwInfo);
                obj.check_error_code(err_code);
            catch ME
                rethrow(ME);
            end
        end

        function shield_info = get_shield_information(obj)
            %GET_SHIELD_INFORMATION Get information about a connected device RF shield by reading its EEPROM
            try
                oShieldInfo = ShieldInfo();
                [err_code, shield_info] = DeviceControlM('get_shield_information', obj.device_handle, oShieldInfo);
                obj.check_error_code(err_code);
            catch ME
                rethrow(ME);
            end
        end

        function delete(obj)
            %RADARDEVICE destroyer for the RadarDevice object
            obj.disconnect();
        end
    end

    methods(Static)
        function version = get_version_full();
            % Returns the full SDK version string including
            % the git tag from which this release was built
            [ec, version] = DeviceControlM('get_version_full');
        end

        function version = get_version();
            % Returns the short SDK version string (excluding
            % the git tag from which this release was built)
            [ec, version] = DeviceControlM('get_version');
        end
    end

    methods (Static)
        function ret = check_error_code(err_code)
            ret = (err_code ~= 0);
            if(ret)
                description = DeviceControlM(':describe_error', err_code);
                ME = MException(['RadarDevice:error' num2str(err_code)], description);
                throw(ME);
            end
        end
    end

end

