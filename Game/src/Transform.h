#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Transform
{
public:
	void computeModelMatrix()
	{
		modelMatrix = getLocalModelMatrix();
		isDirty = false;
	}

	void computeModelMatrix(const glm::mat4& parentGlobalModelMatrix)
	{
		modelMatrix = parentGlobalModelMatrix * getLocalModelMatrix();
		isDirty = false;
	}

	void setLocalPosition(const glm::vec3& newPosition)
	{
		pos = newPosition;
		isDirty = true;
	}

	void setLocalRotation(const glm::quat& newRotation)
	{
		quatRot = newRotation;
		isDirty = true;
	}

	void setLocalScale(const glm::vec3& newScale)
	{
		scale = newScale;
		isDirty = true;
	}

	const glm::vec3& getGlobalPosition() const { return modelMatrix[3]; }
	const glm::vec3& getLocalPosition() const { return pos; }
	const glm::quat& getLocalRotation() const { return quatRot; }
	const glm::vec3& getLocalScale() const { return scale; }

	const glm::mat4& getModelMatrix() const { return modelMatrix; }

	glm::vec3 getRight() const { return modelMatrix[0]; }
	glm::vec3 getUp() const { return modelMatrix[1]; }
	glm::vec3 getForward() const { return -modelMatrix[2]; }
	glm::vec3 getBack() const { return modelMatrix[2]; }

	glm::vec3 getGlobalScale() const { return { glm::length(getRight()), glm::length(getUp()), glm::length(getForward()) }; }

	bool getIsDirty() const { return isDirty; }

protected:
	glm::vec3 pos = glm::vec3(0.0f);
	glm::quat quatRot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	glm::mat4 modelMatrix = glm::mat4(1.0f);

	bool isDirty = true;

	glm::mat4 getLocalModelMatrix()
	{
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), pos);
		glm::mat4 rotationMatrix = glm::mat4_cast(quatRot);
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
		return translationMatrix * rotationMatrix * scaleMatrix;
	}
};