// Copyright (C) 2015-2017 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
#include <vsomeip/vsomeip.hpp>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <string>

#if defined ANDROID || defined __ANDROID__
#include "android/log.h"
#define LOG_TAG "hello_world_service"
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
/*

    runtime -> In vsomeip, the runtime is a singleton class that provides access to the core features of the vsomeip framework. 
        It is used to initialize and manage the lifecycle of vsomeip applications, including the creation of application instances,
        communication setup, and inter-process coordination.

    * hello_world_service is a simple vsomeip service that responds to requests
    * and publishes periodic notifications.
    * It uses a separate thread to send notifications every 2 seconds.
    * The service can be stopped gracefully using a condition variable. 
    * 
    * This service listens for requests on a specific method and responds with a greeting message.
    * The service also offers an event that notifies subscribers every 2 seconds with a counter value.
    * The service is designed to be initialized, started, and stopped gracefully.
    * It uses a separate thread to handle the stop operation, allowing for clean shutdowns.



    vsomeip::application is the main interface to interact with the vsomeip stack from your app.
        It provides APIs to:
        Initialize the app (init())
        Offer or request services
        Send or receive messages
        Register event handlers, callbacks, etc.
        Start the event loop (start())

*/
class hello_world_service {
private:
    std::shared_ptr<vsomeip::runtime> rtm_;
    std::shared_ptr<vsomeip::application> app_;
    bool stop_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::thread stop_thread_;
    uint32_t counter_;
public:
    hello_world_service() :
        rtm_(vsomeip::runtime::get()),
        app_(rtm_->create_application()),
        stop_(false),
        stop_thread_(std::bind(&hello_world_service::stop, this)),
        counter_(0)
    {
    }

    ~hello_world_service() {
        stop_thread_.join();
    }

    bool init() {
        if (!app_->init()) {
            LOG_ERR("Couldn't initialize application");
            return false;
        }

        // Register message handler for request-response
        app_->register_message_handler(
            service_id, service_instance_id, service_method_id,
            std::bind(&hello_world_service::on_message_cbk, this, std::placeholders::_1));

        // Register state handler
        app_->register_state_handler(
            std::bind(&hello_world_service::on_state_cbk, this, std::placeholders::_1));

        // Offer the event with corrected parameters
        app_->offer_event(
            service_id,
            service_instance_id,
            event_id,
            {eventgroup_id},
            vsomeip::event_type_e::ET_EVENT,
            std::chrono::milliseconds(2000), // Notify every 2 seconds
            false,  // change_resets_cycle
            true,   // update_on_change
            nullptr, // epsilon_change_func
            vsomeip::reliability_type_e::RT_RELIABLE // TCP-based
        );

        return true;
    }

    void start() {
        // Start a thread to send periodic notifications
        std::thread notify_thread([this]() {
            while (!stop_) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                send_notification();
            }
        });
        app_->start();
        notify_thread.join();
    }

    void stop() {
        std::unique_lock<std::mutex> its_lock(mutex_);
        while (!stop_) {
            condition_.wait(its_lock);
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
        app_->stop_offer_service(service_id, service_instance_id);
        app_->stop_offer_event(service_id, service_instance_id, event_id);
        app_->unregister_state_handler();
        app_->unregister_message_handler(service_id, service_instance_id, service_method_id);
        app_->stop();
    }

    void terminate() {
        std::lock_guard<std::mutex> its_lock(mutex_);
        stop_ = true;
        condition_.notify_one();
    }

    void on_state_cbk(vsomeip::state_type_e _state) {
        if (_state == vsomeip::state_type_e::ST_REGISTERED) {
            app_->offer_service(service_id, service_instance_id);
            LOG_INF("Service registered, offering service and event group");
        }
    }

    void on_message_cbk(const std::shared_ptr<vsomeip::message> &_request) {
        std::shared_ptr<vsomeip::message> resp = rtm_->create_response(_request);
        std::string str("Hello ");
        str.append(reinterpret_cast<const char*>(_request->get_payload()->get_data()), 
                  0, _request->get_payload()->get_length());
        std::shared_ptr<vsomeip::payload> resp_pl = rtm_->create_payload();
        std::vector<vsomeip::byte_t> pl_data(str.begin(), str.end());
        resp_pl->set_data(pl_data);
        resp->set_payload(resp_pl);
        app_->send(resp);
        LOG_INF("Sent response: %s", str.c_str());
    }

    void send_notification() {
        counter_++;
        std::string counter_str = "Counter: " + std::to_string(counter_);
        std::shared_ptr<vsomeip::payload> payload = rtm_->create_payload();
        std::vector<vsomeip::byte_t> payload_data(counter_str.begin(), counter_str.end());
        payload->set_data(payload_data);
        app_->notify(service_id, service_instance_id, event_id, payload);
        LOG_INF("Published event with payload: %s", counter_str.c_str());
    }


};

