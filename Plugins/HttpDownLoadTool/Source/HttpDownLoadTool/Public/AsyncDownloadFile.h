#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "UObject/Object.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "SubTask.h"
#include "AsyncDownloadFile.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHttpDownloadDelegate, const FString &, DestPathOrMsg, float, Progress);

UENUM(BlueprintType)
enum class ETaskState : uint8
{
	_Ready,
	_Downloading,
	_Succees,
	_Retry,
	_Failed,
};
class UHttpMgrSubsystem;
UCLASS()
//class UAsyncDownloadFile :public UBlueprintAsyncActionBase
class UAsyncDownloadFile :public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()
public:
	friend UHttpMgrSubsystem;


	/**
	*@param InUrl;http file url
	*@param InFileName: file name without path
	*@param InDirectory: directory name
	*@param bClearCache:whether to clean up cached files after file download is complete
	*/
	UFUNCTION(BlueprintCallable, Category = "HttpDownloader", meta = (WorldContext = "WorldContextObject"))
		static UAsyncDownloadFile *DownLoadHttpFileWithFileName(
			UObject *WorldContextObject, 
			const FString& InUrl, 
			const FString& InFileName, 
			const FString &InDirectory,
			const FString& InFileMd5,
			bool bClearCache = false);

	UFUNCTION(BlueprintCallable, Category = "HttpDownloader", meta = (WorldContext = "WorldContextObject"))
	static UAsyncDownloadFile *DownLoadHttpFile(
		UObject *WorldContextObject, 
		const FString& InUrl, 
		const FString &InDirectory,
		const FString& InFileMd5,
		bool bClearCache = false);
protected:
	void StartDownloadTake();
	void RequestFileSize();
	void HandleResponseFileSize(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	
	
	void StartSubTaskDownload(TSharedRef<FSubHttpTask> SubHttpTask);
	//void StartSubTaskDownload(FSubHttpTask*  SubHttpTask);
	void HandleDownload(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded,int32 TaskID);
	void UpdateProgress();

private:
	/**
	 * 广播任务失败
	 */
	void  NotifyFailed();
	/**
	 * 广播任务进入等待阶段
	 */
	void  NotifyWait();
	/**
	* 开始分配子任务
	*/
	void BeginDownload();
	/**
	* 创建子任务
	*/
	void CreateSubTask();
	void UpdateTask();
	/**
	* 这个文件下载完成了
	*/
	void OnFinished();
	/**
	 * 处理请求超时的子任务
	 */
	void CheckDeadTask(double CurTime);
	/**
	 * 保持文件
	 */
	bool SaveToFile();
	
	/**
	* 发生错误
	*/
	void FatalErr(const FString &ErrString);
public:
	UPROPERTY(BlueprintAssignable)
	FHttpDownloadDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FHttpDownloadDelegate OnWait;

	UPROPERTY(BlueprintAssignable)
	FHttpDownloadDelegate OnFail;

	UPROPERTY(BlueprintAssignable)
	FHttpDownloadDelegate OnProgress;

	UPROPERTY(BlueprintAssignable)
	FHttpDownloadDelegate OnCancel;
	
	void OnGetFileMd5(const FString& Md5);
protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	ETaskState				State;//任务状态
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	int32					TryCount;//最大尝试次数
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	FString					Url;//下载地址
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	FString					FileName;//文件名
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	FString					Directory;//文件路径
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	float					Progress;//下载进度
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	int32					CurFileSize;//当前下载了多少个字节了
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	int64					TotalFileSize;//http文件的总大小

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	int32					MaxTask;//最大任务数
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	FString					MD5Str;// MD5 与URL是对应关系
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	FString					FileDataMD5Str;// 源文件的md5
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	bool					bClearCache;//下载完成后是否清理缓存

	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	int32 WaitResponse;
	UPROPERTY()
	UHttpMgrSubsystem*	Mgr;

	TSharedPtr<IHttpRequest> FileSizeRequest;
	TSharedPtr<IHttpRequest> FileDataRequest;
	TArray<TSharedPtr<FSubHttpTask>>	SubTasks;
	//TArray<FSubHttpTask*>	SubTasks;
};