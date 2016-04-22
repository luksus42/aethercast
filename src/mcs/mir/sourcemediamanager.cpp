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

#include "mcs/logger.h"

#include "mcs/common/threadedexecutor.h"
#include "mcs/common/threadedexecutorfactory.h"

#include "mcs/network/udpstream.h"

#include "mcs/report/reportfactory.h"

#include "mcs/video/videoformat.h"
#include "mcs/video/displayoutput.h"

#include "mcs/streaming/mpegtspacketizer.h"
#include "mcs/streaming/rtpsender.h"

#include "mcs/mir/sourcemediamanager.h"

#include "mcs/android/h264encoder.h"

namespace mcs {
namespace mir {

SourceMediaManager::SourceMediaManager(const std::string &remote_address,
                                       const mcs::common::ExecutorFactory::Ptr &executor_factory,
                                       const mcs::video::BufferProducer::Ptr &producer,
                                       const mcs::video::BaseEncoder::Ptr &encoder,
                                       const mcs::network::Stream::Ptr &output_stream,
                                       const mcs::report::ReportFactory::Ptr &report_factory) :
    state_(State::Stopped),
    remote_address_(remote_address),
    producer_(producer),
    encoder_(encoder),
    output_stream_(output_stream),
    report_factory_(report_factory),
    pipeline_(executor_factory, 4) {
}

SourceMediaManager::~SourceMediaManager() {
    if (state_ != State::Stopped)
        pipeline_.Stop();
}

bool SourceMediaManager::Configure() {
    auto rr = mcs::video::ExtractRateAndResolution(format_);

    if (!output_stream_->Connect(remote_address_, sink_port1_))
        return false;

    MCS_DEBUG("dimensions: %dx%d@%d", rr.width, rr.height, rr.framerate);

    video::DisplayOutput output{video::DisplayOutput::Mode::kExtend, rr.width, rr.height, rr.framerate};

    if (!producer_->Setup(output)) {
        MCS_ERROR("Failed to setup buffer producer");
        return false;
    }

    int profile = 0, level = 0, constraint = 0;
    mcs::video::ExtractProfileLevel(format_, &profile, &level, &constraint);

    auto config = encoder_->DefaultConfiguration();
    config.width = rr.width;
    config.height = rr.height;
    config.framerate = rr.framerate;
    config.profile_idc = profile;
    config.level_idc = level;
    config.constraint_set = constraint;

    if (!encoder_->Configure(config)) {
        MCS_ERROR("Failed to configure encoder");
        return false;
    }

    renderer_ = std::make_shared<mcs::mir::StreamRenderer>(
                producer_, encoder_, report_factory_->CreateRendererReport());

    auto rtp_sender = std::make_shared<mcs::streaming::RTPSender>(
                output_stream_, report_factory_->CreateSenderReport());
    rtp_sender->SetDelegate(shared_from_this());

    const auto mpegts_packetizer = mcs::streaming::MPEGTSPacketizer::Create(
                report_factory_->CreatePacketizerReport());

    sender_ = std::make_shared<mcs::streaming::MediaSender>(
                mpegts_packetizer,
                rtp_sender,
                config);

    encoder_->SetDelegate(sender_);

    pipeline_.Add(encoder_);
    pipeline_.Add(renderer_);
    pipeline_.Add(rtp_sender);
    pipeline_.Add(sender_);

    return true;
}

void SourceMediaManager::OnTransportNetworkError() {
    if (auto sp = delegate_.lock())
        sp->OnSourceNetworkError();
}

void SourceMediaManager::Play() {
    if (!IsPaused())
        return;

    MCS_DEBUG("");

    pipeline_.Start();

    state_ = State::Playing;
}

void SourceMediaManager::Pause() {
    if (IsPaused())
        return;

    MCS_DEBUG("");

    pipeline_.Stop();

    state_ = State::Paused;
}

void SourceMediaManager::Teardown() {
    if (state_ == State::Stopped)
        return;

    MCS_DEBUG("");

    pipeline_.Stop();

    state_ = State::Stopped;
}

bool SourceMediaManager::IsPaused() const {
    return state_ == State::Paused ||
           state_ == State::Stopped;
}

void SourceMediaManager::SendIDRPicture() {
    if (!encoder_)
        return;

    encoder_->SendIDRFrame();
}

int SourceMediaManager::GetLocalRtpPort() const {
    return sender_->LocalRTPPort();
}

} // namespace mir
} // namespace mcs
