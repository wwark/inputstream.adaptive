/*
*      Copyright (C) 2016 - 2019 peak3d
*      http://www.peak3d.de
*
*  This Program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  This Program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*
*  <http://www.gnu.org/licenses/>.
*
*/

#pragma once

#include <stdint.h>
#include <vector>

#include "Ap4Types.h"
#include <kodi/addon-instance/Inputstream.h>
#include <TimingConstants.h>
#include <webm/callback.h>
#include <webm/status.h>

class AP4_ByteStream;
class WebmAP4Reader;


class WebmReader : public webm::Callback
{
public:

  struct CUEPOINT
  {
    uint64_t pts;
    uint64_t duration;
    uint64_t pos_start;
    uint64_t pos_end;
  };

  WebmReader(AP4_ByteStream *stream);
  virtual ~WebmReader();

  void GetCuePoints(std::vector<CUEPOINT> &cuepoints);
  bool Initialize();

  void Reset(bool resetPackets = true);
  bool StartStreaming();
  bool SeekTime(uint64_t timeInTs, bool preceeding);

  bool GetInformation(INPUTSTREAM_INFO &info);
  bool ReadPacket(bool streamInfo = false);

  webm::Status OnElementBegin(const webm::ElementMetadata& metadata, webm::Action* action) override;
  webm::Status OnCuePoint(const webm::ElementMetadata& metadata, const webm::CuePoint& cue_point) override;
  webm::Status OnFrame(const webm::FrameMetadata& metadata, webm::Reader* reader, std::uint64_t* bytes_remaining) override;

  uint64_t GetDts() const { return m_dts; }
  uint64_t GetPts() const { return m_pts; }
  uint64_t GetDuration() const { return m_duration; }
  const AP4_Byte *GetPacketData() const { return nullptr; };
  const AP4_Size GetPacketSize() const { return 0; };

private:
  WebmAP4Reader *m_reader = nullptr;
  bool m_framePresent = false;
  uint64_t m_pts = DVD_NOPTS_VALUE;
  uint64_t m_dts = DVD_NOPTS_VALUE;
  uint64_t m_duration = 0;
  std::vector<CUEPOINT> *m_cuePoints = nullptr;
};