
<h1>Encriptor: Parallel File Encryption Tool</h1>
<h2>Multiprocessing Random Permutation Encryption</h2>

<h2>About the Project</h2>
<p>Encriptor is a file encryption tool that leverages multiprocessing to encrypt and decrypt files using a unique random permutation algorithm. The tool breaks down files into segments and processes them concurrently, providing an efficient and parallel approach to text transformation.</p>

<hr>

<h2>System Requirements and Limitations</h2>
<h3>Input Constraints</h3>
<ul>
    <li><strong>Maximum Word Length:</strong> 500 characters</li>
    <li><strong>Supported Characters:</strong>
        <ul>
            <li>Alphabetic characters (A-Z, a-z)</li>
            <li>Numeric characters (0-9)</li>
            <li>Common punctuation marks (.,!?)</li>
            <li>Whitespace characters</li>
        </ul>
    </li>
    <li><strong>File format:</strong>
        <ul>
            <li>It should contain only the above-mentioned characters</li>
            <li>No break lines or special characters</li>
            <li>Text should be continuous without any interruptions</li>
            <li>Avoid having a blank space at the end of the file</li>
        </ul>
    </li>
</ul>

<h3>Processing Constraints</h3>
<ul>
    <li><strong>Maximum Concurrent Processes:</strong> N/A</li>
    <li><strong>Testing Environment:</strong> Linux/macOS</li>
</ul>

<h3>Performance Considerations</h3>
<ul>
    <li>A boost of performance is observed by using a large file and multiple processes.</li>
    <li>Encryption is NOT cryptographically secure</li>
</ul>

<hr>

<h2>How to Use It</h2>
<h3>Prerequisites</h3>
<ul>
    <li>GCC Compiler</li>
    <li>POSIX-compliant Operating System (Linux/macOS)</li>
    <li>Make utility</li>
</ul>

<h3>Installation and Running</h3>

<p>Clone the repository</p>

```bash
git clone https://github.com/yourusername/encriptor.git
cd encriptor
```


Compile the project
```bash
make
```

Run the script
```bash
make run
```

Clean build
```bash
make clean
```

<hr>

<h2>How It Works</h2>
<h3>Encryption Process</h3>
<ol>
    <li><strong>File Segmentation</strong>
        <ul>
            <li>Divides input file into equal-sized segments</li>
            <li>Calculates optimal segment sizes based on file length and process count</li>
        </ul>
    </li>
    <li><strong>Multiprocessing Workflow</strong>
        <ul>
            <li>Uses <code>fork()</code> to create multiple child processes</li>
            <li>Each process handles a specific file segment</li>
            <li>Applies random permutation to words independently</li>
        </ul>
    </li>
    <li><strong>Random Permutation Algorithm</strong>
        <ul>
            <li>Shuffles letter indices of each word</li>
            <li>Generates unique transformation for every word</li>
            <li>Stores original and shuffled indices</li>
        </ul>
    </li>
</ol>

<h3>Decryption Process</h3>
<ul>
    <li>Reverses the permutation process</li>
    <li>Reconstructs original text using stored permutation indices</li>
    <li>Parallel processing for efficient decryption</li>
</ul>

<hr>

<h2>Technical Specifications</h2>
<h3>Multiprocessing Mechanism</h3>
<ul>
    <li><strong>Process Creation</strong>: Uses POSIX <code>fork()</code> system call</li>
    <li><strong>Inter-Process Communication</strong>: Shared memory mapping</li>
    <li><strong>Memory Management</strong>:
        <ul>
            <li><code>mmap()</code> for file and memory mapping</li>
            <li>Dynamic memory allocation</li>
        </ul>
    </li>
</ul>

<h3>Performance Characteristics</h3>
<ul>
    <li><strong>Parallelism</strong>: Concurrent processing of file segments</li>
    <li><strong>Scalability</strong>: Configurable number of processes</li>
    <li><strong>Memory Efficiency</strong>:
        <ul>
            <li>Minimal memory overhead</li>
            <li>Page-aligned memory mapping</li>
            <li>Efficient shared memory usage</li>
        </ul>
    </li>
</ul>

<h3>Implementation Details</h3>
<ul>
    <li><strong>Language</strong>: C (C99 Standard)</li>
    <li><strong>Compilation</strong>: GCC with <code>-Wall -Wextra -g</code> flags</li>
    <li><strong>Supported Platforms</strong>: POSIX-compliant systems</li>
</ul>

<hr>

<h2>License</h2>
<p>MIT License - See LICENSE file for details</p>