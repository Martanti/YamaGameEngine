export module YaMath;
import <math.h>;
import <string>;
import <format>;
// Short for Yama math

/**
 * @brief Yama Engine math library, meant to be used within game logic and transferring data between different middleware.
*/
namespace yamath
{
	/**
	 * @brief A most basic XYZ Vector.
	*/
	export struct Vec3D
	{
		float X;
		float Y;
		float Z;
	};

	/**
	 * @brief A namespace containing mathematical operations.
	*/
	namespace opr
	{
		// RVO should take care of the returns, thus no & or const& or const.

		/**
		 * @brief Converts a tuple with XYZ coordinates into a Vec3D.
		*/
		export inline Vec3D FromTupleXYZ(std::tuple<float, float, float> tpl) noexcept
		{
			auto [x, y, z] = tpl;
			return { .X = x, .Y = y, .Z = z };
		}

		/**
		 * @brief Add two vectors together.
		*/
		export inline Vec3D Add(Vec3D const& a, Vec3D const& b) noexcept
		{
			return Vec3D{ .X = a.X + b.X, .Y = a.Y + b.Y, .Z = a.Z + b.Z};
		}

		/**
		 * @brief Subtract two vectors.
		*/
		export inline Vec3D Sub(Vec3D const& a, Vec3D const& b) noexcept
		{
			return { .X = a.X - b.X, .Y = a.Y - b.Y, .Z = a.Z - b.Z };
		}

		/**
		 * @brief Multiplication between a vector and a scalar.
		*/
		export inline Vec3D Mult(Vec3D const& a, float const& t) noexcept
		{
			return { .X = a.X * t, .Y = a.Y * t, .Z = a.Z * t };
		}

		/**
		 * @brief Divide a vector by a scalar. It divided by zero will return the same vector as provided.
		*/
		export inline Vec3D Div(Vec3D const& a, float const& t) noexcept
		{
			// This is to prevent the exceptions and that the noexcept would allow the optimizations
			if (t == 0)
				return { a };

			auto inversed = 1.f / t;
			return { .X = a.X * inversed, .Y = a.Y * inversed, .Z = a.Z * inversed };
		}

		/**
		 * @brief Get a magnitude/length of a vector.
		*/
		export inline float Length(Vec3D const& a) noexcept
		{
			auto& x = a.X;
			auto& y = a.Y;
			auto& z = a.Z;
			return sqrt((x * x) + (y * y) + (z * z));
		}

		/**
		 * @brief Normalize a vector.
		*/
		export inline Vec3D Normalize(Vec3D const& a) noexcept
		{
			auto length = Length(a);
			if (length == 0)
				return { 0, 0, 0 };
			auto inversedLength = 1 / length;
			return { .X = a.X * inversedLength, .Y = a.Y * inversedLength, .Z = a.Z * inversedLength };
		}

		/**
		 * @brief Check if two vectors have the same values.
		*/
		export inline bool Equal(Vec3D const& a, Vec3D const& b) noexcept
		{
			return a.X == b.X && a.Y == b.Y && a.Z == b.Z;
		}

		/**
		 * @brief Calculate a dot product between two vectors.
		*/
		export inline float Dot(Vec3D const& a, Vec3D const& b) noexcept
		{
			return { a.X * b.X + a.Y * b.Y + a.Z * b.Z };
		}

		/**
		 * @brief Calculate a cross product between two vectors.
		*/
		export inline Vec3D Cross(Vec3D const& a, Vec3D const& b) noexcept
		{
			// I think cacnhin the references will reduce the (already small) time to navigate through the object to get the var time
			auto& ax = a.X;
			auto& ay = a.Y;
			auto& az = a.Z;

			auto& bx = b.X;
			auto& by = b.Y;
			auto& bz = b.Z;
			return {
				.X = ay * bz - az * by,
				.Y = az * bx - ax * bz,
				.Z = ax * by - ay * bx };
		}

		/**
		 * @brief Get a string with vector XYZ in it. Meant for debugging.
		*/
		export inline std::string ToString(Vec3D const& a) noexcept
		{
			return std::format("{} {} {}", a.X, a.Y, a.Z);
		}

		/**
		 * @brief Advance single line string, containing the XYZ axis and appropriate values.
		*/
		export inline std::string ToStringASL(Vec3D const& a) noexcept
		{
			return std::format("X: {}, Y: {}, Z: {}", a.X, a.Y, a.Z);
		}

		/**
		 *@brief Linear interpolation between two vectors. From b to a.

		 Based on (a * step) + (b * (1 - step)) calculation. Limits the step between 0 and 1.
		*/
		export inline Vec3D Lerp(Vec3D const& a, Vec3D const& b, float step) noexcept
		{
			if (Equal(a, b))
				return a;

			if (step > 1)
				step = 1;

			else if (step < 0)
				step = 0;

			// Should be faster than just the if checks

			if (step == 1.f)
				return a;

			else if (step == 0.f)
				return b;

			return Add(Mult(a, step), Mult(b, 1-step));
		}

		/**
		 * @brief PI constant.
		 * @note Meant as an internal variable.
		*/
		constexpr inline float PI = 3.14159f;

		/**
		 * @brief A 180 degrees constant.
		 * @note Meant as an internal variable.
		*/
		constexpr inline float HalfCircleDegrees = 180.f;

		/**
		 * @brief Coefficient for converting degrees to radians.
		 * @note Meant as an internal variable.
		*/
		constexpr inline float ToRadianCoeficient = PI / HalfCircleDegrees;

		/**
		 * @brief Coefficient for converting radians to degrees.
		 * @note Meant as an internal variable.
		*/
		constexpr inline float ToDegreesCoeficient = HalfCircleDegrees / PI;

		/**
		 * @brief Convert an angle in degrees to radians.
		*/
		export inline float ToRadians(float const& angleInDegrees) noexcept
		{
			return angleInDegrees * ToRadianCoeficient;
		}

		/**
		 * @brief Convert an angle in radians to degrees.
		*/
		export inline float ToDegrees(float const& angleInRadians) noexcept
		{
			return angleInRadians * ToDegreesCoeficient;
		}
	}
}