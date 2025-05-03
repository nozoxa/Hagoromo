// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#pragma once

#include "Math/Vector.h"
#include "Math/VectorRegister.h"
#include "Math/UnrealMathVectorCommon.h"

#if HGM_USE_FLOAT32
using FHGMReal = float;
#else
using FHGMReal = double;
#endif

using FHGMVector3 = FVector;
using FHGMTransform = FTransform;
using FHGMQuaternion = FQuat;
using FHGMRotator = FRotator;
using FHGMMatrix = FMatrix;


// ---------------------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------------------
namespace HGMSIMDConstants
{
#if HGM_USE_SIMD_REGISTER_FLOAT4X64
	inline constexpr VectorRegister4Double ZeroReal = GlobalVectorConstants::DoubleZero;
	inline constexpr VectorRegister4Double OneReal = GlobalVectorConstants::DoubleOne;
	inline constexpr VectorRegister4Double TwoReal = GlobalVectorConstants::DoubleTwo;
	inline constexpr VectorRegister4Double OneHalfReal = GlobalVectorConstants::DoubleOneHalf;
	inline constexpr VectorRegister4Double SmallReal = GlobalVectorConstants::DoubleSmallNumber;
	inline constexpr VectorRegister4Double BigReal = GlobalVectorConstants::DoubleBigNumber;
	const VectorRegister4Double AllBitMask = MakeVectorRegisterDouble(~uint32(0), ~uint32(0), ~uint32(0), ~uint32(0));
	inline constexpr VectorRegister4Double RadiansToDegrees = GlobalVectorConstants::DOUBLE_RAD_TO_DEG;
	inline constexpr VectorRegister4Double DegreesToRadians = GlobalVectorConstants::DOUBLE_DEG_TO_RAD;
#else
	inline constexpr VectorRegister4Float ZeroReal = GlobalVectorConstants::FloatZero;
	inline constexpr VectorRegister4Float OneReal = GlobalVectorConstants::FloatOne;
	inline constexpr VectorRegister4Float TwoReal = GlobalVectorConstants::FloatTwo;
	inline constexpr VectorRegister4Float OneHalfReal = GlobalVectorConstants::FloatOneHalf;
	inline constexpr VectorRegister4Float SmallReal = GlobalVectorConstants::SmallNumber;
	inline constexpr VectorRegister4Float BigReal = GlobalVectorConstants::BigNumber;
	const VectorRegister4Float AllBitMask = MakeVectorRegisterFloat(~uint32(0), ~uint32(0), ~uint32(0), ~uint32(0));
	inline constexpr VectorRegister4Float RadiansToDegrees = GlobalVectorConstants::RAD_TO_DEG;
	inline constexpr VectorRegister4Float DegreesToRadians = GlobalVectorConstants::DEG_TO_RAD;
#endif
	inline constexpr VectorRegister4Int ZeroInt = GlobalVectorConstants::IntZero;
	inline constexpr VectorRegister4Int OneInt = GlobalVectorConstants::IntOne;
	inline constexpr VectorRegister4Int MinusOneInt = GlobalVectorConstants::IntMinusOne;
} // End of HGMSIMDConstants


// ---------------------------------------------------------------------------------------
// SIMD-Real
// Computes four floating-point numbers in parallel through the same interface as normal floating-point numbers.
// ---------------------------------------------------------------------------------------

#if HGM_USE_SIMD_REGISTER_FLOAT4X64
using FHGMSIMDReal = VectorRegister4Double;
using FHGMSIMDRealComponentType = double;
#else
using FHGMSIMDReal = VectorRegister4Float;
using FHGMSIMDRealComponentType = float;
#endif

FORCEINLINE FHGMSIMDReal operator+(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	return VectorAdd(A, B);
}


FORCEINLINE FHGMSIMDReal operator-(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	return VectorSubtract(A, B);
}


FORCEINLINE FHGMSIMDReal operator*(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	return VectorMultiply(A, B);
}


FORCEINLINE FHGMSIMDReal operator/(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	return VectorDivide(A, B);
}


FHGMSIMDReal& operator+=(FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	A = A + B;
	return A;
}


FHGMSIMDReal& operator-=(FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	A = A - B;
	return A;
}


FHGMSIMDReal& operator*=(FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	A = A * B;
	return A;
}


FHGMSIMDReal& operator/=(FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	A = A / B;
	return A;
}


/*
* Creates a four-part mask based on component-wise > compares of the input vectors.
*
* @param A	1st vector
* @param B	2nd vector
* @return	VectorRegister( A.x > B.x ? 0xFFFFFFFF : 0, A.y > B.y ? 0xFFFFFFFF : 0, A.z > B.z ? 0xFFFFFFFF : 0, A.w > B.w ? 0xFFFFFFFF : 0)
*/
FORCEINLINE FHGMSIMDReal operator>(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	return VectorCompareGT(A, B);
}


FORCEINLINE FHGMSIMDReal operator<(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	return VectorCompareLT(A, B);
}


FORCEINLINE FHGMSIMDReal operator>=(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	return VectorCompareGE(A, B);
}


FORCEINLINE FHGMSIMDReal operator<=(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	return VectorCompareLE(A, B);
}


FORCEINLINE FHGMSIMDReal operator==(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	return VectorCompareEQ(A, B);
}


FORCEINLINE FHGMSIMDReal operator!=(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	return VectorCompareNE(A, B);
}


FORCEINLINE FHGMSIMDReal operator&(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	return VectorBitwiseAnd(A, B);
}


FORCEINLINE FHGMSIMDReal operator|(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	return VectorBitwiseOr(A, B);
}


FORCEINLINE FHGMSIMDReal& operator&=(FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	A = A & B;
	return A;
}


FORCEINLINE FHGMSIMDReal& operator|=(FHGMSIMDReal& A, const FHGMSIMDReal& B)
{
	A = A | B;
	return A;
}


