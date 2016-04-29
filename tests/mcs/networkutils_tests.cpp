/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <gtest/gtest.h>

#include <sys/socket.h>
#include <fcntl.h>

#include "mcs/networkutils.h"

TEST(NetworkUtils_PickRandomPort, PicksPortsInRange) {
    for (int n = 0; n < 100; n++) {
        auto port = mcs::NetworkUtils::PickRandomPort();
        EXPECT_LE(mcs::NetworkUtils::kMinUserPort, port);
        EXPECT_GE(mcs::NetworkUtils::kMaxUserPort, port);
    }
}

TEST(NetworkUtils_MakeSocketNonBlocking, SocketFlagsCorrectlySet) {
    const auto sock = ::socket(AF_INET, SOCK_DGRAM, 0);

    EXPECT_EQ(0, mcs::NetworkUtils::MakeSocketNonBlocking(sock));

    const auto flags = ::fcntl(sock, F_GETFL, 0);
    EXPECT_TRUE(flags & O_NONBLOCK);
}