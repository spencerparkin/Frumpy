#include "Matrix.h"
#include "Vector.h"
#include <math.h>

using namespace Frumpy;

Matrix::Matrix()
{
	this->Identity();
}

Matrix::Matrix(const Matrix& matrix)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			this->ele[i][j] = matrix.ele[i][j];
}

/*virtual*/ Matrix::~Matrix()
{
}

void Matrix::operator=(const Matrix& matrix)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			this->ele[i][j] = matrix.ele[i][j];
}

void Matrix::Identity()
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			this->ele[i][j] = (i == j) ? 1.0 : 0.0;
}

bool Matrix::SetCol(int col, const Vector& vector)
{
	if (col < 0 || col > 3)
		return false;

	this->ele[0][col] = vector.x;
	this->ele[1][col] = vector.y;
	this->ele[2][col] = vector.z;
	return true;
}

bool Matrix::GetCol(int col, Vector& vector) const
{
	if (col < 0 || col > 3)
		return false;

	vector.x = this->ele[0][col];
	vector.y = this->ele[1][col];
	vector.z = this->ele[2][col];
	return true;
}

void Matrix::TransformVector(const Vector& vector, Vector& vectorTransformed) const
{
	vectorTransformed.x =
		vector.x * this->ele[0][0] +
		vector.y * this->ele[0][1] +
		vector.z * this->ele[0][2];

	vectorTransformed.y =
		vector.x * this->ele[1][0] +
		vector.y * this->ele[1][1] +
		vector.z * this->ele[1][2];

	vectorTransformed.z =
		vector.x * this->ele[2][0] +
		vector.y * this->ele[2][1] +
		vector.z * this->ele[2][2];
}

void Matrix::TransformPoint(const Vector& point, Vector& pointTransformed) const
{
	pointTransformed.x =
		point.x * this->ele[0][0] +
		point.y * this->ele[0][1] +
		point.z * this->ele[0][2] +
		this->ele[0][3];

	pointTransformed.y =
		point.x * this->ele[1][0] +
		point.y * this->ele[1][1] +
		point.z * this->ele[1][2] +
		this->ele[1][3];

	pointTransformed.z =
		point.x * this->ele[2][0] +
		point.y * this->ele[2][1] +
		point.z * this->ele[2][2] +
		this->ele[2][3];

	double w = 
		point.x * this->ele[3][0] +
		point.y * this->ele[3][1] +
		point.z * this->ele[3][2] +
		this->ele[3][3];

	if (w != 1.0)
	{
		pointTransformed.x /= w;
		pointTransformed.y /= w;
		pointTransformed.z /= w;
	}
}

void Matrix::Multiply(const Matrix& leftMatrix, const Matrix& rightMatrix)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			this->ele[i][j] =
				leftMatrix.ele[i][0] * rightMatrix.ele[0][j] +
				leftMatrix.ele[i][1] * rightMatrix.ele[1][j] +
				leftMatrix.ele[i][2] * rightMatrix.ele[2][j] +
				leftMatrix.ele[i][3] * rightMatrix.ele[3][j];
		}
	}
}

bool Matrix::Divide(const Matrix& leftMatrix, const Matrix& rightMatrix)
{
	Matrix rightMatrixInverted;
	if (!rightMatrixInverted.Invert(rightMatrix))
		return false;

	this->Multiply(leftMatrix, rightMatrixInverted);
	return true;
}

bool Matrix::Invert(const Matrix& matrix)
{
	return false;
}

double Matrix::Determinant() const
{
	return 0.0;
}

void Matrix::Rotation(const Vector& axis, double angle)
{
	Vector xAxis(1.0, 0.0, 0.0);
	Vector yAxis(0.0, 1.0, 0.0);
	Vector zAxis(0.0, 0.0, 1.0);

	Vector xAxisRotated, yAxisRotated, zAxisRotated;

	xAxisRotated.Rotation(xAxis, axis, angle);
	yAxisRotated.Rotation(yAxis, axis, angle);
	zAxisRotated.Rotation(zAxis, axis, angle);

	this->Identity();
	this->SetCol(0, xAxisRotated);
	this->SetCol(1, yAxisRotated);
	this->SetCol(2, zAxisRotated);
}

void Matrix::Translation(const Vector& delta)
{
	this->Identity();
	this->ele[0][3] = delta.x;
	this->ele[1][3] = delta.y;
	this->ele[2][3] = delta.z;
}

void Matrix::Projection(double hfovi, double vfovi, double near, double far)
{
	this->Identity();

	// TODO: Check this.  I'm sure this is wrong.
	this->ele[0][0] = 1.0 / tan(hfovi / 2.0);
	this->ele[1][1] = 1.0 / tan(vfovi / 2.0);
	this->ele[2][2] = -far / (far - near);
	this->ele[3][3] = 0.0;
	this->ele[2][3] = far * near / (far - near);
}

namespace Frumpy
{
	Matrix operator*(const Matrix& leftMatrix, const Matrix& rightMatrix)
	{
		Matrix product;
		product.Multiply(leftMatrix, rightMatrix);
		return product;
	}
}