#pragma once

namespace spectra::core::math {
	template <typename T, size_t dimensions>
	Vector<T, dimensions>::Vector() {
		components.reserve(dimensions);
	}

	template<typename T, size_t dimensions>
	Vector<T, dimensions>::~Vector() {
		components.resize(0);
	}

	template <typename T, size_t dimensions>
	Vector<T, dimensions>::Vector(const T* components) {
		for (size_t i = 0; i < dimensions; ++i) {
			this->components.push_back(components[i]);
		}
	}

	template <typename T, size_t dimensions>
	Vector<T, dimensions>::Vector(std::initializer_list<T> components) {
		if (components.size() != dimensions) {
			instrumentation::Instrumentation::log(instrumentation::E_LogLevel::ERROR_, "spectra::core", "Vector",
				"Initializer list size does not match vector dimensions", instrumentation::E_LogComponent::CORE, LOCATION);
		}
		for (const auto& component : components) {
			this->components.push_back(component);
		}
	}

	template <typename T, size_t dimensions>
	Vector<T, dimensions>::Vector(T val) {
		for (size_t i = 0; i < dimensions; ++i) {
			this->components.push_back(val);
		}
	}

	template <typename T, size_t dimensions>
	Vector<T, dimensions>::Vector(const Vector& other)
	{
		this->components.reserve(dimensions);
		for (size_t i = 0; i < dimensions; ++i) {
			this->components.push_back(other.components[i]);
		}
	}

	template<typename T, size_t dimensions>
	Vector<T, dimensions>::Vector(Vector&& other) noexcept {
		this->components = std::move(other.components);
		other.components.resize(0);
	}

}
