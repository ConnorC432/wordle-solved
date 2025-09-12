import json
import wordle

from multiprocessing import Pool, cpu_count
from pathlib import Path
from tqdm import tqdm

def process_guess(args):
    guess, solutions, tmp_dir = args

    results = {solution: wordle.get_feedback(guess, solution) for solution in solutions}

    obj = {guess: results}

    tmp_file = tmp_dir / f"{guess}.json"
    with open(tmp_file, "w") as f:
        json.dump(obj, f, indent=2)
        f.write("\n")

    return tmp_file

if __name__ == "__main__":
    guesses = wordle.get_solutions()
    solutions = guesses
    output_file = Path("JSON/feedback.json")
    tmp_dir = Path("JSON/tmp")
    tmp_dir.mkdir(exist_ok=True)

    if output_file.exists():
        output_file.unlink()

    tasks = [(guess, solutions, tmp_dir) for guess in guesses]

    with Pool(cpu_count()) as pool:
        for tmp_file in tqdm(pool.imap_unordered(process_guess, tasks), total=len(tasks)):
            pass

    with open(output_file, "w") as final_f:
        for tmp_file in sorted(tmp_dir.iterdir()):
            with open(tmp_file, "r") as f:
                final_f.write(f.read())
            tmp_file.unlink()

    tmp_dir.rmdir()
