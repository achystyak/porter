#include "env.hpp"
#include "base/server/server.hpp"

#include "auth/auth.resolver.hpp"
#include "auth/jwt/jwt.auth.guard.hpp"

#include "api/app/app.resolver.hpp"
#include "api/schema/schema.resolver.hpp"
#include "api/user/user.resolver.hpp"

int main(int argc, char **argv)
{
	if (loadEnv().is_null())
		return 1;

	MongoDB::init(
		env["mongo"]["uri"],
		env["mongo"]["database"]);

	Server server;

	server.useAuthGuard(new JwtAuthGuard());

	server.resolve(new AuthResolver());
	server.resolve(new AppResolver());
	server.resolve(new SchemaResolver());
	server.resolve(new UserResolver());

	server.start(env["url"], env["port"]);

	return 0;
}
