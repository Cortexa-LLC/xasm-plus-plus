#!/usr/bin/env python3
import re
import sys

def fix_sign_comparisons(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    original = content
    
    # Fix patterns like: EXPECT_EQ(x.size(), 0) -> EXPECT_EQ(x.size(), 0UL)
    content = re.sub(r'(EXPECT_EQ|ASSERT_EQ|EXPECT_GE|ASSERT_GE|EXPECT_GT|ASSERT_GT|EXPECT_LT|ASSERT_LT|EXPECT_LE|ASSERT_LE)\(([^,]+\.size\(\)[^,]*),\s*(\d+)\)', r'\1(\2, \3UL)', content)
    
    # Fix patterns like: EXPECT_EQ(some_uint_var, 123) -> EXPECT_EQ(some_uint_var, 123U)
    # But only for common unsigned patterns
    content = re.sub(r'(EXPECT_EQ|ASSERT_EQ)\(([^,]+->(?:address|size|data\[[^\]]+\])[^,]*),\s*(\d+)\)', r'\1(\2, \3U)', content)
    
    # Fix patterns like: EXPECT_EQ(count_var, 3) -> EXPECT_EQ(count_var, 3U) for count variables
    content = re.sub(r'(EXPECT_EQ|ASSERT_EQ)\(([a-zA-Z_][a-zA-Z0-9_]*_count),\s*(\d+)\)', r'\1(\2, \3U)', content)
    
    # Fix hex literals without U suffix in comparisons
    content = re.sub(r'(EXPECT_EQ|ASSERT_EQ)\(([^,]+->address[^,]*),\s*(0x[0-9a-fA-F]+)\)', r'\1(\2, \3U)', content)
    
    if content != original:
        with open(filename, 'w') as f:
            f.write(content)
        return True
    return False

if __name__ == '__main__':
    for filename in sys.argv[1:]:
        if fix_sign_comparisons(filename):
            print(f"Fixed: {filename}")
