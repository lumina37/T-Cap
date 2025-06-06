﻿#include <mfapi.h>
#include <mfidl.h>

#include "tcap/helper/error.hpp"
#include "tcap/platforms/mf/source.hpp"

#ifndef _TCAP_LIB_HEADER_ONLY
#    include "tcap/platforms/mf/media_type.hpp"
#endif

namespace tcap::mf {

StreamSubType mapGuidToStreamSubType(const GUID& guid) {
    if (guid == MFVideoFormat_I420) return StreamSubType::eI420;
    if (guid == MFVideoFormat_NV12) return StreamSubType::eNV12;
    return StreamSubType::eUnknown;
}

MediaTypeBox::MediaTypeBox(IMFMediaType* pMediaType, GUID subTypeGuid, int width, int height, int fpsNumerator,
                           int fpsDenominator) noexcept
    : pMediaType_(pMediaType),
      subTypeGuid_(subTypeGuid),
      subType_(mapGuidToStreamSubType(subTypeGuid)),
      width_(width),
      height_(height),
      fpsNumerator_(fpsNumerator),
      fpsDenominator_(fpsDenominator),
      approxFps_((float)fpsNumerator / (float)fpsDenominator) {}

MediaTypeBox::MediaTypeBox(MediaTypeBox&& rhs) noexcept
    : pMediaType_(std::exchange(rhs.pMediaType_, nullptr)),
      subTypeGuid_(rhs.subTypeGuid_),
      subType_(rhs.subType_),
      width_(rhs.width_),
      height_(rhs.height_),
      fpsNumerator_(rhs.fpsNumerator_),
      fpsDenominator_(rhs.fpsDenominator_),
      approxFps_(rhs.approxFps_) {}

MediaTypeBox::~MediaTypeBox() noexcept {
    if (pMediaType_ == nullptr) return;
    pMediaType_->Release();
    pMediaType_ = nullptr;
}

std::expected<MediaTypeBox, Error> MediaTypeBox::create(IMFMediaType* pMediaType) noexcept {
    HRESULT hr;

    GUID subTypeGuid;
    hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subTypeGuid);
    if (FAILED(hr)) {
        return std::unexpected{Error{hr, "get subType failed"}};
    }

    UINT32 width, height;
    hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &width, &height);
    if (FAILED(hr)) {
        return std::unexpected{Error{hr, "get size failed"}};
    }

    UINT32 fpsNumerator, fpsDenominator;
    hr = MFGetAttributeRatio(pMediaType, MF_MT_FRAME_RATE, &fpsNumerator, &fpsDenominator);
    if (FAILED(hr)) {
        return std::unexpected{Error{hr, "get fps failed"}};
    }
    if (fpsDenominator == 0) {
        return std::unexpected{Error{hr, "fpsDenominator is 0"}};
    }

    return MediaTypeBox{pMediaType, subTypeGuid, (int)width, (int)height, (int)fpsNumerator, (int)fpsDenominator};
}

}  // namespace tcap::mf
