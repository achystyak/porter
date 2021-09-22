# Porter

Fast, flexible, minimalist web framework written in C++.

## Features

* 🏗️ 	**HTTP REST Architectured**

* 🧬 	**Domain Driven Design**

* 🥬 	**Mongo Databased**

* 🚢 	**Docker Deployable**

* 🔎 	**JWT Authentication**

* 👀 	**Permission-based Authorization**

* 📅 	**Tasks Scheduling**

* 📻 	**WebSocket Connections**

* 🚇 	**MQTT Queues**

* 🌐 	**Curl Networking**

* 🏎️ 	**Model / API Code Generation**

* 📎 	**Tiny Config (including globally)**

* 📝 	**Easy Logging System**

# Requirements

- g++ (C++17)
- openssl
- make
- Mongocxx Driver https://github.com/mongodb/mongo-cxx-driver
- CPR library https://github.com/whoshuu/cpr.git

# Build and Run

`$_ dev.sh` - building and running local server

`$_ database.sh` - running mongodb docker container

`$_ deploy.sh` - running server docker container

---

# Fundamentals

## Server Initialization

`main.cpp`

```cpp
int main()
{
    // Creating server
    Server server;

    // Including User Resolver
    server.resolve(new UserResolver());

    // Starting server on the main thread
    server.start("localhost", 8000);

    return 0;
}
```

## Resolver

Resolvers are responsible for handling requests from the client, validating incoming parameters, and sending responses.
This is an example of implementing a resolver with a simple GET route `/users/test`:

`/api/user/user.resolver.hpp`

```cpp
class UserResolver : public Resolver
{
public:
    UserResolver() : Resolver("/users")
    {
        resolve(
            GET, "/test", [=](const Request &request)
            {
                json body = {{"status", "online"}};

                return Response::json(body.dump());
            });
    }
};
```

### Resolver public methods

`resolve()`

Creates resolve route for client requests

```cpp
void resolve(
        HttpMethod method,
        const std::string &path,
        const ResponseCallback &callback,
        ResolverOptions options = RO_NONE,
        Ability ability = Ability());
```

`schedule()`

Creates Cron Scheduler

```cpp

    Scheduler *schedule(
        const std::string &cron,
        const ScheduleCallback &callback);

    void unschedule(Scheduler *scheduler);
```

`publisher()`

Creates WebSocket Publisher

```cpp
    Publisher *publisher(
        const std::string &path,
        ResolverOptions options = RO_NONE);
```

Lifecycle methods

```cpp
// Lifecycle
    virtual void serverDidLoad();

    virtual void serverWillDestroy();
```

## Service

Services are responsible for accessing database resources. All services inherit the service base class and implement the singleton pattern.
Three parameters are passed to the macro: its own class (for the singleton template), the service domain type, and the name of the collection in the database.

This is an example of implementing a default service for working with a `users` collection:

`/api/user/user.service.hpp`

```cpp
class UserService : public Service<User>
{
    SERVICE(UserService, Domain::User, "users")

    virtual User findOne(const json &filter) override
    {
        // Your custom implementation here
    }
};
```

The base service class is templated by the model it will work with.
It is assumed that the model is capable of serializing to json format using the library `nlohmann::json`.

Base service implements basic CRUD model. Collections can overload the basic functions of the base service and implement other for their own needs.

```cpp
template <class Entity>
class Service
{
public:
    Service(Domain::Type domain, const std::string &collection);

    virtual Entity findOne(const json &filter);

    virtual std::vector<Entity> find(const json &filter);

    virtual const json create(const Entity &entity);

    virtual json update(const json &filter, const Entity &entity);

    virtual const json remove(const json &filter);
};
```

## Inputs

Inputs are used to store and validate incoming data.

The solution provides basic inputs to implement a CRUD model with built-in validation methods. All basic inputs are templated by the model that the handler operates on.

This is a list of basic inputs:

`/api/user/user.inputs.hpp`

