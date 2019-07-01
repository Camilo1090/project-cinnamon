// Fill out your copyright notice in the Description page of Project Settings.


#include "AITask_TDPMoveTo.h"
#include "Cinnamon.h"
#include "TDPNavigationComponent.h"
#include "TDPNavigationPath.h"
#include "TDPVolume.h"
#include "UObject/Package.h"
#include "TimerManager.h"
#include "AISystem.h"
#include "AIController.h"
#include "VisualLogger/VisualLogger.h"
#include "AIResources.h"
#include "GameplayTasksComponent.h"
#include "DrawDebugHelpers.h"

UAITask_TDPMoveTo::UAITask_TDPMoveTo(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bIsPausable = true;
	MoveRequestID = FAIRequestID::InvalidRequest;

	MoveRequest.SetAcceptanceRadius(GET_AI_CONFIG_VAR(AcceptanceRadius));
	MoveRequest.SetReachTestIncludesAgentRadius(GET_AI_CONFIG_VAR(bFinishMoveOnGoalOverlap));
	MoveRequest.SetAllowPartialPath(GET_AI_CONFIG_VAR(bAcceptPartialPaths));
	MoveRequest.SetUsePathfinding(true);

	mResult.Code = ETDPPathfindingRequestResult::Failed;

	AddRequiredResource(UAIResource_Movement::StaticClass());
	AddClaimedResource(UAIResource_Movement::StaticClass());

	MoveResult = EPathFollowingResult::Invalid;
	bUseContinuousTracking = false;

	Path = MakeShareable<FNavigationPath>(new FNavigationPath());
}

UAITask_TDPMoveTo* UAITask_TDPMoveTo::TDPAIMoveTo(AAIController* Controller, FVector InGoalLocation, AActor* InGoalActor,
	float AcceptanceRadius, EAIOptionFlag::Type StopOnOverlap, EAIOptionFlag::Type AcceptPartialPath,
	bool bLockAILogic, bool bUseContinuosGoalTracking, bool UseAsyncPathfinding)
{
	UAITask_TDPMoveTo* MyTask = Controller ? UAITask::NewAITask<UAITask_TDPMoveTo>(*Controller, EAITaskPriority::High) : nullptr;
	if (MyTask)
	{
		MyTask->mUseAsyncPathfinding = UseAsyncPathfinding;
		// We need to tick the task if we're using async, to check when results are back
		MyTask->bTickingTask = UseAsyncPathfinding;

		FAIMoveRequest MoveReq;
		if (InGoalActor)
		{
			MoveReq.SetGoalActor(InGoalActor);
		}
		else
		{
			MoveReq.SetGoalLocation(InGoalLocation);
		}

		MoveReq.SetAcceptanceRadius(AcceptanceRadius);
		MoveReq.SetReachTestIncludesAgentRadius(FAISystem::PickAIOption(StopOnOverlap, MoveReq.IsReachTestIncludingAgentRadius()));
		MoveReq.SetAllowPartialPath(FAISystem::PickAIOption(AcceptPartialPath, MoveReq.IsUsingPartialPaths()));
		if (Controller)
		{
			MoveReq.SetNavigationFilter(Controller->GetDefaultNavigationFilterClass());
		}

		MyTask->SetUp(Controller, MoveReq, UseAsyncPathfinding);
		MyTask->SetContinuousGoalTracking(bUseContinuosGoalTracking);

		if (bLockAILogic)
		{
			MyTask->RequestAILogicLocking();
		}
	}

	return MyTask;
}

void UAITask_TDPMoveTo::SetUp(AAIController* Controller, const FAIMoveRequest& InMoveRequest, bool UseAsyncPathfinding)
{
	OwnerController = Controller;
	MoveRequest = InMoveRequest;
	mUseAsyncPathfinding = UseAsyncPathfinding;
	bTickingTask = UseAsyncPathfinding;

	// Fail if no nav component
	mNavigationComponent = Cast<UTDPNavigationComponent>(GetOwnerActor()->GetComponentByClass(UTDPNavigationComponent::StaticClass()));
	if (!mNavigationComponent)
	{
#if WITH_EDITOR
		UE_VLOG(this, VisualCinnamonLog, Error, TEXT("TDPMoveTo request failed due missing TDPNavigationComponent"), *MoveRequest.ToString());
		UE_LOG(CinnamonLog, Error, TEXT("TDPMoveTo request failed due missing TDPNavigationComponent on the pawn"));
		return;
#endif
	}

	// Use the path instance from the navcomponent
	//mPath = mNavigationComponent->GetPath();
}

