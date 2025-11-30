#ifndef __PATH_FINDER_H__
#define __PATH_FINDER_H__

#include "HexUtils.h"
#include <vector>
#include <map>
#include <queue>
#include <functional>
#include <algorithm>

class PathFinder {
public:
    // 回调函数：返回移动消耗，负数表示不可通行
    typedef std::function<int(Hex)> CostCallback;

    static std::vector<Hex> findPath(Hex start, Hex end, CostCallback getCost) {
        std::vector<Hex> path;
        if (getCost(end) < 0) return path; // 终点不可达

        typedef std::pair<int, Hex> Element;
        std::priority_queue<Element, std::vector<Element>, std::greater<Element>> frontier;

        frontier.push({ 0, start });

        std::map<Hex, Hex> came_from;
        std::map<Hex, int> cost_so_far;

        came_from[start] = start;
        cost_so_far[start] = 0;

        std::vector<Hex> directions = { 
            Hex(1, 0), Hex(1, -1), Hex(0, -1), Hex(-1, 0), Hex(-1, 1), Hex(0, 1)
        };

        while (!frontier.empty()) {
            Hex current = frontier.top().second;
            frontier.pop();

            if (current == end) break;

            for (Hex dir : directions) {
                Hex next = current + dir;
                int move_cost = getCost(next);
                if (move_cost < 0) continue;

                int new_cost = cost_so_far[current] + move_cost;
                if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next]) {
                    cost_so_far[next] = new_cost;
                    int priority = new_cost + hexDistance(next, end);
                    frontier.push({ priority, next });
                    came_from[next] = current;
                }
            }
        }

        if (came_from.find(end) != came_from.end()) {
            Hex curr = end;
            while (!(curr == start)) {
                path.push_back(curr);
                curr = came_from[curr];
            }
            std::reverse(path.begin(), path.end());
        }
        return path;
    }

private:
    static int hexDistance(Hex a, Hex b) {
        return (std::abs(a.q - b.q) + std::abs(a.q + a.r - b.q - b.r) + std::abs(a.r - b.r)) / 2;
    }
};
#endif