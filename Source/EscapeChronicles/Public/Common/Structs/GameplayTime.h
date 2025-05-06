// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayTime.generated.h"

// A struct for the time used in the game. It has a day, hour, and minute. This struct also supports the SaveGame macro.
USTRUCT()
struct FGameplayTime
{
	GENERATED_BODY()

	FGameplayTime() = default;

	FGameplayTime(const uint32 InDay, const uint8 InHour, const uint8 InMinute)
		: Day(InDay)
		, Hour(FMath::Min<uint8>(InHour, MaxHour))
		, Minute(FMath::Min<uint8>(InMinute, MaxMinute))
	{
	}

	UPROPERTY(EditAnywhere, SaveGame)
	uint32 Day = 0;

	UPROPERTY(EditAnywhere, SaveGame, meta=(ClampMax=23))
	uint8 Hour = 0;

	UPROPERTY(EditAnywhere, SaveGame, meta=(ClampMax=59))
	uint8 Minute = 0;

	FGameplayTime operator+(const FGameplayTime& Other) const;
	FGameplayTime operator-(const FGameplayTime& Other) const;

	FGameplayTime& operator+=(const FGameplayTime& Other)
	{
		*this = *this + Other;

		return *this;
	}

	FGameplayTime& operator-=(const FGameplayTime& Other)
	{
		*this = *this - Other;

		return *this;
	}

	void AddMinutes(const uint32 InMinutes)
	{
		FromTotalMinutes(ToTotalMinutes() + InMinutes);
	}

	void SubtractMinutes(const uint32 InMinutes)
	{
		FromTotalMinutes(FMath::Min<uint32>(0, ToTotalMinutes() - InMinutes));
	}

	// Prefix increment
	FGameplayTime& operator++()
	{
		AddMinutes(1);

		return *this;
	}

	// Postfix increment
	FGameplayTime& operator++(int)
	{
		FGameplayTime OldGameplayTime = *this;
		++*this;

		return OldGameplayTime;
	}

	// Prefix decrement
	FGameplayTime& operator--()
	{
		SubtractMinutes(1);

		return *this;
	}

	// Postfix decrement
	FGameplayTime& operator--(int)
	{
		FGameplayTime OldGameplayTime = *this;
		--*this;

		return OldGameplayTime;
	}

	bool operator==(const FGameplayTime& Other) const
	{
		return Day == Other.Day && Hour == Other.Hour && Minute == Other.Minute;
	}

	bool operator!=(const FGameplayTime& Other) const { return !(*this == Other); }

	bool operator<(const FGameplayTime& Other) const;
	bool operator>(const FGameplayTime& Other) const;

	bool operator<=(const FGameplayTime& Other) const { return *this < Other || *this == Other; }
	bool operator>=(const FGameplayTime& Other) const { return *this > Other || *this == Other; }

	void FromTotalMinutes(uint64 TotalMinutes);

	uint64 ToTotalMinutes() const
	{
		return Day * MaxHourPlusOne * MaxMinutePlusOne + Hour * MaxMinutePlusOne + Minute;
	}

private:
	static constexpr uint8 MaxMinute = 59; 
	static constexpr uint8 MaxHour = 23;

	static constexpr uint8 MaxMinutePlusOne = MaxMinute + 1;
	static constexpr uint8 MaxHourPlusOne = MaxHour + 1;

	void Normalize();
};