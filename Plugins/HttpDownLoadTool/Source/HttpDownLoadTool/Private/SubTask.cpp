#include "SubTask.h"
#include "HttpModule.h"
#include "AsyncDownloadFile.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"

FSubHttpTask::FSubHttpTask()
	:TaskID(INDEX_NONE)
	, bFinished(false)
	, StarPos(0)
	, Size(0)
	, bWaitResponse(false)
{

}

FSubHttpTask::~FSubHttpTask()
{
	Stop();
}

TSharedPtr<IHttpRequest> FSubHttpTask::CreateRequest()
{
	RequestPtr = FHttpModule::Get().CreateRequest();
	RequestPtr->SetURL(URL);
	RequestPtr->SetVerb(TEXT("GET"));
	RequestPtr->SetHeader(TEXT("Range"), Range);//断点续传下载该文件的一个片段 这是一个Range（范围）
	bWaitResponse = true;
	RequestPtr->SetTimeout(10);
	RequestTime = FApp::GetCurrentTime();//当前运行时间
	return RequestPtr;
}

void FSubHttpTask::Stop()
{
	if (RequestPtr.IsValid())
	{
		RequestPtr->CancelRequest();
		RequestPtr = nullptr;
	}
}

void FSubHttpTask::SaveData()
{
	//hcf  http Cache file
	FString SubTaskFileName = MD5Str + TEXT("_") + FString::FromInt(TaskID) + TEXT(".hcf");

	FFileHelper::SaveArrayToFile(RawData, *(CurFilePath / SubTaskFileName));
}

void FSubHttpTask::LoadData()
{
	//hcf  http Cache file
	FString SubTaskFileName = MD5Str + TEXT("_") + FString::FromInt(TaskID) + TEXT(".hcf");
	FFileHelper::LoadFileToArray(RawData, *(CurFilePath / SubTaskFileName));
}


