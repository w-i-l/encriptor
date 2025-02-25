import re
import random

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
        

def generate_file(filename, number_of_words):
    '''
    Generate a file with random words, with the number of words specified.
    The words are separated by spaces.
    They can contain letters, numbers, and only , . ? !
    No new lines.
    The length of the words is random, between 1 and 20 characters.
    '''

    allowed_chars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,?!'

    with open(filename, 'w') as f:
        for _ in range(number_of_words):
            word = ''.join(random.choices(allowed_chars, k=random.randint(1, 20)))
            f.write(word + (' ' if _ < number_of_words - 1 else ''))



# compare_files('files/file.txt', 'files/verification.txt')
# format_file('files/verification.txt')

generate_file('files/verification.txt', 1_000_000)