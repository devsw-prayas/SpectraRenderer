#pragma once
#include <cstdint>

namespace spectra::stl {
	template <bool V> struct BoolConstant { static constexpr bool value = V; };
	using TrueType = BoolConstant<true>;
	using FalseType = BoolConstant<false>;

	//Is Integral
	template <typename T> struct is_integral : FalseType {};
	template <> struct is_integral<bool> : TrueType {};
	template <> struct is_integral<char> : TrueType {};
	template <> struct is_integral<signed char> : TrueType {};
	template <> struct is_integral<unsigned char> : TrueType {};
	template <> struct is_integral<short> : TrueType {};
	template <> struct is_integral<unsigned short> : TrueType {};
	template <> struct is_integral<int> : TrueType {};
	template <> struct is_integral<unsigned int> : TrueType {};
	template <> struct is_integral<long> : TrueType {};
	template <> struct is_integral<unsigned long> : TrueType {};
	template <> struct is_integral<long long> : TrueType {};
	template <> struct is_integral<unsigned long long> : TrueType {};
	template <typename T> inline constexpr bool is_integral_v = is_integral<T>::value;

	//Is Floating Point
	template <typename T> struct is_floating_point : FalseType {};
	template <> struct is_floating_point<float> : TrueType {};
	template <> struct is_floating_point<double> : TrueType {};
	template <> struct is_floating_point<long double> : TrueType {};
	template <typename T> inline constexpr bool is_floating_point_v = is_floating_point<T>::value;

	//Is pointer
	template <typename T> struct is_pointer : FalseType {};
	template <typename T> struct is_pointer<T*> : TrueType {};
	template <typename T> inline constexpr bool is_pointer_v = is_pointer<T>::value;

	//Is same
	template <typename T, typename U> struct is_same : FalseType {};
	template <typename T> struct is_same<T, T> : TrueType {};
	template <typename T, typename U> inline constexpr bool is_same_v = is_same<T, U>::value;

	//Is constant
	template <typename T> struct is_const : FalseType {};
	template <typename T> struct is_const<const T> : TrueType {};
	template <typename T> inline constexpr bool is_const_v = is_const<T>::value;

	//Is destructible
	template <typename T> struct is_trivially_destructible {
		static constexpr bool value = __is_trivially_destructible(T);
	};
	template <typename T> inline constexpr bool is_trivially_destructible_v = is_trivially_destructible<T>::value;

	//Remove const
	template <typename T> struct remove_const { using type = T; };
	template <typename T> struct remove_const<const T> { using type = T; };
	template <typename T> using remove_const_t = typename remove_const<T>::type;

	//Remove reference
	template <typename T> struct remove_reference { using type = T; };
	template <typename T> struct remove_reference<T&> { using type = T; };
	template <typename T> struct remove_reference<T&&> { using type = T; };
	template <typename T> using remove_reference_t = typename remove_reference<T>::type;

	//Add pointer
	template <typename T> struct add_pointer { using type = T*; };
	template <typename T> using add_pointer_t = typename add_pointer<T>::type;

	//Enable if
	template <bool B, typename T = void> struct enable_if {};
	template <typename T> struct enable_if<true, T> { using type = T; };
	template <bool B, typename T> using enable_if_t = typename enable_if<B, T>::type;

	//Conditional
	template <bool B, typename T, typename F> struct conditional { using type = T; };
	template <typename T, typename F> struct conditional<false, T, F> { using type = F; };
	template <bool B, typename T, typename F> using conditional_t = typename conditional<B, T, F>::type;

	//Is convertible
	template <typename From, typename To> struct is_convertible : FalseType {};
	template <typename T> struct is_convertible<T, T> : TrueType {};
	template <typename From, typename To> inline constexpr bool is_convertible_v = is_convertible<From, To>::value;

	//Wrapper for integral constant
	template <typename T, T Value> struct integral_constant {
		static constexpr T value = Value;
		using value_type = T;
		using type = integral_constant;
		constexpr operator value_type() const noexcept { return value; }
		constexpr value_type operator()() const noexcept { return value; }
	};
	template <bool B> using bool_constant = integral_constant<bool, B>;
	using true_type = bool_constant<true>;
	using false_type = bool_constant<false>;

	//Move  semantics

	template <typename T>
	constexpr T&& forward(remove_reference_t<T>& t) noexcept {
		return static_cast<T&&>(t);
	}

	template <typename T>
	constexpr remove_reference_t<T>&& move(T&& t) noexcept {
		return static_cast<remove_reference_t<T>&&>(t);
	}

	template <typename T>
	struct is_numeric : BoolConstant<is_floating_point_v<T> || is_integral_v<T>> {};
	template <typename T> inline constexpr bool is_numeric_v = is_numeric<T>::value;

	template <typename T> struct avx_lanes { static constexpr size_t value = 0; };
	template <> struct avx_lanes<float> { static constexpr size_t value = 8; };
	template <> struct avx_lanes<double> { static constexpr size_t value = 4; };
	template <> struct avx_lanes<int8_t> { static constexpr size_t value = 32; };
	template <> struct avx_lanes<uint8_t> { static constexpr size_t value = 32; };
	template <> struct avx_lanes<int16_t> { static constexpr size_t value = 16; };
	template <> struct avx_lanes<uint16_t> { static constexpr size_t value = 16; };
	template <> struct avx_lanes<int32_t> { static constexpr size_t value = 8; };
	template <> struct avx_lanes<uint32_t> { static constexpr size_t value = 8; };
	template <> struct avx_lanes<int64_t> { static constexpr size_t value = 4; };
	template <> struct avx_lanes<uint64_t> { static constexpr size_t value = 4; };
	template <typename T> inline constexpr size_t avx_lanes_v = avx_lanes<T>::value;

	template <typename T> struct avx512_lanes { static constexpr size_t value = 0; };
	template <> struct avx512_lanes<float> { static constexpr size_t value = 16; };   
	template <> struct avx512_lanes<double> { static constexpr size_t value = 8; };   
	template <> struct avx512_lanes<int8_t> { static constexpr size_t value = 64; };  
	template <> struct avx512_lanes<uint8_t> { static constexpr size_t value = 64; };
	template <> struct avx512_lanes<int16_t> { static constexpr size_t value = 32; }; 
	template <> struct avx512_lanes<uint16_t> { static constexpr size_t value = 32; };
	template <> struct avx512_lanes<int32_t> { static constexpr size_t value = 16; }; 
	template <> struct avx512_lanes<uint32_t> { static constexpr size_t value = 16; };
	template <> struct avx512_lanes<int64_t> { static constexpr size_t value = 8; };  
	template <> struct avx512_lanes<uint64_t> { static constexpr size_t value = 8; };
	template <typename T> inline constexpr size_t avx512_lanes_v = avx512_lanes<T>::value;

	template <typename T>
	struct is_avx_supported : BoolConstant<avx_lanes_v<T> != 0> {};
	template <typename T> inline constexpr bool is_avx_supported_v = is_avx_supported<T>::value;
}
