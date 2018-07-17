/*
  15-462 Computer Graphics I
  Assignment 3: Ray Tracer
  C++ Utility Classes and Functions
  Author: rtark
  Aug 2007
  Updated Oct 2007

  NOTE: You do not need to edit this file for this assignment but may do so, especially to the Camera class

  This file defines the following:
	Vector Class
	Matrix Class
	Camera Class
*/

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

inline float clamp(float value, float min, float max);

/*
	Vector Class - A float triplet class with several vector-operation functions

	This class can be used to simplify vector math programming
*/
class Vector
{
public:
	float x, y, z, w;

	// -- Constructors & Destructors --
	// - Default Constructor - Initializes to Vector <0, 0, 0, [1]>
	Vector (void) : x (0.0f), y (0.0f), z (0.0f), w (1.0f) {}

	// - Parameter Constructor - Initializes to Vector <a, b, c, 1>
	Vector (float a, float b, float c) : x (a), y (b), z (c), w (1.0f) {}

	// - Parameter Constructor - Initializes to Vector <a, b, c, d>
	Vector (float a, float b, float c, float d) : x (a), y (b), z (c), w (d) {}

	// - Default Destructor -
	~Vector ()	{}

	// -- Utility Functions --
	// - Magnitude - Returns the Magnitude of the current Vector
	inline float Magnitude (void) const;

	// - Normalize - Normalizes to a Unit Vector (Scales to magnitude of 1)
	inline Vector Normalize (void);

	// - Scale - Scales the Vector by a factor
	inline Vector Scale (float scaleFactor);

	// - Dot - Calculates the Dot-Product between this and another Vector
	inline float Dot (Vector vec2) const;

	// - Cross - Returns the Cross-Product between this and another Vector
	inline Vector Cross (Vector vec2) const;

	// - clamps each value between min and max
	inline Vector Clamp(float min, float max);

	// -- Operator Overloads to the class --
	// - Assignment Operator - Allows you to simply write "vec1 = vec2"
	Vector operator = (const Vector vec2)
	{
		x = vec2.x;
		y = vec2.y;
		z = vec2.z;
		w = vec2.w;
		return *this;
	}

	// NOTE: The following arithmetic operator overloads DO NOT change the value of the current vector
	// - Add Operator - Returns the sum of vectors
	Vector operator + (const Vector vec2)
	{
		return Vector (x + vec2.x, y + vec2.y, z + vec2.z, w + vec2.w);
	}

	// - Subtract Operator - Returns the difference of vectors
	Vector operator - (const Vector vec2)
	{
		return Vector (x - vec2.x, y - vec2.y, z - vec2.z, w - vec2.w);
	}

	// - Multiply Operator - Returns the vector scaled by a factor
	Vector operator * (const float scaleFactor)
	{
		return Vector (x * scaleFactor, y * scaleFactor, z * scaleFactor, w * scaleFactor);
	}

	// - Divide Operator - Returns the vectors scaled by a factor
	Vector operator / (const float scaleFactor)
	{
		return Vector (x / scaleFactor, y / scaleFactor, z / scaleFactor, w / scaleFactor);
	}

	// - Divide Operator - Returns the point by point division between two vector
	Vector operator / (const Vector otherVector)
	{
		return Vector(x / otherVector.x, y / otherVector.y, z / otherVector.z, w / otherVector.w);
	}

	// - Vector Multiply Operator -
	Vector operator * (const Vector scaleVector)
	{
		return Vector (x * scaleVector.x, y * scaleVector.y, z * scaleVector.z, w * scaleVector.w);
	}
};

/*
	Matrix Class - A 4x4 Matrix class with several matrix-matrix and matrix-vector operation functions

	This class can be used to simplify matrix & vector math programming
*/
class Matrix
{
public:
	// Values are defined with a naming convention of row_column
	//    e.g. _23 is the 2nd Row and 3rd Column element
	float _11; float _12; float _13; float _14;
	float _21; float _22; float _23; float _24;
	float _31; float _32; float _33; float _34;
	float _41; float _42; float _43; float _44;

	// -- Constructors & Destructors --
	// - Default Constructor - Initializes to Identity Matrix
	Matrix (void)
	{
		_12 = _13 = _14 = 0;
		_21 = _23 = _24 = 0;
		_31 = _32 = _34 = 0;
		_41 = _42 = _43 = 0;
		_11 = _22 = _33 = _44 = 1;
	}
	// - Default Destructor -
	~Matrix (void)
	{}

