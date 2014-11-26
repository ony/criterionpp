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
#include <algorithm>

using namespace std;

namespace {
    // The amount of time a benchmark must run for in order for us to have some
    // trust in the raw measurement. We set this threshold so that we can
    // generate enough data to later perform meaningful statistical analyses.
    // The threshold is 30 milliseconds. One use of runBenchmark must
    // accumulate more than 300 milliseconds of total measurements above this
    // threshold before it will finish.
    constexpr std::chrono::milliseconds threshold {30};
    constexpr std::chrono::milliseconds over_thresh_threshold {300};

    inline criterion::measure measure_zero_line(criterion::sysclock::duration time_limit = criterion::seconds(1))
    {
        const auto dummy = [](size_t n) {
            for (size_t i = 0; i < n; ++i) criterion::enforce(i);
        };

        auto sample = criterion::benchmark(dummy, time_limit);

        return criterion::median(sample);
    }
} // anonymous namespace

namespace criterion {
    void enforce(int) noexcept {};

    measure measure::zero_line = measure_zero_line();

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
        else if (t >= picoseconds(1)) k *= 1e12, suffix = " ps";
        else if (t >= femtoseconds(1)) k *= 1e15, suffix = " fs";
        else if (t >= attoseconds(1)) k *= 1e18, suffix = " as";
        else suffix = " s";

        if (k >= 1e9) os << setprecision(4);
        else if (k >= 1e3) os << setprecision(0) << fixed;
        else if (k >= 1e2) os << setprecision(1) << fixed;
        else if (k >= 1e1) os << setprecision(2) << fixed;
        else if (k >= 1e-3) os << setprecision(3) << fixed;
        else os << scientific;
        os << k << suffix;
        return os.str();
    }

    std::vector<measure> benchmark(benchmarkable run, sysclock::duration minimum_time) noexcept
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

            cerr << m << "\r";
            ms.push_back(m);

            auto thresh_gross = m.time - threshold;
            if (thresh_gross > seconds(0)) over_thresh += thresh_gross;

            auto time_taken = end_time - start_time;
            if (time_taken >= minimum_time &&
                over_thresh > over_thresh_threshold &&
                count >= 4)
            {
                cerr << endl;
                if (time_taken > minimum_time * 1.25)
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
                    cerr << endl;
                    return ms;
                }
            }
        }
    }

    measure median(std::vector<measure> &sample) noexcept
    {
        size_t n = sample.size();
        size_t m = n / 2;
        auto it_middle = sample.begin() + m;
        const auto measureLess = [](const measure &a, const measure &b) {
            return a.time * b.iters < b.time * a.iters;
        };
        std::nth_element(sample.begin(), sample.begin() + m, sample.end(), measureLess);
        return *it_middle;
    }

} // namespace criterion
