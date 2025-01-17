// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <cpprest/http_client.h>

class TestRestRequestHandler : public web::http::http_pipeline_stage
{
public:
    TestRestRequestHandler(const std::function<pplx::task<web::http::http_response>(web::http::http_request request)>& handler) : m_handler(handler) {}

    virtual pplx::task<web::http::http_response> propagate(web::http::http_request request)
    {
        return m_handler(request);
    }

private:
    std::function<pplx::task<web::http::http_response>(web::http::http_request request)> m_handler;
};

std::shared_ptr<TestRestRequestHandler> GetTestRestRequestHandler(
    const web::http::status_code statusCode, const utility::string_t& sampleResponseString = {}, const utility::string_t& mimeType = web::http::details::mime_types::application_json);

std::shared_ptr<TestRestRequestHandler> GetTestRestRequestHandler(
    std::function<web::http::status_code(const web::http::http_request& request)> handler);

std::shared_ptr<TestRestRequestHandler> GetHeaderVerificationHandler(
    const web::http::status_code statusCode, const utility::string_t& sampleResponseString, const std::pair<utility::string_t, utility::string_t>& header, web::http::status_code statusCodeOnFailure = web::http::status_codes::BadRequest);
