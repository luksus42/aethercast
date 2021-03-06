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

#include "ac/logger.h"

#include "ac/report/logging/packetizerreport.h"

namespace ac {
namespace report {
namespace logging {

void PacketizerReport::PacketizedFrame(const TimestampUs &timestamp) {
    AC_TRACE("timestamp %lld", timestamp);
}

} // namespace logging
} // namespace report
} // namespace ac
