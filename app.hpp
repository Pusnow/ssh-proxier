#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

class App {
   public:
    static void init_instance(const char *port);
    static App &get_instance() { return *inner; }
    void menu_handler(size_t id);

    void add_host(const char *host) { hosts.push_back(host); }
    void add_bypass(const char *bypass) { bypasses.push_back(bypass); }
    void setup_bypass(const char *interface);

    int loop();
    void check_connection_and_reconnect();
    bool check_connection();
    void init_tray();

   private:
    void init_interfaces();
    void update_interfaces(bool enable);
    void stop_sshd();
    bool start_sshd(size_t id);
    App(const char *port);
    static std::unique_ptr<App> inner;
    std::optional<size_t> connected;
    int update();
    const char *port;
    size_t current;
    std::vector<const char *> hosts;
    std::vector<std::string> interfaces;
    std::vector<const char *> bypasses;
};