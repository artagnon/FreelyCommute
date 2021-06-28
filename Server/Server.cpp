#include "oatpp/network/Server.hpp"

#include "Server/Controller.hpp"
#include "Server/Component.hpp"
#include "Server/DTO.hpp"

using namespace oatpp::web::server;
using namespace oatpp::network;
using namespace oatpp::base;

void run()
{
  auto ObjectMapper = JSONObjectMapper::createShared();
  auto R = HttpRouter::createShared();
  auto C = std::make_shared<Controller>();
  C->addEndpointsToRouter(R);
  OATPP_COMPONENT(std::shared_ptr<ConnectionHandler>, Handler);
  OATPP_COMPONENT(std::shared_ptr<ServerConnectionProvider>, Provider);
  Server S(Provider, Handler);
  OATPP_LOGI("FreelyCommute", "Server running on port %s", Provider->getProperty("port").getData());

  S.run();
}

int main()
{
  Environment::init();
  run();
  Environment::destroy();
  return EXIT_SUCCESS;
}
