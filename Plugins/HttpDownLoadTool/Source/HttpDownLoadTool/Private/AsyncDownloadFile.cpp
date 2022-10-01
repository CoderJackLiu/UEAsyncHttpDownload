#include "AsyncDownloadFile.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "HttpMgrSubsystem.h"
#include "HttpModule.h"
#include "Kismet/BlueprintPathsLibrary.h"

UAsyncDownloadFile::UAsyncDownloadFile(class FObjectInitializer const & Obj)
	:Super(Obj)
	, TryCount(0)
	, Progress(0)
	, CurFileSize(0)
	, TotalFileSize(0)
	, MaxTask(2)
	, WaitResponse(0)
{

}

UAsyncDownloadFile* UAsyncDownloadFile::DownLoadHttpFileWithFileName(
	UObject *WorldContextObject, 
	const FString& InUrl, 
	const FString& InFileName, 
	const FString &InDirectory,
	const FString& InFileMd5,
	bool bClearCache /*= false*/)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	UHttpMgrSubsystem* MySubsystem = GameInstance->GetSubsystem<UHttpMgrSubsystem>();
	UAsyncDownloadFile* HttpDownTask = nullptr;
	if (MySubsystem)
	{
		HttpDownTask = MySubsystem->CreateDownTask(WorldContextObject, InUrl, InFileName, InDirectory,InFileMd5, bClearCache);
		//开始执行任务
		//...
		MySubsystem->ExecDownloadTasks();

	}
	return HttpDownTask;
}

UAsyncDownloadFile* UAsyncDownloadFile::DownLoadHttpFile(UObject* WorldContextObject, const FString& InUrl,
	const FString& InDirectory,const FString& InFileMd5, bool bClearCache)
{
	FString FileName="";
	FString PathPart="",FileNamePart="",ExtensionPart="";
	FPaths::Split(InUrl,PathPart,FileNamePart,ExtensionPart);
	FileName = FileNamePart+"."+ExtensionPart;
	return DownLoadHttpFileWithFileName(WorldContextObject,InUrl,FileName,InDirectory,InFileMd5, bClearCache);
}

void UAsyncDownloadFile::StartDownloadTake()
{
	State = ETaskState::_Downloading;
	if (Url.Len() > 0 && FileName.Len() > 0)
	{
		RequestFileSize();
	}
	else
	{
		FatalErr(TEXT("URL and FileName must set!"));
	}
	TryCount++;
}

void UAsyncDownloadFile::RequestFileSize()
{
	FileSizeRequest = FHttpModule::Get().CreateRequest();
	if (FileSizeRequest)
	{
		FileSizeRequest->SetURL(Url);
		FileSizeRequest->SetVerb(TEXT("GET"));
		FileSizeRequest->SetHeader(TEXT("Range"), TEXT("bytes=0-1"));//表示的是去请求文件的头文件信息
		FileSizeRequest->OnProcessRequestComplete().BindUObject(this, &UAsyncDownloadFile::HandleResponseFileSize);
		FileSizeRequest->ProcessRequest();
	}
}

void UAsyncDownloadFile::HandleResponseFileSize(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	if (bSucceeded&&HttpResponse.IsValid())
	{
		int32 Code = HttpResponse->GetResponseCode();
		if (EHttpResponseCodes::IsOk(Code))
		{
			FString RangeStr = HttpResponse->GetHeader("Content-Range");
			FString RightStr, LeftStr;
			RangeStr.Split(TEXT("/"), &LeftStr, &RightStr);
			if (RightStr.Len() > 0)
			{
				TotalFileSize = FCString::Atoi64(*RightStr);
			}
		}
	}
	else
	{
		FatalErr(TEXT("Response is null"));
	}
	FileSizeRequest.Reset();
	if (TotalFileSize <= 0)
	{
		Mgr->NotifyFailed(this, false);
	}
	else
	{
		BeginDownload();
	}
}

