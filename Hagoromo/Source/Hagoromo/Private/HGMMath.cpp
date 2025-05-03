// Hagoromo : Copyright (c) 2025 nozoxa_0131, MIT License

#include "HGMMath.h"


// ---------------------------------------------------------------------------------------
// SIMDLibrary
// ---------------------------------------------------------------------------------------
template<typename Real>
const THGMSIMDVector3<Real> THGMSIMDVector3<Real>::ZeroVector = FHGMSIMDVector3(HGMSIMDConstants::ZeroReal, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::ZeroReal);

template<typename Real>
const THGMSIMDVector3<Real> THGMSIMDVector3<Real>::OneVector = FHGMSIMDVector3(HGMSIMDConstants::OneReal, HGMSIMDConstants::OneReal, HGMSIMDConstants::OneReal);

template<typename Real>
const THGMSIMDVector3<Real> THGMSIMDVector3<Real>::OneHalfVector = FHGMSIMDVector3(HGMSIMDConstants::OneHalfReal, HGMSIMDConstants::OneHalfReal, HGMSIMDConstants::OneHalfReal);

template<typename Real>
const THGMSIMDVector3<Real> THGMSIMDVector3<Real>::ForwardVector = FHGMSIMDVector3(HGMSIMDConstants::OneReal, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::ZeroReal);

template<typename Real>
const THGMSIMDVector3<Real> THGMSIMDVector3<Real>::RightVector = FHGMSIMDVector3(HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal, HGMSIMDConstants::ZeroReal);

template<typename Real>
const THGMSIMDVector3<Real> THGMSIMDVector3<Real>::UpVector = FHGMSIMDVector3(HGMSIMDConstants::ZeroReal, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal);

template<typename Real>
const THGMSIMDQuaternion<Real> THGMSIMDQuaternion<Real>::Identity = FHGMSIMDQuaternion(HGMSIMDConstants::ZeroReal, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal);

template<typename Real>
const THGMSIMDTransform<Real> THGMSIMDTransform<Real>::Identity = FHGMSIMDTransform(HGMSIMDConstants::OneReal, FHGMSIMDQuaternion::Identity, HGMSIMDConstants::ZeroReal);


void FHGMSIMDLibrary::Load(TArrayView<FHGMSIMDReal> A, const TStaticArray<FHGMSIMDIndex, 4>& SIMDIndexes, const TStaticArray<FHGMReal, 4>& X)
{
	FHGMSIMDLibrary::Load(A[SIMDIndexes[0].PackedIndex], SIMDIndexes[0].ComponentIndex, X[0]);
	FHGMSIMDLibrary::Load(A[SIMDIndexes[1].PackedIndex], SIMDIndexes[1].ComponentIndex, X[1]);
	FHGMSIMDLibrary::Load(A[SIMDIndexes[2].PackedIndex], SIMDIndexes[2].ComponentIndex, X[2]);
	FHGMSIMDLibrary::Load(A[SIMDIndexes[3].PackedIndex], SIMDIndexes[3].ComponentIndex, X[3]);
}


void FHGMSIMDLibrary::Load(TArrayView<FHGMSIMDReal> A, const FHGMSIMDInt& sUnpackedIndex, const TStaticArray<FHGMReal, 4>& X)
{
	TStaticArray<FHGMSIMDIndex, 4> SIMDIndexes = FHGMSIMDLibrary::MakeSIMDIndex(sUnpackedIndex);
	FHGMSIMDLibrary::Load(A, SIMDIndexes, X);
}


void FHGMSIMDLibrary::Load(TArrayView<FHGMSIMDReal> A, const FHGMSIMDInt& sUnpackedIndex, const FHGMSIMDReal& X)
{
	TStaticArray<FHGMReal, 4> UnpackedX {};
	FHGMSIMDLibrary::Store(X, UnpackedX);
	FHGMSIMDLibrary::Load(A, sUnpackedIndex, UnpackedX);
}


void FHGMSIMDLibrary::Load(FHGMSIMDVector3& V, const FHGMVector3& W0, const FHGMVector3& W1, const FHGMVector3& W2, const FHGMVector3& W3)
{
	V.X = MakeVectorRegister(StaticCast<FHGMSIMDRealComponentType>(W0.X), StaticCast<FHGMSIMDRealComponentType>(W1.X), StaticCast<FHGMSIMDRealComponentType>(W2.X), StaticCast<FHGMSIMDRealComponentType>(W3.X));
	V.Y = MakeVectorRegister(StaticCast<FHGMSIMDRealComponentType>(W0.Y), StaticCast<FHGMSIMDRealComponentType>(W1.Y), StaticCast<FHGMSIMDRealComponentType>(W2.Y), StaticCast<FHGMSIMDRealComponentType>(W3.Y));
	V.Z = MakeVectorRegister(StaticCast<FHGMSIMDRealComponentType>(W0.Z), StaticCast<FHGMSIMDRealComponentType>(W1.Z), StaticCast<FHGMSIMDRealComponentType>(W2.Z), StaticCast<FHGMSIMDRealComponentType>(W3.Z));
}


