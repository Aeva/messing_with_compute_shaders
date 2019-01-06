#include <vector>


struct BlobBuilder
{
	std::vector<char> Blob;
	size_t Seek;
	BlobBuilder(size_t Size, char ClearValue='\0')
	{
		Seek = 0;
		Blob.resize(Size, ClearValue);
	}
	template<typename T>
	T* Advance()
	{
		T* Handle = reinterpret_cast<T*>(Blob.data() + Seek);
		Seek += sizeof(T);
		return Handle;
	}
	template<typename T>
	void Write(T Value)
	{
		*Advance<T>() = Value;
	}
	void* Data()
	{
		return reinterpret_cast<void*>(Blob.data());
	}
};