void UAsyncDownloadFile::StartSubTaskDownload(TSharedRef<FSubHttpTask> SubHttpTask)
{
	TSharedPtr<IHttpRequest> pRequset = SubHttpTask->CreateRequest();
	pRequset->OnProcessRequestComplete().BindUObject(this, &UAsyncDownloadFile::HandleDownload, SubHttpTask->TaskID);
	pRequset->ProcessRequest();
}

void UAsyncDownloadFile::HandleDownload(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, int32 TaskID)
{
	TSharedRef<FSubHttpTask> SubTask = SubTasks[TaskID].ToSharedRef();;
	WaitResponse--;
	if (HttpResponse.IsValid() && bSucceeded)
	{
		int32 Code = HttpResponse->GetResponseCode();
		if (EHttpResponseCodes::IsOk(Code))
		{
			SubTask->RawData = HttpResponse->GetContent();
			if (SubTask->RawData.Num() == SubTask->Size)//校验
			{
				SubTask->bFinished = true;
				SubTask->SaveData();
				UpdateProgress();

			}
			else
			{
				FatalErr(TEXT("HandleDownload content size error."));
			}

			SubTask->bWaitResponse = false;
			SubTask->RequestPtr.Reset();
			SubTask->RequestPtr = nullptr;
			if (State != ETaskState::_Succees)
			{
				UpdateTask();
			}
		}
	}
	else
	{
		SubTask->bWaitResponse = false;
		SubTask->RequestPtr.Reset();
		FatalErr(TEXT("HandleDownload Response is null."));
	}
}

void UAsyncDownloadFile::UpdateProgress()
{
	int64 SizeNum = 0;
	bool bSuccess = true;
	for (auto& Item : SubTasks)
	{
		if (Item->bFinished)
		{
			SizeNum+= Item->Size;
		}
		else
		{
			bSuccess = false;
		}
	}
	Progress = float(SizeNum) /(float)TotalFileSize;
	if (OnProgress.IsBound())
	{
		OnProgress.Broadcast(FileName, Progress);
	}
	if (bSuccess)
	{
		OnFinished();
	}
}

void UAsyncDownloadFile::NotifyFailed()
{
	State = ETaskState::_Failed;
	if (OnFail.IsBound())
	{
		OnFail.Broadcast(Directory / FileName, Progress);
	}
}

void UAsyncDownloadFile::NotifyWait()
{
	State = ETaskState::_Retry;
	if (OnWait.IsBound())
	{
		OnWait.Broadcast(Directory / FileName, Progress);
	}

}

void UAsyncDownloadFile::BeginDownload()
{
	CreateSubTask();
	UpdateTask();
}

