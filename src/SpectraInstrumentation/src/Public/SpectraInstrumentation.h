#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#ifdef SpectraInstrumentation_EXPORTS
#  define SpectraInstrumentation_API __declspec(dllexport)
#else
#  define SpectraInstrumentation_API __declspec(dllimport)
#endif

SpectraInstrumentation_API void Init();
