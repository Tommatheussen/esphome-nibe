#pragma once

#include <set>
#include <queue>
#include <vector>
#include <cstddef>

#include "esphome.h"
#include "esphome/core/component.h"

#include "NibeGw.h"
#include <HardwareSerial.h>
#include <WiFiUdp.h>

using namespace esphome;


typedef std::tuple<byte, byte>  request_key_type;
typedef std::vector<byte>       request_data_type;
typedef std::tuple<IPAddress, int> target_type;

class NibeGwComponent: public Component {
    float get_setup_priority() const override { return setup_priority::BEFORE_CONNECTION; }
    const char* TAG = "nibegw";
    const int requests_queue_max = 3;
    int udp_read_port_  = 9999;
    int udp_write_port_ = 10000;
    std::set<IPAddress> udp_source_ip_;
    bool is_connected_ = false;

    std::vector<target_type> udp_targets_;
    std::map<request_key_type, std::queue<request_data_type>> requests_; 
    std::map<request_key_type, request_data_type>             requests_const_; 

    NibeGw* gw_;

    WiFiUDP udp_read_;
    WiFiUDP udp_write_;

    void callback_msg_received(const byte* const data, int len);
    int callback_msg_token_received(eTokenType token, byte* data);
    void callback_debug(byte verbose, char* data);

    void token_request_cache(WiFiUDP& udp, byte address, byte token);

    public:

        void set_read_port(int port) { udp_read_port_ = port; };
        void set_write_port(int port) { udp_write_port_ = port; };

        void add_target(std::string ip, int port)
        {
            IPAddress address;
            address.fromString(ip.c_str());
            auto target = target_type(address, port);
            udp_targets_.push_back(target);
        }

        void add_source_ip(std::string ip){
            IPAddress address;
            address.fromString(ip.c_str());
            udp_source_ip_.insert(address);
        };

        void set_const_request(int address, int token, request_data_type request)
        {
            requests_const_[request_key_type(address, token)] = std::move(request);
        }

        void add_queued_request(int address, int token, request_data_type request)
        {
            auto& queue = requests_[request_key_type(address, token)];
            if (queue.size() >= requests_queue_max) {
                queue.pop();
            }
            queue.push(std::move(request));
        }

        NibeGw& gw() { return *gw_; }

        NibeGwComponent(int uart_no, int dir_pin, int rx_pin, int tx_pin);

        void setup() override;
        void dump_config() override;
        void loop() override;
};
