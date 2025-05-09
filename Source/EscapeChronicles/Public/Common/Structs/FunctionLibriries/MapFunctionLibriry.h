// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

struct FMapFunctionLibrary
{
	template<typename KeyType, typename ValueType, typename SetAllocator = FDefaultSetAllocator,
		typename KeyFunctions = TDefaultMapHashableKeyFuncs<KeyType,ValueType,false>>
	static bool AreMapsEqual(const TMap<KeyType, ValueType, SetAllocator, KeyFunctions>& MapA,
		const TMap<KeyType, ValueType, SetAllocator, KeyFunctions>& MapB)
	{
		// Make sure the size of the maps is the same
		if (MapA.Num() != MapB.Num())
		{
			return false;
		}

		for (const auto& PairA : MapA)
		{
			bool bFoundEqualPair = false;

			// Try to find the same pair in the MapB
			for (const auto& PairB : MapB)
			{
				if (PairA == PairB)
				{
					bFoundEqualPair = true;

					break;
				}
			}

			// If we didn't find the same pair in the MapB, then the maps are not equal
			if (!bFoundEqualPair)
			{
				return false;
			}
		}

		return true;
	}
};