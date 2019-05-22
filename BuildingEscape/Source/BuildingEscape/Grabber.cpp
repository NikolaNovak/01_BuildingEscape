// Copyright Nikola Novak

#include "Grabber.h"

#define OUT

// Sets default values for this component's properties
UGrabber::UGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UGrabber::BeginPlay()
{
	Super::BeginPlay();
	FindPhysicsHandleComponent();
	SetupInputComponent();
}

// Called every frame
void UGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PhysicsHandle) { return; }
	//if the physics handle is attached
	if (PhysicsHandle->GrabbedComponent)
	{
		//move the object we are holding
		PhysicsHandle->SetTargetLocation(GetLineTracePoints().v2);
	}
}

void UGrabber::Grab()
{
	//LINE TRACE and see if we reach any actors with physics body collision channel set
	auto HitResult = GetFirstPhysicsBodyInReach();
	auto ComponentToGrab = HitResult.GetComponent(); //gets the mesh in our case
	auto ActorHit = HitResult.GetActor();

	//if we hit something then attach a physics handle
	if (ActorHit)
	{
		PhysicsHandle->GrabComponentAtLocation(
			ComponentToGrab, //ComponentToGrab
			NAME_None, //grab what bone name, if any
			ComponentToGrab->GetOwner()->GetActorLocation()//grab location
		);
	}
}

void UGrabber::Release()
{
	PhysicsHandle->ReleaseComponent();
}

void UGrabber::FindPhysicsHandleComponent()
{
	PhysicsHandle = GetOwner()->FindComponentByClass<UPhysicsHandleComponent>();
	if (PhysicsHandle == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s doesn't have a PhysicsHandle"), *GetOwner()->GetName());
	}
}

void UGrabber::SetupInputComponent()
{
	InputComponent = GetOwner()->FindComponentByClass<UInputComponent>();
	if (InputComponent)
	{
		InputComponent->BindAction("Grab", IE_Pressed, this, &UGrabber::Grab);
		InputComponent->BindAction("Grab", IE_Released, this, &UGrabber::Release);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s doesn't have a PhysicsHandle"), *GetOwner()->GetName());
	}
}

FHitResult UGrabber::GetFirstPhysicsBodyInReach() const
{
	/// Line-trace (AKA ray-cast) out to reach distance
	FHitResult HitResult;
	FCollisionQueryParams TraceParameters(FName(TEXT("")), false, GetOwner());
	FTwoVectors TracePoints = GetLineTracePoints();
	//with C++17
	//auto [Start, End] = GetLineTracePoints();
	GetWorld()->LineTraceSingleByObjectType(
		OUT HitResult,
		TracePoints.v1, //Start
		TracePoints.v2, //End
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParameters
	);
	return HitResult;
}

FTwoVectors UGrabber::GetLineTracePoints() const
{
	FVector StartLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT StartLocation,
		OUT PlayerViewPointRotation
	);
	FVector EndLocation = StartLocation + PlayerViewPointRotation.Vector() * Reach;
	return FTwoVectors(StartLocation, EndLocation);
}
