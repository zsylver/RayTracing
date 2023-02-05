#include "Renderer.h"
#include "Walnut/Random.h"
#include <algorithm>

namespace Utils
{
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
		return result;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// No resize necessary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height) return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render()
{
	//render every pixel
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { x / (float)m_FinalImage->GetWidth(), y / (float)m_FinalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f; // -1 -> 1

			glm::vec4 color = PerPixel(coord);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

	// Upload to the GPU
	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
	glm::vec3 rayOrigin(0.0f,0.0f, 2.0f);
	glm::vec3 rayDir(coord.x, coord.y, -1.0f);

	glm::vec3 spherePos(0.0f);
	glm::vec3 lightPos(-1.f, -1.f, -1.f);

	float radius = 0.5f;
	//rayDir = glm::normalize(rayDir);

	// Quadratic equation
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance


	// Quadratic formula's a b and c
	float a = glm::dot(rayDir, rayDir);
	float b = 2 * glm::dot(rayOrigin, rayDir);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	// Check the discriminant for solutions
	// b^2 - 4ac

	float discriminant = b * b - 4 * a * c;
	if (discriminant < 0.0f)
		return glm::vec4(0, 0, 0, 1);


	float t0 = (-b - sqrtf(discriminant)) / (2.0f * a);
	//float t1 = (-b + sqrtf(discriminant)) / (2.0f * a);
		
	// p + tv
	glm::vec3 hitPosition = rayOrigin + rayDir * t0;

	// Calculate normal
	glm::vec3 normal = hitPosition - spherePos;
	normal = glm::normalize(normal);

	glm::vec3 lightDir(-1.f, -1.f, -1.f);
	lightDir = glm::normalize(lightDir);
	float lightIntensity = std::max(glm::dot(normal, -lightDir), 0.0f);

	// map -0.3f to 0.f to 1.f
	//light = (light - -0.3f) / (1.0f - -0.3f);

	glm::vec3 sphereColor(0.0f);
	//sphereColor = Walnut::Random::Vec3(0.f, 1.f);
	sphereColor = normal * 0.5f + 0.5f;
	sphereColor *= lightIntensity;
	return glm::vec4(sphereColor, 1.f);

}
