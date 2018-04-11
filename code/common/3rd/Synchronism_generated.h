// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_SYNCHRONISM_PROTOCOL_H_
#define FLATBUFFERS_GENERATED_SYNCHRONISM_PROTOCOL_H_

#include "flatbuffers/flatbuffers.h"

namespace Protocol {

struct ChunkInfo;

struct SyncFile;

struct FileDigest;

struct RebuildInfo;

struct RebuildChunk;

struct ChunkInfo FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_OFFSET = 4,
    VT_LENGTH = 6,
    VT_CHECKSUM = 8,
    VT_MD5 = 10
  };
  uint64_t Offset() const {
    return GetField<uint64_t>(VT_OFFSET, 0);
  }
  uint32_t Length() const {
    return GetField<uint32_t>(VT_LENGTH, 0);
  }
  uint32_t Checksum() const {
    return GetField<uint32_t>(VT_CHECKSUM, 0);
  }
  const flatbuffers::String *Md5() const {
    return GetPointer<const flatbuffers::String *>(VT_MD5);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint64_t>(verifier, VT_OFFSET) &&
           VerifyField<uint32_t>(verifier, VT_LENGTH) &&
           VerifyField<uint32_t>(verifier, VT_CHECKSUM) &&
           VerifyOffset(verifier, VT_MD5) &&
           verifier.Verify(Md5()) &&
           verifier.EndTable();
  }
};

struct ChunkInfoBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_Offset(uint64_t Offset) {
    fbb_.AddElement<uint64_t>(ChunkInfo::VT_OFFSET, Offset, 0);
  }
  void add_Length(uint32_t Length) {
    fbb_.AddElement<uint32_t>(ChunkInfo::VT_LENGTH, Length, 0);
  }
  void add_Checksum(uint32_t Checksum) {
    fbb_.AddElement<uint32_t>(ChunkInfo::VT_CHECKSUM, Checksum, 0);
  }
  void add_Md5(flatbuffers::Offset<flatbuffers::String> Md5) {
    fbb_.AddOffset(ChunkInfo::VT_MD5, Md5);
  }
  explicit ChunkInfoBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ChunkInfoBuilder &operator=(const ChunkInfoBuilder &);
  flatbuffers::Offset<ChunkInfo> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<ChunkInfo>(end);
    return o;
  }
};

inline flatbuffers::Offset<ChunkInfo> CreateChunkInfo(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t Offset = 0,
    uint32_t Length = 0,
    uint32_t Checksum = 0,
    flatbuffers::Offset<flatbuffers::String> Md5 = 0) {
  ChunkInfoBuilder builder_(_fbb);
  builder_.add_Offset(Offset);
  builder_.add_Md5(Md5);
  builder_.add_Checksum(Checksum);
  builder_.add_Length(Length);
  return builder_.Finish();
}

inline flatbuffers::Offset<ChunkInfo> CreateChunkInfoDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t Offset = 0,
    uint32_t Length = 0,
    uint32_t Checksum = 0,
    const char *Md5 = nullptr) {
  return Protocol::CreateChunkInfo(
      _fbb,
      Offset,
      Length,
      Checksum,
      Md5 ? _fbb.CreateString(Md5) : 0);
}

struct SyncFile FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_SRCPATH = 4,
    VT_DESPATH = 6
  };
  const flatbuffers::String *SrcPath() const {
    return GetPointer<const flatbuffers::String *>(VT_SRCPATH);
  }
  const flatbuffers::String *DesPath() const {
    return GetPointer<const flatbuffers::String *>(VT_DESPATH);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_SRCPATH) &&
           verifier.Verify(SrcPath()) &&
           VerifyOffset(verifier, VT_DESPATH) &&
           verifier.Verify(DesPath()) &&
           verifier.EndTable();
  }
};

struct SyncFileBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_SrcPath(flatbuffers::Offset<flatbuffers::String> SrcPath) {
    fbb_.AddOffset(SyncFile::VT_SRCPATH, SrcPath);
  }
  void add_DesPath(flatbuffers::Offset<flatbuffers::String> DesPath) {
    fbb_.AddOffset(SyncFile::VT_DESPATH, DesPath);
  }
  explicit SyncFileBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  SyncFileBuilder &operator=(const SyncFileBuilder &);
  flatbuffers::Offset<SyncFile> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<SyncFile>(end);
    return o;
  }
};

