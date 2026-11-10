// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using Vec3 = glm::vec3;
using Mat4 = glm::mat4;
constexpr float epsilon = 0.0001f;

void printVector(const std::string& label, const Vec3& vector)
{
    std::cout << std::left << std::setw(22) << label << "[ "
              << std::fixed << std::setprecision(4)
              << vector.x << ", " << vector.y << ", " << vector.z << " ]\n";
}

void printMatrix(const std::string& label, const Mat4& matrix)
{
    std::cout << label << "\n";
    for (int row = 0; row < 4; ++row)
    {
        std::cout << "  [ ";
        for (int column = 0; column < 4; ++column)
            std::cout << std::fixed << std::setprecision(4)
                      << matrix[column][row] << (column == 3 ? " " : ", ");
        std::cout << "]\n";
    }
}

Vec3 transformVector(const Mat4& matrix, const Vec3& vector)
{
    return Vec3(matrix * glm::vec4(vector, 1.0f));
}

bool vectorsEqual(const Vec3& first, const Vec3& second)
{
    return glm::length(first - second) < epsilon;
}

bool crossProductIsDistributive(const Vec3& u, const Vec3& v, const Vec3& w)
{
    const Vec3 left = glm::cross(u, v + w);
    const Vec3 right = glm::cross(u, v) + glm::cross(u, w);

    printVector("u x (v + w)", left);
    printVector("(u x v) + (u x w)", right);
    const bool equal = vectorsEqual(left, right);
    std::cout << "Equal: " << std::boolalpha << equal << "\n";
    return equal;
}

Mat4 rotationX(float degrees)
{
    return glm::rotate(Mat4(1.0f), glm::radians(degrees), Vec3(1.0f, 0.0f, 0.0f));
}

Mat4 rotationY(float degrees)
{
    return glm::rotate(Mat4(1.0f), glm::radians(degrees), Vec3(0.0f, 1.0f, 0.0f));
}

void exercise2()
{
    const Mat4 rx = rotationX(45.0f);
    const Mat4 ry = rotationY(90.0f);
    const Vec3 vectors[] = { Vec3(1.0f), Vec3(-1.0f, -1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f) };
    const char* names[] = { "u", "v", "w" };

    printMatrix("Rx (45 degrees)", rx);
    printMatrix("Ry (90 degrees)", ry);
    for (int i = 0; i < 3; ++i)
    {
        printVector(std::string("Rx(") + names[i] + ")", transformVector(rx, vectors[i]));
        printVector(std::string("Ry(") + names[i] + ")", transformVector(ry, vectors[i]));
    }
}

bool rotationsAreNonCommutative(const Vec3& vector)
{
    const Vec3 rotateXThenY = transformVector(rotationY(90.0f) * rotationX(45.0f), vector);
    const Vec3 rotateYThenX = transformVector(rotationX(45.0f) * rotationY(90.0f), vector);

    printVector("Rotate X then Y", rotateXThenY);
    printVector("Rotate Y then X", rotateYThenX);
    const bool different = !vectorsEqual(rotateXThenY, rotateYThenX);
    std::cout << "Results are different: " << std::boolalpha << different << "\n";
    return different;
}

Vec3 orthogonalVector(const Vec3& first, const Vec3& second)
{
    return glm::normalize(glm::cross(first, second));
}

bool areOrthogonal(const Vec3& first, const Vec3& second, const Vec3& candidate)
{
    return std::abs(glm::dot(first, candidate)) < epsilon
        && std::abs(glm::dot(second, candidate)) < epsilon;
}

void printSquareMatrix(const std::string& name, const Mat4& squareMatrix)
{
    std::cout << name << "\n";
    const char* vertexNames[] = { "A", "B", "C", "D" };
    for (int col = 0; col < 4; ++col)
    {
        std::cout << "  Vertex " << vertexNames[col] << " (col " << (col + 1) << "): [ "
                  << std::fixed << std::setprecision(4)
                  << squareMatrix[col].x << ", "
                  << squareMatrix[col].y << ", "
                  << squareMatrix[col].z << ", "
                  << squareMatrix[col].w << " ]\n";
    }
}

