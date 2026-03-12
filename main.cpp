#include "crow.h"
#include <vector>
#include <algorithm>
#include <random>
#include <cstdio>
#include <ctime>
#include <iostream>
using namespace std;
const int n = 60;
const int MAX_PRICE = 500;

class SegmentTree {
private:
    vector<int> tree;
    vector<int> arr;

    void build(int node, int start, int end) {
        if (start == end) {
            tree[node] = arr[start];
        } else {
            int mid = start + (end - start) / 2;
            build(2 * node, start, mid);
            build(2 * node + 1, mid + 1, end);
            tree[node] = max(tree[2 * node], tree[2 * node + 1]);
        }
    }

    void update(int node, int start, int end, int idx, int val) {
        if (start == end) {
            arr[idx] = val;
            tree[node] = val;
        } else {
            int mid = start + (end - start) / 2;
            if (start <= idx && idx <= mid) {
                update(2 * node, start, mid, idx, val);
            } else {
                update(2 * node + 1, mid + 1, end, idx, val);
            }
            tree[node] = max(tree[2 * node], tree[2 * node + 1]);
        }
    }

    int query(int node, int start, int end, int l, int r) {
        if (r < start || end < l) return -1e9; // -Infinity equivalent
        if (l <= start && end <= r) return tree[node];

        int mid = start + (end - start) / 2;
        int p1 = query(2 * node, start, mid, l, r);
        int p2 = query(2 * node + 1, mid + 1, end, l, r);
        return max(p1, p2);
    }
public:
    SegmentTree() {
        arr.assign(n + 1, 0);
        tree.assign(4 * n + 1, 0);
    }

    void initRandomData() {
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<> dis(-15.0, 25.0);

        double currentPrice = 150.0;
        for (int i = 1; i <= n; i++) {
            currentPrice += dis(gen);
            currentPrice = max(10.0, min((double)MAX_PRICE - 10, currentPrice));
            arr[i] = (int)currentPrice;
        }
        build(1, 1, n);
    }

    int getPrice(int idx) { return arr[idx]; }
    const vector<int>& getArray() { return arr; }

    void updatePrice(int idx, int val) { update(1, 1, n, idx, val); }
    int queryPeak(int l, int r) { return query(1, 1, n, l, r); }
};

void logQuery(int l, int r, int peak) {
    FILE* f = fopen("log.txt", "a");
    if(!f) return;

    time_t now = time(nullptr);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&now));


    fprintf(f, "[%s] Query [%d, %d] => Peak: $%d\n", timebuf, l, r, peak);
    fclose(f);
}

int main() {
    crow::SimpleApp app;
    SegmentTree st;
    st.initRandomData();

    // 1. Serve the HTML Frontend
    CROW_ROUTE(app, "/")([](){
        crow::response res;
        res.set_static_file_info("index.html");
        return res;
    });

    // 2. API: Get initial array data
    CROW_ROUTE(app, "/api/data")([&st](){
        crow::json::wvalue x;
        const auto& arr = st.getArray();
        for(int i = 1; i <= n; i++) {
            x["prices"][i-1] = arr[i];
        }
        return x;
    });

    // 3. API: Query Peak in a range
    CROW_ROUTE(app, "/api/query")([&st](const crow::request& req){
        int l = -1, r = -1;
        if(req.url_params.get("l")) l = stoi(req.url_params.get("l"));
        if(req.url_params.get("r")) r = stoi(req.url_params.get("r"));

        if (l < 1 || r > n || l > r) return crow::response(400);

        int peak = st.queryPeak(l, r);
        logQuery(l, r, peak);
        crow::json::wvalue x;
        x["peak"] = peak;
        return crow::response(x);
    });

    CROW_ROUTE(app, "/api/tick").methods(crow::HTTPMethod::POST)([&st](){
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> numUpdatesDist(1, 3);
        uniform_int_distribution<> minDist(1, n);
        uniform_real_distribution<> changeDist(-50.0, 50.0);

        int updates = numUpdatesDist(gen);
        crow::json::wvalue response_data;

        for(int i = 0; i < updates; i++) {
            int randomMinute = minDist(gen);
            int currentPrice = st.getPrice(randomMinute);
            int change = (int)changeDist(gen);
            int newPrice = max(10, min(MAX_PRICE - 10, currentPrice + change));

            st.updatePrice(randomMinute, newPrice);

            response_data["updates"][i]["index"] = randomMinute;
            response_data["updates"][i]["price"] = newPrice;
        }

        return crow::response(response_data);
    });

    cout << "Starting Server on http://localhost:8080\n";
    app.port(8080).multithreaded().run();
}
