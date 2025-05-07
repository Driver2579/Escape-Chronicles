// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayDateTime.generated.h"

// A struct for the time used in the game. It has a day, hour, and minute. This struct also supports the SaveGame macro.
USTRUCT()
struct FGameplayTime
{
	GENERATED_BODY()

	FGameplayTime() = default;

	FGameplayTime(const uint8 InHour, const uint8 InMinute)
		: Hour(FMath::Min<uint8>(InHour, MaxHour))
		, Minute(FMath::Min<uint8>(InMinute, MaxMinute))
	{
	}

	UPROPERTY(EditAnywhere, SaveGame, meta=(ClampMax=23))
	uint8 Hour = 0;

	UPROPERTY(EditAnywhere, SaveGame, meta=(ClampMax=59))
	uint8 Minute = 0;

	/**
	 * Normalizes the time to the correct format.
	 * @return Overflow days. For example, if the time is 25:00, it will be normalized to 01:00 and return 1.
	 */
	uint32 Normalize();

	/**
	 * Adds the given time to this time WITHOUT normalizing it.
	 * @remark You should call Normalize() after this operation to get the correct time.
	 */
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
		return Hour == Other.Hour && Minute == Other.Minute;
	}

	bool operator!=(const FGameplayTime& Other) const { return !(*this == Other); }

	bool operator<(const FGameplayTime& Other) const;
	bool operator>(const FGameplayTime& Other) const;

	bool operator<=(const FGameplayTime& Other) const { return *this < Other || *this == Other; }
	bool operator>=(const FGameplayTime& Other) const { return *this > Other || *this == Other; }

	void FromTotalMinutes(uint16 TotalMinutes);

	uint16 ToTotalMinutes() const
	{
		return Hour * MaxMinutePlusOne + Minute;
	}

	static constexpr uint8 MaxMinute = 59; 
	static constexpr uint8 MaxHour = 23;

	static constexpr uint8 MaxMinutePlusOne = MaxMinute + 1;
	static constexpr uint8 MaxHourPlusOne = MaxHour + 1;
};

// This is required to use FGameplayTime as a key in TMap and TSet
FORCEINLINE uint32 GetTypeHash(const FGameplayTime& GameplayTime)
{
	return FCrc::MemCrc32(&GameplayTime, sizeof(GameplayTime));
}

/**
 * A struct for the date and time used in the game. It has both day and time. This struct also supports the SaveGame
 * macro.
 */
USTRUCT()
struct FGameplayDateTime
{
	GENERATED_BODY()

	FGameplayDateTime() = default;

	FGameplayDateTime(const uint32 InDay, const FGameplayTime InTime)
		: Day(InDay)
		, Time(InTime)
	{
	}

	FGameplayDateTime(const uint32 InDay, const uint8 InHour, const uint8 InMinute)
		: Day(InDay)
		, Time(InHour, InMinute)
	{
	}

	UPROPERTY(EditAnywhere, SaveGame)
	uint32 Day = 0;

	UPROPERTY(EditAnywhere, SaveGame)
	FGameplayTime Time;

	FGameplayDateTime operator+(const FGameplayDateTime& Other) const;
	FGameplayDateTime operator-(const FGameplayDateTime& Other) const;

	FGameplayDateTime& operator+=(const FGameplayDateTime& Other)
	{
		*this = *this + Other;

		return *this;
	}

	FGameplayDateTime& operator-=(const FGameplayDateTime& Other)
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
	FGameplayDateTime& operator++()
	{
		AddMinutes(1);

		return *this;
	}

	// Postfix increment
	FGameplayDateTime& operator++(int)
	{
		FGameplayDateTime OldGameplayTime = *this;
		++*this;

		return OldGameplayTime;
	}

	// Prefix decrement
	FGameplayDateTime& operator--()
	{
		SubtractMinutes(1);

		return *this;
	}

	// Postfix decrement
	FGameplayDateTime& operator--(int)
	{
		FGameplayDateTime OldGameplayTime = *this;
		--*this;

		return OldGameplayTime;
	}

	bool operator==(const FGameplayDateTime& Other) const
	{
		return Day == Other.Day && Time == Other.Time;
	}

	bool operator!=(const FGameplayDateTime& Other) const { return !(*this == Other); }

	bool operator<(const FGameplayDateTime& Other) const;
	bool operator>(const FGameplayDateTime& Other) const;

	bool operator<=(const FGameplayDateTime& Other) const { return *this < Other || *this == Other; }
	bool operator>=(const FGameplayDateTime& Other) const { return *this > Other || *this == Other; }

	void FromTotalMinutes(uint64 TotalMinutes);

	uint64 ToTotalMinutes() const
	{
		return Day * MaxMinutesInDay + Time.ToTotalMinutes();
	}

	static constexpr uint16 MaxMinutesInDay = FGameplayTime::MaxHourPlusOne * FGameplayTime::MaxMinutePlusOne;
};

// This is required to use FGameplayDateTime as a key in TMap and TSet
FORCEINLINE uint32 GetTypeHash(const FGameplayDateTime& GameplayDateTime)
{
	return FCrc::MemCrc32(&GameplayDateTime, sizeof(GameplayDateTime));
}