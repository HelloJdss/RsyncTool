// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_ERRORCODE_PROTOCOL_H_
#define FLATBUFFERS_GENERATED_ERRORCODE_PROTOCOL_H_

#include "flatbuffers/flatbuffers.h"

namespace Protocol {

struct ErrorCode;

enum Err {
  Err_UNKNOWN = 0,
  Err_NO_SUCH_FILE = 1,
  Err_NO_SUCH_DIR = 2,
  Err_MIN = Err_UNKNOWN,
  Err_MAX = Err_NO_SUCH_DIR
};

inline const Err (&EnumValuesErr())[3] {
  static const Err values[] = {
    Err_UNKNOWN,
    Err_NO_SUCH_FILE,
    Err_NO_SUCH_DIR
  };
  return values;
}

inline const char * const *EnumNamesErr() {
  static const char * const names[] = {
    "UNKNOWN",
    "NO_SUCH_FILE",
    "NO_SUCH_DIR",
    nullptr
  };
  return names;
}

inline const char *EnumNameErr(Err e) {
  const size_t index = static_cast<int>(e);
  return EnumNamesErr()[index];
}

struct ErrorCode FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_CODE = 4,
    VT_TIP = 6
  };
  Err Code() const {
    return static_cast<Err>(GetField<int16_t>(VT_CODE, 0));
  }
  const flatbuffers::String *TIP() const {
    return GetPointer<const flatbuffers::String *>(VT_TIP);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int16_t>(verifier, VT_CODE) &&
           VerifyOffset(verifier, VT_TIP) &&
           verifier.Verify(TIP()) &&
           verifier.EndTable();
  }
};

struct ErrorCodeBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_Code(Err Code) {
    fbb_.AddElement<int16_t>(ErrorCode::VT_CODE, static_cast<int16_t>(Code), 0);
  }
  void add_TIP(flatbuffers::Offset<flatbuffers::String> TIP) {
    fbb_.AddOffset(ErrorCode::VT_TIP, TIP);
  }
  explicit ErrorCodeBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ErrorCodeBuilder &operator=(const ErrorCodeBuilder &);
  flatbuffers::Offset<ErrorCode> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ErrorCode>(end);
    return o;
  }
};

inline flatbuffers::Offset<ErrorCode> CreateErrorCode(
    flatbuffers::FlatBufferBuilder &_fbb,
    Err Code = Err_UNKNOWN,
    flatbuffers::Offset<flatbuffers::String> TIP = 0) {
  ErrorCodeBuilder builder_(_fbb);
  builder_.add_TIP(TIP);
  builder_.add_Code(Code);
  return builder_.Finish();
}

inline flatbuffers::Offset<ErrorCode> CreateErrorCodeDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    Err Code = Err_UNKNOWN,
    const char *TIP = nullptr) {
  return Protocol::CreateErrorCode(
      _fbb,
      Code,
      TIP ? _fbb.CreateString(TIP) : 0);
}

inline const Protocol::ErrorCode *GetErrorCode(const void *buf) {
  return flatbuffers::GetRoot<Protocol::ErrorCode>(buf);
}

inline const Protocol::ErrorCode *GetSizePrefixedErrorCode(const void *buf) {
  return flatbuffers::GetSizePrefixedRoot<Protocol::ErrorCode>(buf);
}

inline bool VerifyErrorCodeBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<Protocol::ErrorCode>(nullptr);
}

inline bool VerifySizePrefixedErrorCodeBuffer(
    flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<Protocol::ErrorCode>(nullptr);
}

inline void FinishErrorCodeBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<Protocol::ErrorCode> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedErrorCodeBuffer(
    flatbuffers::FlatBufferBuilder &fbb,
    flatbuffers::Offset<Protocol::ErrorCode> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace Protocol

#endif  // FLATBUFFERS_GENERATED_ERRORCODE_PROTOCOL_H_
