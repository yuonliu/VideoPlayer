#pragma once
#include <iostream>
#include <memory>
#include <cassert>

#include <ConcurrentQueue.h>
#include <EventLoop.h>
#include <../AVCore/AVCore.h>


using PacketPtr = std::shared_ptr<Packet>;
using FramePtr = std::shared_ptr<Frame>;
using PacketQueue = ConcurrentQueue<PacketPtr, std::queue<PacketPtr>>;
using FrameQueue = ConcurrentQueue<FramePtr, std::queue<FramePtr>>;

using PCMQueue = ConcurrentQueue<std::vector<uint8_t>, std::queue<std::vector<uint8_t>>>;

