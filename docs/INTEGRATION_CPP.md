# Інтеграція в C++ програму

## 1. Залежності

```bash
sudo apt install -y build-essential cmake \
  protobuf-compiler libprotobuf-dev
```

Версія `protoc` і `libprotobuf` мають бути сумісні (з одного пакета).

## 2. Підключити proto

Скопіюйте або підключіть як git submodule цей репозиторій.  
Згенеруйте stubs у свій build:

```bash
protoc -I path/to/can_drive_protocol/proto \
  --cpp_out=path/to/your/generated \
  path/to/can_drive_protocol/proto/can_drive.proto
```

Отримаєте:

- `can_drive.pb.h`
- `can_drive.pb.cc`

Додайте `.cc` у свій target і лінкуйте `-lprotobuf`.

Можна використати готові хелпери з цього репо:

- `cpp/include/can_drive_framing.hpp`
- `cpp/include/can_drive_client.hpp`

або написати свій framing за [`PROTOCOL.md`](PROTOCOL.md).

## 3. Мінімальний код

```cpp
#include "can_drive.pb.h"
#include "can_drive_client.hpp"   // або свій TCP + framing

dem::CanDriveClient client;
if (!client.connect("127.0.0.1", 9100)) {
  // handle error
}

dem::CanDriveRequest req;
req.set_lat(50.45);
req.set_lon(30.85);
req.set_azimuth_deg(0.0);
req.set_distance_m(10.0);

dem::CanDriveResponse resp;
if (!client.query(req, resp)) {
  // I/O or parse error — reconnect
}

const auto& data = resp.data();
auto err = data.find("error");
if (err != data.end() && !err->second.empty()) {
  // server-side failure
}

bool can_drive = (data.at("ok") == "1");
```

## 4. Життєвий цикл сокета

1. `connect` при старті модуля / app  
2. Багато `query` на тому ж сокеті  
3. При помилці читання/запису — `close` + `connect` знову  
4. `close` при зупинці  

Не відкривайте нове TCP-з’єднання на кожен запит.

## 5. Збірка прикладу з цього репо

```bash
cmake -S cpp -B build
cmake --build build
./build/can_drive_example --help
```

Приклад сам генерує protobuf у `build/generated` під час cmake (якщо знайдено `protoc`).

## 6. Перевірка сумісності з сервером

1. Підняти CanDrive server (репозиторій `dem`):  
   `python -u tcp_server.py --host 127.0.0.1 --port 9100`
2. Запустити приклад з цього репо або свій клієнт з тими самими `lat/lon/azimuth/distance`.
3. Порівняти `ok` з референсним Python-клієнтом:

```bash
cd python && source .venv/bin/activate
python client.py --once --lat 50.45 --lon 30.85 --azimuth 0 --distance 10
```

## 7. Чеклист

- [ ] Взято актуальний `proto/can_drive.proto`
- [ ] Згенеровано `*.pb.h / *.pb.cc`, лінк `protobuf`
- [ ] Framing: uint32 BE length + payload
- [ ] Persistent TCP на `127.0.0.1:9100`
- [ ] Азимут: `0° = North`, clockwise
- [ ] Рішення: `data.at("ok") == "1"`
- [ ] Таймаут cold ≥ 30 с
- [ ] Smoke збігається з `python/client.py`
