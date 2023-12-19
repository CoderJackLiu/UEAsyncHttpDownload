# UEAsyncHttpDownloadPlugin
HTTP Large File Download Plugin

## Introduction

This project is a plugin for HTTP downloading of large files, featuring asynchronous and segmented download capabilities, with offline download support. It efficiently handles large file download tasks and is ideal for scenarios requiring asynchronous downloading of large data files from the web.

## Features

- **Asynchronous Download**: Supports background asynchronous downloading, preventing the blocking of the main thread and enhancing program efficiency.
- **Segmented Download**: Implements segmented file downloading to optimize network bandwidth usage and increase download speeds.
- **Customizable Segment Size**: Allows users to set the size of each segment, accommodating different network environments and file sizes.
- **Automatic File Saving**: Automatically saves completed files to a specified path.
- **MD5 Verification**: Includes MD5 verification to ensure the integrity of the downloaded file, requiring the server to provide the corresponding MD5 value.
- **Offline Download Support**: Enables downloading files when offline, allowing for greater flexibility in file management and access.

## How to Use

1. **Import the Plugin**: Integrate the plugin into your project to get started.
2. **Blueprint Usage**:
   - Access `UHttpMgrSubsystem` using Blueprints.
   - Call `DownLoadHttpFileWithInterfaceUrl`, passing the relevant parameters for the file download.
3. **C++ Integration**:
   - Retrieve `UHttpMgrSubsystem` in C++.
   - Invoke `DownLoadHttpFileWithInterfaceUrl` to initiate the download process with the necessary parameters.

## Contribution Guidelines

We welcome contributions that enhance or optimize this plugin. Please submit your improvements via Pull Requests on GitHub or propose suggestions in the issues section.

## License

This project is released under the MIT License. For more information, please see the [LICENSE](LICENSE) file.

---

# HTTP大文件下载插件

## 简介

本项目是一个用于HTTP下载大文件的插件，支持异步和分片下载功能，并且支持离线下载。该插件能够高效地处理大文件下载任务，适合需要从网络异步下载大型数据文件的场景。

## 功能特点

- **异步下载**: 支持后台异步下载，避免阻塞主线程，提高程序运行效率。
- **分片下载**: 实现文件分片下载，优化网络带宽使用，提升下载速度。
- **自定义分片大小**: 允许用户自定义设置每个分片的大小，以适应不同的网络环境和文件大小。
- **自动文件保存**: 下载完成的文件会自动保存到指定路径。
- **MD5校验**: 包含MD5校验功能，确保下载文件的完整性，需要服务器返回相应的MD5值。
- **支持离线下载**: 支持在离线状态下下载文件，增强文件管理和访问的灵活性。

## 如何使用

1. **导入插件**：将插件集成到您的项目中开始使用。
2. **蓝图调用**：
   - 通过蓝图获取 `UHttpMgrSubsystem`。
   - 调用 `DownLoadHttpFileWithInterfaceUrl`，传入相应的下载参数。
3. **C++集成**：
   - 在C++中获取 `UHttpMgrSubsystem`。
   - 使用 `DownLoadHttpFileWithInterfaceUrl` 来启动下载过程，并传入必要的参数。

## 贡献指南

欢迎对此插件进行功能增强或优化的贡献。请通过GitHub提交Pull Request或在issues区提出建议。

## 许可证

本项目在MIT许可证下发布。有关更多信息，请查看 [LICENSE](LICENSE) 文件。