void FHGMSIMDLibrary::Load(FHGMSIMDVector3& V, const FHGMVector3& W)
{
	const FHGMSIMDRealComponentType X = StaticCast<FHGMSIMDRealComponentType>(W.X);
	const FHGMSIMDRealComponentType Y = StaticCast<FHGMSIMDRealComponentType>(W.Y);
	const FHGMSIMDRealComponentType Z = StaticCast<FHGMSIMDRealComponentType>(W.Z);
	V.X = VectorSetFloat1(X);
	V.Y = VectorSetFloat1(Y);
	V.Z = VectorSetFloat1(Z);
}


void FHGMSIMDLibrary::Load(FHGMSIMDVector3& V, const FHGMSIMDReal& X, const FHGMSIMDReal& Y, const FHGMSIMDReal& Z)
{
	V.X = X;
	V.Y = Y;
	V.Z = Z;
}


void FHGMSIMDLibrary::Load(TArrayView<FHGMSIMDVector3> V, const FHGMSIMDIndex& SIMDIndex, const FHGMVector3& W)
{
	FHGMSIMDLibrary::Load(V[SIMDIndex.PackedIndex], SIMDIndex.ComponentIndex, W);
}


void FHGMSIMDLibrary::Load(TArrayView<FHGMSIMDVector3> V, const TStaticArray<FHGMSIMDIndex, 4>& SIMDIndexes, TStaticArray<FHGMVector3, 4>& W)
{
	FHGMSIMDLibrary::Load(V[SIMDIndexes[0].PackedIndex], SIMDIndexes[0].ComponentIndex, W[0]);
	FHGMSIMDLibrary::Load(V[SIMDIndexes[1].PackedIndex], SIMDIndexes[1].ComponentIndex, W[1]);
	FHGMSIMDLibrary::Load(V[SIMDIndexes[2].PackedIndex], SIMDIndexes[2].ComponentIndex, W[2]);
	FHGMSIMDLibrary::Load(V[SIMDIndexes[3].PackedIndex], SIMDIndexes[3].ComponentIndex, W[3]);
}


void FHGMSIMDLibrary::Load(TArrayView<FHGMSIMDVector3> V, const FHGMSIMDInt& sUnpackedIndex, TStaticArray<FHGMVector3, 4>& W)
{
	TStaticArray<int32, 4> UnpackedIndexes {};
	FHGMSIMDLibrary::Store(sUnpackedIndex, UnpackedIndexes);

	TStaticArray<FHGMSIMDIndex, 4> SIMDIndexes {};
	SIMDIndexes[0] = FHGMSIMDIndex(UnpackedIndexes[0]);
	SIMDIndexes[1] = FHGMSIMDIndex(UnpackedIndexes[1]);
	SIMDIndexes[2] = FHGMSIMDIndex(UnpackedIndexes[2]);
	SIMDIndexes[3] = FHGMSIMDIndex(UnpackedIndexes[3]);

	FHGMSIMDLibrary::Load(V, SIMDIndexes, W);
}


void FHGMSIMDLibrary::Load(TArrayView<FHGMSIMDVector3> V, const FHGMSIMDInt& sUnpackedIndex, const FHGMSIMDVector3& W)
{
	TStaticArray<int32, 4> UnpackedIndexes {};
	FHGMSIMDLibrary::Store(sUnpackedIndex, UnpackedIndexes);

	TStaticArray<FHGMSIMDIndex, 4> SIMDIndexes {};
	SIMDIndexes[0] = FHGMSIMDIndex(UnpackedIndexes[0]);
	SIMDIndexes[1] = FHGMSIMDIndex(UnpackedIndexes[1]);
	SIMDIndexes[2] = FHGMSIMDIndex(UnpackedIndexes[2]);
	SIMDIndexes[3] = FHGMSIMDIndex(UnpackedIndexes[3]);

	TStaticArray<FHGMVector3, 4> UnpackedW {};
	FHGMSIMDLibrary::Store(W, UnpackedW);

	FHGMSIMDLibrary::Load(V, SIMDIndexes, UnpackedW);
}