void UAITask_TDPMoveTo::SetContinuousGoalTracking(bool bEnable)
{
	bUseContinuousTracking = bEnable;
}

void UAITask_TDPMoveTo::TickTask(float DeltaTime)
{
	if (mAsyncTaskComplete && !mNavigationComponent->GetMoveRequested())
	{
		HandleAsyncPathTaskComplete();
	}
}

void UAITask_TDPMoveTo::FinishMoveTask(EPathFollowingResult::Type InResult)
{
	if (MoveRequestID.IsValid())
	{
		UPathFollowingComponent* PFComp = OwnerController ? OwnerController->GetPathFollowingComponent() : nullptr;
		if (PFComp && PFComp->GetStatus() != EPathFollowingStatus::Idle)
		{
			ResetObservers();
			PFComp->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished, MoveRequestID);
		}
	}

	MoveResult = InResult;
	EndTask();

	if (InResult == EPathFollowingResult::Invalid)
	{
		OnRequestFailed.Broadcast();
	}
	else
	{
		OnMoveFinished.Broadcast(InResult, OwnerController);
	}
}

void UAITask_TDPMoveTo::Activate()
{
	Super::Activate();

	UE_CVLOG(bUseContinuousTracking, GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("Continuous goal tracking requested, moving to: %s"),
		MoveRequest.IsMoveToActorRequest() ? TEXT("actor => looping successful moves!") : TEXT("location => will NOT loop"));

	MoveRequestID = FAIRequestID::InvalidRequest;
	ConditionalPerformMove();
}

void UAITask_TDPMoveTo::ConditionalPerformMove()
{
	if (MoveRequest.IsUsingPathfinding() && OwnerController && OwnerController->ShouldPostponePathUpdates())
	{
		UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("%s> can't path right now, waiting..."), *GetName());
		OwnerController->GetWorldTimerManager().SetTimer(MoveRetryTimerHandle, this, &UAITask_TDPMoveTo::ConditionalPerformMove, 0.2f, false);
	}
	else
	{
		MoveRetryTimerHandle.Invalidate();
		PerformMove();
	}
}

void UAITask_TDPMoveTo::PerformMove()
{
	// Prepare the move first (check for early out)
	CheckPathPreConditions();

	if (!mUseAsyncPathfinding || mNavigationComponent->CanFindPathAsync(MoveRequest.IsMoveToActorRequest() ? MoveRequest.GetGoalActor()->GetActorLocation() : MoveRequest.GetGoalLocation()))
	{
		ResetObservers();
		ResetTimers();
		ResetPaths();
	}

	if (mResult.Code == ETDPPathfindingRequestResult::AlreadyAtGoal)
	{
		MoveRequestID = mResult.MoveId;
		OnRequestFinished(mResult.MoveId, FPathFollowingResult(EPathFollowingResult::Success, FPathFollowingResultFlags::AlreadyAtGoal));
		return;
	}

	// If we're ready to path, then request the path
	if (mResult.Code == ETDPPathfindingRequestResult::ReadyToPath)
	{
		mUseAsyncPathfinding ? RequestPathAsync() : RequestPathSynchronous();

		switch (mResult.Code)
		{
		case ETDPPathfindingRequestResult::Failed:
		{
			FinishMoveTask(EPathFollowingResult::Invalid);
		} break;
		case ETDPPathfindingRequestResult::Success: // Synchronous pathfinding
		{
			MoveRequestID = mResult.MoveId;
			if (IsFinished())
			{
				UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Error, TEXT("%s> re-Activating Finished task!"), *GetName());
			}
			RequestMove(); // Start the move
		} break;
		case ETDPPathfindingRequestResult::Deferred: // Async...we're waiting on the task to return
		{
			MoveRequestID = mResult.MoveId;
			//mAsyncTaskComplete = false;
		} break;
		default:
			checkNoEntry();
			break;
		}
	}
}

