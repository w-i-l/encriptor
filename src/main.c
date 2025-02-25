#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "headers/coder.h"
#include "headers/coder_multiprocess.h"
#include "headers/random_permutation.h"
#include "headers/segmentation.h"

const bool IS_DEBUG_ENABLED = false;

// Test function to verify permutation encoding/decoding works correctly
void test_permutation_functions() {
    printf("\n==== Testing Permutation Functions ====\n");

    // Test word
    char test_word[] = "hello";
    printf("Original word: '%s'\n", test_word);

    // Create permutation
    permutation p = random_permutation(test_word);
    printf("Permutation: word='%s', length=%zu\n", p.char_permutation, p.length);

    // Print original indices
    printf("Indices: ");
    for (size_t i = 0; i < p.length; i++) {
        printf("%d ", p.int_permutation[i]);
    }
    printf("\n");

    // Encode permutation to string
    char *encoded = encoding_permutation_to_string(p);
    printf("Encoded permutation: '%s'\n", encoded);

    // Decode permutation from string
    int_permutation ip = decode_int_permutation(encoded);
    printf("Decoded permutation length: %zu\n", ip.length);

    // Recreate permutation
    permutation p2 = create_permutation(p.char_permutation, ip.int_permutation);
    p2.length = ip.length;

    // Decode the permutation
    char *decoded = decode_permutation(p2);
    printf("Decoded word: '%s'\n", decoded);

    // Free memory
    free(encoded);
    free(decoded);
    free(ip.int_permutation);

    printf("==== End of Test ====\n\n");
}

// Add after decoding is complete
void verify_file_integrity(const char *original_file,
                                                     const char *decoded_file) {
    FILE *orig = fopen(original_file, "r");
    FILE *dec = fopen(decoded_file, "r");

    if (orig == NULL || dec == NULL) {
        printf("Error opening files for verification\n");
        return;
    }

    fseek(orig, 0, SEEK_END);
    fseek(dec, 0, SEEK_END);
    long orig_size = ftell(orig);
    long dec_size = ftell(dec);

    printf("Original file size: %ld bytes\n", orig_size);
    printf("Decoded file size: %ld bytes\n", dec_size);
    printf("Difference: %ld bytes\n", orig_size - dec_size);

    // Reset file positions
    fseek(orig, 0, SEEK_SET);
    fseek(dec, 0, SEEK_SET);

    // Compare character by character
    int orig_char, dec_char;
    long pos = 0;
    int mismatches = 0;

    while ((orig_char = fgetc(orig)) != EOF && (dec_char = fgetc(dec)) != EOF) {
        if (orig_char != dec_char) {
            printf(
                    "Mismatch at position %ld: Original '%c' (0x%02X), Decoded '%c' "
                    "(0x%02X)\n",
                    pos, orig_char, orig_char, dec_char, dec_char);
            mismatches++;
            if (mismatches >= 10) break;    // Limit output
        }
        pos++;
    }

    // Check if one file is longer
    if (orig_char != EOF) {
        printf("Original file has more content starting at position %ld\n", pos);
    } else if (dec_char != EOF) {
        printf("Decoded file has more content starting at position %ld\n", pos);
    }

    fclose(orig);
    fclose(dec);
}

