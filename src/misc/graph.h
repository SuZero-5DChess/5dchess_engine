#include <string>
#include <optional>
#include <vector>
#include <cassert>


class graph
{
    // undirected graph of n vertices
    // represented as an adjacency matrix
    int n_vertices;
    std::vector<std::vector<bool>> adj;
public:
    graph(int n): n_vertices{n}
    {
        adj.resize(n);
        for(int i = 0; i < n; i++)
        {
            adj[i].resize(n, false);
        }
    }
    void add_edge(int u, int v);
    void remove_edge(int u, int v);
    bool not_isolated(int u) const;
    std::vector<int> neighbors(int u) const;
    
    std::optional<std::vector<std::pair<int,int>>> find_matching(std::vector<int>& include) const;
    
    std::string to_string() const;
};
