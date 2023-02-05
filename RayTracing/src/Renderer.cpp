#include "Renderer.h"
#include "Walnut/Random.h"

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
			m_ImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(coord);
		}
	}

	// Upload to the GPU
	m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	uint8_t r = (uint8_t)(coord.x * 255.0f);
	uint8_t g = (uint8_t)(coord.y * 255.0f);

	glm::vec3 rayOrigin(0.0f,0.0f, 2.0f);
	glm::vec3 rayDir(coord.x, coord.y, -1.0f);

	//glm::vec3 dirLight(-100.f, -100.f, 0.f);

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

	if (discriminant >= 0.0f)
		return 0xffff00ff;

	return 0xff000000;
}