	// -- Utitility Functions --
	// - Identity - Initializes the matrix back to the Identity
	Matrix Identity (void)
	{
		_12 = _13 = _14 = 0;
		_21 = _23 = _24 = 0;
		_31 = _32 = _34 = 0;
		_41 = _42 = _43 = 0;
		_11 = _22 = _33 = _44 = 1;
		return *this;
	}

	// - Transpose - Transposes the Matrix
	Matrix Transpose (void)
	{
		Matrix retMatrix;

		retMatrix._11 = _11;
		retMatrix._12 = _21;
		retMatrix._13 = _31;
		retMatrix._14 = _41;

		retMatrix._21 = _12;
		retMatrix._22 = _22;
		retMatrix._23 = _32;
		retMatrix._24 = _42;

		retMatrix._31 = _13;
		retMatrix._32 = _23;
		retMatrix._33 = _33;
		retMatrix._34 = _43;

		retMatrix._41 = _14;
		retMatrix._42 = _24;
		retMatrix._43 = _34;
		retMatrix._44 = _44;
		
		return retMatrix;
	}

	// - Inverse - Calculates the Inverse of the 4x4 Matrix
	Matrix Inverse (void)
	{
		Matrix retMatrix;
		float determinant = _11*_22*_33*_44 + _11*_23*_34*_42 + _11*_24*_32*_43 +
							_12*_21*_34*_43 + _12*_23*_31*_44 + _12*_24*_33*_41 +
							_13*_21*_32*_44 + _13*_22*_34*_41 + _13*_24*_31*_42 +
							_14*_21*_33*_42 + _14*_22*_31*_43 + _14*_23*_32*_41 -
							_11*_22*_34*_43 - _11*_23*_32*_44 - _11*_24*_33*_42 -
							_12*_21*_33*_44 - _12*_23*_34*_41 - _12*_24*_31*_43 -
							_13*_21*_34*_42 - _13*_22*_31*_44 - _13*_24*_32*_41 -
							_14*_21*_32*_43 - _14*_22*_33*_41 - _14*_23*_31*_42;

		retMatrix._11 = _22*_33*_44 + _23*_34*_42 + _24*_32*_43 - _22*_34*_43 - _23*_32*_44 - _24*_33*_42;
		retMatrix._12 = _12*_34*_43 + _13*_32*_44 + _14*_33*_42 - _12*_33*_44 - _13*_34*_42 - _14*_32*_43;
		retMatrix._13 = _12*_23*_44 + _13*_24*_42 + _14*_22*_43 - _12*_24*_43 - _13*_22*_44 - _14*_23*_42;
		retMatrix._14 = _12*_24*_33 + _13*_22*_34 + _14*_23*_32 - _12*_23*_34 - _13*_24*_32 - _14*_22*_33;

		retMatrix._21 = _21*_34*_43 + _23*_31*_44 + _24*_33*_41 - _21*_33*_44 - _23*_34*_41 - _24*_31*_43;
		retMatrix._22 = _11*_33*_44 + _13*_34*_41 + _14*_31*_43 - _11*_34*_43 - _13*_31*_44 - _14*_33*_41;
		retMatrix._23 = _11*_24*_43 + _13*_21*_44 + _14*_23*_41 - _11*_23*_44 - _13*_24*_41 - _14*_21*_43;
		retMatrix._24 = _11*_23*_34 + _13*_24*_31 + _14*_21*_33 - _11*_24*_33 - _13*_21*_34 - _14*_23*_31;

		retMatrix._31 = _21*_32*_44 + _22*_34*_41 + _24*_31*_42 - _21*_34*_42 - _22*_31*_44 - _24*_32*_41;
		retMatrix._32 = _11*_34*_42 + _12*_31*_44 + _14*_32*_41 - _11*_32*_44 - _12*_34*_41 - _14*_31*_42;
		retMatrix._33 = _11*_22*_44 + _12*_24*_41 + _14*_21*_42 - _11*_24*_42 - _12*_21*_44 - _14*_22*_41;
		retMatrix._34 = _11*_24*_32 + _12*_21*_34 + _14*_22*_31 - _11*_22*_34 - _12*_24*_31 - _14*_21*_32;

		retMatrix._41 = _21*_33*_42 + _22*_31*_43 + _23*_32*_41 - _21*_32*_43 - _22*_33*_41 - _23*_31*_42;
		retMatrix._42 = _11*_32*_43 + _12*_33*_41 + _13*_31*_42 - _11*_33*_42 - _12*_31*_43 - _13*_32*_41;
		retMatrix._43 = _11*_23*_42 + _12*_21*_43 + _13*_22*_41 - _11*_22*_43 - _12*_23*_41 - _13*_21*_42;
		retMatrix._44 = _11*_22*_33 + _12*_23*_31 + _13*_21*_32 - _11*_23*_32 - _12*_21*_33 - _13*_22*_31;
		
		return retMatrix * (1.0f / determinant);
	}

