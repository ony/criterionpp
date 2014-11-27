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

#include <tuple>
#include <functional>
#include <vector>
#include <random>

template <typename T>
T sqr(T x) { return x*x; }

struct estimate
{
    double mean;
    double stdev;
    double lbound, ubound;

    estimate() = default;

    estimate(const std::vector<double> &sample)
    {
        const size_t n = sample.size();

        mean = 0;
        for (const auto &value : sample) mean += value;
        mean /= n;

        stdev = 0;
        for (const auto &value : sample) stdev += sqr(value - mean);
        stdev = sqrt(stdev / (n - 1));

        // assume normal distribution
        // 68.3% of the population is contained within 1 standard deviations from the mean.
        // 95% of the population is contained within 2 standard deviations from the mean.
        // 99.7% of the population is contained within 3 standard deviations from the mean.
        lbound = mean - 3*stdev;
        ubound = mean + 3*stdev;
    }

    template <typename T>
    estimate(const std::vector<T> &sample, std::function<double(T)> accessor)
    {
        const size_t n = sample.size();

        mean = 0;
        for (const auto &value : sample) mean += value;
        mean /= n;

        stdev = 0;
        for (const auto &value : sample) stdev += sqr(value - mean);
        stdev = sqrt(stdev / (n - 1));

        // assume normal distribution
        // 68.3% of the population is contained within 1 standard deviations from the mean.
        // 95% of the population is contained within 2 standard deviations from the mean.
        // 99.7% of the population is contained within 3 standard deviations from the mean.
        lbound = mean - 3*stdev;
        ubound = mean + 3*stdev;
    }
};

template <typename T>
using estimator = std::function<double(std::vector<T> &)>;

template <size_t M, typename T>
std::array<estimate, M> bootstrap(std::vector<T> &sample, size_t resamples, std::array<estimator<T>, M> estimators)
{
    const size_t n = sample.size();


    // re-sample and estimate our sample

    std::default_random_engine rng;
    std::uniform_int_distribution<size_t> index_dist(0, n - 1);

    std::array<std::vector<double>, M> esamples;
    for (auto &esample : esamples) esample.resize(resamples);

    for (size_t i = 0; i < resamples; ++i)
    {
        std::vector<T> bsample;
        bsample.reserve(n);
        for (size_t j = 0; j < n; ++j)
        {
            bsample.emplace_back(sample[index_dist(rng)]);
        }
        for (size_t k = 0; k < M; ++k) esamples[k][i] = estimators[k](bsample);
    }

    // lets calculate mean and stdev
    std::array<estimate, M> estimates;
    for (size_t k = 0; k < M; ++k) estimates[k] = {esamples[k]};

    return estimates;
}
