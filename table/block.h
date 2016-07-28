// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef STORAGE_LEVELDB_TABLE_BLOCK_H_
#define STORAGE_LEVELDB_TABLE_BLOCK_H_

#include <stddef.h>
#include <stdint.h>
#include "leveldb/iterator.h"

namespace leveldb {

struct BlockContents;
class Comparator;

// block的布局如下：
//
//   ------------------------------low address ---------------->data_
//   |      record     |                                   |
//   -------------------                                   |
//   |      record     |                                   |
//   -------------------                                   |
//   |      ......     |                                   |
//   -------------------                                   |
//   |      record     |                                   |
//   --------------------------> data_ + restart_offset_   |
//   |    restart[0]   |      |                           size_
//   -------------------      |                            |
//   |    restart[1]   |      |                            |
//   -------------------  n * 32 bit                       |
//   |      ......     |      |                            |
//   -------------------      |                            |
//   |  restart[n-1]   |      |                            |
//   ---------------------------                           |
//   | num_restarts(n) |    32 bit                         |
//   ------------------------------high address------------------
//
//   restart_offset_ = size_ - (n + 1) * sizeof(uint32_t)
//
// record布局如下：
//   ---------------------------------------------------------------------------------
//   |    VarInt    |     VarInt     |   VarInt    |  unshared_bytes   | value_bytes |
//   --------------------------------------------------------------------------------
//   | shared_bytes | unshared_bytes | value_bytes | unshared_key_data | value_data  |
//   ---------------------------------------------------------------------------------
//
//   所谓shared_key_data,是因为sstable里的k/v是按照key有序排放的，所以采用了前缀压缩
//   根据前一个record的key结合shared_bytes就可以构造出当前record的key了
//   前缀压缩并不是连续的，可能是每几个一组可以符合压缩策略，所以需要restart记录的前缀压缩的重启点


class Block {
 public:
  // Initialize the block with the specified contents.
  explicit Block(const BlockContents& contents);

  ~Block();

  size_t size() const { return size_; }
  Iterator* NewIterator(const Comparator* comparator);

 private:
  uint32_t NumRestarts() const;

  const char* data_;
  size_t size_;
  uint32_t restart_offset_;     // Offset in data_ of restart array
  bool owned_;                  // Block owns data_[]

  // No copying allowed
  Block(const Block&);
  void operator=(const Block&);

  class Iter;
};

}  // namespace leveldb

#endif  // STORAGE_LEVELDB_TABLE_BLOCK_H_
