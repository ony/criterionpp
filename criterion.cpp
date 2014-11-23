//
//  Simple library for measuring the performance of C++ code
//    Based on http://hackage.haskell.org/package/criterion
//
//  Copyright (C) 2014 Mykola Orliuk <virkony@gmail.com>
//
//  This library is free software; you can redistribute it and/or modify it
//  under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation; either version 2.1 of the License, or (at
//  your option) any later version.
//
//  This library is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
//  License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this library; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
//

#include "criterion.hpp"

#include <iostream>

#include <sstream>
#include <iomanip>
#include <limits>

using namespace std;

namespace {
    constexpr std::chrono::milliseconds threshold {300};
} // anonymous namespace

namespace criterion {
    void enforce(int) noexcept {};

    std::ostream &operator<<(std::ostream &os, const measure &m) noexcept
    {
        os << m.iters << " for " << human(m.time) << " (cpu " << human(m.cpu_time) << ")";
        if (m.iters > 0)
        {
            os << " ~ " << human(m.time / m.iters) << "/cycle, cpu " << human(m.cpu_time / m.iters) << "/cycle";
        }
        return os;
    }

    std::string human(std::chrono::duration<double> t) noexcept
    {
        using namespace std::chrono;

        ostringstream os;

        double k = t.count();
        if (k < 0) k = -k, os << '-';

        const char *suffix;
        if (t >= seconds(1)) suffix = " s";
        else if (t >= milliseconds(1)) k *= 1e3, suffix = " ms";
        else if (t >= microseconds(1)) k *= 1e6, suffix = " us";
        else if (t >= nanoseconds(1)) k *= 1e9, suffix = " ns";
        else k *= 1e12, suffix = " ps";

        if (k >= 1e9) os << setprecision(4);
        else if (k >= 1e3) os << setprecision(0) << fixed;
        else if (k >= 1e2) os << setprecision(1) << fixed;
        else if (k >= 1e1) os << setprecision(2) << fixed;
        else os << setprecision(3) << fixed;
        os << k << suffix;
        return os.str();
    }

    std::vector<measure> benchmark(benchmarkable run, sysclock::duration time_limit) noexcept
    {
        double iters_state = 1;
        size_t iters = size_t(iters_state);

        std::vector<measure> ms;

        sysclock::duration over_thresh {0};
        auto start_time = sysclock::now();
        for (size_t count = 0;;++count)
        {
            ms.emplace_back();
            measure &m = ms.back();
            auto end_time = m.run(run, iters);

            cerr << m << endl;
            ms.push_back(m);

            auto thresh_gross = m.time - threshold;
            if (thresh_gross > seconds(0)) over_thresh += thresh_gross;

            auto time_taken = end_time - start_time;
            if (time_taken >= time_limit &&
                over_thresh > (threshold * 10) &&
                count >= 4)
            {
                if (time_taken > time_limit * 1.25)
                {
                    cerr << "measurement took " << human(time_taken) << endl;
                }
                return ms;
            }

            for (;;) // next iters
            {
                iters_state *= 1.05;
                auto next_iters = size_t(iters_state);
                if (next_iters > iters)
                {
                    iters = next_iters;
                    break;
                }
                else if (iters_state > std::numeric_limits<decltype(iters)>::max())
                {
                    return ms;
                }
            }
        }
    }

} // namespace criterion
