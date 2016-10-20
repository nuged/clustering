#include <algorithm>
#include <bitset>
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <set>

class Graph {
public:
    using TVertex = size_t;
    using TVertexSet= std::set<TVertex>;
    using TAdjList = std::vector<TVertexSet>;

    Graph(size_t size)
            : graph(size)
    {}

    void AddEdge(const TVertex& a, const TVertex& b) {
        graph[a].insert(b);
        graph[b].insert(a);
    }

    std::vector<TVertexSet> FindComps() const {
        std::vector<TVertexSet> comps;
        bool used[graph.size()];
        for (size_t i = 0; i < graph.size(); ++i) {
            used[i] = false;
        }
        for (size_t i = 0; i < graph.size(); ++i) {
            if (!used[i]) {
                TVertexSet comp;
                comp = DFS(i, comp, used);
                comps.push_back(comp);
            }

        }
        return comps;
    }

    size_t Size() const {
        return graph.size();
    }

    const TAdjList& GetAdjList() const {
        return graph;
    }

private:
    TAdjList graph;

    TVertexSet& DFS(TVertex v, TVertexSet& comp, bool* used) const {
        used[v] = true;
        comp.insert(v);
        /*
        std::cout << "adj of " << v << ":\n";
        for (auto& elem : AdjVertices(v))
            std::cout << "\t" << elem << "\n";
        */
        for (const auto& elem : AdjVertices(v)) {
            if (!used[elem])
                DFS(elem, comp, used);
        }
        return comp;
    }

    const TVertexSet& AdjVertices(TVertex v) const {
        return graph[v];
    }
};

std::ostream& operator<<(std::ostream& out, Graph graph) {
    out << "strict graph {\n";
    for (size_t i = 0; i < graph.Size(); ++i)
        out << "\t" << i << "\n";
    for (size_t i = 0; i < graph.Size(); ++i) {
        for (const auto &item : graph.GetAdjList().at(i))
            out << "\t" << i << "--" << item << "\n";
    }
    out << "}\n";
}

template <typename T, typename Dist, typename Cmp>
class ClusterCC {
public:
    ClusterCC(std::vector<T>& data, Dist distance, size_t lim)
        : data(data), distance(distance), limit(lim), graph(data.size())
    {}

    size_t Solve() {
        GenerateGraph();
//        std::cout << graph;
        clusters = graph.FindComps();
/*
        for (const auto& item : clusters) {
            std::cout << "CC:\n";
            for (const auto& elem : item)
                std::cout << "\t" << elem << "\n";
            std::cout << "\n";
        }
*/
        return clusters.size();
    }

private:
    Graph graph;
    std::vector<T>& data;
    const Dist distance;
    const size_t limit;
    std::vector<Graph::TVertexSet> clusters;
    void GenerateGraph() {
        auto start = time(NULL);
        std::sort(data.begin(), data.end(), Cmp());
        if (data.size() == 0)
            return;

        for (size_t i = 0; i < data.size(); ++i) {
            std::bitset<32> plus2 = -1;
            auto counter = data[i].count();

            if (counter <= 32 - 2) {
                plus2 = plus2 >> (30 - counter);
            }

            auto end = std::upper_bound(data.begin(), data.end(), plus2, Cmp()) - data.begin();

            for (auto j = i; j < end; ++j) {
                auto dist = distance(data[i], data[j]);
                if (dist < limit)
                    graph.AddEdge(i, j);
            }
        }
        auto finish = time(NULL);
        std::cout << "Graph generating time is " << finish - start << " seconds.\n";
    }
};

size_t HammingDistance(const std::bitset<32>& a,
                     const std::bitset<32>& b) {
    std::bitset<32> x = a ^ b;
    return x.count();
}

struct HammingCmp {
    bool operator() (const std::bitset<32>& a,
                     const std::bitset<32>& b) const {
        return a.count() < b.count();
    }
};

int main() {
    auto start = time(NULL);
    const  std::string filename = "/home/ed/Документы/Coding/Homework/CC_Clustering/b200000x32.txt";
    std::ifstream fin(filename);
    size_t N, M;
    fin >> N >> M;
    std::vector<std::bitset<32>> data(N);
    for (size_t i = 0; i < N; ++i) {
        std::bitset<32> x;
        fin >> x;
        data[i] = x;
    }
    fin.close();
    std::sort(data.begin(), data.end(), HammingCmp());
    ClusterCC<std::bitset<32>,
            size_t (*)(const std::bitset<32>&,
                       const std::bitset<32>&),
                               HammingCmp> MyCC(data, HammingDistance, 3);
    auto result = MyCC.Solve();
    std::cout << "Number of clusters:\t";
    std::cout << result;
    std::cout << std::endl;
    auto finish = time(NULL);
    std::cout << "Total time is " << finish - start << " seconds.\n";
    return 0;
}
