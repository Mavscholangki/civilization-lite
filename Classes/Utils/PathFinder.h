#ifndef __PATH_FINDER_H__
#define __PATH_FINDER_H__

#include "HexUtils.h"
#include <vector>
#include <map>
#include <queue>
#include <functional>
#include <algorithm>

/**
 * @brief 寻路算法类
 * 提供基于 A* 算法的路径规划和距离计算
 */
class PathFinder {
public:
    /**
     * @brief 移动成本的回调函数
     * 给定 Hex 坐标，返回该格子的移动成本
     * 返回负数（如 -1）表示该格子不可通行
     */
    using CostCallback = std::function<int(Hex)>;

    /**
     * @brief 使用A*算法寻找从起点到终点的最短路径
     * @param start 起始点坐标
     * @param end 终点坐标
     * @param getCost 一个回调函数，在需要查询特定格子的移动成本
     * @return std::vector<Hex> 包含路径的格子序列
     *         如果路径不通或终点不可达，返回空路径
     */
    static std::vector<Hex> findPath(Hex start, Hex end, CostCallback getCost) {
        std::vector<Hex> path;

        // 如果终点本身不通行，直接返回空路径
        if (getCost(end) < 0) {
            return path;
        }

        // 优先队列，存储待探索的结点，按照优先级排序（最小优先队列）
        // pair 的第一个值为优先级 f = g + h，第二个值为坐标
        typedef std::pair<int, Hex> Element;
        std::priority_queue<Element, std::vector<Element>, std::greater<Element>> frontier;

        frontier.push({ 0, start });

        std::map<Hex, Hex> came_from;    // 记录路径：当前格子是从前一个格子是谁
        std::map<Hex, int> cost_so_far;  // 记录成本：从起点到当前格子的实际成本

        came_from[start] = start;
        cost_so_far[start] = 0;

        // 定义六邻域的 6 个方向偏移量
        std::vector<Hex> directions;
        directions.push_back(Hex(1, 0));
        directions.push_back(Hex(1, -1));
        directions.push_back(Hex(0, -1));
        directions.push_back(Hex(-1, 0));
        directions.push_back(Hex(-1, 1));
        directions.push_back(Hex(0, 1));

        while (!frontier.empty()) {
            Hex current = frontier.top().second;
            frontier.pop();

            // 找到了终点，停止搜索
            if (current == end) {
                break;
            }

            // 探索当前结点的 6 个邻域
            for (Hex dir : directions) {
                Hex next = current + dir;
                int move_cost = getCost(next);

                // 如果邻域不通行，move_cost < 0，跳过它
                if (move_cost < 0) {
                    continue;
                }

                // 计算经过当前结点到邻域的总成本
                int new_cost = cost_so_far[current] + move_cost;

                // 如果邻域没去过，或者找到了更便宜的路线，则进行邻域更新
                if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next]) {
                    cost_so_far[next] = new_cost;

                    // A* 算法的优先级 = 已经走过的成本(g) + 预估剩余成本(h)
                    int priority = new_cost + next.distance(end);

                    frontier.push({ priority, next });
                    came_from[next] = current;
                }
            }
        }

        // 通过回溯图中的记录从终点回到起点，然后反向输出路径
        if (came_from.find(end) != came_from.end()) {
            Hex curr = end;
            while (!(curr == start)) {
                path.push_back(curr);
                curr = came_from[curr];
            }
            // 反向得到的路径是 终点->起点，需要反转得到 起点->终点
            std::reverse(path.begin(), path.end());
        }
        return path;
    }

    /**
     * @brief 获取可达范围内所有格子 (BFS 算法)
     * @param center 中心点
     * @param movementPoints 剩余的移动点数
     * @param getCost 回调函数，返回某格子的移动成本 (-1表示不通行)
     * @return 包含所有可达的 Hex 坐标的向量
     */
    static std::vector<Hex> getReachableHexes(Hex center, int movementPoints, CostCallback getCost) {
        std::vector<Hex> visited;
        visited.push_back(center);

        std::vector<std::vector<Hex>> fringes; // 每一层的边界
        fringes.push_back({ center });

        // 记录每个格子可达时的剩余移动点数，防止重复探索且路径更优时再次探索
        std::map<Hex, int> maxRemainingMoves;
        maxRemainingMoves[center] = movementPoints;

        for (int k = 1; k <= movementPoints; k++) {
            fringes.push_back({});
            for (Hex hex : fringes[k - 1]) {
                // 定义 6 个方向
                std::vector<Hex> directions;
                directions.push_back(Hex(1, 0));
                directions.push_back(Hex(1, -1));
                directions.push_back(Hex(0, -1));
                directions.push_back(Hex(-1, 0));
                directions.push_back(Hex(-1, 1));
                directions.push_back(Hex(0, 1));

                for (Hex dir : directions) {
                    Hex neighbor = hex + dir;
                    int cost = getCost(neighbor); // 查询邻域成本

                    // 获取当前格子剩余的移动点数
                    int currentRemains = maxRemainingMoves[hex];

                    // 如果邻域不通行或剩余点数不支持跨越成本，跳过
                    if (cost < 0 || currentRemains < cost) {
                        continue;
                    }

                    int newRemains = currentRemains - cost;

                    // 如果邻域之前未去过，或新去时剩余的移动点数更多（说明路线更优）
                    if (maxRemainingMoves.find(neighbor) == maxRemainingMoves.end() || newRemains > maxRemainingMoves[neighbor]) {
                        maxRemainingMoves[neighbor] = newRemains;
                        fringes[k].push_back(neighbor);
                        visited.push_back(neighbor);
                    }
                }
            }
        }
        return visited;
    }
};

#endif