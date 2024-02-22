import math

import networkx as nx
import matplotlib.pyplot as plt
from collections import defaultdict

def next_power_of_2(x):
    return 1 if x == 0 else 2 ** math.ceil(math.log2(x))


def generate_partition_sequence(n):
    result = []

    for i in range(n):
        if i == 0:
            result.append([(1, 1)])
        else:
            last_iteration = result[i - 1]
            j, k = last_iteration[-1]

            if k == j:
                result.append(last_iteration + [(next_power_of_2(j + 1), 1)])
            else:
                result.append(last_iteration + [(j, k + 1)])

    return result


def plot_graph(graph, backtrace):
    plt.figure(figsize=(40, 40))
    pos = {node: (node[0], node[1] + node[2]) for node in graph.nodes()}
    node_colors = ['blue' if node in backtrace else 'gray' for node in graph.nodes()]
    nx.draw(graph, pos, with_labels=True, node_size=1400, font_size=8, font_color='white', node_color=node_colors)
    plt.show()


def get_transition_cost(p1: int, p2: int, q1: int, q2: int, latency: int, k: float):
    if p1 == p2 and (q1 + 1) == q2:
        return 0

    # Append another block of the same size
    elif p1 == p2 and q2 == 1:
        return 4

    # Start a new larger block
    elif p1 < p2 and q2 == 1:
        return 4 * k * math.log2(2 * latency * p2) + 4

    raise Exception("Invalid transition!")


def trace_backward(graph, leaf_node):
    backward_trace = [leaf_node]

    while graph.in_degree(leaf_node) > 0:
        predecessor = list(graph.predecessors(leaf_node))[0]
        backward_trace.append(predecessor)
        leaf_node = predecessor

    return backward_trace[::-1]


def find_optimum(latency, ir_size):
    num_transitions = ir_size // latency
    transition_sequence = generate_partition_sequence(num_transitions)
    graph = nx.DiGraph()

    # Add partition nodes to graph... -> not needed as all edges will be touched...
    for transition in range(num_transitions):
        partitions = transition_sequence[transition]
        for partition in partitions:
            graph.add_node((transition,) + partition, cost=0)

    for transition in range(1, num_transitions):
        current_partitions = transition_sequence[transition]
        last_partitions = transition_sequence[transition - 1]

        for last_partition in last_partitions:
            for current_partition in current_partitions:
                p2, q2 = current_partition
                p1, q1 = last_partition

                if ((p1 == q1 and q2 == 1 and p2 >= p1) or
                        (p1 == p2 and q2 == (q1 + 1))):
                    graph.add_edge((transition - 1,) + last_partition, (transition,) + current_partition)

    def find_best_path(key):
        edges = graph.in_edges(key)
        best_edge = None
        best_cost = None

        remove_edges = []
        for edge in edges:
            u, v = edge
            t, p2, q2 = v
            t, p1, q1 = u

            transition_cost = get_transition_cost(p1, p2, q1, q2, latency, 1.5)
            total_cost = graph.nodes[u]["cost"] + transition_cost
            if best_cost is None or total_cost <= best_cost:
                if best_edge is not None:
                    remove_edges.append(best_edge)
                best_cost = total_cost
                best_edge = edge

        if best_cost is not None:
            graph.nodes[key]["cost"] = best_cost
        graph.remove_edges_from(remove_edges)

    def iterate_next_connections(key):
        find_best_path(key)
        node = graph[key].copy()
        for edge in node:
            iterate_next_connections(edge)

    starting_key = (0, 1, 1)
    iterate_next_connections(starting_key)

    leaf_nodes = [node for node in graph.nodes() if graph.in_degree(node)!=0 and graph.out_degree(node)==0]

    best_cost = None
    best_leaf = None
    for leaf in leaf_nodes:
        cost = graph.nodes[leaf]["cost"]
        if best_cost is None or cost <= best_cost:
            best_cost = cost
            best_leaf = leaf

    backtrace = trace_backward(graph, best_leaf)
    normal_backtrace = defaultdict(int)

    for node in backtrace:
        normal_backtrace[node[1] * latency] += 1

    print(normal_backtrace)
    plot_graph(graph, backtrace)


if __name__ == '__main__':
    find_optimum(1, 8)
