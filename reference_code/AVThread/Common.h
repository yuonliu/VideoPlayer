#pragma once
#include <cassert>
#include <iostream>
#include <memory>

#include "AVCore/AVCore.h"
#include "AVThread/ThreadEventLoop.h"
#include "Comm/MessageBus.hpp"
#include "Common/ConcurrentQueue.h"
// #include "Common/DelayQueue.h"

namespace AV {

using PacketPtr = std::shared_ptr<AV::Packet>;
using FramePtr = std::shared_ptr<AV::Frame>;
using PacketQueue = ConcurrentQueue<PacketPtr, std::queue<PacketPtr>>;
using FrameQueue = ConcurrentQueue<FramePtr, std::queue<FramePtr>>;
using PCMQueue =
    ConcurrentQueue<std::vector<uint8_t>, std::queue<std::vector<uint8_t>>>;
class MediaPlayer;

}