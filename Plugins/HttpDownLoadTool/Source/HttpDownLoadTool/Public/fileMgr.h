// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Core/Public/Misc/Paths.h"
#include "fileMgr.generated.h"

/**
 * 
 */
UCLASS()
class HTTPDOWNLOADTOOL_API UfileMgr : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	UFUNCTION(BlueprintCallable, Category = "fileMgr")
		static bool getPak(const FString& path, const FString& PakPath2);
	UFUNCTION(BlueprintCallable, Category = "fileMgr")
		static bool delPak(const FString& path);
	
	UFUNCTION(BlueprintCallable, Category = "fileMgr")
		static void CreateFolder(FString Path);
	UFUNCTION(BlueprintCallable, Category = "fileMgr")
		static void DeleteFolder(FString Path);
	UFUNCTION(BlueprintCallable, Category = "fileMgr")
		static bool DeleteFile(FString Path);
	
	UFUNCTION(BlueprintCallable, Category = "fileMgr")
		static void GetFolderFiles(TArray<FString>& Files, FString Path);
	UFUNCTION(BlueprintCallable, Category = "fileMgr")
		static bool CopyFile(FString to, FString From);

	UFUNCTION(BlueprintCallable, Category = "fileMgr")
		static bool MoveFile(FString to, FString From);

	UFUNCTION(BlueprintCallable, Category = "fileMgr")
		static void getFileList(TArray<FName>& outFileList);
};
