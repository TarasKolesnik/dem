# CanDrive Protocol

Окремий проєкт **лише для інтеграції**: контракт TCP + Protobuf між вашою програмою і CanDrive-сервісом (terrain / проїздність).

Сервер DEM живе в іншому репозиторії (`dem`). Тут — те, що потрібно **клієнту** (C++ / будь-яка мова).

```text
[ ваша програма ]  --TCP :9100 + Protobuf-->  [ CanDrive server ]
   lat, lon, azimuth, distance                   data["ok"] = "0"|"1"
```

## Що всередині

| Шлях | Призначення |
|------|-------------|
| `proto/can_drive.proto` | Єдине джерело контракту |
| `docs/PROTOCOL.md` | Framing, поля, ключі відповіді |
| `docs/INTEGRATION_CPP.md` | Як вбудувати в C++ |
| `cpp/` | Framing helper + приклад клієнта |
| `python/` | Референсний клієнт для перевірки |

## Швидкий старт (C++)

```bash
# залежності (Ubuntu)
sudo apt install -y build-essential cmake protobuf-compiler libprotobuf-dev

# згенерувати stubs
mkdir -p generated
protoc -I proto --cpp_out=generated proto/can_drive.proto

# зібрати приклад
cmake -S cpp -B build
cmake --build build

# сервер CanDrive має вже слухати 127.0.0.1:9100
./build/can_drive_example --host 127.0.0.1 --port 9100 \
  --lat 50.45 --lon 30.85 --azimuth 0 --distance 10
```

Очікуваний результат: у stdout з’явиться `ok=0` або `ok=1`.

## Швидкий старт (Python, перевірка контракту)

```bash
cd python
python3 -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt
python generate_pb.py
python client.py --once --lat 50.45 --lon 30.85 --azimuth 0 --distance 10
```

## Головне для інтегратора

1. Підключитись по TCP до `127.0.0.1:9100` (або хост/порт за домовленістю з ops).
2. Тримати **одне** з’єднання (не reconnect на кожен запит).
3. Слати `CanDriveRequest`, читати `CanDriveResponse`.
4. Framing: **`uint32` big-endian length + protobuf bytes**.
5. Рішення руху: **`response.data().at("ok") == "1"`**.

Деталі — у [`docs/PROTOCOL.md`](docs/PROTOCOL.md) і [`docs/INTEGRATION_CPP.md`](docs/INTEGRATION_CPP.md).

## Версіонування

- Зміни в `.proto` узгоджувати з власником CanDrive-сервера.
- Нові ключі в `data` — backward compatible.
- Зміна field numbers / видалення ключів — breaking change.
