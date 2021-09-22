#pragma once

#include <string>

#define Case(Name, Value) const static std::string Name = Value

namespace Action
{
    Case(None, "None");
    Case(All, "All");
    Case(Read, "Read");
    Case(Create, "Create");
    Case(Update, "Update");
    Case(Delete, "Delete");

    typedef std::string Type;
}

namespace Domain
{
    Case(None, "None");
    Case(All, "All");
    Case(App, "App");
    Case(Auth, "Auth");
    Case(Schema, "Schema");
    Case(Util, "Util");
    Case(User, "User");

    typedef std::string Type;
}