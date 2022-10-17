// Fill out your copyright notice in the Description page of Project Settings.


#include "fileMgr.h"

#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Engine/Engine.h"
#include "IPlatformFilePak.h"
#include "Engine/StreamableManager.h"
#include "Engine/ObjectLibrary.h"


//����һ��Ŀ¼��·������λ����·��
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
//��Ӧ�ģ�ɾ������·����Ŀ¼
void UfileMgr::DeleteFolder(FString Path)
{
	FPlatformFileManager::Get().Get().GetPlatformFile().DeleteDirectoryRecursively(*Path);
}

bool UfileMgr::DeleteFile(FString Path)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	return PlatformFile.DeleteFile(*Path);//���ﲻһ����
}

//��ȡĳһĿ¼�µ������ļ�
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

	//2.���ظղű�����ļ�

	//��ȡ��ǰʹ�õ�ƽ̨,����ʹ�õ���WIN64ƽ̨

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	//��ʼ��PakPlatformFile

	FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();

	PakPlatformFile->Initialize(&PlatformFile, TEXT(""));

	FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);
	FPakFile* PakFile = new FPakFile(&PlatformFile, *SaveContentDir, false);
	return PakPlatformFile->Unmount(*SaveContentDir);
}

bool UfileMgr::getPak(const FString& PakPath, const FString& PakPath2)
{
	//1.�����غõ��ļ���������

	FString SaveContentDir = PakPath;

	//2.���ظղű�����ļ�

	//��ȡ��ǰʹ�õ�ƽ̨,����ʹ�õ���WIN64ƽ̨

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	//��ʼ��PakPlatformFile

	FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();

	PakPlatformFile->Initialize(&PlatformFile, TEXT(""));

	FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);

	//��ȡPak�ļ�

	FPakFile* PakFile = new FPakFile(&PlatformFile, *SaveContentDir, false);

	UE_LOG(LogClass, Log, TEXT("get PakFile..."))

		//����pak�ļ���Mount��.

		FString MountPoint(FPaths::EngineContentDir()); //"/../../../Engine/Content/"��Ӧ·��

	//PakFile->SetMountPoint(*PakPath2);

	//��pak�ļ�mount��ǰ���趨��MountPoint

	if (PakPlatformFile->Mount(*SaveContentDir, 0, *PakPath2))

	{
		UE_LOG(LogClass, Log, TEXT("Mount Success"));

		TArray<FString> FileList;

		//�õ�Pak�ļ���MountPoint·���µ������ļ�

		PakFile->FindFilesAtPath(FileList, *PakFile->GetMountPoint(), true, false, true);

		FStreamableManager StreamableManager;

		//���ļ���·�����д���,ת����StaticLoadObject������·����ʽ

		FString AssetName = FileList[0];

		FString AssetShortName = FPackageName::GetShortName(AssetName);

		FString LeftStr;

		FString RightStr;

		AssetShortName.Split(TEXT("."), &LeftStr, &RightStr);

		AssetName = TEXT("/Engine/") + LeftStr + TEXT(".") + LeftStr;//���Ǽ��ص�ʱ���õ������·��

		FStringAssetReference reference = AssetName;

		//����UObject

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