```cpp
// Input for finding user by id
class FindOneUserInput : public FindOneInput
{
public:
    virtual bool parse(const std::string &data) override
    {
        // Your custom implementation here
    }
};

// Input for finding array of users
class FindUserInput : public FindInput<User>
{
    // Default implementation
};

// Input for creating single user
class CreateUserInput : public CreateInput<User>
{
public:
    virtual json toDocument() override
    {
        // Your custom implementation here
    }
};

// Input for updating users by filter
class UpdateUserInput : public UpdateInput<User>
{
    // Default implementation
};

// Input for deleting users by filter
class DeleteUserInput : public DeleteInput
{
public:
    virtual json toFilter() override
    {
        // your custom implementation here
    }
};
```

## Model

The model used in the solution must support two-way conversion to json model.

This makes MongoDB much easier to work with.
Best practice is to generate the model using the generation tools from this framework.

Simple example of a user model is provided below:

`/api/user/user.hpp`

```cpp
struct User
{
    std::string email;
    std::string password;
    std::string first_name;
    std::string last_name;
};

namespace nlohmann
{
    void from_json(const json &j, User &x);
    void to_json(json &j, const User &x);

    inline void from_json(const json &j, Id &x)
    {
        x.oid = j.at("$oid").get<std::string>();
    }

    inline void from_json(const json &j, User &x)
    {
        x.email = j.at("email").get<std::string>();
        x.password = j.at("password").get<std::string>();
        x.first_name = j.at("firstName").get<std::string>();
        x.last_name = j.at("lastName").get<std::string>();
    }

    inline void to_json(json &j, const User &x)
    {
        j = json::object();
        j["email"] = x.email;
        j["password"] = x.password;
        j["firstName"] = x.first_name;
        j["lastName"] = x.last_name;
    }
}
```

---

# Database

### MongoDB

This framework uses MongoDB to store and transfer data. Base functionality impletented in `MongoDB` class:

`/base/database/database.hpp`

```cpp
class MongoDB
{
public:
    static void init(const std::string &connection, const std::string &dbname);

    static std::vector<std::string> collections();

    static std::vector<std::string> find(const std::string &collection, const json &filter);

    static std::string findOne(const std::string &collection, const json &filter);

    static std::string create(const std::string &collection, const json &document);

    static std::string update(const std::string &collection, const json &filter, const json &document);

    static std::string remove(const std::string &collection, const json &filter);
};
```

Also we should init database in the `main.cpp` file before server initialization:

`main.cpp`

```cpp
int main()
{
    // Initializing database
    MongoDB::init("mongodb:://url", "database_name");

    // Creating server
    Server server;

    // Including App Resolver
    server.resolve(new UserResolver());

    // Starting server on the main thread
    server.start("localhost", 8000);

    return 0;
}
```

---

# Environment Variables

Environment variables are readed from file `config.json` which is located in a folder with server executable file.

There is an example of config file:

`config.json`

```json
{
  "url": "localhost",
  "port": 7000,
  "mongo": {
    "uri": "mongodb://<your-url>:27017",
    "database": "<database-name>"
  },
  "jwt": {
    "timeout": 3600,
    "secret": "<jwt-secret>"
  }
}
```

## env

Access to environment variables provided by `env` static variable.

`env.hpp`

```cpp
static json env;

json loadEnv()
{
    /// parsing config.json...

    return env;
}
```

---

# Authentication

Authentication is the process in which a subject proves that they are who they claim to be. This solution implements base auth classes and JWT-based realization.

### Auth Guard

`/base/auth/auth.guard.hpp`

```cpp
class AuthGuard
{
public:
    // Encodes user's info into crypted string
    virtual std::string encode(const nlohmann::json &payload) = 0;

    // Decodes user's info from crypted string
    virtual nlohmann::json decode(const std::string &jwt) = 0;

    // Validates crypted string signature
    virtual bool validate(const std::string &jwt) = 0;

    // Autheticates user's from request info (+ websocket)
    virtual bool authenticate(Request &request, bool wss = false) = 0;
};
```

### JWT Auth Guard

This solution has custom implementation of `AuthGuard` using JWT authorization.

`/auth/jwt/jwt.auth.guard.hpp`

