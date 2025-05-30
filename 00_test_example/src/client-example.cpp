#include <iomanip>
#include <iostream>
#include <vsomeip/primitive_types.hpp>
#include <vsomeip/vsomeip.hpp>

#define SAMPLE_SERVICE_ID 0x1234
#define SAMPLE_INSTANCE_ID 0x5678

void on_availability(vsomeip::service_t _service, vsomeip::instance_t _instance,
                     bool _is_available) {

  std::cout << "Service [" << std::setw(4) << std::setfill('0') << std::hex
            << _service << "." << _instance << "] is "
            << (_is_available ? "available." : "NOT available.") << std::endl;
}

std::shared_ptr<vsomeip::application> app;
int main() {

  app = vsomeip::runtime::get()->create_application("Hello");
  app->init();
  app->register_availability_handler(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID,
                                     on_availability);

  app->request_service(SAMPLE_SERVICE_ID, SAMPLE_INSTANCE_ID);
  app->start();
}
