#include <cinttypes>
#include <vector>
extern "C" {
#include "sonic_lite.h"
}

// 只能处理单声道，固定采样率
class Atempo {
 public:
  Atempo() {
    sonicInit();
    sonicSetSpeed(1.0);
    sonicSetVolume(1.0);
  }
  ~Atempo() = default;
  void SetSpeed(double speed) { sonicSetSpeed(speed); }
  void SetVolume(double volume) { sonicSetVolume(volume); }
  std::vector<uint16_t> ProcessPCMData(std::vector<uint16_t>& inPCM) {
    std::vector<uint16_t> outPCM(SONIC_INPUT_SAMPLES);

    if (inPCM.size() == 0) {
      sonicFlushStream();
    } else {
      sonicWriteShortToStream((short*)&inPCM[0], inPCM.size());
    }

    int total_samples_num = 0;
    int samples_num;
    do {
      if (total_samples_num >= outPCM.size()) {
        outPCM.resize(total_samples_num * 2);
      }
      samples_num = sonicReadShortFromStream(
          (short*)&(outPCM[total_samples_num]), SONIC_INPUT_SAMPLES);
      total_samples_num += samples_num;
    } while (samples_num > 0);
    outPCM.resize(total_samples_num);
    return outPCM;
  }
};