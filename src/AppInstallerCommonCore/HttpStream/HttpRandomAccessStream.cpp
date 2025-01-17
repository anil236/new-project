// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "HttpRandomAccessStream.h"
#include "Public/AppInstallerDownloader.h"

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage::Streams;

// Note: the HttpRandomAccessStream is passed to the AppxPackaging COM API
// All exceptions thrown across dll boundaries should be WinRT exception not custom exceptions.
// The HRESULTs will be mapped to UI error code by the appropriate component
namespace AppInstaller::Utility::HttpStream
{
    IAsyncOperation<IRandomAccessStream> HttpRandomAccessStream::InitializeAsync(const Uri& uri)
    {
        auto strong_this{ get_strong() };

        try
        {
            strong_this->m_httpHelper = co_await HttpClientWrapper::CreateAsync(uri);
            strong_this->m_size = strong_this->m_httpHelper->GetFullFileSize();
            strong_this->m_httpLocalCache = std::make_unique<HttpLocalCache>();
        }
        catch (const ServiceUnavailableException& e)
        {
            strong_this->m_retryAfter = e.RetryAfter();
            throw;
        }

        co_return strong_this.as<IRandomAccessStream>();
    }

    uint64_t HttpRandomAccessStream::Size() const
    {
        return m_size;
    }

    void HttpRandomAccessStream::Size(uint64_t value)
    {
        UNREFERENCED_PARAMETER(value);
        THROW_HR(E_NOTIMPL);
    }

    uint64_t HttpRandomAccessStream::Position() const
    {
        return m_requestedPosition;
    }

    bool HttpRandomAccessStream::CanRead() const
    {
        return true;
    }

    bool HttpRandomAccessStream::CanWrite() const
    {
        return false;
    }

    IInputStream HttpRandomAccessStream::GetInputStreamAt(uint64_t position) const
    {
        UNREFERENCED_PARAMETER(position);
        THROW_HR(E_NOTIMPL);
    }

    IOutputStream HttpRandomAccessStream::GetOutputStreamAt(uint64_t position) const
    {
        UNREFERENCED_PARAMETER(position);
        THROW_HR(E_NOTIMPL);
    }

    IRandomAccessStream HttpRandomAccessStream::CloneStream() const
    {
        THROW_HR(E_NOTIMPL);
    }

    void HttpRandomAccessStream::Seek(uint64_t position)
    {
        m_requestedPosition = position;
    }

    IAsyncOperationWithProgress<IBuffer, uint32_t> HttpRandomAccessStream::ReadAsync(
        IBuffer buffer,
        uint32_t count,
        InputStreamOptions options)
    {
        IBuffer result = co_await m_httpLocalCache->ReadFromCacheAndDownloadIfNecessaryAsync(
            m_requestedPosition,
            count,
            m_httpHelper.get(),
            options);
        winrt::check_hresult(ULong64Add(m_requestedPosition, result.Length(), &m_requestedPosition));

        co_return result;
    }

    std::chrono::seconds HttpRandomAccessStream::RetryAfter() const
    {
        return m_retryAfter;
    }
}