	// -- Operator Overloads to the class --
	// - Assignment Operator - Allows you to simply write "mat1 = mat2"
	Matrix operator = (Matrix b)
	{
		_11 = b._11; _12 = b._12; _13 = b._13; _14 = b._14;
		_21 = b._21; _22 = b._22; _23 = b._23; _24 = b._24;
		_31 = b._31; _32 = b._32; _33 = b._33; _34 = b._34;
		_41 = b._41; _42 = b._42; _43 = b._43; _44 = b._44;

		return *this;
	}

	// - Matrix-Vector Multiplication - Returns the Vector multiplied by this matrix
	Vector operator * (const Vector vec)
	{
		Vector retVec;

		retVec.x = vec.x * _11 + vec.y * _12 + vec.z * _13 + vec.w * _14;
		retVec.y = vec.x * _21 + vec.y * _22 + vec.z * _23 + vec.w * _24;
		retVec.z = vec.x * _31 + vec.y * _32 + vec.z * _33 + vec.w * _34;
		retVec.w = vec.x * _41 + vec.y * _42 + vec.z * _43 + vec.w * _44;

		return retVec;
	}

	// - Matrix-Scale Multiplication - Returns the Matrix multiplied by the scalar value
	Matrix operator * (const float scalar)
	{
		Matrix retMatrix = (*this);

		retMatrix._11 *= scalar;
		retMatrix._12 *= scalar;
		retMatrix._13 *= scalar;
		retMatrix._14 *= scalar;

		retMatrix._21 *= scalar;
		retMatrix._22 *= scalar;
		retMatrix._23 *= scalar;
		retMatrix._24 *= scalar;

		retMatrix._31 *= scalar;
		retMatrix._32 *= scalar;
		retMatrix._33 *= scalar;
		retMatrix._34 *= scalar;

		retMatrix._41 *= scalar;
		retMatrix._42 *= scalar;
		retMatrix._43 *= scalar;
		retMatrix._44 *= scalar;

		return retMatrix;
	}

	// - Matrix-Matrix Multiplication - Returns the Matrix multiplied by the 2nd Matrix
	Matrix operator * (const Matrix mat) // Multiply Operator
	{
		Matrix retMatrix;

		retMatrix._11 = (_11 * mat._11) + (_12 * mat._21) + (_13 * mat._31) + (_14 * mat._41);
		retMatrix._12 = (_11 * mat._12) + (_12 * mat._22) + (_13 * mat._32) + (_14 * mat._42);
		retMatrix._13 = (_11 * mat._13) + (_12 * mat._23) + (_13 * mat._33) + (_14 * mat._43);
		retMatrix._14 = (_11 * mat._14) + (_12 * mat._24) + (_13 * mat._34) + (_14 * mat._44);

		retMatrix._21 = (_21 * mat._11) + (_22 * mat._21) + (_23 * mat._31) + (_24 * mat._41);
		retMatrix._22 = (_21 * mat._12) + (_22 * mat._22) + (_23 * mat._32) + (_24 * mat._42);
		retMatrix._23 = (_21 * mat._13) + (_22 * mat._23) + (_23 * mat._33) + (_24 * mat._43);
		retMatrix._24 = (_21 * mat._14) + (_22 * mat._24) + (_23 * mat._34) + (_24 * mat._44);

		retMatrix._31 = (_31 * mat._11) + (_32 * mat._21) + (_33 * mat._31) + (_34 * mat._41);
		retMatrix._32 = (_31 * mat._12) + (_32 * mat._22) + (_33 * mat._32) + (_34 * mat._42);
		retMatrix._33 = (_31 * mat._13) + (_32 * mat._23) + (_33 * mat._33) + (_34 * mat._43);
		retMatrix._34 = (_31 * mat._14) + (_32 * mat._24) + (_33 * mat._34) + (_34 * mat._44);

		retMatrix._41 = (_41 * mat._11) + (_42 * mat._21) + (_43 * mat._31) + (_44 * mat._41);
		retMatrix._42 = (_41 * mat._12) + (_42 * mat._22) + (_43 * mat._32) + (_44 * mat._42);
		retMatrix._43 = (_41 * mat._13) + (_42 * mat._23) + (_43 * mat._33) + (_44 * mat._43);
		retMatrix._44 = (_41 * mat._14) + (_42 * mat._24) + (_43 * mat._34) + (_44 * mat._44);

		return retMatrix;
	}
};


