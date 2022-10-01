#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"
#include "DeveloperSettings/Public/Engine/DeveloperSettings.h"
#include "Interfaces/IHttpRequest.h"
#include "HttpMgrSubsystem.generated.h"

class UAsyncDownloadFile;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreateDownloadFileDelegate, UAsyncDownloadFile*,DownloadFile);

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "HttpMgrSetting"))
class HTTPDOWNLOADTOOL_API UHttpManagerSetting :public UDeveloperSettings
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
	int32						MaxParallel;//最大的下载并行数
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
	int32						MaxTryCount;//最大的重连次数
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
	int32						RequestKBSize;//每个子任务的大小
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
	float						ResponseTimeout;//请求超时的时间
	UPROPERTY(config, EditAnywhere, Category = "Config | HttpManager")
	FString						CurFilePath; //子文件的缓存位置
};
class UAsyncDownloadFile;
UCLASS()
class UHttpMgrSubsystem :public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, Category = "HttpDownloader", meta = (WorldContext = "WorldContextObject"))
	void DownLoadHttpFileWithInterfaceUrl(
	UObject *WorldContextObject, 
	const FString& InUrl, 
	const FString& InDirectory, 
	bool bClearCache = false);
	
	void RequestFileDataInfo(const FString& InUrl, UObject *WorldContextObject,
		FString InDirectory,
		bool bClearCache = false);
	void HandleResponseFileDataInfo(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded,
		UObject *WorldContextObject,
		FString InDirectory,
		bool bClearCache = false);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const { return true; }

	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection);

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize();

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const { return TStatId();/* RETURN_QUICK_DECLARE_CYCLE_STAT(UHttpDownloadSubsystem, STATGROUP_Tickables);*/ }
public:
	UAsyncDownloadFile* CreateDownTask(
		UObject *WorldContextObject, 
		const FString &Url, 
		const FString &FileName, 
		const FString &Directory,
		const FString& InFileMd5,
		bool bClearCache);

	void ExecDownloadTasks();

	void NotifyFailed(UAsyncDownloadFile *HttpFile, bool bForceFailed);

	void NotifyFinised(UAsyncDownloadFile *HttpFile);
	void ClearMgr();
private:
	void Init();
public:

	int32						MaxParallel;
	int32						MaxTryCount;
	int32						RequestKBSize;
	float						ResponseTimeout;
	FString						CurFilePath;
	UPROPERTY()
	TArray<UAsyncDownloadFile *>	DownloadHttpFiles;

	UPROPERTY(BlueprintAssignable)
	FOnCreateDownloadFileDelegate OnCreateDownloadFile;
};