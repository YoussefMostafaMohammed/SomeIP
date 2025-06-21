// Copyright (C) 2015-2017 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#include <vsomeip/vsomeip.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>

#if defined ANDROID || defined __ANDROID__
#include "android/log.h"
#define LOG_TAG "hello_world_client"
#define LOG_INF(...) fprintf(stdout, __VA_ARGS__), fprintf(stdout, "\n"), (void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, ##__VA_ARGS__)
#define LOG_ERR(...) fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n"), (void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, ##__VA_ARGS__)
#else
#include <cstdio>
#define LOG_INF(...) fprintf(stdout, __VA_ARGS__), fprintf(stdout, "\n")
#define LOG_ERR(...) fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n")
#endif

static vsomeip::service_t service_id = 0x1111;
static vsomeip::instance_t service_instance_id = 0x2222;
static vsomeip::method_t service_method_id = 0x3333;
static vsomeip::event_t event_id = 0x4444;
static vsomeip::eventgroup_t eventgroup_id = 0x5555;

class hello_world_client {
public:
    hello_world_client() :
        rtm_(vsomeip::runtime::get()),
        app_(rtm_->create_application()),
        stop_(false),
        stop_thread_(std::bind(&hello_world_client::stop, this))
    {
    }

    ~hello_world_client() {
        stop_thread_.join();
    }

    bool init() {
        if (!app_->init()) {
            LOG_ERR("Couldn't initialize application");
            return false;
        }

        // Register state handler
        app_->register_state_handler(
            std::bind(&hello_world_client::on_state_cbk, this, std::placeholders::_1));

        // Register message handler for responses
        app_->register_message_handler(
            service_id, service_instance_id, service_method_id,
            std::bind(&hello_world_client::on_message_cbk, this, std::placeholders::_1));

        // Register message handler for events
        app_->register_message_handler(
            service_id, service_instance_id, event_id,
            std::bind(&hello_world_client::on_event_cbk, this, std::placeholders::_1));

        // Register availability handler
        app_->register_availability_handler(
            service_id, service_instance_id,
            std::bind(&hello_world_client::on_availability_cbk, this,
                      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

        return true;
    }

    void start() {
        app_->start();
    }

    void stop() {
        std::unique_lock<std::mutex> its_lock(mutex_);
        while (!stop_) {
            condition_.wait(its_lock);
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
        app_->unregister_state_handler();
        app_->unregister_message_handler(service_id, service_instance_id, service_method_id);
        app_->unregister_message_handler(service_id, service_instance_id, event_id);
        app_->release_service(service_id, service_instance_id);
        app_->release_event(service_id, service_instance_id, event_id);
        app_->unsubscribe(service_id, service_instance_id, eventgroup_id);
        app_->stop();
    }

    void terminate() {
        std::lock_guard<std::mutex> its_lock(mutex_);
        stop_ = true;
        condition_.notify_one();
    }

    void on_state_cbk(vsomeip::state_type_e _state) {
        if (_state == vsomeip::state_type_e::ST_REGISTERED) {
            app_->request_service(service_id, service_instance_id);
            LOG_INF("Client registered, requesting service");
        }
    }

    void on_availability_cbk(vsomeip::service_t _service, vsomeip::instance_t _instance, bool _is_available) {
        if (service_id == _service && service_instance_id == _instance && _is_available) {
            LOG_INF("Service available, sending request and subscribing to event group");

            // Send request
            std::shared_ptr<vsomeip::message> rq = rtm_->create_request();
            rq->set_service(service_id);
            rq->set_instance(service_instance_id);
            rq->set_method(service_method_id);
            std::shared_ptr<vsomeip::payload> pl = rtm_->create_payload();
            std::string str("World");
            std::vector<vsomeip::byte_t> pl_data(str.begin(), str.end());
            pl->set_data(pl_data);
            rq->set_payload(pl);
            app_->send(rq);
            LOG_INF("Sent request: %s", str.c_str());

            // Subscribe to event group
            app_->request_event(service_id, service_instance_id, event_id, {eventgroup_id}, vsomeip::event_type_e::ET_EVENT);
            app_->subscribe(service_id, service_instance_id, eventgroup_id);
        }      
    }

    void on_message_cbk(const std::shared_ptr<vsomeip::message> &_response) {
        if (service_id == _response->get_service() &&
            service_instance_id == _response->get_instance() &&
            vsomeip::message_type_e::MT_RESPONSE == _response->get_message_type() &&
            vsomeip::return_code_e::E_OK == _response->get_return_code()) {
            std::shared_ptr<vsomeip::payload> pl = _response->get_payload();
            std::string resp(reinterpret_cast<const char*>(pl->get_data()), pl->get_length());
            LOG_INF("Received response: %s", resp.c_str());
        }
    }

    void on_event_cbk(const std::shared_ptr<vsomeip::message> &_event) {
        if (service_id == _event->get_service() &&
            service_instance_id == _event->get_instance() &&
            vsomeip::message_type_e::MT_NOTIFICATION == _event->get_message_type()) {
            std::shared_ptr<vsomeip::payload> pl = _event->get_payload();
            std::string event_data(reinterpret_cast<const char*>(pl->get_data()), pl->get_length());
            LOG_INF("Received event: %s", event_data.c_str());
        }
    }

private:
    std::shared_ptr<vsomeip::runtime> rtm_;
    std::shared_ptr<vsomeip::application> app_;
    bool stop_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::thread stop_thread_;
};

