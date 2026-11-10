#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <random>
#include <stdexcept>

#include "glm/fwd.hpp"
#include "glm/matrix.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_cross_product.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace glm;

const float THRESHOLD = (float)1.0e-5;

// --------------------- EXERCISE 1 -------------------------

/**
 * Helper for exercise 1.
 * Returns a random vec3 with components in the range [1, 5].
 */
vec3 randomVec() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<float> dis(1, 5);
    return {dis(gen), dis(gen), dis(gen)};
}

/**
 * Helper for exercise 1.
 * Validates the triple product proposition using the given vectors i, j, and k.
 *
 * @param i The first vector.
 * @param j The second vector.
 * @param k The third vector.
 * @return True if the triple product proposition holds, false otherwise.
 */
bool validateTripleProduct(const vec3& i, const vec3& j, const vec3& k) {
    vec3 result_left = cross(i, cross(j, k));
    vec3 result_right = j * dot(i, k) - k * dot(i, j);

    cout << "  i x (j x k) = " << to_string(result_left) << "\n";
    cout << "  j(i ⋅ k) − k(i ⋅ j) = " << to_string(result_right) << "\n";

    return all(epsilonEqual(result_left, result_right, THRESHOLD));
}

/**
 * 1. Show that the triple product proposition is true using vectors
 */
void ex1() {
    cout << "1. Show that the triple product proposition is true using vectors\n\n";

    vec3 i = randomVec();
    cout << "  i: " << to_string(i) << "\n";

    vec3 j = randomVec();
    cout << "  j: " << to_string(j) << "\n";

    vec3 k = randomVec();
    cout << "  k: " << to_string(k) << "\n";

    if (validateTripleProduct(i, j, k)) {
        cout << "Proposition is true.\n";
    } else {
        cout << "Proposition is false.\n";
    }

    cout << "\n";
}


// --------------------- EXERCISE 2 -------------------------

/**
 * Helper struct for exercise 2.
 * Represents a camera frame from view and up vectors.
 */
struct CameraFrame {
    vec3 u;
    vec3 v;
    vec3 w;
};

/**
 * Helper function for exercise 2.
 * Computes a camera frame from view and up vectors.
 *
 * @param view The view vector.
 * @param up The up vector.
 * @return The camera frame.
 */
CameraFrame CameraFrameFromViewUp(const vec3& view, const vec3& up) {
    if (length(view) < THRESHOLD) {
        throw invalid_argument("View vector is too small.");
    }

    vec3 v = normalize(view);
    vec3 w = cross(up, v);

    if (length(w) < THRESHOLD) {
        throw invalid_argument("Up vector cannot be parallel to view vector.");
    }

    cout << "  v: " << to_string(v) << "\n";

    w = normalize(w);
    cout << "  w: " << to_string(w) << "\n";

    vec3 u = cross(v, w);
    cout << "  u: " << to_string(u) << "\n";

    return {.u = u, .v = v, .w = w};
}

/**
 * Helper function for exercise 2.
 * Tests the CameraFrameFromViewUp function with a given view and up vector.
 *
 * @param testNum The test number.
 * @param view The view vector.
 */
