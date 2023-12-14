import re

def format_file(filename):
    # betweemn all the wrods can be only one space
    # should keep all the special characters

    with open(filename, 'r') as f:
        text = f.read()
        text = re.sub(r'\s+', ' ', text)
        text = re.sub(r'(\w)([.,!?])', r'\1 \2', text)
        text = re.sub(r'([.,!?])(\w)', r'\1 \2', text)
        text = re.sub(r'\s+', ' ', text)
        text = re.sub(r'\s([.,!?])', r'\1', text)

    with open(filename, 'w') as f:
        f.write(text)

format_file('file.txt')

def compare_files(first, second):
    with open(first, 'r') as f:
        first_text = f.read()
    with open(second, 'r') as f:
        second_text = f.read()
    
    for word1, word2 in zip(first_text.split(), second_text.split()):
        if word1 != word2:
            print(word1, word2, sep=' | ')
            return False

compare_files('file.txt', 'decoded.txt')