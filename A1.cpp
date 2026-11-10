#include <iostream>
#include <stdexcept>
#include <cmath>
#include <random>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>


using namespace std;
using namespace glm;

const float THRESHOLD = (float)1.0e-5;


// Exercise 1: triple product proposition// -------------------------------------------------------------

bool vectorsEqual(const vec3& first, const vec3& second)
{
    return all(epsilonEqual(first, second, THRESHOLD));
}


vec3 generateVector() {
    static mt19937 generator(2026);
    uniform_real_distribution<float> distribution(1.0f, 5.0f);
    return vec3(distribution(generator), distribution(generator), distribution(generator));
}

// i × (j × k) = j(i ⋅ k) − k(i ⋅ j)

void ex1() {
    cout << "1. Show that the triple product proposition is true using vectors: i x (j x k) = j(i . k) - k(i . j)\n\n";

    vec3 i = generateVector();
    vec3 j = generateVector();
    vec3 k = generateVector();

    cout << "  i: " << to_string(i) << "\n";
    cout << "  j: " << to_string(j) << "\n";
    cout << "  k: " << to_string(k) << "\n";

    vec3 left = cross(i, cross(j, k));
    vec3 right = j * dot(i, k) - k * dot(i, j);

    cout << "  i x (j x k): " << to_string(left) << "\n";
    cout << "  j(i . k) - k(i . j) : " << to_string(right) << "\n";

    const bool equal = vectorsEqual(left, right);
    cout << "Equal: " << boolalpha << equal << "\n\n";
}

// Exercise 2: Camera Coordinate Frame// -------------------------------------------------------------

struct CameraFrame {
    vec3 u;
    vec3 v;
    vec3 w;
};

// Computes camera frame (u, v, w) from view and up vectors
CameraFrame CameraFrameFromViewUp(const vec3& view, const vec3& up) {
    if (length(view) < THRESHOLD) {
        throw invalid_argument("View vector is too small.");
    }

    vec3 v = normalize(view);
    vec3 w = cross(up, v);

    if (length(w) < THRESHOLD) {
        throw invalid_argument("Up vector cannot be parallel to view vector.");
    }

    w = normalize(w);
    vec3 u = cross(v, w);

    cout << "  v: " << to_string(v) << "\n";
    cout << "  w: " << to_string(w) << "\n";
    cout << "  u: " << to_string(u) << "\n";

    return CameraFrame{ u, v, w };
}

// Tests a single camera frame case and checks if it's orthonormal
void testCameraFrame(int testNum, const vec3& viewVector, const vec3& upVector = vec3(0.0f, 1.0f, 0.0f)) {
    cout << "Test " << testNum << ":\n";
    cout << "  view vector: " << to_string(viewVector) << "\n";
    cout << "  up vector:   " << to_string(upVector) << "\n";

    try {
        CameraFrame frame = CameraFrameFromViewUp(viewVector, upVector);

        // Verify that vectors have unit length
        bool uLen = epsilonEqual(length(frame.u), 1.0f, THRESHOLD);
        bool vLen = epsilonEqual(length(frame.v), 1.0f, THRESHOLD);
        bool wLen = epsilonEqual(length(frame.w), 1.0f, THRESHOLD);

        // Verify mutual orthogonality (dot products must be 0)
        bool uv = epsilonEqual(dot(frame.u, frame.v), 0.0f, THRESHOLD);
        bool vw = epsilonEqual(dot(frame.v, frame.w), 0.0f, THRESHOLD);
        bool wu = epsilonEqual(dot(frame.w, frame.u), 0.0f, THRESHOLD);

        cout << "  unit length checks: u=" << uLen << " v=" << vLen << " w=" << wLen << "\n";
        cout << "  orthogonality checks: u.v=" << uv << " v.w=" << vw << " w.u=" << wu << "\n";

        if (uLen && vLen && wLen && uv && vw && wu) {
            cout << "Camera Frame is valid.\n";
        } else {
            cout << "Camera Frame is invalid.\n";
        }
    } catch (const invalid_argument& e) {
        cout << "  error: " << e.what() << "\n";
        cout << "Camera Frame is invalid.\n";
    }

    cout << "\n";
}

void ex2() {
    cout << "2. Create a coordinate frame for the camera from view and up vectors.\n\n";

    // Test cases given in the assignment notes
    testCameraFrame(1, vec3(2.0f, 3.0f, 5.0f));
    testCameraFrame(2, vec3(3.0f, 2.0f, 5.0f));
    testCameraFrame(3, vec3(0.0f, 1.0f, 0.0f));
}

// Exercise 3: Rodrigues Rotation Formula (Vector form)// -------------------------------------------------------------

// Rotates vector v around axis a by theta radians
vec3 rodriguesRotation(const vec3& v, const vec3& a, float theta) {
    if (length(a) < THRESHOLD) {
        throw invalid_argument("Rotation axis is too small.");
    }

    vec3 unitAxis = normalize(a);
    float cos_theta = cos(theta);
    float sin_theta = sin(theta);

    return v * cos_theta + cross(unitAxis, v) * sin_theta + unitAxis * dot(unitAxis, v) * (1.0f - cos_theta);
}

