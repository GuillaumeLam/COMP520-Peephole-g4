import csv
import re
import os

from scipy.special import comb

base = './PeepholeBenchmarks'

files = []

i = 0

full_command = False

for direc in os.listdir(base):
	for filename in os.listdir(base + '/' + direc):
		if filename.endswith('.j'):
			lines = []
			with open(base + '/' + direc + '/' + filename) as file:
				for line in file:
					line = line.strip()
					line = re.sub('\d', '#', line)

					if not line.startswith('.') and not line.endswith(':') and line:
						if not full_command:
							line = line.split()[0]

						lines.append(line)

			files.append(lines)


def find_pattern(list_a, list_b):
	min_len = 2

	def find_ngrams(input_list, n):
		return zip(*[input_list[i:] for i in range(n)])

	seq_list_a = []

	for seq_len in range(min_len, len(list_a) + 1):
		seq_list_a += [val for val in find_ngrams(list_a, seq_len)]

	seq_list_b = []
	for seq_len in range(min_len, len(list_b) + 1):
		seq_list_b += [val for val in find_ngrams(list_b, seq_len)]

	all_sequences = seq_list_a + seq_list_b

	counter = {}
	for seq in all_sequences:
		counter[seq] = counter.get(seq, 0) + 1

	filtered_counter = {k: v for k, v in counter.items() if v > 1}

	return filtered_counter


# 1: check for exact matches
patterns = {}
count = 0

for i, a in enumerate(files):
	for j, b in enumerate(files):
		if not i <= j:
			patterns.update(find_pattern(a, b))
			count += 1
			print('done ' + str(count) + '/' + str(int(comb(len(files), 2))))

patterns = sorted(patterns.items(), key=lambda x: x[1], reverse=True)

# 2: check for only keycommand matches

if full_command:
	w = csv.writer(open('patterns1.csv', 'w'))
else:
	w = csv.writer(open('patterns2.csv', 'w'))

for key, val in patterns:
	w.writerow([key, val])

