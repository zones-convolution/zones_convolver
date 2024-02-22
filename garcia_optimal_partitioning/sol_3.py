import math
import networkx as nx
import matplotlib.pyplot as plt
from collections import defaultdict


def plot_graph(graph, backtrace):
    plt.figure(figsize=(10, 10))
    pos = {node: (node[0], node[2] + node[3]) for node in graph.nodes()}
    node_colors = ['blue' if node in backtrace else 'gray' for node in graph.nodes()]
    nx.draw(graph, pos, with_labels=True, node_size=1400, font_size=8, font_color='white', node_color=node_colors)
    plt.show()


def next_power_of_2(x: int):
    return 1 if x == 0 else 2 ** math.ceil(math.log2(x))


def get_next_sequence(sequence: list[tuple[int, int, int]]):
    next_sequence = []

    for element in sequence:
        n, s, q = element
        next_sequence.append((n + 1, s, q))

    n, s, q = sequence[-1]
    if q == s:
        # Transition to longer sub-filter
        next_sub_filter_size = next_power_of_2(s + 1)
        next_sequence.append((n + next_sub_filter_size, next_sub_filter_size, 1))
    else:
        # Continuation of sub-filter
        next_sequence.append((n, s, q + 1))

    return next_sequence


def is_transition_viable(first: tuple[int, int, int], second: tuple[int, int, int]) -> bool:
    n2, s2, q2 = second
    n1, s1, q1 = first

    clearance_condition = True

    # Check for clearance because we moved to a new subfilter
    if q2 == 1:
        clearance = n1 - s2 + 1
        clearance_condition = clearance >= 1

    condition_1 = s1 == q1 and q2 == 1 and s2 >= s1
    condition_2 = s1 == s2 and q2 == (q1 + 1)

    return (condition_1 or condition_2) and clearance_condition


def get_viable_transitions(node: tuple[int, int, int], to: list[tuple[int, int, int]]):
    viable_transitions: list[tuple[int, int, int]] = []
    for target in to:
        if is_transition_viable(node, target):
            viable_transitions.append(target)

    return viable_transitions


def get_transition_cost(first: tuple[int, int, int], second: tuple[int, int, int], latency: int, k: float):
    n1, s1, q1 = first
    n2, s2, q2 = second

    # Continuation of a sub filter
    if s1 == s2 and (q1 + 1) == q2:
        return 0

    # Repetition of preceding sub filter
    elif s1 == s2 and q2 == 1:
        return 4

    # Start a new larger block
    elif s1 < s2 and q2 == 1:
        return 4 * k * math.log2(2 * latency * s2) + 4

    raise Exception("Invalid transition!")


def get_graph_leafs(graph):
    return [node for node in graph.nodes() if graph.in_degree(node) != 0 and graph.out_degree(node) == 0]


def trace_backward(graph, leaf_node):
    backward_trace = [leaf_node]

    while graph.in_degree(leaf_node) > 0:
        predecessor = list(graph.predecessors(leaf_node))[0]
        backward_trace.append(predecessor)
        leaf_node = predecessor

    return backward_trace[::-1]


def find_optimum(latency, ir_size):
    num_transitions = ir_size // latency
    sequence = [(1, 1, 1)]
    k = 1.7

    graph = nx.DiGraph()
    graph.add_node((0, 1, 1, 1), cost=0)

    for transition_index in range(1, num_transitions):
        next_sequence = get_next_sequence(sequence)
        for element in sequence:
            key1 = (transition_index - 1,) + element
            if not graph.has_node(key1):
                continue

            accumulated_cost = graph.nodes[key1]["cost"]

            viable_transitions = get_viable_transitions(element, next_sequence)
            for transition in viable_transitions:
                key2 = (transition_index,) + transition
                transition_cost = get_transition_cost(element, transition, latency, k)
                total_cost = transition_cost + accumulated_cost

                edges_to_remove = None
                if graph.has_node(key2):
                    node_cost = graph.nodes[key2]["cost"]
                    if total_cost < node_cost:
                        in_edges = graph.in_edges(key2)
                        edges_to_remove = []
                        for edge in in_edges:
                            edges_to_remove.append(edge)
                        graph.add_edge(key1, key2)
                        graph.nodes[key2]["cost"] = total_cost
                else:
                    graph.add_edge(key1, key2)
                    graph.nodes[key2]["cost"] = total_cost

                if edges_to_remove is not None:
                    graph.remove_edges_from(edges_to_remove)

        sequence = next_sequence

    leafs = get_graph_leafs(graph)
    best_cost = None
    best_leafs = []
    for leaf in leafs:
        cost = graph.nodes[leaf]["cost"]
        if best_cost is None or cost <= best_cost:
            if best_cost is not None and cost < best_cost:
                best_leafs.clear()
            best_cost = cost
            best_leafs.append(leaf)

    backtraces = []
    for best_leaf in best_leafs:
        backtraces.append(trace_backward(graph, best_leaf))
    # plot_graph(graph, backtrace)

    normal_backtraces = []

    for backtrace in backtraces:
        normal_backtrace = defaultdict(int)
        for node in backtrace:
            normal_backtrace[node[2] * latency] += 1
        for key in normal_backtrace:
            normal_backtrace[key] = math.ceil(normal_backtrace[key] / (key / latency))
        normal_backtraces.append(normal_backtrace)

    print(normal_backtraces)
    # plot_graph(graph, backtraces[-1])


if __name__ == '__main__':
    find_optimum(1024, 65536)
