# Scanner (Lexical Analyzer)

## General Overview

The Scanner (or Lexer) is the first step in our compilation process. Its main job is to read raw source code (which is just a giant, continuous string of text) and group the characters into meaningful "words" or "symbols" that the compiler can actually understand. We call these units **Tokens**.

For example, if we write the following code in our file:
`var x = 10;`

The Scanner doesn't read a sentence; internally it sees:
`[Letter v][Letter a][Letter r][Space][Letter x]...`

And its only job is to process that and output something clean, logical, and categorized:
`[TOKEN_VAR] [TOKEN_IDENTIFIER("x")] [TOKEN_EQUAL] [TOKEN_NUMBER("10")] [TOKEN_SEMICOLON]`

Unlike classic compilers where the scanner eats the whole file at once returning a gigantic (and memory-heavy) list of all Tokens at the same time, our scanner in clox works **on demand** (pull model).
The compiler repeatedly calls the scanner saying *"give me the next token"*. The scanner then "wakes up", processes a bit of code, generates a single token, and waits until it's asked for another one.

---

## Technical Details and Inner Workings

To fully grasp what the code does behind the scenes and why certain technical decisions were made (like how loops are managed), here is a step-by-step breakdown.

### 1. The Scanner's State: `start` and `current` Pointers

Working purely in C means dynamically handling text strings can destroy our compiler's performance. So we don't get lost when reading the input sequence, the Scanner stores its state in core variables using two reading pointers: `start` and `current`.

```text
Code:      v  a  r     x     =     1  0  ;  \0
           ^  ^
       start  current
```

- **`start`**: Always points to the first letter of the Token we are **currently** analyzing.
- **`current`**: This is our "hungry" explorer. It steps forward letter by letter looking ahead to figure out what shape the current Token has.

**Why do we use this twin-pointer technique?**
Performance and static memory control! Instead of allocating new memory (`malloc` and `strcpy`) on every call to copy the variable's "text", the generated Token simply boils down to storing a pointer to the exact source code (`start`) and a length calculated by subtracting spatial memory addresses (`current - start`). Zero useless copies!

### 2. Main Flow: `scanToken()`

This function orchestrates all the work. Its exact logical lifecycle on each call is as follows:

1. **`skipWhitespace()`**: Its first job is to clear the path for the compiler of all "human garbage" like spaces, newlines, and comments, getting rid of them before it even attempts to generate a Token.
2. **`scanner.start = scanner.current;`**: We update (anchor) the start pointer making it match wherever we ended up after clearing spaces.
3. We check with `isAtEnd()` if we hit the `\0` mark (end of file). If so, immediately send `TOKEN_EOF`.
4. The explorer reads the character it's pointing to and moves forward with `advance()`.
5. Thanks to a router (with many `if` / `switch` statements), depending on what the character was, it redirects responsibilities to specific subfunctions (like processing text or processing numbers).

### 3. Anatomy of the Clean-up: `skipWhitespace()`

The function that clears the weeds isn't as dumb as it seems. It runs an infinite loop `for(;;)` reviewing the code depending on what it reads with a glance (`peek`, which returns the char but **does not consume** it, meaning the pointer doesn't advance).

**Quirks and Technical Reasons:**
- **Newlines (`\n`):** When it sees a blank newline, it doesn't just advance; it importantly does **`scanner.line++`**. It is absolutely crucial to do this count here because it's the only way to know what line we're on if later the compiler finds an error and has to yell: *"Hey! Error on line 32!"*.
- **Comments (`//`):** To us, comments don't matter; they act like a huge blank space. If we see the char pointed to is a slash `/` and immediately the next one is also `/` (`peekNext`), we interpret it as a comment. In this case, the inner loop enters a destructive frenzy, eating and advancing letter by letter until hitting a newline. *If it turns out it was just a normal mathematical division slash `/`, the process pauses so `scanToken()` can safely capture it as a normal Token later.*

### 4. Handling Strings (`string()`)