/*
	Camera Class - Class to help keep track of camera movements

	This class keeps the camera vectors together and adds a bit of functionality around it
	NOTE: This is unoptimized code for better readability
*/
class Camera
{
public:
	Vector position;
	Vector target;
	Vector up;
	float fieldOfView;
	float nearClip, farClip;

	// -- Constructors & Destructors --
	// - Default Constructor - Initializes to Vector <0, 0, 0>
	Camera (void) {}

	// - Parameter Constructor - Initializes to Vector <a, b, c>
	Camera (Vector p, Vector t, Vector u) { position = p; target = t; up = u; }

	// - Default Destructor -
	~Camera ()	{}

	// -- Accessor Functions --
	// - GetPosition - Returns the position vector of the camera
	Vector GetPosition (void) { return position; }

	// - SetPosition - Sets the position vector of the camera
	void SetPosition (const Vector vec) { position = vec; }

	// - GetTarget - Returns the target vector of the camera
	Vector GetTarget (void) { return target; }

	// - SetTarget - Sets the target vector of the camera
	void SetTarget (const Vector vec) { target = vec; }

	// - GetUp - Returns the up vector of the camera
	Vector GetUp (void) { return up; }

	// - SetUp - Sets the up vector of the camera
	void SetUp (const Vector vec) { up = vec; }

	// - GetFOV - Returns the Field of View
	float GetFOV (void) { return fieldOfView; }

	// - SetFOV - Sets of Field of View
	void SetFOV (float fov) { fieldOfView = fov; }

	// - GetNearClip - Returns the Near-Clip Distance
	float GetNearClip (void) { return nearClip; }

	// - SetNearClip - Sets the Near-Clip Distance
	void SetNearClip (float nc) { nearClip = nc; }

	// - GetFarClip - Returns the Far-Clip Distance
	float GetFarClip (void) { return farClip; }

	// - SetNearClip - Sets the Near-Clip Distance
	void SetFarClip (float fc) { farClip = fc; }
};




// --- Vector Class Functions ---

inline float Vector::Magnitude (void) const
{
	return (float)sqrt (x * x + y * y + z * z);
}

inline Vector Vector::Normalize (void)
{
	float currentMagnitude = Magnitude ();
	x /= currentMagnitude;
	y /= currentMagnitude;
	z /= currentMagnitude;
	return *this;
}

inline Vector Vector::Scale (float scaleFactor)
{
	x *= scaleFactor;
	y *= scaleFactor;
	z *= scaleFactor;
	w *= scaleFactor;
	return *this;
}

inline float Vector::Dot (Vector vec2) const
{
	return x * vec2.x + y * vec2.y + z * vec2.z;
}

inline Vector Vector::Cross (Vector vec2) const
{
	Vector crossVec;
	crossVec.x = y * vec2.z - vec2.y * z;
	crossVec.y = vec2.x * z - x * vec2.z;
	crossVec.z = x * vec2.y - vec2.x * y;
	crossVec.w = 0.0f;
	return crossVec;
}


inline Vector Vector::Clamp(float min, float max) {
	return Vector(clamp(x, min, max), clamp(y, min, max), clamp(z, min, max), clamp(w, min, max));
}
inline float clamp(float value, float min, float max)
{
	if (value < min) return min;
	else if (value > max) return max;
	else return value;
}
#endif // UTILS_H
