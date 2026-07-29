// =====================================================================
// segment.cpp
// Simple Image Segmentation using Ford–Fulkerson (DFS)
// Foreground = original colors, Background = black
// =====================================================================

// These two lines allow PNG/JPG loading + saving
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

#include <iostream>
#include <vector>
#include <queue>
#include <cmath>
using namespace std;

// =====================================================================
// 1. Edge structure for graph
// Each edge has:
//   v   -> where it goes
//   cap -> capacity (how strong the connection is)
//   rev -> index of reverse edge in adjacency list
// =====================================================================
struct Edge {
    int v, cap, rev;
    Edge(int _v, int _cap, int _rev) : v(_v), cap(_cap), rev(_rev) {}
};

// =====================================================================
// 2. MaxFlow class using Ford-Fulkerson (DFS)
// =====================================================================
struct MaxFlow {
    int N;
    vector<vector<Edge>> adj;   // graph edges
    vector<char> visited;       // to avoid revisiting nodes in DFS

    MaxFlow(int n = 0) : N(n), adj(n), visited(n, 0) {}

    // Add edge u -> v with capacity "cap"
    // Also add reverse edge v -> u with capacity 0
    void addEdge(int u, int v, int cap) {
        adj[u].push_back(Edge(v, cap, adj[v].size()));
        adj[v].push_back(Edge(u, 0, adj[u].size() - 1));
    }

    // DFS tries to push flow from u to t
    int dfs(int u, int t, int flow) {
        if (u == t) return flow;    // reached sink

        visited[u] = 1;

        for (auto &e : adj[u]) {
            if (!visited[e.v] && e.cap > 0) {
                // push as much as possible through this edge
                int pushed = dfs(e.v, t, min(flow, e.cap));
                if (pushed > 0) {
                    e.cap -= pushed;                         // reduce forward capacity
                    adj[e.v][e.rev].cap += pushed;           // increase reverse capacity
                    return pushed;
                }
            }
        }
        return 0;  // no flow possible
    }

    // Repeat DFS until no more flow is pushed
    int fordFulkerson(int s, int t) {
        int maxFlow = 0;
        while (true) {
            fill(visited.begin(), visited.end(), 0);
            int f = dfs(s, t, INT_MAX);
            if (f == 0) break;
            maxFlow += f;
        }
        return maxFlow;
    }
};

// =====================================================================
// 3. Helper: compute color distance between two RGB pixels
// =====================================================================
double colorDist(double r1,double g1,double b1,
                 double r2,double g2,double b2)
{
    return sqrt((r1-r2)*(r1-r2) +
                (g1-g2)*(g1-g2) +
                (b1-b2)*(b1-b2));
}

// =====================================================================
// 4. MAIN PROGRAM
// =====================================================================
int main() {

    // ------------------------------ Load image -------------------------
    string inputName;
    cout << "Enter image name: ";
    cin >> inputName;

    int w, h, ch;
    unsigned char *img = stbi_load(inputName.c_str(), &w, &h, &ch, 3); // load RGB
    if (!img) {
        cout << "Error loading image\n";
        return 1;
    }

    int pixels = w * h;

    int S = pixels;     // Source node index
    int T = pixels + 1; // Sink node index

    MaxFlow mf(pixels + 2);

    auto id = [&](int r, int c){ return r * w + c; };

    // ------------------------------ FIND BACKGROUND COLOR -------------------------
    // We assume background is near the borders.
    long long Rsum=0, Gsum=0, Bsum=0;
    long long count = 0;

    // top and bottom rows
    for (int c = 0; c < w; c++) {
        int p1 = id(0, c);
        int p2 = id(h - 1, c);
        Rsum += img[p1*3]; Gsum += img[p1*3+1]; Bsum += img[p1*3+2];
        Rsum += img[p2*3]; Gsum += img[p2*3+1]; Bsum += img[p2*3+2];
        count += 2;
    }
    // left and right columns
    for (int r = 0; r < h; r++) {
        int p1 = id(r, 0);
        int p2 = id(r, w - 1);
        Rsum += img[p1*3]; Gsum += img[p1*3+1]; Bsum += img[p1*3+2];
        Rsum += img[p2*3]; Gsum += img[p2*3+1]; Bsum += img[p2*3+2];
        count += 2;
    }

    // average border color = background estimate
    double bgR = Rsum / (double)count;
    double bgG = Gsum / (double)count;
    double bgB = Bsum / (double)count;

    // Some tuning parameters
    double sigma = 30.0;     // controls smoothness
    double lambda = 50.0;    // weight between neighbors
    double dataScale = 5.0;  // scaling factor

    // ------------------------------ BUILD GRAPH ---------------------------------
    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {

            int p = id(r, c);
            double R1 = img[p*3];
            double G1 = img[p*3+1];
            double B1 = img[p*3+2];

            // Distance to background color
            double d = colorDist(R1, G1, B1, bgR, bgG, bgB);

            // If pixel is very different from background → likely foreground
            int capSrc  = int(d * dataScale);

            // If pixel is close to background → likely background
            int capSink = int(max(0.0, 150.0 - d) * dataScale);

            // Connect pixel to Source and Sink
            mf.addEdge(S, p, capSrc);
            mf.addEdge(p, T, capSink);

            // Connect neighbors (right + down)
            int dr[2] = {1, 0};
            int dc[2] = {0, 1};

            for (int k = 0; k < 2; k++) {
                int nr = r + dr[k], nc = c + dc[k];
                if (nr < 0 || nr >= h || nc < 0 || nc >= w) continue;

                int q = id(nr, nc);

                double R2 = img[q*3];
                double G2 = img[q*3+1];
                double B2 = img[q*3+2];

                // If neighbors have similar color, give them strong connection
                double diff = colorDist(R1,G1,B1, R2,G2,B2);
                int w_pq = int(lambda * exp(-(diff*diff)/(2*sigma*sigma)));

                mf.addEdge(p, q, w_pq);
                mf.addEdge(q, p, w_pq);
            }
        }
    }

    // ------------------------------ RUN FORD–FULKERSON -------------------------
    cout << "Running Ford-Fulkerson...\n";
    mf.fordFulkerson(S, T);

    // ------------------------------ FIND FOREGROUND NODES -----------------------
    vector<char> vis(pixels+2, 0);

    queue<int> q;
    q.push(S);
    vis[S] = 1;

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (auto &e : mf.adj[u]) {
            if (e.cap > 0 && !vis[e.v]) {
                vis[e.v] = 1;
                q.push(e.v);
            }
        }
    }

    // ------------------------------ CREATE FINAL IMAGE --------------------------
    // Foreground = original pixel
    // Background = black
    vector<unsigned char> outRGB(pixels * 3);

    for (int i = 0; i < pixels; i++) {
        if (vis[i]) {
            outRGB[i*3 + 0] = img[i*3 + 0];
            outRGB[i*3 + 1] = img[i*3 + 1];
            outRGB[i*3 + 2] = img[i*3 + 2];
        } else {
            outRGB[i*3 + 0] = 0;
            outRGB[i*3 + 1] = 0;
            outRGB[i*3 + 2] = 0;
        }
    }

    stbi_write_png("output.png", w, h, 3, outRGB.data(), w*3);
    cout << "Saved output.png\n";

    stbi_image_free(img);
    return 0;
}