void FHGMSIMDLibrary::Store(const FHGMSIMDReal& A, FHGMReal& X, FHGMReal& Y, FHGMReal& Z, FHGMReal& W)
{
	FHGMSIMDRealComponentType Values[4] {};
	VectorStoreAligned(A, Values);
	X = StaticCast<FHGMReal>(Values[0]);
	Y = StaticCast<FHGMReal>(Values[1]);
	Z = StaticCast<FHGMReal>(Values[2]);
	W = StaticCast<FHGMReal>(Values[3]);
}


void FHGMSIMDLibrary::Store(TConstArrayView<FHGMSIMDReal> A, const TStaticArray<FHGMSIMDIndex, 4>& SIMDIndexes, TStaticArray<FHGMReal, 4>& X)
{
	FHGMSIMDLibrary::Store(A[SIMDIndexes[0].PackedIndex], SIMDIndexes[0].ComponentIndex, X[0]);
	FHGMSIMDLibrary::Store(A[SIMDIndexes[1].PackedIndex], SIMDIndexes[1].ComponentIndex, X[1]);
	FHGMSIMDLibrary::Store(A[SIMDIndexes[2].PackedIndex], SIMDIndexes[2].ComponentIndex, X[2]);
	FHGMSIMDLibrary::Store(A[SIMDIndexes[3].PackedIndex], SIMDIndexes[3].ComponentIndex, X[3]);
}


void FHGMSIMDLibrary::Store(TConstArrayView<FHGMSIMDReal> A, const FHGMSIMDInt& sUnpackedIndex, TStaticArray<FHGMReal, 4>& X)
{
	TStaticArray<int32, 4> UnpackedIndexes {};
	FHGMSIMDLibrary::Store(sUnpackedIndex, UnpackedIndexes);

	TStaticArray<FHGMSIMDIndex, 4> SIMDIndexes {};
	SIMDIndexes[0] = FHGMSIMDIndex(UnpackedIndexes[0]);
	SIMDIndexes[1] = FHGMSIMDIndex(UnpackedIndexes[1]);
	SIMDIndexes[2] = FHGMSIMDIndex(UnpackedIndexes[2]);
	SIMDIndexes[3] = FHGMSIMDIndex(UnpackedIndexes[3]);

	FHGMSIMDLibrary::Store(A, SIMDIndexes, X);
}


void FHGMSIMDLibrary::Store(TConstArrayView<FHGMSIMDReal> A, const FHGMSIMDInt& sUnpackedIndex, FHGMSIMDReal& X)
{
	TStaticArray<FHGMReal, 4> UnpackedReals {};
	FHGMSIMDLibrary::Store(A, sUnpackedIndex, UnpackedReals);
	FHGMSIMDLibrary::Load(X, UnpackedReals);
}


void FHGMSIMDLibrary::Store(const FHGMSIMDInt& A, int32& X, int32& Y, int32& Z, int32& W)
{
	alignas(FHGMSIMDInt) int32 Values[4] {};
	VectorIntStoreAligned(A, Values);
	X = Values[0];
	Y = Values[1];
	Z = Values[2];
	W = Values[3];
}


void FHGMSIMDLibrary::Store(const FHGMSIMDVector3& V, FHGMVector3& W0, FHGMVector3& W1, FHGMVector3& W2, FHGMVector3& W3)
{
	TStaticArray<FHGMReal, 4> XValues {};
	FHGMSIMDLibrary::Store(V.X, XValues);

	TStaticArray<FHGMReal, 4> YValues {};
	FHGMSIMDLibrary::Store(V.Y, YValues);

	TStaticArray<FHGMReal, 4> ZValues {};
	FHGMSIMDLibrary::Store(V.Z, ZValues);

	W0 = FHGMVector3(XValues[0], YValues[0], ZValues[0]);
	W1 = FHGMVector3(XValues[1], YValues[1], ZValues[1]);
	W2 = FHGMVector3(XValues[2], YValues[2], ZValues[2]);
	W3 = FHGMVector3(XValues[3], YValues[3], ZValues[3]);
}