```cpp
class JwtAuthGuard : public AuthGuard
{
public:
    std::string encode(const nlohmann::json &payload) override
    {
        // return JWT Token
    }

    nlohmann::json decode(const std::string &jwt) override
    {
        // decoding JWT token and return user's info
    }

    bool validate(const std::string &jwt) override
    {
        // validating JWT token signature (key is from env)
    }

    bool authenticate(Request &request, bool wss) override
    {
        // Checking Authorization header and modifying request with user's info
    }
};
```

## Enabling JWT Authentication

At first we should added `JwtAuthGuard` into server init in `main.cpp`:

`main.cpp`

```cpp
int main()
{
    // Initializing database
    MongoDB::init("mongodb:://url", "database_name");

    // Creating server
    Server server;

    // Including App Resolver
    server.useAuthGuard(new JwtAuthGuard());

    // Including App Resolver
    server.resolve(new UserResolver());

    // Starting server on the main thread
    server.start("localhost", 8000);

    return 0;
}
```

To enable Jwt authentication we should implement the `AuthResolver` and `AuthService` classes which provide `/login` and `/signup` methods. Notice that access to `AuthGuard` provided by `server()` resolver's getter.

`/auth/auth.resolver.hpp`

```C++
class AuthResolver : public Resolver
{
public:
    AuthResolver() : Resolver("/auth")
    {
        resolve(
            POST, "/signup", [=](const Request &request)
            { return AuthService::shared()->signup(request); });

        resolve(
            POST, "/login", [=](const Request &request)
            { return this->login(request); });

        resolve(
            GET, "/session", [=](const Request &request)
            { return Response::json(request.session.dump()); },
            RO_AUTH_GUARDED);
    }

    Response login(const Request &request)
    {
        json user = AuthService::shared()->login(request);
        if (user.is_object())
        {
            // Accessing to Auth Guard to encode data
            auto jwt = server()->authGuard()->encode(user);
            json response = {{"token", jwt}};

            return Response::json(response.dump());
        }
        return Response::badRequest();
    }
};
```

`/auth/auth.service.hpp`

```C++
class AuthService : public Service<User>
{
    SERVICE(AuthService, Domain::Auth, "users")

    json signup(const Request &request)
    {
        json payload = json::parse(request.body);

            result = {{"email", payload["email"]},
                      {"password", payload["password"]}};

        return this->create(result);
    }

    json login(const Request &request)
    {
        json result;
        json payload = json::parse(request.body);
        json filter = {{"email", payload["email"]},
                       {"password", payload["password"]}};

        auto data = this->find(filter);
        if (data.size())
        {
            result = {{"_id", data.front().id}};
        }
        return result;
    }
};
```

If we want to enable authrorization in resolver route, we should specify resolver option `RO_AUTH_GUARDED` like this:

`/api/user/user.resolver.hpp`

```cpp
class UserResolver : public Resolver
{
public:
    UserResolver() : Resolver("/users")
    {
        resolve(
            GET, "/test", [=](const Request &request)
            {
                json body = {{"status", "online"}};
                return Response::json(body.dump());
            },
            RO_AUTH_GUARDED);
    }
};
```

Now our `/users/test` is JWT authorizated. User's info stored in `request.session` variable

---

# Authorization

This solution has permission-based authorization, provided by `ability` classes.

### Ability

Ability model contains:

`action` - which action is allowed (Read, Create, Update, Delete)

`domain` - which action is allowed (User, etc...)

`filter` - used to filter which data is allowed

`/base/auth//ability/ability.hpp`

```cpp
class Ability
{
public:
    Action::Type action;
    Domain::Type domain;
    json filter;
};
```

### Ability Factory

Ability Factory provides methods for setting up abilities for concrete user. Wecan specify what user can or cannot doing with your API:

`/base/auth//ability/ability.factory.hpp`

```cpp
class AbilityFactory
{
public:

    // Singleton pattern implementation
    static AbilityFactory *shared();

    // Specifies what user CAN do in API
    AbilityFactory *can(
        const json &userId,
        Action::Type action,
        Domain::Type domain,
        json filter = json());

    // Specifies what user CAN NOT do in API
    AbilityFactory *cannot(
        const json &userId,
        Action::Type action,
        Domain::Type domain,
        json filter = json())

};
```

### Ability Guard

Ability Guard provides methods for build abilities for concrete user by `request.session` using app services.

