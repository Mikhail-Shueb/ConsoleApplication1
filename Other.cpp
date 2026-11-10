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

const float THRESHOLD = (float)1.0e-4;

void ex1();
void ex2();
bool vectorsEqual(const vec3& first, const vec3& second);
vec3 generateVector();

int main()
{
	ex1();
	return 0;
}

vec3 generateVector() {
	static mt19937 generator(2026);
	uniform_real_distribution<float> distribution(1.0f, 5.0f);
	return vec3(distribution(generator), distribution(generator), distribution(generator));
}

// i × (j × k) = j(i ⋅ k) − k(i ⋅ j)

void ex1() {
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
	std::cout << "Equal: " << std::boolalpha << equal << "\n";
}

bool vectorsEqual(const vec3& first, const vec3& second)
{
	return glm::length(first - second) < THRESHOLD;
}


//void ex2() {
//	vec3 u(1.0f, 1.0f, 1.0f);
//	vec3 v(-1.0f, -1.0f, 0.0f);
//	vec3 w(1.0f, 0.0f, 0.0f);
//	vec3 u = generateVector();
//	vec3 v = generateVector();
//	vec3 w = generateVector();
//
//	cout << "u: " << u.x << ", " << u.y << ", " << u.z << endl;
//	cout << "v: " << v.x << ", " << v.y << ", " << v.z << endl;
//	cout << "w: " << w.x << ", " << w.y << ", " << w.z << endl;
//
//	vec3 v_plus_w = v + w;
//	vec3 left = cross(u, v_plus_w);
//	vec3 right = cross(u, v) + cross(u, w);
//
//	cout << "u x (v + w): " << left.x << ", " << left.y << ", " << left.z << endl;
//	cout << "u x v + u x w: " << right.x << ", " << right.y << ", " << right.z << endl;
//
//}