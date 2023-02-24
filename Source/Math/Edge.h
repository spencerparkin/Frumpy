#pragma once

#include "Defines.h"
#include <stdint.h>

namespace Frumpy
{
	struct FRUMPY_API Edge
	{
		int i, j;

		bool operator<(const Edge& edge) const
		{
			int64_t keyA = this->CalcKey();
			int64_t keyB = edge.CalcKey();
			return keyA < keyB;
		}

		int64_t CalcKey() const
		{
			return (this->i < this->j) ? ((int64_t(this->i) << 32) | int64_t(this->j)) : ((int64_t(this->j) << 32) | int64_t(this->i));
		}
	};
}