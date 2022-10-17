// Fill out your copyright notice in the Description page of Project Settings.


#include "fileMgr.h"

#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Engine/Engine.h"
#include "IPlatformFilePak.h"
#include "Engine/StreamableManager.h"
#include "Engine/ObjectLibrary.h"


//创建一个目录，路径参数位绝对路径
void UfileMgr::CreateFolder(FString Path)
{
	FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*Path);
}

bool UfileMgr::CopyFile(FString Path, FString From)
{
	return FPlatformFileManager::Get().Get().GetPlatformFile().CopyFile(*Path, *From);
}

bool UfileMgr::MoveFile(FString to, FString From)
{
	return FPlatformFileManager::Get().Get().GetPlatformFile().MoveFile(*to, *From);
}
//相应的，删除绝对路径的目录
void UfileMgr::DeleteFolder(FString Path)
{
	FPlatformFileManager::Get().Get().GetPlatformFile().DeleteDirectoryRecursively(*Path);
}

bool UfileMgr::DeleteFile(FString Path)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFile.DeleteFile(*Path);//这里不一样！
}

//获取某一目录下的所有文件
void UfileMgr::GetFolderFiles(TArray<FString>& Files, FString Path)
{
	FPaths::NormalizeDirectoryName(Path);
	IFileManager& FileManager = IFileManager::Get();
	FString FinalPath = Path / TEXT("*");
	FileManager.FindFiles(Files, *FinalPath, true, true);
}



bool UfileMgr::delPak(const FString& path)
{
	FString SaveContentDir = path;

	//2.加载刚才保存的文件

	//获取当前使用的平台,这里使用的是WIN64平台

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	//初始化PakPlatformFile

	FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();

	PakPlatformFile->Initialize(&PlatformFile, TEXT(""));

	FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);
	FPakFile* PakFile = new FPakFile(&PlatformFile, *SaveContentDir, false);
	return PakPlatformFile->Unmount(*SaveContentDir);
}

bool UfileMgr::getPak(const FString& PakPath, const FString& PakPath2)
{
	//1.把下载好的文件保存起来

	FString SaveContentDir = PakPath;

	//2.加载刚才保存的文件

	//获取当前使用的平台,这里使用的是WIN64平台

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	//初始化PakPlatformFile

	FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();

	PakPlatformFile->Initialize(&PlatformFile, TEXT(""));

	FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);

	//获取Pak文件

	FPakFile* PakFile = new FPakFile(&PlatformFile, *SaveContentDir, false);

	UE_LOG(LogClass, Log, TEXT("get PakFile..."))

		//设置pak文件的Mount点.

		FString MountPoint(FPaths::EngineContentDir()); //"/../../../Engine/Content/"对应路径

	//PakFile->SetMountPoint(*PakPath2);

	//对pak文件mount到前面设定的MountPoint

	if (PakPlatformFile->Mount(*SaveContentDir, 0, *PakPath2))

	{
		UE_LOG(LogClass, Log, TEXT("Mount Success"));

		TArray<FString> FileList;

		//得到Pak文件中MountPoint路径下的所有文件

		PakFile->FindFilesAtPath(FileList, *PakFile->GetMountPoint(), true, false, true);

		FStreamableManager StreamableManager;

		//对文件的路径进行处理,转换成StaticLoadObject的那种路径格式

		FString AssetName = FileList[0];

		FString AssetShortName = FPackageName::GetShortName(AssetName);

		FString LeftStr;

		FString RightStr;

		AssetShortName.Split(TEXT("."), &LeftStr, &RightStr);

		AssetName = TEXT("/Engine/") + LeftStr + TEXT(".") + LeftStr;//我们加载的时候用的是这个路径

		FStringAssetReference reference = AssetName;

		//加载UObject

		UObject* LoadObject = StreamableManager.LoadSynchronous(reference);

		if (LoadObject != nullptr)

		{
			UE_LOG(LogClass, Log, TEXT("Object Load Success..."))

				//TheLoadObject = LoadObject;

		}

		else

		{
			UE_LOG(LogClass, Log, TEXT("Can not Load asset..."))

		}

	}

	else

	{
		UE_LOG(LogClass, Error, TEXT("Mount Failed"));

	}
	if (PakPath != "") {
		getPak("", PakPath2);
	}
	return false;
}


void UfileMgr::getFileList(TArray<FName>& outFileList)
{
	UObjectLibrary* objectLibrary = UObjectLibrary::CreateLibrary(UWorld::StaticClass(), false, GIsEditor);
	if (objectLibrary)
	{
		objectLibrary->AddToRoot();
	}

	objectLibrary->LoadAssetDataFromPath(TEXT("/Game"));
	TArray<FAssetData> arrayAssetData;
	objectLibrary->GetAssetDataList(arrayAssetData);

	for (int32 index = 0; index < arrayAssetData.Num(); ++index)
	{
		outFileList.Add(arrayAssetData[index].AssetName);
	}

}