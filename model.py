#!/usr/bin/env python
# -*- coding: utf-8 -*-

import random
import torch
import torch.nn as nn
import torch.optim as optim
import argparse
from torch.utils.data import Dataset, DataLoader


parser = argparse.ArgumentParser()
parser.add_argument('-a', '--answer', type=str, help="The correct 5-letter Wordle answer")
parser.add_argument('-e', '--epochs', type=int, default=30, help="Number of training epochs (default=30)")
parser.add_argument('-b', '--batch-size', type=int, default=64, help="Batch size (default=64)")
parser.add_argument('-s', '--save', type=str, default=None, help="Save the model")
parser.add_argument('-l', '--load', type=str, default=None, help="Load the model")
args = parser.parse_args()

with open("answers.txt") as f:
    WORDS = f.read().splitlines()
    target_word = args.answer.lower() if args.answer else random.choice(WORDS)

epochs = args.epochs
batch_size = args.batch_size
save_file = args.save
load_file = args.load

### Dataset
class WordGuessDataset(Dataset):
    def __init__(self, words, char_to_idx):
        self.words = words
        self.char_to_idx = char_to_idx
        self.data = [self.word_to_indices(w) for w in words]

    def word_to_indices(self, word):
        return [self.char_to_idx[c] for c in word]

    def __len__(self):
        return len(self.data)

    def __getitem__(self, idx):
        return torch.tensor(self.data[idx], dtype=torch.long), torch.tensor(self.data[idx], dtype=torch.long)


### Model
class WordGuessModel(nn.Module):
    def __init__(self, vocab_size, hidden_dim):
        super().__init__()
        self.embedding = nn.Embedding(vocab_size, hidden_dim, padding_idx=0)
        self.lstm = nn.LSTM(hidden_dim, hidden_dim, num_layers=2, batch_first=True, dropout=0.3)
        self.fc = nn.Linear(hidden_dim, vocab_size)

    def forward(self, x):
        emb = self.embedding(x)
        out, _ = self.lstm(emb)
        logits = self.fc(out[:, -1, :])
        return logits


### Helper
ALPHABET = "abcdefghijklmnopqrstuvwxyz"

def build_vocab(words):
    char_to_idx = {ch: i + 1 for i, ch in enumerate(ALPHABET)}  # 0 = padding
    return char_to_idx


### Wordle Game Simulation
def get_feedback(guess, target):
    feedback = ['_'] * 5
    for i, c in enumerate(guess):
        if c == target[i]:
            feedback[i] = 'G'
        elif c in target:
            feedback[i] = 'Y'
    return feedback

def filter_candidates(candidates, guess, feedback):
    new_candidates = []
    for word in candidates:
        match = True
        for i, f in enumerate(feedback):
            if f == 'G' and word[i] != guess[i]:
                match = False
            elif f == 'Y':
                if guess[i] == word[i] or guess[i] not in word:
                    match = False
            elif f == '_':
                if guess[i] in word:
                    match = False
        if match:
            new_candidates.append(word)
    return new_candidates

class WordleGame:
    def __init__(self, model, device, char_to_idx, words):
        self.model = model
        self.device = device
        self.char_to_idx = char_to_idx
        self.vocab = words

    def _word_to_tensor(self, word):
        indices = [self.char_to_idx[c] for c in word]
        return torch.tensor(indices, dtype=torch.long, device=self.device).unsqueeze(0)

    def play_game(self, target_word):
        candidates = self.vocab.copy()
        guesses = 0

        while True:
            guesses += 1
            guess = random.choice(candidates)

            if guess == target_word:
                return guesses

            feedback = get_feedback(guess, target_word)
            candidates = filter_candidates(candidates, guess, feedback)
            if not candidates:
                return guesses


### Main
if __name__ == "__main__":
    char_to_idx = build_vocab(WORDS)

    # Dataset & Loader
    dataset = WordGuessDataset(WORDS, char_to_idx)
    loader = DataLoader(dataset, batch_size=batch_size, shuffle=True)

    # Model
    vocab_size = len(ALPHABET) + 1
    hidden_dim = 128
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    model = WordGuessModel(vocab_size, hidden_dim).to(device)

    if load_file is not None:
        model.load_state_dict(torch.load(f"{load_file}.pt", map_location=device))
        model.eval()

    if load_file is None:
        # Loss & Optim
        loss_fn = nn.CrossEntropyLoss()
        optimizer = optim.Adam(model.parameters(), lr=0.001)

        # Train simple language model on words
        for epoch in range(epochs):
            model.train()
            for batch_x, batch_y in loader:
                batch_x, batch_y = batch_x.to(device), batch_y.to(device)
                optimizer.zero_grad()
                logits = model(batch_x)
                loss = loss_fn(logits, batch_y[:, -1])
                loss.backward()
                optimizer.step()

        if save_file is not None:
            torch.save(model.state_dict(), f"{save_file}.pt")

    # Play Wordle
    game = WordleGame(model, device, char_to_idx, WORDS)
    guesses_taken = game.play_game(target_word)
    print(guesses_taken)