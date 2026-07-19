#include "Math4D.h"

// Math functions for the 4D module that don't fit in any other file.
// Prefer using UGeometry4D, UVectorFuncs4D, etc if those are appropriate.

double UMath4D::Float8ToDouble(const uint8 InFloat8)
{
	const uint8 f8Exponent = (InFloat8 >> 3) & 0xF;
	const uint8 f8Mantissa = InFloat8 & 0x7;
	const uint64 f64Sign = uint64(InFloat8 & 0x80) << 56;
	uint64 f64Exponent = 0;
	uint64 f64Mantissa = 0;
	if (f8Exponent == 0)
	{
		if (f8Mantissa != 0)
		{
			// Subnormal: real exponent is -6 so multiply 3-bit mantissa by 2^-9.
			const double value = f8Mantissa * 0.001953125;
			return (f64Sign == 0) ? value : -value;
		}
	}
	else if (f8Exponent == 0xF)
	{
		// Infinity or NaN: Exponent is maxed out.
		f64Exponent = uint64(0x7FF) << 52;
		f64Mantissa = uint64(f8Mantissa) << (52 - 3);
	}
	else
	{
		// Normalized number.
		const uint64 f8RealExp = uint64(f8Exponent) - 7; // float8 bias is 7.
		const uint64 f64BiasedExp = uint64(f8RealExp + 1023); // double bias is 1023.
		f64Exponent = f64BiasedExp << 52;
		f64Mantissa = uint64(f8Mantissa) << (52 - 3); // Convert 3-bit mantissa to 52-bit.
	}
	union
	{
		double D;
		uint64 Bits;
	} U;
	U.Bits = f64Sign | f64Exponent | f64Mantissa;
	return U.D;
}

double UMath4D::Float16ToDouble(const uint16 InFloat16)
{
	const uint16 f16Exponent = (InFloat16 >> 10) & 0x1F;
	const uint16 f16Mantissa = InFloat16 & 0x3FF;
	const uint64 f64Sign = uint64(InFloat16 & 0x8000) << 48;
	uint64 f64Exponent = 0;
	uint64 f64Mantissa = 0;
	if (f16Exponent == 0)
	{
		if (f16Mantissa != 0)
		{
			// Subnormal: real exponent is -14 so multiply 10-bit mantissa by 2^-24.
			const double value = f16Mantissa * 0.000000059604644775390625;
			return (f64Sign == 0) ? value : -value;
		}
	}
	else if (f16Exponent == 0x1F)
	{
		// Infinity or NaN.
		f64Exponent = uint64(0x7FF) << 52;
		f64Mantissa = uint64(f16Mantissa) << (52 - 10);
	}
	else
	{
		// Normalized number.
		const uint64 f16RealExp = uint64(f16Exponent) - 15; // float16 bias is 15.
		const uint64 f64BiasedExp = f16RealExp + 1023; // double bias is 1023.
		f64Exponent = f64BiasedExp << 52;
		f64Mantissa = uint64(f16Mantissa) << (52 - 10); // Convert 10-bit mantissa to 52-bit.
	}
	union
	{
		double D;
		uint64 Bits;
	} U;
	U.Bits = f64Sign | f64Exponent | f64Mantissa;
	return U.D;
}

uint8 UMath4D::DoubleToFloat8(const double InDouble)
{
	union
	{
		double D;
		uint64 Bits;
	} U;
	U.D = InDouble;
	const uint64 f64Exponent = (U.Bits >> 52) & 0x7FF;
	const uint64 f64Mantissa = U.Bits & ((uint64(1) << 52) - 1);
	const uint8 f8Sign = (U.Bits >> 56) & 0x80;
	if (f64Exponent == 0x7FF)
	{
		// Infinity or NaN.
		uint8 f8Mantissa = f64Mantissa >> 49; // Get the top 3 bits of the mantissa.
		// Special case: NaN values must be preserved as NaN.
		if (f8Mantissa == 0 && f64Mantissa != 0)
		{
			f8Mantissa = 0x1;
		}
		return f8Sign | uint8(0xF << 3) | f8Mantissa;
	}
	const int32 f64RealExp = int32(f64Exponent) - 1023;
	if (f64RealExp > 8)
	{
		// Overflow: too large, becomes Infinity.
		return f8Sign | uint8(0xF << 3);
	}
	if (f64RealExp < -6)
	{
		// Subnormal or underflow to zero (exponent becomes zero).
		if (f64RealExp < -9)
		{
			// Too small, flush to zero (or negative zero).
			return f8Sign;
		}
		// Convert to subnormal.
		const int32 shift = (-6 - f64RealExp);
		const uint64 f64MantissaBits = (uint64(1) << 52) | f64Mantissa;
		const uint64 roundBit = (f64MantissaBits >> (52 + shift - 4)) & 1;
		uint64 f8MantissaBits = f64MantissaBits >> (52 + shift - 3); // 3-bit mantissa.
		// Round and clamp.
		f8MantissaBits += roundBit;
		if (f8MantissaBits > 0x7)
		{
			f8MantissaBits = 0x7;
		}
		return f8Sign | uint8(f8MantissaBits);
	}
	// Normal case.
	const uint64 roundBit = (f64Mantissa >> (52 - 4)) & 1;
	uint8 f8Exponent = uint8(f64RealExp + 7); // Re-bias to float8.
	uint64 f8MantissaBits = f64Mantissa >> (52 - 3);
	f8MantissaBits += roundBit;
	// If the mantissa overflows, we need to increment the exponent.
	if (f8MantissaBits > 0x7)
	{
		f8MantissaBits = 0;
		f8Exponent += 1;
		// Overflow may go to Infinity.
		if (f8Exponent >= 0xF)
		{
			return f8Sign | uint8(0xF << 3);
		}
	}
	return f8Sign | uint8(f8Exponent << 3) | uint8(f8MantissaBits);
}

