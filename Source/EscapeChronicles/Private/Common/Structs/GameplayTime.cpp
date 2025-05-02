// Fill out your copyright notice in the Description page of Project Settings.

#include "Common/Structs/GameplayTime.h"

void FGameplayTime::Normalize()
{
	if (Minute > MaxMinute)
	{
		Hour += Minute / MaxMinutePlusOne;
		Minute %= MaxMinutePlusOne;
	}
	else
	{
		while (Minute > MaxMinute)
		{
			Minute -= MaxMinutePlusOne;
			++Hour;
		}
	}

	if (Hour > MaxHour)
	{
		Day += Hour / MaxHourPlusOne;
		Hour %= MaxHourPlusOne;
	}
	else
	{
		while (Hour > MaxHour)
		{
			Hour -= MaxHourPlusOne;
			++Day;
		}
	}
}

FGameplayTime FGameplayTime::operator+(const FGameplayTime& Other) const
{
	FGameplayTime Result = *this;

	Result.Day += Other.Day;
	Result.Hour += Other.Hour;
	Result.Minute += Other.Minute;

	Result.Normalize();

	return Result;
}

FGameplayTime FGameplayTime::operator-(const FGameplayTime& Other) const
{
	FGameplayTime Result = *this;

	const int32 TotalMinutesThis = Day * MaxHourPlusOne * MaxMinutePlusOne + Hour * MaxMinutePlusOne + Minute;
	const int32 TotalMinutesOther = Other.Day * MaxHourPlusOne * MaxMinutePlusOne + Other.Hour * MaxMinutePlusOne +
		Other.Minute;

	const int32 Diff = FMath::Max(0, TotalMinutesThis - TotalMinutesOther);
	Result.FromTotalMinutes(Diff);

	return Result;
}

bool FGameplayTime::operator<(const FGameplayTime& Other) const
{
	if (Day != Other.Day)
	{
		return Day < Other.Day;
	}

	if (Hour != Other.Hour)
	{
		return Hour < Other.Hour;
	}

	return Minute < Other.Minute;
}

bool FGameplayTime::operator>(const FGameplayTime& Other) const
{
	if (Day != Other.Day)
	{
		return Day > Other.Day;
	}

	if (Hour != Other.Hour)
	{
		return Hour > Other.Hour;
	}

	return Minute > Other.Minute;
}

void FGameplayTime::FromTotalMinutes(uint64 TotalMinutes)
{
	TotalMinutes = FMath::Max<uint64>(0, TotalMinutes);

	Day = TotalMinutes / (MaxHourPlusOne * MaxMinutePlusOne);
	TotalMinutes %= MaxHourPlusOne * MaxMinutePlusOne;
	Hour = TotalMinutes / MaxMinutePlusOne;
	Minute = TotalMinutes % MaxMinutePlusOne;
}