// Tests rodriguesRotation and compares with the expected vector
void testRodriguesRotation(int testNum, const vec3& v, const vec3& a, float thetaDeg, const vec3& expected) {
    float thetaRad = radians(thetaDeg);

    cout << "Test " << testNum << ":\n";
    cout << "  v: " << to_string(v) << "\n";
    cout << "  a: " << to_string(a) << "\n";
    cout << "  theta: " << thetaDeg << " degrees (" << thetaRad << " radians)\n";
    cout << "  expected: " << to_string(expected) << "\n";

    try {
        vec3 result = rodriguesRotation(v, a, thetaRad);
        cout << "  result: " << to_string(result) << "\n";

        bool ok = all(epsilonEqual(result, expected, THRESHOLD));
        if (ok) {
            cout << "Result was equal to expected.\n";
        } else {
            cout << "Result was not equal to expected.\n";
        }
    } catch (const invalid_argument& e) {
        cout << "  error: " << e.what() << "\n";
    }

    cout << "\n";
}

void ex3() {
    cout << "3. Implement the Rodrigues Rotation Formula using vectors.\n\n";

    // Test 1: Rotate (2, 3, 1) around axis (1, 1, 1) by 120 degrees
    // (Cyclic coordinate shift (x, y, z) -> (z, x, y), all values in [1, 5])
    testRodriguesRotation(1, vec3(2, 3, 1), vec3(1, 1, 1), 120.0f, vec3(1, 2, 3));

    // Test 2: Rotate (1, 2, 3) around axis (1, 1, 1) by 240 degrees
    // (Double cyclic shift (x, y, z) -> (y, z, x), all values in [1, 5])
    testRodriguesRotation(2, vec3(1, 2, 3), vec3(1, 1, 1), 240.0f, vec3(2, 3, 1));

    // Test 3: Rotate (3, 4, 2) around axis (1, 1, 1) by 120 degrees
    // (Cyclic shift (3, 4, 2) -> (2, 3, 4), all values in [1, 5])
    testRodriguesRotation(3, vec3(3, 4, 2), vec3(1, 1, 1), 120.0f, vec3(2, 3, 4));
}

// -------------------------------------------------------------
// Exercise 4: Rodrigues Rotation Formula (Matrix form)
// -------------------------------------------------------------

// Computes the 3x3 rotation matrix using Rodrigues formula:
// R = I + sin(theta)*A + (1 - cos(theta))*A^2
// where A is the skew-symmetric cross-product matrix of unit axis a
mat3 rodriguesRotationMatrix(const vec3& a, float theta) {
    if (length(a) < THRESHOLD) {
        throw invalid_argument("Rotation axis is too small.");
    }

    vec3 unitA = normalize(a);

    // Skew-symmetric cross product matrix A = [unitA]_x
    // In GLM column-major order: mat3(col0, col1, col2)
    mat3 A(
         0.0f,    unitA.z, -unitA.y, // col 0
        -unitA.z, 0.0f,     unitA.x, // col 1
         unitA.y, -unitA.x, 0.0f     // col 2
    );

    mat3 I(1.0f);
    float cos_theta = cos(theta);
    float sin_theta = sin(theta);

    return I + sin_theta * A + (1.0f - cos_theta) * (A * A);
}

// Tests rodriguesRotationMatrix and verifies with R * v
void testRodriguesMatrix(int testNum, const vec3& v, const vec3& a, float thetaDeg, const vec3& expected) {
    float thetaRad = radians(thetaDeg);

    cout << "Test " << testNum << ":\n";
    cout << "  v: " << to_string(v) << "\n";
    cout << "  a: " << to_string(a) << "\n";
    cout << "  theta: " << thetaDeg << " degrees (" << thetaRad << " radians)\n";
    cout << "  expected: " << to_string(expected) << "\n";

    try {
        mat3 R = rodriguesRotationMatrix(a, thetaRad);
        vec3 result = R * v;
        cout << "  result: " << to_string(result) << "\n";

        bool ok = all(epsilonEqual(result, expected, THRESHOLD));
        if (ok) {
            cout << "Result was equal to expected.\n";
        } else {
            cout << "Result was not equal to expected.\n";
        }
    } catch (const invalid_argument& e) {
        cout << "  error: " << e.what() << "\n";
    }

    cout << "\n";
}

