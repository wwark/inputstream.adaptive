#ifndef MEDIA_CDM_CDM_ADAPTER_H_
#define MEDIA_CDM_CDM_ADAPTER_H_

#include <string>
#include <vector>
#include <inttypes.h>
#include <memory>
#include <mutex>

#include "../../base/native_library.h"
#include "../../base/compiler_specific.h"
#include "../../base/macros.h"
#include "api/content_decryption_module.h"
#include "../base/cdm_config.h"


// cdm::VideoFrame and cdm::VideoFrame_2 common implementation.
class VideoFrameImpl : public cdm::VideoFrame, public cdm::VideoFrame_2
{
// public:
//  VideoFrameImpl();
//  ~VideoFrameImpl() override { if (frame_buffer_) frame_buffer_->Destroy(); }
//
//  void SetFormat(cdm::VideoFormat format) final { format_ = format; }
//  cdm::VideoFormat Format() const final { return format_; }
//  void SetSize(cdm::Size size) final { size_ = size; }
//  cdm::Size Size() const final { return size_; }
//  void SetFrameBuffer(cdm::Buffer* frame_buffer) final { frame_buffer_ = frame_buffer; }
//  cdm::Buffer* FrameBuffer() final { return frame_buffer_; }
//  void SetPlaneOffset(cdm::VideoPlane plane, uint32_t offset) final { plane_offsets_[plane] = offset; }
//  uint32_t PlaneOffset(cdm::VideoPlane plane) final { return plane_offsets_[plane]; }
//  void SetStride(cdm::VideoPlane plane, uint32_t stride) final { strides_[plane] = stride; }
//  uint32_t Stride(cdm::VideoPlane plane) final { return strides_[plane]; }
//  void SetTimestamp(int64_t timestamp) final { timestamp_ = timestamp; }
//  int64_t Timestamp() const final { return timestamp_; }
//
//  // cdm::VideoFrame_2 specific implementation.
//  void SetColorSpace(cdm::ColorSpace color_space) final { color_space_ = color_space; }
//
// private:
//  // See ISO 23001-8:2016, section 7. Value 2 means "Unspecified".
//  cdm::ColorSpace color_space_ =  {2, 2, 2, cdm::ColorRange::kInvalid};
//  cdm::VideoFormat format_ = cdm::kUnknownVideoFormat;
//  cdm::Size size_;
//  cdm::Buffer* frame_buffer_ = nullptr;
//  uint32_t plane_offsets_[cdm::kMaxPlanes]{};
//  uint32_t strides_[cdm::kMaxPlanes]{};
//  int64_t timestamp_ = 0;
//
// private:
//  DISALLOW_COPY_AND_ASSIGN(VideoFrameImpl);
};