FORCEINLINE FHGMSIMDReal operator-(const FHGMSIMDReal& A)
{
	return VectorNegate(A);
}


FORCEINLINE FHGMSIMDReal operator~(const FHGMSIMDReal& A)
{
#if HGM_USE_SIMD_REGISTER_FLOAT4X64
	return VectorCastIntToDouble(VectorIntNot(VectorCastDoubleToInt(A)));
#else
	return VectorCastIntToFloat(VectorIntNot(VectorCastFloatToInt(A)));
#endif
}


// ---------------------------------------------------------------------------------------
// SIMD-Int
// ---------------------------------------------------------------------------------------
using FHGMSIMDInt = VectorRegister4Int;


FORCEINLINE FHGMSIMDInt operator>(const FHGMSIMDInt& A, const FHGMSIMDInt& B)
{
	return VectorIntCompareGT(A, B);
}


FORCEINLINE FHGMSIMDInt operator<(const FHGMSIMDInt& A, const FHGMSIMDInt& B)
{
	return VectorIntCompareLT(A, B);
}


FORCEINLINE FHGMSIMDInt operator>=(const FHGMSIMDInt& A, const FHGMSIMDInt& B)
{
	return VectorIntCompareGE(A, B);
}


FORCEINLINE FHGMSIMDInt operator<=(const FHGMSIMDInt& A, const FHGMSIMDInt& B)
{
	return VectorIntCompareLE(A, B);
}


FORCEINLINE FHGMSIMDInt operator==(const FHGMSIMDInt& A, const FHGMSIMDInt& B)
{
	return VectorIntCompareEQ(A, B);
}


FORCEINLINE FHGMSIMDInt operator!=(const FHGMSIMDInt& A, const FHGMSIMDInt& B)
{
	return VectorIntNot(VectorIntCompareEQ(A, B));
}


FORCEINLINE FHGMSIMDInt operator&(const FHGMSIMDInt& A, const FHGMSIMDInt& B)
{
	return VectorIntAnd(A, B);
}


FORCEINLINE FHGMSIMDInt operator|(const FHGMSIMDInt& A, const FHGMSIMDInt& B)
{
	return VectorIntOr(A, B);
}


// ---------------------------------------------------------------------------------------
// SIMD-Vector
// Contains four FVector.
// Presents FVector-like interface.
// ---------------------------------------------------------------------------------------
template<typename Real>
struct THGMSIMDVector3
{
	constexpr THGMSIMDVector3()
	{
		// Intentionally uninitialized to improve performance.
		// Use FHGMSIMDLibrary::Load() to set the value.
		// Use FHGMSIMDLibrary::Store() to get the value.
	}

	constexpr THGMSIMDVector3(const FHGMSIMDReal& X, const FHGMSIMDReal& Y, const FHGMSIMDReal& Z)
		: X(X)
		, Y(Y)
		, Z(Z)
	{
	}

	template<typename Real>
	THGMSIMDVector3<Real>& operator=(const THGMSIMDVector3<Real>& V)
	{
		this->X = V.X;
		this->Y = V.Y;
		this->Z = V.Z;

		return *this;
	}

	FHGMSIMDReal X {};
	FHGMSIMDReal Y {};
	FHGMSIMDReal Z {};

	static const THGMSIMDVector3<Real> ZeroVector;
	static const THGMSIMDVector3<Real> OneVector;
	static const THGMSIMDVector3<Real> OneHalfVector;
	static const THGMSIMDVector3<Real> ForwardVector;
	static const THGMSIMDVector3<Real> RightVector;
	static const THGMSIMDVector3<Real> UpVector;
};


template<typename Real>
THGMSIMDVector3<Real> operator+(const THGMSIMDVector3<Real>& V, const THGMSIMDVector3<Real>& W)
{
	THGMSIMDVector3<Real> Result {};
	Result.X = V.X + W.X;
	Result.Y = V.Y + W.Y;
	Result.Z = V.Z + W.Z;

	return Result;
}


template<typename Real>
THGMSIMDVector3<Real> operator-(const THGMSIMDVector3<Real>& V, const THGMSIMDVector3<Real>& W)
{
	THGMSIMDVector3<Real> Result {};
	Result.X = V.X - W.X;
	Result.Y = V.Y - W.Y;
	Result.Z = V.Z - W.Z;

	return Result;
}


// Multiplication of Vector components.
// Note: It is not cross product.
template<typename Real>
THGMSIMDVector3<Real> operator*(const THGMSIMDVector3<Real>& V, const THGMSIMDVector3<Real>& W)
{
	THGMSIMDVector3<Real> Result {};
	Result.X = V.X * W.X;
	Result.Y = V.Y * W.Y;
	Result.Z = V.Z * W.Z;

	return Result;
}


template<typename Real>
THGMSIMDVector3<Real> operator/(const THGMSIMDVector3<Real>& V, const THGMSIMDVector3<Real>& W)
{
	THGMSIMDVector3<Real> Result {};
	Result.X = V.X / W.X;
	Result.Y = V.Y / W.Y;
	Result.Z = V.Z / W.Z;

	return Result;
}


template<typename Real>
THGMSIMDVector3<Real>& operator+=(THGMSIMDVector3<Real>& V, const THGMSIMDVector3<Real>& W)
{
	V.X += W.X;
	V.Y += W.Y;
	V.Z += W.Z;

	return V;
}


template<typename Real>
THGMSIMDVector3<Real>& operator-=(THGMSIMDVector3<Real>& V, const THGMSIMDVector3<Real>& W)
{
	V = V - W;
	return V;
}


template<typename Real>
THGMSIMDVector3<Real>& operator*=(THGMSIMDVector3<Real>& V, const THGMSIMDVector3<Real>& W)
{
	V = V * W;
	return V;
}


template<typename Real>
THGMSIMDVector3<Real>& operator/=(THGMSIMDVector3<Real>& V, const THGMSIMDVector3<Real>& W)
{
	V = V / W;
	return V;
}


