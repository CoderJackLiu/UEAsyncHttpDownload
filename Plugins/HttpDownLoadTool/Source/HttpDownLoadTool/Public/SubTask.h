#pragma once
#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"

class UAsyncHttpFile;
class  FSubHttpTask :public TSharedFromThis<FSubHttpTask>
{
public:
	FSubHttpTask();
	~FSubHttpTask();
	int32					 TaskID;//任务编号
	FString					 URL;
	FString					 MD5Str;
	FString					 CurFilePath;
	FString					 Range;//范围
	bool					 bFinished;//任务是否完成
	bool					 bWaitResponse;//是否需要等待
	int32					 StarPos;//文件下载的字节位置
	int32					 Size;//该子任务的字节大小
	TArray<uint8>			 RawData;//下载下来后的数据
	double					 RequestTime;//请求时间
	TSharedPtr<IHttpRequest> RequestPtr;//请求体
	/**
	 * 创建请求体
	 */
	TSharedPtr<IHttpRequest> CreateRequest();
	/**
	 * 取消该子任务
	 */
	void Stop();

	void SaveData();
	void LoadData();

};