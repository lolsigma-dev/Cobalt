//
// Created by @binninwl_ on 11/08/2025.
//
#pragma once
#include "HTTP.hpp"
#include <cpr/cpr.h>

enum RequestMethods
{
    H_GET,
    H_HEAD,
    H_POST,
    H_PUT,
    H_DELETE,
    H_OPTIONS
};

inline std::map<std::string, RequestMethods> RequestMethodMap = {
   { "get", H_GET },
   { "head", H_HEAD },
   { "post", H_POST },
   { "put", H_PUT },
   { "delete", H_DELETE },
   { "options", H_OPTIONS }
};


int CHTTP::GetObjects(lua_State* L)
{
    lua_getglobal(L, "game");
    lua_getfield(L, -1, "GetService");
    lua_pushvalue(L, -2);
    lua_pushstring(L, "InsertService");
    lua_call(L, 2, 1);

    lua_getfield(L, -1, "LoadLocalAsset");
    lua_pushvalue(L, -2);
    lua_pushstring(L, lua_tostring(L, 2));
    lua_call(L, 2, 1);

    lua_newtable(L);
    lua_pushvalue(L, -2);
    lua_rawseti(L, -2, 1);
    return 1;
}

int CHTTP::HTTPGet(lua_State* L)
{
    std::string Url;

    if (!lua_isstring(L, 1))
    {
        luaL_checkstring(L, 2);
        Url = lua_tostring(L, 2);
    }
    else
    {
        Url = lua_tostring(L, 1);
    }

    if (Url.find("http://") != 0 && Url.find("https://") != 0)
    {
        luaL_argerror(L, 2, "Invalid protocol (expected 'http://' or 'https://')");
        return 0;
    }

    std::string GameId;
    lua_getglobal(L, "game");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "GameId");
        if (lua_isstring(L, -1))
            GameId = lua_tostring(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    std::string PlaceId;
    lua_getglobal(L, "game");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "PlaceId");
        if (lua_isstring(L, -1))
            PlaceId = lua_tostring(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    std::optional<std::string> HWID;
    HW_PROFILE_INFO hwProfileInfo;
    GetCurrentHwProfile(&hwProfileInfo);

    using Json = nlohmann::json;
    cpr::Header Headers;
    Json SessionIdJson;

    SessionIdJson["GameId"] = GameId;
    SessionIdJson["PlaceId"] = PlaceId;

    Headers.insert({ "User-Agent", "Roblox/Cobalt" });
    Headers.insert({ "Roblox-Session-Id", SessionIdJson.dump() });
    Headers.insert({ "Roblox-Place-Id", PlaceId });
    Headers.insert({ "Roblox-Game-Id", GameId });
    Headers.insert({ "Exploit-Identifier", "Cobalt" });
    Headers.insert({ "Exploit-Guid", HWID.value_or("Unknown") });
    Headers.insert({ "Cobalt-Fingerprint", HWID.value_or("Unknown") });
    Headers.insert({ "Accept", "*/*" });

    return Yielder->YielderExecution(L, [Url, Headers]() -> std::function<int(lua_State*)>
        {
            cpr::Response Result;
            try
            {
                Result = cpr::Get(cpr::Url{ Url }, cpr::Header(Headers));
            }
            catch (const std::exception& ex)
            {
                std::stringstream Error;
                Error << "HTTPGet failed: " << ex.what();
                std::string ErrorString = Error.str();
                return [ErrorString](lua_State* L) -> int
                    {
                        lua_pushstring(L, ErrorString.c_str());
                        return 1;
                    };
            }
            catch (...)
            {
                return [](lua_State* L) -> int
                    {
                        lua_pushstring(L, "HTTPGet failed: unknown exception");
                        return 1;
                    };
            }

            return [Result, Url](lua_State* L) -> int
                {
                    if (Result.error.code != cpr::ErrorCode::OK)
                    {
                        std::stringstream Error;
                        Error << "HTTPGet failed: " << Result.error.message << ", Code: " << Result.status_code;
                        std::string ErrorString = Error.str();
                        lua_pushstring(L, ErrorString.c_str());
                        return 1;
                    }

                    if (Result.status_code != 200)
                    {
                        std::stringstream Error;
                        Error << "HTTPGet returned status: " << Result.status_code << ", Error: " << Result.error.message;
                        std::string ErrorString = Error.str();
                        lua_pushstring(L, ErrorString.c_str());
                        return 1;
                    }

                    lua_pushlstring(L, Result.text.data(), Result.text.size());
                    return 1;
                };
        });
}