void UAITask_TDPMoveTo::Pause()
{
	if (OwnerController && MoveRequestID.IsValid())
	{
		OwnerController->PauseMove(MoveRequestID);
	}

	ResetTimers();
	Super::Pause();
}

void UAITask_TDPMoveTo::Resume()
{
	Super::Resume();

	if (!MoveRequestID.IsValid() || (OwnerController && !OwnerController->ResumeMove(MoveRequestID)))
	{
		UE_CVLOG(MoveRequestID.IsValid(), GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("%s> Resume move failed, starting new one."), *GetName());
		ConditionalPerformMove();
	}
}

void UAITask_TDPMoveTo::SetObservedPath(FNavPathSharedPtr InPath)
{
	if (PathUpdateDelegateHandle.IsValid() && Path.IsValid())
	{
		Path->RemoveObserver(PathUpdateDelegateHandle);
	}

	PathUpdateDelegateHandle.Reset();

	Path = InPath;
	if (Path.IsValid())
	{
		// disable auto repaths, it will be handled by move task to include ShouldPostponePathUpdates condition
		Path->EnableRecalculationOnInvalidation(false);
		PathUpdateDelegateHandle = Path->AddObserver(FNavigationPath::FPathObserverDelegate::FDelegate::CreateUObject(this, &UAITask_TDPMoveTo::OnPathEvent));
	}
}

void UAITask_TDPMoveTo::CheckPathPreConditions()
{
#if WITH_EDITOR
	UE_VLOG(this, VisualCinnamonLog, Log, TEXT("TDPMoveTo: %s"), *MoveRequest.ToString());
#endif

	mResult.Code = ETDPPathfindingRequestResult::Failed;

	// Fail if no nav component
	mNavigationComponent = Cast<UTDPNavigationComponent>(GetOwnerActor()->GetComponentByClass(UTDPNavigationComponent::StaticClass()));
	if (!mNavigationComponent)
	{
#if WITH_EDITOR
		UE_VLOG(this, VisualCinnamonLog, Error, TEXT("TDPMoveTo request failed due missing TDPNavigationComponent on the pawn"), *MoveRequest.ToString());
		UE_LOG(CinnamonLog, Error, TEXT("TDPMoveTo request failed due missing TDPNavigationComponent on the pawn"));
		return;
#endif
	}

	if (MoveRequest.IsValid() == false)
	{
#if WITH_EDITOR
		UE_VLOG(this, VisualCinnamonLog, Error, TEXT("TDPMoveTo request failed due MoveRequest not being valid. Most probably desired Goal Actor not longer exists"), *MoveRequest.ToString());
		UE_LOG(CinnamonLog, Error, TEXT("TDPMoveTo request failed due MoveRequest not being valid. Most probably desired Goal Actor not longer exists"));
#endif
		return;
	}

	if (OwnerController->GetPathFollowingComponent() == nullptr)
	{
#if WITH_EDITOR
		UE_VLOG(this, VisualCinnamonLog, Error, TEXT("TDPMoveTo request failed due missing PathFollowingComponent"));
		UE_LOG(CinnamonLog, Error, TEXT("TDPMoveTo request failed due missing PathFollowingComponent"));
#endif
		return;
	}

	bool bCanRequestMove = true;
	bool bAlreadyAtGoal = false;

	if (!MoveRequest.IsMoveToActorRequest())
	{
		if (MoveRequest.GetGoalLocation().ContainsNaN() || FAISystem::IsValidLocation(MoveRequest.GetGoalLocation()) == false)
		{
#if WITH_EDITOR
			UE_VLOG(this, VisualCinnamonLog, Error, TEXT("TDPMoveTo: Destination is not valid! Goal(%s)"), TEXT_AI_LOCATION(MoveRequest.GetGoalLocation()));
			UE_LOG(CinnamonLog, Error, TEXT("TDPMoveTo: Destination is not valid! Goal(%s)"));
#endif
			bCanRequestMove = false;
		}

		bAlreadyAtGoal = bCanRequestMove && OwnerController->GetPathFollowingComponent()->HasReached(MoveRequest);
	}
	else
	{
		bAlreadyAtGoal = bCanRequestMove && OwnerController->GetPathFollowingComponent()->HasReached(MoveRequest);
	}

	if (bAlreadyAtGoal)
	{
#if WITH_EDITOR
		UE_VLOG(this, VisualCinnamonLog, Log, TEXT("TDPMoveTo: already at goal!"));
		UE_LOG(CinnamonLog, Log, TEXT("TDPMoveTo: already at goal!"));
#endif
		mResult.MoveId = OwnerController->GetPathFollowingComponent()->RequestMoveWithImmediateFinish(EPathFollowingResult::Success);
		mResult.Code = ETDPPathfindingRequestResult::AlreadyAtGoal;
	}

	if (bCanRequestMove)
	{
		mResult.Code = ETDPPathfindingRequestResult::ReadyToPath;
	}

	return;
}

