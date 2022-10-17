#pragma once
//#include "AsyncDownloadFile.h"
//#include "MergeDownloadFileTask.generated.h"

class FMergeDownloadFileTask : public FNonAbandonableTask
{
	FString FilePath;
public:

	FString FileMd5="";
	UAsyncDownloadFile* DownloadFile;
	FMergeDownloadFileTask(const FString& InFilePath, UAsyncDownloadFile* InDownloadFile)
		:FilePath(InFilePath), DownloadFile(InDownloadFile)
	{
		FileMd5 = "";
	}
	
	void DoWork()
	{
		TArray<uint8> TotalFileData;

		FFileHelper::LoadFileToArray(TotalFileData,*FilePath);
		FileMd5 = FMD5::HashBytes(TotalFileData.GetData(),TotalFileData.Num());
		if(DownloadFile)
			DownloadFile->OnGetFileMd5(FileMd5);
	}

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FMergeDownloadFileTask, STATGROUP_ThreadPoolAsyncTasks);
	}
	
};
