#include "Matrix3x3.h"
#include "Quaternion.h"
#include "Vector3.h"

using namespace Frumpy;

Matrix3x3::Matrix3x3()
{
}

Matrix3x3::Matrix3x3(const Matrix3x3& matrix)
{
}

Matrix3x3::Matrix3x3(const Quaternion& quat)
{
	quat.GetToMatrix(*this);
}

/*virtual*/ Matrix3x3::~Matrix3x3()
{
}

void Matrix3x3::Identity()
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			this->ele[i][j] = (i == j) ? 1.0 : 0.0;
}

void Matrix3x3::GetAxes(Vector3& xAxis, Vector3& yAxis, Vector3& zAxis) const
{
	xAxis.x = this->ele[0][0];
	xAxis.y = this->ele[1][0];
	xAxis.z = this->ele[2][0];

	yAxis.x = this->ele[0][1];
	yAxis.y = this->ele[1][1];
	yAxis.z = this->ele[2][1];

	zAxis.x = this->ele[0][2];
	zAxis.y = this->ele[1][2];
	zAxis.z = this->ele[2][2];
}

void Matrix3x3::SetAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis)
{
	this->ele[0][0] = xAxis.x;
	this->ele[1][0] = xAxis.y;
	this->ele[2][0] = xAxis.z;

	this->ele[0][1] = yAxis.x;
	this->ele[1][1] = yAxis.y;
	this->ele[2][1] = yAxis.z;

	this->ele[0][2] = zAxis.x;
	this->ele[1][2] = zAxis.y;
	this->ele[2][2] = zAxis.z;
}

void Matrix3x3::Multiply(const Matrix3x3& leftMatrix, const Matrix3x3& rightMatrix)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			this->ele[i][j] =
				leftMatrix.ele[i][0] * rightMatrix.ele[0][j] +
				leftMatrix.ele[i][1] * rightMatrix.ele[1][j] +
				leftMatrix.ele[i][2] * rightMatrix.ele[2][j];
		}
	}
}

bool Matrix3x3::Divide(const Matrix3x3& leftMatrix, const Matrix3x3& rightMatrix)
{
	Matrix3x3 rightMatrixInverted;
	if (!rightMatrixInverted.Invert(rightMatrix))
		return false;

	this->Multiply(leftMatrix, rightMatrixInverted);
	return true;
}

bool Matrix3x3::Invert(const Matrix3x3& matrix)
{
	double det = matrix.Determinant();
	if (det == 0.0)
		return false;

	this->ele[0][0] = (matrix.ele[1][1] * matrix.ele[2][2] - matrix.ele[2][1] * matrix.ele[1][2]) / det;
	this->ele[0][1] = -(matrix.ele[0][1] * matrix.ele[2][2] - matrix.ele[2][1] * matrix.ele[0][2]) / det;
	this->ele[0][2] = (matrix.ele[0][1] * matrix.ele[1][2] - matrix.ele[1][1] * matrix.ele[0][2]) / det;
	this->ele[1][0] = -(matrix.ele[1][0] * matrix.ele[2][2] - matrix.ele[2][0] * matrix.ele[1][2]) / det;
	this->ele[1][1] = (matrix.ele[0][0] * matrix.ele[2][2] - matrix.ele[2][0] * matrix.ele[0][2]) / det;
	this->ele[1][2] = -(matrix.ele[0][0] * matrix.ele[1][2] - matrix.ele[1][0] * matrix.ele[0][2]) / det;
	this->ele[2][0] = (matrix.ele[1][0] * matrix.ele[2][1] - matrix.ele[2][0] * matrix.ele[1][1]) / det;
	this->ele[2][1] = -(matrix.ele[0][0] * matrix.ele[2][1] - matrix.ele[2][0] * matrix.ele[0][1]) / det;
	this->ele[2][2] = (matrix.ele[0][0] * matrix.ele[1][1] - matrix.ele[1][0] * matrix.ele[0][1]) / det;

	return true;
}

void Matrix3x3::Transpose(const Matrix3x3& matrix)
{
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			this->ele[i][j] = matrix.ele[j][i];
}

double Matrix3x3::Determinant() const
{
	double det =
		this->ele[0][0] * (this->ele[1][1] * this->ele[2][2] - this->ele[2][1] * this->ele[1][2]) -
		this->ele[0][1] * (this->ele[1][0] * this->ele[2][2] - this->ele[2][0] * this->ele[1][2]) +
		this->ele[0][2] * (this->ele[1][0] * this->ele[2][1] - this->ele[2][0] * this->ele[1][1]);

	return det;
}

void Matrix3x3::SetRotation(double yaw, double pitch, double roll)
{
	//...
}

void Matrix3x3::GetRotation(double& yaw, double& pitch, double& roll) const
{
	//...
}

void Matrix3x3::SetRotation(const Vector3& axis, double angle)
{
	//...
}

void Matrix3x3::GetRotation(Vector3& axis, double& angle) const
{
	//...
}

namespace Frumpy
{
	Matrix3x3 operator*(const Matrix3x3& leftMatrix, const Matrix3x3& rightMatrix)
	{
		Matrix3x3 product;
		product.Multiply(leftMatrix, rightMatrix);
		return product;
	}
}