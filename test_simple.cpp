//
//  Example of usage criterionpp
//
//  Copyright (C) 2014 Mykola Orliuk <virkony@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify it
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

#include <iostream>

#include "criterion.hpp"

using namespace std;
using namespace criterion;
using std::chrono::nanoseconds;

long fact(long n, long factor = 1) noexcept
{
    if (n == 0) return factor;
    else return fact(n - 1, factor * n);
}

int main()
{
    auto sample = [](size_t n) noexcept {
        for (size_t i = 0; i < n; ++i) enforce(fact(20 + i % 4));
    };

    auto ms = benchmark(sample);
    return 0;
}
