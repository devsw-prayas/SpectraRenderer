#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#ifdef KerbecsValidation_EXPORTS
#  define KerbecsValidation_API __declspec(dllexport)
#else
#  define KerbecsValidation_API __declspec(dllimport)
#endif

KerbecsValidation_API void Init();
