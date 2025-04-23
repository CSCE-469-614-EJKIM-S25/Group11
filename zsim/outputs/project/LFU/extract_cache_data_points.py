import os
import re
import csv

def extract_l3_and_instrs(zsim_path):
    stats = {'benchmark': os.path.basename(os.path.dirname(zsim_path))}

    with open(zsim_path, 'r') as f:
        lines = f.readlines()

    l3_totals = {"mGETS": 0, "mGETXIM": 0, "mGETXSM": 0}
    total_instrs = 0
    total_cycles = 0
    total_cCycles = 0

    in_l3_block = False
    in_core_block = False

    for line in lines:
        # Detect L3 blocks (e.g., "  l3-0:" or "  l3-0bX:")
        if re.match(r'\s+l3-0(?:b\d+)?:', line):
            in_l3_block = True
            continue
        if in_l3_block and not line.startswith("  "):
            in_l3_block = False
            continue
        if in_l3_block:
            for key in l3_totals:
                if key in line:
                    value = int(re.search(r'(\d+)', line).group(1))
                    l3_totals[key] += value

        # Detect core blocks (e.g., "  westmere-0:")
        if re.match(r'\s+westmere-\d+:', line):
            in_core_block = True
            continue
        if in_core_block and not line.startswith("  "):
            in_core_block = False
            continue
        if in_core_block:
            if "instrs" in line and "approxInstrs" not in line:
                match = re.search(r'(\d+)', line)
                if match:
                    total_instrs += int(match.group(1))
            elif "cycles" in line:
                match = re.search(r'(\d+)', line)
                if match:
                    total_cycles += int(match.group(1))
            elif "cCycles" in line:
                match = re.search(r'(\d+)', line)
                if match:
                    total_cCycles += int(match.group(1))

    # Derived metrics
    total_misses = l3_totals["mGETS"] + l3_totals["mGETXIM"] + l3_totals["mGETXSM"]
    overall_cycles = total_cycles + total_cCycles
    ipc = total_instrs / overall_cycles if overall_cycles > 0 else 0
    mkpi = (total_misses / total_instrs * 1000) if total_instrs > 0 else 0

    stats.update(l3_totals)
    stats['instrs'] = total_instrs
    stats['cycles'] = total_cycles
    stats['cCycles'] = total_cCycles
    stats['overall_cycles'] = overall_cycles
    stats['IPC'] = round(ipc, 4)
    stats['total_misses'] = total_misses
    stats['MKPI'] = round(mkpi, 4)

    return stats

# Main directory
root_dir = "."
output_csv = "l3_and_instrs_summary.csv"

# Process all benchmark folders
all_data = []

for folder in os.listdir(root_dir):
    bench_dir = os.path.join(root_dir, folder)
    zsim_file = os.path.join(bench_dir, "zsim.out")

    if os.path.isdir(bench_dir) and os.path.isfile(zsim_file):
        try:
            stats = extract_l3_and_instrs(zsim_file)
            all_data.append(stats)
        except Exception as e:
            print(f"⚠️ Failed to process {folder}: {e}")

# Write CSV
with open(output_csv, "w", newline="") as f:
    fieldnames = [
        "benchmark", "mGETS", "mGETXIM", "mGETXSM", "instrs",
        "cycles", "cCycles", "overall_cycles", "IPC", "total_misses", "MKPI"
    ]
    writer = csv.DictWriter(f, fieldnames=fieldnames)
    writer.writeheader()
    for row in all_data:
        writer.writerow(row)

print(f"\n✅ Done! Results saved to {output_csv}")
