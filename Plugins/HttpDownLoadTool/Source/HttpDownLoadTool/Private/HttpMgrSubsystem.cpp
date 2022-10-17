#include "HttpMgrSubsystem.h"
#include "Misc/Paths.h"
#include <AsyncDownloadFile.h>
#include "Misc/App.h"
#include "HttpModule.h"

UHttpManagerSetting::UHttpManagerSetting(const FObjectInitializer& ObjectInitializer)
	: MaxParallel(5)
	, MaxTryCount(10)
	, RequestKBSize(1024)
	, ResponseTimeout(5.0f)
{
	CurFilePath = FPaths::ProjectSavedDir() / TEXT("HttpCacheFiles");
}

void UHttpMgrSubsystem::DownLoadHttpFileWithInterfaceUrl(UObject* WorldContextObject,
	const FString& InUrl, const FString& InDirectory, bool bClearCache)
{
	RequestFileDataInfo(InUrl,WorldContextObject,InDirectory,bClearCache);
}

void UHttpMgrSubsystem::RequestFileDataInfo(const FString& InUrl, UObject *WorldContextObject,
		FString InDirectory,
		bool bClearCache)
{
	TSharedPtr<IHttpRequest> FileDataRequest = FHttpModule::Get().CreateRequest();
	if (FileDataRequest.IsValid())
	{
		FileDataRequest->SetURL(InUrl);
		FileDataRequest->SetVerb(TEXT("GET"));
		FileDataRequest->OnProcessRequestComplete().BindUObject(this, &UHttpMgrSubsystem::HandleResponseFileDataInfo,WorldContextObject,InDirectory,bClearCache);
		FileDataRequest->ProcessRequest();
	}
}

void UHttpMgrSubsystem::HandleResponseFileDataInfo(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse,
	bool bSucceeded,UObject *WorldContextObject,
	FString InDirectory,
	bool bClearCache)
{
	if (bSucceeded&&HttpResponse.IsValid())
	{
		int32 Code = HttpResponse->GetResponseCode();
		if (EHttpResponseCodes::IsOk(Code))
		{
			FString ContentString = HttpResponse->GetContentAsString();
			TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ContentString);
			FJsonSerializer::Deserialize(Reader,JsonObject);
			const TArray< TSharedPtr<FJsonValue> >& DataArray =JsonObject->GetArrayField("data");
			for(const auto&ItFileData:DataArray)
			{
				const TSharedPtr<FJsonObject>& FileDataObj =  ItFileData->AsObject();
				FString src = FileDataObj->GetStringField("src");
				FString fileName = FileDataObj->GetStringField("name");
				FString md5 = FileDataObj->GetStringField("md5");
				UAsyncDownloadFile* DownloadFile = UAsyncDownloadFile::DownLoadHttpFileWithFileName(WorldContextObject,src,fileName,InDirectory,md5, bClearCache);
				OnCreateDownloadFile.Broadcast(DownloadFile);
			}
			
		}
	}
	else
	{
		
	}
}

void UHttpMgrSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Init();
}

void UHttpMgrSubsystem::Deinitialize()
{
	Super::Deinitialize();
	ClearMgr();
}

void UHttpMgrSubsystem::Tick(float DeltaTime)
{
	if (IsTickable())
	{	
		for (int32 i = 0; i < DownloadHttpFiles.Num(); i++)
		{
			UAsyncDownloadFile *HttpFile = DownloadHttpFiles[i];
			if (HttpFile)
			{
				if (HttpFile->State == ETaskState::_Downloading)
				{
					//检测是否有超时任务
					double CurTime = FApp::GetCurrentTime();
					HttpFile->CheckDeadTask(CurTime);
				}
			}
			else
			{
				DownloadHttpFiles.RemoveAt(i);
			}

		}
	}
}

bool UHttpMgrSubsystem::IsTickable() const
{
	return DownloadHttpFiles.Num() > 0;
}

UAsyncDownloadFile* UHttpMgrSubsystem::CreateDownTask(
	UObject *WorldContextObject, 
	const FString &Url,
	const FString &FileName,
	const FString &Directory,
	const FString& InFileMd5,
	bool bClearCache)
{
	UAsyncDownloadFile* HttpFile = NewObject<UAsyncDownloadFile>(WorldContextObject, *(Directory / FileName));
	if (HttpFile)
	{
		HttpFile->Url = Url;
		HttpFile->FileName = FileName;
		HttpFile->Directory = Directory;
		HttpFile->State = ETaskState::_Ready;
		HttpFile->Mgr = this;
		HttpFile->FileDataMD5Str = InFileMd5;
		HttpFile->bClearCache = bClearCache;
		HttpFile->MD5Str = (FMD5::HashAnsiString(*Url)).Append(InFileMd5);
		DownloadHttpFiles.AddUnique(HttpFile);
	}
	return HttpFile;

}
void UHttpMgrSubsystem::Init()
{
	const UHttpManagerSetting* Config = GetDefault<UHttpManagerSetting>();
	this->MaxParallel = Config->MaxParallel;
	this->MaxTryCount = Config->MaxTryCount;
	this->RequestKBSize = Config->RequestKBSize;
	this->ResponseTimeout = Config->ResponseTimeout;
	this->CurFilePath = Config->CurFilePath;
}
void UHttpMgrSubsystem::ExecDownloadTasks()
{
	int32 NumActiveTask = 0;
	TArray<UAsyncDownloadFile *> ProbTasks;
	for (int32 i = 0; i < DownloadHttpFiles.Num(); i++)
	{
		UAsyncDownloadFile* HttpFile = DownloadHttpFiles[i];
		if (HttpFile)
		{
			ETaskState State = HttpFile->State;
			switch (State)
			{
			case ETaskState::_Ready://准备状态
				ProbTasks.Add(HttpFile);
				break;
			case ETaskState::_Downloading:
				NumActiveTask++;
				break;
			case ETaskState::_Succees:
				DownloadHttpFiles.Remove(HttpFile);
				--i;
				break;
			case ETaskState::_Retry:
				if (HttpFile->TryCount <= MaxTryCount)
				{
					ProbTasks.Add(HttpFile);
				}
				break;
			case ETaskState::_Failed:

				break;
			}
		}
	}
	for (int i = 0; i < ProbTasks.Num(); ++i)
	{
		UAsyncDownloadFile* HttpFile = ProbTasks[i];
		if (HttpFile&&NumActiveTask < MaxParallel)
		{
			HttpFile->StartDownloadTake();
		}
	}
}

void UHttpMgrSubsystem::NotifyFailed(UAsyncDownloadFile *HttpFile, bool bForceFailed)
{
	if (!bForceFailed)
	{
		if (HttpFile)
		{
			if (HttpFile->TryCount < MaxTryCount)
			{
				HttpFile->NotifyWait();
			}
			else
			{
				HttpFile->NotifyFailed();
			}
		}
	}
	ExecDownloadTasks();
}

void UHttpMgrSubsystem::NotifyFinised(UAsyncDownloadFile *HttpFile)
{
	ExecDownloadTasks();
	if(DownloadHttpFiles.Contains(HttpFile))
	{
		DownloadHttpFiles.Remove(HttpFile);
	}
	//ClearMgr();
}

void UHttpMgrSubsystem::ClearMgr()
{
	for (int32 i = DownloadHttpFiles.Num() - 1; i >= 0; --i)
	{
		DownloadHttpFiles[i]->RemoveFromRoot();
		DownloadHttpFiles.RemoveAt(i);
		continue;
	}
}

