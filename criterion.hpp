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
#include <cassert>

#include "statistiscs.hpp"

namespace criterion {

    using seconds = std::chrono::seconds;

    using sysclock = std::chrono::system_clock;

    // additional duration types
    typedef std::chrono::duration<std::chrono::nanoseconds::rep, std::pico> picoseconds;
    typedef std::chrono::duration<std::chrono::nanoseconds::rep, std::femto> femtoseconds;
    typedef std::chrono::duration<std::chrono::nanoseconds::rep, std::atto> attoseconds;

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
        size_t iters = 0;

        std::chrono::duration<double> iteration_time() const
        { return std::chrono::duration<double>(time) / iters; }

        std::chrono::duration<double> iteration_cpu_time() const
        { return std::chrono::duration<double>(cpu_time) / iters; }


        /// Measure the execution of a benchmark a given number of times.
        /// Note that result is adjusted according "zero line" which
        ///      corresponds to cycle with a single statement of storing
        ///      iteration to volatile stack variable.
        sysclock::time_point run(benchmarkable run, size_t n) noexcept
        {
            // align to CPU tick (hint from https://github.com/pernatiy/C-Benchmark)
            auto run_time = cpuclock::now();
            auto start_cpu_time = cpuclock::now();
            while (run_time == start_cpu_time) start_cpu_time = cpuclock::now();

            auto start_time = sysclock::now();
            run(n);
            auto end_cpu_time = cpuclock::now();
            auto end_time = sysclock::now();

            assert( start_time <= end_time );
            assert( start_cpu_time <= end_cpu_time );

            time = end_time - start_time;
            cpu_time = end_cpu_time - start_cpu_time;
            if (zero_line.iters > 0) // have zero "line"?
            {
                // adjust according to empty benchmark
                auto adjustment = zero_line.cpu_time * n / zero_line.iters;
                time -= adjustment;
                cpu_time -= adjustment;
            }
            iters = n;
            return end_time;
        }

        static measure zero_line;
    };
    std::ostream &operator<<(std::ostream &, const measure &) noexcept;

    /// Convert duriation to a string. The string will consist of four
    /// decimal places, followed by a short description of the time units.
    std::string human(std::chrono::duration<double> t) noexcept;

    /// Run a single benchmark, and return measurements collected while
    /// executing it.
    std::vector<measure> benchmark(benchmarkable run, sysclock::duration minimum_time = seconds(5)) noexcept;

    /// Dummy function to force calculation of int expression
    template <typename T>
    inline void enforce(T x) noexcept
    { asm("" : /* no out */ : "r" (x)); } // tell compiler optimizations to back off

    /// Basic analysis
    measure median(std::vector<measure> &sample) noexcept;
    inline measure median(std::vector<measure> &&sample) noexcept
    { return median(sample); }

    measure min_cpu(std::vector<measure> &sample) noexcept;
    inline measure min_cpu(std::vector<measure> &&sample) noexcept
    { return min_cpu(sample); }

    /// A bit more complicated analysis
    struct analysis
    {
        struct estimate {
            std::chrono::duration<double> value, lbound, ubound;
            static constexpr float ci = 0.997; // fixed
        } mean, stdev, median;

        analysis(std::vector<measure> &sample) noexcept;
        analysis(std::vector<measure> &&sample) noexcept
            : analysis(sample)
        {}
    };
    std::ostream &operator<<(std::ostream &, const analysis &) noexcept;
} // namespace criterion
