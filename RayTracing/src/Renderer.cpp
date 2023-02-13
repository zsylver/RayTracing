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

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	Ray ray;
	ray.Origin = camera.GetPosition();

	//render every pixel
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { x / (float)m_FinalImage->GetWidth(), y / (float)m_FinalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f; // -1 -> 1
			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];

			glm::vec4 color = TraceRay(scene, ray);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

	// Upload to the GPU
	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{
	glm::vec3 lightPos(-1.f, -1.f, -1.f);

	//rayDir = glm::normalize(rayDir);

	// Quadratic equation
	// (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
	// a = ray origin
	// b = ray direction
	// r = radius
	// t = hit distance

	if (scene.Spheres.size() == 0)
		return glm::vec4(0, 0, 0, 1);

	const Sphere* closestSphere = nullptr;
	float hitDistance = FLT_MAX;
	const Sphere& sphere = scene.Spheres[0];

	for (const Sphere& sphere : scene.Spheres)
	{
		glm::vec3 origin = ray.Origin - sphere.Position;

		// Quadratic formula's a b and c
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2 * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		// Check the discriminant for solutions
		// b^2 - 4ac

		float discriminant = b * b - 4 * a * c;
		if (discriminant < 0.0f)
			continue;


		float t0 = (-b - sqrtf(discriminant)) / (2.0f * a);
		//float t1 = (-b + sqrtf(discriminant)) / (2.0f * a);
		if (t0 < hitDistance)
		{
			hitDistance = t0;
			closestSphere = &sphere;
		}
	}

	if (closestSphere == nullptr)
		return glm::vec4(0, 0, 0, 1);

	glm::vec3 origin = ray.Origin - closestSphere->Position;
	// p + tv
	glm::vec3 hitPosition = origin + ray.Direction * hitDistance;

	// Calculate normal
	glm::vec3 normal = hitPosition; //- sphere.Position;
	normal = glm::normalize(normal);

	glm::vec3 lightDir(-1.f, -1.f, -1.f);
	lightDir = glm::normalize(lightDir);
	float lightIntensity = std::max(glm::dot(normal, -lightDir), 0.0f);

	// map -0.3f to 0.f to 1.f
	//light = (light - -0.3f) / (1.0f - -0.3f);

	glm::vec3 sphereColor = closestSphere->Albedo;
	//sphereColor = Walnut::Random::Vec3(0.f, 1.f);
	//sphereColor = normal * 0.5f + 0.5f;
	sphereColor *= lightIntensity;
	return glm::vec4(sphereColor, 1.f);

}