There is a base interface of ability guard:

`/base/auth/ability/ability.guard.hpp`

```cpp
class AbilityGuard
{
public:
    AbilityGuard() {}
    virtual ~AbilityGuard() {}

    virtual void buildAbilities(AbilityFactory *factory, const json &session) = 0;
};
```

Default implementation of ability guard:

`/auth/ability/ability.guard.hpp`

```cpp
class DefaultAbilityGuard : public AbilityGuard
{
public:
    virtual void buildAbilities(AbilityFactory *factory, const json &session) override
    {
        User user = UserService::shared()->findOne(session);

        // Allow user to update users
        factory->can(user.id, Action::Update, Domain::User);

        // Deny user to delete users
        factory->cannot(user.id, Action::Delete, Domain::User);

        // Allow user to read only himself
        factory->can(user.id, Action::Read, Domain::User, {{"email", user.email}});

        // Allow user to create only users with email domain @mail.com
        factory->can(user.id, Action::Create, Domain::User, {
            {"email", {
                {"$regex": "@mail.com"},
                {"$options": "i"}}
            }});
    }
};
```

## Enabling Authorization

To enable authorization at first we should modify `DefaultAbilityGuard:buildAbilities()` method according to our needs.

Next we should specify allowed action and domain in the resolver route:

```cpp
class UserResolver : public Resolver
{
public:
    UserResolver() : Resolver("/users")
    {
        resolve(
            GET, "/test", [=](const Request &request)
            {
                json body = {{"status", "online"}};

                return Response::json(body.dump());
            },
            RO_AUTH_GUARDED,
            {Action::Read, Domain::User});
    }
};
```

Now our `/users/test` allowed only for users, that have ability to `Action::Read`:`Domain::User`.

Notice, that authorization requires `RO_AUTH_GUARDED` option.

---

# Cron Schedule

Cron is useful utility that allows running tasks on a given schedule without any user intervention. It is mostly used for automating recurring jobs like running scheduled backups, sending emails, system maintenance, and various other recurring jobs.

Cron functionality implemented using `Schduler` class.

### Scheduler

Base functionality of scheduler class:

`/base/scheduler/scheduler.hpp`

```cpp
class Scheduler
{
public:
    Scheduler(const std::string &cron, const ScheduleCallback &callback);

    // Getters
    const std::string &cron() const;
    const ScheduleCallback &callback() const;
};
```

Schduler start his cron schdule when initialized. Base Resolver provides method `schedule()` to schedule job.

There is an example of using schduler:

```cpp
class UserResolver : public Resolver
{
public:
    UserResolver() : Resolver("/users")
    {
        // Repeats job every minute
        schedule("* * * * *", [=]()
             { Log_Debug("Hi from cron!"); });
    }
};
```

# WebSocket Connection

A WebSocket is a persistent connection between a client and server. WebSockets provide a bidirectional, full-duplex communications channel that operates over HTTP through a single TCP/IP socket connection.
In this solution server-client communication provided by `Publisher` class.

### Publisher

Base functionality of Publisher class:

```cpp
class Publisher
{
public:
    Publisher(const std::string &path, ResolverOptions options = RO_NONE);
};
```

Publisher open connection when initialized. Publisher route may be auth guarded just like resolver routes.

Base Resolver provides method `publisher()` to operate with WebSocket connection.

There is an example of using publisher:

```cpp
class UserResolver : public Resolver
{
public:
    UserResolver() : Resolver("/users")
    {
        // Initialize WS route at /users/websoket
        m_pub = publisher("/websocket");

        // Repeats job every minute
        schedule("* * * * *", [=]()
                 { m_pub->publish("Hi from cron!"); });
    }

private:
    Publisher *m_pub;
};
```

---

# MQTT Connection

MQTT is a lightweight publish/subscribe messaging protocol designed for M2M (machine to machine) telemetry in low bandwidth environments.

### Queuer

You can initialize `Queuer` as server or as client:

`/base/queuer/queuer.hpp`

```cpp
class Queuer
{
public:
    static Queuer *host(const std::string &host, const size_t &port);
    static Queuer *client(const std::string &host, const size_t &port);

    bool send(const std::string &topic, const std::string &message);
    void subscribe(const QueuerCallback &callback);

    bool isHost() const;
};
```