void UAITask_TDPMoveTo::RequestPathSynchronous()
{
	mResult.Code = ETDPPathfindingRequestResult::Failed;

#if WITH_EDITOR
	UE_VLOG(this, VisualCinnamonLog, Log, TEXT("TDPMoveTo: Requesting Synchronous pathfinding!"));
	UE_LOG(CinnamonLog, Log, TEXT("TDPMoveTo: Requesting Synchronous pathfinding!"));
#endif

	if (mNavigationComponent->FindPath(MoveRequest.IsMoveToActorRequest() ? MoveRequest.GetGoalActor()->GetActorLocation() : MoveRequest.GetGoalLocation()))
	{
		mPath = mNavigationComponent->GetPath();
		mResult.Code = ETDPPathfindingRequestResult::Success;
	}

	return;
}

void UAITask_TDPMoveTo::RequestPathAsync()
{
	mResult.Code = ETDPPathfindingRequestResult::Failed;

	// Fail if no nav component
	UTDPNavigationComponent* TDPNavigationComponent = Cast<UTDPNavigationComponent>(GetOwnerActor()->GetComponentByClass(UTDPNavigationComponent::StaticClass()));
	if (!TDPNavigationComponent)
		return;

	//mAsyncTaskComplete = false;

	// Request the async path
	FVector targetPosition = MoveRequest.IsMoveToActorRequest() ? MoveRequest.GetGoalActor()->GetActorLocation() : MoveRequest.GetGoalLocation();
	TDPNavigationComponent->FindPathAsync(targetPosition, mAsyncTaskComplete);
	mPath = mNavigationComponent->GetPath();
	mAsyncTaskComplete = TDPNavigationComponent->GetPath()->IsReady();

	mResult.Code = ETDPPathfindingRequestResult::Deferred;
}

/* Requests the move, based on the current path */
void UAITask_TDPMoveTo::RequestMove()
{
	mResult.Code = ETDPPathfindingRequestResult::Failed;

	LogPathHelper();

	// Copy the SVO path into a regular path for now, until we implement our own path follower.
	mPath->CreateUENavigationPath(*Path);
	Path->MarkReady();

	UPathFollowingComponent* PFComp = OwnerController ? OwnerController->GetPathFollowingComponent() : nullptr;
	if (PFComp == nullptr)
	{
		FinishMoveTask(EPathFollowingResult::Invalid);
		return;
	}

	PathFinishDelegateHandle = PFComp->OnRequestFinished.AddUObject(this, &UAITask_TDPMoveTo::OnRequestFinished);
	SetObservedPath(Path);

	const FAIRequestID RequestID = Path->IsValid() ? OwnerController->RequestMove(MoveRequest, Path) : FAIRequestID::InvalidRequest;

	if (RequestID.IsValid())
	{
#if WITH_EDITOR
		UE_VLOG(this, VisualCinnamonLog, Log, TEXT("TDP Pathfinding successful, moving"));
		UE_LOG(CinnamonLog, Log, TEXT("TDP Pathfinding successful, moving"));
#endif
		mResult.MoveId = RequestID;
		mResult.Code = ETDPPathfindingRequestResult::Success;
	}

	if (mResult.Code == ETDPPathfindingRequestResult::Failed)
	{
		mResult.MoveId = OwnerController->GetPathFollowingComponent()->RequestMoveWithImmediateFinish(EPathFollowingResult::Invalid);
	}
}

