#pragma once
#include"utility.h"
using value_type = unsigned long long;
class Morton64 {
public:
	value_type		m_Value;
	template<unsigned int precision>
	void Encode(float x, float y, float z, value_type lastBits)
	{
		static_assert(precision <= 21, "The highest precision for 64-bit Morton code is 21.");

		x = Utility::clamp(x * (1ll << precision), 0.0f, (1ll << precision) - 1.0f);
		y = Utility::clamp(y * (1ll << precision), 0.0f, (1ll << precision) - 1.0f);
		z = Utility::clamp(z * (1ll << precision), 0.0f, (1ll << precision) - 1.0f);

		value_type xx = ExpandBits(static_cast<value_type>(x)) << (66 - 3 * precision);
		value_type yy = ExpandBits(static_cast<value_type>(y)) << (65 - 3 * precision);
		value_type zz = ExpandBits(static_cast<value_type>(z)) << (64 - 3 * precision);

		constexpr value_type bitMask = ~value_type(0) >> (3 * precision);

		m_Value = xx + yy + zz + (lastBits & bitMask);
	}
	void Encode(float x, float y, float z)
	{
		float x1 = Utility::clamp(x * (1ll << 21), 0.0f, (1ll << 21) - 1.0f);
		float y1 = Utility::clamp(y * (1ll << 21), 0.0f, (1ll << 21) - 1.0f);
		float z1 = Utility::clamp(z * (1ll << 21), 0.0f, (1ll << 21) - 1.0f);

		value_type xx = ExpandBits(static_cast<value_type>(x1));
		value_type yy = ExpandBits(static_cast<value_type>(y1));
		value_type zz = ExpandBits(static_cast<value_type>(z1));
		//cout << x << " " << y << " " << z << endl;
		m_Value = (xx << 2) + (yy << 1) + zz;
	}
private:
	
	value_type ExpandBits(value_type bits)
	{
		bits = (bits | (bits << 32)) & 0xFFFF00000000FFFFu;
		bits = (bits | (bits << 16)) & 0x00FF0000FF0000FFu;
		bits = (bits | (bits << 8)) & 0xF00F00F00F00F00Fu;
		bits = (bits | (bits << 4)) & 0x30C30C30C30C30C3u;
		return (bits | (bits << 2)) & 0x9249249249249249u;
	}
};