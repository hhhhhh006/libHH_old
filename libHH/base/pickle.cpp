#include "pickle.h"
#include "bits.h"

namespace base {

	// static
const int Pickle::kPayloadUnit = 64;
static const size_t kCapacityReadOnly = static_cast<size_t>(-1);


PickleIterator::PickleIterator(const Pickle& pickle)
	: payload_(pickle.payload()),
	read_index_(0),
	end_index_(pickle.payload_size())
{

}

template <typename Type>
inline bool PickleIterator::ReadBuiltinType(Type* result)
{
	const char* read_from = GetReadPointerAndAdvance<Type>();
	if (!read_from)
		return false;

	if (sizeof(Type) > sizeof(uint32_t))
		memcpy(result, read_from, sizeof(*result));
	else
		*result = *reinterpret_cast<const Type*>(read_from);
	return true;
}

void PickleIterator::Advance(size_t size)
{
	size_t aligned_size = bits::Align(size, sizeof(uint32_t));
	if (end_index_ - read_index_ < aligned_size)
		read_index_ = end_index_;
	else
		read_index_ += aligned_size;
}

template<typename Type>
const char* PickleIterator::GetReadPointerAndAdvance()
{
	if (sizeof(Type) > end_index_ - read_index_)
	{
		read_index_ = end_index_;
		return nullptr;
	}
	const char* current_read_ptr = payload_ + read_index_;
	Advance(sizeof(Type));
	return current_read_ptr;
}

const char* PickleIterator::GetReadPointerAndAdvance(int num_bytes)
{
	if (num_bytes < 0 || end_index_ - read_index_ < static_cast<size_t>(num_bytes))
	{
		read_index_ = end_index_;
		return nullptr;
	}
	const char* current_read_ptr = payload_ + read_index_;
	Advance(num_bytes);
	return current_read_ptr;
}

const char* PickleIterator::GetReadPointerAndAdvance(int num_elements, size_t size_element)
{
	int64_t num_bytes = static_cast<int64_t>(num_elements) * size_element;
	int num_bytes32 = static_cast<int>(num_bytes);
	if (num_bytes != static_cast<int64_t>(num_bytes32))
		return nullptr;
	return GetReadPointerAndAdvance(num_bytes32);
}


bool PickleIterator::ReadBool(bool* result)
{
	return ReadBuiltinType(result);
}

bool PickleIterator::ReadInt(int* result)
{
	return ReadBuiltinType(result);
}

bool PickleIterator::ReadUInt16(uint16_t* result)
{
	return ReadBuiltinType(result);
}

bool PickleIterator::ReadUInt32(uint32_t* result)
{
	return ReadBuiltinType(result);
}

bool PickleIterator::ReadInt32(int32_t* result)
{
	return ReadBuiltinType(result);
}

bool PickleIterator::ReadInt64(int64_t* result)
{
	return ReadBuiltinType(result);
}

bool PickleIterator::ReadUInt64(uint64_t* result)
{
	return ReadBuiltinType(result);
}

bool PickleIterator::ReadSizeT(size_t* result)
{
	// Always read size_t as a 64-bit value to ensure compatibility between 32-bit
	// and 64-bit processes.
	uint64_t result_uint64 = 0;
	bool success = ReadBuiltinType(&result_uint64);
	*result = static_cast<size_t>(result_uint64);
	return success && (*result == result_uint64);
}

bool PickleIterator::ReadFloat(float* result)
{
	// The source data may not be properly aligned, and unaligned float reads
	// cause SIGBUS on some ARM platforms, so force using memcpy to copy the data
	// into the result.
	const char* read_from = GetReadPointerAndAdvance<float>();
	if (!read_from)
		return false;
	memcpy(result, read_from, sizeof(*result));
	return true;
}

bool PickleIterator::ReadDouble(double* result)
{
	// The source data may not be properly aligned, and unaligned double reads
	// cause SIGBUS on some ARM platforms, so force using memcpy to copy the data
	// into the result.
	const char* read_from = GetReadPointerAndAdvance<double>();
	if (!read_from)
		return false;
	memcpy(result, read_from, sizeof(*result));
	return true;
}

bool PickleIterator::ReadString(std::string* result)
{
	int len = 0;
	if (!ReadInt(&len))
		return false;

	const char* read_from = GetReadPointerAndAdvance(len);
	if (!read_from)
		return false;

	result->assign(read_from, len);
	return true;
}

bool PickleIterator::ReadString16(std::wstring* result)
{
	int len;
	if (!ReadInt(&len))
		return false;

	const char* read_from = GetReadPointerAndAdvance(len, sizeof(wchar_t));
	if (!read_from)
		return false;

	result->assign(reinterpret_cast<const wchar_t*>(read_from), len);
	return true;
}

bool PickleIterator::ReadData(const char** data, int* length)
{
	*data = 0;
	*length = 0;

	if (!ReadInt(length))
		return false;

	return ReadBytes(data, *length);
}

bool PickleIterator::ReadBytes(const char** data, int length)
{
	const char* read_from = GetReadPointerAndAdvance(length);
	if (!read_from)
		return false;

	*data = read_from;
	return true;
}




Pickle::Pickle()
	: header_(nullptr),
	header_size_(sizeof(Header)),
	capacity_after_header_(0),
	write_offset_(0)
{
	Resize(kPayloadUnit);
	header_->payload_size = 0;
}

Pickle::Pickle(int header_size)
	: header_(nullptr),
	header_size_(bits::Align(header_size, sizeof(uint32_t))),
	capacity_after_header_(0),
	write_offset_(0)
{
	Resize(kPayloadUnit);
	header_->payload_size = 0;
}

Pickle::Pickle(const char* data, int data_len)
	: header_(reinterpret_cast<Header*>(const_cast<char*>(data))),
	header_size_(0),
	capacity_after_header_(kCapacityReadOnly),
	write_offset_(0)
{
	if (data_len >= static_cast<int>(sizeof(Header)))
		header_size_ = data_len - header_->payload_size;

	if (header_size_ > static_cast<unsigned int>(data_len))
		header_size_ = 0;

	if (header_size_ != bits::Align(header_size_, sizeof(uint32_t)))
		header_size_ = 0;

	if (!header_size_)
		header_ = nullptr;
}

Pickle::Pickle(const Pickle& other)
	: header_(nullptr),
	header_size_(other.header_size_),
	capacity_after_header_(0),
	write_offset_(other.write_offset_)
{
	Resize(other.header_->payload_size);
	memcpy(header_, other.header_, header_size_ + other.header_->payload_size);
}

Pickle::~Pickle()
{
	if (capacity_after_header_ != kCapacityReadOnly)
		free(header_);
}

Pickle& Pickle::operator=(const Pickle& other)
{
	if (this == &other)
	{
		return *this;
	}
	if (capacity_after_header_ == kCapacityReadOnly)
	{
		header_ = nullptr;
		capacity_after_header_ = 0;
	}
	if (header_size_ != other.header_size_)
	{
		free(header_);
		header_ = nullptr;
		header_size_ = other.header_size_;
	}
	Resize(other.header_->payload_size);
	memcpy(header_, other.header_, other.header_size_ + other.header_->payload_size);
	write_offset_ = other.write_offset_;
	return *this;
}

void Pickle::Resize(size_t new_capacity)
{
	capacity_after_header_ = bits::Align(new_capacity, kPayloadUnit);
	void* p = realloc(header_, GetTotalAllocatedSize());
	header_ = reinterpret_cast<Header*>(p);
}

void* Pickle::ClaimBytes(size_t num_bytes)
{
	void* p = ClaimUninitializedBytesInternal(num_bytes);
	memset(p, 0, num_bytes);
	return p;
}

const char* Pickle::FindNext(size_t header_size, const char* range_start, const char* range_end)
{
	size_t pickle_size = 0;
	if (!PeekNext(header_size, range_start, range_end, &pickle_size))
		return nullptr;

	if (pickle_size > static_cast<size_t>(range_end - range_start))
		return nullptr;

	return range_start + pickle_size;
}

bool Pickle::PeekNext(size_t header_size, const char* range_start, const char* range_end, size_t* pickle_size)
{
	size_t length = static_cast<size_t>(range_end - range_start);
	if (length < sizeof(Header))
		return false;

	if (length < header_size)
		return false;

	const Header* header = reinterpret_cast<const Header*>(range_start);
	if (header->payload_size > std::numeric_limits<size_t>::max() - header_size)
	{
		// If payload_size causes an overflow, we return maximum possible
		// pickle size to indicate that.
		*pickle_size = std::numeric_limits<size_t>::max();
	}
	else
	{
		*pickle_size = header_size + header->payload_size;
	}
	return true;
}

size_t Pickle::GetTotalAllocatedSize() const
{
	if (capacity_after_header_ == kCapacityReadOnly)
		return 0;
	return header_size_ + capacity_after_header_;
}

bool Pickle::WriteString(const std::string& value)
{
	if (!WriteInt(static_cast<int>(value.size())))
		return false;
	return WriteBytes(value.data(), static_cast<int>(value.size()));
}

bool Pickle::WriteString16(const std::wstring& value)
{
	if (!WriteInt(static_cast<int>(value.size())))
		return false;
	return WriteBytes(value.data(), static_cast<int>(value.size()) * sizeof(wchar_t));
}

bool Pickle::WriteData(const char* data, int length)
{
	return length >= 0 && WriteInt(length) && WriteBytes(data, length);
}

bool Pickle::WriteBytes(const void* data, int length)
{
	WriteBytesCommon(data, length);
	return true;
}

void Pickle::Reserve(size_t additional_capacity)
{
	size_t data_len = bits::Align(additional_capacity, sizeof(uint32_t));
	size_t new_size = write_offset_ + data_len;
	if (new_size > capacity_after_header_)
		Resize(capacity_after_header_ * 2 + new_size);
}

template <size_t length> void Pickle::WriteBytesStatic(const void* data) {
	WriteBytesCommon(data, length);
}

template void Pickle::WriteBytesStatic<2>(const void* data);
template void Pickle::WriteBytesStatic<4>(const void* data);
template void Pickle::WriteBytesStatic<8>(const void* data);

void* Pickle::ClaimUninitializedBytesInternal(size_t num_bytes)
{
	size_t date_len = bits::Align(num_bytes, sizeof(uint32_t));
	size_t new_size = write_offset_ + date_len;
	if (new_size > capacity_after_header_)
	{
		size_t new_capacity = capacity_after_header_ * 2;
		const size_t kPickleHeapAlign = 4096;
		if (new_capacity > kPickleHeapAlign)
			new_capacity = bits::Align(new_capacity, kPickleHeapAlign) - kPayloadUnit;
		Resize(std::max(new_capacity, new_size));
	}

	char* write = mutable_payload() + write_offset_;
	memset(write + num_bytes, 0, date_len - num_bytes);
	header_->payload_size = static_cast<uint32_t>(new_size);
	write_offset_ = new_size;
	return write;
}

void Pickle::WriteBytesCommon(const void* data, size_t length)
{
	void* write = ClaimUninitializedBytesInternal(length);
	memcpy(write, data, length);
}


}