As host you can use only `send()` method. As client - both `send()` and `subscribe()`. Method `isHost()` identify is queuer a host.

## Basic usage of MQTT queuer:

### Queuer::host

`main.cpp`

```cpp
int main()
{
    // Creating server
    Server server;

    // Starting Queuer::host
    server.addQueuer(Queuer::host("localhost", 1883));

    // Starting server on the main thread
    server.start("localhost", 8000);

    return 0;
}
```

`/api/user/user.resolver.hpp`

```cpp
class UserResolver : public Resolver
{
public:
    UserResolver() : Resolver("/users")
    {
        // Sending message via MQTT
        schedule("* * * * *",
        [=](){
            server()->hostQueuers().front()->send("topic", "It works!");
        });
    }
};
```

### Queuer::client

`main.cpp`

```cpp
int main()
{
    // Creating server
    Server server;

    // Subscribing to mqtt://localhost:1883
    server.addQueuer(Queuer::client("localhost", 1883));

    // Starting server on the main thread
    server.start("localhost", 8001);

    return 0;
}
```

`/api/user/user.resolver.hpp`

```cpp
class UserResolver : public Resolver
{
public:
    UserResolver() : Resolver("/users")
    {
        resolve(
            GET, "/test", [=](const Request &request)
            {
                server()->clientQueuers().front()->subscribe(
                    [=](const std::string &topic, const std::string &message) {
                        Log_Debug("MQTT: %s %s", topic.c_str(), message.c_str());
                    }
                );
            },
            RO_AUTH_GUARDED,
            {Action::Read, Domain::User});
    }
};
```

Now we have MQTT connection between two servers (8000 as host and 8001 as client).

---

# Networking

### Curl Client

Curl Client provides methods to make http requests. It uses `CPR` library and implement a decorator-pattern to hide implementation details.

Implementation:

`base/curl/curl.hpp`

```cpp
class Curl
{
public:
    typedef std::map<std::string, std::string> Headers;

    static Response Get(
        const std::string &uri,
        std::string body = "",
        Headers headers = Headers());

    static Response Post(
        const std::string &uri,
        std::string body = "",
        Headers headers = Headers());

    static Response Put(
        const std::string &uri,
        std::string body = "",
        Headers headers = Headers());

    static Response Patch(
        const std::string &uri,
        std::string body = "",
        Headers headers = Headers());

    static Response Delete(
        const std::string &uri,
        std::string body = "",
        Headers headers = Headers());
};
```

Basic usage of curl client:

```cpp
class UserResolver : public Resolver
{
public:
    UserResolver() : Resolver("/users")
    {
        resolve(
            GET, "/external", [=](const Request &request)
            {
                auto response = Curl::Get("https://www.google.com");
                json body = {
                    {"status", response.body.size()},
                    {"code", response.code}};

                return Response::json(body.dump());
            });
    }
};
```

---

# Logging

Implementation of logging functions:

`/base/logger/logger.hpp`

```cpp
#define Log_Info(fmt, ...)

#define Log_Success(fmt, ...)

#define Log_Debug(fmt, ...)

#define Log_Warning(fmt, ...)

#define Log_Error(fmt, ...)
```

Usage of logging functions:

```cpp
class UserResolver : public Resolver
{
public:
    UserResolver() : Resolver("/users")
    {
        Log_Debug("%s", "UserResolver initialized");
    }
};
```

---

# Model / API Generation

This solution provides model and CRUD api code generation from json model using js scripts. To enable this functionality you should install https://nodejs.org.

At first we should create a model in json format:

`/models/document.json`
```json
{
    "title": "string",
    "description": "string",
    "version": 0,
    "isEnabled": true,
    "options": [{
        "name": "string",
        "value": ["1", "2", "3"]
    }]
}
```

Next in terminal:

```bash
cd gen

# creates c++ document class according to document.json data
npm run gen document 

# also creates CRUD api classes
npm run gen document api 

# scanning MongoDB database and creates model and CRUD api for each collection
npm run gen:schema

```

---
