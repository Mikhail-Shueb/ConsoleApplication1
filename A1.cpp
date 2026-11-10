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

// Main // -------------------------------------------------------------

int main() {
    cout << "\nAssignment A1: Math Challenge\n\n";

    ex1();
    ex2();
    ex3();

    return 0;
}