void UAsyncDownloadFile::CreateSubTask()
{
	const int32 TaskSize = Mgr->RequestKBSize * 1024;//1MB 大小
	int32 StartSize = 0;
	SubTasks.Empty();
	int32 TaskID = 0;

	IPlatformFile& PlatFileModule = FPlatformFileManager::Get().GetPlatformFile();


	FString CurFilePath = Mgr->CurFilePath / MD5Str;
	TMap<int32, FString> SuccessSubTasks;
	if (PlatFileModule.DirectoryExists(*CurFilePath))
	{
		TArray<FString> CacheFiles;
		SuccessSubTasks.Empty();
		PlatFileModule.FindFiles(CacheFiles, *CurFilePath, TEXT("hcf"));
		for (int32 i = 0; i < CacheFiles.Num(); ++i)
		{
			FString LStr, RStr;
			CacheFiles[i].Split(TEXT("_"), &LStr, &RStr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			FString LStr1, RStr1;
			RStr.Split(TEXT("."), &LStr1, &RStr1, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			SuccessSubTasks.Add(FCString::Atoi(*LStr1), LStr);
		}
	}
	while (StartSize < TotalFileSize)
	{
		TSharedPtr<FSubHttpTask> SubHttpTask;
		int32 TempSize = 0;
		FString TempRange;
		SubHttpTask = MakeShareable(new FSubHttpTask);//创建子任务
		SubHttpTask->TaskID = TaskID;
		SubHttpTask->URL = Url;
		SubHttpTask->MD5Str = MD5Str;
		SubHttpTask->StarPos = StartSize;
		SubHttpTask->CurFilePath = CurFilePath;//缓存文件的位置

		if (SuccessSubTasks.Contains(TaskID))
		{
			SubHttpTask->bFinished = true;
			SubHttpTask->bWaitResponse = false;
			SubHttpTask->LoadData();
		}
		if (TotalFileSize > (StartSize + TaskSize))
		{
			TempSize = TaskSize;
			TempRange = FString::Printf(TEXT("bytes=%d-%d"), StartSize, StartSize + TempSize - 1);//1024-2047 
		}
		else
		{
			TempSize = TotalFileSize - StartSize;
			TempRange = FString::Printf(TEXT("bytes=%d-"), StartSize);
		}

		ensureMsgf(TempSize > 0, TEXT("SubTask Size Is Null!"));
		if (SubHttpTask.IsValid())
		{
			SubHttpTask->Size = TempSize;
			SubHttpTask->Range = TempRange;
			SubTasks.Add(SubHttpTask);
		}
		StartSize += TempSize;
		++TaskID;
	}

	bool bSuccess = true;
	for (auto& Sub : SubTasks)
	{
		if (!Sub->bFinished)
		{
			bSuccess = false;
			break;
		}
	}
	if (bSuccess)
	{
		OnFinished();
	}
}

void UAsyncDownloadFile::UpdateTask()
{
	if (WaitResponse <= MaxTask)
	{
		for (int32 i = 0; i < SubTasks.Num(); ++i)
		{
			TSharedRef<FSubHttpTask> SubTask = SubTasks[i].ToSharedRef();
			if (!SubTask->bFinished && !SubTask->bWaitResponse)
			{
				WaitResponse++;
				StartSubTaskDownload(SubTask);
				break;
			}
		}
	}
}

void UAsyncDownloadFile::OnFinished()
{
	if( SaveToFile())
	{
		State = ETaskState::_Succees;
		if (OnSuccess.IsBound())
		{
			OnSuccess.Broadcast(FileName, Progress);
			Mgr->NotifyFinised(this);

		}
		if (bClearCache)
		{
			//删除缓存文件
			IPlatformFile& PlatFileModule = FPlatformFileManager::Get().GetPlatformFile();
			PlatFileModule.DeleteDirectoryRecursively(*(Mgr->CurFilePath / MD5Str));
		}
	}
	else
	{
		FatalErr(FString::Printf(TEXT("%s md5不匹配"),*FileName));
	}
	
}

void UAsyncDownloadFile::CheckDeadTask(double CurTime)
{
	if (WaitResponse <= MaxTask)
	{
		for (int32 i = 0; i < SubTasks.Num(); ++i)
		{
			TSharedRef<FSubHttpTask> Task = SubTasks[i].ToSharedRef();
			if (!Task->bFinished&&Task->bWaitResponse)
			{
				float TimeOut = CurTime - Task->RequestTime;
				if (TimeOut > Mgr->ResponseTimeout)
				{
					WaitResponse++;
					StartSubTaskDownload(Task);
				}
			}
		}
	}
}

bool UAsyncDownloadFile::SaveToFile()
{
	FString FilePath = Directory / FileName;
	FArchive* Wirter = IFileManager::Get().CreateFileWriter(*FilePath);
	TArray<uint8> TotalFileData;
	if (Wirter)
	{
		for (int32 i = 0; i < SubTasks.Num(); ++i)
		{
			TSharedRef<FSubHttpTask> Task = SubTasks[i].ToSharedRef();
			Wirter->Serialize(Task->RawData.GetData(), Task->RawData.Num());
			TotalFileData.Append(Task->RawData);
		}
		Wirter->Close();
	}

	FString Md5 = FMD5::HashBytes(TotalFileData.GetData(),TotalFileData.Num());
	// MD5不区分大小写，这里都转成大写比较
	bool bEqual = Md5.ToUpper().Equals(FileDataMD5Str.ToUpper());
	return bEqual;
}

void UAsyncDownloadFile::FatalErr(const FString &ErrString)
{
	State = ETaskState::_Failed;
	OnFail.Broadcast(ErrString, -1);
	Mgr->NotifyFailed(this, true);
}

