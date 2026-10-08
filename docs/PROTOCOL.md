# Протокол CanDrive

## 1. Транспорт

| Параметр | Значення |
|----------|----------|
| Протокол | TCP IPv4 |
| Default | `127.0.0.1:9100` |
| Режим | request → response на тому ж сокеті |
| TLS / auth | немає |
| Клієнтів | 1 (типовий deployment) |

Рекомендовано: **persistent connection** на час роботи програми.

---

## 2. Framing

Кожне Protobuf-повідомлення:

```text
┌────────────────────┬──────────────────────────────┐
│ uint32 BE length   │ protobuf payload (N bytes)   │
│ 4 bytes            │ N = length                   │
└────────────────────┴──────────────────────────────┘
```

- `length` — big-endian (`htonl` / `ntohl`)
- `length` = розмір **лише** serialized protobuf
- Читати рівно `4 + N` байт (враховувати partial `recv`)

### Write

```text
body = Serialize(message)
hdr  = htonl(body.size())
send_all(hdr || body)
```

### Read

```text
hdr  = read_exact(4)
N    = ntohl(hdr)
body = read_exact(N)
Parse(message, body)
```

Максимальний frame на сервері: 4 MiB (на практиці ≪ 1 KB).

---

## 3. Повідомлення

Файл: `proto/can_drive.proto`, package `dem`.

### Request — `CanDriveRequest`

| Поле | Тип | Опис |
|------|-----|------|
| `lat` | double | WGS84, градуси |
| `lon` | double | WGS84, градуси |
| `azimuth_deg` | double | Compass: `0` = North, за годинниковою |
| `distance_m` | double | Дистанція вперед, м (`> 0`, типово `10`) |

`(lat, lon)` — **центр** транспортного засобу на старті.

### Response — `CanDriveResponse`

```protobuf
map<string, string> data = 1;
```

Усі values — **рядки**. Числа парсити на клієнті (`std::stod` тощо).

| Key | Приклад | Опис |
|-----|---------|------|
| `ok` | `"1"` / `"0"` | **`"1"`** = можна їхати, **`"0"`** = ні / помилка |
| `grade_pct` | `"3.240000"` | Ефективний похил, % |
| `delta_h_m` | `"-0.320000"` | `z_end - z_start`, м |
| `z_start_m` | `"133.550000"` | Висота в центрі на старті, м |
| `z_end_m` | `"133.230000"` | Висота в центрі в кінці, м |
| `error` | `""` | Порожньо при успіху; інакше текст помилки |

**Обов’язково для логіки руху:** тільки `data["ok"]`.

Якщо `error` не порожній — розрахунок не вдався, `ok` буде `"0"`.

---

## 4. Семантика на сервері (довідково)

Сервер враховує габарити ТЗ (фіксовано на сервері):

- ширина **1.5 м**
- довжина **2.0 м**

Перевіряється коридор під корпусом уздовж азимуту на `distance_m`.  
Поріг похилу за замовчуванням: **15%** (`ok="0"` якщо вище).

Latency:

- перший запит у новій зоні — секунди (завантаження DEM);
- повтор у тій самій зоні / зміна лише азимуту — мілісекунди (кеш).

Рекомендовані таймаути клієнта: cold **30–60 с**, hot **1–2 с**.

---

## 5. Послідовність байтів

```text
Client → Server:  [4B BE len][CanDriveRequest]
Server → Client:  [4B BE len][CanDriveResponse]
```

Не надсилати наступний request, поки не отримано response (або не зроблено reconnect).
