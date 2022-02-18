#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

static void trim_string(std::string &str) {
    str.erase(0, str.find_first_not_of(" "));
    str.erase(str.find_last_not_of(" ") + 1);
}

class App {
   public:
    static void init_instance(int port, const std::vector<const char *> &&hosts,
                              const std::vector<const char *> &&bypasses);
    static App &get_instance() { return *inner; }
    void menu_handler(size_t id);

    void setup_bypass(const char *interface);

    int loop();
    void check_connection();
    void init_objcxx();

   private:
    void update_interfaces(bool enable);
    void update_interface(bool enable, bool enabled, const char *interface,
                          const char *server, int port);
    void stop_sshd();
    bool start_sshd(size_t id);
    App(int port, const std::vector<const char *> &&hosts,
        const std::vector<const char *> &&bypasses);
    static std::unique_ptr<App> inner;
    std::optional<size_t> connected;
    int update();
    const int port;
    size_t current;
    const std::vector<const char *> hosts;
    const std::vector<const char *> bypasses;
};