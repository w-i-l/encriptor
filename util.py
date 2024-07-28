import re

def format_file(filename):

    with open(filename, 'r') as f:
        text = f.read()
        text = re.sub(r'\s+', ' ', text)
        text = re.sub(r'(\w)([.,!?])', r'\1 \2', text)
        text = re.sub(r'([.,!?])(\w)', r'\1 \2', text)
        text = re.sub(r'\s+', ' ', text)
        text = re.sub(r'\s([.,!?])', r'\1', text)

    with open(filename, 'w') as f:
        f.write(text)

format_file('files/file.txt')

def compare_files(first, second):
    with open(first, 'r') as f:
        first_text = f.read()
    with open(second, 'r') as f:
        second_text = f.read()
    
    chars = 0
    for _, (word1, word2) in enumerate(zip(first_text.split(), second_text.split())):
        chars += len(word2) + 1
        if word1 != word2:
            print(word1, word2, chars, sep=' | ')
            return False

compare_files('files/file.txt', 'files/verification.txt')