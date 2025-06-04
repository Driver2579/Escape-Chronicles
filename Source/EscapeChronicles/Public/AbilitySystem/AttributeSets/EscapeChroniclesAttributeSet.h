// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Components/AbilitySystemComponents/EscapeChroniclesAbilitySystemComponent.h"
#include "EscapeChroniclesAttributeSet.generated.h"

/**
 * This macro defines a set of helper functions for accessing and initializing attributes.
 *
 * The following example of the macro:\n
 *		ATTRIBUTE_ACCESSORS(ThisClass, Health)\n
 * will create the following functions:\n
 *		static FGameplayAttribute GetHealthAttribute();\n
 *		float GetHealth() const;\n
 *		void SetHealth(float NewVal);\n
 *		void InitHealth(float NewVal);
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	ATTRIBUTE_GETTERS(ClassName, PropertyName) \
	ATTRIBUTE_SETTERS(ClassName, PropertyName)

/**
 * This macro defines a set of helper functions for getting attributes.
 *
 * The following example of the macro:\n
 *		ATTRIBUTE_GETTERS(ThisClass, Health)\n
 * will create the following functions:\n
 *		static FGameplayAttribute GetHealthAttribute();\n
 *		float GetHealth() const;
 */
#define ATTRIBUTE_GETTERS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)

/**
 * This macro defines a set of helper functions for setting attributes and initializing.
 *
 * The following example of the macro:\n
 *		ATTRIBUTE_SETTERS(ThisClass, Health)\n
 * will create the following functions:\n 
 *		void SetHealth(float NewVal);\n
 *		void InitHealth(float NewVal);
 */
#define ATTRIBUTE_SETTERS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// Base attribute set class in this project
UCLASS()
class ESCAPECHRONICLES_API UEscapeChroniclesAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UEscapeChroniclesAbilitySystemComponent* GetOwningEscapeChroniclesAbilitySystemComponent() const
	{
		return CastChecked<UEscapeChroniclesAbilitySystemComponent>(GetOwningAbilitySystemComponent(),
			ECastCheckedType::NullAllowed);
	}

protected:
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/**
	 * Always called on PreAttributeChange. Should be overriden by clients to clamp attribute values between allowed
	 * ranges.\n
	 * For example, to make sure health isn't less than 0 and isn't more than the maximum health.
	 */
	virtual void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const {}
};