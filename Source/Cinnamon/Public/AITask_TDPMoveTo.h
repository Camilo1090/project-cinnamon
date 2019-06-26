// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "AITypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "Tasks/AITask.h"
#include "ThreadSafeBool.h"
#include "TDPDefinitions.h"
#include "AITask_TDPMoveTo.generated.h"

class AAIController;
class UTDPNavigationComponent;
class TDPNavigationPath;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTDPMoveTaskCompletedSignature, TEnumAsByte<EPathFollowingResult::Type>, Result, AAIController*, AIController);

/**
 * 
 */
UCLASS()
class CINNAMON_API UAITask_TDPMoveTo : public UAITask
{
	GENERATED_BODY()

public:
	UAITask_TDPMoveTo(const FObjectInitializer& ObjectInitializer);

	/** tries to start move request and handles retry timer */
	void ConditionalPerformMove();

	/** prepare move task for activation */
	void SetUp(AAIController* Controller, const FAIMoveRequest& InMoveRequest, bool UseAsyncPathfinding);

	EPathFollowingResult::Type GetMoveResult() const { return MoveResult; }
	bool WasMoveSuccessful() const { return MoveResult == EPathFollowingResult::Success; }

	UFUNCTION(BlueprintCallable, Category = "AI|Tasks", meta = (AdvancedDisplay = "AcceptanceRadius,StopOnOverlap,AcceptPartialPath,bUsePathfinding,bUseContinuosGoalTracking", DefaultToSelf = "Controller", BlueprintInternalUseOnly = "TRUE", DisplayName = "TDP Move To Location or Actor"))
		static UAITask_TDPMoveTo* TDPAIMoveTo(AAIController* Controller, FVector GoalLocation, AActor* GoalActor = nullptr,
			float AcceptanceRadius = -1.f, EAIOptionFlag::Type StopOnOverlap = EAIOptionFlag::Default, EAIOptionFlag::Type AcceptPartialPath = EAIOptionFlag::Default, 
			bool bLockAILogic = true, bool bUseContinuosGoalTracking = false, bool UseAsyncPathfinding = false);

	UE_DEPRECATED(4.12, "This function is now depreacted, please use version with FAIMoveRequest parameter")
		void SetUp(AAIController* Controller, FVector GoalLocation, AActor* GoalActor = nullptr, float AcceptanceRadius = -1.f, EAIOptionFlag::Type StopOnOverlap = EAIOptionFlag::Default);

	/** Allows custom move request tweaking. Note that all MoveRequest need to
	*	be performed before PerformMove is called. */
	FAIMoveRequest& GetMoveRequestRef() { return MoveRequest; }

	/** Switch task into continuous tracking mode: keep restarting move toward goal actor. Only pathfinding failure or external cancel will be able to stop this task. */
	void SetContinuousGoalTracking(bool bEnable);

	virtual void TickTask(float DeltaTime) override;

protected:
	void LogPathHelper();

	FThreadSafeBool mAsyncTaskComplete;
	bool mUseAsyncPathfinding;

	UPROPERTY(BlueprintAssignable)
	FGenericGameplayTaskDelegate OnRequestFailed;

	UPROPERTY(BlueprintAssignable)
	FTDPMoveTaskCompletedSignature OnMoveFinished;

	/** parameters of move request */
	UPROPERTY()
	FAIMoveRequest MoveRequest;

	/** handle of path following's OnMoveFinished delegate */
	FDelegateHandle PathFinishDelegateHandle;

	/** handle of path's update event delegate */
	FDelegateHandle PathUpdateDelegateHandle;

	/** handle of active ConditionalPerformMove timer  */
	FTimerHandle MoveRetryTimerHandle;

	/** handle of active ConditionalUpdatePath timer */
	FTimerHandle PathRetryTimerHandle;

	/** request ID of path following's request */
	FAIRequestID MoveRequestID;

	/** currently followed path */
	FNavPathSharedPtr Path;

	TSharedPtr<TDPNavigationPath> mPath = nullptr;

	TEnumAsByte<EPathFollowingResult::Type> MoveResult;
	uint8 bUseContinuousTracking : 1;

	virtual void Activate() override;
	virtual void OnDestroy(bool bOwnerFinished) override;

	virtual void Pause() override;
	virtual void Resume() override;

	/** finish task */
	void FinishMoveTask(EPathFollowingResult::Type InResult);

	/** stores path and starts observing its events */
	void SetObservedPath(FNavPathSharedPtr InPath);

	//FPathFollowingRequestResult myResult;

	FTDPPathfindingRequestResult mResult;

	UTDPNavigationComponent* mNavigationComponent;

	void CheckPathPreConditions();

	void RequestPathSynchronous();
	void RequestPathAsync();

	void RequestMove();

	void HandleAsyncPathTaskComplete();

	void ResetPaths();

	/** remove all delegates */
	virtual void ResetObservers();

	/** remove all timers */
	virtual void ResetTimers();

	/** tries to update invalidated path and handles retry timer */
	void ConditionalUpdatePath();

	/** start move request */
	virtual void PerformMove();

	/** event from followed path */
	virtual void OnPathEvent(FNavigationPath* InPath, ENavPathEvent::Type Event);

	/** event from path following */
	virtual void OnRequestFinished(FAIRequestID RequestID, const FPathFollowingResult& Result);
};
