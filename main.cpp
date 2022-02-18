#include <array>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <sstream>

#include "app.hpp"

constexpr size_t EXEC_BUFFER_SIZE = 128;
constexpr std::string_view LOCAL_HOST("127.0.0.1");

static std::optional<std::string> exec(const char *cmd) {
    std::array<char, EXEC_BUFFER_SIZE> buffer;
    std::string result;

    FILE *pipe = popen(cmd, "r");

    if (!pipe) {
        pclose(pipe);
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    int status = pclose(pipe);

    int err = WEXITSTATUS(status);

    if (err) return std::nullopt;

    return result;
}

template <class... Types>
static std::string do_snprintf(const char *msg, Types... args) {
    size_t needed = snprintf(NULL, 0, msg, args...);
    char str[needed + 1];
    snprintf(str, needed + 1, msg, args...);
    return std::string(str);
}

std::unique_ptr<App> App::inner;
void App::init_instance(int port, const std::vector<const char *> &&hosts,
                        const std::vector<const char *> &&bypasses) {
    App::inner.reset(new App(port, std::move(hosts), std::move(bypasses)));
}

void App::menu_handler(size_t menu_id) {
    size_t current = connected.value_or(-1);

    if (current == menu_id) {
        connected = std::nullopt;
        update_interfaces(false);
        stop_sshd();
    } else {
        stop_sshd();
        bool ok = start_sshd(menu_id);
        if (ok) {
            connected = menu_id;
            update_interfaces(true);
        } else {
            connected = std::nullopt;
            update_interfaces(false);
            stop_sshd();
        }
    }
}

void App::setup_bypass(const char *interface) {
    const size_t sz = bypasses.size();
    if (!sz) return;

    std::string str(bypasses[0]);
    for (size_t i = 1; i < sz; ++i) {
        str += " ";
        str += bypasses[i];
    }
    exec(do_snprintf("networksetup -setproxybypassdomains %s %s", interface,
                     str.c_str())
             .c_str());
}

void App::update_interface(bool enable, bool enabled, const char *interface,
                           const char *server, int port) {
    if (enable) {
        if (!enabled || LOCAL_HOST != server || port != this->port) {
            exec(do_snprintf("networksetup -setsocksfirewallproxy "
                             "\"%s\" 127.0.0.1 %d off",
                             interface, port)
                     .c_str());
            setup_bypass(interface);
        }

    } else {
        // disable
        if (enabled) {
            exec(do_snprintf("networksetup -setsocksfirewallproxystate "
                             "\"%s\" off",
                             interface)
                     .c_str());
        }
    }
}

void App::stop_sshd() {
    exec(do_snprintf("pkill -f \"ssh -fN -D%d\"", port).c_str());
}

bool App::start_sshd(size_t id) {
    auto &host = hosts[id];
    auto result = exec(do_snprintf("ssh -fN -D%d %s", port, host).c_str());
    return result.has_value();
}

void App::check_connection() {
    if (!connected.has_value()) return;

    auto result = exec(
        do_snprintf("nc -z -x 127.0.0.1:%d 127.0.0.1 22 >/dev/null 2>&1", port)
            .c_str());

    if (result.has_value()) return;

    // reconnect
    stop_sshd();
    bool ok = start_sshd(*connected);
    if (ok) {
        update_interfaces(true);
    } else {
        connected = std::nullopt;
        update_interfaces(false);
        stop_sshd();
    }
}

App::App(int port, const std::vector<const char *> &&hosts,
         const std::vector<const char *> &&bypasses)
    : connected(std::nullopt),
      port(port),
      current(-1),
      hosts(hosts),
      bypasses(bypasses) {
    init_objcxx();
    stop_sshd();
    update_interfaces(false);
}

static constexpr std::string_view separator_str = "--";
int main(const int argc, const char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s [port_number] [host_name] ...", argv[0]);
        return 1;
    }

    int last_host = argc;

    for (int i = 0; i < argc; ++i) {
        if (separator_str == argv[i]) {
            last_host = i;
            break;
        }
    }

    std::vector<const char *> hosts;
    std::vector<const char *> bypasses;

    for (size_t i = 2; i < last_host; ++i) {
        hosts.push_back(argv[i]);
    }
    bypasses.push_back("*.local");
    bypasses.push_back("169.254/16");
    for (size_t i = last_host + 1; i < argc; ++i) {
        bypasses.push_back(argv[i]);
    }

    App::init_instance(std::atoi(argv[1]), std::move(hosts),
                       std::move(bypasses));
    auto &app = App::get_instance();

    auto last_time = std::chrono::system_clock::now();

    while (app.loop() == 0) {
        auto this_time = std::chrono::system_clock::now();

        auto diff = this_time - last_time;

        if (diff > std::chrono::minutes(1)) {
            last_time = this_time;
            app.check_connection();
        }
    }

    return 0;
}
