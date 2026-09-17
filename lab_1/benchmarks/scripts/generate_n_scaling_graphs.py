from __future__ import annotations

import csv
from pathlib import Path
from xml.sax.saxutils import escape

LAB_ROOT = Path(__file__).resolve().parents[2]
DATA_DIRECTORY = LAB_ROOT / "benchmarks" / "data"
OUTPUT_DIRECTORY = LAB_ROOT / "benchmarks" / "graphs"
WIDTH = 1200
HEIGHT = 760
LEFT = 130
RIGHT = 60
TOP = 100
BOTTOM = 130
PLOT_WIDTH = WIDTH - LEFT - RIGHT
PLOT_HEIGHT = HEIGHT - TOP - BOTTOM
COLORS = ["#1769aa", "#d65a31", "#2e8b57"]


def read_csv(filename: str) -> list[dict[str, str]]:
    with (DATA_DIRECTORY / filename).open(newline="", encoding="utf-8") as file:
        return list(csv.DictReader(file))


def graph(filename: str, title: str, y_label: str,
          series: list[tuple[str, list[tuple[float, float]], str]]) -> None:
    values = [value for _, points, _ in series for _, value in points]
    x_values = sorted({x for _, points, _ in series for x, _ in points})
    y_max = max(values) * 1.10 if max(values) > 0 else 1.0

    def x_position(value: float) -> float:
        return LEFT + (value - x_values[0]) / (x_values[-1] - x_values[0]) * PLOT_WIDTH

    def y_position(value: float) -> float:
        return TOP + (1.0 - value / y_max) * PLOT_HEIGHT

    svg: list[str] = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{WIDTH}" height="{HEIGHT}" viewBox="0 0 {WIDTH} {HEIGHT}">',
        '<rect width="100%" height="100%" fill="white"/>',
        f'<text x="{WIDTH / 2}" y="42" text-anchor="middle" font-family="Arial, sans-serif" font-size="26" font-weight="bold">{escape(title)}</text>',
        '<text x="600" y="72" text-anchor="middle" font-family="Arial, sans-serif" font-size="16">One run per configuration; fixed worker count: 20</text>',
    ]

    for tick in range(6):
        value = y_max * tick / 5
        y = y_position(value)
        svg.extend([
            f'<line x1="{LEFT}" y1="{y:.2f}" x2="{WIDTH - RIGHT}" y2="{y:.2f}" stroke="#d9d9d9" stroke-width="1"/>',
            f'<text x="{LEFT - 12}" y="{y + 5:.2f}" text-anchor="end" font-family="Arial, sans-serif" font-size="15">{value:.2f}</text>',
        ])

    for x_value in x_values:
        x = x_position(x_value)
        svg.extend([
            f'<line x1="{x:.2f}" y1="{TOP}" x2="{x:.2f}" y2="{HEIGHT - BOTTOM}" stroke="#eeeeee" stroke-width="1"/>',
            f'<text x="{x:.2f}" y="{HEIGHT - BOTTOM + 30}" text-anchor="middle" font-family="Arial, sans-serif" font-size="14">{x_value / 1_000_000:.0f}</text>',
        ])

    svg.extend([
        f'<line x1="{LEFT}" y1="{HEIGHT - BOTTOM}" x2="{WIDTH - RIGHT}" y2="{HEIGHT - BOTTOM}" stroke="black" stroke-width="2"/>',
        f'<line x1="{LEFT}" y1="{TOP}" x2="{LEFT}" y2="{HEIGHT - BOTTOM}" stroke="black" stroke-width="2"/>',
        f'<text x="{WIDTH / 2}" y="{HEIGHT - 42}" text-anchor="middle" font-family="Arial, sans-serif" font-size="18">Input limit n (millions)</text>',
        f'<text x="32" y="{HEIGHT / 2}" text-anchor="middle" transform="rotate(-90 32 {HEIGHT / 2})" font-family="Arial, sans-serif" font-size="18">{escape(y_label)}</text>',
    ])

    legend_x = LEFT + 20
    for index, (name, points, color) in enumerate(series):
        polyline = " ".join(f"{x_position(x):.2f},{y_position(y):.2f}" for x, y in points)
        svg.append(f'<polyline points="{polyline}" fill="none" stroke="{color}" stroke-width="3"/>')
        for x, y in points:
            svg.append(f'<circle cx="{x_position(x):.2f}" cy="{y_position(y):.2f}" r="4" fill="{color}"/>')
        legend_y = TOP + 25 + index * 28
        svg.extend([
            f'<line x1="{legend_x}" y1="{legend_y}" x2="{legend_x + 28}" y2="{legend_y}" stroke="{color}" stroke-width="3"/>',
            f'<text x="{legend_x + 38}" y="{legend_y + 5}" font-family="Arial, sans-serif" font-size="16">{escape(name)}</text>',
        ])

    svg.append('</svg>')
    (OUTPUT_DIRECTORY / filename).write_text("\n".join(svg), encoding="utf-8")


def points(rows: list[dict[str, str]], field: str) -> list[tuple[float, float]]:
    return [(float(row["n"]), float(row[field])) for row in rows]


def main() -> None:
    task2 = read_csv("task2_n_scaling_threads20.csv")
    task3 = read_csv("task3_n_scaling_threads20.csv")
    comparison = read_csv("pthread_openmp_n_scaling_threads20.csv")

    graph(
        "task2_runtime_increasing_n.svg",
        "Task 2: Serial vs Pthreads Runtime as n Increases",
        "Elapsed wall time (seconds)",
        [
            ("Serial", points(task2, "serial_seconds"), COLORS[0]),
            ("Pthreads", points(task2, "parallel_seconds"), COLORS[1]),
        ],
    )
    graph(
        "task2_speedup_increasing_n.svg",
        "Task 2: Pthreads Speedup as n Increases",
        "Speedup (serial time / Pthreads time)",
        [("Pthreads speedup", points(task2, "speedup"), COLORS[1])],
    )
    graph(
        "task3_runtime_increasing_n.svg",
        "Task 3: Serial vs OpenMP Runtime as n Increases",
        "Elapsed wall time (seconds)",
        [
            ("Serial", points(task3, "serial_seconds"), COLORS[0]),
            ("OpenMP", points(task3, "parallel_seconds"), COLORS[2]),
        ],
    )
    graph(
        "task3_speedup_increasing_n.svg",
        "Task 3: OpenMP Speedup as n Increases",
        "Speedup (serial time / OpenMP time)",
        [("OpenMP speedup", points(task3, "speedup"), COLORS[2])],
    )
    graph(
        "pthread_openmp_runtime_increasing_n.svg",
        "Task 3: Pthreads vs OpenMP Runtime as n Increases",
        "Elapsed wall time (seconds)",
        [
            ("Pthreads", points(comparison, "pthreads_seconds"), COLORS[1]),
            ("OpenMP", points(comparison, "openmp_seconds"), COLORS[2]),
        ],
    )


if __name__ == "__main__":
    main()
