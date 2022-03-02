#include <glm/gtc/quaternion.hpp>
#include <glm/exponential.hpp>

// this function is copied from the glm source on git which is using an MIT license
// i had to do this because the feng-linux machines are running an older
// version of glm which doesnt contain the quat look at function

namespace Quat
{
    glm::quat LookAt( const glm::vec3& direction,const glm::vec3& up)
    {
        glm::mat3 Result;

		Result[2] = -direction;
		glm::vec3 const& Right = glm::cross(up, Result[2]);
		Result[0] = Right * glm::inversesqrt(glm::max(static_cast<glm::vec3>(0.00001), glm::dot(Right, Right)));
		Result[1] = cross(Result[2], Result[0]);

		return quat_cast(Result);
    }

    glm::quat quat_cast( const glm::mat3& m )
    {
        float fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
		float fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
		float fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
		float fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

		int biggestIndex = 0;
		float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
		if(fourXSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourXSquaredMinus1;
			biggestIndex = 1;
		}
		if(fourYSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourYSquaredMinus1;
			biggestIndex = 2;
		}
		if(fourZSquaredMinus1 > fourBiggestSquaredMinus1)
		{
			fourBiggestSquaredMinus1 = fourZSquaredMinus1;
			biggestIndex = 3;
		}

		float biggestVal = sqrt(fourBiggestSquaredMinus1 + static_cast<float>(1)) * static_cast<float>(0.5);
		float mult = static_cast<float>(0.25) / biggestVal;

		switch(biggestIndex)
		{
		case 0:
			return glm::quat(biggestVal, (m[1][2] - m[2][1]) * mult, (m[2][0] - m[0][2]) * mult, (m[0][1] - m[1][0]) * mult);
		case 1:
			return glm::quat((m[1][2] - m[2][1]) * mult, biggestVal, (m[0][1] + m[1][0]) * mult, (m[2][0] + m[0][2]) * mult);
		case 2:
			return glm::quat((m[2][0] - m[0][2]) * mult, (m[0][1] + m[1][0]) * mult, biggestVal, (m[1][2] + m[2][1]) * mult);
		case 3:
			return glm::quat((m[0][1] - m[1][0]) * mult, (m[2][0] + m[0][2]) * mult, (m[1][2] + m[2][1]) * mult, biggestVal);
		default: // Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
			assert(false);
			return glm::quat(1, 0, 0, 0);
		}
    }
}