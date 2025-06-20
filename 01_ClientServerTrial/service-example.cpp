#include <iomanip>
#include <iostream>
#include <sstream>
#include <vsomeip/vsomeip.hpp>

#define SAMPLE_SERVICE_ID 0x1234
#define SAMPLE_INSTANCE_ID 0x5678
#define SAMPLE_METHOD_ID 0x0421

std::shared_ptr<vsomeip::application> app;

int main() {

  app = vsomeip::runtime::get()->create_application("World");
  app->init();
  app->offer_service(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID);
  app->start();
}