void exercise4()
{
    std::cout << "--- Exercise 4: Orthogonal Vectors to a Plane ---\n";
    std::cout << "Plane basis vectors: a = [1, 2, 3], b = [4, 5, 6]\n";
    std::cout << "\n1) Analytical Computation:\n";
    std::cout << "   Normal vector n_unnorm = a x b:\n";
    std::cout << "     nx = (2*6 - 3*5) = 12 - 15 = -3\n";
    std::cout << "     ny = -(1*6 - 3*4) = -(6 - 12) = 6\n";
    std::cout << "     nz = (1*5 - 2*4) = 5 - 8 = -3\n";
    std::cout << "   n_unnorm = [-3, 6, -3]\n";
    std::cout << "   Magnitude: sqrt((-3)^2 + 6^2 + (-3)^2) = sqrt(54) = 3*sqrt(6) ~= 7.3485\n";
    std::cout << "   Unit normal n = [-3/(3*sqrt(6)), 6/(3*sqrt(6)), -3/(3*sqrt(6))]\n";
    std::cout << "                 = [-1/sqrt(6), 2/sqrt(6), -1/sqrt(6)] ~= [-0.4082, 0.8165, -0.4082]\n";
    std::cout << "\n   Analytical Verification (Dot Products):\n";
    std::cout << "     a . n = 1*(-1/sqrt(6)) + 2*(2/sqrt(6)) + 3*(-1/sqrt(6)) = (-1 + 4 - 3)/sqrt(6) = 0 (Orthogonal)\n";
    std::cout << "     b . n = 4*(-1/sqrt(6)) + 5*(2/sqrt(6)) + 6*(-1/sqrt(6)) = (-4 + 10 - 6)/sqrt(6) = 0 (Orthogonal)\n";

    const Vec3 a(1.0f, 2.0f, 3.0f);
    const Vec3 b(4.0f, 5.0f, 6.0f);
    const Vec3 normal = orthogonalVector(a, b);

    std::cout << "\n2) GLM Implementation Results:\n";
    printVector("a x b", glm::cross(a, b));
    printVector("orthogonalVector(a, b)", normal);
    std::cout << "a . normal = " << std::fixed << std::setprecision(4) << glm::dot(a, normal) << "\n";
    std::cout << "b . normal = " << std::fixed << std::setprecision(4) << glm::dot(b, normal) << "\n";
    std::cout << "areOrthogonal(a, b, normal): " << std::boolalpha << areOrthogonal(a, b, normal) << "\n";

    std::cout << "\n3) 2D Cartesian Basis Test (X = [1,0,0], Y = [0,1,0]):\n";
    const Vec3 x(1.0f, 0.0f, 0.0f);
    const Vec3 y(0.0f, 1.0f, 0.0f);
    const Vec3 normalXY = orthogonalVector(x, y);
    printVector("orthogonalVector(X, Y)", normalXY);
    std::cout << "areOrthogonal(X, Y, normalXY): " << std::boolalpha << areOrthogonal(x, y, normalXY) << "\n";

    std::cout << "\n4) What happens if you swap vectors in orthogonalVector?\n";
    const Vec3 swappedNormalAB = orthogonalVector(b, a);
    const Vec3 swappedNormalYX = orthogonalVector(y, x);
    printVector("orthogonalVector(b, a)", swappedNormalAB);
    printVector("orthogonalVector(Y, X)", swappedNormalYX);
    std::cout << "Conclusion: Swapping vectors negates the output unit vector (flips direction by 180 deg)\n"
              << "because the cross product is anti-commutative: u x v = -(v x u).\n";
}

