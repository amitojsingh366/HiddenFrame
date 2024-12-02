#include "crow.h"
#include <hiddenframe_headers.h>
#include <crow/middlewares/cors.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <utils.h>
#include <jwt-cpp/jwt.h>
#include <authorization.h>
#include <crow/middlewares/cookie_parser.h>
#include <snowflake.h>

using namespace std;

const string BASE_API_URL = "http://localhost:8080";

int main()
{
    Snowflake flaker(1, 1);

    crow::Crow<crow::CookieParser, crow::CORSHandler, AuthorizationMiddleware> app;

    CROW_ROUTE(app, "/")
        .methods(crow::HTTPMethod::GET)(
            []()
            { return "Hello, World!"; });

    CROW_ROUTE(app, "/images")
        // .CROW_MIDDLEWARES(app, AuthorizationMiddleware) // this was done for testing auth middleware
        .methods(crow::HTTPMethod::GET)(
            []()
            {
                string staticPath = "./static";
                vector<crow::json::wvalue> photos;

                for (const auto &entry : filesystem::directory_iterator(staticPath))
                {
                    string filename = entry.path().filename().string();
                    string id = filename.substr(0, filename.find_last_of('.')); // Remove the extension

                    crow::json::wvalue photo;
                    photo["id"] = id;
                    photo["filename"] = filename;
                    photo["url"] = BASE_API_URL + "/static/" + filename;
                    photos.push_back(photo);
                }

                crow::json::wvalue jsonResponse;
                jsonResponse = crow::json::wvalue::list(photos.begin(), photos.end());

                return jsonResponse;
            });

    CROW_ROUTE(app, "/decode/<string>")
        .methods(crow::HTTPMethod::POST)(
            [](const crow::request &req, const string &id)
            {
                try
                {
                    auto jsonBody = crow::json::load(req.body);
                    if (!jsonBody)
                    {
                        crow::json::wvalue errorResponse;
                        errorResponse["success"] = false;
                        errorResponse["message"] = "Invalid JSON";
                        return crow::response(400, errorResponse);
                    }

                    string key = jsonBody["key"].s();
                    string imagePath = "./static/" + id;

                    if (!filesystem::exists(imagePath))
                    {
                        crow::json::wvalue errorResponse;
                        errorResponse["success"] = false;
                        errorResponse["message"] = "Image not found";
                        return crow::response(404, errorResponse);
                    }

                    image imgptr = image(imagePath);
                    int skipsize = imgptr.decodeKey(key, imgptr.height * imgptr.width * imgptr.channels);
                    string payload = imgptr.retrieve_payload(skipsize);

                    crow::json::wvalue jsonResponse;
                    jsonResponse["success"] = true;
                    jsonResponse["message"] = binaryToString(payload);
                    return crow::response(200, jsonResponse);
                }
                catch (const std::exception &e)
                {
                    crow::json::wvalue errorResponse;
                    errorResponse["success"] = false;
                    errorResponse["message"] = e.what();
                    return crow::response(500, errorResponse);
                }
            });

    // internal route for POC pourposes only
    CROW_ROUTE(app, "/register/admin")
        .methods(crow::HTTPMethod::POST)(
            [](const crow::request &req)
            {
                try
                {
                    auto jsonBody = crow::json::load(req.body);
                    // username and password sent as json in req body
                    string username = jsonBody["username"].s();
                    string password = jsonBody["password"].s();
                    // int inviteId = jsonBody["inviteId"].i();

                    createNewAdmin(username, password);
                }
                catch (const std::runtime_error &e)
                {
                    crow::json::wvalue error_json;
                    error_json["success"] = true;
                    error_json["error"] = e.what();
                    cout << e.what() << endl;
                    return crow::response(401, error_json);
                }
                crow::json::wvalue success_json;
                success_json["success"] = true;
                return crow::response(200, success_json);
            });

    CROW_ROUTE(app, "/register")
        .methods(crow::HTTPMethod::POST)(
            [](const crow::request &req)
            {
                try
                {
                    auto jsonBody = crow::json::load(req.body);
                    // username and password sent as json in req body
                    string username = jsonBody["username"].s();
                    string password = jsonBody["password"].s();
                    int inviteId = jsonBody["inviteId"].i();

                    createNewUser(username, password, inviteId);
                }
                catch (const std::runtime_error &e)
                {
                    crow::json::wvalue error_json;
                    error_json["success"] = true;
                    error_json["error"] = e.what();
                    return crow::response(401, error_json);
                }
                crow::json::wvalue success_json;
                success_json["success"] = true;
                return crow::response(200, success_json);
            });

    CROW_ROUTE(app, "/login")
        .methods(crow::HTTPMethod::POST)(
            [&app, &flaker](const crow::request &req)
            {
                auto jsonBody = crow::json::load(req.body);
                // username and password sent as json in req body
                string username = jsonBody["username"].s();
                string password = jsonBody["password"].s();
                // check username and password against database here
                bool validcredentials = authenticateUser(username, password);
                // TEST
                if (validcredentials)
                {
                    cout << "user authentication was successful";
                }
                // if valid, generate token and return it
                if (validcredentials)
                {
                    string tokenId = to_string(flaker.nextId()); // this needs to more random. store this in
                    // store the token in the .
                    string secret = std::getenv("JWT_SECRET");
                    // int expTime = (int)std::getenv("JWT_EXP_HOURS");
                    //  create token and set to exp in 3 days
                    auto token = jwt::create()
                                     .set_type("JWS")
                                     .set_issuer("HiddenFrame")
                                     .set_id(tokenId)
                                     .set_payload_claim("username", jwt::claim(username))
                                     .set_issued_at(std::chrono::system_clock::now())
                                     .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours{3000})
                                     .sign(jwt::algorithm::hs256{secret});

                    saveToken(username, tokenId);

                    auto &ctx = app.get_context<crow::CookieParser>(req);
                    ctx.set_cookie("token", token)
                        .path("/")
                        .max_age(120); // this needs to be same as token expiry time

                    crow::json::wvalue success_json;
                    success_json["success"] = true;
                    success_json["token"] = token;
                    return crow::response(200, success_json);
                }
                else
                {
                    crow::json::wvalue error_json;
                    error_json["success"] = true;
                    error_json["error"] = "Invalid credentials";
                    return crow::response(401, error_json);
                }
            });

    CROW_ROUTE(app, "/user")
        .methods(crow::HTTPMethod::PATCH)
        .CROW_MIDDLEWARES(app, AuthorizationMiddleware)(
            [&app](const crow::request &req)
            {
                auto jsonBody = crow::json::load(req.body);
                if (!jsonBody)
                {
                    return crow::response(400, "Invalid JSON");
                }

                auto &ctx = app.get_context<AuthorizationMiddleware>(req);
                string newPassword = jsonBody["password"].s();

                try
                {
                    bool success = changePassword(ctx.username, newPassword);
                    if (success)
                    {
                        crow::json::wvalue jsonResponse;
                        jsonResponse["success"] = true;
                        jsonResponse["message"] = "Password changed successfully";
                        return crow::response(200, jsonResponse);
                    }
                    else
                    {
                        crow::json::wvalue jsonResponse;
                        jsonResponse["success"] = false;
                        jsonResponse["message"] = "Failed to change password";
                        return crow::response(500, jsonResponse);
                    }
                }
                catch (const std::exception &e)
                {
                    crow::json::wvalue errorResponse;
                    errorResponse["success"] = false;
                    errorResponse["message"] = e.what();
                    return crow::response(500, errorResponse);
                }
            });
    CROW_ROUTE(app, "/invites")
        .methods(crow::HTTPMethod::GET)
        .CROW_MIDDLEWARES(app, AuthorizationMiddleware)(
            [&app](const crow::request &req)
            {
                auto &ctx = app.get_context<AuthorizationMiddleware>(req);
                vector<crow::json::wvalue> invites = listInvites(ctx.userId);

                crow::json::wvalue jsonResponse;
                jsonResponse = crow::json::wvalue::list(invites.begin(), invites.end());

                return jsonResponse;
            });

    CROW_ROUTE(app, "/invites/create")
        .methods(crow::HTTPMethod::POST)
        .CROW_MIDDLEWARES(app, AuthorizationMiddleware)(
            [&app](const crow::request &req, crow::response &res)
            {
                auto &ctx = app.get_context<AuthorizationMiddleware>(req);
                try
                {
                    int inviteId = createInvite(ctx.username);
                    if (inviteId == -1)
                    {
                        crow::json::wvalue error_json;
                        error_json["success"] = false;
                        error_json["message"] = "User has reached the maximum number of invites.";
                        res = crow::response(401, error_json);
                        res.end();
                    }
                    crow::json::wvalue success_json;
                    success_json["success"] = true;
                    success_json["inviteId"] = inviteId;
                    res = crow::response(200, success_json);
                }
                catch (const std::exception &e)
                {
                    crow::json::wvalue error_json;
                    error_json["success"] = false;
                    error_json["error"] = e.what();
                    cout << e.what() << endl;
                    res = crow::response(500, error_json);
                }
                res.end();
            });

    CROW_ROUTE(app, "/image/upload")
        .methods(crow::HTTPMethod::POST)(
            [&app, &flaker](const crow::request &req)
            {
                int64_t random = flaker.nextId();

                string fileData;       // @patrick: this is the image data
                string metaDataString; // @patrick: this is of format { name: string, size: int, ext: string }, size is file size, ext is file extension
                string message;        // this will remain uninitialized if no message sent from frontend (FUTURE: it'll be uninitialized if no/invalid token provided)

                auto content_type = req.get_header_value("Content-Type");

                if (content_type.find("multipart/form-data") != string::npos)
                {
                    crow::multipart::message msg(req);

                    auto filePart = msg.get_part_by_name("file");
                    auto metaPart = msg.get_part_by_name("meta");
                    auto messagePart = msg.get_part_by_name("message");

                    if (!filePart.body.empty())
                    {
                        fileData = filePart.body;
                    }
                    else
                    {
                        crow::json::wvalue error_json;
                        error_json["success"] = false;
                        error_json["error"] = "No image found in the request";
                        return crow::response(400, error_json);
                    }

                    if (!metaPart.body.empty())
                    {
                        metaDataString = metaPart.body;
                    }
                    else
                    {
                        crow::json::wvalue error_json;
                        error_json["success"] = false;
                        error_json["error"] = "No metadata found in the request";
                        return crow::response(400, error_json);
                    }

                    if (!messagePart.body.empty())
                    {
                        message = messagePart.body;
                    }

                    try
                    {
                        // image i/o here
                        auto meta = crow::json::load(metaDataString);
                        if (!meta)
                        {
                            crow::json::wvalue error_json;
                            error_json["success"] = false;
                            error_json["error"] = "Invalid JSON in metadata";
                            return crow::response(400, error_json);
                        }
                        string fileExt = meta["ext"].s();
                        string fileName = to_string(random) + "." + fileExt;
                        string filePath = "./static/" + fileName;
                        string key;

                        int fileSize = meta["size"].i();

                        // check if token is sent, then verify token
                        auto &cookie_ctx = app.get_context<crow::CookieParser>(req);
                        std::string token = cookie_ctx.get_cookie("token");
                        auto [isAuthed, _] = verify_token(token);

                        cout << "token" << token << endl;
                        cout << "isAuthed: " << isAuthed << endl;

                        if (message != "" && isAuthed)
                        {
                            std::vector<unsigned char> convertedData(fileSize + 1);
                            memcpy(convertedData.data(), fileData.c_str(), fileSize + 1);
                            image imgptr = image(convertedData.data(), fileSize, fileExt);
                            string messageBN = stringToBinary(message);
                            int skipSize = imgptr.calculateSkipSize(imgptr.height * imgptr.width * imgptr.channels, convertedData.size() / 2);
                            cout << "The ideal skip size is " << skipSize << endl;
                            imgptr.modify_image(skipSize, messageBN);
                            imgptr.write_image(filePath);
                            key = imgptr.generateKey(imgptr.height * imgptr.width * imgptr.channels, imgptr.channels, skipSize);
                            // AMITOJ, Here is the key.
                            cout << "The image size is " << imgptr.height * imgptr.width * imgptr.channels << endl;
                        }
                        else
                        {
                            ofstream outputFile(filePath, ios::out | ios::binary);
                            if (outputFile)
                            {
                                outputFile << fileData;
                                outputFile.close();
                            }
                            else
                            {
                                crow::json::wvalue error_json;
                                error_json["success"] = false;
                                error_json["error"] = "File writing failed";
                                return crow::response(500, error_json);
                            }
                        }

                        crow::json::wvalue success_json;
                        success_json["success"] = true;
                        success_json["url"] = BASE_API_URL + "/static/" + fileName;
                        if (!key.empty())
                            success_json["key"] = key;

                        return crow::response(200, success_json);
                    }
                    catch (const exception &e)
                    {
                        crow::json::wvalue error_json;
                        error_json["success"] = false;
                        error_json["error"] = e.what();
                        return crow::response(500, error_json);
                    }
                }
                else
                {
                    crow::json::wvalue error_json;
                    error_json["success"] = false;
                    error_json["error"] = "Invalid content type";
                    return crow::response(400, error_json);
                }
            });

    app.port(8080).multithreaded().run();
}