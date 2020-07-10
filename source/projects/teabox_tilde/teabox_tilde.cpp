/// @file
/// @copyright Copyright 2020 Timothy Place. All rights reserved.
/// @license         Use of this source code is governed by the MIT License found in the License.md file.
///
/// Teabox demuxer object for Max
/// Original written by Tim Place
/// Copyright 2004 by Electrotap L.L.C.


#include "c74_min.h"

using namespace c74::min;


class teabox : public object<teabox>, public sample_operator<1,9> {
private:
    float   m_hw_version {};    // version number (major.minor) of the connected Teabox
    char    m_counter {};       // used for keeping track of the current sensor number
    float   m_data[9];          // one container for the data from each sensor
    float   m_last_value;       // used for error correction in the perform method

public:
    MIN_DESCRIPTION { "Demultiplex data signals originating from a Teabox sensor interface." };
    MIN_TAGS        { "sensor, interface" };
    MIN_AUTHOR      { "Timothy Place, Electrotap" };
    MIN_RELATED     { "teabox.bits~, teabox.count~" };


    inlet<>  m_in1 {this, "(signal) input from Teabox" };

    outlet<> m_out1 {this, "(signal) demultiplexed sensor signal 1", "signal" };
    outlet<> m_out2 {this, "(signal) demultiplexed sensor signal 2", "signal" };
    outlet<> m_out3 {this, "(signal) demultiplexed sensor signal 3", "signal" };
    outlet<> m_out4 {this, "(signal) demultiplexed sensor signal 4", "signal" };
    outlet<> m_out5 {this, "(signal) demultiplexed sensor signal 5", "signal" };
    outlet<> m_out6 {this, "(signal) demultiplexed sensor signal 6", "signal" };
    outlet<> m_out7 {this, "(signal) demultiplexed sensor signal 7", "signal" };
    outlet<> m_out8 {this, "(signal) demultiplexed sensor signal 8", "signal" };
    outlet<> m_out9 {this, "(signal) digital sensors encoded as an integer", "signal" };
    outlet<> out_dump {this, "(attributes) dumpout" };


    message<> m_getversion { this, "getversion", "Query for the Teabox firmware version.",
        MIN_FUNCTION {
            auto version_major = (int)((m_hw_version * 4095.0) + 0.49) >> 8;
            auto version_minor = (int)((m_hw_version * 4095.0) + 0.49) & 255;
            auto version_string = std::to_string(version_major);
            version_string += ".";
            version_string += std::to_string(version_minor);

            out_dump.send("version", version_string);
            return {};
        }
    };


    message<> m_getstatus { this, "getstatus", "Query for the Teabox firmware version.",
        MIN_FUNCTION {
            out_dump.send("status", m_hw_version ? 1 : 0);
            return {};
        }
    };


    message<> dspsetup {this, "dspsetup",
        MIN_FUNCTION {
           return m_getstatus();
       }
    };


    samples<9> operator()(sample value) {
        long bitmask;

        if (value < 0.0 || m_counter > 9) {         // If the sample is the start flag...
            if (m_last_value < 0.0)                 // Actually - if all 16 toggles on the Teabox digital inputs are high,
                m_data[8] = m_last_value;          // it will look identical to the start flag, so we compensate for that here.
            m_counter = 0;
        }
        else if (m_counter == 0) {                  // if the sample is hardware version number...
            m_hw_version = value * 8.0;
            ++m_counter;
        }
        else{
            m_data[m_counter - 1] = value * 8.0;    // Normalize the range
            ++m_counter;
        }

        // POST-PROCESS TOGGLE INPUT BITMASK
        if (m_data[8] < 0) {
            bitmask = m_data[8] * 32768;           // 4096 = 32768 / 8 (we already multiplied by 8)
            bitmask ^= -32768;
            bitmask = 32768 + (bitmask);           // 2^3
        }
        else
            bitmask = m_data[8] * 4096;            // 4096 = 32768 / 8 (we already multiplied by 8)

        m_last_value = value;
        return { m_data[0], m_data[1], m_data[2], m_data[3], m_data[4], m_data[5], m_data[6], m_data[7], (float)bitmask };
    }
};

MIN_EXTERNAL(teabox);
