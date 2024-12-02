#include "authorization.h"
#include <jwt-cpp/jwt.h>
#include <hiddenframe_headers.h>
#include <crow/middlewares/cookie_parser.h>
#include <crow/middlewares/cors.h>
#include <iostream>

using namespace std;

std::pair<bool, std::variant<std::string, std::tuple<int, std::string, std::string>>> verify_token(const std::string &token)
{
    try
    {
        auto decoded = jwt::decode(token);

        if (decoded.has_id() && decoded.has_expires_at())
        {
            if (decoded.get_expires_at() < std::chrono::system_clock::now())
            {
                return {false, "Token expired"};
            }

            string token_id = decoded.get_id();
            string secret = std::getenv("JWT_SECRET");

            auto verifier = jwt::verify()
                                .with_issuer("HiddenFrame")
                                .allow_algorithm(jwt::algorithm::hs256{secret});
            verifier.verify(decoded); // this will error if verification fails

            auto [user_id, username] = verifyTokenWithDb(token_id); // verify with db

            return {true, std::make_tuple(user_id, token_id, username)};
        }
        else
        {
            return {false, "Invalid token"};
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Token verification failed: " << e.what() << std::endl;
        return {false, e.what()};
    }
}

template <typename AllContext>
void AuthorizationMiddleware::before_handle(crow::request &req, crow::response &res, context &ctx, AllContext &all_ctx)
{
    try
    {
        string token = req.get_header_value("Authorization");

        cout << token << endl;

        if (token.empty())
        {
            auto &cookie_ctx = all_ctx.template get<crow::CookieParser>();
            token = cookie_ctx.get_cookie("token");
        }

        cout << "Token: " << token << endl;

        auto [success, result] = verify_token(token);

        if (!success)
        {
            crow::json::wvalue error_json;
            error_json["success"] = false;
            error_json["error"] = std::get<std::string>(result);
            res = crow::response(401, error_json);
            res.end();
            return;
        }

        auto [user_id, token_id, username] = std::get<std::tuple<int, std::string, std::string>>(result);
        ctx.userId = user_id;
        ctx.tokenId = token_id;
        ctx.username = username;

        cout << "User: " << username << " with id: " << user_id << endl;
    }
    catch (const std::exception &e)
    {
        crow::json::wvalue error_json;
        error_json["success"] = false;
        error_json["error"] = e.what();
        res = crow::response(401, error_json);
        res.end();
        return;
    }
}

//@ amitoj - I commented out args to suppress compile warnings
void AuthorizationMiddleware::after_handle(crow::request & /*req*/, crow::response & /*res*/, context & /*ctx*/)
{
    // Implement any post-processing logic here
}

// Explicit template instantiation
template void AuthorizationMiddleware::before_handle<crow::detail::context<crow::CookieParser, crow::CORSHandler, AuthorizationMiddleware>>(crow::request &req, crow::response &res, context &ctx, crow::detail::context<crow::CookieParser, crow::CORSHandler, AuthorizationMiddleware> &all_ctx);