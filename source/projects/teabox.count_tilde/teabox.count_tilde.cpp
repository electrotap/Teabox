/// @file
/// @copyright Copyright 2020 Timothy Place. All rights reserved.
/// @license         Use of this source code is governed by the MIT License found in the License.md file.
///
/// Teabox counter object for Max
/// Original written by Tim Place
/// Copyright 2005 by Electrotap L.L.C.


#include "c74_min.h"

using namespace c74::min;


class teabox_count : public object<teabox_count>, public sample_operator<1,1> {
private:
    long m_value {};

public:
    MIN_DESCRIPTION { "Determine the current index of data signals originating from a Teabox sensor interface." };
    MIN_TAGS        { "sensor, interface" };
    MIN_AUTHOR      { "Timothy Place, Electrotap" };
    MIN_RELATED     { "teabox.bits~, teabox~" };


    inlet<>  m_in1 {this, "(signal) input from Teabox" };
    outlet<> m_out1 {this, "(signal) index", "signal" };


    sample operator()(sample value) {
        if (value < 0)
            m_value = 0;
        else
            ++m_value;
        return m_value;
    }
};

MIN_EXTERNAL(teabox_count);