// Scalar * Vector
template<typename Real>
THGMSIMDVector3<Real> operator*(const FHGMSIMDReal& A, const THGMSIMDVector3<Real> V)
{
	THGMSIMDVector3<Real> Result = V;
	Result.X = A * Result.X;
	Result.Y = A * Result.Y;
	Result.Z = A * Result.Z;

	return Result;
}


template<typename Real>
THGMSIMDVector3<Real> operator*(const THGMSIMDVector3<Real> V, const FHGMSIMDReal& A)
{
	THGMSIMDVector3<Real> Result = V;
	return A * Result;
}


template<typename Real>
THGMSIMDVector3<Real>& operator*=(THGMSIMDVector3<Real>& V, const FHGMSIMDReal& A)
{
	V = V * A;
	return V;
}


template<typename Real>
THGMSIMDVector3<Real> operator/(const THGMSIMDVector3<Real> V, const FHGMSIMDReal& A)
{
	THGMSIMDVector3<Real> Result = V;
	Result.X = Result.X / A;
	Result.Y = Result.Y / A;
	Result.Z = Result.Z / A;

	return Result;
}


template<typename Real>
THGMSIMDVector3<Real> operator-(const THGMSIMDVector3<Real> V)
{
	THGMSIMDVector3<Real> Result {};
	Result.X = -V.X;
	Result.Y = -V.Y;
	Result.Z = -V.Z;

	return Result;
}


#if HGM_USE_SIMD_REGISTER_FLOAT4X64
using FHGMSIMDVector3 = THGMSIMDVector3<double>;
#else
using FHGMSIMDVector3 = THGMSIMDVector3<float>;
#endif


// ---------------------------------------------------------------------------------------
// SIMD-Quaternion
// Contains four FQuat.
// Instruction converts Math/Quat.h and Math/UnrealMathFPU.h to SIMD.
// ---------------------------------------------------------------------------------------
template<typename Real>
struct THGMSIMDQuaternion
{
	constexpr THGMSIMDQuaternion()
	{
	}

	constexpr THGMSIMDQuaternion(const FHGMSIMDReal& X, const FHGMSIMDReal& Y, const FHGMSIMDReal& Z, const FHGMSIMDReal& W)
		: X(X)
		, Y(Y)
		, Z(Z)
		, W(W)
	{
	}

	FHGMSIMDReal X;
	FHGMSIMDReal Y;
	FHGMSIMDReal Z;
	FHGMSIMDReal W;

	static const THGMSIMDQuaternion<Real> Identity;
};


template<typename Real>
THGMSIMDQuaternion<Real> operator+(const THGMSIMDQuaternion<Real>& Q, const THGMSIMDQuaternion<Real>& R)
{
	return THGMSIMDQuaternion<Real>(Q.X + R.X, Q.Y + R.Y, Q.Z + R.Z, Q.W + R.W);
}


template<typename Real>
THGMSIMDQuaternion<Real>& operator+=(THGMSIMDQuaternion<Real>& Q, const THGMSIMDQuaternion<Real>& R)
{
	Q.X += R.X;
	Q.Y += R.Y;
	Q.Z += R.Z;
	Q.W += R.W;

	return Q;
}


template<typename Real>
THGMSIMDQuaternion<Real> operator-(const THGMSIMDQuaternion<Real>& Q, const THGMSIMDQuaternion<Real>& R)
{
	return THGMSIMDQuaternion<Real>(Q.X - R.X, Q.Y - R.Y, Q.Z - R.Z, Q.W - R.W);
}


template<typename Real>
THGMSIMDQuaternion<Real>& operator-=(THGMSIMDQuaternion<Real>& Q, const THGMSIMDQuaternion<Real>& R)
{
	Q.X -= R.X;
	Q.Y -= R.Y;
	Q.Z -= R.Z;
	Q.W -= R.W;

	return Q;
}


template<typename Real>
THGMSIMDQuaternion<Real> operator*(const THGMSIMDQuaternion<Real>& Q, const THGMSIMDQuaternion<Real>& R)
{
	// See UnrealMathFPU::VectorQuaternionMultiply().
	const FHGMSIMDReal T0 = (Q.Z - Q.Y) * (R.Y - R.Z);
	const FHGMSIMDReal T1 = (Q.W + Q.X) * (R.W + R.X);
	const FHGMSIMDReal T2 = (Q.W - Q.X) * (R.Y + R.Z);
	const FHGMSIMDReal T3 = (Q.Y + Q.Z) * (R.W - R.X);
	const FHGMSIMDReal T4 = (Q.Z - Q.X) * (R.X - R.Y);
	const FHGMSIMDReal T5 = (Q.Z + Q.X) * (R.X + R.Y);
	const FHGMSIMDReal T6 = (Q.W + Q.Y) * (R.W - R.Z);
	const FHGMSIMDReal T7 = (Q.W - Q.Y) * (R.W + R.Z);
	const FHGMSIMDReal T8 = T5 + T6 + T7;
	const FHGMSIMDReal T9 = HGMSIMDConstants::OneHalfReal * (T4 + T8);

	return THGMSIMDQuaternion<Real>(T1 + T9 - T8, T2 + T9 - T7, T3 + T9 - T6, T0 + T9 - T5);
}


template<typename Real>
THGMSIMDQuaternion<Real>& operator*=(THGMSIMDQuaternion<Real>& Q, const THGMSIMDQuaternion<Real>& R)
{
	Q = Q * R;
	return Q;
}


#if HGM_USE_SIMD_REGISTER_FLOAT4X64
using FHGMSIMDQuaternion = THGMSIMDQuaternion<double>;
#else
using FHGMSIMDQuaternion = THGMSIMDQuaternion<float>;
#endif


// ---------------------------------------------------------------------------------------
// SIMD-Transform
// Contains four FTransform.
// ---------------------------------------------------------------------------------------
template<typename Real>
struct THGMSIMDTransform
{
	constexpr THGMSIMDTransform()
	{
	}

