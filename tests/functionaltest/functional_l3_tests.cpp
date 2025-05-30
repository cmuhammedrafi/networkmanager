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
#include <core/core.h>
#include <websocket/websocket.h>
#include <jsonrpc/jsonrpc.h>


using namespace WPEFramework;
using namespace std;

int main ()
{
    sleep(60);
    Core::SystemInfo::SetEnvironment(_T("THUNDER_ACCESS"), (_T("127.0.0.1:9998")));

    std::shared_ptr<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement>> jsonPlugin;
    jsonPlugin = make_shared<WPEFramework::JSONRPC::LinkType<WPEFramework::Core::JSON::IElement> >("org.rdk.NetworkManager.1", "");
    if (jsonPlugin == nullptr) {
        cout << "Failed to create JSONRPC link" << endl;
        return -1;
    }
}