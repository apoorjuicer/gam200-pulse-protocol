// ==================================================================================
// math.h  
// Author: Benjamin Ban
// Brief: Vector2 and Matrix3 class
//        
//        
// ==================================================================================
#include "pch/pch.h"

#ifndef MATH_H_
#define MATH_H_

class Vector2 {
public:
	// ctr and dtr
	Vector2();
	Vector2(float x_compo, float y_compo);
	Vector2(Vector2 const& v);
	~Vector2();

	// copy assignment operator= - assign vector to another
	Vector2& operator=(Vector2  vec);

	// subscript operator[] - access element of vector
	float& operator[](unsigned int i);
	float operator[](unsigned int i) const;

	// arithmetic operators: vector operations +-
	Vector2& operator+=(Vector2 vec);
	Vector2& operator-=(Vector2 vec);

	// arithmetic operators: scalar operations */
	Vector2& operator*=(float scalar);
	Vector2& operator/=(float scalar);

	// arithmetic operators: negation
	Vector2 operator-(void) const;

	// length of vector
	float Len() const;

	// squared length of vector
	float LenSq() const;

	// normalization
	Vector2 Norm() const; // non-mutating, returns a normalized copy
	void       Normalize(); // mutating, normalizes in place
	//


private:
	float m[2];
public:
	// delegates
	float& x;  //!< Access to X component.
	float& y;  //!< Access to Y component.
}; // end of vector class

float distance(Vector2   v0, Vector2  v1);
float distanceSq(Vector2   v0, Vector2  v1);
float dot(Vector2   v0, Vector2  v1);
float cross(Vector2   v0, Vector2  v1);

Vector2 operator+(Vector2   v0, Vector2  v1);
Vector2 operator-(Vector2   v0, Vector2  v1);
Vector2 operator*(Vector2  v, float s);     // vector * scalar
Vector2 operator*(float s, Vector2  v);         // scalar * vector
Vector2 operator/(Vector2  v, float s);     // vector / scalar
std::ostream& operator<<(std::ostream& os, Vector2 const& vec);

class Matrix3 {
private:
		float m[9];      // column-major flat array
public:
	Matrix3(); //default identity
	Matrix3(float e00, float e10, float e20,
		float e01, float e11, float e21,
		float e02, float e12, float e22);
	Matrix3(Matrix3 const& m);
	~Matrix3();

	// = operator: assignment
	Matrix3& operator=(Matrix3 const& copied);
	// () operators: component access
	float& operator()(unsigned int column, unsigned int row);
	const float& operator()(unsigned int column, unsigned int row) const;

	// iterators
	float* Begin();
	const float* Begin() const;
	float* End();
	const float* End() const;

	void	SetTo(float e00, float e10, float e20,
		float e01, float e11, float e21,
		float e02, float e12, float e22);
	void	SetToZero();			// zero all 9 elements
	void	SetToIdentity();	// set 9 elements to define an identity matrix


}; // end of matrix 3 class

// * operator: matrix multiplication
Matrix3 operator*(Matrix3 const& l, Matrix3 const& r);

// * operator: scalar multiplication
Matrix3 operator*(float s, Matrix3 const& mat);

std::ostream& operator<<(std::ostream& os, Matrix3 const& mat3);

#endif	/* MATH_H_ */