void FHGMSIMDLibrary::Store(const FHGMSIMDVector3& V, uint32 ComponentIndex, FHGMVector3& W)
{
#if HGM_USE_FLOAT32
	FVector3f Wf {};
	FHGMSIMDLibrary::Store(V.X, ComponentIndex, Wf.X);
	FHGMSIMDLibrary::Store(V.Y, ComponentIndex, Wf.Y);
	FHGMSIMDLibrary::Store(V.Z, ComponentIndex, Wf.Z);

	W.X = StaticCast<FHGMReal>(Wf.X);
	W.Y = StaticCast<FHGMReal>(Wf.Y);
	W.Z = StaticCast<FHGMReal>(Wf.Z);
#else
	FHGMSIMDLibrary::Store(V.X, ComponentIndex, W.X);
	FHGMSIMDLibrary::Store(V.Y, ComponentIndex, W.Y);
	FHGMSIMDLibrary::Store(V.Z, ComponentIndex, W.Z);
#endif
}


void FHGMSIMDLibrary::Store(TArrayView<const FHGMSIMDVector3> V, const TStaticArray<FHGMSIMDIndex, 4>& SIMDIndexes, TStaticArray<FHGMVector3, 4>& W)
{
	FHGMSIMDLibrary::Store(V[SIMDIndexes[0].PackedIndex], SIMDIndexes[0].ComponentIndex, W[0]);
	FHGMSIMDLibrary::Store(V[SIMDIndexes[1].PackedIndex], SIMDIndexes[1].ComponentIndex, W[1]);
	FHGMSIMDLibrary::Store(V[SIMDIndexes[2].PackedIndex], SIMDIndexes[2].ComponentIndex, W[2]);
	FHGMSIMDLibrary::Store(V[SIMDIndexes[3].PackedIndex], SIMDIndexes[3].ComponentIndex, W[3]);
}


void FHGMSIMDLibrary::Store(TArrayView<const FHGMSIMDVector3> V, const FHGMSIMDInt& sUnpackedIndex, TStaticArray<FHGMVector3, 4>& W)
{
	TStaticArray<int32, 4> UnpackedIndexes {};
	FHGMSIMDLibrary::Store(sUnpackedIndex, UnpackedIndexes);

	TStaticArray<FHGMSIMDIndex, 4> SIMDIndexes {};
	SIMDIndexes[0] = FHGMSIMDIndex(UnpackedIndexes[0]);
	SIMDIndexes[1] = FHGMSIMDIndex(UnpackedIndexes[1]);
	SIMDIndexes[2] = FHGMSIMDIndex(UnpackedIndexes[2]);
	SIMDIndexes[3] = FHGMSIMDIndex(UnpackedIndexes[3]);

	FHGMSIMDLibrary::Store(V, SIMDIndexes, W);
}


void FHGMSIMDLibrary::Store(TArrayView<const FHGMSIMDVector3> V, const FHGMSIMDInt& sUnpackedIndex, FHGMSIMDVector3& W)
{
	TStaticArray<FHGMVector3, 4> UnpackedVectors {};
	FHGMSIMDLibrary::Store(V, sUnpackedIndex, UnpackedVectors);
	FHGMSIMDLibrary::Load(W, UnpackedVectors);
}


void FHGMSIMDLibrary::Store(const FHGMSIMDQuaternion& sQ, TStaticArray<FHGMQuaternion, 4>& Quaternions)
{
	TStaticArray<FHGMReal, 4> XValues {};
	FHGMSIMDLibrary::Store(sQ.X, XValues);

	TStaticArray<FHGMReal, 4> YValues {};
	FHGMSIMDLibrary::Store(sQ.Y, YValues);

	TStaticArray<FHGMReal, 4> ZValues {};
	FHGMSIMDLibrary::Store(sQ.Z, ZValues);

	TStaticArray<FHGMReal, 4> WValues {};
	FHGMSIMDLibrary::Store(sQ.W, WValues);

	Quaternions[0] = FHGMQuaternion(XValues[0], YValues[0], ZValues[0], WValues[0]);
	Quaternions[1] = FHGMQuaternion(XValues[1], YValues[1], ZValues[1], WValues[1]);
	Quaternions[2] = FHGMQuaternion(XValues[2], YValues[2], ZValues[2], WValues[2]);
	Quaternions[3] = FHGMQuaternion(XValues[3], YValues[3], ZValues[3], WValues[3]);
}


void FHGMSIMDLibrary::Store(const FHGMSIMDTransform& sTransform, TStaticArray<FTransform, 4>& Transforms)
{
	TStaticArray<FHGMVector3, 4> Scale3Ds {};
	FHGMSIMDLibrary::Store(sTransform.Scale3D, Scale3Ds);

	TStaticArray<FHGMQuaternion, 4> Rotations {};
	FHGMSIMDLibrary::Store(sTransform.Rotation, Rotations);

	TStaticArray<FHGMVector3, 4> Translations {};
	FHGMSIMDLibrary::Store(sTransform.Translation, Translations);

	Transforms[0] = FHGMTransform(Rotations[0], Translations[0], Scale3Ds[0]);
	Transforms[1] = FHGMTransform(Rotations[1], Translations[1], Scale3Ds[1]);
	Transforms[2] = FHGMTransform(Rotations[2], Translations[2], Scale3Ds[2]);
	Transforms[3] = FHGMTransform(Rotations[3], Translations[3], Scale3Ds[3]);
}