uint16 UMath4D::DoubleToFloat16(const double InDouble)
{
	union
	{
		double D;
		uint64 Bits;
	} U;
	U.D = InDouble;
	const uint64 f64Exponent = (U.Bits >> 52) & 0x7FF;
	const uint64 f64Mantissa = U.Bits & ((uint64(1) << 52) - 1);
	const uint16 f16Sign = (U.Bits >> 48) & 0x8000;
	if (f64Exponent == 0x7FF)
	{
		// Infinity or NaN.
		uint16 f16Mantissa = f64Mantissa >> 42; // Get the top 10 bits of the mantissa.
		// Special case: NaN values must be preserved as NaN.
		if (f16Mantissa == 0 && f64Mantissa != 0)
		{
			f16Mantissa = 0x1;
		}
		return f16Sign | uint16(0x1F << 10) | f16Mantissa;
	}
	const int32 f64RealExp = int32(f64Exponent) - 1023;
	if (f64RealExp > 15)
	{
		// Overflow: too large, becomes Infinity.
		return f16Sign | uint16(0x1F << 10);
	}
	if (f64RealExp < -14)
	{
		// Subnormal or underflow to zero.
		if (f64RealExp < -25)
		{
			// Too small, flush to zero.
			return f16Sign;
		}
		// Convert to subnormal.
		const int32 shift = (-14 - f64RealExp);
		const uint64 f64MantissaBits = (uint64(1) << 52) | f64Mantissa;
		const uint64 roundBit = (f64MantissaBits >> (52 + shift - 11)) & 1;
		uint64 f16MantissaBits = f64MantissaBits >> (52 + shift - 10); // 10-bit mantissa.
		// Round and clamp.
		f16MantissaBits += roundBit;
		if (f16MantissaBits > 0x3FF)
		{
			f16MantissaBits = 0x3FF;
		}
		return f16Sign | uint16(f16MantissaBits);
	}
	// Normal case.
	const uint64 roundBit = (f64Mantissa >> (52 - 11)) & 1;
	uint16 f16Exponent = uint16(f64RealExp + 15); // Re-bias to float16.
	uint64 f16MantissaBits = f64Mantissa >> (52 - 10);
	f16MantissaBits += roundBit;
	// If the mantissa overflows, increment exponent.
	if (f16MantissaBits > 0x3FF)
	{
		f16MantissaBits = 0;
		f16Exponent += 1;
		if (f16Exponent >= 0x1F)
		{
			return f16Sign | uint16(0x1F << 10);
		}
	}
	return f16Sign | uint16(f16Exponent << 10) | uint16(f16MantissaBits);
}

FLinearColor UMath4D::ColorLightened(const FLinearColor InColor, const float InAmount) {
	return FLinearColor(
		InColor.R + (1.0f - InColor.R) * InAmount,
		InColor.G + (1.0f - InColor.G) * InAmount,
		InColor.B + (1.0f - InColor.B) * InAmount,
		InColor.A
	);
}

double UMath4D::Vector2_Angle(const FVector2f& InVector)
{
	return FMath::Atan2((double)InVector.Y, (double)InVector.X);
}

double UMath4D::Vector2_AngleToPoint(const FVector2f& InFrom, const FVector2f& InTo)
{
	return Vector2_Angle(InFrom - InTo);
}

int32 UMath4D::FindCommonInt32(const TArray<int32>& InA, const TArray<int32>& InB, int32& OutAIndex, int32& OutBIndex)
{
	for (int32 AIndex = 0; AIndex < InA.Num(); ++AIndex)
	{
		const int32 aItem = InA[AIndex];
		for (int32 BIndex = 0; BIndex < InB.Num(); ++BIndex)
		{
			const int32 bItem = InB[BIndex];
			if (aItem == bItem)
			{
				OutAIndex = AIndex;
				OutBIndex = BIndex;
				return aItem;
			}
		}
	}
	return INT32_MIN;
}
