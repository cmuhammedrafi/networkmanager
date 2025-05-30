/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2025 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#include "Module.h"
#include <iostream>


using namespace WPEFramework;
using namespace std;

#define LOG_RED(fmt, ...)   printf("\033[1;31m" fmt "\033[0m\n", ##__VA_ARGS__)
#define LOG_GREEN(fmt, ...) printf("\033[1;32m" fmt "\033[0m\n", ##__VA_ARGS__)


std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> jsonPlugin;

std::string getjsonResponse(std::string methodName, const JsonObject parameters = JsonObject())
{
    std::string result;
    JsonObject response;
    jsonPlugin->Invoke<JsonObject, JsonObject>(5000, _T(methodName.c_str()), parameters, response);
    response.ToString(result);
    if(response.HasLabel(_T("success")) && response[_T("success")].Boolean() == false) {
        LOG_RED("Failed to get %s: %s", methodName.c_str(), response[_T("message")].String().c_str());
        return "";
    }

    if (result.empty()) {
        LOG_RED("No response received for %s", methodName.c_str());
        return "";
    }

    LOG_GREEN("%s response: %s", methodName.c_str(), result.c_str());
    return result;
}

int main ()
{
    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));
    jsonPlugin = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >("org.rdk.NetworkManager.1", "");
    if (jsonPlugin == nullptr) {
        cout << "Failed to create JSONRPC link" << endl;
        return -1;
    }

    getjsonResponse("GetIPSettings");
    getjsonResponse("IsConnectedToInternet");
    getjsonResponse("GetCaptivePortalURI");
    getjsonResponse("GetConnectivityCheckEndpoints");
    getjsonResponse("GetCaptivePortalURI");
    getjsonResponse("GetPrimaryInterface");
    getjsonResponse("GetAvailableInterfaces");
    getjsonResponse("GetLogLevel");
    getjsonResponse("GetStunEndpoint");
    getjsonResponse("GetPublicIP");
}