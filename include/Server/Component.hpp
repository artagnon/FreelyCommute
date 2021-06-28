#pragma once

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/core/macro/component.hpp"

using JSONObjectMapper = oatpp::parser::json::mapping::ObjectMapper;
using DataObjectMapper = oatpp::data::mapping::ObjectMapper;
using namespace oatpp::web::server;
using namespace oatpp::network;

class Component
{
public:
  /**
   *  Create ConnectionProvider component which listens on the port
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<ServerConnectionProvider>, serverConnectionProvider)
  ([] {
    return tcp::server::ConnectionProvider::createShared({"localhost", 8000, Address::IP_4});
  }());

  /**
   *  Create Router component
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<HttpRouter>, httpRouter)
  ([] {
    return HttpRouter::createShared();
  }());

  /**
   *  Create ConnectionHandler component which uses Router component to route requests
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<ConnectionHandler>, serverConnectionHandler)
  ([] {
    OATPP_COMPONENT(std::shared_ptr<HttpRouter>, router);
    return HttpConnectionHandler::createShared(router);
  }());

  /**
   *  Create ObjectMapper component to serialize/deserialize DTOs in Contoller's API
   */
  OATPP_CREATE_COMPONENT(std::shared_ptr<DataObjectMapper>, apiObjectMapper)
  ([] {
    return JSONObjectMapper::createShared();
  }());
};
