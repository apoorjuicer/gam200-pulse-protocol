// ==================================================================================
// math.cpp 
// Author: Benjamin Ban
// Brief: Vector2 and Matrix3x3 class
//        
//        
// ==================================================================================
#include "pch/pch.h"


/************************************************* */
   /********************ctor/dtor******************** */
   /************************************************* */
   //default ctor
Vector2::Vector2() :m{ 0.0f, 0.0f },
x(m[0]), y(m[1]) { /*empty by design*/
}

//2 float param ctor
Vector2::Vector2(float x_, float y_) : m{ x_, y_ },
x(m[0]), y(m[1]) { /*empty by design*/
}

//copy ctor
Vector2::Vector2(Vector2 const& v) : m{ v.m[0], v.m[1] },
x(m[0]), y(m[1]) {/*empty by design*/
}

//default dtor
Vector2::~Vector2() = default;

/************************************************* */
/*************member functions******************** */
/************************************************* */

//copy assignment
Vector2& Vector2::operator=(Vector2 v) {
    if (this != &v) { m[0] = v.m[0]; m[1] = v.m[1]; }
    return *this;
}

// element access
float& Vector2::operator[](unsigned int i) {
    assert(i < 2);
    return m[i];
}
float Vector2::operator[](unsigned int i) const {
    assert(i < 2);
    return m[i];
}

Vector2& Vector2::operator+=(Vector2 v) {
    m[0] += v.m[0];
    m[1] += v.m[1];
    return *this;
}
Vector2& Vector2::operator-=(Vector2 v) {
    m[0] -= v.m[0];
    m[1] -= v.m[1];
    return *this;
}

Vector2& Vector2::operator*=(float s) {
    m[0] *= s;
    m[1] *= s;
    return *this;
}
Vector2& Vector2::operator/=(float s) {
    assert(s != 0.0f); //divison by zero
    m[0] /= s; m[1] /= s;
    return *this;
}

// unary minus
Vector2 Vector2::operator-() const {
    return Vector2(-m[0], -m[1]);
}

// length / normalization
float Vector2::Len() const {
    return std::sqrt(m[0] * m[0] + m[1] * m[1]);
}
float Vector2::LenSq() const {
    return m[0] * m[0] + m[1] * m[1];
}

Vector2 Vector2::Norm() const {
    float L = Len();
    return (L > 0.0f) ? Vector2(m[0] / L, m[1] / L) : *this;
}

void Vector2::Normalize() {
    float L = Len();
    if (L > 0.0f) { *this /= L; }
}

float distance(Vector2  v0, Vector2  v1) {
    float dx = v1.x - v0.x;
    float dy = v1.y - v0.y;
    return std::sqrt((dx * dx) + (dy * dy));
}

float distanceSq(Vector2   v0, Vector2  v1) {
    float dx = v1.x - v0.x;
    float dy = v1.y - v0.y;
    return (dx * dx) + (dy * dy);
}

float dot(Vector2  v0, Vector2  v1) {
    return (v0.x * v1.x) + (v0.y * v1.y);
}

float cross(Vector2   v0, Vector2  v1) {
    return (v0.x * v1.y) - (v0.y * v1.x);
}

Vector2 operator+(Vector2   v0, Vector2  v1) {
    Vector2 tmp{ v0 };
    return tmp += v1;
}
Vector2 operator-(Vector2   v0, Vector2  v1) {
    Vector2 tmp{ v0 };
    return tmp -= v1;
}
Vector2 operator*(Vector2  v, float s) { // vector * scalar
    Vector2 tmp{ v };
    return tmp *= s;
}
Vector2 operator*(float s, Vector2  v) { // scalar * vector
    Vector2 tmp{ v };
    return tmp *= s;
}
Vector2 operator/(Vector2  v, float s) {
    Vector2 tmp{ v };
    return tmp /= s;
}

std::ostream& operator<<(std::ostream& os, Vector2 const& vec) {
    os << std::fixed << std::setprecision(3);
    return os << "Vector2(" << vec[0] << ", " << vec[1] << ")";
}

/* ==============================MAT3 =======================*/
Matrix3x3::Matrix3x3() { SetToIdentity(); }

