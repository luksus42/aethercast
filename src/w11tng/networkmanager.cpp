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

#include <algorithm>

#include <mcs/logger.h>

#include "networkmanager.h"

namespace w11tng {

mcs::NetworkManager::Ptr NetworkManager::Create() {
    return std::shared_ptr<NetworkManager>(new NetworkManager())->FinalizeConstruction();
}

std::shared_ptr<NetworkManager> NetworkManager::FinalizeConstruction() {
    auto sp = shared_from_this();

    p2p_device_ = P2PDeviceStub::Create(sp);

    return sp;
}

NetworkManager::NetworkManager() {
}

NetworkManager::~NetworkManager() {
}

void NetworkManager::OnSystemReady() {
}

void NetworkManager::OnSystemKilled() {
}

void NetworkManager::SetDelegate(mcs::NetworkManager::Delegate *delegate) {
    delegate_ = delegate;
}

bool NetworkManager::Setup() {
    return false;
}

void NetworkManager::Scan(const std::chrono::seconds &timeout) {
    p2p_device_->Find(timeout);
}

bool NetworkManager::Connect(const mcs::NetworkDevice::Ptr &device) {
   return false;
}

bool NetworkManager::Disconnect(const mcs::NetworkDevice::Ptr &device) {
    // Supplicant doesn't take any arguments yet when disconnected. It will
    // either stop the whole group we're operating or disconnects us from
    // a group
    return p2p_device_->Disconnect();
}

void NetworkManager::SetWfdSubElements(const std::list<std::string> &elements) {
}

std::vector<mcs::NetworkDevice::Ptr> NetworkManager::Devices() const {
    std::vector<mcs::NetworkDevice::Ptr> values;
    std::transform(devices_.begin(), devices_.end(),
                   std::back_inserter(values),
                   [=](const std::pair<std::string,w11tng::NetworkDevice::Ptr> &value) {
        return value.second;
    });
    return values;
}

mcs::IpV4Address NetworkManager::LocalAddress() const {
    return mcs::IpV4Address();
}

bool NetworkManager::Running() const {
    return p2p_device_->Connected();
}

bool NetworkManager::Scanning() const {
    return p2p_device_->Scanning();
}

void NetworkManager::OnChanged() {
    if (delegate_)
        delegate_->OnChanged();
}

void NetworkManager::OnDeviceFound(const std::string &path) {
    if (devices_.find(path) != devices_.end())
        return;

    auto device = NetworkDevice::Create(path);
    device->SetDelegate(shared_from_this());

    devices_[path] = device;

    if (delegate_)
        delegate_->OnDeviceFound(device);
}

void NetworkManager::OnDeviceLost(const std::string &path) {
    if (devices_.find(path) == devices_.end())
        return;

    auto device = devices_[path];
    devices_.erase(path);

    if (delegate_)
        delegate_->OnDeviceLost(device);
}

void NetworkManager::OnDeviceChanged(const NetworkDevice::Ptr &device) {
    if (delegate_)
        delegate_->OnDeviceChanged(device);
}

} // namespace w11tng
