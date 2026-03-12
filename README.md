# Stock Market Peak Finder

A real-time stock market visualisation tool backed by a C++ HTTP server. Range-maximum queries over a live price feed are answered in O(log N) time using a Segment Tree data structure.

---

## Overview

The application simulates a 60-minute market session where prices update continuously. A browser-based chart allows the user to drag-select any time range and instantly retrieve the peak price within that range. All query logic runs on the server — the frontend makes REST API calls and renders the result.

**Key characteristics:**

- Segment Tree built and maintained entirely in C++ (no JavaScript computation for queries)
- Live market simulation via randomised price ticks at 250 ms intervals
- Query results logged persistently to `log.txt` with timestamps
- Single-page frontend served directly by the C++ backend

---

## Project Structure

```
.
├── main.cpp        # C++ backend — HTTP server, Segment Tree, REST API
├── index.html      # Single-page frontend — chart rendering and API calls
└── log.txt         # Auto-generated query log (created on first query)
```

---

## Dependencies

### C++ Backend

The backend depends on the [Crow](https://github.com/CrowCpp/Crow) C++ micro-framework for HTTP routing.

- **Crow** — header-only, requires C++14 or later
- **Boost** (required by Crow) — `boost::asio` for async I/O
- A C++ compiler supporting C++14 (`g++`, `clang++`)

Install Crow and Boost on Debian/Ubuntu:

```bash
sudo apt install libboost-all-dev
```

Download or clone the Crow header into your project directory:

```bash
git clone https://github.com/CrowCpp/Crow.git
cp Crow/include/crow.h .
```

---

## Building

Compile `main.cpp` with Boost linked:

```bash
g++ -std=c++14 -O2 -o main main.cpp -lboost_system -lpthread
```

If Crow is installed system-wide or via a package manager, adjust the include path accordingly. On some systems you may need `-lboost_thread` as well.

---

## Running

1. Ensure `index.html` is in the same directory as the compiled binary.
2. Start the server:

```bash
./main
```

3. Open a browser and navigate to:

```
http://localhost:8080
```

The server initialises a Segment Tree with random price data on startup and begins serving requests immediately.

---

## REST API

All endpoints are served on `localhost:8080`.

### `GET /`

Serves `index.html`. No parameters.

---

### `GET /api/data`

Returns the full initial price array.

**Response:**

```json
{
  "prices": [142, 155, 167, ...]
}
```

The array contains 60 integer values corresponding to minutes 1 through 60.

---

### `GET /api/query?l={l}&r={r}`

Queries the Segment Tree for the maximum (peak) price in the closed range `[l, r]`.

**Parameters:**

| Parameter | Type    | Description                          |
|-----------|---------|--------------------------------------|
| `l`       | integer | Left bound of the range (1-indexed)  |
| `r`       | integer | Right bound of the range (1-indexed) |

**Constraints:** `1 <= l <= r <= 60`

**Response:**

```json
{
  "peak": 461
}
```

Returns HTTP 400 if the range is invalid.

Each successful query is appended to `log.txt` in the following format:

```
[2026-03-09 14:42:50] Query [38, 45] => Peak: $461
```

---

### `POST /api/tick`

Advances the market by one tick. Randomly selects 1–3 price indices, applies a random price change to each, and updates the Segment Tree.

**Request body:** None

**Response:**

```json
{
  "updates": [
    { "index": 12, "price": 203 },
    { "index": 47, "price": 381 }
  ]
}
```

The frontend calls this endpoint every 250 ms when the market simulation is running.

---

## Usage

### Starting and Stopping the Market

Click **START MARKET** in the controls panel to begin live price simulation. The button label toggles to **STOP MARKET**. Prices update at 250 ms intervals.

### Selecting a Time Range

Click and drag across the bar chart to select a range of minutes. The server is queried immediately and the peak price for the selected range is displayed, along with a reference line on the chart.

### Clearing a Selection

Click **CLEAR SELECTION** to deselect the current range.

---

## Configuration

Two constants control the simulation parameters and are defined at the top of `main.cpp`:

| Constant    | Default | Description                              |
|-------------|---------|------------------------------------------|
| `n`         | `60`    | Number of time intervals (minutes)       |
| `MAX_PRICE` | `500`   | Upper bound for generated prices         |

The same constants are mirrored in `index.html` for chart rendering. If you change them in `main.cpp`, update the corresponding values in the frontend as well.

---

## Algorithm Notes

The Segment Tree is 1-indexed and allocated with `4 * n + 1` nodes. Build time is O(N). Point updates and range-maximum queries each run in O(log N). The tree is held in memory for the lifetime of the server process; there is no persistence of price data across restarts.
