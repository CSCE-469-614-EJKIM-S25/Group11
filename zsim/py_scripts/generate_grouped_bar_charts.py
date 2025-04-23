
import os
import pandas as pd
import matplotlib.pyplot as plt

# Define paths for each replacement policy
policies = ['LFU', 'LRU', 'SRRIP', 'SHiP']
colors = {'LFU': '#FF0000', 'LRU': '#49b344', 'SRRIP': '#0352fc', 'SHiP': '#A18594'}
metrics = ['overall_cycles', 'IPC', 'MKPI']
metric_titles = {'overall_cycles': 'Total Cycles', 'IPC': 'IPC', 'MKPI': 'MPKI'}
ylabels = {'overall_cycles': 'Total Cycles (1E9)', 'IPC': 'IPC', 'MKPI': 'MPKI'}

# Benchmark labels and mapping for formatting
benchmarks_spec = [
    'bzip2', 'gcc', 'mcf', 'hmmer', 'sjeng', 'libquantum', 'xalan',
    'cactusADM', 'namd', 'soplex', 'calculix', 'lbm'
]
benchmarks_parsec = [
    'blackscholes_8c_simlarge', 'bodytrack_8c_simlarge', 'canneal_8c_simlarge',
    'fluidanimate_8c_simlarge', 'streamcluster_8c_simlarge',
    'swaptions_8c_simlarge', 'x264_8c_simlarge'
]
ignore_benchmarks = ['dedup_8c_simlarge', 'freqmine_8c_simlarge']
all_benchmarks = benchmarks_spec + benchmarks_parsec
formatted_labels = {
    'blackscholes_8c_simlarge': 'blackscholes',
    'bodytrack_8c_simlarge': 'bodytrack',
    'canneal_8c_simlarge': 'canneal',
    'fluidanimate_8c_simlarge': 'fluidanimate',
    'streamcluster_8c_simlarge': 'streamcluster',
    'swaptions_8c_simlarge': 'swaptions',
    'x264_8c_simlarge': 'x264'
}

# Plotting function
def plot_grouped_bar(metric, title):
    data = {policy: [] for policy in policies}

    for policy in policies:
        path = os.path.join(policy, 'l3_and_instrs_summary.csv')
        if not os.path.isfile(path):
            print(f"File not found: {path}")
            return

        df = pd.read_csv(path)

        # Filter and reorder benchmarks
        df = df[~df['benchmark'].isin(ignore_benchmarks)]
        df = df.set_index('benchmark').reindex(all_benchmarks)

        data[policy] = df[metric].tolist()

    # Build plot
    num_benchmarks = len(all_benchmarks)
    x = range(num_benchmarks)
    bar_width = 0.20

    fig, ax = plt.subplots(figsize=(18, 7))
    for i, policy in enumerate(policies):
        offset = [xi + i * bar_width - bar_width for xi in x]
        bar_data = data[policy]

        if metric == 'overall_cycles':
            bar_data = [v / 1e9 for v in bar_data]

        bars = ax.bar(offset, bar_data, width=bar_width, label=policy, color=colors[policy])

        # Add bar values outside the bars
        for j, bar in enumerate(bars):
            height = bar.get_height()
            label = f'{height:.3f}' if metric == 'overall_cycles' else f'{height:.2f}'
            ax.text(
                bar.get_x() + bar.get_width() / 2, height,
                label, ha='center', va='bottom', rotation=90, fontsize=8
            )

    # Add separation line between SPEC and PARSEC
    spec_len = len(benchmarks_spec)
    ax.axvline(spec_len - 0.5, color='black', linestyle='--')

    # X-tick labels
    xtick_labels = [formatted_labels.get(b, b) for b in all_benchmarks]
    ax.set_xticks([r for r in x])
    ax.set_xticklabels(xtick_labels, rotation=45, ha='right')

    # Add bold SPEC and PARSEC category titles below x-axis
    mid_spec = (spec_len - 1) / 2
    mid_parsec = spec_len + (len(benchmarks_parsec) - 1) / 2
    y_min, _ = ax.get_ylim()
    ax.text(mid_spec, y_min - (y_min * 0.05), 'SPEC', ha='center', va='top', fontsize=10, fontweight='bold')
    ax.text(mid_parsec, y_min - (y_min * 0.05), 'PARSEC', ha='center', va='top', fontsize=10, fontweight='bold')

    ax.set_title(title)
    ax.set_ylabel(ylabels[metric])
    ax.legend()
    plt.tight_layout()
    plt.savefig(f'{metric}_grouped_bar_chart.png')
    print(f"Saved: {metric}_grouped_bar_chart.png")
    plt.close()

# Generate plots for all three metrics
for metric in metrics:
    plot_grouped_bar(metric, metric_titles[metric])
