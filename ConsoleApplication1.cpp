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

void printSquare(const std::string& name, const Mat4& model)
{
    const glm::vec4 square[] = {
        glm::vec4(-0.5f, 0.5f, 0.0f, 1.0f),
        glm::vec4(0.5f, 0.5f, 0.0f, 1.0f),
        glm::vec4(0.5f, -0.5f, 0.0f, 1.0f),
        glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f)
    };

    std::cout << name << "\n";
    for (int i = 0; i < 4; ++i)
        printVector("  vertex " + std::to_string(i + 1), Vec3(model * square[i]));
}

void exercise5()
{
    const Mat4 centerSquare(1.0f);
    const Mat4 upperSquare = glm::translate(Mat4(1.0f), Vec3(2.0f, 1.0f, 0.0f))
        * glm::rotate(Mat4(1.0f), glm::radians(45.0f), Vec3(0.0f, 0.0f, 1.0f));
    const Mat4 lowerSquare = glm::translate(Mat4(1.0f), Vec3(-2.0f, -2.0f, 0.0f))
        * glm::scale(Mat4(1.0f), Vec3(2.0f, 2.0f, 1.0f));

    printMatrix("Center unit square matrix", centerSquare);
    printSquare("Center square vertices", centerSquare);
    printMatrix("Upper square matrix: T(2, 1) * Rz(45 degrees)", upperSquare);
    printSquare("Upper square vertices", upperSquare);
    printMatrix("Lower square matrix: T(-2, -2) * S(2)", lowerSquare);
    printSquare("Lower square vertices", lowerSquare);
}

void exercise4()
{
    const Vec3 a(1.0f, 2.0f, 3.0f);
    const Vec3 b(4.0f, 5.0f, 6.0f);
    const Vec3 normal = orthogonalVector(a, b);

    printVector("a x b", glm::cross(a, b));
    printVector("Unit normal", normal);
    std::cout << "a dot normal: " << glm::dot(a, normal) << "\n";
    std::cout << "b dot normal: " << glm::dot(b, normal) << "\n";
    std::cout << "Orthogonal to both: " << std::boolalpha << areOrthogonal(a, b, normal) << "\n";

    const Vec3 x(1.0f, 0.0f, 0.0f);
    const Vec3 y(0.0f, 1.0f, 0.0f);
    printVector("X cross Y", orthogonalVector(x, y));
    printVector("Y cross X", orthogonalVector(y, x));
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