When `scanToken()` finds double quotes `"`, it begins absorbing everything.
The algorithm is to devour everything that exists until stumbling upon another closing double quote, and if it hits the end of the file, it returns the dreaded unterminated string error.
**The super important technical nuance:** The compiler for the _Lox_ language (to which this project belongs) supports and accepts _multi-line strings_ (strings with enter breaks in the middle of the sentence). Because of this, if internally during its collecting journey it forces its way through newlines `\n`, the global variable `scanner.line` is manually incremented so future line errors don't lose synchronization.

### 5. Recognizing Words ("Identifiers" vs "Keywords")

The language is forced to detect regular English characters or underscores `_` (`isAlpha`). If it finds one, it begins greedily eating letters and any extra digits using `identifier()`. 

When finishing the full lexeme, a huge dilemma comes into play: **We have the word "while". Is this a user variable named "while" or is it a reserved keyword (the while loop) of our own language?**
Here, our subfunction will immediately validate and test the word to assign categories using *`identifierType()`*: 

**Why use such a strange validator like a `switch` (a hand-coded trie) instead of a friendly Hash Table to find valid keywords?**
1. **Extreme Performance and Speed:** A Hash Table requires iterating mathematically over all the letters in the generated word to create a numerical value (Hash), going to the table, solving conflicts if the index clashes, and comparing its contents again. Our engine uses a gigantic hand-crafted, letter-by-letter `switch(...)` that is infinitely faster in raw compute time: If the first letter of the captured word is a 'w', the processor skips dozens of checks and then simply verifies internally and mathematically by pointer size (overall length) and by comparing the remaining literal string to see if the next positions are `h - i - l - e`. Ultra-fast determinism.
2. If both mathematical components match, the validation is instantaneous and returns, e.g., `TOKEN_WHILE`. If the comparison fails, the lexer scraps the idea of a "Reserved Word" and falls back to the default `TOKEN_IDENTIFIER` (assuming it was a name invented by the programmer).

### 6. Combined Tokens or "Lookahead" (`!=`, `<=`, `==`)

When we are collecting mathematical operators or simple comparators, we have a problem. Certain syntax parts are resolved in one letter, for example `!` would return the negation conditional order, but if you don't look ahead, when a programmer tries to add an inequality `!=`, your program would only emit the `!`.

We crush this issue with the commuter method **`match('=')`**:
When bumping into one of the tricky cases, it checks the letter immediately following the current sequence.
- If the next text is an `=`: then `match` internally devours the space making our real pointer step one additional spot; this way, the system notifies back a superior combined Token, e.g. `TOKEN_BANG_EQUAL`.
- Conversely, if the next letter is just a normal space `! _`: it doesn't advance any fragment and safeguards a `false` validation, defaulting safely to a single regular emission `TOKEN_BANG`.

---

### 7. Visual Step-by-Step: Generating a Token

Let's visually trace how the exact token `var` is generated from the string `"var x = 10;"`.

```text
[Initial State]
Compiler calls: scanToken()
Code:  v  a  r     x  ...
       ^
    start/current

Step 1: skipWhitespace()
Action: No spaces found at the current index. Doesn't advance.

Step 2: Anchor 'start'
Action: scanner.start = scanner.current
Code:  v  a  r     x  ...
       ^
    start/current

Step 3: Read Character
Action: char c = advance();
Code:  v  a  r     x  ...
       ^  ^
   start  current
Result: c = 'v'

Step 4: Route by Character
Action: isAlpha('v') is true -> calls identifier()

Step 5: identifier() Loop
Action: While peek() isAlpha or isDigit, advance()
Code:  v  a  r     x  ...
       ^     ^
   start     current
Loop consumes 'a' and 'r'. Stops at ' ' (space).
Code:  v  a  r     x  ...
       ^           ^
   start           current

Step 6: Identify Keyword
Action: identifierType() is called.
- Checks start[0] which is 'v'.
- Length is current - start (3).
- Matches "ar" for the remaining 2 characters.
Result: Returns TOKEN_VAR

Step 7: makeToken(TOKEN_VAR)
Action: Creates the Token struct.

[Resulting Token Struct]
 ├─ type:   TOKEN_VAR
 ├─ start:  [pointer to 'v' in original string]
 ├─ length: 3
 └─ line:   1

Compiler receives the Token and pauses until the next call!
```