	constexpr THGMSIMDTransform(const FHGMSIMDVector3& Scale3D, const FHGMSIMDQuaternion& Rotation, const FHGMSIMDVector3& Translation)
		: Scale3D(Scale3D)
		, Rotation(Rotation)
		, Translation(Translation)
	{
	}

	FHGMSIMDVector3 Scale3D {};
	FHGMSIMDQuaternion Rotation {};
	FHGMSIMDVector3 Translation {};

	static const THGMSIMDTransform<Real> Identity;
};


#if HGM_USE_SIMD_REGISTER_FLOAT4X64
using FHGMSIMDTransform = THGMSIMDTransform<double>;
#else
using FHGMSIMDTransform = THGMSIMDTransform<float>;
#endif


// ---------------------------------------------------------------------------------------
// SIMDLibrary
// ---------------------------------------------------------------------------------------
// Index packed in 4-element units
struct FHGMSIMDIndex
{
	FHGMSIMDIndex()
	{
	}

	FHGMSIMDIndex(int32 UnpackedIndex)
		: PackedIndex(UnpackedIndex / 4)
		, ComponentIndex(UnpackedIndex % 4)
	{
	}

	FHGMSIMDIndex(int32 PackedIndex, int32 ComponentIndex)
		: PackedIndex(PackedIndex)
		, ComponentIndex(ComponentIndex)
	{
	}

	FORCEINLINE int32 GetUnpackedIndex() const
	{
		return PackedIndex * 4 + ComponentIndex;
	}

	int32 PackedIndex;
	int32 ComponentIndex;
};


struct HAGOROMO_API FHGMSIMDLibrary
{
	// Memory --> Register.
	// Real :
	FORCEINLINE static void Load(FHGMSIMDReal& A, FHGMReal X)
	{
		A = VectorSetFloat1(StaticCast<FHGMSIMDRealComponentType>(X));
	}

	FORCEINLINE static void Load(FHGMSIMDReal& A, FHGMReal X, FHGMReal Y, FHGMReal Z, FHGMReal W)
	{
		A = MakeVectorRegister(StaticCast<FHGMSIMDRealComponentType>(X), StaticCast<FHGMSIMDRealComponentType>(Y), StaticCast<FHGMSIMDRealComponentType>(Z), StaticCast<FHGMSIMDRealComponentType>(W));
	}

	FORCEINLINE static void Load(FHGMSIMDReal& A, const TStaticArray<FHGMReal, 4>& RealValues)
	{
		TStaticArray<FHGMSIMDRealComponentType, 4> Values {};
		FHGMSIMDRealComponentType* RawValues = nullptr;
		RawValues = Values.GetData();
		RawValues[0] = StaticCast<FHGMSIMDRealComponentType>(RealValues[0]);
		RawValues[1] = StaticCast<FHGMSIMDRealComponentType>(RealValues[1]);
		RawValues[2] = StaticCast<FHGMSIMDRealComponentType>(RealValues[2]);
		RawValues[3] = StaticCast<FHGMSIMDRealComponentType>(RealValues[3]);
		A = VectorLoadAligned(RawValues);
	}

	FORCEINLINE static void Load(FHGMSIMDReal& A, uint32 ComponentIndex, FHGMReal X)
	{
		(((FHGMSIMDRealComponentType*)&(A))[ComponentIndex]) = X;
	}

	static void Load(TArrayView<FHGMSIMDReal> A, const TStaticArray<FHGMSIMDIndex, 4>& SIMDIndexes, const TStaticArray<FHGMReal, 4>& X);

	static void Load(TArrayView<FHGMSIMDReal> A, const FHGMSIMDInt& sUnpackedIndex, const TStaticArray<FHGMReal, 4>& X);

	static void Load(TArrayView<FHGMSIMDReal> A, const FHGMSIMDInt& sUnpackedIndex, const FHGMSIMDReal& X);

	FORCEINLINE static constexpr FHGMSIMDReal LoadConstant(FHGMReal X, FHGMReal Y, FHGMReal Z, FHGMReal W)
	{
		return MakeVectorRegisterConstant(StaticCast<FHGMSIMDRealComponentType>(X), StaticCast<FHGMSIMDRealComponentType>(Y), StaticCast<FHGMSIMDRealComponentType>(Z), StaticCast<FHGMSIMDRealComponentType>(W));
	}

	FORCEINLINE static constexpr FHGMSIMDReal LoadConstant(FHGMReal A)
	{
		return MakeVectorRegisterConstant(StaticCast<FHGMSIMDRealComponentType>(A), StaticCast<FHGMSIMDRealComponentType>(A), StaticCast<FHGMSIMDRealComponentType>(A), StaticCast<FHGMSIMDRealComponentType>(A));
	}

	// Int :
	FORCEINLINE static void Load(FHGMSIMDInt& A, int32 X)
	{
		A = VectorIntSet1(X);
	}

	FORCEINLINE static void Load(FHGMSIMDInt& A, int32 X, int32 Y, int32 Z, int32 W)
	{
		A = MakeVectorRegisterInt(X, Y, Z, W);
	}

	FORCEINLINE static void Load(FHGMSIMDInt& A, const TStaticArray<int32, 4>& IntValues)
	{
		A = VectorIntLoadAligned(IntValues.GetData());
	}

	FORCEINLINE static void Load(FHGMSIMDInt& A, uint32 ComponentIndex, int32 X)
	{
		(((int32*)&(A))[ComponentIndex]) = X;
	}

	// Vector :
	static void Load(FHGMSIMDVector3& V, const FHGMVector3& W0, const FHGMVector3& W1, const FHGMVector3& W2, const FHGMVector3& W3);

	FORCEINLINE static void Load(FHGMSIMDVector3& V, const TStaticArray<FHGMVector3, 4>& Vectors)
	{
		FHGMSIMDLibrary::Load(V, Vectors[0], Vectors[1], Vectors[2], Vectors[3]);
	}

