#pragma once

#include "phrase/format.hh"

#include <cassert>
#include <memory>

namespace phrase {

/* Format written:
 * uin16_t count of target phrases
 * bunch of layouts one after another with target phrases
 */
class TargetWriter {
  public:
    TargetWriter(FileFormat &file, std::size_t initial_buffer = 16384)
      : buffer_(initial_buffer), 
        buffer_end_(static_cast<char*>(buffer_.get() + initial_buffer)),
        file_(file) {}

  private:
    friend class TargetBundleWriter;

    void Write(char *current) {
      file_.DirectWriteTargetPhrases(buffer_.get(), current_ - static_cast<char*>(buffer_.get()));
    }

    void Reallocate(std::size_t to) {
      buffer_.call_realloc(to);
      buffer_end_ = static_cast<char*>(buffer_.get()) + to;
    }

    util::scoped_malloc buffer_;
    char *buffer_end_;

    FileFormat &file_;
};

// Write target phrases for a given source phrase.
class TargetBundleWriter {
  public:
    explicit TargetBundleWriter(TargetRecords &master) : master_(master) {
      // Save space for count.
      current_ = BufferBegin() + sizeof(uint16_t);
    }

    ~TargetBundleWriter() {
      *reinterpret_cast<uint16_t*>(BufferBegin()) = count_;
      master_.Write(current_);
    }

    uint64_t Offset() const { return master_.Offset(); }

    void *Allocate(std::size_t size) {
      ++count_;
      if (UTIL_LIKELY(current_ + size <= BufferEnd()))
        return Increment(size);
      Reallocate(size);
      return Increment(size);
    }

    bool Continue(void *&base, std::ptrdiff_t additional) {
      assert(base > buffer_.get() && base < BufferEnd());
      if (UTIL_LIKELY(current_ + additional <= BufferEnd())) {
        current_ += additional;
        return false;
      }
      std::size_t difference = static_cast<char*>(base) - BufferBegin();
      Reallocate(additional);
      current_ += additional;
      base = BufferBegin() + difference;
      return true;
    }

  private:
    void *Increment(std::size_t size) {
      void *ret = current_;
      current_ += size;
      return ret;
    }

    void Reallocate(std::size_t additional) {
      std::size_t existing_size = BufferEnd() - BufferBegin();
      std::size_t desired = existing_size + std::max<std::size_t>(additional, existing_size);
      std::size_t current_offset = current_ - BufferBegin();
      master_.Reallocate(desired);
      current_ = BufferBegin() + current_offset;
    }

    util::scoped_malloc &Buffer() { return master_.buffer_; }
    char *BufferBegin() { return reinterpret_cast<char*>(Buffer().get()); }
    char *BufferEnd() { return master_.buffer_end_; }

    char *current_;
    uint16_t count_ = 0;
    RecordWriter &master_;
};

} // namespace phrase