inline flatbuffers::Offset<SyncFile> CreateSyncFile(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> SrcPath = 0,
    flatbuffers::Offset<flatbuffers::String> DesPath = 0) {
  SyncFileBuilder builder_(_fbb);
  builder_.add_DesPath(DesPath);
  builder_.add_SrcPath(SrcPath);
  return builder_.Finish();
}

inline flatbuffers::Offset<SyncFile> CreateSyncFileDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *SrcPath = nullptr,
    const char *DesPath = nullptr) {
  return Protocol::CreateSyncFile(
      _fbb,
      SrcPath ? _fbb.CreateString(SrcPath) : 0,
      DesPath ? _fbb.CreateString(DesPath) : 0);
}

struct FileDigest FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_SRCPATH = 4,
    VT_DESPATH = 6,
    VT_SPLITSIZE = 8,
    VT_INFOS = 10
  };
  const flatbuffers::String *SrcPath() const {
    return GetPointer<const flatbuffers::String *>(VT_SRCPATH);
  }
  const flatbuffers::String *DesPath() const {
    return GetPointer<const flatbuffers::String *>(VT_DESPATH);
  }
  uint32_t Splitsize() const {
    return GetField<uint32_t>(VT_SPLITSIZE, 0);
  }
  const flatbuffers::Vector<flatbuffers::Offset<ChunkInfo>> *Infos() const {
    return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<ChunkInfo>> *>(VT_INFOS);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffset(verifier, VT_SRCPATH) &&
           verifier.Verify(SrcPath()) &&
           VerifyOffset(verifier, VT_DESPATH) &&
           verifier.Verify(DesPath()) &&
           VerifyField<uint32_t>(verifier, VT_SPLITSIZE) &&
           VerifyOffset(verifier, VT_INFOS) &&
           verifier.Verify(Infos()) &&
           verifier.VerifyVectorOfTables(Infos()) &&
           verifier.EndTable();
  }
};

struct FileDigestBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_SrcPath(flatbuffers::Offset<flatbuffers::String> SrcPath) {
    fbb_.AddOffset(FileDigest::VT_SRCPATH, SrcPath);
  }
  void add_DesPath(flatbuffers::Offset<flatbuffers::String> DesPath) {
    fbb_.AddOffset(FileDigest::VT_DESPATH, DesPath);
  }
  void add_Splitsize(uint32_t Splitsize) {
    fbb_.AddElement<uint32_t>(FileDigest::VT_SPLITSIZE, Splitsize, 0);
  }
  void add_Infos(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ChunkInfo>>> Infos) {
    fbb_.AddOffset(FileDigest::VT_INFOS, Infos);
  }
  explicit FileDigestBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  FileDigestBuilder &operator=(const FileDigestBuilder &);
  flatbuffers::Offset<FileDigest> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<FileDigest>(end);
    return o;
  }
};

inline flatbuffers::Offset<FileDigest> CreateFileDigest(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> SrcPath = 0,
    flatbuffers::Offset<flatbuffers::String> DesPath = 0,
    uint32_t Splitsize = 0,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<ChunkInfo>>> Infos = 0) {
  FileDigestBuilder builder_(_fbb);
  builder_.add_Infos(Infos);
  builder_.add_Splitsize(Splitsize);
  builder_.add_DesPath(DesPath);
  builder_.add_SrcPath(SrcPath);
  return builder_.Finish();
}

inline flatbuffers::Offset<FileDigest> CreateFileDigestDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    const char *SrcPath = nullptr,
    const char *DesPath = nullptr,
    uint32_t Splitsize = 0,
    const std::vector<flatbuffers::Offset<ChunkInfo>> *Infos = nullptr) {
  return Protocol::CreateFileDigest(
      _fbb,
      SrcPath ? _fbb.CreateString(SrcPath) : 0,
      DesPath ? _fbb.CreateString(DesPath) : 0,
      Splitsize,
      Infos ? _fbb.CreateVector<flatbuffers::Offset<ChunkInfo>>(*Infos) : 0);
}