	static void Load(FHGMSIMDVector3& V, const FHGMVector3& W);

	static void Load(FHGMSIMDVector3& V, const FHGMSIMDReal& X, const FHGMSIMDReal& Y, const FHGMSIMDReal& Z);

	// V[ComponentIndex] = W.
	FORCEINLINE static void Load(FHGMSIMDVector3& V, uint32 ComponentIndex, const FHGMVector3& W)
	{
		FHGMSIMDLibrary::Load(V.X, ComponentIndex, W.X);
		FHGMSIMDLibrary::Load(V.Y, ComponentIndex, W.Y);
		FHGMSIMDLibrary::Load(V.Z, ComponentIndex, W.Z);
	}

	static void Load(TArrayView<FHGMSIMDVector3> V, const FHGMSIMDIndex& SIMDIndex, const FHGMVector3& W);

	static void Load(TArrayView<FHGMSIMDVector3> V, const TStaticArray<FHGMSIMDIndex, 4>& SIMDIndexes, TStaticArray<FHGMVector3, 4>& W);

	static void Load(TArrayView<FHGMSIMDVector3> V, const FHGMSIMDInt& sUnpackedIndex, TStaticArray<FHGMVector3, 4>& W);

	static void Load(TArrayView<FHGMSIMDVector3> V, const FHGMSIMDInt& sUnpackedIndex, const FHGMSIMDVector3& W);

	// Quaternion :
	static void Load(FHGMSIMDQuaternion& sQ, const FHGMQuaternion& Q0, const FHGMQuaternion& Q1, const FHGMQuaternion& Q2, const FHGMQuaternion& Q3)
	{
		FHGMSIMDLibrary::Load(sQ.X, Q0.X, Q1.X, Q2.X, Q3.X);
		FHGMSIMDLibrary::Load(sQ.Y, Q0.Y, Q1.Y, Q2.Y, Q3.Y);
		FHGMSIMDLibrary::Load(sQ.Z, Q0.Z, Q1.Z, Q2.Z, Q3.Z);
		FHGMSIMDLibrary::Load(sQ.W, Q0.W, Q1.W, Q2.W, Q3.W);
	}

	FORCEINLINE static void Load(FHGMSIMDQuaternion& sQ, const TStaticArray<FHGMQuaternion, 4>& Quaternions)
	{
		FHGMSIMDLibrary::Load(sQ, Quaternions[0], Quaternions[1], Quaternions[2], Quaternions[3]);
	}

	FORCEINLINE static void Load(FHGMSIMDQuaternion& sQ, const FHGMQuaternion& Q)
	{
		FHGMSIMDLibrary::Load(sQ.X, Q.X);
		FHGMSIMDLibrary::Load(sQ.Y, Q.Y);
		FHGMSIMDLibrary::Load(sQ.Z, Q.Z);
		FHGMSIMDLibrary::Load(sQ.W, Q.W);
	}

	// Transform :
	FORCEINLINE static void Load(FHGMSIMDTransform& sTransform, const FHGMTransform& Transform)
	{
		FHGMSIMDLibrary::Load(sTransform.Scale3D, Transform.GetScale3D());
		FHGMSIMDLibrary::Load(sTransform.Rotation, Transform.GetRotation());
		FHGMSIMDLibrary::Load(sTransform.Translation, Transform.GetTranslation());
	}

	// Register --> Memory.
	// Real :
	static void Store(const FHGMSIMDReal& A, FHGMReal& X, FHGMReal& Y, FHGMReal& Z, FHGMReal& W);

	FORCEINLINE static void Store(const FHGMSIMDReal& A, TStaticArray<FHGMReal, 4>& RealValues)
	{
		FHGMSIMDLibrary::Store(A, RealValues[0], RealValues[1], RealValues[2], RealValues[3]);
	}

	FORCEINLINE static void Store(const FHGMSIMDReal& A, uint32 ComponentIndex, FHGMReal& X)
	{
		X = VectorGetComponentDynamic(A, ComponentIndex);
	}

	static void Store(TConstArrayView<FHGMSIMDReal> A, const TStaticArray<FHGMSIMDIndex, 4>& SIMDIndexes, TStaticArray<FHGMReal, 4>& X);

	static void Store(TConstArrayView<FHGMSIMDReal> A, const FHGMSIMDInt& sUnpackedIndex, TStaticArray<FHGMReal, 4>& X);

	static void Store(TConstArrayView<FHGMSIMDReal> A, const FHGMSIMDInt& sUnpackedIndex, FHGMSIMDReal& X);

	// Int :
	static void Store(const FHGMSIMDInt& A, int32& X, int32& Y, int32& Z, int32& W);

	FORCEINLINE static void Store(const FHGMSIMDInt& A, TStaticArray<int32, 4>& IntValues)
	{
		VectorIntStoreAligned(A, IntValues.GetData());
	}

	FORCEINLINE static void Store(const FHGMSIMDInt& A, uint32 ComponentIndex, int32& X)
	{
		X = (((int32*)&(A))[ComponentIndex]);
	}

	// Vector :
	static void Store(const FHGMSIMDVector3& V, FHGMVector3& W0, FHGMVector3& W1, FHGMVector3& W2, FHGMVector3& W3);

	FORCEINLINE static void Store(const FHGMSIMDVector3& V, TStaticArray<FHGMVector3, 4>& Vectors)
	{
		FHGMSIMDLibrary::Store(V, Vectors[0], Vectors[1], Vectors[2], Vectors[3]);
	}

	static void Store(const FHGMSIMDVector3& V, uint32 ComponentIndex, FHGMVector3& W);

	static void Store(TArrayView<const FHGMSIMDVector3> V, const TStaticArray<FHGMSIMDIndex, 4>& SIMDIndexes, TStaticArray<FHGMVector3, 4>& W);

