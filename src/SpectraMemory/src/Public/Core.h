#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#ifdef SpectraMemory_EXPORTS
#  define SpectraMemory_API __declspec(dllexport)
#else
#  define SpectraMemory_API __declspec(dllimport)
#endif