FHGMSIMDVector3 FHGMSIMDLibrary::Select(const FHGMSIMDReal& Mask, const FHGMSIMDVector3& V, const FHGMSIMDVector3& W)
{
	// Select by component unit
	FHGMSIMDVector3 Result {};
	FHGMSIMDLibrary::Load(Result, FHGMSIMDLibrary::Select(Mask, V.X, W.X), FHGMSIMDLibrary::Select(Mask, V.Y, W.Y), FHGMSIMDLibrary::Select(Mask, V.Z, W.Z));

	return Result;
}


FHGMSIMDQuaternion FHGMSIMDLibrary::Select(const FHGMSIMDReal& sMask, const FHGMSIMDQuaternion& sQ1, const FHGMSIMDQuaternion& sQ2)
{
	FHGMSIMDQuaternion sResult {};
	sResult.X = FHGMSIMDLibrary::Select(sMask, sQ1.X, sQ2.X);
	sResult.Y = FHGMSIMDLibrary::Select(sMask, sQ1.Y, sQ2.Y);
	sResult.Z = FHGMSIMDLibrary::Select(sMask, sQ1.Z, sQ2.Z);
	sResult.W = FHGMSIMDLibrary::Select(sMask, sQ1.W, sQ2.W);

	return sResult;
}


// ---------------------------------------------------------------------------------------
// MathLibrary
// ---------------------------------------------------------------------------------------
FHGMSIMDQuaternion FHGMMathLibrary::Slerp(const FHGMSIMDQuaternion& sQ1, const FHGMSIMDQuaternion& sQ2, const FHGMSIMDReal& sT)
{
	FHGMSIMDReal sRawCosom = sQ1.X * sQ2.X +
								sQ1.Y * sQ2.Y +
								sQ1.Z * sQ2.Z +
								sQ1.W * sQ2.W;

	const FHGMSIMDReal sSign =  FHGMSIMDLibrary::Select(sRawCosom > HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal, -HGMSIMDConstants::OneReal);
	sRawCosom *= sSign;

	FHGMSIMDReal sScale0 = HGMSIMDConstants::OneReal - sT;
	FHGMSIMDReal sScale1 = sT * sSign;

	const FHGMSIMDReal sOmega = FHGMMathLibrary::Acos(sRawCosom);
	const FHGMSIMDReal sInvSin = HGMSIMDConstants::OneReal / FHGMMathLibrary::Sin(sOmega);
	const FHGMSIMDReal sScale01 = FHGMMathLibrary::Sin(sScale0 * sOmega) * sInvSin;
	const FHGMSIMDReal sScale11 = FHGMMathLibrary::Sin(sScale1 * sOmega) * sInvSin;

	FHGMSIMDReal sThreshould {};
	FHGMSIMDLibrary::Load(sThreshould, 0.9999);

	sScale0 = FHGMSIMDLibrary::Select(sRawCosom < sThreshould, sScale01, sScale0);
	sScale1 = FHGMSIMDLibrary::Select(sRawCosom < sThreshould, sScale11, sScale1);

	return FHGMSIMDQuaternion(
			sScale0 * sQ1.X + sScale1 * sQ2.X,
			sScale0 * sQ1.Y + sScale1 * sQ2.Y,
			sScale0 * sQ1.Z + sScale1 * sQ2.Z,
			sScale0 * sQ1.W + sScale1 * sQ2.W);
}


FHGMSIMDReal FHGMMathLibrary::DotProduct(const FHGMSIMDVector3& V, const FHGMSIMDVector3& W)
{
	// Component * Component
	FHGMSIMDReal XX = V.X * W.X;
	const FHGMSIMDReal YY = V.Y * W.Y;
	const FHGMSIMDReal ZZ = V.Z * W.Z;

	// Vector0: XX + YY + ZZ
	// Vector1: XX + YY + ZZ
	// Vector2: XX + YY + ZZ
	// Vector3: XX + YY + ZZ
	XX += YY;
	XX += ZZ;

	return XX;
}


