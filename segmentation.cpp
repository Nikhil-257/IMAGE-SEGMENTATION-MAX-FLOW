#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#include <bits/stdc++.h>
#include <cmath>
#include <algorithm>
using namespace std;

// -------- Ford–Fulkerson (DFS) --------

struct Edge {
    int v, cap, rev;
    Edge(int _v, int _cap, int _rev) : v(_v), cap(_cap), rev(_rev) {}
};

struct MaxFlow {
    int N;
    vector<vector<Edge>> adj;
    vector<bool> visited;

    MaxFlow(int n) : N(n), adj(n), visited(n) {}

    void addEdge(int u, int v, int cap) {
        adj[u].push_back(Edge(v, cap, adj[v].size()));
        adj[v].push_back(Edge(u, 0, adj[u].size() - 1));
    }

    int dfs(int u, int t, int flow) {
        if (u == t) return flow;
        visited[u] = true;
        for (auto &e : adj[u]) {
            if (!visited[e.v] && e.cap > 0) {
                int pushed = dfs(e.v, t, min(flow, e.cap));
                if (pushed > 0) {
                    e.cap -= pushed;
                    adj[e.v][e.rev].cap += pushed;
                    return pushed;
                }
            }
        }
        return 0;
    }

    int fordFulkerson(int s, int t) {
        int maxFlow = 0, flow;
        while (true) {
            fill(visited.begin(), visited.end(), false);
            flow = dfs(s, t, INT_MAX);
            if (flow == 0) break;
            maxFlow += flow;
        }
        return maxFlow;
    }
};

// ------------ MAIN PROGRAM ------------

int main() {
    string iname;

    cout << "Enter input image name: ";
    cin >> iname;

    const char* input = iname.c_str();
    const char* output = "output.png";

    int w, h, channels;

    //  Load as RGB
    unsigned char* img = stbi_load(input, &w, &h, &channels, 3);

    if (!img) {
        cout << "Error: cannot load image\n";
        return 0;
    }

    int pixels = w * h;
    int S = pixels;
    int T = pixels + 1;

    MaxFlow g(pixels + 2);

    auto id = [&](int r, int c) { return r * w + c; };

    //  Compute mean background color ONCE
    double meanR = 0, meanG = 0, meanB = 0;

    for (int i = 0; i < pixels; i++) {
        meanR += img[i*3 + 0];
        meanG += img[i*3 + 1];
        meanB += img[i*3 + 2];
    }

    meanR /= pixels;
    meanG /= pixels;
    meanB /= pixels;

    double sigma = 30.0;

    //  Build graph
    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {

            int p = id(r, c);

            int rpx = img[p*3 + 0];
            int gpx = img[p*3 + 1];
            int bpx = img[p*3 + 2];

            //  color distance from background
            double dist = sqrt(
                (rpx - meanR)*(rpx - meanR) +
                (gpx - meanG)*(gpx - meanG) +
                (bpx - meanB)*(bpx - meanB)
            );

            //  bigger dist ? likely foreground
            int capSrc  = dist * 5;

            //  smaller dist ? likely background
            int capSink = max(0.0, 200.0 - dist) * 5;

            g.addEdge(S, p, capSrc);
            g.addEdge(p, T, capSink);

            const int dr[2] = {1, 0};
            const int dc[2] = {0, 1};

            for (int k = 0; k < 2; k++) {
                int nr = r + dr[k], nc = c + dc[k];
                if (nr >= 0 && nr < h && nc >= 0 && nc < w) {
                    int q = id(nr, nc);

                    int r2 = img[q*3 + 0];
                    int g2 = img[q*3 + 1];
                    int b2 = img[q*3 + 2];

                    double dist2 = sqrt(
                        (r2 - meanR)*(r2 - meanR) +
                        (g2 - meanG)*(g2 - meanG) +
                        (b2 - meanB)*(b2 - meanB)
                    );

                    double diff = dist - dist2;
                    int w_pq = (int)(50 * exp(-(diff * diff) / (2 * sigma * sigma)));

                    g.addEdge(p, q, w_pq);
                    g.addEdge(q, p, w_pq);
                }
            }
        }
    }

    cout << "Running Ford–Fulkerson...\n";
    int flow = g.fordFulkerson(S, T);
    cout << "Max flow = " << flow << endl;

    vector<unsigned char> mask(pixels);

    vector<bool> vis(pixels + 2, false);
    queue<int> q;
    q.push(S);
    vis[S] = true;

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (auto &e : g.adj[u]) {
            if (e.cap > 0 && !vis[e.v]) {
                vis[e.v] = true;
                q.push(e.v);
            }
        }
    }

    //  white = foreground
    for (int i = 0; i < pixels; i++)
        mask[i] = vis[i] ? 255 : 0;

    stbi_write_png(output, w, h, 1, mask.data(), w);

    cout << "Saved output as output.png\n";
    stbi_image_free(img);
    return 0;
}
