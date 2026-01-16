#!/bin/bash
set -uo pipefail

# 1. Compile with debug symbols and all warnings
echo "Compiling..."
rm -rf generated/ utfConverter
make release

# 2. Run conversions
echo "Running conversions..."
./utfConverter test_files/utf32_peq_be.txt
./utfConverter test_files/utf32_peq.txt     
./utfConverter test_files/utf8_peq.txt

# 3. Define comparisons (Expected vs Generated)
comparisons="
expected/utf8_peq_convertido_utf32.txt    generated/CONV_utf8_peq.txt
expected/utf32_peq_convertido_utf8.txt   generated/CONV_utf32_peq.txt
expected/utf32_peq_be_convertido_utf8.txt generated/CONV_utf32_peq_be.txt
"

echo "--- Results ---"
errors=0

while read -r expected generated; do
    # Skip empty lines
    [ -z "$expected" ] && continue
    
    if diff -q "$expected" "$generated" > /dev/null; then
        echo "✅ PASS: $generated matches $expected"
    else
        echo "❌ FAIL: $generated differs from $expected"
        echo "Expected"
        xxd $expected
        echo "Generated"
        xxd $generated
        ((errors++))
    fi
done <<< "$comparisons"