void ex4() {
    cout << "4. Implement the Rodrigues Rotation Formula in its matrix form.\n\n";

    vec3 ex(1, 0, 0);
    vec3 ey(0, 1, 0);
    vec3 ez(0, 0, 1);

    int testNum = 1;

    // Rotate each axis 3 times, each time by a different axis (rotate it by x, y, and z) by 90 degrees
    // X-axis rotated by x, y, z
    testRodriguesMatrix(testNum++, ex, ex, 90.0f, vec3(1, 0, 0));
    testRodriguesMatrix(testNum++, ex, ey, 90.0f, vec3(0, 0, -1));
    testRodriguesMatrix(testNum++, ex, ez, 90.0f, vec3(0, 1, 0));

    // Y-axis rotated by x, y, z
    testRodriguesMatrix(testNum++, ey, ex, 90.0f, vec3(0, 0, 1));
    testRodriguesMatrix(testNum++, ey, ey, 90.0f, vec3(0, 1, 0));
    testRodriguesMatrix(testNum++, ey, ez, 90.0f, vec3(-1, 0, 0));

    // Z-axis rotated by x, y, z
    testRodriguesMatrix(testNum++, ez, ex, 90.0f, vec3(0, -1, 0));
    testRodriguesMatrix(testNum++, ez, ey, 90.0f, vec3(1, 0, 0));
    testRodriguesMatrix(testNum++, ez, ez, 90.0f, vec3(0, 0, 1));
}

// -------------------------------------------------------------
// Exercise 5: Matrix Transpose Property (AB)^T = B^T * A^T
// -------------------------------------------------------------

// Validates matrix equality component by component using epsilonEqual as required by PDF
bool matricesEqual(const mat3& A, const mat3& B, float eps = THRESHOLD) {
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (!epsilonEqual(A[i][j], B[i][j], eps)) {
                return false;
            }
        }
    }
    return true;
}

// Generates a random 3x3 matrix with values between minVal and maxVal (default [-5, 5])
mat3 randomMat(float minVal = -5.0f, float maxVal = 5.0f) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<float> dis(minVal, maxVal);
    return mat3(
        dis(gen), dis(gen), dis(gen),
        dis(gen), dis(gen), dis(gen),
        dis(gen), dis(gen), dis(gen)
    );
}

// Validates (AB)^T = B^T * A^T
bool validateTranspose(const mat3& A, const mat3& B) {
    mat3 left = transpose(A * B);
    mat3 right = transpose(B) * transpose(A);

    cout << "  (AB)^T:\n" << to_string(left) << "\n";
    cout << "  B^T * A^T:\n" << to_string(right) << "\n";

    bool ok = matricesEqual(left, right);
    if (ok) {
        cout << "Proposition is true.\n";
    } else {
        cout << "Proposition is false.\n";
    }
    return ok;
}

void ex5() {
    cout << "5. Show that the following proposition is true using matrices and their transpose: (AB)^T = B^T * A^T\n\n";

    mat3 A = randomMat();
    mat3 B = randomMat();

    cout << "  A:\n" << to_string(A) << "\n";
    cout << "  B:\n" << to_string(B) << "\n";

    validateTranspose(A, B);
    cout << "\n";
}

// -------------------------------------------------------------
// Exercise 6: Matrix Inverse Property (AB)^-1 = B^-1 * A^-1
// -------------------------------------------------------------

// Generates an invertible random 3x3 matrix with non-zero determinant
mat3 randomInvertibleMat() {
    mat3 m;
    do {
        m = randomMat(-5.0f, 5.0f);
    } while (abs(determinant(m)) < 0.1f); // Avoid singular or poorly conditioned matrices
    return m;
}

// Validates (AB)^-1 = B^-1 * A^-1
bool validateInverse(const mat3& A, const mat3& B) {
    float detA = determinant(A);
    float detB = determinant(B);

    if (abs(detA) < THRESHOLD || abs(detB) < THRESHOLD) {
        throw invalid_argument("Matrix A or B is not invertible (det == 0).");
    }

    mat3 left = inverse(A * B);
    mat3 right = inverse(B) * inverse(A);

    cout << "  (AB)^-1:\n" << to_string(left) << "\n";
    cout << "  B^-1 * A^-1:\n" << to_string(right) << "\n";

    bool ok = matricesEqual(left, right, 1.0e-4f);
    if (ok) {
        cout << "Proposition is true.\n";
    } else {
        cout << "Proposition is false.\n";
    }
    return ok;
}

void ex6() {
    cout << "6. Show that the following proposition is true using matrices and their inverse: (AB)^-1 = B^-1 * A^-1\n\n";

    mat3 A = randomInvertibleMat();
    mat3 B = randomInvertibleMat();

    cout << "  A (det = " << determinant(A) << "):\n" << to_string(A) << "\n";
    cout << "  B (det = " << determinant(B) << "):\n" << to_string(B) << "\n";

    try {
        validateInverse(A, B);
    } catch (const invalid_argument& e) {
        cout << "  error: " << e.what() << "\n";
    }

    cout << "\n";
}

// -------------------------------------------------------------
// Main
// -------------------------------------------------------------

int main() {
    cout << "\nAssignment A1: Math Challenge\n\n";

    ex1();
    ex2();
    ex3();
    ex4();
    ex5();
    ex6();

    return 0;
}
