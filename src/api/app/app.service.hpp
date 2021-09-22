#pragma once
#include "../../base/service/service.hpp"
#include "../../base/libs/json/json.hpp"
#include "../../base/common/shared.hpp"

using namespace nlohmann;

class AppService : public Service<json>
{
    SERVICE(AppService, Domain::App, "app_info")
};