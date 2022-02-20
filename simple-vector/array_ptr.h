#pragma once

#include <algorithm>

template<typename Type>
class ArrayPtr {
public:
	ArrayPtr() = default;

	explicit ArrayPtr(size_t size) {
		raw_ptr_ = (size == 0) ? nullptr : new Type[size];
	}

	ArrayPtr(size_t size, Type&& value)
			: raw_ptr_(new Type[size]{}) {
		raw_ptr_[0] = std::move(value);
	}

	explicit ArrayPtr(Type *raw_ptr) noexcept
			: raw_ptr_(raw_ptr) {
	}

	ArrayPtr(const ArrayPtr&) = delete;

	ArrayPtr(ArrayPtr&& other) noexcept {
		swap(other);
	}

	~ArrayPtr() {
		delete[] raw_ptr_;
	}

	ArrayPtr& operator=(const ArrayPtr&) = delete;

	[[nodiscard]] Type *Release() noexcept {
		Type *p = raw_ptr_;
		raw_ptr_ = nullptr;
		return p;
	}

	Type& operator[](size_t index) noexcept {
		return raw_ptr_[index];
	}

	const Type& operator[](size_t index) const noexcept {
		return raw_ptr_[index];
	}

	explicit operator bool() const {
		return raw_ptr_ != nullptr;
	}

	Type *Get() const noexcept {
		return raw_ptr_;
	}

	void swap(ArrayPtr& other) noexcept {
		std::swap(other.raw_ptr_, raw_ptr_);
	}

private:
	Type *raw_ptr_ = nullptr;
};