int main(void) {
    if (IS_DEBUG_ENABLED) {
        printf("Debugging is enabled\n");
        test_permutation_functions();
    }
    // Test the permutation functions first to verify they work correctly

    char input_filename[] = "files/verification.txt";
    char permutations_filename[] = "files/permutations.txt";
    char encoded_filename[] = "files/encoded.txt";
    char output_filename[] = "files/decoded.txt";

    // Number of processes to use - you can change this to increase parallelism
    int process_count = 40;    // Try with 2 processes initially

    // Create a simple test file if it doesn't exist
    FILE *test_file = fopen(input_filename, "r");
    if (test_file == NULL) {
        printf("Creating test file...\n");
        test_file = fopen(input_filename, "w");
        if (test_file != NULL) {
            fprintf(
                    test_file,
                    "The Project Gutenberg eBook of Deadfalls and Snares This ebook is "
                    "for the use of anyone anywhere in the United States and most other "
                    "parts of the world at no cost and with almost no restrictions "
                    "whatsoever. You may copy it, give it away or re-use it under the "
                    "terms of the Project Gutenberg License included with this ebook or "
                    "online at www. gutenberg. org. If you are not located in the United "
                    "States, you will have to check the laws of the country where you "
                    "are located before using this eBook.\n");
            fclose(test_file);
            printf("Test file created with sample content\n");
        }
    } else {
        fclose(test_file);
        printf("Using existing input file\n");
    }

    printf("\n==== Starting encryption process with %d process(es) ====\n",
                 process_count);
    printf("Input file: %s\n", input_filename);
    printf("Output files: %s and %s\n", encoded_filename, permutations_filename);
    fflush(stdout);

    // Display input file content
    test_file = fopen(input_filename, "r");
    if (test_file != NULL && IS_DEBUG_ENABLED) {
        printf("\nInput file content:\n");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), test_file) != NULL) {
            printf("%s", buffer);
        }
        fclose(test_file);
        printf("\n");
    }

    // Get segments for the input file based on the number of processes
    process_segment_info *segments =
            get_segments_for_file(input_filename, process_count);
    if (segments == NULL) {
        fprintf(stderr, "Failed to segment file\n");
        return 1;
    }

    if (IS_DEBUG_ENABLED) {
        printf("\nSegment information:\n");
        for (int i = 0; i < process_count; i++) {
            print_segment_info(segments[i]);
            printf("\n");
        }
    }

    printf("Is file segmented correctly: %s\n",
                 is_file_segmented_correctly(input_filename, segments, process_count)
                         ? "Yes"
                         : "No");

    printf("\n==== ENCODING ====\n");
    fflush(stdout);
    int encode_result = encode_multiprocess(
        input_filename, 
        permutations_filename,
        encoded_filename, segments,
        process_count
    );
    if (encode_result == -1) {
        perror("Error encoding file");
        free(segments);
        return 1;
    }

    if (IS_DEBUG_ENABLED) {
        // Add debugging to check the created files
        printf("\nChecking created files...\n");
    }

    // Check encoded file
    test_file = fopen(encoded_filename, "r");
    if (test_file == NULL) {
        perror("Encoded file was not created");
    } else if (IS_DEBUG_ENABLED) {
        printf("Encoded file content:\n");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), test_file) != NULL) {
            printf("%s", buffer);
        }
        fclose(test_file);
    }

    // Check permutations file
    test_file = fopen(permutations_filename, "r");
    if (test_file == NULL) {
        perror("Permutations file was not created");
    } else if (IS_DEBUG_ENABLED) {
        printf("\nPermutations file content:\n");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), test_file) != NULL) {
            printf("%s", buffer);
        }
        fclose(test_file);
    }

    printf("\n==== DECODING ====\n");

    process_segment_info *segments2 =
            get_segments_for_file(encoded_filename, process_count);
    if (segments2 == NULL) {
        fprintf(stderr, "Failed to segment encoded file\n");
        free(segments);
        return 1;
    }

    if (IS_DEBUG_ENABLED) {
        printf("\nEncoded file segment information:\n");
        for (int i = 0; i < process_count; i++) {
            print_segment_info(segments2[i]);
            printf("\n");
        }
    }

    int decode_result =
            decode_multiprocess(encoded_filename, permutations_filename,
                                                    output_filename, segments2, process_count);
    if (decode_result == -1) {
        perror("Error decoding file");
        free(segments);
        free(segments2);
        return 1;
    }

    // Check output file
    test_file = fopen(output_filename, "r");
    if (test_file == NULL) {
        perror("Output file was not created");
    } else if (IS_DEBUG_ENABLED) {
        printf("\nDecoded output file content:\n");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), test_file) != NULL) {
            printf("%s", buffer);
        }
        fclose(test_file);
    }

    free(segments);
    free(segments2);

    printf("\n==== Process completed ====\n");

    if (IS_DEBUG_ENABLED) {
        verify_file_integrity(input_filename, output_filename);
    }
    return 0;
}