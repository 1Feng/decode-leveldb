#include <cassert>
#include <string>
#include "leveldb/db.h"


int main() {
  leveldb::DB* db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
  assert(status.ok());
  std::string key("key");
  std::string value("value");
  status = db->Put(leveldb::WriteOptions(), key, value);
  assert(status.ok());
  //for (size_t i = 0; i < 27; ++i) {
  //  std::string key;
  //  key.push_back('a' + i);
  //  std::string value("v");
  //  status = db->Put(leveldb::WriteOptions(), key, value);
  //  assert(status.ok());
  //}
  delete db;
  return 0;
}
