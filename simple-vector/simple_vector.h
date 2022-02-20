#pragma once

#include "array_ptr.h"
#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <utility>

class ReserveProxyObj {
public:
	ReserveProxyObj(size_t capacity_to_reserve)
			: capacity_(capacity_to_reserve) {
	}

	size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
	return {capacity_to_reserve};
}

template<typename Type>
class SimpleVector {
public:
	using Iterator = Type *;
	using ConstIterator = const Type *;

	SimpleVector() noexcept = default;

	SimpleVector(ReserveProxyObj capacity) {
		capacity_ = capacity.capacity_;
	}

	explicit SimpleVector(size_t size)
			: SimpleVector(size, Type()) {
	}

	SimpleVector(size_t size, const Type& value)
			: size_(size), capacity_(size), items_(size) {
		std::fill(items_.Get(), items_.Get() + size_, Type{value});
	}

	SimpleVector(std::initializer_list<Type> init)
			: size_(init.size()), capacity_(init.size()), items_(init.size()) {
		int s = 0;
		for (auto i: init) {
			*(items_.Get() + s) = Type{i};
			++s;
		}
	}

	SimpleVector(const SimpleVector& other) {
		SimpleVector tmp(other.capacity_);
		std::copy(other.begin(), other.end(), tmp.begin());
		swap(tmp);
		size_ = other.size_;
	}

	SimpleVector(SimpleVector&& other) noexcept {
		swap(other);
	}

	SimpleVector& operator=(const SimpleVector& rhs) {
		if (this != &rhs) {
			SimpleVector rhs_copy(rhs);
			swap(rhs_copy);
		}
		size_ = rhs.size_;
		return *this;
	}

	void Reserve(size_t new_capacity) {
		if (capacity_ == 0) {
			SimpleVector tmp(new_capacity);
			swap(tmp);
			size_ = 0;
		} else if (capacity_ < new_capacity) {
			SimpleVector tmp(new_capacity);
			std::copy(this->begin(), this->end(), tmp.begin());
			size_t size = this->size_;
			swap(tmp);
			size_ = size;
		}
	}

	size_t GetSize() const noexcept {
		return size_;
	}

	size_t GetCapacity() const noexcept {
		return capacity_;
	}

	bool IsEmpty() const noexcept {
		return size_ == 0;
	}

	Type& operator[](size_t index) noexcept {
		return items_[index];
	}

	const Type& operator[](size_t index) const noexcept {
		return items_[index];
	}

	Type& At(size_t index) {
		if (index >= size_) {
			throw std::out_of_range("out_of_range");
		}
		return items_[index];
	}

	const Type& At(size_t index) const {
		if (index >= size_) {
			throw std::out_of_range("out_of_range");
		}
		return items_[index];
	}

	void Clear() noexcept {
		size_ = 0;
	}

	void Resize(size_t new_size) {
		if (size_ < new_size) {
			if (new_size <= capacity_) {
				auto first = items_.Get() + size_;
				auto last = items_.Get() + new_size;
				while (first < last) {
					*first = std::move(Type());
					first++;
				}
			} else {
				size_t new_capacity = std::max(new_size, capacity_ * 2);
				auto dist = std::distance(items_.Get(), items_.Get() + new_size);
				ArrayPtr<Type> tmp_vector(dist, Type{});
				std::move(items_.Get(), items_.Get() + size_, tmp_vector.Get());
				capacity_ = new_capacity;
				tmp_vector.swap(items_);
			}
		}
		size_ = new_size;
	}

	Iterator begin() noexcept {
		return items_.Get();
	}

	Iterator end() noexcept {
		return items_.Get() + size_;
	}

	ConstIterator begin() const noexcept {
		return cbegin();
	}

	ConstIterator end() const noexcept {
		return cend();
	}

	ConstIterator cbegin() const noexcept {
		return items_.Get();
	}

	ConstIterator cend() const noexcept {
		return items_.Get() + size_;
	}

	void NewCapacity() {
		size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
		ArrayPtr<Type> tmp_vector(new_capacity);
		std::move(begin(), end(), tmp_vector.Get());
		capacity_ = new_capacity;
		tmp_vector.swap(items_);
	}

	void PushBack(const Type& item) {
		if (size_ < capacity_) {
			items_[size_] = item;
		} else {
			NewCapacity();
			items_[size_] = item;
		}
		++size_;
	}

	void PushBack(Type&& item) {
		if (size_ < capacity_) {
			items_[size_] = std::move(item);
		} else {
			NewCapacity();
			items_[size_] = std::move(item);
		}
		++size_;
	}

	Iterator Insert(ConstIterator pos, const Type& value) {
		size_t to_pos = std::distance(begin(), Iterator(pos));
		if (size_ < capacity_) {
			std::copy_backward((begin() + to_pos), end(), (end() + 1));
			items_[to_pos] = value;
		} else {
			NewCapacity();
			std::copy_backward((begin() + to_pos), end(), (end() + 1));
			items_[to_pos] = value;
		}
		++size_;
		return (begin() + to_pos);
	}

	Iterator Insert(Iterator pos, Type&& value) {
		assert(pos >= begin() && pos <= end());
		size_t to_pos = std::distance(begin(), Iterator(pos));
		if (size_ < capacity_) {
			std::move_backward((begin() + to_pos), end(), (end() + 1));
			items_[to_pos] = std::move(value);
		} else {
			NewCapacity();
			std::move_backward((begin() + to_pos), end(), (end() + 1));
			items_[to_pos] = std::move(value);
		}
		++size_;
		return (begin() + to_pos);
	}

	void PopBack() noexcept {
		if (size_ != 0) {
			--size_;
		}
	}

	Iterator Erase(ConstIterator pos) {
		if (0 < size_ && pos <= items_.Get() + size_) {
			std::copy(Iterator(pos + 1), end(), Iterator(pos));
			--size_;
		}
		return Iterator(pos);
	}

	Iterator Erase(Iterator pos) {
		if (0 < size_ && pos <= items_.Get() + size_) {
			std::move(Iterator(pos + 1), end(), Iterator(pos));
			--size_;
		}
		return Iterator(pos);
	}

	void swap(SimpleVector& other) noexcept {
		std::swap(other.size_, size_);
		std::swap(other.capacity_, capacity_);
		other.items_.swap(items_);
	}

private:
	size_t size_ = 0;
	size_t capacity_ = 0;
	ArrayPtr<Type> items_{};
};


template<typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template<typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs == rhs);
}

template<typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return lhs < rhs || lhs == rhs;
}

template<typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template<typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return lhs > rhs || lhs == rhs;
}
