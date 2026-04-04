#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#ifdef SpectraProfiler_EXPORTS
#  define SpectraProfiler_API __declspec(dllexport)
#else
#  define SpectraProfiler_API __declspec(dllimport)
#endif
