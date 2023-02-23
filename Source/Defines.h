#pragma once

#if defined FRUMPY_EXPORTS
#	define FRUMPY_API		__declspec(dllexport)
#elif defined FRUMPY_IMPORTS
#	define FRUMPY_API		__declspec(dllimport)
#else
#	define FRUMPY_API
#endif

#define FRUMPY_PI						3.1415926536
#define FRUMPY_EPS						1e-5
#define FRUMPY_DEGS_TO_RADS(x)			((x) * (FRUMPY_PI / 180.0))
#define FRUMPY_RADS_TO_DEGS(x)			((x) * (180.0 / FRUMPY_PI))
#define FRUMPY_MIN(x,y)					((x) < (y) ? (x) : (y))
#define FRUMPY_MAX(x,y)					((x) > (y) ? (x) : (y))
#define FRUMPY_CLAMP(x,a,b)				FRUMPY_MIN(FRUMPY_MAX(x,a),b)
#define FRUMPY_ENDIAN_SWAP4(x)			((((x) & 0xFF000000) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | (((x) & 0x000000FF) << 24))
#define FRUMPY_SIGN(x)					((x) < 0.0 ? -1.0 : 1.0)
#define FRUMPY_SQUARED(x)				((x) * (x))