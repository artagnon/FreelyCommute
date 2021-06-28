#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "DTO.hpp"

using namespace oatpp::web::server::api;

#include OATPP_CODEGEN_BEGIN(ApiController)

class Controller : public ApiController
{
public:
  Controller(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, ObjectMapper))
      : ApiController(ObjectMapper)
  {
  }

public:
  ENDPOINT("GET", "/hello", root)
  {
    auto dto = MessageDto::createShared();
    dto->statusCode = 200;
    dto->message = "Hello World!";
    return createDtoResponse(Status::CODE_200, dto);
  }
};

#include OATPP_CODEGEN_END(ApiController)