	static void Store(TArrayView<const FHGMSIMDVector3> V, const FHGMSIMDInt& sUnpackedIndex, TStaticArray<FHGMVector3, 4>& W);

	static void Store(TArrayView<const FHGMSIMDVector3> V, const FHGMSIMDInt& sUnpackedIndex, FHGMSIMDVector3& W);

	// Quaternion :
	static void Store(const FHGMSIMDQuaternion& sQ, TStaticArray<FHGMQuaternion, 4>& Quaternions);

	// Transform :
	static void Store(const FHGMSIMDTransform& sTransform, TStaticArray<FTransform, 4>& Transforms);

	// Cast.
	FORCEINLINE static FHGMSIMDReal CastIntToReal(const FHGMSIMDInt& A)
	{
#if HGM_USE_SIMD_REGISTER_FLOAT4X64
		return VectorCastIntToDouble(A);
#else
		return VectorCastIntToFloat(A);
#endif
	}

	FORCEINLINE static FHGMSIMDInt CastRealToInt(const FHGMSIMDReal& A)
	{
#if HGM_USE_SIMD_REGISTER_FLOAT4X64
		return VectorCastDoubleToInt(A);
#else
		return VectorCastFloatToInt(A);
#endif
	}

	// Can be used as a mask for FHGMSIMDLibrary::Select()
	// Note: Do not pass numbers other than 1 and 0 as arguments.
	// ( e.g. 1, 0, 0, 1 --> 0xFFFFFFFF, 0, 0, 0xFFFFFFFF )
	FORCEINLINE static FHGMSIMDReal MakeComponentwiseMask(uint32 XMask, uint32 YMask, uint32 ZMask, uint32 WMask)
	{
		return MakeVectorRegister(~uint32(0) * XMask, ~uint32(0) * YMask, ~uint32(0) * ZMask, ~uint32(0) * WMask);
	}

	// Bitwise selection.
	// If you want to select by component, use a per-component mask. ( e.g. FHGMSIMDReal comparison operator )
	FORCEINLINE static FHGMSIMDReal Select(const FHGMSIMDReal& Mask, const FHGMSIMDReal& A, const FHGMSIMDReal& B)
	{
		return VectorSelect(Mask, A, B);
	}

	FORCEINLINE static FHGMSIMDReal Select(const FHGMSIMDInt& Mask, const FHGMSIMDReal& A, const FHGMSIMDReal& B)
	{
		return FHGMSIMDLibrary::Select(FHGMSIMDLibrary::CastIntToReal(Mask), A, B);
	}

	static FHGMSIMDVector3 Select(const FHGMSIMDReal& Mask, const FHGMSIMDVector3& V, const FHGMSIMDVector3& W);

	FORCEINLINE static FHGMSIMDVector3 Select(const FHGMSIMDInt& Mask, const FHGMSIMDVector3& V, const FHGMSIMDVector3& W)
	{
		return FHGMSIMDLibrary::Select(FHGMSIMDLibrary::CastIntToReal(Mask), V, W);
	}

	static FHGMSIMDQuaternion Select(const FHGMSIMDReal& Mask, const FHGMSIMDQuaternion& sQ1, const FHGMSIMDQuaternion& sQ2);

	FORCEINLINE static FHGMSIMDQuaternion Select(const FHGMSIMDInt& Mask, const FHGMSIMDQuaternion& sQ1, const FHGMSIMDQuaternion& sQ2)
	{
		return FHGMSIMDLibrary::Select(FHGMSIMDLibrary::CastIntToReal(Mask), sQ1, sQ2);
	}

	// Returns true if any of the per-component masks obtained by FHGMSIMDReal::operator>() etc. are valid.
	FORCEINLINE static bool IsAnyMaskSet(const FHGMSIMDReal& Mask)
	{
		return StaticCast<bool>(VectorMaskBits(Mask));
	}

	// Convert unpacked index to SIMDIndex.
	FORCEINLINE static TStaticArray<FHGMSIMDIndex, 4> MakeSIMDIndex(const FHGMSIMDInt& UnpackedIndex)
	{
		int32 Index0, Index1, Index2, Index3 = 0;
		FHGMSIMDLibrary::Store(UnpackedIndex, Index0, Index1, Index2, Index3);

		TStaticArray<FHGMSIMDIndex, 4> Result {};
		Result[0] = FHGMSIMDIndex(Index0);
		Result[1] = FHGMSIMDIndex(Index1);
		Result[2] = FHGMSIMDIndex(Index2);
		Result[3] = FHGMSIMDIndex(Index3);

		return Result;
	}
};


// ---------------------------------------------------------------------------------------
// MathLibrary
// ---------------------------------------------------------------------------------------
struct FHGMMathLibrary
{
	template<typename T>
	FORCEINLINE static T Abs(T A)
	{
		return FMath::Abs(A);
	}

	FORCEINLINE static FHGMSIMDReal Abs(const FHGMSIMDReal& A)
	{
		return VectorAbs(A);
	}

	template<typename T>
	FORCEINLINE static T Min(T A, T B)
	{
		return FMath::Min(A, B);
	}

