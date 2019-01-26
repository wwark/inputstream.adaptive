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
#include "Ap4Types.h"
#include <kodi/addon-instance/Inputstream.h>
#include <webm/callback.h>
#include <webm/status.h>

class AP4_ByteStream;
class WebmAP4Reader;


class WebmReader : public webm::Callback
{
public:
  WebmReader(AP4_ByteStream *stream);
  virtual ~WebmReader();

  bool Initialize();

  void Reset(bool resetPackets = true);
  bool StartStreaming(AP4_UI32 typeMask);
  bool SeekTime(uint64_t timeInTs, bool preceeding);

  bool GetInformation(INPUTSTREAM_INFO &info);
  bool ReadPacket(bool streamInfo = false);

  webm::Status OnFrame(const webm::FrameMetadata& metadata, webm::Reader* reader, std::uint64_t* bytes_remaining) override;

  //uint64_t GetDts() const { return m_pkt.dts == PTS_UNSET ? PTS_UNSET : m_pkt.dts; }
  //uint64_t GetPts() const { return m_pkt.pts == PTS_UNSET ? PTS_UNSET : m_pkt.pts; }
  //uint64_t GetDuration() const { return m_pkt.duration; }
  //const AP4_Byte *GetPacketData() const { return m_pkt.data; };
  //const AP4_Size GetPacketSize() const { return m_pkt.size; };

private:
  WebmAP4Reader *m_reader = nullptr;
  bool m_framePresent = false;
};