FHGMSIMDVector3 FHGMMathLibrary::CrossProduct(const FHGMSIMDVector3& V, const FHGMSIMDVector3& W)
{
	FHGMSIMDVector3 OrthogonalVector {};
	FHGMSIMDLibrary::Load(OrthogonalVector,
		V.Y * W.Z - V.Z * W.Y,	// X
		V.Z * W.X - V.X * W.Z,	// Y
		V.X * W.Y - V.Y * W.X);	// Z

	return OrthogonalVector;
}


FHGMSIMDReal FHGMMathLibrary::ReciprocalSqrt(const FHGMSIMDReal& LenghtSquared)
{
#if HGM_USE_ESTIMATED_RECIPROCAL_SQUARE_ROOT
	return VectorReciprocalSqrtEstimate(LenghtSquared);
#else
	return VectorReciprocalSqrt(LenghtSquared);
#endif
}


FHGMSIMDReal FHGMMathLibrary::ReciprocalLength(const FHGMSIMDVector3& V)
{
	return FHGMMathLibrary::ReciprocalSqrt(FHGMMathLibrary::LengthSquared(V));
}


FHGMSIMDReal FHGMMathLibrary::LengthSquared(const FHGMSIMDVector3& V)
{
	return FHGMMathLibrary::DotProduct(V, V);
}


FHGMSIMDReal FHGMMathLibrary::Length(const FHGMSIMDVector3& V)
{
	return VectorSqrt(FHGMMathLibrary::DotProduct(V, V));
}


FORCEINLINE FHGMVector3& FHGMMathLibrary::SafeNormalize(FHGMVector3& V)
{
	const FHGMReal LengthSquared = FHGMMathLibrary::DotProduct(V, V);

	if (LengthSquared == StaticCast<FHGMReal>(1.0))
	{
		return V;
	}
	else if (LengthSquared < UE_SMALL_NUMBER)
	{
		V = FHGMVector3::ZeroVector;
		return V;
	}

#if HGM_USE_ESTIMATED_RECIPROCAL_SQUARE_ROOT
	const FHGMReal InverseScale = FMath::InvSqrtEst(LengthSquared);
#else
	const FHGMReal InverseScale = FMath::InvSqrt(LengthSquared);
#endif

	V.X *= InverseScale;
	V.Y *= InverseScale;
	V.Z *= InverseScale;

	return V;
}


FHGMSIMDVector3& FHGMMathLibrary::SafeNormalize(FHGMSIMDVector3& V)
{
	const FHGMSIMDReal LengthSquared = FHGMMathLibrary::DotProduct(V, V);
	const FHGMSIMDReal SafeMask = LengthSquared > HGMSIMDConstants::SmallReal;
	const FHGMSIMDReal SafeLengthSquared = FHGMSIMDLibrary::Select(SafeMask, LengthSquared, HGMSIMDConstants::OneReal);
#if HGM_USE_ESTIMATED_RECIPROCAL_SQUARE_ROOT
	const FHGMSIMDReal InverseScale = VectorReciprocalSqrtEstimate(SafeLengthSquared);
#else
	const FHGMSIMDReal InverseScale = VectorReciprocalSqrt(SafeLengthSquared);
#endif // End of HGM_USE_ESTIMATED_RECIPROCAL_SQUARE_ROO

	V = FHGMSIMDLibrary::Select(SafeMask, V * InverseScale, FHGMSIMDVector3::ZeroVector);
	return V;
}


FHGMSIMDVector3 FHGMMathLibrary::MakeSafeNormal(const FHGMSIMDVector3& V)
{
	FHGMSIMDReal sLengthSquared = FHGMMathLibrary::DotProduct(V, V);
	const FHGMSIMDReal sSafeMask = sLengthSquared > HGMSIMDConstants::SmallReal;
	sLengthSquared = FHGMSIMDLibrary::Select(sSafeMask, sLengthSquared, HGMSIMDConstants::OneReal);

#if HGM_USE_ESTIMATED_RECIPROCAL_SQUARE_ROOT
	FHGMSIMDReal sInverseScale = VectorReciprocalSqrtEstimate(sLengthSquared);
#else
	FHGMSIMDReal sInverseScale = VectorReciprocalSqrt(sLengthSquared);
#endif

	return FHGMSIMDLibrary::Select(sSafeMask, V * sInverseScale, FHGMSIMDVector3::ZeroVector);
}


