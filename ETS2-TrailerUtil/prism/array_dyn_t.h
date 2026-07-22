#pragma once

#include <stdexcept>

#pragma pack(push, 1)
namespace prism
{
	class array_dyn_base_t
	{
	protected:
		void* items;      //0x0008
	public:
		uint64_t size;     //0x0010
		uint64_t capacity; //0x0018

		// Unknown
		virtual void initialize();

		// literally deletes all items
		virtual uint64_t clear();

		// Moves the array items list to a different memory location
		virtual uint64_t reallocate();

		virtual bool ensure_capacity(uint64_t index);
	};

	template<typename T = void*>
	class array_dyn_t : public array_dyn_base_t // Size: 0x0020
	{
	public:
		array_dyn_t(void* vtable_address)
		{
			*(void**)this = vtable_address;
		}

		T& operator[](uint64_t index) {
			if (index >= capacity)
				throw std::out_of_range("[prism] Index out of range");

			if constexpr (std::is_same_v<T, void*>) {
				return static_cast<void**>(items)[index];
			}
			else {
				return static_cast<T*>(items)[index];
			}
		}

		const T& operator[](uint64_t index) const {
			if (index >= capacity)
				throw std::out_of_range("[prism] Index out of range");

			if constexpr (std::is_same_v<T, void*>) {
				return static_cast<void**>(items)[index];
			}
			else {
				return static_cast<T*>(items)[index];
			}
		}

		class iterator {
			using raw_type = std::conditional_t<std::is_same_v<T, void*>, void*, T>;
			raw_type* ptr;
		public:
			iterator(raw_type* p) : ptr(p) {}
			raw_type& operator*() const { return *ptr; }
			iterator& operator++() { ++ptr; return *this; }
			bool operator!=(const iterator& other) const { return ptr != other.ptr; }
		};

		iterator begin() {
			using raw_type = std::conditional_t<std::is_same_v<T, void*>, void*, T>;
			return iterator(static_cast<raw_type*>(items));
		}

		iterator end() {
			using raw_type = std::conditional_t<std::is_same_v<T, void*>, void*, T>;
			return iterator(static_cast<raw_type*>(items) + size);
		}
	};
	static_assert(sizeof(array_dyn_t<>) == 0x20);
};
#pragma pack(pop)