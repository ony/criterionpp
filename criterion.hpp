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

#pragma once

#include <time.h>

#include <iosfwd>
#include <functional>
#include <chrono>
#include <ratio>
#include <vector>

namespace criterion {

    using seconds = std::chrono::seconds;

    using sysclock = std::chrono::system_clock;

    struct cpuclock
    {
        typedef std::chrono::nanoseconds duration;
        typedef duration::rep rep;
        typedef duration::period period;
        typedef std::chrono::time_point<cpuclock, duration> time_point;

        static constexpr bool is_steady = true;

        static time_point from_timespec(const timespec &ts) noexcept
        { return time_point(seconds(ts.tv_sec) + duration(ts.tv_nsec)); }

        static time_point now() noexcept
        {
            struct timespec ts;
            (void) clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
            return from_timespec(ts);
        }
    };

    typedef std::function<void(size_t) noexcept> benchmarkable;

    struct measure {
        sysclock::duration time;
        cpuclock::duration cpu_time;
        size_t iters;

        /// Measure the execution of a benchmark a given number of times
        sysclock::time_point run(benchmarkable run, size_t n) noexcept
        {
            auto start_time = sysclock::now();
            auto start_cpu_time = cpuclock::now();
            run(n);
            auto end_time = sysclock::now();
            auto end_cpu_time = cpuclock::now();

            time = end_time - start_time;
            cpu_time = end_cpu_time - start_cpu_time;
            iters = n;
            return end_time;
        }
    };
    std::ostream &operator<<(std::ostream &, const measure &) noexcept;

    /// Convert duriation to a string. The string will consist of four
    /// decimal places, followed by a short description of the time units.
    std::string human(std::chrono::duration<double> t) noexcept;

    /// Run a single benchmark, and return measurements collected while
    /// executing it.
    std::vector<measure> benchmark(benchmarkable run, sysclock::duration time_limit = seconds(15)) noexcept;

    void enforce(int) noexcept;
} // namespace criterion