Matrix3x3::Matrix3x3(float e00, float e10, float e20,
    float e01, float e11, float e21,
    float e02, float e12, float e22) {
    m[0] = e00; m[1] = e10; m[2] = e20;
    m[3] = e01; m[4] = e11; m[5] = e21;
    m[6] = e02; m[7] = e12; m[8] = e22;
}
Matrix3x3::Matrix3x3(Matrix3x3 const& copied) {
    std::memcpy(m, copied.Begin(), sizeof(m));
}

Matrix3x3::~Matrix3x3() = default;

Matrix3x3& Matrix3x3::operator=(Matrix3x3 const& other) {
    if (this != &other) {
        std::memcpy(m, other.Begin(), sizeof(m));
    }
    return *this;
}


// () operators: component access
float& Matrix3x3::operator()(unsigned int column, unsigned int row) {
    assert(column < 3 && row < 3);
    return m[column * 3 + row];
}
const float& Matrix3x3::operator()(unsigned int column, unsigned int row) const {
    assert(column < 3 && row < 3);
    return m[column * 3 + row];
}

//iterators
float* Matrix3x3::Begin() { return &m[0]; }
float const* Matrix3x3::Begin() const { return &m[0]; }
float* Matrix3x3::End() { return &m[9]; }
float const* Matrix3x3::End() const { return &m[9]; }


void Matrix3x3::SetTo(float e00, float e10, float e20,
    float e01, float e11, float e21,
    float e02, float e12, float e22) {
    m[0] = e00; m[1] = e10; m[2] = e20;
    m[3] = e01; m[4] = e11; m[5] = e21;
    m[6] = e02; m[7] = e12; m[8] = e22;
}

void Matrix3x3::SetToIdentity() {
    m[0] = 1.0f; m[1] = 0.0f; m[2] = 0.0f; // col 0
    m[3] = 0.0f; m[4] = 1.0f; m[5] = 0.0f; // col 1
    m[6] = 0.0f; m[7] = 0.0f; m[8] = 1.0f; // col 2
}

void Matrix3x3::SetToZero() {
    m[0] = 0.0f; m[1] = 0.0f; m[2] = 0.0f; // col 0
    m[3] = 0.0f; m[4] = 0.0f; m[5] = 0.0f; // col 1
    m[6] = 0.0f; m[7] = 0.0f; m[8] = 0.0f; // col 2
}

void Matrix3x3::SetToTranspose() {
    for (unsigned int i = 0; i < 3; ++i) {
        for (unsigned int j = i + 1; j < 3; ++j) {
            //std::swap((*this)(i, j), (*this)(j, i));
            std::swap((i, j), (j, i));
        }
    }
}

Matrix3x3 Matrix3x3::GetTranspose() const {
    Matrix3x3 result{};
    for (unsigned int i = 0; i < 3; ++i) {
        for (unsigned int j = 0; j < 3; ++j) {
            //result(i, j) = (*this)(j, i);
            result(i, j) =  static_cast<float>(j, i);
        }
    }
    return result;
}

// * operator: matrix multiplication
Matrix3x3 operator*(Matrix3x3 const& L, Matrix3x3 const& R) {
    Matrix3x3 tmp{};
    for (unsigned col = 0; col < 3; ++col) {
        // take column c of R once
        float r0 = R(col, 0);
        float r1 = R(col, 1);
        float r2 = R(col, 2);

        // each row of result is a dot of row(L) with that column
        for (unsigned row = 0; row < 3; ++row) {
            tmp(col, row) = L(0, row) * r0
                            + L(1, row) * r1
                            + L(2, row) * r2;
        }
    }
    return tmp;
}

// * operator: scalar multiplication
Matrix3x3 operator*(float s, Matrix3x3 const& mat) {
    Matrix3x3 tmp{ mat };
    for (float* p = tmp.Begin(); p != tmp.End(); ++p) {
        *p *= s;
    }
    return tmp;
}

std::ostream& operator<<(std::ostream& os, Matrix3x3 const& M) {
    os << "Printing Matrix\n";
    for (unsigned int row = 0; row < 3; ++row) {
        os << "[ ";
        for (unsigned int col = 0; col < 3; ++col) {
            os << M(col, row) << ' ';   // <-- (col,row)
        }
        os << "]\n";
    }
    return os;
}