FHGMSIMDVector3 FHGMMathLibrary::MakeUnsafeNormal(const FHGMSIMDVector3& V)
{
	const FHGMSIMDReal LengthSquared = FHGMMathLibrary::DotProduct(V, V);
#if HGM_USE_ESTIMATED_RECIPROCAL_SQUARE_ROOT
	const FHGMSIMDReal InverseScale = VectorReciprocalSqrtEstimate(LengthSquared);
#else
	const FHGMSIMDReal InverseScale = VectorReciprocalSqrt(LengthSquared);
#endif

	return V * InverseScale;
}


FHGMSIMDQuaternion FHGMMathLibrary::SafeNormalize(FHGMSIMDQuaternion& sQ)
{
	FHGMSIMDReal sSquareSum = sQ.X * sQ.X + sQ.Y * sQ.Y + sQ.Z * sQ.Z + sQ.W * sQ.W;
	const FHGMSIMDReal sSafeMask = sSquareSum > HGMSIMDConstants::SmallReal;
	sSquareSum = FHGMSIMDLibrary::Select(sSafeMask, sSquareSum, HGMSIMDConstants::OneReal);
	const FHGMSIMDReal sScale = FHGMMathLibrary::ReciprocalSqrt(sSquareSum);
	sQ.X *= sScale;
	sQ.Y *= sScale;
	sQ.Z *= sScale;
	sQ.W *= sScale;

	sQ = FHGMSIMDLibrary::Select(sSafeMask, sQ, FHGMSIMDQuaternion::Identity);

	return sQ;
}


FHGMQuaternion FHGMMathLibrary::MakeLookAtQuaternion(const FHGMVector3& Forward, const FHGMVector3& UpVector)
{
	const FHGMVector3 Right = FHGMMathLibrary::MakeSafeNormal(FHGMMathLibrary::CrossProduct(UpVector, Forward));
	const FHGMVector3 Up = FHGMMathLibrary::MakeSafeNormal(FHGMMathLibrary::CrossProduct(Forward, Right));
	const FHGMMatrix RotationMatrix(Forward, Right, Up, FHGMVector3::ZeroVector);

	FHGMQuaternion Result(RotationMatrix);
	Result.Normalize();

	return Result;
}


FHGMSIMDVector3 FHGMMathLibrary::GetSafeScaleReciprocal(const FHGMSIMDVector3& Scale3D)
{
	FHGMSIMDVector3 sSafeReciprocalScale {};
	sSafeReciprocalScale.X = FHGMSIMDLibrary::Select(FHGMMathLibrary::Abs(Scale3D.X) <= HGMSIMDConstants::SmallReal, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal / Scale3D.X);
	sSafeReciprocalScale.Y = FHGMSIMDLibrary::Select(FHGMMathLibrary::Abs(Scale3D.Y) <= HGMSIMDConstants::SmallReal, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal / Scale3D.Y);
	sSafeReciprocalScale.Z = FHGMSIMDLibrary::Select(FHGMMathLibrary::Abs(Scale3D.Z) <= HGMSIMDConstants::SmallReal, HGMSIMDConstants::ZeroReal, HGMSIMDConstants::OneReal / Scale3D.Z);

	return sSafeReciprocalScale;
}


FHGMSIMDVector3 FHGMMathLibrary::RotateVector(const FHGMSIMDQuaternion& Q, const FHGMSIMDVector3& V)
{
	// See Quat.h
	FHGMSIMDVector3 QVector {};
	FHGMSIMDLibrary::Load(QVector, Q.X, Q.Y, Q.Z);
	const FHGMSIMDVector3 TT = HGMSIMDConstants::TwoReal * FHGMMathLibrary::CrossProduct(QVector, V);
	return V + (Q.W * TT) + FHGMMathLibrary::CrossProduct(QVector, TT);
}


FHGMSIMDVector3 FHGMMathLibrary::UnrotateVector(const FHGMSIMDQuaternion& Q, const FHGMSIMDVector3& V)
{
	// See Quat.h
	FHGMSIMDVector3 sQ {};
	sQ.X = -Q.X;
	sQ.Y = -Q.Y;
	sQ.Z = -Q.Z;

	const FHGMSIMDVector3 sTT = HGMSIMDConstants::TwoReal * FHGMMathLibrary::CrossProduct(sQ, V);
	const FHGMSIMDVector3 sResult = V + (Q.W * sTT) + FHGMMathLibrary::CrossProduct(sQ, sTT);

	return sResult;
}


