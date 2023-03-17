// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabberPawn.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
UGrabberPawn::UGrabberPawn()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGrabberPawn::BeginPlay()
{
	Super::BeginPlay();

	// ...
	UInputComponent* inputComponent = this->GetOwner()->FindComponentByClass<UInputComponent>();
	inputComponent->BindAction("Grab", IE_Pressed, this, &UGrabberPawn::Grab);
	//->BindAction("Grab", IE_Released, this, &UGrabberPawn::Release);
	inputComponent->BindAction("Throw", IE_Pressed, this, &UGrabberPawn::Throw);

	this->grabbedObjAnchor = this->GetOwner()->FindComponentByClass<USceneComponent>()->GetChildComponent(0);
}


// Called every frame
void UGrabberPawn::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	FVector location;
	FRotator rotation;
	this->GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(location, rotation);

	FVector lineTraceEnd = location + rotation.Vector() * this->REACH;

	if (this->grabbedActor != nullptr)
	{
		if (this->physicsHandle != nullptr)
		{
			if (this->grabbedObjAnchor != nullptr)
			{
				FVector anchor = this->grabbedObjAnchor->GetComponentLocation();
				this->physicsHandle->SetTargetLocation(anchor);

				UE_LOG(LogTemp, Warning, TEXT("Anchor position: %f, %f, %f"), anchor.X, anchor.Y, anchor.Z);
				UE_LOG(LogTemp, Warning, TEXT("Name: %s"), *this->grabbedObjAnchor->GetName());
			}
			else
				this->physicsHandle->SetTargetLocation(lineTraceEnd);
		}
	}
}

void UGrabberPawn::Grab()
{
	FVector location;
	FRotator rotation;
	this->GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(location, rotation);

	FVector lineTraceEnd = location + rotation.Vector() * this->REACH;
	DrawDebugLine(this->GetWorld(), location, lineTraceEnd, FColor::Red, false, 2.0, 0, 5.0f);

	FHitResult hitResult;
	FCollisionQueryParams traceParams(FName(TEXT("myQuery")), false, this->GetOwner());
	this->GetWorld()->LineTraceSingleByObjectType(hitResult, location, lineTraceEnd,
		FCollisionObjectQueryParams(ECC_PhysicsBody), traceParams);
	this->grabbedActor = hitResult.GetActor();
	if (this->grabbedActor != nullptr)
	{
		this->hasGrabbed = true;
		this->primitiveComp = hitResult.GetComponent();

		this->physicsHandle = this->grabbedActor->FindComponentByClass<UPhysicsHandleComponent>();
		if (this->physicsHandle != nullptr)
		{
			this->primitiveComp->AddForceAtLocation(lineTraceEnd * 1000.0f, lineTraceEnd, EName::None);
			this->physicsHandle->GrabComponentAtLocation(this->primitiveComp, EName::None, lineTraceEnd);
		}
		else
		{
			this->grabbedActor->SetActorLocation(lineTraceEnd);
		}
	}
}

void UGrabberPawn::Release()
{
	this->hasGrabbed = false;
	this->grabbedActor = nullptr;

	if (this->physicsHandle != nullptr)
	{
		this->physicsHandle->ReleaseComponent();
	}
}

void UGrabberPawn::Throw()
{
	if (this->grabbedActor != nullptr)
	{
		if (this->physicsHandle != nullptr)
		{
			float tempReach = 2500.0f;
			float throwStrength = 250000.0f * this->primitiveComp->GetMass();

			FVector location;
			FRotator rotation;
			this->GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(location, rotation);
			FVector lineTraceEnd = location + (rotation.Vector() * tempReach);
			DrawDebugLine(this->GetWorld(), location, lineTraceEnd, FColor::Green, false, 5.0, 0, 5.0f);

			FVector location2 = this->grabbedActor->GetActorLocation();
			FVector direction1 = UKismetMathLibrary::GetDirectionUnitVector(location2, lineTraceEnd);
			FVector lineTraceEnd2 = location2 + (direction1 * throwStrength);
			DrawDebugLine(this->GetWorld(), location2, lineTraceEnd2, FColor::Blue, false, 3.0, 0, 5.0f);

			this->primitiveComp->AddForce(lineTraceEnd2, EName::None);

			Release();
		}
	}
}

