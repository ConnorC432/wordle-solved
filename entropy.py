import json
import wordle

from multiprocessing import Pool, cpu_count

def get_entropies(solutions, all_solutions):
    tasks = [(guess, solutions) for guess in all_solutions]

    with Pool(cpu_count()) as pool:
        results = pool.starmap(wordle.get_entropy, tasks)

    return results

if __name__ == '__main__':
    words = wordle.get_solutions()
    results = get_entropies(words, words)

    entropy_dict = {word: entropy for word, entropy in results}

    with open("JSON/entropy.json", "w") as f:
        json.dump(entropy_dict, f, indent=2)