FHGMSIMDVector3 FHGMMathLibrary::RotateAngleAxisRad(const FHGMSIMDVector3& sVector, const FHGMSIMDReal& sAngleRad, const FHGMSIMDVector3& sAxis)
{
	// Copy TVector<T>::RotateAngleAxisRad().
	FHGMSIMDReal sS, sC;
	FHGMMathLibrary::SinCos(&sS, &sC, &sAngleRad);

	const FHGMSIMDReal sXX	= sAxis.X * sAxis.X;
	const FHGMSIMDReal sYY	= sAxis.Y * sAxis.Y;
	const FHGMSIMDReal sZZ	= sAxis.Z * sAxis.Z;

	const FHGMSIMDReal sXY	= sAxis.X * sAxis.Y;
	const FHGMSIMDReal sYZ	= sAxis.Y * sAxis.Z;
	const FHGMSIMDReal sZX	= sAxis.Z * sAxis.X;

	const FHGMSIMDReal sXS	= sAxis.X * sS;
	const FHGMSIMDReal sYS	= sAxis.Y * sS;
	const FHGMSIMDReal sZS	= sAxis.Z * sS;

	const FHGMSIMDReal sOMC	= HGMSIMDConstants::OneReal - sC;

	return FHGMSIMDVector3(
		(sOMC * sXX + sC) * sVector.X + (sOMC * sXY - sZS) * sVector.Y + (sOMC * sZX + sYS) * sVector.Z,
		(sOMC * sXY + sZS) * sVector.X + (sOMC * sYY + sC) * sVector.Y + (sOMC * sYZ - sXS) * sVector.Z,
		(sOMC * sZX - sYS) * sVector.X + (sOMC * sYZ + sXS) * sVector.Y + (sOMC * sZZ + sC) * sVector.Z
	);
}


FHGMSIMDVector3 FHGMMathLibrary::GetUnitAxis(const FHGMSIMDQuaternion& sQ, const FHGMSIMDInt& sAxisIndex)
{
	TStaticArray<FHGMQuaternion, 4> UnpackedQuaternions {};
	FHGMSIMDLibrary::Store(sQ, UnpackedQuaternions);

	TStaticArray<int32, 4> UnpackedAxisIndexes {};
	FHGMSIMDLibrary::Store(sAxisIndex, UnpackedAxisIndexes);

	TStaticArray<FHGMVector3, 4> UnpackedAxes {};
	for (int32 ComponentIndex = 0; ComponentIndex < 4; ++ComponentIndex)
	{
		EAxis::Type AxisType = StaticCast<EAxis::Type>(UnpackedAxisIndexes[ComponentIndex]);

		switch (AxisType)
		{
			case EAxis::X:
			UnpackedAxes[ComponentIndex] = UnpackedQuaternions[ComponentIndex].GetAxisX();
			break;

			case EAxis::Y:
			UnpackedAxes[ComponentIndex] = UnpackedQuaternions[ComponentIndex].GetAxisY();
			break;

			case EAxis::Z:
			UnpackedAxes[ComponentIndex] = UnpackedQuaternions[ComponentIndex].GetAxisZ();
			break;

			default:
			UnpackedAxes[ComponentIndex] = FHGMVector3::ZeroVector;
			break;
		}
	}

	FHGMSIMDVector3 sResult {};
	FHGMSIMDLibrary::Load(sResult, UnpackedAxes);

	return sResult;
}


FHGMSIMDQuaternion FHGMMathLibrary::Inverse(const FHGMSIMDQuaternion& Q)
{
	return { -Q.X, -Q.Y, -Q.Z, Q.W };
}


FHGMSIMDTransform FHGMMathLibrary::Inverse(const FHGMSIMDTransform& Transform)
{
	const FHGMSIMDQuaternion InvRotation = FHGMMathLibrary::Inverse(Transform.Rotation);
	const FHGMSIMDVector3 InvScale3D = FHGMMathLibrary::GetSafeScaleReciprocal(Transform.Scale3D);
	const FHGMSIMDVector3 InvTranslationNoScale = InvScale3D * -Transform.Translation;
	FHGMSIMDVector3 InvTranslation = FHGMMathLibrary::RotateVector(InvRotation, InvTranslationNoScale);

	return { InvScale3D, InvRotation, InvTranslation };
}


FHGMSIMDVector3 FHGMMathLibrary::PointPlaneProject(const FHGMSIMDVector3& sV, const FHGMSIMDVector3& sPlaneOrigin, const FHGMSIMDVector3& sPlaneNormal)
{
	const FHGMSIMDReal sPlaneProjection = FHGMMathLibrary::DotProduct(sPlaneNormal, sPlaneOrigin);
	const FHGMSIMDReal sPlaneDot = FHGMMathLibrary::DotProduct(sPlaneNormal, sV) - sPlaneProjection;

	return sV - (sPlaneDot) * sPlaneNormal;
}