struct RebuildInfo FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_SIZE = 4
  };
  int64_t Size() const {
    return GetField<int64_t>(VT_SIZE, 0);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int64_t>(verifier, VT_SIZE) &&
           verifier.EndTable();
  }
};

struct RebuildInfoBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_Size(int64_t Size) {
    fbb_.AddElement<int64_t>(RebuildInfo::VT_SIZE, Size, 0);
  }
  explicit RebuildInfoBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  RebuildInfoBuilder &operator=(const RebuildInfoBuilder &);
  flatbuffers::Offset<RebuildInfo> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<RebuildInfo>(end);
    return o;
  }
};

inline flatbuffers::Offset<RebuildInfo> CreateRebuildInfo(
    flatbuffers::FlatBufferBuilder &_fbb,
    int64_t Size = 0) {
  RebuildInfoBuilder builder_(_fbb);
  builder_.add_Size(Size);
  return builder_.Finish();
}

struct RebuildChunk FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_OFFSET = 4,
    VT_LENGTH = 6,
    VT_ISMD5 = 8,
    VT_DATA = 10
  };
  uint64_t Offset() const {
    return GetField<uint64_t>(VT_OFFSET, 0);
  }
  uint32_t Length() const {
    return GetField<uint32_t>(VT_LENGTH, 0);
  }
  bool IsMd5() const {
    return GetField<uint8_t>(VT_ISMD5, 0) != 0;
  }
  const flatbuffers::String *Data() const {
    return GetPointer<const flatbuffers::String *>(VT_DATA);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<uint64_t>(verifier, VT_OFFSET) &&
           VerifyField<uint32_t>(verifier, VT_LENGTH) &&
           VerifyField<uint8_t>(verifier, VT_ISMD5) &&
           VerifyOffset(verifier, VT_DATA) &&
           verifier.Verify(Data()) &&
           verifier.EndTable();
  }
};

struct RebuildChunkBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_Offset(uint64_t Offset) {
    fbb_.AddElement<uint64_t>(RebuildChunk::VT_OFFSET, Offset, 0);
  }
  void add_Length(uint32_t Length) {
    fbb_.AddElement<uint32_t>(RebuildChunk::VT_LENGTH, Length, 0);
  }
  void add_IsMd5(bool IsMd5) {
    fbb_.AddElement<uint8_t>(RebuildChunk::VT_ISMD5, static_cast<uint8_t>(IsMd5), 0);
  }
  void add_Data(flatbuffers::Offset<flatbuffers::String> Data) {
    fbb_.AddOffset(RebuildChunk::VT_DATA, Data);
  }
  explicit RebuildChunkBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  RebuildChunkBuilder &operator=(const RebuildChunkBuilder &);
  flatbuffers::Offset<RebuildChunk> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = flatbuffers::Offset<RebuildChunk>(end);
    return o;
  }
};

inline flatbuffers::Offset<RebuildChunk> CreateRebuildChunk(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t Offset = 0,
    uint32_t Length = 0,
    bool IsMd5 = false,
    flatbuffers::Offset<flatbuffers::String> Data = 0) {
  RebuildChunkBuilder builder_(_fbb);
  builder_.add_Offset(Offset);
  builder_.add_Data(Data);
  builder_.add_Length(Length);
  builder_.add_IsMd5(IsMd5);
  return builder_.Finish();
}

inline flatbuffers::Offset<RebuildChunk> CreateRebuildChunkDirect(
    flatbuffers::FlatBufferBuilder &_fbb,
    uint64_t Offset = 0,
    uint32_t Length = 0,
    bool IsMd5 = false,
    const char *Data = nullptr) {
  return Protocol::CreateRebuildChunk(
      _fbb,
      Offset,
      Length,
      IsMd5,
      Data ? _fbb.CreateString(Data) : 0);
}

}  // namespace Protocol

#endif  // FLATBUFFERS_GENERATED_SYNCHRONISM_PROTOCOL_H_
