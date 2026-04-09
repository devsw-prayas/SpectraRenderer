#include "SpectraCudaBackend.h"

#define  ALLOW_SYSCALL
#include "SpecCudaSyscall.h"

#ifdef  SPECTRA_OPTIX_AVAILABLE
#include <optix_function_table_definition.h>
#endif
