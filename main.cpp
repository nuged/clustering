#include <algorithm>
#include <bitset>
#include <iostream>
#include <fstream>
#include <list>
#include <unordered_set>
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

    // DFS возвращает компоненту связности
    TVertexSet& DFS(TVertex v, TVertexSet& comp, bool* used) const {
        used[v] = true;
        comp.insert(v);
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
        clusters = graph.FindComps();
        return clusters.size();
    }

private:
    Graph graph;
    std::vector<T>& data;
    const Dist distance;
    const size_t limit;
    std::vector<Graph::TVertexSet> clusters;

    void GenerateGraph() {
        if (data.size() == 0)
            return;

        auto start = time(NULL);

        // отсортируем строки по количеству единиц
        std::sort(data.begin(), data.end(), Cmp());

        for (size_t i = 0; i < data.size(); ++i) {
            std::bitset<32> plus2 = -1; // строка полностью из единиц
            auto counter = data[i].count(); // количество единиц в рассматриваемой строке
            if (counter <= 32 - 2) {
                plus2 = plus2 >> (30 - counter); // оставляем в строке на две единицы больше,
            }                                    // чем в рассматриваемой

            // локальным поиском найдем индекс первой строки, в которой на 3 единицы больше
            auto end = std::upper_bound(data.begin(), data.end(), plus2, Cmp()) - data.begin();

            // сравниваем рассматриваемую строку только с теми, в которых не больше, чем counter + 2 единицы
            for (auto j = i; j < end; ++j) {
                auto dist = distance(data[i], data[j]);
                // добавляем в граф ребро, если расстояние между рассматриваемыми строками <= 2
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

    const  std::string filename = "b200000x32.txt";
    std::ifstream fin(filename);
    size_t N, M;
    fin >> N >> M;
    // поместим строки в set, чтобы избавиться от повторяющихся строк
    std::unordered_set<std::bitset<32>> dataset;
    for (size_t i = 0; i < N; ++i) {
        std::bitset<32> x;
        fin >> x;
        dataset.insert(x);
    }
    fin.close();

    // поместим строки в vector для дальнейшей работы
    std::vector<std::bitset<32>> data(dataset.size());
    size_t i = 0;
    for (const auto& item : dataset) {
        data[i] = item;
        ++i;
    }

    ClusterCC<std::bitset<32>,
            size_t (*)(const std::bitset<32>&,
                       const std::bitset<32>&),
            HammingCmp> MyCC(data, HammingDistance, 3);
    auto result = MyCC.Solve();

    auto finish = time(NULL);
    std::cout << "Total time is " << finish - start << " seconds.\n";
    std::cout << "Number of clusters:\t";
    std::cout << result;
    std::cout << std::endl;
    return 0;
}