void exercise5()
{
    std::cout << "\n--- Exercise 5: 2D Square Matrix Encoding & Transformations ---\n";
    std::cout << "Question 1: How to encode the center unit square via a matrix?\n";
    std::cout << "Answer: As a 4x4 matrix where each column represents a vertex [x, y, z, 1]^T in homogeneous coordinates:\n";
    std::cout << "  Col 1: Vertex A = [-0.5,  0.5, 0, 1]^T  (Top-Left)\n";
    std::cout << "  Col 2: Vertex B = [ 0.5,  0.5, 0, 1]^T  (Top-Right)\n";
    std::cout << "  Col 3: Vertex C = [ 0.5, -0.5, 0, 1]^T  (Bottom-Right)\n";
    std::cout << "  Col 4: Vertex D = [-0.5, -0.5, 0, 1]^T  (Bottom-Left)\n\n";

    // Encode unit square centered at origin with side 1 as a 4x4 matrix
    const Mat4 centerSquare(
        glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f), // Col 0: Vertex A
        glm::vec4( 0.5f,  0.5f, 0.0f, 1.0f), // Col 1: Vertex B
        glm::vec4( 0.5f, -0.5f, 0.0f, 1.0f), // Col 2: Vertex C
        glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f)  // Col 3: Vertex D
    );
    printSquareMatrix("Center unit square matrix S (columns are vertices A, B, C, D):", centerSquare);

    std::cout << "\nQuestion 2 & 3: Sequence of transformations to output remaining squares:\n";
    std::cout << "1. Upper Square (center at (2, 1), side length 1):\n";
    std::cout << "   - Diagram shows vertex A' at the top (0, 0.7071), B' at right, C' at bottom, D' at left.\n";
    std::cout << "   - To move A from 135 deg to 90 deg, rotate by -45 deg (clockwise).\n";
    std::cout << "   - Then translate by (2, 1, 0).\n";
    std::cout << "   - Transformation: M_upper = T(2, 1, 0) * Rz(-45 deg)\n";

    const Mat4 upperTransform = glm::translate(Mat4(1.0f), Vec3(2.0f, 1.0f, 0.0f))
        * glm::rotate(Mat4(1.0f), glm::radians(-45.0f), Vec3(0.0f, 0.0f, 1.0f));
    const Mat4 upperSquare = upperTransform * centerSquare;

    printMatrix("\nUpper Transformation Matrix M_upper:", upperTransform);
    printSquareMatrix("Transformed Upper Square (M_upper * S):", upperSquare);

    std::cout << "\n2. Lower Square (center at (-2, -2), side length 2):\n";
    std::cout << "   - Diagram shows vertex C'' at top-left, D'' at top-right, A'' at bottom-right, B'' at bottom-left.\n";
    std::cout << "   - Side length increases from 1 to 2 -> Scale S(2, 2, 1).\n";
    std::cout << "   - Vertex positions are inverted (A -> bottom-right, B -> bottom-left) -> Rotate Rz(180 deg).\n";
    std::cout << "   - Then translate by (-2, -2, 0).\n";
    std::cout << "   - Transformation: M_lower = T(-2, -2, 0) * Rz(180 deg) * S(2, 2, 1)\n";

    const Mat4 lowerTransform = glm::translate(Mat4(1.0f), Vec3(-2.0f, -2.0f, 0.0f))
        * glm::rotate(Mat4(1.0f), glm::radians(180.0f), Vec3(0.0f, 0.0f, 1.0f))
        * glm::scale(Mat4(1.0f), Vec3(2.0f, 2.0f, 1.0f));
    const Mat4 lowerSquare = lowerTransform * centerSquare;

    printMatrix("\nLower Transformation Matrix M_lower:", lowerTransform);
    printSquareMatrix("Transformed Lower Square (M_lower * S):", lowerSquare);
}

int main()
{
    std::mt19937 generator(2026);
    std::uniform_real_distribution<float> distribution(-5.0f, 5.0f);
    const auto randomVector = [&]
    {
        return Vec3(distribution(generator), distribution(generator), distribution(generator));
    };

    std::cout << "Exercise 1: cross product distributivity\n";
    const Vec3 u = randomVector();
    const Vec3 v = randomVector();
    const Vec3 w = randomVector();
    printVector("u", u);
    printVector("v", v);
    printVector("w", w);
    crossProductIsDistributive(u, v, w);

    std::cout << "\nExercise 2: rotation matrices\n";
    exercise2();

    std::cout << "\nExercise 3: rotations are not commutative\n";
    rotationsAreNonCommutative(randomVector());

    std::cout << "\nExercise 4: orthogonal vectors\n";
    exercise4();

    std::cout << "\nExercise 5: square transformations\n";
    exercise5();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
