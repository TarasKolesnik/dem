#include "can_drive_client.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void usage(const char* argv0) {
  std::cerr
      << "Usage: " << argv0
      << " [--host HOST] [--port PORT] [--lat LAT] [--lon LON]\n"
      << "       [--azimuth DEG] [--distance M]\n";
}

}  // namespace

int main(int argc, char** argv) {
  std::string host = "127.0.0.1";
  int port = 9100;
  double lat = 50.45;
  double lon = 30.85;
  double azimuth = 0.0;
  double distance = 10.0;

  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    auto need = [&](const char* name) -> std::string {
      if (i + 1 >= argc) {
        std::cerr << "missing value for " << name << "\n";
        std::exit(2);
      }
      return argv[++i];
    };
    if (a == "--host") {
      host = need("--host");
    } else if (a == "--port") {
      port = std::stoi(need("--port"));
    } else if (a == "--lat") {
      lat = std::stod(need("--lat"));
    } else if (a == "--lon") {
      lon = std::stod(need("--lon"));
    } else if (a == "--azimuth") {
      azimuth = std::stod(need("--azimuth"));
    } else if (a == "--distance") {
      distance = std::stod(need("--distance"));
    } else if (a == "--help" || a == "-h") {
      usage(argv[0]);
      return 0;
    } else {
      std::cerr << "unknown arg: " << a << "\n";
      usage(argv[0]);
      return 2;
    }
  }

  dem::CanDriveClient client;
  if (!client.connect(host, port)) {
    std::cerr << "connect failed: " << host << ":" << port << "\n";
    return 1;
  }

  dem::CanDriveRequest req;
  req.set_lat(lat);
  req.set_lon(lon);
  req.set_azimuth_deg(azimuth);
  req.set_distance_m(distance);

  dem::CanDriveResponse resp;
  if (!client.query(req, resp)) {
    std::cerr << "query failed\n";
    return 1;
  }

  const auto& data = resp.data();
  for (const auto& kv : data) {
    std::cout << kv.first << "=" << kv.second << "\n";
  }

  const auto it = data.find("ok");
  if (it == data.end()) {
    std::cerr << "missing key ok\n";
    return 1;
  }
  return it->second == "1" ? 0 : 3;
}