void UAITask_TDPMoveTo::HandleAsyncPathTaskComplete()
{
	mResult.Code = ETDPPathfindingRequestResult::Success;
	// Request the move
	RequestMove();
	mNavigationComponent->SetMoveRequested(true);
	// Flag that we've processed the task
	mAsyncTaskComplete = false;

	if (mNavigationComponent->DrawPath)
	{
		FlushDebugStrings(mNavigationComponent->GetWorld());
		FlushPersistentDebugLines(mNavigationComponent->GetWorld());
		mPath->DrawDebugVisualization(mNavigationComponent->GetWorld(), *mNavigationComponent->GetVolume());
	}
}

void UAITask_TDPMoveTo::ResetPaths()
{
	if (Path.IsValid())
	{
		Path->ResetForRepath();
	}

	if (mPath.IsValid())
	{
		mPath->Reset();
	}
}

/** Renders the octree path as a 3d tunnel in the visual logger */
void UAITask_TDPMoveTo::LogPathHelper()
{
#if WITH_EDITOR
#if ENABLE_VISUAL_LOG

	UTDPNavigationComponent* TDPNavigationComponent = Cast<UTDPNavigationComponent>(GetOwnerActor()->GetComponentByClass(UTDPNavigationComponent::StaticClass()));
	if (!TDPNavigationComponent)
	{
		return;
	}

	FVisualLogger& Vlog = FVisualLogger::Get();
	if (Vlog.IsRecording() && mPath.IsValid() && mPath->GetPath().Num())
	{
		FVisualLogEntry* Entry = Vlog.GetEntryToWrite(OwnerController->GetPawn(), OwnerController->GetPawn()->GetWorld()->TimeSeconds);
		if (Entry)
		{
			const auto& points = mPath->GetPath();
			for (int i = 0; i < points.Num(); i++)
			{
				if (i == 0 || i == points.Num() - 1)
				{
					continue;
				}

				const auto& point = points[i];

				float size = 0.f;

				if (point.LayerIndex == 0)
				{
					size = TDPNavigationComponent->GetVolume()->GetVoxelSizeInLayer(0) * 0.25f;
				}
				else
				{
					size = TDPNavigationComponent->GetVolume()->GetVoxelSizeInLayer(point.LayerIndex - 1);
				}

				UE_VLOG_BOX(OwnerController->GetPawn(), VisualCinnamonLog, Verbose, FBox(point.Position + FVector(size * 0.5f), point.Position - FVector(size * 0.5f)), FColor::Black, TEXT_EMPTY);
			}
		}
	}
#endif // ENABLE_VISUAL_LOG
#endif // WITH_EDITOR
}

void UAITask_TDPMoveTo::ResetObservers()
{
	if (Path.IsValid())
	{
		Path->DisableGoalActorObservation();
	}

	if (PathFinishDelegateHandle.IsValid())
	{
		UPathFollowingComponent* PFComp = OwnerController ? OwnerController->GetPathFollowingComponent() : nullptr;
		if (PFComp)
		{
			PFComp->OnRequestFinished.Remove(PathFinishDelegateHandle);
		}

		PathFinishDelegateHandle.Reset();
	}

	if (PathUpdateDelegateHandle.IsValid())
	{
		if (Path.IsValid())
		{
			Path->RemoveObserver(PathUpdateDelegateHandle);
		}

		PathUpdateDelegateHandle.Reset();
	}
}

void UAITask_TDPMoveTo::ResetTimers()
{
	if (MoveRetryTimerHandle.IsValid())
	{
		if (OwnerController)
		{
			OwnerController->GetWorldTimerManager().ClearTimer(MoveRetryTimerHandle);
		}

		MoveRetryTimerHandle.Invalidate();
	}

	if (PathRetryTimerHandle.IsValid())
	{
		if (OwnerController)
		{
			OwnerController->GetWorldTimerManager().ClearTimer(PathRetryTimerHandle);
		}

		PathRetryTimerHandle.Invalidate();
	}
}

void UAITask_TDPMoveTo::OnDestroy(bool bInOwnerFinished)
{
	Super::OnDestroy(bInOwnerFinished);

	ResetObservers();
	ResetTimers();

	if (MoveRequestID.IsValid())
	{
		UPathFollowingComponent* PFComp = OwnerController ? OwnerController->GetPathFollowingComponent() : nullptr;
		if (PFComp && PFComp->GetStatus() != EPathFollowingStatus::Idle)
		{
			PFComp->AbortMove(*this, FPathFollowingResultFlags::OwnerFinished, MoveRequestID);
		}
	}

	// clear the shared pointer now to make sure other systems
	// don't think this path is still being used
	Path = nullptr;
	mPath = nullptr;
}