void testCameraFrame(int testNum, const vec3& test_view_vector) {
    vec3 up_vector = vec3(0.0f, 1.0f, 0.0f);

    cout << "Test " << testNum << ":\n";
    cout << "  view vector: " << to_string(test_view_vector) << "\n";
    cout << "  up vector: " << to_string(up_vector) << "\n";

    try {
        CameraFrame frame = CameraFrameFromViewUp(test_view_vector, up_vector);

        cout << "\nCamera Frame: \n";
        bool u_length_bool = epsilonEqual(length(frame.u), 1.0f, THRESHOLD);
        bool v_length_bool = epsilonEqual(length(frame.v), 1.0f, THRESHOLD);
        bool w_length_bool = epsilonEqual(length(frame.w), 1.0f, THRESHOLD);

        cout << "  u length: " << u_length_bool << " v length: " << v_length_bool
             << " w length: " << w_length_bool << "\n";

        bool u_dot_v = epsilonEqual(dot(frame.u, frame.v), 0.0f, THRESHOLD);
        bool v_dot_w = epsilonEqual(dot(frame.v, frame.w), 0.0f, THRESHOLD);
        bool w_dot_u = epsilonEqual(dot(frame.w, frame.u), 0.0f, THRESHOLD);

        cout << "  u dot v: " << u_dot_v << " v dot w: " << v_dot_w << " w dot u: " << w_dot_u
             << "\n";

        if (u_length_bool && v_length_bool && w_length_bool && u_dot_v && v_dot_w && w_dot_u) {
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

/**
 * 2. Create a coordinate frame for the camera from view and up vectors.
 */
void ex2() {
    std::cout << "2. Create a coordinate frame for the camera from view and up vectors.\n\n";

    vec3 test_view_vector = vec3(2.0f, 3.0f, 5.0f);
    testCameraFrame(1, test_view_vector);

    test_view_vector = vec3(3.0f, 2.0f, 5.0f);
    testCameraFrame(2, test_view_vector);

    test_view_vector = vec3(0.0f, 1.0f, 0.0f);
    testCameraFrame(3, test_view_vector);
}


// --------------------- EXERCISE 3 -------------------------

/**
 * Helper function for exercise 3.
 * Rodrigues Rotation Formula using vectors.
 *
 * @param v The vector to rotate.
 * @param a The axis of rotation.
 * @param theta The angle of rotation in radians.
 * @return The rotated vector.
 */
vec3 rodriguesRotation(const vec3& v, const vec3& a, float theta) {
    // Can we use glm_vec2_rotate?

    if (length(a) < THRESHOLD) {
        throw invalid_argument("Rotation axis is too small.");
    }

    float cos_theta = cos(theta);
    float sin_theta = sin(theta);

    return v * cos_theta + cross(a, v) * sin_theta + a * (1 - cos_theta) * dot(a, v);
}

/**
 * Helper function for exercise 3.
 * Tests the Rodrigues Rotation Formula using vectors.
 *
 * @param testNum The test number.
 * @param v The vector to rotate.
 * @param a The axis of rotation.
 * @param theta The angle of rotation in degrees.
 * @param expected The expected result.
 */
void testRodriguesRotation(int testNum, const vec3 v, const vec3 a, float theta,
                           const vec3& expected) {
    float thetaRad = radians(theta);

    cout << "Test " << testNum << ":\n";
    cout << "  v: " << to_string(v) << "\n";
    cout << "  a: " << to_string(a) << "\n";
    cout << "  theta: " << theta << " degrees (" << thetaRad << " radians)\n";
    cout << "  expected: " << to_string(expected) << "\n";

    try {
        vec3 result = rodriguesRotation(v, a, thetaRad);
        cout << "  result: " << to_string(result) << "\n";

        bool result_bool = all(epsilonEqual(result, expected, THRESHOLD));
        if (result_bool) {
            cout << "Result was equal to expected.\n";
        } else {
            cout << "Result was not equal to expected.\n";
        }

        cout << "\n";
    } catch (const invalid_argument& e) {
        cout << "  error: " << e.what() << "\n";
    }
}

/**
 * 3. Implement the Rodrigues Rotation Formula using vectors.
 */
void ex3() {
    cout << "3. Implement the Rodrigues Rotation Formula using vectors.\n\n";

    // Test 1 -> Rotate (1, 2, 3) around X-axis by 90 degrees
    int testNum = 1;
    vec3 v = vec3(1, 2, 3);
    vec3 a = vec3(1, 0, 0);
    int theta = 90;
    vec3 expected = vec3(1, -3, 2);
    testRodriguesRotation(testNum, v, a, theta, expected);

    // Test 2 -> Rotate (2, 3, 4) around Y-axis by 90 degrees
    testNum++;
    v = vec3(2, 3, 4);
    a = vec3(0, 1, 0);
    theta = 90;
    expected = vec3(4, 3, -2);
    testRodriguesRotation(testNum, v, a, theta, expected);

    // Test 3 -> Rotate (3, 2, 1) around Z-axis by 180 degrees
    testNum++;
    v = vec3(3, 2, 1);
    a = vec3(0, 0, 1);
    theta = 180;
    expected = vec3(-3, -2, 1);
    testRodriguesRotation(testNum, v, a, theta, expected);
}


// --------------------- EXERCISE 4 -------------------------

/**
 * Helper function for exercise 4.
 * Rodrigues Rotation Formula in its matrix form.
 *
 * @param a The axis of rotation.
 * @param theta The angle of rotation in radians.
 * @return The rotation matrix.
 */
mat3 rodriguesRotationMatrix(const vec3 a, float theta) {
    if (length(a) < THRESHOLD) {
        throw invalid_argument("Rotation axis is too small.");
    }

    mat3 I = mat3(1);

    // Can we use matrixCross3?
    mat3 A = matrixCross3(a);

    return I + sin(theta) * A + (1 - cos(theta)) * A * A;
}

/**
 * Helper function for exercise 4.
 * Tests the rodriguesRotationMatrix function and verifies the result.
 *
 * @param testNum The test number.
 * @param v The vector to rotate.
 * @param a The axis of rotation.
 * @param theta The angle of rotation in degrees.
 * @param expected The expected result.
 */
void testRodriguesMatrix(int testNum, const vec3& v, const vec3& a, float theta,
                         const vec3& expected) {
    float thetaRad = radians(theta);

    cout << "Test " << testNum << ":\n";
    cout << "  v: " << to_string(v) << "\n";
    cout << "  a: " << to_string(a) << "\n";
    cout << "  theta: " << theta << " degrees (" << thetaRad << " radians)\n";
    cout << "  expected: " << to_string(expected) << "\n";

    try {
        mat3 R = rodriguesRotationMatrix(a, thetaRad);
        vec3 result = R * v;
        cout << "  result: " << to_string(result) << "\n";

        bool result_bool = all(epsilonEqual(result, expected, THRESHOLD));
        if (result_bool) {
            cout << "Result was equal to expected.\n";
        } else {
            cout << "Result was not equal to expected.\n";
        }

        cout << "\n";
    } catch (const invalid_argument& e) {
        cout << "  error: " << e.what() << "\n";
    }
}

/**
 * 4. Implement the Rodrigues Rotation Formula in its matrix form.
 */
void ex4() {
    cout << "4. Implement the Rodrigues Rotation Formula in its matrix form.\n\n";

    vec3 ex(1, 0, 0);
    vec3 ey(0, 1, 0);
    vec3 ez(0, 0, 1);

    // The rotation is always 90 degrees.
    float theta = 90;

    // Rotate each axis 3 times, each time by a different axis (rotate it by x, y, and z).

    int testNum = 1;

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


// --------------------- EXERCISE 5 -------------------------

/**
 * Helper function for exercise 5.
 * Compares two matrices for equality.
 *
 * @param X The first matrix to compare.
 * @param Y The second matrix to compare.
 * @param threshold The threshold for comparison.
 * @return True if the matrices are equal, false otherwise.
 */
bool matEqual(const mat3& X, const mat3& Y, float threshold) {
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            if (!epsilonEqual(X[i][j], Y[i][j], threshold)) return false;
    return true;
}

/**
 * Helper function for exercise 5.
 * Compares the transpose of a matrix with the expected result.
 *
 * @param A The matrix to transpose.
 * @param B The expected result.
 */
void validateTranspose(const mat3& A, const mat3& B) {
    mat3 result_left = transpose(A * B);
    mat3 result_right = transpose(B) * transpose(A);

    cout << "  (AB)^T = " << to_string(result_left) << "\n";
    cout << "  B^T*A^T = " << to_string(result_right) << "\n";

    if (matEqual(result_left, result_right, THRESHOLD)) {
        cout << "Proposition is correct.\n";
    } else {
        cout << "Proposition is incorrect.\n";
    }
}

/**
 * Helper for exercises 5 and 6.
 * Returns a random mat3 with components in the range [-5, 5].
 */
mat3 randomMat() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_real_distribution<float> dis(-5, 5);
    return {dis(gen), dis(gen), dis(gen), dis(gen), dis(gen),
            dis(gen), dis(gen), dis(gen), dis(gen)};
}

/**
 * 5. Show that the following proposition is true using matrices and their transpose
 */
void ex5() {
    cout
        << "5. Show that the following proposition is true using matrices and their transpose.\n\n";

    mat3 A = randomMat();
    cout << "  A: " << to_string(A) << "\n";

    mat3 B = randomMat();
    cout << "  B: " << to_string(B) << "\n";

    validateTranspose(A, B);

    cout << "\n";
}

// --------------------- EXERCISE 6 -------------------------

/**
 * Helper function for exercise 6.
 * Compares the inverse of a matrix using matrix multiplication.
 * 
 * @param A The matrix to invert.
 * @param B The matrix to compare with the inverse.
 */
void validateInverse(const mat3& A, const mat3& B) {
    mat3 result_left = inverse(A * B);
    mat3 result_right = inverse(B) * inverse(A);

    cout << "  (AB)^-1 = " << to_string(result_left) << "\n";
    cout << "  B^-1*A^-1 = " << to_string(result_right) << "\n";

    if (matEqual(result_left, result_right, THRESHOLD)) {
        cout << "Proposition is correct.\n";
    } else {
        cout << "Proposition is incorrect.\n";
    }
}

/**
 * 6. Show that the following proposition is true using matrices and their inverse
 */
void ex6() {
    cout << "6. Show that the following proposition is true using matrices and their inverse.\n\n";

    mat3 A = randomMat();
    cout << "  A: " << to_string(A) << "\n";

    mat3 B = randomMat();
    cout << "  B: " << to_string(B) << "\n";

    validateInverse(A, B);

    cout << "\n";
}

int main() {
    cout << "\nGroup 20 CGJ Assignment 1\n\n";
    ex1();
    ex2();
    ex3();
    ex4();
    ex5();
    ex6();
    return 0;
}
