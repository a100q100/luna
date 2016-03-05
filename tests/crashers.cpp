//
// luna
//
// Copyright © 2016 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

TEST(crashers, 1_logging)
{
    luna::set_logger([](luna::log_level level, const std::string &mesg)
                         {
                             std::cout << to_string(level) << ": " << mesg << std::endl;
                         });

    luna::server server{luna::server::port{8080}};

    server.handle_request(luna::request_method::GET, "/hello", [](auto matches, auto params) -> luna::response
        {
            return {"hello"};
        });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/hello"});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);

    luna::reset_logger();
}

TEST(crashers, 2_query_params_with_no_values)
{
    luna::server server{luna::server::port{8080}};

    server.handle_request(luna::request_method::GET, "/hello", [](auto matches, auto params) -> luna::response
        {
            EXPECT_EQ(2, params.size());
            EXPECT_EQ(1, params.count("key1"));
            EXPECT_EQ("", params.at("key1"));
            EXPECT_EQ(1, params.count("key2"));
            EXPECT_EQ("foo", params.at("key2"));
            return {"hello"};
        });

    auto res = cpr::Get(cpr::Url{"http://localhost:8080/hello"}, cpr::Parameters{{"key1", ""}, {"key2", "foo"}});
    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(crashers, 2_query_params_with_no_values_post)
{
    luna::server server{luna::server::port{8080}};

    server.handle_request(luna::request_method::POST, "/hello", [](auto matches, auto params) -> luna::response
        {
            EXPECT_EQ(2, params.size());
            EXPECT_EQ(1, params.count("key1"));
            EXPECT_EQ("", params.at("key1"));
            EXPECT_EQ(1, params.count("key2"));
            EXPECT_EQ("foo", params.at("key2"));
            return {"hello"};
        });

    auto res = cpr::Post(cpr::Url{"http://localhost:8080/hello"}, cpr::Payload{{"key1", ""}, {"key2", "foo"}});
    ASSERT_EQ(201, res.status_code);
    ASSERT_EQ("hello", res.text);
}