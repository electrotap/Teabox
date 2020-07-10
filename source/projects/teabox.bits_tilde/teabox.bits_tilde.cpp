/// @file
/// @copyright Copyright 2020 Timothy Place. All rights reserved.
/// @license         Use of this source code is governed by the MIT License found in the License.md file.
///
/// Teabox bit decoder object for Max
/// Original written by Tim Place
/// Copyright 2004 by Electrotap L.L.C.


#include "c74_min.h"

using namespace c74::min;


class teabox_bits : public object<teabox_bits>, public sample_operator<1,16> {
public:
    MIN_DESCRIPTION { "Decode the output of digital sensors from a Teabox sensor interface." };
    MIN_TAGS        { "sensor, interface" };
    MIN_AUTHOR      { "Timothy Place, Electrotap" };
    MIN_RELATED     { "teabox~, teabox.count~" };


    inlet<>  m_in1 {this, "(signal) bitmasked input" };

    outlet<> m_out1 {this, "(signal) bit decoded output 1", "signal" };
    outlet<> m_out2 {this, "(signal) bit decoded output 2", "signal" };
    outlet<> m_out3 {this, "(signal) bit decoded output 3", "signal" };
    outlet<> m_out4 {this, "(signal) bit decoded output 4", "signal" };
    outlet<> m_out5 {this, "(signal) bit decoded output 5", "signal" };
    outlet<> m_out6 {this, "(signal) bit decoded output 6", "signal" };
    outlet<> m_out7 {this, "(signal) bit decoded output 7", "signal" };
    outlet<> m_out8 {this, "(signal) bit decoded output 8", "signal" };
    outlet<> m_out9 {this, "(signal) bit decoded output 9", "signal" };
    outlet<> m_out10 {this, "(signal) bit decoded output 10", "signal" };
    outlet<> m_out11 {this, "(signal) bit decoded output 11", "signal" };
    outlet<> m_out12 {this, "(signal) bit decoded output 12", "signal" };
    outlet<> m_out13 {this, "(signal) bit decoded output 13", "signal" };
    outlet<> m_out14 {this, "(signal) bit decoded output 14", "signal" };
    outlet<> m_out15 {this, "(signal) bit decoded output 15", "signal" };
    outlet<> m_out16 {this, "(signal) bit decoded output 16", "signal" };


    samples<16> operator()(sample value) {
        auto bitmask = static_cast<int>(value);
        return {
            static_cast<sample>((bitmask & 1) > 0),
            static_cast<sample>((bitmask & 2) > 0),
            static_cast<sample>((bitmask & 4) > 0),
            static_cast<sample>((bitmask & 8) > 0),
            static_cast<sample>((bitmask & 16) > 0),
            static_cast<sample>((bitmask & 32) > 0),
            static_cast<sample>((bitmask & 64) > 0),
            static_cast<sample>((bitmask & 128) > 0),
            static_cast<sample>((bitmask & 256) > 0),
            static_cast<sample>((bitmask & 512) > 0),
            static_cast<sample>((bitmask & 1024) > 0),
            static_cast<sample>((bitmask & 2048) > 0),
            static_cast<sample>((bitmask & 4096) > 0),
            static_cast<sample>((bitmask & 8192) > 0),
            static_cast<sample>((bitmask & 16384) > 0),
            static_cast<sample>((bitmask & 32768) > 0)
        };
    }
};

MIN_EXTERNAL(teabox_bits);
