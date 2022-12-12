#pragma once

#include "Defines.h"

namespace Frumpy
{
	class Vector;

	// These are 4x4 matrices geared toward 3D computer graphics applications.
	class FRUMPY_API Matrix
	{
	public:
		Matrix();
		Matrix(const Matrix& matrix);
		virtual ~Matrix();

		void Identity();

		bool SetCol(int col, const Vector& vector);
		bool GetCol(int col, Vector& vector) const;

		void TransformVector(const Vector& vector, Vector& vectorTransformed) const;
		void TransformPoint(const Vector& point, Vector& pointTransformed) const;

		void Multiply(const Matrix& leftMatrix, const Matrix& rightMatrix);
		bool Divide(const Matrix& leftMatrix, const Matrix& rightMatrix);

		bool Invert(const Matrix& matrix);

		double Determinant() const;

		void Rotation(const Vector& axis, double angle);
		void Translation(const Vector& delta);
		void Projection(double hfovi, double vfovi, double near, double far);

		void operator=(const Matrix& matrix);

		double ele[4][4];
	};

	FRUMPY_API Matrix operator*(const Matrix& leftMatrix, const Matrix& rightMatrix);
}