namespace media {

uint64_t gtc();

class CdmAdapterClient
{
public:
  enum CDMADPMSG
  {
    kError,
    kSessionMessage,
    kSessionExpired,
    kSessionKeysChange,
    kSessionClosed,
    kLegacySessionError
  };
  virtual void OnCDMMessage(const char* session, uint32_t session_size, CDMADPMSG msg, const uint8_t *data, size_t data_size, uint32_t status) = 0;
  virtual void CDMLog(const char *msg) = 0;
  virtual cdm::Buffer *AllocateBuffer(size_t sz) = 0;
};

class CdmAdapter : public std::enable_shared_from_this<CdmAdapter>
  , public cdm::Host_9
  , public cdm::Host_10
  , public cdm::Host_11
{
 public:
	CdmAdapter(const std::string& key_system,
    const std::string& cdm_path,
    const std::string& base_path,
    const CdmConfig& cdm_config,
    CdmAdapterClient *client);

	void RemoveClient();

	void SetServerCertificate(uint32_t promise_id,
		const uint8_t* server_certificate_data,
		uint32_t server_certificate_data_size);

	void CreateSessionAndGenerateRequest(uint32_t promise_id,
		cdm::SessionType session_type,
		cdm::InitDataType init_data_type,
		const uint8_t* init_data,
		uint32_t init_data_size);

	void LoadSession(uint32_t promise_id,
		cdm::SessionType session_type,
		const char* session_id,
		uint32_t session_id_size);

	void UpdateSession(uint32_t promise_id,
		const char* session_id,
		uint32_t session_id_size,
		const uint8_t* response,
		uint32_t response_size);

	void CloseSession(uint32_t promise_id,
		const char* session_id,
		uint32_t session_id_size);

        void RemoveSession(uint32_t promise_id,
		const char* session_id,
		uint32_t session_id_size);

	void TimerExpired(void* context);

	cdm::Status Decrypt(const cdm::InputBuffer_2& encrypted_buffer,
		cdm::DecryptedBlock* decrypted_buffer);

  cdm::Status InitializeAudioDecoder(
		const cdm::AudioDecoderConfig_2& audio_decoder_config);

	cdm::Status InitializeVideoDecoder(
		const cdm::VideoDecoderConfig_3& video_decoder_config);

	void DeinitializeDecoder(cdm::StreamType decoder_type);

	void ResetDecoder(cdm::StreamType decoder_type);

	cdm::Status DecryptAndDecodeFrame(const cdm::InputBuffer_2& encrypted_buffer,
		VideoFrameImpl* video_frame);

	cdm::Status DecryptAndDecodeSamples(const cdm::InputBuffer_2& encrypted_buffer,
		cdm::AudioFrames* audio_frames);

	void OnPlatformChallengeResponse(
		const cdm::PlatformChallengeResponse& response);

	void OnQueryOutputProtectionStatus(cdm::QueryResult result,
		uint32_t link_mask,
		uint32_t output_protection_mask);

  // cdm::Host implementation.

	cdm::Buffer* Allocate(uint32_t capacity) override;

	void SetTimer(int64_t delay_ms, void* context) override;

	cdm::Time GetCurrentWallTime() override;

  void OnResolveKeyStatusPromise(uint32_t promise_id,
    cdm::KeyStatus key_status) override;

	void OnResolveNewSessionPromise(uint32_t promise_id,
    const char* session_id,
    uint32_t session_id_size) override;

	void OnResolvePromise(uint32_t promise_id) override;

	void OnRejectPromise(uint32_t promise_id,
    cdm::Exception error,
    uint32_t system_code,
    const char* error_message,
    uint32_t error_message_size)override;

  void OnSessionMessage(const char* session_id,
    uint32_t session_id_size,
    cdm::MessageType message_type,
    const char* message,
    uint32_t message_size) override;

	void OnSessionKeysChange(const char* session_id,
    uint32_t session_id_size,
    bool has_additional_usable_key,
    const cdm::KeyInformation* keys_info,
    uint32_t keys_info_count) override;

	void OnExpirationChange(const char* session_id,
    uint32_t session_id_size,
    cdm::Time new_expiry_time) override;

	void OnSessionClosed(const char* session_id,
    uint32_t session_id_size) override;

	void SendPlatformChallenge(const char* service_id,
    uint32_t service_id_size,
    const char* challenge,
    uint32_t challenge_size) override;

	void EnableOutputProtection(uint32_t desired_protection_mask) override;

	void QueryOutputProtectionStatus() override;

	void OnDeferredInitializationDone(cdm::StreamType stream_type,
    cdm::Status decoder_status) override;

	cdm::FileIO* CreateFileIO(cdm::FileIOClient* client) override;

	void RequestStorageId(uint32_t version) override;

    // Host_10 specific
  void OnInitialized(bool success) override;

  // Host_11 specific
  cdm::CdmProxy* RequestCdmProxy(cdm::CdmProxyClient* client) override;


public: //Misc
	virtual ~CdmAdapter();
	bool valid(){ return library_ != 0; };
private:
  virtual void Initialize(const std::string& cdm_path);
  void SendClientMessage(const char* session, uint32_t session_size, CdmAdapterClient::CDMADPMSG msg, const uint8_t *data, size_t data_size, uint32_t status);

  // Keep a reference to the CDM.
  base::NativeLibrary library_;

  std::string cdm_base_path_;
  CdmAdapterClient *client_;
  std::mutex client_mutex_, decrypt_mutex_;

  std::string key_system_;
  CdmConfig cdm_config_;

  cdm::MessageType message_type_;
  cdm::Buffer *active_buffer_;

  cdm::ContentDecryptionModule_9 *cdm9_;
  cdm::ContentDecryptionModule_10 *cdm10_;
  cdm::ContentDecryptionModule_11 *cdm11_;

  DISALLOW_COPY_AND_ASSIGN(CdmAdapter);
};

class CdmFileIoImpl : NON_EXPORTED_BASE(public cdm::FileIO)
{
public:
  CdmFileIoImpl(std::string base_path, cdm::FileIOClient* client);

  virtual void Open(const char* file_name, uint32_t file_name_size) override;
  virtual void Read() override;
  virtual void Write(const uint8_t* data, uint32_t data_size) override;
  virtual void Close() override;

private:
  std::string base_path_;
  cdm::FileIOClient* client_;
  FILE *file_descriptor_;
  uint8_t *data_buffer_;
  bool opened_;
};


}  // namespace media

#endif  // MEDIA_CDM_CDM_ADAPTER_H_
