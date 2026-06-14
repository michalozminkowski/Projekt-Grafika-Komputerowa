#include "Camera.h"

glm::mat4 Core::createPerspectiveMatrix(float zNear, float zFar, float frustumScale)
{
	glm::mat4 perspective(0.0f);
    perspective[0][0] = 1.f;
	perspective[1][1] = frustumScale;
	perspective[2][2] = (zFar + zNear) / (zNear - zFar);
	perspective[3][2] = (2 * zFar * zNear) / (zNear - zFar);
	perspective[2][3] = -1;
	perspective[3][3] = 0;

	return perspective;
}

glm::mat4 Core::createViewMatrix( glm::vec3 position, glm::vec3 forward, glm::vec3 up )
{
	glm::vec3 f = glm::normalize(forward);
	glm::vec3 s = glm::normalize(glm::cross(f, up));
	glm::vec3 u = glm::cross(s, f);

	glm::mat4 cameraRotation(1.0f);
	cameraRotation[0][0] = s.x; cameraRotation[1][0] = s.y; cameraRotation[2][0] = s.z;
	cameraRotation[0][1] = u.x; cameraRotation[1][1] = u.y; cameraRotation[2][1] = u.z;
	cameraRotation[0][2] = -f.x; cameraRotation[1][2] = -f.y; cameraRotation[2][2] = -f.z;

	glm::mat4 cameraTranslation(1.0f);
	cameraTranslation[3] = glm::vec4(-position, 1.0f);

	return cameraRotation * cameraTranslation;
}
