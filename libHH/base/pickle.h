#ifndef PICKLE_H__
#define PICKLE_H__

#include <stdint.h>
#include <string>

#include "base/base_export.h"
#include "base/compiler_specific.h"

namespace base {

class Pickle;

class BASE_EXPORT PickleIterator
{
public:
	PickleIterator() : payload_(nullptr), read_index_(0), end_index_(0) {}
	explicit PickleIterator(const Pickle& pickle);

	bool ReadBool(bool* result);
	bool ReadInt(int* result);
	bool ReadUInt16(uint16_t* result);
	bool ReadUInt32(uint32_t* result);
	bool ReadInt32(int32_t* result);
	bool ReadInt64(int64_t* result);
	bool ReadUInt64(uint64_t* result);
	bool ReadSizeT(size_t* result);
	bool ReadFloat(float* result);
	bool ReadDouble(double* result);
	bool ReadString(std::string* result);
	bool ReadString16(std::wstring* result);

	// 不执行深度拷贝，只赋值指针，在消息生命周期内有效
	bool ReadData(const char** data, int* length);

	// 不执行深度拷贝，只赋值指针，在消息生命周期内有效
	bool ReadBytes(const char** data, int length);


	bool ReadLength(int* result) {
		return ReadInt(result) && *result >= 0;
	}

	// Skips bytes in the read buffer and returns true if there are at least
	// num_bytes available. Otherwise, does nothing and returns false.
	bool SkipBytes(int num_bytes) {
		return !!GetReadPointerAndAdvance(num_bytes);
	}

private:
	template <typename Type>
	bool ReadBuiltinType(Type* result);

	//移动读取指针，不能超过end_index_
	void Advance(size_t size);

	// 获取当前读取指针，并移动到下次读取位置
	template<typename Type>
	const char* GetReadPointerAndAdvance();

	const char* GetReadPointerAndAdvance(int num_bytes);

	const char* GetReadPointerAndAdvance(int num_elements, size_t size_element);

	const char* payload_;  // Start of our pickle's payload.
	size_t read_index_;    // Offset of the next readable byte in payload.
	size_t end_index_;     // Payload size.

};


class BASE_EXPORT Pickle
{
public:
	Pickle();

	explicit Pickle(int header_size);

	// 不执行深度拷贝
	Pickle(const char* data, int data_len);

	Pickle(const Pickle& other);

	virtual ~Pickle();

	Pickle& operator=(const Pickle& other);

	// 返回包的总大小，包括包头
	size_t size() const { return header_size_ + header_->payload_size; }

	const void* data() const { return header_; }

	// 返回动态分配的总容量大小
	size_t GetTotalAllocatedSize() const;

	bool WriteBool(bool value)		 { return WriteInt(value ? 1 : 0);}
	bool WriteInt(int value)		 { return WritePOD(value); }
	bool WriteUInt16(uint16_t value) { return WritePOD(value); }
	bool WriteUInt32(uint32_t value) { return WritePOD(value); }
	bool WriteInt32(int32_t value)   { return WritePOD(value); }
	bool WriteInt64(int64_t value)	 { return WritePOD(value); }
	bool WriteUInt64(uint64_t value) { return WritePOD(value); }
	bool WriteFloat(float value)	 { return WritePOD(value); }
	bool WriteDouble(double value)   { return WritePOD(value); }
	bool WriteSizeT(size_t value) {
		// Always write size_t as a 64-bit value to ensure compatibility between
		// 32-bit and 64-bit processes.
		return WritePOD(static_cast<uint64_t>(value));
	}
	bool WriteString(const std::string& value);
	bool WriteString16(const std::wstring& value);
	// "Data" is a blob with a length. When you read it out you will be given the
	// length. See also WriteBytes.
	bool WriteData(const char* data, int length);
	// "Bytes" is a blob with no length. The caller must specify the length both
	// when reading and writing. It is normally used to serialize PoD types of a
	// known size. See also WriteData.
	bool WriteBytes(const void* data, int length);

	// Reserves space for upcoming writes when multiple writes will be made and
	// their sizes are computed in advance. It can be significantly faster to call
	// Reserve() before calling WriteFoo() multiple times.
	void Reserve(size_t additional_capacity);

	struct Header 
	{
		uint32_t payload_size;
	};

	// Returns the header, cast to a user-specified type T.  The type T must be a
	// subclass of Header and its size must correspond to the header_size passed
	// to the Pickle constructor.
	template <class T>
	T* headerT() {
		return static_cast<T*>(header_);
	}
	template <class T>
	const T* headerT() const {
		return static_cast<const T*>(header_);
	}

	// 有效数据大小
	size_t payload_size() const {
		return header_ ? header_->payload_size : 0;
	}

	const char* payload() const {
		return reinterpret_cast<const char*>(header_) + header_size_;
	}

	// Returns the address of the byte immediately following the currently valid
	// header + payload.
	const char* end_of_payload() const {
		return header_ ? payload() + payload_size() : NULL;
	}

protected:
	char* mutable_payload() {
		return reinterpret_cast<char*>(header_) + header_size_;
	}

	size_t capacity_after_header() const {
		return capacity_after_header_;
	}

	// Resize the capacity, note that the input value should not include the size
	// of the header.
	void Resize(size_t new_capacity);

	// Claims |num_bytes| bytes of payload. This is similar to Reserve() in that
	// it may grow the capacity, but it also advances the write offset of the
	// pickle by |num_bytes|. Claimed memory, including padding, is zeroed.
	//
	// Returns the address of the first byte claimed.
	void* ClaimBytes(size_t num_bytes);

	// Find the end of the pickled data that starts at range_start.  Returns NULL
	// if the entire Pickle is not found in the given data range.
	static const char* FindNext(size_t header_size, const char* range_start, const char* range_end);

	// Parse pickle header and return total size of the pickle. Data range
	// doesn't need to contain entire pickle.
	// Returns true if pickle header was found and parsed. Callers must check
	// returned |pickle_size| for sanity (against maximum message size, etc).
	// NOTE: when function successfully parses a header, but encounters an
	// overflow during pickle size calculation, it sets |pickle_size| to the
	// maximum size_t value and returns true.
	static bool PeekNext(size_t header_size, const char* range_start, const char* range_end, size_t* pickle_size);

	// The allocation granularity of the payload.
	static const int kPayloadUnit;


private:
	friend class PickleIterator;

	Header* header_;
	size_t header_size_;

	//有效容量大小，不包括包头大小
	size_t capacity_after_header_;

	//不包括包头
	size_t write_offset_;

	// Just like WriteBytes, but with a compile-time size, for performance.
	template<size_t length>
    void BASE_EXPORT WriteBytesStatic(const void* data) {
        WriteBytesCommon(data, length);
    }

	// Writes a POD by copying its bytes.
	template <typename T> bool WritePOD(const T& data) {
		WriteBytesStatic<sizeof(data)>(&data);
		return true;
	}

	inline void* ClaimUninitializedBytesInternal(size_t num_bytes);
	inline void WriteBytesCommon(const void* data, size_t length);

};

}



#endif // PICKLE_H__
