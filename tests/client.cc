#define BOOST_TEST_MODULE test_client
#include <stdlib.h>
#include <openssl/md5.h>
#include <iostream>
#include <memory>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include "libiqxmlrpc/libiqxmlrpc.h"
#include "libiqxmlrpc/http_client.h"
#include "libiqxmlrpc/http_errors.h"
#include "client_common.h"
#include "client_opts.h"

using namespace boost::unit_test;
using namespace iqxmlrpc;

// Global
Client_opts test_config;
Client_base* test_client = 0;

class ClientFixture {
public:
  ClientFixture()
  {
    int argc = boost::unit_test::framework::master_test_suite().argc;
    char** argv = boost::unit_test::framework::master_test_suite().argv;
    test_config.configure(argc, argv);
    test_client = test_config.create_instance();
  }
};

BOOST_GLOBAL_FIXTURE( ClientFixture );

BOOST_AUTO_TEST_CASE( introspection_test )
{
  BOOST_REQUIRE(test_client);
  Introspection_proxy introspect(test_client);
  Response retval( introspect() );
  BOOST_REQUIRE_MESSAGE(!retval.is_fault(), retval.fault_string());

  BOOST_MESSAGE("system.listMethods output:");
  const Value& v = retval.value();
  for (Array::const_iterator i = v.arr_begin(); i != v.arr_end(); ++i)
  {
    BOOST_MESSAGE("\t" + i->get_string());
  }
}

BOOST_AUTO_TEST_CASE( auth_test )
{
  BOOST_REQUIRE(test_client);

  BOOST_CHECKPOINT("Successful authorization");
  test_client->set_authinfo("goodman", "loooooooooooooooooongpaaaaaaaaaaaassssswwwwwwoooooord");
  Response retval( test_client->execute("echo_user", 0) );
  BOOST_CHECK( !retval.is_fault() );
  BOOST_CHECK_EQUAL( retval.value().get_string(), "goodman" );

  try {
    BOOST_CHECKPOINT("Unsuccessful authorization");
    test_client->set_authinfo("badman", "");
    retval = test_client->execute("echo_user", 0);

  } catch (const iqxmlrpc::http::Error_response& e) {
    BOOST_CHECK_EQUAL(e.response_header()->code(), 401);
    test_client->set_authinfo("", "");
    return;
  }

  test_client->set_authinfo("", "");
  BOOST_ERROR("'401 Unauthrozied' required");
}

BOOST_AUTO_TEST_CASE( echo_test )
{
  BOOST_REQUIRE(test_client);
  Echo_proxy echo(test_client);
  Response retval(echo("Hello"));
  BOOST_CHECK(retval.value().get_string() == "Hello");
}

BOOST_AUTO_TEST_CASE( error_method_test )
{
  BOOST_REQUIRE(test_client);
  Error_proxy err(test_client);
  Response retval(err(""));
  BOOST_CHECK(retval.is_fault());
  BOOST_CHECK(retval.fault_code() == 123 && retval.fault_string() == "My fault");
}

BOOST_AUTO_TEST_CASE( get_file_test )
{
  BOOST_REQUIRE(test_client);
  Get_file_proxy get_file(test_client);
  Response retval( get_file(1048576) ); // request 1Mb

  const Value& v = retval.value();
  const Binary_data& d = v["data"];
  const Binary_data& m = v["md5"];

  typedef const unsigned char md5char;
  typedef const char strchar;
  unsigned char md5[16];

  MD5(reinterpret_cast<md5char*>(d.get_data().data()),
    d.get_data().length(), md5);
  std::auto_ptr<Binary_data> gen_md5( Binary_data::from_data(
    reinterpret_cast<strchar*>(md5), sizeof(md5)) );

  BOOST_MESSAGE("Recieved MD5:   " + m.get_base64());
  BOOST_MESSAGE("Calculated MD5: " + gen_md5->get_base64());
  BOOST_WARN_MESSAGE(false, "TODO: Binary_data::operator ==(const Binary_data&)");
  BOOST_CHECK(gen_md5->get_base64() == m.get_base64());
}

BOOST_AUTO_TEST_CASE( stop_server )
{
  if (!test_config.stop_server())
    return;

  BOOST_REQUIRE(test_client);
  Stop_server_proxy stop(test_client);

  try {
    stop();
  } catch (const iqnet::network_error&) {}
}

// vim:ts=2:sw=2:et