void UAITask_TDPMoveTo::OnRequestFinished(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (RequestID == mResult.MoveId)
	{
		if (Result.HasFlag(FPathFollowingResultFlags::UserAbort) && Result.HasFlag(FPathFollowingResultFlags::NewRequest) && !Result.HasFlag(FPathFollowingResultFlags::ForcedScript))
		{
			UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("%s> ignoring OnRequestFinished, move was aborted by new request"), *GetName());
		}
		else
		{
			// reset request Id, FinishMoveTask doesn't need to update path following's state
			mResult.MoveId = FAIRequestID::InvalidRequest;

			if (bUseContinuousTracking && MoveRequest.IsMoveToActorRequest() && Result.IsSuccess())
			{
				UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("%s> received OnRequestFinished and goal tracking is active! Moving again in next tick"), *GetName());
				GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UAITask_TDPMoveTo::PerformMove);
			}
			else
			{
				FinishMoveTask(Result.Code);
			}
		}
	}
	else if (IsActive())
	{
		UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Warning, TEXT("%s> received OnRequestFinished with not matching RequestID!"), *GetName());
	}
}

void UAITask_TDPMoveTo::OnPathEvent(FNavigationPath* InPath, ENavPathEvent::Type Event)
{
	const static UEnum* NavPathEventEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ENavPathEvent"));
	UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("%s> Path event: %s"), *GetName(), *NavPathEventEnum->GetNameStringByValue(Event));

	switch (Event)
	{
	case ENavPathEvent::NewPath:
	case ENavPathEvent::UpdatedDueToGoalMoved:
	case ENavPathEvent::UpdatedDueToNavigationChanged:
		if (InPath && InPath->IsPartial() && !MoveRequest.IsUsingPartialPaths())
		{
			UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT(">> partial path is not allowed, aborting"));
			UPathFollowingComponent::LogPathHelper(OwnerController, InPath, MoveRequest.GetGoalActor());
			FinishMoveTask(EPathFollowingResult::Aborted);
		}
#if ENABLE_VISUAL_LOG
		else if (!IsActive())
		{
			UPathFollowingComponent::LogPathHelper(OwnerController, InPath, MoveRequest.GetGoalActor());
		}
#endif // ENABLE_VISUAL_LOG
		break;

	case ENavPathEvent::Invalidated:
		ConditionalUpdatePath();
		break;

	case ENavPathEvent::Cleared:
	case ENavPathEvent::RePathFailed:
		UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT(">> no path, aborting!"));
		FinishMoveTask(EPathFollowingResult::Aborted);
		break;

	case ENavPathEvent::MetaPathUpdate:
	default:
		break;
	}
}

void UAITask_TDPMoveTo::ConditionalUpdatePath()
{
	// mark this path as waiting for repath so that PathFollowingComponent doesn't abort the move while we 
	// micro manage repathing moment
	// note that this flag fill get cleared upon repathing end
	if (Path.IsValid())
	{
		Path->SetManualRepathWaiting(true);
	}

	if (MoveRequest.IsUsingPathfinding() && OwnerController && OwnerController->ShouldPostponePathUpdates())
	{
		UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("%s> can't path right now, waiting..."), *GetName());
		OwnerController->GetWorldTimerManager().SetTimer(PathRetryTimerHandle, this, &UAITask_TDPMoveTo::ConditionalUpdatePath, 0.2f, false);
	}
	else
	{
		PathRetryTimerHandle.Invalidate();

		ANavigationData* NavData = Path.IsValid() ? Path->GetNavigationDataUsed() : nullptr;
		if (NavData)
		{
			NavData->RequestRePath(Path, ENavPathUpdateType::NavigationChanged);
		}
		else
		{
			UE_VLOG(GetGameplayTasksComponent(), LogGameplayTasks, Log, TEXT("%s> unable to repath, aborting!"), *GetName());
			FinishMoveTask(EPathFollowingResult::Aborted);
		}
	}
}
