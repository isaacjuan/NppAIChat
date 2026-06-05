#ifndef SCI_POSITION_H
#define SCI_POSITION_H

#include <stddef.h>

typedef ptrdiff_t Sci_Position;
typedef size_t Sci_PositionU;
typedef intptr_t Sci_PositionCR;

#ifdef _WIN32
#define SCI_METHOD __stdcall
#else
#define SCI_METHOD
#endif

#endif
