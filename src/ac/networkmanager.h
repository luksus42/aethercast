/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef NETWORKMANAGER_H_
#define NETWORKMANAGER_H_

#include <boost/noncopyable.hpp>

#include <vector>
#include <list>
#include <chrono>

#include "ip_v4_address.h"
#include "networkdevice.h"
#include "non_copyable.h"

namespace ac {
class NetworkManager : private ac::NonCopyable {
public:
    typedef std::shared_ptr<NetworkManager> Ptr;

    class Delegate : private ac::NonCopyable {
    public:
        virtual void OnDeviceFound(const NetworkDevice::Ptr &peer) = 0;
        virtual void OnDeviceLost(const NetworkDevice::Ptr &peer) = 0;
        virtual void OnDeviceStateChanged(const NetworkDevice::Ptr &peer) = 0;
        virtual void OnDeviceChanged(const NetworkDevice::Ptr &peer) = 0;
        virtual void OnChanged() = 0;
        virtual void OnReadyChanged() = 0;

    protected:
        Delegate() = default;
    };

    enum class Capability {
        kSource,
        kSink
    };

    static std::string CapabilityToStr(Capability capability);

    virtual void SetDelegate(Delegate * delegate) = 0;

    virtual void SetCapabilities(const std::vector<Capability> &capabilities) = 0;
    virtual std::vector<Capability> Capabilities() const = 0;

    virtual bool Setup() = 0;
    virtual void Release() = 0;

    virtual void Scan(const std::chrono::seconds &timeout) = 0;
    virtual bool Connect(const NetworkDevice::Ptr &device) = 0;
    virtual bool Disconnect(const NetworkDevice::Ptr &device) = 0;

    virtual std::vector<NetworkDevice::Ptr> Devices() const = 0;
    virtual IpV4Address LocalAddress() const = 0;
    virtual bool Running() const = 0;
    virtual bool Scanning() const = 0;
    virtual bool Ready() const = 0;

protected:
    NetworkManager() = default;
};
} // namespace ac
#endif
