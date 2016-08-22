// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_TABLE_FORMAT_H_
#define STORAGE_LEVELDB_TABLE_FORMAT_H_

#include <string>
#include <stdint.h>
#include "leveldb/slice.h"
#include "leveldb/status.h"
#include "leveldb/table_builder.h"

namespace leveldb {

class Block;
class RandomAccessFile;
struct ReadOptions;

// BlockHandle is a pointer to the extent of a file that stores a data
// block or a meta block.
// BlockHandle标识了一个sst文件内的block的起始位置(offset)和大小(size)
class BlockHandle {
 public:
  BlockHandle();

  // The offset of the block in the file.
  uint64_t offset() const { return offset_; }
  void set_offset(uint64_t offset) { offset_ = offset; }

  // The size of the stored block
  uint64_t size() const { return size_; }
  void set_size(uint64_t size) { size_ = size; }

  void EncodeTo(std::string* dst) const;
  Status DecodeFrom(Slice* input);

  // Maximum encoding length of a BlockHandle
  // 因为是使用的VarInt64编码，所以每个unint64_t需要1-10个byte
  enum { kMaxEncodedLength = 10 + 10 };

 private:
  uint64_t offset_;
  uint64_t size_;
};

// 文件的Footer(页脚)，标识了两个重要的存放索引的block的位置（index block 和 metablock index）
// 一个完整的文件结构如下：
//   -----------------------------------------------
//   |    data block    |                      |
//   --------------------                      |
//   |      ... ...     |                      |
//   --------------------                      |
//   |    data block    |                      |
//   --------------------                      |
//   |    meta block    |                      |
//   --------------------                      |
//   |      ... ...     |                      |
//   --------------------                      |
//   |    meta block    |                   sstable
//   --------------------                      |
//   | meta index block |                      |
//   --------------------                      |
//   | data index block |                      |
//   --------------------------------          |
//   | metaindex_handle |      |               |
//   --------------------      |               |
//   |   index_handle   |      |               |
//   --------------------    Footer            |
//   |     padding      |      |               |
//   --------------------      |               |
//   |  magic number    |      |               |
//   -----------------------------------------------
//
// 其中：
//   index block 存放了所有的data block的索引
//     一条data block索引格式如下：
//             --------------------------------------------------
//             | a key >= the last key in block | offset | size |
//             --------------------------------------------------
//   metablock index 存放了所有的metablock的索引
//   因为两个handle都使用的变长的编码规则,padding用来当20×2个字节没用完时做填充
//
// Footer encapsulates the fixed information stored at the tail
// end of every table file.
class Footer {
 public:
  Footer() { }

  // The block handle for the metaindex block of the table
  const BlockHandle& metaindex_handle() const { return metaindex_handle_; }
  void set_metaindex_handle(const BlockHandle& h) { metaindex_handle_ = h; }

  // The block handle for the index block of the table
  const BlockHandle& index_handle() const {
    return index_handle_;
  }
  void set_index_handle(const BlockHandle& h) {
    index_handle_ = h;
  }

  void EncodeTo(std::string* dst) const;
  Status DecodeFrom(Slice* input);

  // Encoded length of a Footer.  Note that the serialization of a
  // Footer will always occupy exactly this many bytes.  It consists
  // of two block handles and a magic number.
  enum {
    kEncodedLength = 2*BlockHandle::kMaxEncodedLength + 8
  };

 private:
  BlockHandle metaindex_handle_;
  BlockHandle index_handle_;
};

// kTableMagicNumber was picked by running
//    echo http://code.google.com/p/leveldb/ | sha1sum
// and taking the leading 64 bits.
static const uint64_t kTableMagicNumber = 0xdb4775248b80fb57ull;

// 1-byte type + 32-bit crc
static const size_t kBlockTrailerSize = 5;

struct BlockContents {
  Slice data;           // Actual contents of data
  bool cachable;        // True iff data can be cached
  bool heap_allocated;  // True iff caller should delete[] data.data()
};

// Read the block identified by "handle" from "file".  On failure
// return non-OK.  On success fill *result and return OK.
extern Status ReadBlock(RandomAccessFile* file,
                        const ReadOptions& options,
                        const BlockHandle& handle,
                        BlockContents* result);

// Implementation details follow.  Clients should ignore,

inline BlockHandle::BlockHandle()
    : offset_(~static_cast<uint64_t>(0)),
      size_(~static_cast<uint64_t>(0)) {
}

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_FORMAT_H_
