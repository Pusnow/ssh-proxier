#pragma once
#include <memory>
#include <optional>
#include <string>
#include <vector>

class App {
   public:
    static void init_instance(const char *port,
                              const std::vector<const char *> &&hosts,
                              const std::vector<const char *> &&bypasses);
    static App &get_instance() { return *inner; }
    void menu_handler(size_t id);

    void setup_bypass(const char *interface);

    int loop();
    void check_connection();
    void init_objcxx();

   private:
    void init_interfaces();
    void update_interfaces(bool enable);
    void stop_sshd();
    bool start_sshd(size_t id);
    App(const char *port, const std::vector<const char *> &&hosts,
        const std::vector<const char *> &&bypasses);
    static std::unique_ptr<App> inner;
    std::optional<size_t> connected;
    int update();
    const char *port;
    size_t current;
    const std::vector<const char *> hosts;
    std::vector<std::string> interfaces;
    const std::vector<const char *> bypasses;
};