// Fill out your copyright notice in the Description page of Project Settings.

#include "Common/Structs/GameplayDateTime.h"

// === FGameplayTime ===

uint32 FGameplayTime::Normalize()
{
	uint32 OverflowDays = 0;
    
	if (Minute > MaxMinute)
	{
		Hour += Minute / MaxMinutePlusOne;
		Minute %= MaxMinutePlusOne;
	}

	if (Hour > MaxHour)
	{
		OverflowDays = Hour / MaxHourPlusOne;
		Hour %= MaxHourPlusOne;
	}

	return OverflowDays;
}

FGameplayTime FGameplayTime::operator+(const FGameplayTime& Other) const
{
	FGameplayTime Result = *this;

	Result.Hour += Other.Hour;
	Result.Minute += Other.Minute;

	return Result;
}

FGameplayTime FGameplayTime::operator-(const FGameplayTime& Other) const
{
	const uint16 Diff = FMath::Max(0, ToTotalMinutes() - Other.ToTotalMinutes());

	FGameplayTime Result;
	Result.FromTotalMinutes(Diff);

	return Result;
}

bool FGameplayTime::operator<(const FGameplayTime& Other) const
{
	if (Hour != Other.Hour)
	{
		return Hour < Other.Hour;
	}

	return Minute < Other.Minute;
}

bool FGameplayTime::operator>(const FGameplayTime& Other) const
{
	if (Hour != Other.Hour)
	{
		return Hour > Other.Hour;
	}

	return Minute > Other.Minute;
}

void FGameplayTime::FromTotalMinutes(uint16 TotalMinutes)
{
	TotalMinutes = FMath::Max<uint64>(0, TotalMinutes);

	Hour = TotalMinutes / MaxMinutePlusOne % MaxHourPlusOne;
	Minute = TotalMinutes % MaxMinutePlusOne;
}

// === FGameplayDateTime ===

FGameplayDateTime FGameplayDateTime::operator+(const FGameplayDateTime& Other) const
{
	FGameplayDateTime Result = *this;

	Result.Day += Other.Day;
	Result.Time += Other.Time;

	// Normalize the time and add its overflow days to the day
	const uint32 OverflowDays = Result.Time.Normalize();
	Result.Day += OverflowDays;

	return Result;
}

FGameplayDateTime FGameplayDateTime::operator-(const FGameplayDateTime& Other) const
{
	const uint64 Diff = FMath::Max<uint64>(0, ToTotalMinutes() - Other.ToTotalMinutes());

	FGameplayDateTime Result;
	Result.FromTotalMinutes(Diff);

	return Result;
}

bool FGameplayDateTime::operator<(const FGameplayDateTime& Other) const
{
	if (Day != Other.Day)
	{
		return Day < Other.Day;
	}

	return Time < Other.Time;
}

bool FGameplayDateTime::operator>(const FGameplayDateTime& Other) const
{
	if (Day != Other.Day)
	{
		return Day > Other.Day;
	}

	return Time > Other.Time;
}

void FGameplayDateTime::FromTotalMinutes(uint64 TotalMinutes)
{
	TotalMinutes = FMath::Max<uint64>(0, TotalMinutes);

	Day = TotalMinutes / MaxMinutesInDay;
	TotalMinutes %= MaxMinutesInDay;

	Time.FromTotalMinutes(TotalMinutes);
}