int request(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_getfield(L, 1, "Url");
    if (!lua_isstring(L, -1))
        luaL_error(L, "Missing or invalid 'Url'");

    std::string Url = lua_tostring(L, -1);
    lua_pop(L, 1);

    if (Url.find("http://") != 0 && Url.find("https://") != 0)
    {
        luaL_error(L, "Invalid protocol (expected 'http://' or 'https://')");
        return 0;
    }
    if (Url.find("https://httpbin.org/user-agent") != 0)
    {
        ROBLOX::Print(1, (std::string("URL called: ") + Url).c_str());
        Url = "https://postman-echo.com/get?foo=bar";
    }

    auto Method = H_GET;
    lua_getfield(L, 1, "Method");
    if (lua_isstring(L, -1))
    {
        std::string MethodStr = lua_tostring(L, -1);
        std::transform(MethodStr.begin(), MethodStr.end(), MethodStr.begin(), ::tolower);
        if (!RequestMethodMap.count(MethodStr))
            luaL_error(L, "Invalid request method: '%s'", MethodStr.c_str());
        Method = RequestMethodMap[MethodStr];
    }
    lua_pop(L, 1);

    cpr::Header Headers;
    lua_getfield(L, 1, "Headers");
    if (lua_istable(L, -1))
    {
        lua_pushnil(L);
        while (lua_next(L, -2))
        {
            if (!lua_isstring(L, -2) || !lua_isstring(L, -1))
                luaL_error(L, "'Headers' must have string keys/values");
            std::string Key = lua_tostring(L, -2);
            if (_stricmp(Key.c_str(), "Content-Length") == 0)
                luaL_error(L, "'Content-Length' cannot be overwritten");
            Headers[Key] = lua_tostring(L, -1);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    cpr::Cookies Cookies;
    lua_getfield(L, 1, "Cookies");
    if (lua_istable(L, -1))
    {
        lua_pushnil(L);
        while (lua_next(L, -2))
        {
            if (!lua_isstring(L, -2) || !lua_isstring(L, -1))
                luaL_error(L, "'Cookies' must have string keys/values");
            Cookies[lua_tostring(L, -2)] = lua_tostring(L, -1);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    std::string Body;
    lua_getfield(L, 1, "Body");
    if (lua_isstring(L, -1))
    {
        if (Method == H_GET || Method == H_HEAD)
            luaL_error(L, "'Body' is not allowed for GET/HEAD requests");
        Body = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    std::string GameId;
    lua_getglobal(L, "game");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "GameId");
        if (lua_isstring(L, -1))
            GameId = lua_tostring(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    std::string PlaceId;
    lua_getglobal(L, "game");
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "PlaceId");
        if (lua_isstring(L, -1))
            PlaceId = lua_tostring(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    std::optional<std::string> HWID;
    HW_PROFILE_INFO hwProfileInfo;
    GetCurrentHwProfile(&hwProfileInfo);

    nlohmann::json SessionIdJson;
    SessionIdJson["GameId"] = GameId;
    SessionIdJson["PlaceId"] = PlaceId;

    Headers.insert({ "User-Agent", "Cobalt" });
    Headers.insert({ "Roblox-Session-Id", SessionIdJson.dump() });
    Headers.insert({ "Cobalt-Fingerprint", HWID.value_or("Unknown") });

    return Yielder->YielderExecution(L, [=]() -> std::function<int(lua_State*)>
        {
            cpr::Response Response;
            try
            {
                switch (Method)
                {
                case H_GET: Response = cpr::Get(cpr::Url{ Url }, Cookies, Headers); break;
                case H_HEAD: Response = cpr::Head(cpr::Url{ Url }, Cookies, Headers); break;
                case H_POST: Response = cpr::Post(cpr::Url{ Url }, cpr::Body{ Body }, Cookies, Headers); break;
                case H_PUT: Response = cpr::Put(cpr::Url{ Url }, cpr::Body{ Body }, Cookies, Headers); break;
                case H_DELETE: Response = cpr::Delete(cpr::Url{ Url }, cpr::Body{ Body }, Cookies, Headers); break;
                case H_OPTIONS: Response = cpr::Options(cpr::Url{ Url }, cpr::Body{ Body }, Cookies, Headers); break;
                default: throw std::runtime_error("Invalid request method"); break;
                }
            }
            catch (const std::exception& ex)
            {
                std::string Err = std::string("Request failed: ") + ex.what();
                return [Err](lua_State* L) -> int
                    {
                        lua_pushstring(L, Err.c_str());
                        return 1;
                    };
            }

            return [Response](lua_State* L) -> int
                {
                    if (Response.error.code != cpr::ErrorCode::OK)
                    {
                        std::string Err = "Request failed: " + Response.error.message;
                        lua_pushstring(L, Err.c_str());
                        return 1;
                    }

                    lua_newtable(L);

                    lua_pushboolean(L, Response.status_code >= 200 && Response.status_code < 300);
                    lua_setfield(L, -2, "Success");

                    lua_pushinteger(L, Response.status_code);
                    lua_setfield(L, -2, "StatusCode");

                    std::string Phrase;
                    switch (Response.status_code)
                    {
                    case 100: Phrase = "Continue"; break;
                    case 101: Phrase = "Switching Protocols"; break;
                    case 102: Phrase = "Processing"; break;
                    case 103: Phrase = "Early Hints"; break;

                    case 200: Phrase = "OK"; break;
                    case 201: Phrase = "Created"; break;
                    case 202: Phrase = "Accepted"; break;
                    case 203: Phrase = "Non-Authoritative Information"; break;
                    case 204: Phrase = "No Content"; break;
                    case 205: Phrase = "Reset Content"; break;
                    case 206: Phrase = "Partial Content"; break;
                    case 207: Phrase = "Multi-Status"; break;
                    case 208: Phrase = "Already Reported"; break;
                    case 226: Phrase = "IM Used"; break;

                    case 300: Phrase = "Multiple Choices"; break;
                    case 301: Phrase = "Moved Permanently"; break;
                    case 302: Phrase = "Found"; break;
                    case 303: Phrase = "See Other"; break;
                    case 304: Phrase = "Not Modified"; break;
                    case 305: Phrase = "Use Proxy"; break;
                    case 307: Phrase = "Temporary Redirect"; break;
                    case 308: Phrase = "Permanent Redirect"; break;

                    case 400: Phrase = "Bad Request"; break;
                    case 401: Phrase = "Unauthorized"; break;
                    case 402: Phrase = "Payment Required"; break;
                    case 403: Phrase = "Forbidden"; break;
                    case 404: Phrase = "Not Found"; break;
                    case 405: Phrase = "Method Not Allowed"; break;
                    case 406: Phrase = "Not Acceptable"; break;
                    case 407: Phrase = "Proxy Authentication Required"; break;
                    case 408: Phrase = "Request Timeout"; break;
                    case 409: Phrase = "Conflict"; break;
                    case 410: Phrase = "Gone"; break;
                    case 411: Phrase = "Length Required"; break;
                    case 412: Phrase = "Precondition Failed"; break;
                    case 413: Phrase = "Payload Too Large"; break;
                    case 414: Phrase = "URI Too Long"; break;
                    case 415: Phrase = "Unsupported Media Type"; break;
                    case 416: Phrase = "Range Not Satisfiable"; break;
                    case 417: Phrase = "Expectation Failed"; break;
                    case 418: Phrase = "I'm a teapot"; break;
                    case 422: Phrase = "Unprocessable Entity"; break;
                    case 423: Phrase = "Locked"; break;
                    case 424: Phrase = "Failed Dependency"; break;
                    case 426: Phrase = "Upgrade Required"; break;
                    case 428: Phrase = "Precondition Required"; break;
                    case 429: Phrase = "Too Many Requests"; break;
                    case 431: Phrase = "Request Header Fields Too Large"; break;
                    case 451: Phrase = "Unavailable For Legal Reasons"; break;

                    case 500: Phrase = "Internal Server Error"; break;
                    case 501: Phrase = "Not Implemented"; break;
                    case 502: Phrase = "Bad Gateway"; break;
                    case 503: Phrase = "Service Unavailable"; break;
                    case 504: Phrase = "Gateway Time-out"; break;
                    case 505: Phrase = "HTTP Version Not Supported"; break;
                    case 506: Phrase = "Variant Also Negotiates"; break;
                    case 507: Phrase = "Insufficient Storage"; break;
                    case 508: Phrase = "Loop Detected"; break;
                    case 510: Phrase = "Not Extended"; break;
                    case 511: Phrase = "Network Authentication Required"; break;

                    default: Phrase = std::string(); break;
                    }
                    lua_pushstring(L, Phrase.c_str());
                    lua_setfield(L, -2, "StatusMessage");

                    lua_newtable(L);
                    for (auto& Header : Response.header)
                    {
                        lua_pushstring(L, Header.first.c_str());
                        lua_pushstring(L, Header.second.c_str());
                        lua_settable(L, -3);
                    }
                    lua_setfield(L, -2, "Headers");

                    lua_newtable(L);
                    for (auto& C : Response.cookies.map_)
                    {
                        lua_pushstring(L, C.first.c_str());
                        lua_pushstring(L, C.second.c_str());
                        lua_settable(L, -3);
                    }
                    lua_setfield(L, -2, "Cookies");

                    lua_pushlstring(L, Response.text.data(), Response.text.size());
                    lua_setfield(L, -2, "Body");

                    return 1;
                };
        });
}


namespace Hooking {
    namespace MetaHooks {
        lua_CFunction OriginalIndex;
        lua_CFunction OriginalNamecall;

        std::vector<const char*> UnsafeFunction = {
            "TestService.Run", "TestService", "Run",
            "OpenVideosFolder", "OpenScreenshotsFolder", "GetRobuxBalance", "PerformPurchase",
            "PromptBundlePurchase", "PromptNativePurchase", "PromptProductPurchase", "PromptPurchase",
            "PromptThirdPartyPurchase", "Publish", "GetMessageId", "OpenBrowserWindow", "RequestInternal",
            "ExecuteJavaScript", "ToggleRecording", "TakeScreenshot", "HttpRequestAsync", "GetLast",
            "SendCommand", "GetAsync", "GetAsyncFullUrl", "RequestAsync", "MakeRequest",
            "AddCoreScriptLocal", "SaveScriptProfilingData", "GetUserSubscriptionDetailsInternalAsync",
            "GetUserSubscriptionStatusAsync", "PerformBulkPurchase", "PerformCancelSubscription",
            "PerformPurchaseV2", "PerformSubscriptionPurchase", "PerformSubscriptionPurchaseV2",
            "PrepareCollectiblesPurchase", "PromptBulkPurchase", "PromptCancelSubscription",
            "PromptCollectiblesPurchase", "PromptGamePassPurchase", "PromptNativePurchaseWithLocalPlayer",
            "PromptPremiumPurchase", "PromptRobloxPurchase", "PromptSubscriptionPurchase",
            "ReportAbuse", "ReportAbuseV3", "ReturnToJavaScript", "OpenNativeOverlay",
            "OpenWeChatAuthWindow", "EmitHybridEvent", "OpenUrl", "PostAsync", "PostAsyncFullUrl",
            "RequestLimitedAsync", "Load", "CaptureScreenshot", "CreatePostAsync", "DeleteCapture",
            "DeleteCapturesAsync", "GetCaptureFilePathAsync", "SaveCaptureToExternalStorage",
            "SaveCapturesToExternalStorageAsync", "GetCaptureUploadDataAsync", "RetrieveCaptures",
            "SaveScreenshotCapture", "Call", "GetProtocolMethodRequestMessageId",
            "GetProtocolMethodResponseMessageId", "PublishProtocolMethodRequest",
            "PublishProtocolMethodResponse", "Subscribe", "SubscribeToProtocolMethodRequest",
            "SubscribeToProtocolMethodResponse", "GetDeviceIntegrityToken", "GetDeviceIntegrityTokenYield",
            "NoPromptCreateOutfit", "NoPromptDeleteOutfit", "NoPromptRenameOutfit", "NoPromptSaveAvatar",
            "NoPromptSaveAvatarThumbnailCustomization", "NoPromptSetFavorite", "NoPromptUpdateOutfit",
            "PerformCreateOutfitWithDescription", "PerformRenameOutfit", "PerformSaveAvatarWithDescription",
            "PerformSetFavorite", "PerformUpdateOutfit", "PromptCreateOutfit", "PromptDeleteOutfit",
            "PromptRenameOutfit", "PromptSaveAvatar", "PromptSetFavorite", "PromptUpdateOutfit"
        };

        int IndexHook(lua_State* L)
        {
            if (L->userdata->Capabilities == Capabilities)
            {
                std::string Key = lua_isstring(L, 2) ? lua_tostring(L, 2) : "";
                for (const char* Function : UnsafeFunction)
                {
                    if (Key == Function)
                    {
                        ROBLOX::Print(3, "Function '%s' has been disabled for security reasons", Function);
                        return 0;
                    }
                }

                if (L->userdata->Script.expired())
                {
                    if (Key == "HttpGet" || Key == "HttpGetAsync")
                    {
                        lua_pushcclosure(L, CHTTP::HTTPGet, nullptr, 0);
                        return 1;
                    }
                    else if (Key == "GetObjects")
                    {
                        lua_pushcclosure(L, CHTTP::GetObjects, nullptr, 0);
                        return 1;
                    }
                }
            }

            return OriginalIndex(L);
        };

        int NamecallHook(lua_State* L)
        {
            if (L->userdata->Capabilities == Capabilities)
            {
                std::string Key = L->namecall->data;
                for (const char* Function : UnsafeFunction)
                {
                    if (Key == Function)
                    {
                        ROBLOX::Print(3, "Function '%s' has been disabled for security reasons", Function);
                        return 0;
                    }
                }

                if (L->userdata->Script.expired())
                {
                    if (Key == "HttpGet" || Key == "HttpGetAsync")
                    {
                        return CHTTP::HTTPGet(L);
                    }
                    else if (Key == "GetObjects")
                    {
                        return CHTTP::GetObjects(L);
                    }
                }
            }

            return OriginalNamecall(L);
        };

        void Initialize(lua_State* L)
        {
            int OriginalTop = lua_gettop(L);

            lua_getglobal(L, "game");
            luaL_getmetafield(L, -1, "__index");
            if (lua_type(L, -1) == LUA_TFUNCTION || lua_type(L, -1) == LUA_TLIGHTUSERDATA)
            {
                Closure* IndexClosure = clvalue(luaA_toobject(L, -1));
                OriginalIndex = IndexClosure->c.f;
                IndexClosure->c.f = IndexHook;
            }
            lua_pop(L, 1);

            luaL_getmetafield(L, -1, "__namecall");
            if (lua_type(L, -1) == LUA_TFUNCTION || lua_type(L, -1) == LUA_TLIGHTUSERDATA)
            {
                Closure* NamecallClosure = clvalue(luaA_toobject(L, -1));
                OriginalNamecall = NamecallClosure->c.f;
                NamecallClosure->c.f = NamecallHook;
            }
            lua_pop(L, 1);

            lua_settop(L, OriginalTop);
        }

    }
}

void CHTTP::InitLib(lua_State* L) {
    declare__Global(L, "HTTPGet", CHTTP::HTTPGet);
    declare__Global(L, "request", request);
    declare__Global(L, "http_request", request);

    lua_newtable(L);
    declare__Tablemember(L, "request", request);
    lua_setglobal(L, "http");

    Hooking::MetaHooks::Initialize(L);
}