	FORCEINLINE static FHGMSIMDReal Min(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
	{
		return VectorMin(A, B);
	}

	template<typename T>
	FORCEINLINE static T Max(T A, T B)
	{
		return FMath::Max(A, B);
	}

	FORCEINLINE static FHGMSIMDReal Max(const FHGMSIMDReal& A, const FHGMSIMDReal& B)
	{
		return VectorMax(A, B);
	}

	template<typename T>
	FORCEINLINE static T Clamp(T TValue, T MinValue, T MaxValue)
	{
		return FMath::Clamp(TValue, MinValue, MaxValue);
	}

	FORCEINLINE static FHGMSIMDReal Clamp(const FHGMSIMDReal& T, const FHGMSIMDReal& MinValue, const FHGMSIMDReal& MaxValue)
	{
		return FHGMMathLibrary::Max(FHGMMathLibrary::Min(T, MaxValue), MinValue);
	}

	FORCEINLINE static FHGMSIMDReal Clamp(FHGMReal T, const FHGMSIMDReal& MinValue, const FHGMSIMDReal& MaxValue)
	{
		return FHGMMathLibrary::Clamp(VectorSetFloat1(StaticCast<FHGMSIMDRealComponentType>(T)), MinValue, MaxValue);
	}

	template<typename T, typename U>
	FORCEINLINE static T Lerp(T A, T B, U Alpha)
	{
		return FMath::Lerp(A, B, Alpha);
	}

	FORCEINLINE static FHGMSIMDReal Lerp(const FHGMSIMDReal& A, const FHGMSIMDReal& B, const FHGMSIMDReal& Alpha)
	{
		return VectorLerp(A, B, Alpha);
	}

	FORCEINLINE static FHGMSIMDVector3 Lerp(const FHGMSIMDVector3& V, const FHGMSIMDVector3& W, const FHGMSIMDReal& Alpha)
	{
		FHGMSIMDVector3 Result {};
		Result.X = FHGMMathLibrary::Lerp(V.X, W.X, Alpha);
		Result.Y = FHGMMathLibrary::Lerp(V.Y, W.Y, Alpha);
		Result.Z = FHGMMathLibrary::Lerp(V.Z, W.Z, Alpha);

		return Result;
	}

	static FHGMSIMDQuaternion Slerp(const FHGMSIMDQuaternion& sQ1, const FHGMSIMDQuaternion& sQ2, const FHGMSIMDReal& sT);

	template<typename T>
	FORCEINLINE static T Pow(T A, T B)
	{
		return FMath::Pow(A, B);
	}

	FORCEINLINE static FHGMSIMDReal Pow(const FHGMSIMDReal& Base, const FHGMSIMDReal& Exponent)
	{
		return VectorPow(Base, Exponent);
	}

	FORCEINLINE static FHGMReal Sin(FHGMReal X)
	{
		return FMath::Sin(X);
	}

	FORCEINLINE static FHGMSIMDReal Sin(const FHGMSIMDReal& X)
	{
		return VectorSin(X);
	}

	FORCEINLINE static FHGMReal Cos(FHGMReal X)
	{
		return FMath::Cos(X);
	}

	FORCEINLINE static FHGMSIMDReal Cos(const FHGMSIMDReal& X)
	{
		return VectorCos(X);
	}

	FORCEINLINE static FHGMReal Acos(FHGMReal X)
	{
		return FMath::Acos(X);
	}

	FORCEINLINE static FHGMSIMDReal Acos(const FHGMSIMDReal& X)
	{
		return VectorACos(X);
	}

	FORCEINLINE static FHGMReal Atan2(FHGMReal Y, FHGMReal X)
	{
		return FMath::Atan2(Y, X);
	}

	FORCEINLINE static FHGMSIMDReal Atan2(const FHGMSIMDReal& Y, const FHGMSIMDReal& X)
	{
		return VectorATan2(Y, X);
	}

	FORCEINLINE static void SinCos(FHGMReal* OutSin, FHGMReal* OutCos, const FHGMReal* AngleRadians)
	{
		return FMath::SinCos(OutSin, OutCos, *AngleRadians);
	}

	FORCEINLINE static void SinCos(FHGMSIMDReal* sOutSin, FHGMSIMDReal* sOutCos, const FHGMSIMDReal* sAngleRadians)
	{
		return VectorSinCos(sOutSin, sOutCos, sAngleRadians);
	}

	FORCEINLINE static FHGMReal RadiansToDegrees(FHGMReal Radians)
	{
		return FMath::RadiansToDegrees(Radians);
	}

	FORCEINLINE static FHGMSIMDReal RadiansToDegrees(FHGMSIMDReal& sRadians)
	{
		return sRadians * HGMSIMDConstants::RadiansToDegrees;
	}

	FORCEINLINE static FHGMReal DegreesToRadians(FHGMReal Degrees)
	{
		return FMath::DegreesToRadians(Degrees);
	}

	FORCEINLINE static FHGMSIMDReal DegreesToRadians(FHGMSIMDReal& Degrees)
	{
		return Degrees * HGMSIMDConstants::DegreesToRadians;
	}

	// e.g.
	// Value:4, Multiple: 4 --> 4
	// Value:5, Multiple: 4 --> 8
	// Value:7, Multiple: 4 --> 8
	// Value:9, Multiple: 4 --> 12
	FORCEINLINE static int32 RoundUpToMultiple(int32 Value, int32 Multiple)
	{
		return ((Value + Multiple - 1) / Multiple) * Multiple;
	}

	FORCEINLINE static FHGMReal DotProduct(const FHGMVector3& V, const FHGMVector3& W)
	{
		return FHGMVector3::DotProduct(V, W);
	}

	static FHGMSIMDReal DotProduct(const FHGMSIMDVector3& V, const FHGMSIMDVector3& W);

	FORCEINLINE static FHGMVector3 CrossProduct(const FHGMVector3& V, const FHGMVector3& W)
	{
		return FVector::CrossProduct(V, W);
	}

	static FHGMSIMDVector3 CrossProduct(const FHGMSIMDVector3& V, const FHGMSIMDVector3& W);

	FORCEINLINE static FHGMReal Sqrt(FHGMReal A)
	{
		return FMath::Sqrt(A);
	}

	FORCEINLINE static FHGMSIMDReal Sqrt(const FHGMSIMDReal& A)
	{
		return VectorSqrt(A);
	}

	FORCEINLINE static FHGMReal ReciprocalSqrt(FHGMReal LenghtSquared)
	{
		return FMath::InvSqrt(LenghtSquared);
	}

	static FHGMSIMDReal ReciprocalSqrt(const FHGMSIMDReal& LenghtSquared);

	FORCEINLINE static FHGMReal ReciprocalLength(const FHGMVector3& V)
	{
		return FHGMMathLibrary::ReciprocalSqrt(FHGMMathLibrary::LengthSquared(V));
	}

	static FHGMSIMDReal ReciprocalLength(const FHGMSIMDVector3& V);

	FORCEINLINE static FHGMReal LengthSquared(const FHGMVector3& V)
	{
		return FHGMMathLibrary::DotProduct(V, V);
	}

	static FHGMSIMDReal LengthSquared(const FHGMSIMDVector3& V);

	FORCEINLINE static FHGMReal Length(const FHGMVector3& V)
	{
		return V.Length();
	}

	static FHGMSIMDReal Length(const FHGMSIMDVector3& V);

	// Note: The argument vector is normalized.
	static FHGMVector3& SafeNormalize(FHGMVector3& V);

	static FHGMSIMDVector3& SafeNormalize(FHGMSIMDVector3& V);

	FORCEINLINE static FHGMVector3 MakeSafeNormal(const FHGMVector3& V)
	{
		return V.GetSafeNormal();
	}

	static FHGMSIMDVector3 MakeSafeNormal(const FHGMSIMDVector3& V);

	FORCEINLINE static FHGMVector3 MakeUnsafeNormal(const FHGMVector3& V)
	{
		return V.GetUnsafeNormal();
	}

	FHGMSIMDVector3 MakeUnsafeNormal(const FHGMSIMDVector3& V);

	static FHGMSIMDQuaternion SafeNormalize(FHGMSIMDQuaternion& sQ);

	static FHGMQuaternion MakeLookAtQuaternion(const FHGMVector3& TargetVector, const FHGMVector3& UpVector);

	static FHGMSIMDVector3 GetSafeScaleReciprocal(const FHGMSIMDVector3& Scale3D);

	static FHGMSIMDVector3 RotateVector(const FHGMSIMDQuaternion& Q, const FHGMSIMDVector3& V);

	static FHGMSIMDVector3 UnrotateVector(const FHGMSIMDQuaternion& Q, const FHGMSIMDVector3& V);

	static FHGMSIMDVector3 RotateAngleAxisRad(const FHGMSIMDVector3& sVector, const FHGMSIMDReal& sAngleRad, const FHGMSIMDVector3& sAxis);

	FORCEINLINE static FHGMSIMDVector3 RotateAngleAxis(const FHGMSIMDVector3& sVector, const FHGMSIMDReal& sAngleDeg, const FHGMSIMDVector3& sAxis)
	{
		return FHGMMathLibrary::RotateAngleAxisRad(sVector, sAngleDeg * HGMSIMDConstants::DegreesToRadians, sAxis);
	}

	FORCEINLINE static FHGMSIMDVector3 GetAxisX(const FHGMSIMDQuaternion& sQ)
	{
		return FHGMMathLibrary::RotateVector(sQ, FHGMSIMDVector3::ForwardVector);
	}

	FORCEINLINE static FHGMSIMDVector3 GetAxisY(const FHGMSIMDQuaternion& sQ)
	{
		return FHGMMathLibrary::RotateVector(sQ, FHGMSIMDVector3::RightVector);
	}

	FORCEINLINE static FHGMSIMDVector3 GetAxisZ(const FHGMSIMDQuaternion& sQ)
	{
		return FHGMMathLibrary::RotateVector(sQ, FHGMSIMDVector3::UpVector);
	}

	FORCEINLINE static FHGMSIMDVector3 GetForwardVector(const FHGMSIMDQuaternion& sQ)
	{
		return FHGMMathLibrary::GetAxisX(sQ);
	}

	FORCEINLINE static FHGMSIMDVector3 GetRightVector(const FHGMSIMDQuaternion& sQ)
	{
		return FHGMMathLibrary::GetAxisY(sQ);
	}

	FORCEINLINE static FHGMSIMDVector3 GetUpVector(const FHGMSIMDQuaternion& sQ)
	{
		return FHGMMathLibrary::GetAxisZ(sQ);
	}

	static FHGMSIMDVector3 GetUnitAxis(const FHGMSIMDQuaternion& sQ, const FHGMSIMDInt& sAxisIndex);

	static FHGMSIMDQuaternion Inverse(const FHGMSIMDQuaternion& Q);

	static FHGMSIMDTransform Inverse(const FHGMSIMDTransform& Transform);

	FORCEINLINE static FHGMSIMDVector3 TransformPosition(const FHGMSIMDTransform& Transform, const FHGMSIMDVector3& V)
	{
		return FHGMMathLibrary::RotateVector(Transform.Rotation, Transform.Scale3D * V) + Transform.Translation;
	}

	FORCEINLINE static FHGMSIMDVector3 InverseTransformPosition(const FHGMSIMDTransform& sTransform, const FHGMSIMDVector3& sV)
	{
		return FHGMMathLibrary::UnrotateVector(sTransform.Rotation, sV - sTransform.Translation) * FHGMMathLibrary::GetSafeScaleReciprocal(sTransform.Scale3D);
	}

	FORCEINLINE static FHGMSIMDVector3 TransformVector(const FHGMSIMDTransform& sTransform, const FHGMSIMDVector3& sV)
	{
		return FHGMMathLibrary::RotateVector(sTransform.Rotation, sTransform.Scale3D * sV);
	}

	FORCEINLINE static FHGMSIMDVector3 InverseTransformVector(const FHGMSIMDTransform& Transform, const FHGMSIMDVector3& sV)
	{
		return FHGMMathLibrary::UnrotateVector(Transform.Rotation, sV) * FHGMMathLibrary::GetSafeScaleReciprocal(Transform.Scale3D);
	}

	static FHGMSIMDVector3 PointPlaneProject(const FHGMSIMDVector3& sV, const FHGMSIMDVector3& sPlaneOrigin, const FHGMSIMDVector3& sPlaneNormal);
};
